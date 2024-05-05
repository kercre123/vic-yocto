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

Object Push Profile server
*/

#define __OI_MODULE__ OI_MODULE_OPP_SRV
#include "oi_opp_server.h"
#include "oi_opp_common.h"
#include "oi_obexspec.h"
#include "oi_obexsrv.h"
#include "oi_argcheck.h"
#include "oi_utils.h"
#include "oi_memmgr.h"
#include "oi_dispatch.h"
#include "oi_debug.h"
#include "oi_assert.h"
#include "oi_bt_assigned_nos.h"
#include "oi_sdpdb.h"
#include "oi_sdp_utils.h"
#include "oi_std_utils.h"
#include "oi_opp_test.h"

#include "oi_init_flags.h"
#include "oi_config_table.h"
#include "oi_bt_profile_config.h"

/*
 * Service class ID list
 */

static const OI_DATAELEM ServiceClassIDList[] = {
    OI_ELEMENT_UUID32(OI_UUID_OBEXObjectPush)
};

/*
 * Profile descriptor list
 */

static const OI_DATAELEM Profile0[] = {
    OI_ELEMENT_UUID32(OI_UUID_OBEXObjectPush),
#ifdef OI_OBEX_OVER_L2CAP
    OI_ELEMENT_UINT16(0x0102) /* version 1.2 */
#else
    OI_ELEMENT_UINT16(0x0100) /* version 1.0 */
#endif
};

static const OI_DATAELEM ProfileDescriptorList[] = {
    OI_ELEMENT_SEQ(Profile0)
};

/*
 * SDP Attribute lists
 */

static const OI_SDPDB_ATTRIBUTE ServiceDescription[] = {
    { OI_ATTRID_ServiceClassIDList,             OI_ELEMENT_SEQ(ServiceClassIDList) },
    { OI_ATTRID_BluetoothProfileDescriptorList, OI_ELEMENT_SEQ(ProfileDescriptorList) }
};



/*
 * Note that the order of these states is important.
 */
typedef enum {
    OPP_SERVER_DISCONNECTED = 0, /* doing nothing right now */
    OPP_SERVER_CONNECTING   = 1, /* server has indicated a connection */
    OPP_SERVER_CONNECTED    = 2, /* server is connected */
    OPP_PULL_PENDING        = 3, /* pull has been requested */
    OPP_CLIENT_PULLING      = 4, /* pull is in progress  */
    OPP_PUSH_PENDING        = 5, /* push has been requested */
    OPP_CLIENT_PUSHING      = 6  /* push is in progress  */
} OPP_SERVER_STATE;


/*
 * Struct for an OBEX OPP server
 */
typedef struct {

    OI_OBEX_SERVER_HANDLE serverHandle;

    const OI_OPP_OBJSYS_FUNCTIONS *objops;
    const OI_OPP_SERVER_CALLBACKS *callbacks;

    OI_UINT32 srecHandle;

    OI_OPP_SERVER_OBJECT_FORMATS supportedFormats;  /**< Supported formats as a bit vector.*/

    OI_UINT8 numAttributes;
    OI_SDPDB_ATTRIBUTE sdpAttributes[3]; /**< Supported format and Protocol Descriptor list */

} OPP_SERVER;


/*
 * Struct for an OPP connection.
 */
typedef struct {

    OPP_SERVER_STATE state;
    OPP_SERVER *server;      /* Pointer to server for this connection */

    OI_OPP_HANDLE ofsHandle; /* Object file store handle for current operation */

    OI_UINT32 maxReadSize;

    OI_OPP_GENERIC_OBJECT rcvObj;

    OI_OBEXSRV_CONNECTION_HANDLE obexHandle; /* Handle for underlying OBEX connection */

    DISPATCH_CB_HANDLE closeCB;

    OI_BOOL   final;
    OI_BOOL   allowPush;
    OI_BOOL   allowPull;
    OI_BOOL   ofsAcceptPending;       /**< Waiting for application to accept/reject incoming PUT */
    OI_BOOL   ofsCfmPending;          /**< Waiting for Object File System to call our cfm function */
    OI_BOOL   deferredPPError;        /**< Push/Pull error occurred while awaiting OFS cfm. */
    OI_STATUS deferredErrorStatus;    /**< The specific error whose handling has been deferred */

} OPPSRV_CONNECTION;



#define LookupConnection(handle)   ((OPPSRV_CONNECTION*)OI_OBEXSRV_GetConnectionContext(handle))
#define LookupServer(handle)       ((OPP_SERVER*)OI_OBEXSRV_GetServerContext(handle))



/**
 * Connection policy.
 */
static const OI_CONNECT_POLICY  connectPolicy =
{
    OI_ELEMENT_UUID32(OI_UUID_OBEXObjectPush), /* OI_DATAELEM         serviceUuid         */
    FALSE,                                     /* OI_BOOL             mustBeMaster        */
    NULL,                                      /* OI_L2CAP_FLOWSPEC   *flowspec           */
    0                                          /* OI_UINT8            powerSavingDisables */
};

#define    OI_CASE(a) case a: return(#a) ;

OI_CHAR  *OI_OPPServer_eventText(OI_OPP_SERVER_EVENT event)
{
#ifdef OI_DEBUG
    switch (event) {
        OI_CASE(OI_OPP_SERVER_EVENT_PUSH);
        OI_CASE(OI_OPP_SERVER_EVENT_PULL);
        OI_CASE(OI_OPP_SERVER_EVENT_PUSH_PROGRESS);
        OI_CASE(OI_OPP_SERVER_EVENT_PULL_PROGRESS);
        OI_CASE(OI_OPP_SERVER_EVENT_PUSH_COMPLETE);
        OI_CASE(OI_OPP_SERVER_EVENT_PULL_COMPLETE);
    }
    return "unknown event";
#else
    return "";
#endif      /* OI_DEBUG */
}


#ifdef OI_DEBUG
static OI_CHAR* ServerStateText(OPP_SERVER_STATE state)
{
    switch (state) {
        OI_CASE(OPP_SERVER_DISCONNECTED);
        OI_CASE(OPP_SERVER_CONNECTING);
        OI_CASE(OPP_SERVER_CONNECTED);
        OI_CASE(OPP_PULL_PENDING);
        OI_CASE(OPP_CLIENT_PULLING);
        OI_CASE(OPP_PUSH_PENDING);
        OI_CASE(OPP_CLIENT_PUSHING);
    }
    return("unknown state");

}

#else

#define ServerStateText(state)  ("")

#endif  /* OI_DEBUG */


#define SetState(cn, newState) \
    do { \
        OI_DBGTRACE(("OPP Server state %s ==> %s", ServerStateText((cn)->state), ServerStateText(newState))); \
        (cn)->state = (newState); \
    } while (0)


/*
 * Tries to figure out the type of the object being pushed from the suffix on the
 * name.
 */

static OI_CHAR* ObjTypeFromSuffix(OI_OBEX_UNICODE *name)
{
    OI_CHAR suffix[5];
    OI_UINT len;
    OI_INT i;

    /*
     * The spec says that a OBEX name should be a null terminated but we
     * are a little more paranoid.
     */
    for (len = name->len; len > 0; --len) {
        if (name->str[len - 1] != 0) {
            break;
        }
    }
    /*
     * A suffixed name must be at least 5 characters long (e.g. x.vcf).
     */
    if (len >= 5) {
        /*
         * Convert suffix (including '.') to ascii.
         */
        for (i = 0; i < 4; ++i) {
            suffix[i] = (OI_CHAR) (name->str[len - 4 + i] & 0xFF);
        }
        suffix[i] = 0;
        OI_DBGPRINT2(("ObjecTypeFromSuffix %s", suffix));
        if (OI_Strcmp(suffix, OI_OBEX_VCARD_SUFFIX) == 0) {
            return OI_OBEX_VCARD_TYPE;
        }
        if (OI_Strcmp(suffix, OI_OBEX_VCALENDAR_SUFFIX) == 0) {
            return OI_OBEX_VCALENDAR_TYPE;
        }
        if (OI_Strcmp(suffix, OI_OBEX_VNOTE_SUFFIX) == 0) {
            return OI_OBEX_VNOTE_TYPE;
        }
        if (OI_Strcmp(suffix, OI_OBEX_VMESSAGESUFFIX) == 0) {
            return OI_OBEX_VMESSAGE_TYPE;
        }
        if (OI_Strcmp(suffix, OI_OBEX_ICALENDAR_SUFFIX) == 0) {
            return OI_OBEX_ICALENDAR_TYPE;
        }
    }
    return "";
}


/*
 * Brute force check for supported format type.
 */
static OI_BOOL IsSupportedType(OI_CHAR *type,
                               OI_UINT typeLen,
                               OI_OPP_SERVER_OBJECT_FORMATS formats)
{
    if (formats == OI_OPP_SERVER_OBJ_FORMAT_ANY) {
        return TRUE;
    }
    if (OI_StrncmpInsensitive(type, OI_OBEX_VCARD_TYPE, (OI_UINT16)typeLen) == 0) {
        return formats & (OI_OPP_SERVER_OBJ_FORMAT_VCARD_2_1 | OI_OPP_SERVER_OBJ_FORMAT_VCARD_3_0);
    }
    if (OI_StrncmpInsensitive(type, OI_OBEX_VCALENDAR_TYPE, (OI_UINT16)typeLen) == 0) {
        return (formats & OI_OPP_SERVER_OBJ_FORMAT_VCAL_1_0);
    }
    if (OI_StrncmpInsensitive(type, OI_OBEX_ICALENDAR_TYPE, (OI_UINT16)typeLen) == 0) {
        return (formats & OI_OPP_SERVER_OBJ_FORMAT_ICAL_2_0);
    }
    if (OI_StrncmpInsensitive(type, OI_OBEX_VNOTE_TYPE, (OI_UINT16)typeLen) == 0) {
        return (formats & OI_OPP_SERVER_OBJ_FORMAT_VNOTE);
    }
    if (OI_StrncmpInsensitive(type, OI_OBEX_VMESSAGE_TYPE, (OI_UINT16)typeLen) == 0) {
        return (formats & OI_OPP_SERVER_OBJ_FORMAT_VMESSAGE);
    }
    return FALSE;
}


static void ObjectClose(OPPSRV_CONNECTION *connection,
                        OI_BOOL disconnected,
                        OI_STATUS status)
{
    OI_OPP_SERVER_EVENT_DATA eventData;
    OPP_SERVER_STATE snapState;
    OI_OPP_SERVER_EVENT_IND eventInd;

    snapState = connection->state;

    switch (snapState) {
        case OPP_PULL_PENDING:
        case OPP_CLIENT_PULLING:
            eventData.event = OI_OPP_SERVER_EVENT_PULL_COMPLETE;
            eventData.data.pullComplete.finalSize = connection->rcvObj.objSize;
            eventData.data.pullComplete.status = status;
            eventInd = connection->server->callbacks->eventInd;
            break;
        case OPP_PUSH_PENDING:
        case OPP_CLIENT_PUSHING:
            eventData.event = OI_OPP_SERVER_EVENT_PUSH_COMPLETE;
            eventData.data.pushComplete.finalSize = connection->rcvObj.objSize;
            eventData.data.pushComplete.status = status;
            eventInd = connection->server->callbacks->eventInd;
            break;
        default:
            eventInd = NULL;
            break;
    }
    /*
     * Free any saved header data.
     */
    OI_FreeIf(&connection->rcvObj.name.str);
    OI_FreeIf(&connection->rcvObj.type.data);
    OI_MemZero(&connection->rcvObj, sizeof(OI_OPP_GENERIC_OBJECT));
    if (disconnected) {
        /*
         * Break association between the handle and the OPP connection
         */
        OI_OBEXSRV_SetConnectionContext(connection->obexHandle, NULL);
    } else {
        /*
         * Make connection available for other operations.
         */
        SetState(connection, OPP_SERVER_CONNECTED);
    }
    /*
     * If there is an open object, close the object and clear OFS state.
     */
    if (connection->ofsHandle) {
        OI_DBGTRACE(("objops->close %d", status));
        connection->server->objops->Close(connection->ofsHandle, status, connection->obexHandle);
        connection->ofsHandle = NULL;
    }
    connection->ofsAcceptPending = FALSE;
    connection->ofsCfmPending = FALSE;

    /*
     * If applicable indicate the PULL/PUSH completion event now.
     */
    if (eventInd != NULL) {
        eventInd(connection->obexHandle, &eventData);
    }
    /*
     * If we disconnected call the disconnect indication now
     */
    if (disconnected) {
        if (connection->server->callbacks->disconnectInd != NULL) {
            connection->server->callbacks->disconnectInd(connection->obexHandle);
        }
        OI_Free(connection);
    }
}


static void DeferredObjectClose(DISPATCH_ARG *darg)
{
    OPPSRV_CONNECTION *connection = LookupConnection(Dispatch_GetArg(darg, OI_OBEXSRV_CONNECTION_HANDLE));
    if (connection == NULL) {
        OI_LOG_ERROR(("DeferredObjectClose - connection has gone"));
    } else {
        connection->closeCB = 0;
        ObjectClose(connection, FALSE, connection->deferredErrorStatus);
    }

}


static void SetupDeferredObjectClose(OPPSRV_CONNECTION *connection,
                                     OI_STATUS status)
{
    DISPATCH_ARG darg;

    /*
     * There can only be one object open at any one time, so there can only be one object close
     * pending at any one time.
     */
    OI_ASSERT(!connection->closeCB);

    OI_DBGPRINTSTR(("SetupDeferredObjectClose"));
    /*
     * OBEX returns a CLEANUP status when the operation is complete
     */
    if (status == OI_OBEX_CLEANUP) {
        status = OI_OK;
    }
    /*
     * We cannot call back into the application on this thread so we must complete the object close on
     * the dispatch thread.
     */

    connection->deferredErrorStatus = status;
    Dispatch_SetArg(darg, connection->obexHandle);
    OI_Dispatch_RegisterFunc(DeferredObjectClose, &darg, &connection->closeCB);
}

/*
 * Invoke DeferredObjectClose() if it's queued, and return error code if caller
 * should abort.
 */
static OI_STATUS CheckDeferredObjectClose(OPPSRV_CONNECTION *connection)
{
    if (!connection) {
        return OI_STATUS_INVALID_HANDLE;
    }
    if (connection->closeCB) {
        OI_DBGPRINTSTR(("Deferred close is queued - calling close immediately"));
        OI_Dispatch_CallFunc(connection->closeCB);
    }
    return OI_OK;
}


static void PushWriteCfm(OI_OPP_HANDLE handle,
                         OI_STATUS status,
                         OI_OPP_CONNECTION oppContext)
{
    OPPSRV_CONNECTION *connection = LookupConnection(oppContext);
    OI_STATUS retVal;

    OI_DBGTRACE(("PushWriteCfm %d", status));

    /*
     * First check we are expecting this call.
     */
    if (!connection) {
        OI_SLOG_ERROR(OI_STATUS_INVALID_HANDLE, ("Invalid connection handle"));
        return;
    }
    if ((connection->state != OPP_CLIENT_PUSHING) || !connection->ofsCfmPending) {
        OI_LOG_ERROR(("Push write confirm called at wrong time"));
        return;
    }
    /*
     * Confirm is no longer pending
     */
    connection->ofsCfmPending = FALSE;
    if (OI_SUCCESS(status)) {
        /*
         * Report continue or done to client.
         */
        status = OI_OBEXSRV_PutResponse(connection->obexHandle, NULL, connection->final ? OI_OK : OI_OBEX_CONTINUE);
        if (!OI_SUCCESS(status)) {
            OI_LOG_ERROR(("OI_OBEXSRV_PutResponse failed %d", status));
        }
    } else {
        /*
         * Report error to client.
         */
        retVal = OI_OBEXSRV_PutResponse(connection->obexHandle, NULL, status);
        if (!OI_SUCCESS(retVal)) {
            OI_LOG_ERROR(("OI_OBEXSRV_PutResponse failed %d", retVal));
        }
    }
    if (!OI_SUCCESS(status)) {
        SetupDeferredObjectClose(connection, status);
    }
}


/*
 * Return first body header.
 */
static void FirstBodyHeader(DISPATCH_ARG *darg)
{
    OPPSRV_CONNECTION *connection = LookupConnection(Dispatch_GetArg(darg, OI_OPP_SERVER_CONNECTION_HANDLE));
    OI_STATUS status;

    if (!connection) {
        return;
    }

    connection->ofsCfmPending = TRUE;
    status = connection->server->objops->Write(connection->ofsHandle,
                                               connection->rcvObj.objData.data,
                                               connection->rcvObj.objData.len,
                                               PushWriteCfm,
                                               connection->obexHandle);
    connection->rcvObj.objData.data = NULL;
    connection->rcvObj.objData.len = 0;
    if (!OI_SUCCESS(status)) {
        OI_STATUS   retVal;

        OI_LOG_ERROR(("OFS Write() failed %d", status));

        connection->ofsCfmPending = FALSE;
        /*
         * Report error to client.
         */
        retVal = OI_OBEXSRV_PutResponse(connection->obexHandle, NULL, status);
        if (!OI_SUCCESS(retVal)) {
            OI_SLOG_ERROR(retVal, ("OI_OBEXSRV_PutResponse failed"));
        }
        SetupDeferredObjectClose(connection, status);
    }
}


static void PushOpenCfm(OI_OPP_HANDLE handle,
                        OI_STATUS status,
                        OI_OPP_CONNECTION oppContext)
{
    OPPSRV_CONNECTION *connection = LookupConnection(oppContext);

    OI_DBGTRACE(("PushOpenCfm %d", status));

    /*
     * First check we are expecting this call.
     */
    if (!connection) {
        OI_SLOG_ERROR(OI_STATUS_INVALID_HANDLE, ("Invalid connection handle"));
        return;
    }
    if ((connection->state != OPP_PUSH_PENDING) || !connection->ofsCfmPending) {
        OI_SLOG_ERROR(OI_STATUS_INVALID_STATE, ("Push open confirm called at wrong time"));
        return;
    }
    /*
     * Confirm is no longer pending
     */
    connection->ofsCfmPending = FALSE;
    /*
     * If OFS open was successful, we must eventuall call the object close() method.
     * If OFS open failed, we must NOT call the object close() method.
     * Handle is the trigger to call close() or not.
     */
    connection->ofsHandle = OI_SUCCESS(status) ? handle : NULL;
    if (OI_SUCCESS(status)) {
        DISPATCH_ARG darg;
        connection->ofsHandle = handle;
        SetState(connection, OPP_CLIENT_PUSHING);
        Dispatch_SetArg(darg, connection->obexHandle);
        (void) OI_Dispatch_RegisterFunc(FirstBodyHeader, &darg, NULL);
    } else {
        OI_OBEXSRV_PutResponse(connection->obexHandle, NULL, status);
        SetupDeferredObjectClose(connection, status);
    }
}


static OI_STATUS ObjPushInd(OI_OBEXSRV_CONNECTION_HANDLE connectionId,
                            OI_OBEX_HEADER_LIST *cmdHeaders,
                            OI_STATUS obexStatus)
{
    OPPSRV_CONNECTION *connection = LookupConnection(connectionId);
    OI_UINT typeLen;
    OI_CHAR *objType;
    OI_STATUS status = obexStatus;
    static int count = 0;

    OI_DBGTRACE(("ObjPushInd %d", obexStatus));

    /*
     * Check for an outstanding close operation.
     */
    status = CheckDeferredObjectClose(connection);
    if (!OI_SUCCESS(status)) {
        return status;
    }
    /*
     * If an OFS confirm is pending, status must indicate a disconnect event.  Normally,
     *   there cannot be a push indication until we have responded to the previous push
     *   indication, disconnect is the exception.
     */
    if (connection->ofsCfmPending) {
        if (OI_OBEX_NOT_CONNECTED == obexStatus) {
            /*
             * Ignore this error - we are depending on Obex to call our disconnect indication.
             */
            OI_DBGTRACE(("ObjPushInd while ofsCfmPending, ignoring status %d\n", obexStatus));
            return OI_OK;
        }
        /*
         * This can happen when a remote device sends a command before
         * we've replied to the previous command.  Defer error handling
         * until OFS finishes.
         */
        OI_DBGTRACE(("Deferring error handling (%d) until OFS cfm completed", obexStatus));

        connection->deferredPPError = TRUE;
        connection->deferredErrorStatus = obexStatus;

        /*
         * If OBEX passed us an OI_OK status, inform it that an error occurred.
         * Otherwise, OBEX knows about the error already and we're OI_OK.
         */
        return (OI_SUCCESS(obexStatus) ? OI_STATUS_INTERNAL_ERROR : OI_OK);
    }

    /*
     * If OBEX reported an error the PUSH operation has been terminated so
     * all we need to do to cleanup is close the object. We return OI_OK
     * because OBEX knows about the error.
     */
    if (!OI_SUCCESS(obexStatus) && (obexStatus != OI_OBEX_CONTINUE)) {
        SetupDeferredObjectClose(connection, obexStatus);
        return OI_OK;
    }

    /*
     * Check that the server is allowing PUSH on this connection.
     */
    if (!connection->allowPush) {
        OI_DBGPRINTSTR(("OPP server ObjPushInd - server disallows push"));
        return OI_OBEX_ACCESS_DENIED;
    }

    if (cmdHeaders == NULL) {
        status = OI_OBEX_REQUIRED_HEADER_NOT_FOUND;
        goto ObjPushError;
    }

    if (connection->state == OPP_SERVER_CONNECTED) {
        SetState(connection, OPP_PUSH_PENDING);
    }

    if ((connection->state != OPP_CLIENT_PUSHING) && (connection->state != OPP_PUSH_PENDING)) {
        return OI_OBEX_SERVICE_UNAVAILABLE;
    }

    if (obexStatus == OI_OBEX_CONTINUE) {
        connection->final = FALSE;
    } else {
        connection->final = TRUE;
    }

    status = OPPCommon_ParseHeaders(&connection->rcvObj, cmdHeaders);
    if (!OI_SUCCESS(status)) {
        goto ObjPushError;
    }

    if (connection->state == OPP_PUSH_PENDING) {
        /*
         * Wait until we have seen a body header before attempting to open the
         * object. This ensures that we have all the other headers we need.
         */
        if (connection->final || (connection->rcvObj.objData.data != NULL)) {
            /*
             * Name is mandatory.
             */
            if (connection->rcvObj.name.str == NULL) {
                status = OI_OBEX_ACCESS_DENIED;
                goto ObjPushError;
            }
            /*
             * Some broken implementations do not provide a type header. So we
             * try to figure out the type from the suffix on the object name.
             *
             * Some broken implementations do not null terminate the type string
             * so we need the size too.
             */
            if (connection->rcvObj.type.data == NULL) {
                objType = ObjTypeFromSuffix(&connection->rcvObj.name);
                typeLen = OI_StrLen(objType);
            } else {
                objType = (OI_CHAR*) connection->rcvObj.type.data;
                typeLen = connection->rcvObj.type.len;
            }
            /*
             * Check we support this object type.
             */
            if (!IsSupportedType(objType, typeLen, connection->server->supportedFormats)) {
                status = OI_OBEX_UNSUPPORTED_MEDIA_TYPE;
                goto ObjPushError;
            }
            /*
             * Try and open the object.
             */
            if (connection->server->callbacks->eventInd == NULL) {
                connection->ofsCfmPending = TRUE;
                status = connection->server->objops->OpenWrite(&connection->rcvObj.name,
                                                               objType,
                                                               connection->rcvObj.objSize,
                                                               PushOpenCfm,
                                                               connection->obexHandle);
                if (!OI_SUCCESS(status)) {
                    connection->ofsCfmPending = FALSE;
                }
            } else {
                OI_OPP_SERVER_EVENT_DATA eventData;
                eventData.event = OI_OPP_SERVER_EVENT_PUSH;
                eventData.data.push.localName = &connection->rcvObj.name;
                eventData.data.push.totalSize = connection->rcvObj.objSize;
                eventData.data.push.objType = objType;
                connection->ofsAcceptPending = TRUE;
                connection->server->callbacks->eventInd(connection->obexHandle, &eventData);
                status = OI_OK;
            }
        } else {
            /*
             * Get more data from the client.
             */
            status = OI_OBEXSRV_PutResponse(connectionId, NULL, obexStatus);
        }
    } else {
        /*
         * Write data to the object
         */
        connection->ofsCfmPending = TRUE;
        status = connection->server->objops->Write(connection->ofsHandle,
                                                   connection->rcvObj.objData.data,
                                                   connection->rcvObj.objData.len,
                                                   PushWriteCfm,
                                                   connection->obexHandle);
        if (!OI_SUCCESS(status)) {
            connection->ofsCfmPending = FALSE;
        }
        connection->rcvObj.objData.data = NULL;
        connection->rcvObj.objData.len = 0;
    }

    if (!OI_SUCCESS(status)) {
        goto ObjPushError;
    }

    return OI_OK;

ObjPushError:

    OI_DBGTRACE(("OPP server object push failed %d", status));
    SetupDeferredObjectClose(connection, status);
    return status;
}


static void PullOpenCfm(OI_OPP_HANDLE handle,
                        const OI_OBEX_UNICODE *name,
                        const OI_CHAR *type,
                        OI_UINT32 size,
                        OI_BYTE * data,
                        OI_UINT16 data_len,
                        OI_STATUS status,
                        OI_OPP_CONNECTION oppContext)
{
    OPPSRV_CONNECTION *connection = LookupConnection(oppContext);
    OI_OBEX_HEADER hdrs[3];
    OI_OBEX_HEADER_LIST hdrList;

    OI_DBGTRACE(("PullOpenCfm %d", status));

    /*
     * First check we are expecting this call.
     */
    if (!connection) {
        OI_SLOG_ERROR(OI_STATUS_INVALID_HANDLE, ("Invalid connection handle"));
        return;
    }
    if ((connection->state != OPP_PULL_PENDING) || !connection->ofsCfmPending) {
        OI_LOG_ERROR(("Pull open confirm called at wrong time"));
        return;
    }
    /*
     * Confirm is no longer pending
     */
    connection->ofsCfmPending = FALSE;
    /*
     * If OFS open was successful, we must eventuall call the object close() method.
     * If OFS open failed, we must NOT call the object close() method.
     * Handle is the trigger to call close() or not.
     */
    connection->ofsHandle = OI_SUCCESS(status) ? handle :NULL;
    /*
     * If push/pull error occurred while awaiting the cfm, fail the current operation
     */
    if (connection->deferredPPError) {
        connection->deferredPPError = FALSE;
        status = connection->deferredErrorStatus;
    }
    /*
     * Now finally comes the 'normal' handling of the confirm
     */
    if (!OI_SUCCESS(status)) {
        /*
         * Reject the pull request by reporting an error status to the client.
         */
        (void) OI_OBEXSRV_GetResponse(connection->obexHandle, NULL, status);
    } else {
        if (connection->server->callbacks->eventInd) {
            OI_OPP_SERVER_EVENT_DATA eventData;
            eventData.event = OI_OPP_SERVER_EVENT_PULL;
            eventData.data.pull.localName = name;
            eventData.data.pull.totalSize = size;
            eventData.data.pull.objType = type;
            connection->rcvObj.objSize = size;
            connection->server->callbacks->eventInd(connection->obexHandle, &eventData);
        }

        /*
         * Send the basic info for the default object to the client.
         */
        hdrList.count = 0;
        hdrList.list = hdrs;

        hdrs[hdrList.count].id = OI_OBEX_HDR_LENGTH;
        hdrs[hdrList.count].val.length = size;
        ++hdrList.count;
        #ifdef OI_TEST_HARNESS
            if (TEST_OPP_MISSING_SERVER_PULL_HEADER != OI_OPP_GetTestName()) {
                hdrs[hdrList.count].id = OI_OBEX_HDR_NAME;
                hdrs[hdrList.count].val.name = *name;
                ++hdrList.count;
            }
        #else
            hdrs[hdrList.count].id = OI_OBEX_HDR_NAME;
            hdrs[hdrList.count].val.name = *name;
            ++hdrList.count;
        #endif
        hdrs[hdrList.count].id = OI_OBEX_HDR_TYPE;
        hdrs[hdrList.count].val.type.data = (OI_BYTE*) type;
        hdrs[hdrList.count].val.type.len = OI_StrLen(type) + 1; /* include nul terminator */
        ++hdrList.count;

        status = OI_OBEXSRV_GetResponse(connection->obexHandle, &hdrList, OI_OBEX_CONTINUE);
    }

    if (OI_SUCCESS(status)) {
        SetState(connection, OPP_CLIENT_PULLING);
    } else {
        SetupDeferredObjectClose(connection, status);
    }
}


/*
 * Called when to confirm read of data from an object. Passes in the data read
 * from the object.
 */

static void PullReadCfm(OI_OPP_HANDLE handle,
                        OI_BYTE *data,
                        OI_UINT32 len,
                        OI_STATUS status,
                        OI_OPP_CONNECTION oppContext)
{
    OPPSRV_CONNECTION *connection = LookupConnection(oppContext);
    OI_STATUS obexStatus;
    OI_OBEX_HEADER bodyHdr;
    OI_OBEX_HEADER_LIST hdrList;

    OI_DBGTRACE(("PullReadCfm %d", status));

    /*
     * First check we are expecting this call.
     */
    if (!connection) {
        OI_SLOG_ERROR(OI_STATUS_INVALID_HANDLE, ("Invalid connection handle"));
        return;
    }
    if ((connection->state != OPP_CLIENT_PULLING) || !connection->ofsCfmPending) {
        OI_LOG_ERROR(("Pull read confirm called at wrong time"));
        return;
    }
    /*
     * Confirm is no longer pending
     */
    connection->ofsCfmPending = FALSE;
    if (!OI_SUCCESS(status) && (status != OI_STATUS_END_OF_FILE)) {
        /*
         * Report error to client.
         */
        OI_OBEXSRV_GetResponse(connection->obexHandle, NULL, status);
    } else {
        /*
         * Send the next body header.
         */
        if (status == OI_STATUS_END_OF_FILE) {
            obexStatus = OI_OK;
            bodyHdr.id = OI_OBEX_HDR_END_OF_BODY;
        } else {
            obexStatus = OI_OBEX_CONTINUE;
            bodyHdr.id = OI_OBEX_HDR_BODY;
        }
        bodyHdr.val.body.len = (OI_UINT16) len;
        bodyHdr.val.body.data = data;
        hdrList.count = 1;
        hdrList.list = &bodyHdr;
        /*
         * Send the GET command response to client.
         */
        status = OI_OBEXSRV_GetResponse(connection->obexHandle, &hdrList, obexStatus);
    }
    /*
     * Close the object if there was an error
     */
    if (!OI_SUCCESS(status)) {
        SetupDeferredObjectClose(connection, status);
    }
}


/*
 * The only object that can be PULLED is the default object - the owner's
 * business card.
 */
static OI_STATUS ObjPullInd(OI_OBEXSRV_CONNECTION_HANDLE connectionId,
                            OI_OBEX_HEADER_LIST *cmdHeaders,
                            OI_STATUS obexStatus)
{
    OPPSRV_CONNECTION *connection = LookupConnection(connectionId);
    OI_STATUS status = OI_OK;
    OI_INT i;

    OI_DBGTRACE(("ObjPullInd %d", obexStatus));

    /*
     * Check for an outstanding close operation.
     */
    status = CheckDeferredObjectClose(connection);
    if (!OI_SUCCESS(status)) {
        return status;
    }
    /*
     * If OBEX reported an error the PULL operation has been terminated so
     * all we need to do to cleanup is close the object. We return OI_OK
     * because OBEX already knows about the error.
     */
    if (!OI_SUCCESS(obexStatus) && (obexStatus != OI_OBEX_CONTINUE)) {
        if (connection->ofsCfmPending) {
            /*
             * We are waiting for OFS cfm callback so cannot close the object yet. Record the fact
             * the a push/pull error occurred and the object close will be handled by the ofs cfm
             * callback. The only error we expect while awaiting OFS cfm is DISCONNECT anything
             * else is an internal error.
             */
            if (obexStatus != OI_OBEX_NOT_CONNECTED) {
                OI_LOG_ERROR(("Internal error, ObjPullInd (%d) while awaiting OFS cfm", obexStatus));
            }
            OI_DBGTRACE(("Deferring error handling (%d) until OFS cfm completed", obexStatus));
            connection->deferredPPError = TRUE;
            connection->deferredErrorStatus = obexStatus;
        } else {
            SetupDeferredObjectClose(connection, obexStatus);
        }
        return OI_OK;
    }

    /*
     * In the absence of push/pull errors, there should never be a OFS cfm outstanding.
     */
    OI_ASSERT(!connection->ofsCfmPending);
    if (connection->ofsCfmPending) {
        OI_LOG_ERROR(("Internal error, ObjPullInd (%d) while awaiting OFS cfm", obexStatus));
        connection->deferredPPError = TRUE;
        connection->deferredErrorStatus = OI_OBEX_ACCESS_DENIED;
        return OI_OBEX_ACCESS_DENIED;
    }

    /*
     * Check that the server is allowing PULL on this connection.
     */
    if (!connection->allowPull) {
        OI_DBGPRINTSTR(("OPP server ObjPullInd - server disallows pull"));
        return OI_OBEX_ACCESS_DENIED;
    }

    /*
     * If the PULL operation is now complete close the object.
     */
    if (obexStatus == OI_OK) {
        goto ObjPullComplete;
    }

    OI_ASSERT(obexStatus == OI_OBEX_CONTINUE);

    if (connection->state == OPP_SERVER_CONNECTED) {
        OI_BD_ADDR addr;
        OI_OBEX_BYTESEQ *type = NULL;

        status = OI_OBEXSRV_GetClientAddr(connectionId, &addr);
        if (!OI_SUCCESS(status)) {
            goto ObjPullComplete;
        }

        OI_DBGTRACE(("OPP server pull request from %:", &addr));

        /*
         * Starting a PULL operation - parse the headers
         */
        for (i = 0; i < cmdHeaders->count; ++i) {
            OI_OBEX_HEADER *tmpHdr = cmdHeaders->list + i;
            switch (tmpHdr->id) {
                case OI_OBEX_HDR_NAME:
                    /*
                     * Section 4.3.2 of Bluetooth OPP spec says that the name
                     * header must NOT be used for object pull, but the example
                     * in section 8.4.1 of the IrDA OBEX spec (version 1.2)
                     * shows the use of an empty name header. So we will allow
                     * an empty name header...
                     */
                    if (tmpHdr->val.name.len != 0) {
                        status = OI_OBEX_ACCESS_DENIED;
                        OI_LOG_ERROR(("Name header not allowed for object pull"));
                        goto ObjPullComplete;
                    }
                    break;
                case OI_OBEX_HDR_TYPE:
                    type = &(tmpHdr->val.type);
                    break;
                default:
                    /* Ignore other headers. */
                    break;
            }
        }
        if ((type == NULL) || (type->len == 0)) {
            OI_DBGPRINTSTR(("Object pull no type header"));
            status = OI_OBEX_ACCESS_DENIED;
            goto ObjPullComplete;
        }
        /*
         * vcard is the only object type allowed for object pull.
         * Some broken implementations do not null terminate the type string
         * so we perform string comparison without considering the string terminator.
         */
        if (0 != OI_StrncmpInsensitive( OI_OBEX_VCARD_TYPE,
                                        (OI_CHAR*) type->data,
                                        (OI_UINT16) OI_StrLen(OI_OBEX_VCARD_TYPE))) {
            status = OI_OBEX_NOT_FOUND;
            goto ObjPullComplete;
        }

        /*
         * Try to open the default object.  Specify the default object explicitly, type->data from
         * opp client may not be null terminted (contrary to spec).
         */
        SetState(connection, OPP_PULL_PENDING);
        connection->ofsCfmPending = TRUE;
        status = connection->server->objops->OpenRead(NULL, (OI_CHAR*) OI_OBEX_VCARD_TYPE,
            connection->maxReadSize, PullOpenCfm, (OI_OPP_CONNECTION) connection->obexHandle);
        if (!OI_SUCCESS(status)) {
            connection->ofsCfmPending = FALSE;
            goto ObjPullComplete;
        }
    } else {
        if (connection->state != OPP_CLIENT_PULLING) {
            return OI_OBEX_SERVICE_UNAVAILABLE;
        }
        /*
         * Read from the default object.
         */
        connection->ofsCfmPending = TRUE;

        status = connection->server->objops->Read(connection->ofsHandle, connection->maxReadSize, PullReadCfm, (OI_OPP_CONNECTION) connection->obexHandle);
        if (status == OI_STATUS_END_OF_FILE) {
            PullReadCfm(connection->ofsHandle, NULL, 0, status, (OI_OPP_CONNECTION) connection->obexHandle);
            status = OI_OK;
        }
        if (!OI_SUCCESS(status)) {
            connection->ofsCfmPending = FALSE;
            OI_DBGTRACE(("read from default object returned %d", status));
            goto ObjPullComplete;
        }
    }

    return OI_OK;


ObjPullComplete:

    SetupDeferredObjectClose(connection, status);
    return status;
}


OI_STATUS OI_OPP_AcceptConnect(OI_OPP_SERVER_CONNECTION_HANDLE connectionId,
                               OI_BOOL allowPush,
                               OI_BOOL allowPull)
{
    OPPSRV_CONNECTION *connection = LookupConnection(connectionId);
    OI_STATUS status;
    OI_BOOL accept = allowPush || allowPull;

    OI_DBGTRACE(("OI_OPP_AcceptConnect allowPush:%d allowPull:%d", allowPush, allowPull));

    if (!connection) {
        return OI_STATUS_INVALID_HANDLE;
    }
    if ((connection->state != OPP_SERVER_CONNECTING) || (connectionId != connection->obexHandle)) {
        return OI_OBEX_INVALID_OPERATION;
    }
    /*
     * If PUSH is allowed we need to be able to write the object
     */
    if (allowPush && !connection->server->objops->OpenWrite) {
        status = OI_STATUS_INVALID_PARAMETERS;
        goto RejectConnection;
    }
    /*
     * If PULL is allowed we need to be able to read the object
     */
    if (allowPull && !connection->server->objops->OpenRead) {
        status = OI_STATUS_INVALID_PARAMETERS;
        goto RejectConnection;
    }

    status = OI_OBEXSRV_AcceptConnect(connection->obexHandle, accept, accept ? OI_OK : OI_OBEX_ACCESS_DENIED, NULL);
    if (accept && OI_SUCCESS(status)) {
        /*
         * Save access rights.
         */
        connection->allowPush = allowPush;
        connection->allowPull = allowPull;
        /*
         * Establish the maximum read size for object read operations.
         */
        connection->maxReadSize = OI_OBEXSRV_OptimalBodyHeaderSize(connection->obexHandle);
        /*
         * Server is connected.
         */
        SetState(connection, OPP_SERVER_CONNECTED);
    } else {
        /*
         * Dissociate OPP connection from the OBEX connection handle
         */
        OI_OBEXSRV_SetConnectionContext(connectionId, NULL);
        OI_Free(connection);
    }
    return status;

RejectConnection:

    OI_OBEXSRV_AcceptConnect(connection->obexHandle, FALSE, OI_OBEX_ACCESS_DENIED, NULL);
    OI_OBEXSRV_SetConnectionContext(connectionId, NULL);
    OI_Free(connection);
    return status;
}


OI_STATUS OI_OPPServer_ForceDisconnect(OI_OPP_SERVER_CONNECTION_HANDLE connectionId)
{

    OPPSRV_CONNECTION *connection = LookupConnection(connectionId);

    if (!connection) {
        return OI_STATUS_INVALID_HANDLE;
    }

    OI_DBGTRACE(("OI_OPPServer_ForceDisconnect connectionId:%#x %s", connectionId, ServerStateText(connection->state)));

    if (connection->state < OPP_SERVER_CONNECTED) {
        return OI_OBEX_NOT_CONNECTED;
    }
    return OI_OBEXSRV_ForceDisconnect(connection->obexHandle);
}


static void ServerConnectInd(OI_OBEXSRV_CONNECTION_HANDLE connectionId,
                             OI_BOOL                      unauthorized,
                             OI_BYTE                      *userId,
                             OI_UINT8                     userIdLen,
                             OI_OBEX_REALM                *realm)

{
    OI_OBEX_SERVER_HANDLE serverHandle = OI_OBEXSRV_GetServerHandle(connectionId);
    OPPSRV_CONNECTION *connection = NULL;
    OI_STATUS status = OI_OK;
    OPP_SERVER *server = LookupServer(serverHandle);
    OI_BD_ADDR clientAddr;

    OI_ASSERT(serverHandle);

    OI_DBGTRACE(("ServerConnectInd"));

    if (!server) {
        status = OI_OBEX_INTERNAL_SERVER_ERROR;
        OI_SLOG_ERROR(status, ("Server not found for connection"));
        goto RejectConnection;
    }
    /*
     * OPP does not permit OBEX authentication.
     */
    if (unauthorized) {
        status = OI_OBEX_INVALID_OPERATION;
        goto RejectConnection;
    }
    /*
     * Allocate a connection
     */
    connection = OI_Calloc(sizeof(*connection));
    if (!connection) {
        status = OI_STATUS_NO_RESOURCES;
        goto RejectConnection;
    }
    /*
     * Associate OPP connection with the OPP server and underlying OBEX connection
     */
    OI_OBEXSRV_SetConnectionContext(connectionId, connection);
    connection->obexHandle = connectionId;
    connection->server = server;
    SetState(connection, OPP_SERVER_CONNECTING);
    /*
     * If the application wants to know about connection indications
     * call the connect indication callback, otherwise simply accept the
     * connection.
     */
    status = OI_OBEXSRV_GetClientAddr(connectionId, &clientAddr);
    if (OI_SUCCESS(status)) {
        OI_L2CAP_CID cid;
        /*
         * Register OPP connection with the AMP policy manager
         */
        status = OI_OBEXSRV_GetL2capCID(connectionId, &cid);

        if (server->callbacks->connectInd) {
            /*
             * Pass address of connecting client to application.
             */
            server->callbacks->connectInd(&clientAddr, connectionId);
        } else {
            status = OI_OPP_AcceptConnect(connectionId, TRUE, TRUE);
            if (!OI_SUCCESS(status)) {
                goto RejectConnection;
            }
        }
    } else {
        goto RejectConnection;
    }
    return;

RejectConnection:

    if (connection) {
        /*
         * Dissociate OPP connection from the OBEX connection handle
         */
        OI_OBEXSRV_SetConnectionContext(connectionId, NULL);
        OI_Free(connection);
    }
    (void) OI_OBEXSRV_AcceptConnect(connectionId, FALSE, status, NULL);
}


static void ServerDisconnectInd(OI_OBEXSRV_CONNECTION_HANDLE connectionId)
{
    OPPSRV_CONNECTION *connection = LookupConnection(connectionId);

    OI_DBGTRACE(("ServerDisconnectInd connectionId:%#x", connectionId));

    if (!connection) {
        OI_SLOG_ERROR(OI_STATUS_INVALID_HANDLE, ("Disconnect for invalid handle"));
        return;
    }
    /*
     * If close is already pending on the dispatcher cancel it.
     */
    if (connection->closeCB) {
        OI_Dispatch_CancelFunc(connection->closeCB);
    }
    /*
     * Status to be reported to OFS and app depends on whether we're idle or pushing/pulling
     */
    ObjectClose(connection, TRUE, connection->state > OPP_SERVER_CONNECTED? OI_OBEX_NOT_CONNECTED : OI_OK);
}



#define MAX_OPP_FORMAT   8


static OI_STATUS InitFormatsAttribute(OI_UINT32 formats,
                                      OI_SDPDB_ATTRIBUTE *supportedFormats)
{
    OI_DATAELEM *formatList = NULL;
    OI_UINT i;
    OI_UINT numFormats;

    /*
     * Formats are specified as a bit vector that identifies the supported
     * object formats.
     */
    if (formats == OI_OPP_SERVER_OBJ_FORMAT_ANY) {
        numFormats = 1;
        formatList = OI_Malloc(sizeof(OI_DATAELEM));
        if (formatList == NULL) {
            return OI_STATUS_OUT_OF_MEMORY;
        }
        OI_SET_UINT_ELEMENT(formatList[0], (OI_UINT8) 0xFF);
    } else {
        /*
         * Count how many formats have been specified.
         */
        numFormats = 0;
        for (i = 0; i < MAX_OPP_FORMAT; ++i) {
            if (formats & (1 << i)) {
                ++numFormats;
            }
        }
        if (numFormats > 0) {
            formatList = OI_Malloc(numFormats * sizeof(OI_DATAELEM));
            if (formatList == NULL) {
                return OI_STATUS_OUT_OF_MEMORY;
            }
            /*
             * Supported formats are numbered in the range 1 .. MAX_OPP_FORMAT
             * (in the Bluetooth Specification 1.1 OPP spec this is 1..6)
             */
            numFormats = 0;
            for (i = 0; i < MAX_OPP_FORMAT; ++i) {
                if (formats & (1 << i)) {
                    OI_SET_UINT_ELEMENT(formatList[numFormats], (OI_UINT8) (i + 1));
                    ++numFormats;
                }
            }
        }
    }
    /*
     * Initialize attribute with the formats element list.
     */
    if (numFormats == 0) {
        OI_SET_EMPTY_LIST_ELEMENT(supportedFormats->Element, OI_DATAELEM_SEQ);
    } else {
        OI_SET_LIST_ELEMENT(supportedFormats->Element, OI_DATAELEM_SEQ, formatList[0], numFormats);
    }
    supportedFormats->Id = OI_ATTRID_SupportedFormatsList;
    return OI_OK;
}


static void ServerProgressInd(OI_OBEXSRV_CONNECTION_HANDLE connectionId,
                              OI_UINT8 cmd,
                              OI_UINT32 progress)
{
    OPPSRV_CONNECTION *connection = LookupConnection(connectionId);
    if (connection != NULL) {
        OI_OPP_SERVER_EVENT_DATA eventData;

        OI_DBGTRACE(("%s progress %d", cmd == OI_OBEX_CMD_PUT ? "PUT" : "GET", progress));

        if (connection->server->callbacks->eventInd) {
            eventData.event = (cmd == OI_OBEX_CMD_PUT) ? OI_OPP_SERVER_EVENT_PUSH_PROGRESS : OI_OPP_SERVER_EVENT_PULL_PROGRESS;
            eventData.data.pushProgress.bytesTransferred = progress;
            connection->server->callbacks->eventInd(connectionId, &eventData);
        }
    }
}


OI_STATUS OI_OPPServer_Register(const OI_OPP_SERVER_CALLBACKS *callbacks,
                                const OI_OPP_OBJSYS_FUNCTIONS *objectOperations,
                                OI_OPP_SERVER_OBJECT_FORMATS   supportedFormats,
                                const OI_SDP_STRINGS          *strings,
                                OI_OPP_SERVER_HANDLE          *serverInstance)
{
    static const OI_OBEXSRV_CB_LIST CBList = {
        ServerConnectInd,
        ServerDisconnectInd,
        ObjPullInd,
        ObjPushInd,
        NULL,   /* no setpath */
        NULL,   /* no bulk get indication */
        NULL,   /* No action commands */
        ServerProgressInd,
        NULL,
        NULL
    };
    OI_STATUS status;
    OI_SDPDB_SERVICE_RECORD srec;
    OPP_SERVER *server;
    OI_OBEX_CONNECTION_OPTIONS connOpts;
    OI_OBEX_LOWER_PROTOCOL lowerProtocol[2];
    OI_UINT numProtocols = 0;

    OI_DBGTRACE(("OI_OPPServer_Register"));

    OI_ARGCHECK(callbacks);
    /*
     * connectInd and disconnectInd must be both NULL or both non-NULL
     */
    OI_ARGCHECK((callbacks->connectInd == NULL) == (callbacks->disconnectInd == NULL));

    server = OI_Calloc(sizeof(OPP_SERVER));
    if (server == NULL) {
        status = OI_STATUS_NO_RESOURCES;
        goto ErrorExit;
    }

    if (objectOperations->OpenRead) {
        OI_ARGCHECK(objectOperations->Read);
    }
    if (objectOperations->OpenWrite) {
        OI_ARGCHECK(objectOperations->Write);
    }
    OI_ARGCHECK(objectOperations->Close);

    /*
     * Store connect and disconnect indication callbacks.
     */
    server->callbacks = callbacks;
    server->objops = objectOperations;

    /*
     * Try and get the preferred OPP channel number.
     */
    lowerProtocol[numProtocols].protocol = OI_OBEX_LOWER_RFCOMM;
    lowerProtocol[numProtocols].svcId.rfcommChannel = OI_CONFIG_TABLE_GET(OPP_SRV)->rfcomm_channel_pref;
    ++numProtocols;
    lowerProtocol[numProtocols].protocol = OI_OBEX_LOWER_L2CAP;
    lowerProtocol[numProtocols].svcId.l2capPSM = OI_CONFIG_TABLE_GET(OPP_SRV)->l2cap_psm_pref;
    ++numProtocols;
    OI_DBGTRACE(("Listening on %d RFCOMM SCN, and %d L2CAP PSM",
        OI_CONFIG_TABLE_GET(OPP_SRV)->rfcomm_channel_pref,
        OI_CONFIG_TABLE_GET(OPP_SRV)->l2cap_psm_pref));

    /*
     * We allow SRM for OPP
     */
    connOpts.enableSRM = TRUE;
    status = OI_OBEXSRV_RegisterServer(NULL, &CBList, OI_OBEXSRV_AUTH_NONE, &connOpts,
        lowerProtocol, numProtocols, &connectPolicy, &server->serverHandle);
    if (!OI_SUCCESS(status)) {
        goto ErrorExit;
    }

    /*
     * Register service record
     */
    srec.Attributes = ServiceDescription;
    srec.NumAttributes = OI_ARRAYSIZE(ServiceDescription);
    srec.Strings = strings->attrs;
    srec.NumStrings = strings->num;

    /*
     * Initialize supported formats attribute list.
     */
    OI_ASSERT(server->numAttributes < OI_ARRAYSIZE(server->sdpAttributes));
    server->supportedFormats = supportedFormats;
    status = InitFormatsAttribute(supportedFormats, &server->sdpAttributes[server->numAttributes]);
    ++server->numAttributes;
    if (!OI_SUCCESS(status)) {
        goto ErrorExit;
    }

    OI_DBGTRACE(("Registered Object Push Profile"));

    OI_INIT_FLAG_INCREMENT(OPP_SRV);
    /*
     * Associated FTP server with the OBEX server
     */
    OI_OBEXSRV_SetServerContext(server->serverHandle, server);
    /*
     * Return the server instance handle to the caller
     */
    *serverInstance = server->serverHandle;

    return OI_OK;

ErrorExit:

    OI_LOG_ERROR(("OI_OPPServer_Register failed %d", status));

    if (server) {
        if (server->serverHandle) {
            (void) OI_OBEXSRV_DeregisterServer(server->serverHandle);
        }
        OI_Free(server);
    }

    return status;
}


OI_STATUS OI_OPPServer_Deregister(OI_OPP_SERVER_HANDLE serverInstance)
{
    OPP_SERVER *server = LookupServer(serverInstance);
    OI_STATUS status;

    OI_DBGTRACE(("OI_OPPServer_Deregister"));

    if (!OI_INIT_FLAG_VALUE(OPP_SRV)) {
        return OI_STATUS_NOT_REGISTERED;
    }
    if (!server) {
        return OI_STATUS_INVALID_HANDLE;
    }
    status = OI_OBEXSRV_DeregisterServer(server->serverHandle);
    if (!OI_SUCCESS(status)) {
        OI_LOG_ERROR(("Error deregistering OPP server %d", status));
    } else {
        OI_Free(server);
        OI_INIT_FLAG_DECREMENT(OPP_SRV);
    }
    return status;
}


OI_STATUS OI_OPPServer_GetServiceRecord(OI_OPP_SERVER_HANDLE serverInstance,
                                        OI_UINT32 *handle)
{
    OPP_SERVER *server = LookupServer(serverInstance);

    OI_CHECK_INIT(OPP_SRV);

    if (!server) {
        return OI_STATUS_INVALID_HANDLE;
    }
    *handle = server->srecHandle;
    return OI_OK;
}


OI_STATUS OI_OPPServer_AcceptPush(OI_OPP_SERVER_CONNECTION_HANDLE connectionId,
                                  OI_OBEX_UNICODE                *objname,
                                  OI_BOOL                         accept)
{
    OPPSRV_CONNECTION *connection = LookupConnection(connectionId);

    if (!connection) {
        return OI_STATUS_INVALID_HANDLE;
    }
    if (connection->state != OPP_PUSH_PENDING) {
        return OI_STATUS_INVALID_STATE;
    }
    if (!connection->ofsAcceptPending) {
        return OI_STATUS_INVALID_STATE;
    }
    connection->ofsAcceptPending = FALSE;
    if (accept) {
        OI_STATUS status;
        connection->ofsCfmPending = TRUE;
        if (objname) {
            status = connection->server->objops->OpenWrite(objname,
                                                           (OI_CHAR*) connection->rcvObj.type.data,
                                                           connection->rcvObj.objSize,
                                                           PushOpenCfm,
                                                           connection->obexHandle);
        } else {
            status = connection->server->objops->OpenWrite(&connection->rcvObj.name,
                                                           (OI_CHAR*) connection->rcvObj.type.data,
                                                           connection->rcvObj.objSize,
                                                           PushOpenCfm,
                                                           connection->obexHandle);
        }
        if (!OI_SUCCESS(status)) {
            /*
             * If file could not be opened, return failure to remote device,
             * and cleanup state. This mimics the behavior of a reject.
             */
            connection->ofsCfmPending = FALSE;
            SetState(connection, OPP_SERVER_CONNECTED);
            OI_FreeIf(&connection->rcvObj.name.str);
            OI_FreeIf(&connection->rcvObj.type.data);
            OI_MemZero(&connection->rcvObj, sizeof(OI_OPP_GENERIC_OBJECT));
            OI_OBEXSRV_PutResponse(connectionId, NULL, OI_OBEX_ACCESS_DENIED);
        }
        return status;
    } else {
        connection->ofsCfmPending = FALSE;
        SetState(connection, OPP_SERVER_CONNECTED);
        OI_FreeIf(&connection->rcvObj.name.str);
        OI_FreeIf(&connection->rcvObj.type.data);
        OI_MemZero(&connection->rcvObj, sizeof(OI_OPP_GENERIC_OBJECT));
        return OI_OBEXSRV_PutResponse(connectionId, NULL, OI_OBEX_ACCESS_DENIED);
    }
}
