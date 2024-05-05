/**
 * Copyright (c) 2016, The Linux Foundation. All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are
 * met:
 *     * Redistributions of source code must retain the above copyright
 *       notice, this list of conditions and the following disclaimer.
 *     * Redistributions in binary form must reproduce the above
 *       copyright notice, this list of conditions and the following
 *       disclaimer in the documentation and/or other materials provided
 *       with the distribution.
 *     * Neither the name of The Linux Foundation nor the names of its
 *       contributors may be used to endorse or promote products derived
 *       from this software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED "AS IS" AND ANY EXPRESS OR IMPLIED
 * WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF
 * MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NON-INFRINGEMENT
 * ARE DISCLAIMED.  IN NO EVENT SHALL THE COPYRIGHT OWNER OR CONTRIBUTORS
 * BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
 * CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
 * SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR
 * BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY,
 * WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE
 * OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN
 * IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 *
 */

/**
@file
@internal

Object Push Client
*/

#define __OI_MODULE__ OI_MODULE_OPP_CLI

#include "oi_bt_assigned_nos.h"
#include "oi_memmgr.h"
#include "oi_obexcli.h"
#include "oi_opp_client.h"
#include "oi_opp_common.h"
#include "oi_dispatch.h"

#include "oi_utils.h"
#include "oi_debug.h"
#include "oi_argcheck.h"
#include "oi_assert.h"

#include "oi_init_flags.h"
#include "oi_config_table.h"
#include "oi_bt_profile_config.h"


/**
 * These are the various client connection states.
 */

typedef enum {
    CLIENT_STATE_DISCONNECTED,      /**< Initial state */
    CLIENT_STATE_CONNECTING,        /**< In the process of creating service-level connection */
    CLIENT_STATE_CONNECTED,         /**< Connected, no operations in progress */
    CLIENT_STATE_PUSH_PENDING,      /**< Waiting on OFS read open confirm before starting 1st PUT */
    CLIENT_STATE_PUSHING,           /**< Putting an object */
    CLIENT_STATE_PULL_PENDING,      /**< From issuing 1st get until OFS write open confirm is called */
    CLIENT_STATE_PULLING            /**< While getting and OFS writing */
} OPP_CLIENT_STATE;


typedef enum {
    OPP_NO_ABORT,
    OPP_ABORT_SENT,
    OPP_ABORT_CONFIRMED
} OPP_ABORT_STATE;

/**
 * Struct for an OPP client connection.
 */

typedef struct {

    OI_OPP_GENERIC_OBJECT curObj;           /**< Keeps track of received object state */
    OI_OBEXCLI_CONNECTION_HANDLE id;        /**< The underlying OBEX client connection */
    OI_OPP_CLIENT_EVENT_CB eventCB;         /**< Callback function to report operation completion */
    OI_OPP_CLIENT_CANCEL_CFM cancelCfm;     /**< Callback function to report operation cancellation completion */
    const OI_OPP_OBJSYS_FUNCTIONS *objops;  /**< Interface to an object filing system */
    OI_OPP_HANDLE handle;                   /**< The handle for object operations */
    OI_UINT32 maxReadSize;                  /**< Maximum packet read size */
    DISPATCH_CB_HANDLE pendingClose;        /**< Close operation that has been registered with the dispatcher */
    OPP_CLIENT_STATE state;                 /**< State of this connection */
    OPP_ABORT_STATE abortState;
    OI_BOOL  final;                         /**< Indicates if this is a final packet */
    OI_BOOL  ofsCfmPending;                 /**< Waiting for Object File System to call our cfm function */
    OI_BOOL  disconnected;                  /**< Received disconnect indication, waiting to call event callback. */
    OI_BOOL  nameOverrideEnabled;           /**< Require Name Override <default: FALSE> */
    OI_BOOL  nameOverridePending;           /**< Required Name Override pending */
    OI_BOOL  pullStarted;                   /**< Status to be reported on closing an object */
    OI_STATUS closeStatus;

} OPP_CLIENT;

static void AbortCfm(OI_OBEXCLI_CONNECTION_HANDLE connectionId);

static void DeferredPushOpenCfm(OI_OPP_HANDLE handle,
                               const OI_OBEX_UNICODE *name,
                               const OI_CHAR *type,
                               OI_UINT32 size,
                               OI_BYTE * data,
                               OI_UINT16 data_len,
                               OI_STATUS status,
                               OI_OPP_CONNECTION oppContext);

#define LookupClient(handle)   ((OPP_CLIENT*)OI_OBEXCLI_GetConnectionContext(handle))


#define IS_CLIENT_CONNECTED(client)   ((client) && (client->state >= CLIENT_STATE_CONNECTED))
#define IS_CLIENT_CONNECTING(client)  ((client) && (client->state == CLIENT_STATE_CONNECTING))
#define IS_CLIENT_IDLE(client)        ((client) && (client->state == CLIENT_STATE_CONNECTED) && (client->abortState == OPP_NO_ABORT))

/**
 * Connection policy.
 */
static const OI_CONNECT_POLICY  connectPolicy =
{
    OI_ELEMENT_UUID32(OI_UUID_OBEXObjectPush), /* OI_DATAELEM         serviceUuid          */
    FALSE,                                     /* OI_BOOL             mustBeMaster         */
    NULL,                                      /* OI_L2CAP_FLOWSPEC   *flowspec            */
    0                                          /* OI_UINT8            powerSavingDisables  */
};


#define OI_CASE(a) case a: return(#a);


OI_CHAR *OI_OPPClient_eventText(OI_OPP_CLIENT_EVENT event)
{
#ifdef OI_DEBUG
    switch (event) {
        OI_CASE(OI_OPP_CLIENT_CONNECTED);
        OI_CASE(OI_OPP_CLIENT_DISCONNECT);
        OI_CASE(OI_OPP_CLIENT_PUSH_STARTED);
        OI_CASE(OI_OPP_CLIENT_PULL_STARTED);
        OI_CASE(OI_OPP_CLIENT_PUSH_PROGRESS);
        OI_CASE(OI_OPP_CLIENT_PULL_PROGRESS);
        OI_CASE(OI_OPP_CLIENT_PUSH_COMPLETE);
        OI_CASE(OI_OPP_CLIENT_PULL_COMPLETE);
    }
    return "unknown event";
#else
    return "";
#endif      /* OI_DEBUG */
}

#ifdef OI_DEBUG
static OI_CHAR* ClientStateText(OPP_CLIENT_STATE state)
{
    switch (state) {
        OI_CASE(CLIENT_STATE_DISCONNECTED);
        OI_CASE(CLIENT_STATE_CONNECTING);
        OI_CASE(CLIENT_STATE_CONNECTED);
        OI_CASE(CLIENT_STATE_PUSH_PENDING);
        OI_CASE(CLIENT_STATE_PUSHING);
        OI_CASE(CLIENT_STATE_PULL_PENDING);
        OI_CASE(CLIENT_STATE_PULLING);
    }
    return "unknown state";
}
#else
#define ClientStateText(state)  ("")
#endif  /* OI_DEBUG */


#define SetState(client, newState) \
    do { \
        OI_DBGPRINT(("Client state %s ==> %s", ClientStateText((client)->state), ClientStateText(newState))); \
        (client)->state = (newState); \
    } while (0)




/*********************** FORWARD DEFINITIONS ***************************/

static void ClientPutCfm(OI_OBEXCLI_CONNECTION_HANDLE connectionId,
                         OI_OBEX_HEADER_LIST *rspHeaders,
                         OI_STATUS status);

static void ClientBulkPutCfm(OI_OBEXCLI_CONNECTION_HANDLE connectionId,
                             OI_UINT8 numBuffers,
                             OI_UINT8 *bulkDataBuffer[],
                             OI_UINT32 bufferLen[],
                             OI_STATUS status);

static void CloseObject(OPP_CLIENT *client,
                        OI_STATUS status);


static void PullWriteCfm(OI_OPP_HANDLE handle,
                         OI_STATUS status,
                         OI_OPP_CONNECTION oppContext);



/******************* END OF FORWARD DEFINITIONS ************************/


static void ClientConnectCfm(OI_OBEXCLI_CONNECTION_HANDLE connectionId,
                             OI_BOOL readOnly, /* not used for OPP */
                             OI_STATUS status)
{
    OPP_CLIENT *client = LookupClient(connectionId);
    OI_OPP_CLIENT_EVENT_DATA    evt;
    OI_OPP_CLIENT_EVENT_CB      cbSv;

    OI_DBGTRACE(("ClientConnectCfm %d, id = %d", status, connectionId));

    if (!client) {
        OI_SLOG_ERROR(OI_STATUS_INVALID_HANDLE, ("Invalid connection handle"));
        return;
    }
    OI_ASSERT(connectionId == client->id);

    cbSv = client->eventCB;
    OI_MemZero(&evt, sizeof(evt));
    evt.event = OI_OPP_CLIENT_CONNECTED;

    if (OI_SUCCESS(status)) {
        OI_L2CAP_CID cid;
        OI_DBGPRINT2(("OPP client connected"));
        SetState(client, CLIENT_STATE_CONNECTED);
        /*
         * Establish the maximum read size for object read operations.
         */
        client->maxReadSize = OI_OBEXCLI_OptimalBodyHeaderSize(connectionId);
        cbSv(connectionId, &evt, OI_OK);
        /*
         * Register connection with the AMP policy manager
         */
        status = OI_OBEXCLI_GetL2capCID(connectionId, &cid);
    } else {
        SetState(client, CLIENT_STATE_DISCONNECTED);
        cbSv(connectionId, &evt, status);
        OI_Free(client);
    }
}


static void ClientDisconnectInd(OI_OBEXCLI_CONNECTION_HANDLE connectionId)
{
    OPP_CLIENT *client = LookupClient(connectionId);

    OI_DBGTRACE(("ClientDisconnectInd, id = %d", connectionId));

    if (!client) {
        OI_SLOG_ERROR(OI_STATUS_INVALID_HANDLE, ("Invalid connection handle"));
        return;
    }
    /*
     * record the disconnect event for subsequent processes
     */
    client->disconnected = TRUE;

    /*
     * If close is already pending, just call it now
     */
    if (client->pendingClose) {
        OI_DBGPRINT2(("Disconnect : calling close immediately"));
        OI_Dispatch_CallFunc(client->pendingClose);
        /*
         * The client object has (hopefully) been freed by the pendingClose().
         * Disconnect has been handled completely.
         */
        return;
    }
    if (client->ofsCfmPending) {
        /*
         * We're waiting for an OFS confirm callback, just have to keep waiting
         */
    } else {
        /*
         * Status to be reported to OFS and app depends on whether we're idle or pushing/pulling
         */
        CloseObject(client, client->state > CLIENT_STATE_CONNECTED? OI_OBEX_NOT_CONNECTED : OI_OK);
        OI_Dispatch_CallFunc(client->pendingClose);
    }
}


static void ClientProgressInd(OI_OBEXCLI_CONNECTION_HANDLE connectionId,
                              OI_UINT8 cmd,
                              OI_UINT32 progress)
{
    OPP_CLIENT *client = LookupClient(connectionId);
    if (client) {
        OI_OPP_CLIENT_EVENT_DATA evt;

        OI_DBGPRINT(("%s progress %d", cmd == OI_OBEX_CMD_PUT ? "PUT" : "GET", progress));

        OI_MemZero(&evt, sizeof(OI_OPP_CLIENT_EVENT_DATA));
        evt.event = (cmd == OI_OBEX_CMD_PUT) ? OI_OPP_CLIENT_PUSH_PROGRESS : OI_OPP_CLIENT_PULL_PROGRESS;
        evt.data.pushProgress.bytesTransferred = progress;
        client->eventCB(client->id, &evt, OI_OK);
    }
}


/**
 * Connect to remote OBEX object push profile server.
 */

OI_STATUS OI_OPPClient_Connect(OI_BD_ADDR *addr,
                               OI_OBEX_LOWER_PROTOCOL *lowerProtocol,
                               OI_OPP_CLIENT_CONNECTION_HANDLE *connectionId,
                               OI_OPP_CLIENT_EVENT_CB eventCB,
                               const OI_OPP_OBJSYS_FUNCTIONS *objectFunctions)
{
    static const OI_OBEXCLI_CB_LIST callbacks = {
        ClientConnectCfm,
        ClientDisconnectInd,
        NULL, /* no authentication challenge */
        ClientProgressInd,
        ClientBulkPutCfm
    };
    OPP_CLIENT *client;
    OI_STATUS status;
    OI_OBEX_CONNECTION_OPTIONS connOpts;

    OI_DBGTRACE(("OI_OPPClient_Connect addr:%: %s connectionId:%x", addr, OI_OBEX_LowerProtocolTxt(lowerProtocol), connectionId));

    OI_ARGCHECK(eventCB && objectFunctions);
    OI_ARGCHECK(objectFunctions->OpenRead);
    OI_ARGCHECK(objectFunctions->Read);
    OI_ARGCHECK(objectFunctions->OpenWrite);
    OI_ARGCHECK(objectFunctions->Write);
    OI_ARGCHECK(objectFunctions->Close);

    client = OI_Calloc(sizeof(OPP_CLIENT));
    if (client == NULL) {
        return OI_STATUS_NO_RESOURCES;
    }
    client->eventCB = eventCB;
    client->objops = objectFunctions;
    /*
     * We allow SRM for OPP
     */
    connOpts.enableSRM = TRUE;
    status = OI_OBEXCLI_Connect(addr,
                                lowerProtocol,
                                &connOpts,
                                OI_OBEXCLI_AUTH_NONE,
                                NULL,
                                &callbacks,
                                connectionId,
                                &connectPolicy);

    if (OI_SUCCESS(status)) {
        client->id = *connectionId;
        /*
         * Associate OPP client connection with the OBEX connection
         */
        status = OI_OBEXCLI_SetConnectionContext(client->id, client);
        OI_ASSERT(OI_SUCCESS(status));
        SetState(client, CLIENT_STATE_CONNECTING);
    } else {
        OI_Free(client);
        OI_SLOG_ERROR(status, ("OI_OBEXCLI_Connect failed"));
    }
    return status;
}


OI_STATUS OI_OPPClient_Disconnect(OI_OPP_CLIENT_CONNECTION_HANDLE connectionId)
{
    OPP_CLIENT *client = LookupClient(connectionId);
    OI_STATUS status;

    OI_DBGTRACE(("OI_OPPClient_Disconnect, id %d", connectionId));

    if (IS_CLIENT_CONNECTING(client)) {
        OI_DBGPRINT(("Connecting"));
        return OI_OBEX_CONNECT_IN_PROGRESS;
    }
    if (!IS_CLIENT_CONNECTED(client)) {
        OI_DBGPRINT(("Not connected"));
        return OI_OBEX_NOT_CONNECTED;
    }
    status = OI_OBEXCLI_Disconnect(client->id, NULL);
    if (!OI_SUCCESS(status)) {
        OI_DBGPRINT(("OI_OBEXCLI_Disconnect failed (%d)", status));
    }
    return status;
}


static void CloseObjectCB(DISPATCH_ARG *darg)
{
    OPP_CLIENT *client = LookupClient(Dispatch_GetArg(darg, OI_OPP_CLIENT_CONNECTION_HANDLE));
    OI_STATUS status;
    OI_OPP_CLIENT_EVENT_DATA evt;
    OPP_CLIENT_STATE snapState;

    if (!client) {
        OI_LOG_ERROR(("CloseObject - client has gone"));
        return;
    }

    snapState = client->state;
    client->pendingClose = 0;
    client->nameOverridePending = FALSE;

    /*
     * Confirm is not pending
     */
    client->ofsCfmPending = FALSE;

    /*
     * Reset state before calling event callback in case the callback function
     * calls right back into OPP.
     */
    client->final = FALSE;
    if (client->disconnected) {
        SetState(client, CLIENT_STATE_DISCONNECTED);
    } else if (client->state > CLIENT_STATE_CONNECTED) {
        SetState(client, CLIENT_STATE_CONNECTED);
    }
    if (client->abortState != OPP_NO_ABORT) {
        status = OI_OBEX_CLIENT_ABORTED_COMMAND;
    } else {
        status = client->closeStatus;
    }

    switch (snapState) {
    case CLIENT_STATE_PUSHING:
        client->objops->Close(client->handle, status, (OI_OPP_CONNECTION) client->id);
        /* falling through */
    case CLIENT_STATE_PUSH_PENDING:
        evt.event = OI_OPP_CLIENT_PUSH_COMPLETE;
        evt.data.pushComplete.finalSize = client->curObj.objSize;
        client->eventCB(client->id, &evt, status);
        break;
    case CLIENT_STATE_PULLING:
        client->objops->Close(client->handle, status, (OI_OPP_CONNECTION) client->id);
        /* falling through */
    case CLIENT_STATE_PULL_PENDING:
        evt.event = OI_OPP_CLIENT_PULL_COMPLETE;
        evt.data.pullComplete.finalSize = client->curObj.objSize;
        client->eventCB(client->id, &evt, status);
        break;
    default:
        break;
    }
    /*
     * Free any saved header data.
     */
    OI_FreeIf(&client->curObj.name.str);
    client->curObj.name.len = 0;
    OI_FreeIf(&client->curObj.type.data);
    client->curObj.type.len = 0;
    OI_MemZero(&client->curObj, sizeof(OI_OPP_GENERIC_OBJECT));
    OI_MemZero(&evt, sizeof(OI_OPP_CLIENT_EVENT_DATA));
    /*
     * Confirm the abort if there was one
     */
    if (client->abortState == OPP_ABORT_CONFIRMED) {
        SetState(client, CLIENT_STATE_CONNECTED);
        client->cancelCfm(client->id);
        client->cancelCfm = NULL;
        client->abortState = OPP_NO_ABORT;
    }
    /*
     * Report the disconnection and clean up
     */
    if (client->disconnected) {
        OI_OPP_CLIENT_EVENT_DATA evt;
        OI_MemZero(&evt, sizeof(OI_OPP_CLIENT_EVENT_DATA));
        evt.event = OI_OPP_CLIENT_DISCONNECT;
        client->eventCB((OI_OPP_CONNECTION) client->id, &evt, OI_OK);
        OI_Free(client);
    }
}

/*
 * Put a callback function on the dispatcher to close the object.
 */
static void CloseObject(OPP_CLIENT *client,
                        OI_STATUS status)
{
    if (!client->pendingClose) {
        DISPATCH_ARG darg;
        OI_DBGPRINT2(("CloseObject"));
        client->closeStatus = status;
        Dispatch_SetArg(darg, client->id);
        (void) OI_Dispatch_RegisterFunc(CloseObjectCB, &darg, &client->pendingClose);
    }
}


static void PushReadCfm(OI_OPP_HANDLE handle,
                        OI_BYTE *data,
                        OI_UINT32 len,
                        OI_STATUS status,
                        OI_OPP_CONNECTION oppContext)
{
    OPP_CLIENT *client = LookupClient(oppContext);
    OI_STATUS obexStatus;

    if (!client || (client->state != CLIENT_STATE_PUSHING) || !client->ofsCfmPending) {
        OI_LOG_ERROR(("Push read confirm called at wrong time"));
        return;
    }

    OI_DBGPRINT2(("PushReadCfm %d bytes %d", len, status));

    /*
     * Confirm is no longer pending
     */
    client->ofsCfmPending = FALSE;
    /*
     * Check for disconnect or abort that occurred during wait for this confirm.
     */
    if (client->disconnected || (client->abortState != OPP_NO_ABORT)) {
        CloseObject(client, OI_OBEX_NOT_CONNECTED);
        return;
    }

    if (status == OI_STATUS_END_OF_FILE) {
        status = OI_OK;
        obexStatus = OI_OK;
    } else {
        obexStatus = OI_OBEX_CONTINUE;
    }
    if (!OI_SUCCESS(status)) {
        /*
         * Let obex know the put operation terminated with an error.  The object will be closed
         * when the ClientBulkPutCfm is called with failed status.
         */
        status = OI_OBEXCLI_BulkPut(client->id, 0, NULL, NULL, status);
    } else {
        status = OI_OBEXCLI_BulkPut(client->id, 1, &data, &len, obexStatus);
    }
    if (!OI_SUCCESS(status)) {
        OI_DBGPRINT(("PushReadCfm closing object %d", status));
        CloseObject(client, status);
    }
}


static void ClientBulkPutCfm(OI_OBEXCLI_CONNECTION_HANDLE connectionId,
                             OI_UINT8 numBuffers,
                             OI_UINT8 *bulkDataBuffer[],
                             OI_UINT32 bufferLen[],
                             OI_STATUS status)
{
    OPP_CLIENT *client = LookupClient(connectionId);

    OI_DBGPRINT2(("OPP client bulk put confirm %d", status));

    if (!client) {
        OI_SLOG_ERROR(OI_STATUS_INVALID_HANDLE, ("Invalid connection handle"));
        return;
    }

    if (status == OI_OBEX_CONTINUE) {
        if (client->final) {
            /*
             * We should not get a continue response after we have sent the
             * final packet.
             */
            OI_LOG_ERROR(("OBEX incorrect continue reponse"));
            status = OI_OBEX_INVALID_OPERATION;
        } else {
            /*
             * Set pending first because the confirm may be called from within the OFS function
             */
            client->ofsCfmPending = TRUE;

            status = client->objops->Read(client->handle, client->maxReadSize, PushReadCfm, (OI_OPP_CONNECTION) client->id);
            if (status == OI_STATUS_END_OF_FILE) {
                PushReadCfm(client->handle, NULL, 0, status, (OI_OPP_CONNECTION) client->id);
                status = OI_OK;
            }
            if (!OI_SUCCESS(status)) {
                /*
                 * If OFS returns error, there is no pending confirm callback
                 */
                client->ofsCfmPending = FALSE;
            }
        }
        if (!OI_SUCCESS(status)) {
            /*
             * Let obex know the put operation terminated with an error.
             */
            status = OI_OBEXCLI_BulkPut(client->id, 0, NULL, NULL, status);
            CloseObject(client, status);
        }
    } else {
        /*
         * We are either done or got an error. If we have not send an ABORT
         * we close the object, otherwise we wait for the ABORT confirmation.
         */
        if (client->abortState == OPP_NO_ABORT) {
            /*
             * We are either done or got an error, in either case we need to close
             * the object.
             */
            client->ofsCfmPending = FALSE;
            CloseObject(client, status);
        }
    }
}


static void ClientPutCfm(OI_OBEXCLI_CONNECTION_HANDLE connectionId,
                         OI_OBEX_HEADER_LIST *rspHeaders,
                         OI_STATUS status)
{
    OPP_CLIENT *client = LookupClient(connectionId);

    OI_DBGPRINT2(("OPP client put confirm %d", status));

    if (!client) {
        OI_SLOG_ERROR(OI_STATUS_INVALID_HANDLE, ("Invalid connection handle"));
        return;
    }
    if (status == OI_OBEX_CONTINUE) {
        if (client->final) {
            /*
             * We should not get a continue response after we have sent the
             * final packet.
             */
            OI_LOG_ERROR(("OBEX incorrect continue reponse"));
            status = OI_OBEX_INVALID_OPERATION;
        } else {
            /*
             * Set pending first because the confirm may be called from within the OFS function
             */
            client->ofsCfmPending = TRUE;

            status = client->objops->Read(client->handle, client->maxReadSize, PushReadCfm, (OI_OPP_CONNECTION) client->id);
            if (status == OI_STATUS_END_OF_FILE) {
                PushReadCfm(client->handle, NULL, 0, status, (OI_OPP_CONNECTION) client->id);
                status = OI_OK;
            }
            if (!OI_SUCCESS(status)) {
                /*
                 * If OFS returns error, there is no pending confirm callback
                 */
                client->ofsCfmPending = FALSE;
            }
        }

        if (!OI_SUCCESS(status)) {
            /*
             * Let obex know the put operation terminated with an error.
             */
            OI_OBEXCLI_Put(client->id, NULL, NULL, status);
            CloseObject(client, status);
        }
    } else {
        /*
         * We are either done or got an error. If we have not send an ABORT
         * we close the object, otherwise we wait for the ABORT confirmation.
         */
        if (client->abortState == OPP_NO_ABORT) {
            client->ofsCfmPending = FALSE;
            CloseObject(client, status);
        }
    }
}


static void PushOpenCfm(OI_OPP_HANDLE handle,
                        const OI_OBEX_UNICODE *name,
                        const OI_CHAR *type,
                        OI_UINT32 size,
                        OI_BYTE * data,
                        OI_UINT16 data_len,
                        OI_STATUS status,
                        OI_OPP_CONNECTION oppContext)
{
    OPP_CLIENT *client = LookupClient(oppContext);

    OI_DBGTRACE(("PushOpenCfm %d", status));

    if (!client || (client->state != CLIENT_STATE_PUSH_PENDING) || !client->ofsCfmPending) {
        OI_LOG_ERROR(("Push open confirm called at wrong time"));
        return;
    }

    if (OI_SUCCESS(status)) {
        OI_OPP_CLIENT_EVENT_DATA evt;

        client->handle = handle;
        client->curObj.objSize = size;
        OI_MemZero(&evt, sizeof(OI_OPP_CLIENT_EVENT_DATA));
        evt.event = OI_OPP_CLIENT_PUSH_STARTED;
        evt.data.pushStarted.fileName = &client->curObj.name;
        evt.data.pushStarted.totalBytes = size;
        client->eventCB(client->id, &evt, OI_OK);
    } else {
        DeferredPushOpenCfm(handle, name, type, size, data, data_len, status, oppContext);
    }
}

static void StripPath(OI_OBEX_UNICODE *name)
{
    OI_INT          i;
    OI_INT          copyLength;
    OI_OBEX_UNICODE tmpName;

    if ((name == NULL) || (name->str == NULL)) {
        OI_SLOG_ERROR(OI_STATUS_INVALID_PARAMETERS,("StripPath"));
        return;
    }

    tmpName = *name;
    i = (OI_INT) tmpName.len;

    while (--i >= 0) {
        switch (tmpName.str[i]) {
            /* There are 3 recognized File System Separators */
        case ':':  /* Apple */
        case '/':  /* Linux/Unix */
        case '\\': /* Microsoft */
            goto FoundDirSeparator;

        default:
            break;
        }
    }

    return;

FoundDirSeparator:
    i++;
    copyLength = (tmpName.len - i) * sizeof(OI_CHAR16);
    if (copyLength) {
        name->str = OI_Malloc(copyLength);
        if (name->str) {
            OI_MemCopy(name->str, &tmpName.str[i], copyLength);
            name->len = (tmpName.len - i);
            OI_DBGPRINT(("Strip Path for OBEX Name header: %?S --> %?S",
                         tmpName.str, tmpName.len,
                         name->str, name->len));
            OI_Free(tmpName.str);
        } else {
            /* Malloc Failed.  Restore previous name */
            OI_SLOG_ERROR(OI_STATUS_OUT_OF_MEMORY,("StripPath"));
            *name = tmpName;
        }
    }
}

static void PushOpenCfmWithoutOverride(OI_OPP_HANDLE handle,
                                       const OI_OBEX_UNICODE *name,
                                       const OI_CHAR *type,
                                       OI_UINT32 size,
                                       OI_BYTE * data,
                                       OI_UINT16 data_len,
                                       OI_STATUS status,
                                       OI_OPP_CONNECTION oppContext)
{
    OI_OPP_CLIENT_EVENT_DATA evt;
    OPP_CLIENT *client = LookupClient(oppContext);

    /*
     * Notify the app that the push is starting.  The transfer will
     * start immediately, since no NameOverride feedback is expected
     * from the application.
     */

    OI_DBGTRACE(("PushOpenCfmWithoutOverride %d", status));

    if (!client) {
        OI_SLOG_ERROR(OI_STATUS_INVALID_HANDLE, ("Invalid connection handle"));
        return;
    }

    OI_MemZero(&evt, sizeof(OI_OPP_CLIENT_EVENT_DATA));
    evt.event = OI_OPP_CLIENT_PUSH_STARTED;
    evt.data.pushStarted.fileName = &client->curObj.name;
    evt.data.pushStarted.totalBytes = size;
    client->eventCB(client->id, &evt, status);

    StripPath(&client->curObj.name);

    /*
     * Do the actual OpenCfm work.
     */
    DeferredPushOpenCfm(handle, &client->curObj.name, type, size, data, data_len,
        status, oppContext);
}

static void DeferredPushOpenCfm(OI_OPP_HANDLE handle,
                                const OI_OBEX_UNICODE *name,
                                const OI_CHAR *type,
                                OI_UINT32 size,
                                OI_BYTE * data,
                                OI_UINT16 data_len,
                                OI_STATUS status,
                                OI_OPP_CONNECTION oppContext)
{
    OPP_CLIENT *client = LookupClient(oppContext);
    OI_UINT16 len;
    OI_OBEX_HEADER hdrs[4];
    OI_OBEX_HEADER_LIST hdrList;

    OI_DBGTRACE(("DeferredPushOpenCfm %d", status));

    if (!client) {
        OI_SLOG_ERROR(OI_STATUS_INVALID_HANDLE, ("Invalid connection handle"));
        return;
    }
    /*
     * Confirm is no longer pending
     */
    client->ofsCfmPending = FALSE;
    /*
     * Check for disconnect or abort that occurred during wait for this confirm.
     */
    if (client->disconnected || (client->abortState != OPP_NO_ABORT)) {
        if (OI_SUCCESS(status)) {
            /*
             * OFS reported a successful open, set state to PUSHING to trigger close() callback.
             */
            SetState(client, CLIENT_STATE_PUSHING);
        }
        CloseObject(client, OI_OBEX_NOT_CONNECTED);
        return;
    }

    hdrList.list = hdrs;
    hdrList.count = 0;

    if (OI_SUCCESS(status) || status == OI_STATUS_END_OF_FILE) {
        SetState(client, CLIENT_STATE_PUSHING);
        client->handle = handle;
        /*
         * Send the initial object headers.
         */
        hdrs[hdrList.count].id = OI_OBEX_HDR_LENGTH;
        hdrs[hdrList.count].val.length = size;
        client->curObj.objSize = size;
        ++hdrList.count;
        hdrs[hdrList.count].id = OI_OBEX_HDR_NAME;
        hdrs[hdrList.count].val.name = client->curObj.name;
        ++hdrList.count;
        /*
         * Type header is optional
         */
        if (type != NULL) {
            len = OI_StrLen(type);
            if ( len > 0 ){
                hdrs[hdrList.count].id = OI_OBEX_HDR_TYPE;
                hdrs[hdrList.count].val.type.data = (OI_BYTE*) type;
                hdrs[hdrList.count].val.type.len = len + 1; /* include null termination */
                ++hdrList.count;
            }
        }
        /*
         * Body Header
         */
        if (data != NULL && data_len > 0) {
            hdrs[hdrList.count].id = OI_OBEX_HDR_BODY;
            hdrs[hdrList.count].val.body.data = (OI_BYTE*) data;
            hdrs[hdrList.count].val.body.len = data_len;
            ++hdrList.count;
        }
        status = OI_OBEXCLI_Put(client->id, &hdrList, ClientPutCfm,
            (status == OI_STATUS_END_OF_FILE) ? OI_OK : OI_OBEX_CONTINUE);
    }

    if (!OI_SUCCESS(status)) {
        CloseObject(client, status);
    }
}

OI_STATUS OI_OPPClient_Push(OI_OPP_CLIENT_CONNECTION_HANDLE connectionId,
                            const OI_OBEX_UNICODE *name,
                            const OI_CHAR *type)
{
    OPP_CLIENT *client = LookupClient(connectionId);
    OI_STATUS status = OI_OK;

    OI_DBGTRACE(("OI_OPPClient_Push, id %d", connectionId));

    if (!IS_CLIENT_CONNECTED(client)) {
        OI_DBGPRINT(("OPP client not connected"));
        return OI_OBEX_NOT_CONNECTED;
    }
    if (!IS_CLIENT_IDLE(client)) {
        OI_DBGPRINT(("Operation already in progress client state = %s", ClientStateText(client->state)));
        return OI_OBEX_OPERATION_IN_PROGRESS;
    }
    client->final = FALSE;
    SetState(client, CLIENT_STATE_PUSH_PENDING);

    /*
     * Set pending first because the confirm may be called from within the OFS function
     */
    client->ofsCfmPending = TRUE;

    /*
     * Duplicate file name into our mem space
     */
    OI_FreeIf(&client->curObj.name.str);
    client->curObj.name.len = 0;
    if (name && name->str) {
        client->curObj.name.str = OI_Malloc(name->len * sizeof(OI_CHAR16));
        if (client->curObj.name.str) {
            client->curObj.name.len = name->len;
            OI_MemCopy(client->curObj.name.str, name->str, name->len * sizeof(OI_CHAR16));
        }
    }

    /*
     * Duplicate type name into our mem space
     */
    OI_FreeIf(&client->curObj.type.data);
    client->curObj.type.len = 0;
    if (type) {
        // Preserve the terminating null when allocating/copying
        OI_UINT typeLen = OI_StrLen(type) + 1;
        client->curObj.type.data = OI_Malloc(typeLen);
        if (client->curObj.type.data) {
            client->curObj.type.len = typeLen;
            OI_MemCopy(client->curObj.type.data, type, typeLen);
        }
    }

    /*
     * If Name Override Enabaled, wait for App to supply name, otherwise use
     * same filename for remote naming.
     */
    if (client->nameOverrideEnabled) {
        client->nameOverridePending = TRUE;
        status = client->objops->OpenRead(name, type, client->maxReadSize,
            PushOpenCfm, (OI_OPP_CONNECTION) client->id);
    } else {
        status = client->objops->OpenRead(name, type, client->maxReadSize,
            PushOpenCfmWithoutOverride, (OI_OPP_CONNECTION) client->id);
    }

    if (!OI_SUCCESS(status)) {
        OI_DBGTRACE(("OI_OPPClient_Push failed: %d", status));
        /*
         * If the OFS OpenRead() call failed, there is no pending confirm.
         * Set
         */
        client->ofsCfmPending = FALSE;
        SetState(client, CLIENT_STATE_CONNECTED);
    }

    return status;
}


static void PullAbortCfm(OI_OBEXCLI_CONNECTION_HANDLE connectionId)
{
    OPP_CLIENT *client = LookupClient(connectionId);
    OI_DBGTRACE(("PullAbortCfm, id %d", connectionId));

    if (!client) {
        OI_SLOG_ERROR(OI_STATUS_INVALID_HANDLE, ("Invalid connection handle"));
    } else {
        CloseObject(client, OI_OBEX_OFS_ERROR);
    }
}


static void GetRcvData(OI_OBEXCLI_CONNECTION_HANDLE connectionId,
                       OI_OBEX_HEADER_LIST *rspHeaders,
                       OI_STATUS rcvStatus)
{
    OPP_CLIENT *client = LookupClient(connectionId);
    OI_STATUS status = rcvStatus;

    if (!client) {
        OI_SLOG_ERROR(OI_STATUS_INVALID_HANDLE, ("Invalid connection handle"));
        return;
    }
    if (OI_SUCCESS(rcvStatus) || (rcvStatus == OI_OBEX_CONTINUE)) {
        client->final = (rcvStatus == OI_OK);
        status = OPPCommon_ParseHeaders(&client->curObj, rspHeaders);
        if (OI_SUCCESS(status)) {
            client->ofsCfmPending = TRUE;
            status = client->objops->Write(client->handle,
                                           client->curObj.objData.data,
                                           client->curObj.objData.len,
                                           PullWriteCfm,
                                           (OI_OPP_CONNECTION) client->id);

            client->curObj.objData.data = NULL;
            client->curObj.objData.len = 0;
            if (!OI_SUCCESS(status)) {
                client->ofsCfmPending = FALSE;
            }
        }
    }
    if (!OI_SUCCESS(status)) {
        CloseObject(client, status);
    }
}


static void PullWriteCfm(OI_OPP_HANDLE handle,
                         OI_STATUS status,
                         OI_OPP_CONNECTION oppContext)
{
    OPP_CLIENT *client = LookupClient(oppContext);

    if (!client || (client->state != CLIENT_STATE_PULLING) || !client->ofsCfmPending) {
        OI_LOG_ERROR(("Pull write confirm called at wrong time"));
        return;
    }

    OI_DBGPRINT2(("PullWriteCfm %d\n", status));

    /*
     * Confirm is no longer pending
     */
    client->ofsCfmPending = FALSE;
    /*
     * Check for disconnect or abort that occurred during wait for this confirm.
     */
    if (client->disconnected || (client->abortState != OPP_NO_ABORT)) {
        CloseObject(client, OI_OBEX_NOT_CONNECTED);
        return;
    }
    /*
     * If we had not received the final packet call for more.
     */
    if (!client->final) {
        if (!OI_SUCCESS(status)) {
            /*
             * Let OBEX know we are terminating the GET because of an error.
             */
            status = OI_OBEXCLI_Abort(client->id, PullAbortCfm);
        } else {
            status = OI_OBEXCLI_Get(client->id, NULL, GetRcvData, TRUE);
        }
    }
    /*
     * On error or when we are done pulling, close the object.
     */
    if (client->final || !OI_SUCCESS(status)) {
        CloseObject(client, status);
    }
}


/*
 * Return first body header.
 */
static void FirstBodyHeader(DISPATCH_ARG *darg)
{
    OPP_CLIENT *client = LookupClient(Dispatch_GetArg(darg, OI_OPP_CLIENT_CONNECTION_HANDLE));
    OI_STATUS status;

    if (!client) {
        OI_LOG_ERROR(("FirstBodyHeader - client has gone"));
        return;
    }
    /*
     * Set pending first because the confirm may be called from within the OFS function
     */
    client->ofsCfmPending = TRUE;

    status = client->objops->Write(client->handle,
                                   client->curObj.objData.data,
                                   client->curObj.objData.len,
                                   PullWriteCfm,
                                   (OI_OPP_CONNECTION) client->id);
    if (!OI_SUCCESS(status)) {
        /*
         * If OFS returns error, there is no pending confirm callback
         */
        client->ofsCfmPending = FALSE;
    }

    client->curObj.objData.data = NULL;
    client->curObj.objData.len = 0;
    if (!OI_SUCCESS(status)) {
        CloseObject(client, status);
    }
}


static void PullOpenCfm(OI_OPP_HANDLE handle,
                        OI_STATUS status,
                        OI_OPP_CONNECTION oppContext)
{
    OPP_CLIENT *client = LookupClient(oppContext);

    if (!client || (client->state != CLIENT_STATE_PULL_PENDING) || !client->ofsCfmPending) {
        OI_LOG_ERROR(("Pull open confirm called at wrong time"));
        return;
    }

    OI_DBGPRINT2(("PullOpenCfm %d",status));

    /*
     * Confirm is no longer pending
     */
    client->ofsCfmPending = FALSE;
    /*
     * Check for disconnect or abort that occurred during wait for this confirm.
     */
    if (client->disconnected || (client->abortState != OPP_NO_ABORT)) {
        if (OI_SUCCESS(status)) {
            /*
             * OFS reported a successful open, set state to PULLING to trigger close() callback.
             */
            SetState(client, CLIENT_STATE_PULLING);
        }
        CloseObject(client, OI_OBEX_NOT_CONNECTED);
        return;
    }

    if (!OI_SUCCESS(status)) {
        /*
         * If the operation is not complete, let OBEX know we are terminating
         * the GET because of an error.
         */
        if (!client->final) {
            status = OI_OBEXCLI_Abort(client->id, PullAbortCfm);
            if (!OI_SUCCESS(status)) {
                /*
                 * The abort failed, close the object terminating the current operation
                 */
                CloseObject(client, status);
            }
        } else {
            /*
             * On error when we are done pulling, close the object.
             */
            CloseObject(client, status);
        }
    } else {
        DISPATCH_ARG darg;
        SetState(client, CLIENT_STATE_PULLING);
        client->handle = handle;
        Dispatch_SetArg(darg, client->id);
        (void) OI_Dispatch_RegisterFunc(FirstBodyHeader, &darg, NULL);
    }
}

/*
 * Confirm or Override the default name
 */
static OI_STATUS NameOverride(OPP_CLIENT            *client,
                              const OI_OBEX_UNICODE *name)
{
    OI_OBEX_UNICODE tmpName = {0,NULL};
    OI_STATUS       status = OI_OK;

    switch (client->state) {
    case CLIENT_STATE_PULL_PENDING:
        status = client->objops->OpenWrite(name ? name : &client->curObj.name,
                                           (OI_CHAR*) client->curObj.type.data,
                                           client->curObj.objSize,
                                           PullOpenCfm,
                                           (OI_OPP_CONNECTION) client->id);
        if (!OI_SUCCESS(status)) {
            /*
             * If OFS returns error, there is no pending confirm callback
             */
            client->ofsCfmPending = FALSE;
        }
        break;

    case CLIENT_STATE_PUSH_PENDING:
        /*
         * On PUSH, if name is supplied, Save name for remote naming
         * purposes only. Open original file specified for reading.
         */
        if (name && name->str && name->str[0]) {
            /*
             * Preserve Original File Name
             */
            tmpName.str = client->curObj.name.str;
            tmpName.len = client->curObj.name.len;
            client->curObj.name.str = OI_Malloc(name->len * sizeof(OI_CHAR16));
            if (client->curObj.name.str) {
                /*
                 * Copy New File Name
                 */
                client->curObj.name.len = name->len;
                OI_MemCopy(client->curObj.name.str, name->str, name->len * sizeof(OI_CHAR16));
                OI_FreeIf(&tmpName.str);
            } else {
                /*
                 * Malloc Failure: Use original name
                 */
                name = NULL;
                client->curObj.name.str = tmpName.str;
                tmpName.str = NULL;
            }
        } else {
            StripPath(&client->curObj.name);
        }

        DeferredPushOpenCfm(client->handle,
                            &client->curObj.name,
                            (OI_CHAR *) client->curObj.type.data,
                            client->curObj.objSize,
                            NULL,
                            0,
                            OI_OK,
                            client->id);

        break;

    default:
        status = OI_STATUS_INVALID_STATE;
        break;
    }
    return status;
}

static void GetRcv(OI_OBEXCLI_CONNECTION_HANDLE connectionId,
                   OI_OBEX_HEADER_LIST *rspHeaders,
                   OI_STATUS rcvStatus)
{
    OPP_CLIENT *client = LookupClient(connectionId);
    OI_STATUS status = rcvStatus;

    OI_DBGTRACE(("GetRcv %d", rcvStatus));

    if (!client) {
        OI_SLOG_ERROR(OI_STATUS_INVALID_HANDLE, ("Invalid connection handle"));
        return;
    }

    if (OI_SUCCESS(rcvStatus) || (rcvStatus == OI_OBEX_CONTINUE)) {
        client->final = (rcvStatus == OI_OK);
        status = OPPCommon_ParseHeaders(&client->curObj, rspHeaders);
        /*
         * Wait until we have seen a body header before attempting to open the
         * object.
         */
        if (client->final || (client->curObj.objData.data != NULL)) {

            /* The IrMC OBEX spec and Bluetooth OPP spec disagree on whether
             * the NAME header is required or not.  In the name of greatest
             * compatibility, a NULL file name will be passed to the
             * application for the application to deal with the issue.
             */
            if (client->curObj.name.str == NULL || client->curObj.name.len == 0 || client->curObj.name.str[0] == '\0') {
                OI_LOG_ERROR(("Remote device failed to provide mandatory NAME header.\n"));
            }

            /*
             * Set pending first because the confirm may be called from within the OFS function
             */
            client->ofsCfmPending = TRUE;

            if (client->nameOverrideEnabled) {
                client->nameOverridePending = TRUE;
            } else {
                /*
                 * Use existing curObj name
                 */
                OI_DBGPRINT2(("internal NameOverride"));
                status = NameOverride(client, NULL);
            }

            /*
             * PULL_STARTED event must be sent after setting up for Name
             * Overriding, because it may be invoked from callback
             */
            if (client->pullStarted == FALSE) {
                OI_OPP_CLIENT_EVENT_DATA evt;

                client->pullStarted = TRUE;
                OI_MemZero(&evt, sizeof(OI_OPP_CLIENT_EVENT_DATA));
                evt.event = OI_OPP_CLIENT_PULL_STARTED;
                evt.data.pullStarted.fileName = &client->curObj.name;
                evt.data.pullStarted.totalBytes = client->curObj.objSize;
                /*
                 * Alert App that Transfer Starting. App must call OI_OPPClient_NameOverride
                 * if Name Overriding has been Enabled.
                 */
                client->eventCB(client->id, &evt, OI_OK);
            }
        } else {
            status = OI_OBEXCLI_Get(client->id, NULL, GetRcv, TRUE);
        }
    }
    if (!OI_SUCCESS(status)) {
        CloseObject(client, status);
    }
}

/*
 * Confirm or Override the default name
 */
OI_STATUS OI_OPPClient_NameOverride(OI_OPP_CLIENT_CONNECTION_HANDLE connectionId,
                                    const OI_OBEX_UNICODE          *name)
{
    OPP_CLIENT *client = LookupClient(connectionId);
    OI_STATUS status = OI_STATUS_INVALID_STATE;

    OI_DBGTRACE(("OI_OPPClient_NameOverride"));
    if (!IS_CLIENT_CONNECTED(client)) {
        return OI_OBEX_NOT_CONNECTED;
    }

    if (client->nameOverridePending) {
        client->nameOverridePending = FALSE;
        status = NameOverride(client, name);
    }
    return status;
}

/*
 * Pull the default object.
 */

OI_STATUS OI_OPPClient_Pull(OI_OPP_CLIENT_CONNECTION_HANDLE connectionId)
{
    OPP_CLIENT *client = LookupClient(connectionId);
    static const OI_CHAR typeName[] = OI_OBEX_VCARD_TYPE;
    OI_STATUS status;
    OI_OBEX_HEADER hdr;
    OI_OBEX_HEADER_LIST hdrList;

    OI_DBGTRACE(("OI_OPPClient_Pull, id %d", connectionId));

    if (!IS_CLIENT_CONNECTED(client)) {
        OI_DBGPRINT(("OPP client not connected"));
        return OI_OBEX_NOT_CONNECTED;
    }
    if (!IS_CLIENT_IDLE(client)) {
        OI_DBGPRINT(("Operation already in progress client state = %s", ClientStateText(client->state)));
        return OI_OBEX_OPERATION_IN_PROGRESS;
    }
    client->final = FALSE;
    client->pullStarted = FALSE;
    SetState(client, CLIENT_STATE_PULL_PENDING);

    /*
     * There is only one header required for pulling the default object. A type
     * header initialized to the vcard type string.
     */
    hdr.id = OI_OBEX_HDR_TYPE;
    hdr.val.type.data = (OI_BYTE*) typeName;
    hdr.val.type.len = OI_ARRAYSIZE(typeName);

    hdrList.list = &hdr;
    hdrList.count = 1;
    status = OI_OBEXCLI_Get(client->id, &hdrList, GetRcv, TRUE);

    if (!OI_SUCCESS(status)) {
        SetState(client, CLIENT_STATE_CONNECTED);
    }
    return status;
}



static void AbortCfm(OI_OBEXCLI_CONNECTION_HANDLE connectionId)
{
    OPP_CLIENT *client = LookupClient(connectionId);

    OI_DBGPRINT(("AbortCfm connectionId:%#x", connectionId));
    if (!client) {
        OI_SLOG_ERROR(OI_STATUS_INVALID_HANDLE, ("Invalid connection handle"));
        return;
    }
    if (client->abortState != OPP_ABORT_SENT) {
        OI_DBGPRINT(("Did not expect AbortCfm callback"));
        return;
    }
    client->abortState = OPP_ABORT_CONFIRMED;
    CloseObject(client, OI_OBEX_CLIENT_ABORTED_COMMAND);
    OI_Dispatch_CallFunc(client->pendingClose);
}


OI_STATUS OI_OPPClient_Cancel(OI_OPP_CLIENT_CONNECTION_HANDLE connectionId,
                              OI_OPP_CLIENT_CANCEL_CFM cancelCfm)
{
    OPP_CLIENT *client = LookupClient(connectionId);
    OI_STATUS status;

    OI_DBGTRACE(("OI_OPPClient_Cancel connectionId:%#x", connectionId));

    if (!IS_CLIENT_CONNECTED(client)) {
        return OI_OBEX_NOT_CONNECTED;
    }
    if (IS_CLIENT_IDLE(client)) {
        return OI_OBEX_INVALID_OPERATION;
    }

    OI_DBGPRINT(("OI_OPPClient_Cancel, state %s", ClientStateText(client->state)));

    status = OI_OBEXCLI_Abort(client->id, AbortCfm);
    if (OI_SUCCESS(status)) {
        client->abortState = OPP_ABORT_SENT;
        client->cancelCfm = cancelCfm;
    } else {
        /*
         * Abort attempt resulted in some error.
         */
        OI_SLOG_ERROR(status, ("OI_OBEXCLI_Abort failed"));
        CloseObject(client, OI_OBEX_CLIENT_ABORTED_COMMAND);
    }
    return status;
}

OI_STATUS OI_OPPClient_EnableNameOverride(OI_OPP_CLIENT_CONNECTION_HANDLE connectionId,
                                          OI_BOOL                         nameOverrideEnabled)
{
    OPP_CLIENT *client = LookupClient(connectionId);

    OI_DBGTRACE(("OI_OPPClient_EnableNameOverride: %d", nameOverrideEnabled));
    if (IS_CLIENT_CONNECTED(client)) {
        client->nameOverrideEnabled = (OI_UINT8)(nameOverrideEnabled ? TRUE : FALSE);
        return OI_OK;
    } else {
        return OI_OBEX_NOT_CONNECTED;
    }
}
