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

API's for OBEX Client
*/

#define __OI_MODULE__ OI_MODULE_OBEX_CLI

#include "oi_dispatch.h"
#include "oi_bytestream.h"
#include "oi_osinterface.h"
#include "oi_memmgr.h"
#include "oi_argcheck.h"
#include "oi_utils.h"
#include "oi_assert.h"
#include "oi_debug.h"
#include "oi_handle.h"

#include "oi_init_flags.h"
#include "oi_config_table.h"
#include "oi_bt_profile_config.h"

#include "oi_obextest.h"
#include "oi_obex_lower.h"
#include "oi_obexcommon.h"
#include "oi_obexauth.h"
#include "oi_obexcli.h"


/*
 * Timeout for a response to an abort command
 */
#define OBEX_ABORT_TIMEOUT   OI_SECONDS(2)

/*
 * How long we will wait for a disconnect to complete
 */
#define OBEX_DISCONNECT_TIMEOUT   OI_SECONDS(5)



#define CONNECT_REQ_LEN     16
#define DISCONNECT_REQ_LEN  3
#define ABORT_RSP_LEN       3

#define TMP_BYTESTREAM_SIZE 8


/**
 * Connection states - the order matters.
 */

typedef enum {
    INVALID_CONNECTION     = 0,
    LOWER_LAYER_CONNECTING = 1,
    LOWER_LAYER_CONNECTED  = 2,
    OBEX_DISCONNECTING     = 3,
    OBEX_CONNECTED         = 4,
    OBEX_ABORTING          = 5,
    OBEX_GETTING           = 6,
    OBEX_PUTTING           = 7,
    OBEX_SETTING_PATH      = 8,
    OBEX_BULK_PUTTING      = 9,
    OBEX_DO_ACTION         = 10,
} CONNECTION_STATE;


#define COMMAND_IN_PROGRESS(state)   ((state) >= OBEX_GETTING)


typedef enum {
    NO_ABORT,
    ABORT_PENDING,  /**< Abort has been request but an operation is in progress */
    ABORT_SENT      /**< Abort has been sent and we are waiting for a response */
} ABORT_STATE;


struct _BULK_PUT {
    OI_STATUS status;
    OBEX_BULK_DATA_LIST current; /**< Block currently being written */
    OBEX_BULK_DATA_LIST head;    /**< Oldest block not yet confirmed */
    OBEX_BULK_DATA_LIST tail;    /**< Newest block not yet sent or confirmed */
};

struct _PUT {
    OI_OBEXCLI_PUT_CFM confirm;
};

struct _GET {
    OI_OBEXCLI_GET_RECV_DATA recvData;
};

struct _CONNECT {
    OI_OBEXCLI_CONNECT_CFM confirm;
};

struct _SETPATH {
    OI_OBEXCLI_SETPATH_CFM confirm;
};

struct _ACTION {
    OI_OBEXCLI_ACTION_CFM confirm;
};


/**
 * Connection state
 */

typedef struct {

    OBEX_COMMON common;

    /**
     * Application specific context.
     */
    void *context;

    OI_INTERVAL responseTimeout;     /** current timeout value on this connection */
    DISPATCH_CB_HANDLE timeoutCB;    /** dispatch function handle for timeout function */

    OI_OBEXCLI_DISCONNECT_IND disconnectInd;

    /**
     * Callback function for obtaining a password from the application.
     */
    OI_OBEXCLI_AUTH_CHALLENGE_IND challengeCB;

    OI_OBEXCLI_PROGRESS_IND progressCB; /** Callback to allow upper layer to track progress */

    OI_OBEXCLI_BULK_PUT_CFM bulkPutCfm; /** Callback to confirm bulk puts */

    /**
     * Union of callback information for each cmd
     */
    union {
        struct _PUT put;
        struct _BULK_PUT bulkPut;
        struct _GET get;
        struct _CONNECT connect;
        struct _SETPATH setpath;
        struct _ACTION action;
    } CB;

    /**
     * Copy of the headers passed in to a connect request.
     */
    OI_OBEX_HEADER connectHdrs[OI_OBEX_MAX_CONNECT_HDRS];
    OI_UINT8 connectHdrCount;

    /**
     * If a target was specified != OI_OBEX_INVALID_CONNECTION_ID
     */
    OI_UINT32 cid;

    /**
     * Indicates if the current operation is being aborted.
     */
    ABORT_STATE abort;
    /**
     * The operation that was being aborted
     */
    CONNECTION_STATE abortingState;
    OI_OBEXCLI_ABORT_CFM abortConfirm;

    /**
     * TRUE if an transaction from the upper layer is in progress
     */
    OI_BOOL busy;

    /**
     * TRUE if srm has been requested for the current operation
     */
    OI_BOOL srmRequested;

    /*
     * Indicates that we are in the process of dropping the lower layer connection.
     */
    OI_BOOL disconnectingLowerLayer;

    /*
     * Indicates whether the client requires server authentication.
     */
    OI_OBEXCLI_AUTHENTICATION authRequired;

    CONNECTION_STATE state;

} OBEXCLI_CONNECTION;


/**
 * Context for asynchronously reporting an error.
 */

typedef struct {
    OBEXCLI_CONNECTION *connection;
    OI_STATUS status;
} OBEXCLI_ERROR_CONTEXT;



/**
 * Type for an OBEX client handle
 */
static const OI_CHAR* ObexClientHandleType = "OBEXCLI_CONNECTION";


/***************** Forward function definition *************************/

static void SendAbort(OBEXCLI_CONNECTION *connection);

static void LowerConnectCfm(OI_OBEX_LOWER_CONNECTION lowerConnection,
                            OI_UINT16 recvMtu,
                            OI_UINT16 sendMtu,
                            OI_STATUS result);


static void LowerDisconnectInd(OI_OBEX_LOWER_CONNECTION lowerConnection,
                               OI_STATUS reason);

static void LowerWriteCfm(OI_OBEX_LOWER_CONNECTION lowerConnection,
                          OI_MBUF *mbuf,
                          OI_BOOL queueFull,
                          OI_STATUS result);


static void LowerRecvDataInd(OI_OBEX_LOWER_CONNECTION lowerConnection,
                             OI_BYTE *dataBuf,
                             OI_UINT16 dataLen);


static const OI_OBEX_LOWER_CALLBACKS lowerCallbacks =
{
    LowerConnectCfm,
    NULL,
    LowerDisconnectInd,
    LowerWriteCfm,
    LowerRecvDataInd
};


/************************************************************************/



/*
 * Get a connection struct from a connection handle
 */
#define LookupConnection(connectionHandle)  ((OBEXCLI_CONNECTION*)OI_HANDLE_Deref(connectionHandle))


/*
 * Macro so we can see where the fatal error originated
 */
#define FatalClientError(connection, status) \
    do { \
        OI_SLOG_ERROR(status, ("Fatal OBEXCLI error")); \
        FatalObexError(connection, status); \
    } while (0)



#define SetState(connection, newState) \
    do { \
        OI_DBGTRACE(("State %s -> %s", ClientStateTxt((connection)->state), ClientStateTxt(newState))); \
        (connection)->state = (newState); \
    } while(0);

#define SetAbortState(connection, newState) \
    do { \
        if ((connection)->abort != (newState)) { OI_DBGTRACE(("Abort State %s -> %s", AbortStateTxt((connection)->abort), AbortStateTxt(newState))); }\
        (connection)->abort = (newState); \
    } while(0);

#ifdef OI_DEBUG
static const OI_CHAR* ClientStateTxt(CONNECTION_STATE state)
{
    switch (state) {
        case INVALID_CONNECTION:     return "INVALID_CONNECTION";
        case LOWER_LAYER_CONNECTING: return "LOWER_LAYER_CONNECTING";
        case LOWER_LAYER_CONNECTED:  return "LOWER_LAYER_CONNECTED";
        case OBEX_DISCONNECTING:     return "OBEX_DISCONNECTING";
        case OBEX_CONNECTED:         return "OBEX_CONNECTED";
        case OBEX_ABORTING:          return "OBEX_ABORTING";
        case OBEX_GETTING:           return "OBEX_GETTING";
        case OBEX_PUTTING:           return "OBEX_PUTTING";
        case OBEX_SETTING_PATH:      return "OBEX_SETTING_PATH";
        case OBEX_BULK_PUTTING:      return "OBEX_BULK_PUTTING";
        case OBEX_DO_ACTION:         return "OBEX_DO_ACTION";
        default:                     return "Invalid";
    }
}

static const OI_CHAR* AbortStateTxt(ABORT_STATE state)
{
    switch (state) {
        case NO_ABORT:      return "NO_ABORT";
        case ABORT_PENDING: return "ABORT_PENDING";
        case ABORT_SENT:    return "ABORT_SENT";
        default:            return "Invalid";
    }
}

#endif


typedef enum {
    CONFIRM_COMPLETED,
    CONFIRM_ALL
} CONFIRM_FILTER;


/*
 * Maximum number of buffers we will confirm at one time
 */
#define MAX_CONFIRM 16


/*
 * Determines if all bulk puts can be safely confirmed.
 */
static OI_BOOL CanConfirmAllBulkPuts(OBEXCLI_CONNECTION *connection)
{
    OBEX_BULK_DATA_LIST bulkData = connection->CB.bulkPut.head;
    while (bulkData) {
        /*
         * A bulk data block can be confirmed to the upper layer if all the bytes sent to the lower
         * layer have been confirmed by the lower layer.
         */
        if (bulkData->bytesConfirmed < bulkData->bytesSent) {
            return FALSE;
        }
        bulkData = bulkData->next;
    }
    return TRUE;
}


/*
 * Called to confirm BulkPut operations.
 * Returns TRUE if all the blocks were confirmed.
 */
static OI_BOOL ConfirmBulkPuts(OBEXCLI_CONNECTION *connection,
                               OI_STATUS status,
                               CONFIRM_FILTER  filter)
{
    OI_STATUS cfmStatus = OI_OK;
    OBEX_BULK_DATA_LIST bulkData = connection->CB.bulkPut.head;
    OI_UINT cfmCount = 0;
    OI_UINT8 *cfmBuffers[MAX_CONFIRM];
    OI_UINT32 cfmLengths[MAX_CONFIRM];

    OI_DBGPRINT2(("ConfirmBulkPuts %d %s", status, (filter == CONFIRM_ALL) ? "ALL" : "COMPLETED"));

    while (bulkData) {
        OI_DBGPRINT2(("bulkData %#08x sent:%d confirmed:%d final:%d",
            bulkData, bulkData->bytesSent, bulkData->bytesConfirmed,
            bulkData->final));
        /*
         * Stored error status overrides the one passed in.
         */
        if (!OI_SUCCESS(connection->CB.bulkPut.status)) {
            cfmStatus = connection->CB.bulkPut.status;
        } else {
            cfmStatus = status;
        }
        /*
         * If only confirming completed blocks return if this block is incomplete
         */
        if ((filter == CONFIRM_COMPLETED) && (bulkData->bytesConfirmed < bulkData->blockSize)) {
            /*
             * Save the error for use with the remaining confirm callbacks
             */
            connection->CB.bulkPut.status = (cfmStatus == OI_OBEX_CONTINUE) ? OI_OK : cfmStatus;
            break;
        }
        if (bulkData->final) {
            /*
             *  Final block needs an OI_OK to confirm it.
             */
            if (cfmStatus == OI_OBEX_CONTINUE) {
                break;
            }
        } else {
            /*
             * Not the final block so unless there is an error use the CONTINUE status
             */
            if (OI_SUCCESS(cfmStatus)) {
                cfmStatus = OI_OBEX_CONTINUE;
            }
        }
        cfmBuffers[cfmCount] = bulkData->blockBuffer;
        cfmLengths[cfmCount] = bulkData->blockSize;
        connection->CB.bulkPut.head = bulkData->next;
        /*
         * If there is no head there is no tail
         */
        if (!connection->CB.bulkPut.head) {
            connection->CB.bulkPut.tail = NULL;
            connection->CB.bulkPut.current = NULL;
        }
        OI_DBGPRINT2(("Free bulk data %#08x", bulkData));
        OI_Free(bulkData);
        bulkData = connection->CB.bulkPut.head;
        if (++cfmCount == MAX_CONFIRM) {
            OI_DBGTRACE(("ConfirmBulkPuts confirmed %d", cfmCount));
            connection->bulkPutCfm(connection->common.connectionHandle, cfmCount, cfmBuffers, cfmLengths, cfmStatus);
            cfmCount = 0;
        }
    }
    OI_DBGPRINT2(("ConfirmBulkPuts confirmed %s%d", bulkData ? "" : "all ", cfmCount));
    connection->bulkPutCfm(connection->common.connectionHandle,
        cfmCount, cfmBuffers, cfmLengths, cfmStatus);
    return (bulkData == NULL);
}


static void AbortConfirm(OBEXCLI_CONNECTION *connection)
{
    /*
     * Cleanup after we receive the abort response
     */
    switch (connection->abortingState) {
        case OBEX_BULK_PUTTING:
            if (connection->CB.bulkPut.head) {
                /*
                 * Confirm bulk put in case where there are no blocks queued
                 */
                ConfirmBulkPuts(connection, OI_OBEX_CLIENT_ABORTED_COMMAND, CONFIRM_ALL);
            } else {
                connection->bulkPutCfm(connection->common.connectionHandle, 0, NULL, NULL, connection->CB.bulkPut.status);
            }
            break;
        case OBEX_PUTTING:
            connection->CB.put.confirm(connection->common.connectionHandle, NULL, OI_OBEX_CLIENT_ABORTED_COMMAND);
            break;
        case OBEX_GETTING:
            connection->CB.get.recvData(connection->common.connectionHandle, NULL, OI_OBEX_CLIENT_ABORTED_COMMAND);
            break;
        default:
            break;
    }
    /*
     * This should be the response to the abort command.
     */
    SetAbortState(connection, NO_ABORT);
    SetState(connection, OBEX_CONNECTED);
    /*
     * Call the appropriate callback to let the application know
     * that the last GET or PUT operation was aborted.
     */
    connection->busy = FALSE;
    connection->abortConfirm(connection->common.connectionHandle);
}


/*
 * Call appropriate callback functions when a connection is dropped then free
 * the connection state information.
 *
 * For GETTING/PUTTING we check the busy flag to determine if the get/put callback
 * needs to be called.
 */
static void FreeConnection(OBEXCLI_CONNECTION *connection,
                           OI_STATUS status)
{
    CONNECTION_STATE state = connection->state;

    OI_DBGPRINT2(("FreeConnection %d", status));

    /*
     * Invalidate state in case upper layer calls back in on the callback
     * thread and attempts to use the now invalid connection handle.
     */
    SetState(connection, INVALID_CONNECTION);
    /*
     * Clear connection timeout if one is set.
     */
    if (connection->timeoutCB) {
        OI_Dispatch_CancelFunc(connection->timeoutCB);
        connection->timeoutCB = 0;
    }

    if (NULL != connection->common.mbuf) {
        OI_DBGTRACE(("Mbuf was not freed prior to disconnect"));
        OI_MBUF_Free(connection->common.mbuf);
        connection->common.mbuf = NULL;
    }
    /*
     * Let abort status override the disconnect status if an abort was requested.
     */
    if (connection->abort != NO_ABORT) {
        SetAbortState(connection, NO_ABORT);
        status = OI_OBEX_CLIENT_ABORTED_COMMAND;
    }
    /*
     * Report disconnect to application if there is an operation in progress.
     */
    switch (state) {
        case LOWER_LAYER_CONNECTING:
        case LOWER_LAYER_CONNECTED:
            /*
             * If we didn't establish  the OBEX connection we need to confirm
             * that the connect attempt failed rather than indicating a
             * disconnect.
             */
            connection->CB.connect.confirm(connection->common.connectionHandle, FALSE, status);
            /*
             * This will prevent us from indicating a disconnect.
             */
            connection->disconnectInd = NULL;
            break;
        case OBEX_GETTING:
            OI_DBGPRINTSTR(("FreeConnection getting"));
            if (connection->busy) {
                connection->CB.get.recvData(connection->common.connectionHandle, NULL, status);
            }
            if (connection->abort != NO_ABORT) {
                connection->abortConfirm(connection->common.connectionHandle);
            }
            break;
        case OBEX_BULK_PUTTING:
            /*
             * Callback all pending cfms in the order they were issued.
             */
            ConfirmBulkPuts(connection, status, CONFIRM_ALL);
            if (connection->abort != NO_ABORT) {
                connection->abortConfirm(connection->common.connectionHandle);
            }
            break;
        case OBEX_PUTTING:
            OI_DBGPRINTSTR(("FreeConnection putting"));
            if (connection->busy) {
                connection->CB.put.confirm(connection->common.connectionHandle, NULL, status);
            }
            if (connection->abort != NO_ABORT) {
                connection->abortConfirm(connection->common.connectionHandle);
            }
            break;
        case OBEX_DO_ACTION:
            connection->CB.action.confirm(connection->common.connectionHandle, status);
            break;
        case OBEX_SETTING_PATH:
            connection->CB.setpath.confirm(connection->common.connectionHandle, status);
            break;
        case OBEX_ABORTING:
            OI_DBGPRINTSTR(("FreeConnection while aborting"));
            AbortConfirm(connection);
            break;
        default:
            break;
    }
    OI_FreeIf(&connection->common.authentication);
    if (connection->disconnectInd != NULL) {
        connection->disconnectInd(connection->common.connectionHandle);
    }
    OI_HANDLE_Free(connection->common.connectionHandle);
    OI_Free(connection);
    OI_INIT_FLAG_DECREMENT(OBEX_CLI);
}


static void LowerDisconnectInd(OI_OBEX_LOWER_CONNECTION lowerConnection,
                               OI_STATUS reason)
{
    OBEXCLI_CONNECTION *connection = (OBEXCLI_CONNECTION*)lowerConnection->context;

    OI_DBGTRACE(("LowerDisconnectInd %d", reason));

    if (connection != NULL) {
        /*
         * If the lower layer connection was dropped by the server while we have an
         * operation in progress we need to report a generic error status.
         */
        if (OI_SUCCESS(reason) && (connection->state >= OBEX_CONNECTED)) {
            reason = OI_OBEX_NOT_CONNECTED;
        }
        FreeConnection(connection, reason);
    }
}


/*
 * Handle a fatal error - drop the lower connection, cleanup, and try to let
 * the application know what happened.
 */

static void FatalObexError(OBEXCLI_CONNECTION *connection,
                           OI_STATUS status)
{
    SetAbortState(connection, NO_ABORT);

    /*
     * If there is a lower layer connection disconnect it.
     */
    if (connection->state >= LOWER_LAYER_CONNECTED) {
        connection->disconnectingLowerLayer = TRUE;
        status = connection->common.lowerConnection->ifc->disconnect(connection->common.lowerConnection);
        /*
         * We are going to trust that the lower lower will complete the disconnect.
         */
        if (!OI_SUCCESS(status)) {
            OI_SLOG_ERROR(status, ("Lower layer disconnect failed"));
        }
    } else {
        /*
         * Free resources allocated for the connection.
         */
        FreeConnection(connection, status);
    }
}


/*
 * Dispatch function for asynchronously reporting an aborted commmand.
 */

static void CommandError(DISPATCH_ARG *darg)
{
    OBEXCLI_ERROR_CONTEXT ctxt;

    ctxt = Dispatch_GetArg(darg, OBEXCLI_ERROR_CONTEXT);
    FatalClientError(ctxt.connection, ctxt.status);
}


static void ReportError(OBEXCLI_CONNECTION *connection,
                        OI_STATUS status)
{
    OBEXCLI_ERROR_CONTEXT ctxt;
    DISPATCH_ARG darg;

    ctxt.connection = connection;
    ctxt.status = status;
    Dispatch_SetArg(darg, ctxt);
    OI_Dispatch_RegisterFunc(CommandError, &darg, NULL);
}


/**
 * Timeout an OBEX operation.
 */

static void ResponseTimeout(DISPATCH_ARG *darg)
{
    OBEXCLI_CONNECTION *connection = Dispatch_GetArg(darg, OBEXCLI_CONNECTION*);
    FatalClientError(connection, OI_OBEX_CONNECTION_TIMEOUT);
}


/**
 * When we start to send an obex command we expect to receive a response within
 * some reasonable timeout period.
 */

static void SetCommandTimeout(OBEXCLI_CONNECTION *connection,
                              OI_INTERVAL timeout)
{
    DISPATCH_ARG darg;

    if (OI_Dispatch_IsValidHandle(connection->timeoutCB)) {
        /*
         * Timeout has already been registered - just extend the timeout period.
         */
        OI_Dispatch_SetFuncTimeout(connection->timeoutCB, timeout);
    } else {
        /*
         * Register a new timeout.
         */
        Dispatch_SetArg(darg, connection);
        OI_Dispatch_RegisterTimedFunc(ResponseTimeout, &darg, timeout, &connection->timeoutCB);
    }
}


/**
 * Map an OBEX response to an OI_STATUS
 */

static OI_STATUS MapResponse(OI_UINT8 rspCode,
                             OI_STATUS status)
{
    switch (rspCode) {
        case OI_OBEX_FINAL(OI_OBEX_RSP_OK):
            status = OI_OK;
            break;
        case OI_OBEX_FINAL(OI_OBEX_RSP_CONTINUE):
            status = OI_OBEX_CONTINUE;
            break;
        case OI_OBEX_FINAL(OI_OBEX_RSP_BAD_REQUEST):
            status = OI_OBEX_BAD_REQUEST;
            break;
        case OI_OBEX_FINAL(OI_OBEX_RSP_FORBIDDEN):
            status = OI_OBEX_ACCESS_DENIED;
            break;
        case OI_OBEX_FINAL(OI_OBEX_RSP_NOT_FOUND):
            status = OI_OBEX_NOT_FOUND;
            break;
        case OI_OBEX_FINAL(OI_OBEX_RSP_NOT_IMPLEMENTED):
            status = OI_OBEX_NOT_IMPLEMENTED;
            break;
        case OI_OBEX_FINAL(OI_OBEX_RSP_LENGTH_REQUIRED):
            status = OI_OBEX_LENGTH_REQUIRED;
            break;
        case OI_OBEX_FINAL(OI_OBEX_RSP_SERVICE_UNAVAILABLE):
            status = OI_OBEX_SERVICE_UNAVAILABLE;
            break;
        case OI_OBEX_FINAL(OI_OBEX_RSP_NOT_ACCEPTABLE):
            status = OI_OBEX_VALUE_NOT_ACCEPTABLE;
            break;
        case OI_OBEX_FINAL(OI_OBEX_RSP_PRECONDITION_FAILED):
            status = OI_OBEX_PRECONDITION_FAILED;
            break;
        case OI_OBEX_FINAL(OI_OBEX_RSP_DATABASE_FULL):
            status = OI_OBEX_DATABASE_FULL;
            break;
        case OI_OBEX_FINAL(OI_OBEX_RSP_CONFLICT):
            status = OI_OBEX_CONFLICT;
            break;
        case OI_OBEX_FINAL(OI_OBEX_RSP_NOT_MODIFIED):
            status = OI_OBEX_NOT_MODIFIED;
            break;
        case OI_OBEX_FINAL(OI_OBEX_RSP_DATABASE_LOCKED):
            status = OI_OBEX_DATABASE_LOCKED;
            break;
        case OI_OBEX_FINAL(OI_OBEX_RSP_INTERNAL_SERVER_ERROR):
            status = OI_OBEX_INTERNAL_SERVER_ERROR;
            break;
        case OI_OBEX_FINAL(OI_OBEX_RSP_UNSUPPORTED_MEDIA_TYPE):
            status = OI_OBEX_UNSUPPORTED_MEDIA_TYPE;
            break;
        case OI_OBEX_FINAL(OI_OBEX_RSP_UNAUTHORIZED):
            status = OI_OBEX_UNAUTHORIZED;
            break;
        case OI_OBEX_FINAL(OI_OBEX_RSP_PARTIAL_CONTENT):
            status = OI_OBEX_PARTIAL_CONTENT;
            break;
        case OI_OBEX_FINAL(OI_OBEX_RSP_METHOD_NOT_ALLOWED):
            status = OI_OBEX_METHOD_NOT_ALLOWED;
            break;
    }
    return status;
}


static OI_STATUS SendBulk(OBEXCLI_CONNECTION *connection)
{
    OI_STATUS status;
    OI_BOOL final;

    OI_ASSERT(!connection->busy);

    if (!connection->CB.bulkPut.head) {
        OI_DBGTRACE(("SendBulk - nothing to send"));
        return OI_OK;
    }
    status = OI_OBEXCOMMON_SendBulk(&connection->common, &connection->CB.bulkPut.current, &connection->busy, &final);
    if (OI_SUCCESS(status)) {
        /*
         * Start a command timer if this was the last send or we are not doing SRM
         */
        if (final || !(connection->common.srm & OI_OBEX_SRM_ENABLED)) {
            SetCommandTimeout(connection, connection->responseTimeout);
        }
    } else {
        OI_DBGTRACE(("OI_OBEXCOMMON_SendBulk failed %d", status));
    }
    return status;
}


static OI_STATUS BulkPutWriteCfm(OBEXCLI_CONNECTION *connection,
                                 OI_UINT16 bytesConfirmed,
                                 OBEX_BULK_DATA_LIST bulkData,
                                 OI_BOOL queueFull,
                                 OI_STATUS status)
{
    OBEX_BULK_DATA_LIST list = bulkData;

    OI_DBGPRINT2(("BulkPutWriteCfm bytesConfirmed %d queueFull:%d %d", bytesConfirmed, queueFull, status));

    /*
     * Track progress for body data
     */
    connection->common.progressBytes += bytesConfirmed;
    /*
     * Figure out which bulk puts are being confirmed
     */
    while (bytesConfirmed) {
        OI_UINT32 max = list->bytesSent - list->bytesConfirmed;
        if (bytesConfirmed >= max) {
            bytesConfirmed -= (OI_UINT16)max;
        } else {
            max = bytesConfirmed;
        }
        list->bytesConfirmed += max;
        OI_DBGPRINT2(("bulkData %#08x blockSize:%d sent:%d confirmed:%d", list, list->blockSize, list->bytesSent, list->bytesConfirmed));
        if (list->bytesConfirmed < list->blockSize) {
            break;
        }
        list = list->next;
    }
    /*
     * Report that the operation was aborted if we are aborting the transaction.
     */
    if (connection->abort != NO_ABORT) {
        status = OI_OBEX_CLIENT_ABORTED_COMMAND;
    }
    /*
     * Previous error status overrides the one passed in
     */
    if (!OI_SUCCESS(connection->CB.bulkPut.status)) {
        status = connection->CB.bulkPut.status;
    } else {
        connection->CB.bulkPut.status = status;
    }
    /*
     * If we had an error confirm all outstanding bulk puts.
     */
    if (!OI_SUCCESS(status)) {
        /*
         * If all bulk puts are confirmed we either disconnect or abort
         */
        if (CanConfirmAllBulkPuts(connection)) {
            if (connection->abort != NO_ABORT) {
                OI_DBGTRACE(("OBEX client aborting BULK_PUT operation"));
                SendAbort(connection);
            } else {
                FatalClientError(connection, status);
            }
        }
        return OI_OK;
    }
    /*
     * If we are not doing SRM we are done for now.
     */
    if (!(connection->common.srm & OI_OBEX_SRM_ENABLED)) {
        return OI_OK;
    }
    /*
     * Confirm all completed bulk data blocks except the final block
     */
    ConfirmBulkPuts(connection, OI_OBEX_CONTINUE, CONFIRM_COMPLETED);
    /*
     * If the queue is not full we can write more data if we have any
     */
    if (!queueFull) {
        connection->busy = FALSE;
        status = SendBulk(connection);
        if (!OI_SUCCESS(status)) {
            /*
             * Cannot report an error status if there is any data queued with lower layer
             */
            if (connection->CB.bulkPut.head->bytesConfirmed < connection->CB.bulkPut.head->bytesSent) {
                connection->CB.bulkPut.status = status;
                status = OI_OK;
            }
        }
    }
    return status;
}


static void LowerWriteCfm(OI_OBEX_LOWER_CONNECTION lowerConnection,
                          OI_MBUF *mbuf,
                          OI_BOOL queueFull,
                          OI_STATUS result)
{
    OBEXCLI_CONNECTION *connection = (OBEXCLI_CONNECTION*)lowerConnection->context;
    OI_UINT16 numBytes;
    void *context = mbuf->context.v;

    if (connection == NULL) {
        OI_SLOG_ERROR(OI_OBEX_NOT_CONNECTED, ("Write confirmation %d for unknown connection", result));
        return;
    }

    numBytes = OI_MBUF_Free(mbuf);
    connection->common.mbuf = NULL;

    OI_DBGTRACE(("Write completed %d bytes: freed mbuf %#x %d", numBytes, mbuf, result));

    if (connection->state == OBEX_BULK_PUTTING) {
        #ifdef OI_TEST_HARNESS
            if (OI_ObexTest.fastAbort) {
                SendAbort(connection);
                return;
            }
        #endif
        result = BulkPutWriteCfm(connection, numBytes - BULK_PUT_HDR_SIZE, (OBEX_BULK_DATA_LIST)context, queueFull, result);
    }
    if (OI_SUCCESS(result)) {
        if (connection->common.srm & OI_OBEX_SRM_ENABLED) {
            /*
             * Report progress if the upper layer has provided a progress callback
             */
            if (connection->progressCB && connection->common.progressBytes &&
                ((connection->state == OBEX_PUTTING) || (connection->state == OBEX_BULK_PUTTING))) {
                connection->progressCB(connection->common.connectionHandle, OI_OBEX_CMD_PUT,
                    connection->common.progressBytes);
            }
        }
    } else {
        FatalClientError(connection, result);
    }
}


static OI_STATUS ClientSendPacket(OBEXCLI_CONNECTION *connection,
                                  OI_INTERVAL timeout)
{
    OI_STATUS status;
    OI_BOOL queueFull;

    OI_DBGPRINT2(("Sending mbuf %#x", connection->common.mbuf));

    status = connection->common.lowerConnection->ifc->write(connection->common.lowerConnection, connection->common.mbuf, FALSE, &queueFull);
    if (OI_SUCCESS(status)) {
        SetCommandTimeout(connection, timeout);
    } else {
        OI_MBUF_Free(connection->common.mbuf);
        connection->common.mbuf = NULL;
    }
    return status;
}


static OI_STATUS ClientSendBodySegment(OBEXCLI_CONNECTION *connection)
{
    OI_STATUS status;

    OI_DBGTRACE(("ClientSendBodySegment"));

    status = OI_OBEXCOMMON_MarshalBodySegment(&connection->common);
    if (OI_SUCCESS(status)) {
        status = ClientSendPacket(connection, connection->responseTimeout);
    }
    return status;
}


/*
 * Send an parameterless OBEX command.
 */
static OI_STATUS ClientSendCommand(OBEXCLI_CONNECTION   *connection,
                                   OI_UINT8             opcode,
                                   OI_INTERVAL          timeout,
                                   const OI_OBEX_HEADER_LIST *optHeaders)
{
    OI_STATUS status;
    OI_OBEX_HEADER headers[1];
    OI_UINT16 headerCount = 0;
    OI_BYTE_STREAM pkt;

    OI_ASSERT(OI_OBEX_IS_FINAL(opcode));

    OI_OBEXCOMMON_InitPacket(&connection->common, opcode, &pkt);
    /*
     * If a target was specified at connect time we must send the connection id
     * in the command
     */
    if (connection->cid != OI_OBEX_INVALID_CONNECTION_ID) {
        OI_OBEX_HEADER *hdr = &headers[headerCount++];
        hdr->id = OI_OBEX_HDR_CONNECTION_ID;
        hdr->val.connectionId = connection->cid;
    }
    status = OI_OBEXCOMMON_MarshalPacket(&connection->common, &pkt, headers, headerCount, optHeaders);
    if (OI_SUCCESS(status)) {
        status = ClientSendPacket(connection, timeout);
    }
    /*
     * OBEX client is busy until we confirm the put to the upper layer
     */
    connection->busy = TRUE;
    return status;
}



/**
 * Handle response to an OBEX disconnect request.
 */

static void DisconnectResponse(OBEXCLI_CONNECTION *connection,
                               OI_BYTE_STREAM *rcvPacket,
                               OI_UINT8 rspCode)
{
    OI_STATUS status;

    status = MapResponse(rspCode, OI_OBEX_DISCONNECT_FAILED);
    if (!OI_SUCCESS(status)) {
        FatalClientError(connection, status);
    } else {
        /*
         * Terminate the underlying lower layer connection
         */
        connection->disconnectingLowerLayer = TRUE;
        status = connection->common.lowerConnection->ifc->disconnect(connection->common.lowerConnection);
        if (!OI_SUCCESS(status)) {
            FatalClientError(connection, OI_OBEX_DISCONNECT_FAILED);
        }
    }
}


static void DefaultAbortCfm(OI_OBEXCLI_CONNECTION_HANDLE connectionId)
{
    OI_DBGTRACE(("Commmand aborted connection:%d", connectionId));
}


static void SendAbort(OBEXCLI_CONNECTION *connection)
{
    OI_STATUS status;

    if (connection->abort == ABORT_SENT) {
        return;
    }
    if (!COMMAND_IN_PROGRESS(connection->state)) {
        OI_DBGTRACE(("Nothing to abort"));
        return;
    }

    OI_DBGTRACE(("Sending abort command"));

    /*
     * Save the state we were in when we send the ABORT
     */
    connection->abortingState = connection->state;

    if (connection->common.srm & OI_OBEX_SRM_ENABLED) {
        /*
         * Make sure lower layer flow is enabled. We will discard any data received
         */
        connection->common.lowerConnection->ifc->flowControl(connection->common.lowerConnection, TRUE /* ON */);
    }
    SetState(connection, OBEX_ABORTING);
    /*
     * Use default abort confirm if there isn't one set.
     */
    if (!connection->abortConfirm) {
        connection->abortConfirm = DefaultAbortCfm;
    }
    status = ClientSendCommand(connection, OI_OBEX_CMD_ABORT, OBEX_ABORT_TIMEOUT, NULL);
    if (OI_SUCCESS(status)) {
        connection->busy = TRUE;
        SetAbortState(connection, ABORT_SENT);
    } else {
        /*
         * If we could not send the abort command something has gone badly
         * wrong so we need to attempt to disconnect.
         */
        OI_SLOG_ERROR(status, ("Could not send abort command"));
        FatalClientError(connection, OI_OBEX_ERROR);
    }
}



static void AppendBulkData(OBEXCLI_CONNECTION *connection,
                           OBEX_BULK_DATA_LIST bulkDataHead,
                           OBEX_BULK_DATA_LIST bulkDataTail)
{
    if (connection->CB.bulkPut.tail) {
        OI_ASSERT(!connection->CB.bulkPut.tail->final);
        connection->CB.bulkPut.tail->next = bulkDataHead;
    } else {
        OI_ASSERT(connection->CB.bulkPut.head == NULL);
        connection->CB.bulkPut.head = bulkDataHead;
    }
    connection->CB.bulkPut.tail = bulkDataTail;
    if (!connection->CB.bulkPut.current) {
        connection->CB.bulkPut.current = bulkDataHead;
    }
}


OI_STATUS OI_OBEXCLI_BulkPut(OI_OBEXCLI_CONNECTION_HANDLE connectionId,
                             OI_UINT8 numBuffers,
                             OI_UINT8 *bulkDataBuffer[],
                             OI_UINT32 bufferLength[],
                             OI_STATUS status)
{
    OI_BOOL final = (status != OI_OBEX_CONTINUE);
    OBEXCLI_CONNECTION *connection;
    OBEX_BULK_DATA_LIST bulkDataHead;
    OBEX_BULK_DATA_LIST bulkDataTail;

    /*
     * Must be connected to an OBEX server.
     */
    connection = LookupConnection(connectionId);
    if ((connection == NULL) || (connection->state < OBEX_CONNECTED)) {
        return OI_OBEX_NOT_CONNECTED;
    }
    /*
     * Check that bulk put is supported
     */
    if (!connection->bulkPutCfm) {
        return OI_STATUS_INVALID_PARAMETERS;
    }
    if (connection->abort != NO_ABORT) {
        return OI_OBEX_CLIENT_ABORTED_COMMAND;
    }
    /*
     * Unlike other operations we allow multiple bulk puts
     */
    if (connection->busy && (connection->state != OBEX_BULK_PUTTING)) {
        return OI_OBEX_OPERATION_IN_PROGRESS;
    }
    if (connection->state == OBEX_PUTTING) {
        OI_DBGTRACE(("Starting bulk put"));
        SetState(connection, OBEX_BULK_PUTTING);
        connection->CB.bulkPut.head = NULL;
        connection->CB.bulkPut.tail = NULL;
        connection->CB.bulkPut.current = NULL;
        connection->CB.bulkPut.status = (status == OI_OBEX_CONTINUE) ? OI_OK : status;
    }
    OI_DBGTRACE(("OI_OBEXCLI_BulkPut numBuffers:%d busy:%d final:%d %d", numBuffers, connection->busy, final, status));
    /*
     * The only way to terminate a PUT is to send an abort so if the application
     * called us with an error status we need to abort the PUT.
     */
    if (!OI_SUCCESS(status) && (status != OI_OBEX_CONTINUE)) {
        SetAbortState(connection, ABORT_PENDING);
        connection->CB.bulkPut.status = status;
        /*
         * If we can confirm all bulk puts send the abort otherwise the abort will be sent from BulkPutWriteCfm
         */
        if (CanConfirmAllBulkPuts(connection)) {
            OI_DBGTRACE(("OBEX client aborting BULK_PUT operation %d", status));
            SendAbort(connection);
        }
        return OI_OK;
    }
    /*
     * Must be already in a Put transaction
     */
    if (connection->state != OBEX_BULK_PUTTING) {
        OI_SLOG_ERROR(OI_STATUS_INVALID_STATE, ("BulkPut state %s", ClientStateTxt(connection->state)));
        return OI_STATUS_INVALID_STATE;
    }
    /*
     * No more puts allowed after the final one.
     */
    if (connection->CB.bulkPut.tail && connection->CB.bulkPut.tail->final) {
        OI_SLOG_ERROR(OI_STATUS_INVALID_STATE, ("BulkPut after final"));
        return OI_STATUS_INVALID_STATE;
    }
    /*
     * Unless we got an error status we require at least one buffer to send.
     */
    if (numBuffers == 0) {
        return OI_STATUS_INVALID_PARAMETERS;
    }
    /*
     * Allocate data structures to track the bulk data transfer
     */
    status = OI_OBEXCOMMON_AllocBulkData(numBuffers, bulkDataBuffer, bufferLength, &bulkDataHead, &bulkDataTail);
    if (OI_SUCCESS(status)) {
        OI_ASSERT(bulkDataTail != NULL);
        bulkDataTail->final = final;
        AppendBulkData(connection, bulkDataHead, bulkDataTail);
        /*
         * Queue bulk data buffers if the lower layer is busy
         */
        if (!connection->busy) {
            status = SendBulk(connection);
        } else {
            OI_DBGPRINT2(("OI_OBEXCLI_BulkPut queuing bulk data block"));
        }
    }
    /*
     * Initiate a disconnect if the bulk put operation failed internally.
     */
    if (!OI_SUCCESS(status)) {
        OI_DBGTRACE(("OI_OBEXCLI_BulkPut failed: %d letting disconnect handle cleanup", status));
        FatalClientError(connection, status);
    }
    return OI_OK;
}


/**
 * Called by the application to put data.
 */

OI_STATUS OI_OBEXCLI_Put(OI_OBEXCLI_CONNECTION_HANDLE connectionId,
                         OI_OBEX_HEADER_LIST const *cmdHeaders,
                         OI_OBEXCLI_PUT_CFM putCfm,
                         OI_STATUS status)
{
    OI_OBEX_HEADER headers[2];
    OI_UINT16 headerCount = 0;
    OI_BYTE_STREAM pkt;
    OI_OBEX_HEADER *hdr;
    OBEXCLI_CONNECTION *connection;
    OI_UINT8 cmd = (status == OI_OBEX_CONTINUE) ? OI_OBEX_CMD_PUT : OI_OBEX_FINAL(OI_OBEX_CMD_PUT);

    OI_DBGTRACE(("OI_OBEXCLI_Put connectionId:%x %d cmdHeaders:%d=",
                 connectionId, status, cmdHeaders ? cmdHeaders->list : NULL, cmdHeaders ? cmdHeaders->count : 0));

    OI_ARGCHECK(putCfm != NULL);
    /*
     * Must be connected to an OBEX server.
     */
    connection = LookupConnection(connectionId);
    if ((connection == NULL) || (connection->state < OBEX_CONNECTED)) {
        OI_DBGTRACE(("Invalid OBEX connection %s", connection ? ClientStateTxt(connection->state) : "NULL"));
        return OI_OBEX_NOT_CONNECTED;
    }
    if (connection->abort != NO_ABORT) {
        return OI_OBEX_CLIENT_ABORTED_COMMAND;
    }
    /*
     * Only one operation at a time on each connection.
     */
    if (connection->busy || ((connection->state != OBEX_CONNECTED) && (connection->state != OBEX_PUTTING))) {
        OI_DBGTRACE(("OBEX client PUT: another operation %s is in progress", ClientStateTxt(connection->state)));
        return OI_OBEX_OPERATION_IN_PROGRESS;
    }
    /*
     * Save callback function.
     */
    connection->CB.put.confirm = putCfm;
    /*
     * The only way to terminate a PUT is to send an abort so if the application
     * called us with an error status we need to abort the put.
     */
    if (!OI_SUCCESS(status) && (status != OI_OBEX_CONTINUE)) {
        OI_SLOG_ERROR(status, ("OBEX client terminating PUT operation"));
        SendAbort(connection);
        return OI_OK;
    }

    OI_DBGTRACE(("OBEX client issuing PUT request"));

    if (NULL == cmdHeaders) {
        return OI_STATUS_INVALID_PARAMETERS;
    }

    OI_DBGPRINT2(("OBEX client issuing %s PUT request", OI_SUCCESS(status) ? "final" : "continue"));

    OI_OBEXCOMMON_InitPacket(&connection->common, cmd, &pkt);

    /*
     * If a target was specified at connect time we must send the connection id
     * in the first PUT packet.
     */
    if (connection->state == OBEX_CONNECTED) {
        connection->srmRequested = FALSE;
        if (connection->cid != OI_OBEX_INVALID_CONNECTION_ID) {
            hdr = &headers[headerCount++];
            hdr->id = OI_OBEX_HDR_CONNECTION_ID;
            hdr->val.connectionId = connection->cid;
        }
        /*
         * SRM is not enabled unless the server responds with SRM_ENABLED
         */
        connection->common.srm &= ~OI_OBEX_SRM_ENABLED;
        SetState(connection, OBEX_PUTTING);
        connection->common.progressBytes = 0;
    }
    /*
     * If SRM is supported for this connection request that it be enabled for this PUT
     */
    if (!connection->srmRequested && (connection->common.srm & OI_OBEX_SRM_SUPPORTED)) {
        hdr = &headers[headerCount++];
        hdr->id = OI_OBEX_HDR_SINGLE_RESPONSE_MODE;
        hdr->val.srmParam = OI_OBEX_SRM_ENABLED;
        connection->srmRequested = TRUE;
        OI_DBGTRACE(("OBEX client requesing SRM for this PUT"));
    }
    status = OI_OBEXCOMMON_MarshalPacket(&connection->common, &pkt, headers, headerCount, cmdHeaders);
    if (OI_SUCCESS(status)) {
        status = ClientSendPacket(connection, connection->responseTimeout);
    }
    if (!OI_SUCCESS(status)) {
        goto PutReqError;
    }
    /*
     * OBEX client is busy until we confirm the put to the upper layer
     */
    connection->busy = TRUE;
    return OI_OK;

PutReqError:

    OI_SLOG_ERROR(status, ("OBEXCLI_PutReq"));
    SetState(connection, OBEX_CONNECTED);

    return status;
}


/**
 * Request data from an OBEX server.
 */

OI_STATUS OI_OBEXCLI_Get(OI_OBEXCLI_CONNECTION_HANDLE connectionId,
                         OI_OBEX_HEADER_LIST const *cmdHeaders,
                         OI_OBEXCLI_GET_RECV_DATA getRecvData,
                         OI_BOOL final)
{
    OI_OBEX_HEADER headers[2];
    OI_UINT16 headerCount = 0;
    OI_UINT8 cmd = (final) ? OI_OBEX_FINAL(OI_OBEX_CMD_GET) : OI_OBEX_CMD_GET;
    OI_OBEX_HEADER *hdr;
    OI_BYTE_STREAM pkt;
    OI_STATUS status;
    OBEXCLI_CONNECTION *connection;

    OI_DBGTRACE(("OI_OBEXCLI_Get connectionId:%x final:%d cmdHeaders:%x count:%d",
                 connectionId, final, cmdHeaders ? cmdHeaders->list : NULL,
                 cmdHeaders ? cmdHeaders->count : 0));

    OI_ARGCHECK(getRecvData != NULL);

    /*
     * Must be connected to an OBEX server.
     */
    connection = LookupConnection(connectionId);
    if ((connection == NULL) || (connection->state < OBEX_CONNECTED)) {
        OI_DBGTRACE(("Invalid OBEX connection %s", connection ? ClientStateTxt(connection->state) : "NULL"));
        return OI_OBEX_NOT_CONNECTED;

    }
    /*
     * Check we are in the correct state for a GET request.
     */
    if (connection->busy || ((connection->state != OBEX_CONNECTED) && (connection->state != OBEX_GETTING))) {
        OI_DBGTRACE(("OBEX client GET: another operation is in progress"));
        return OI_OBEX_OPERATION_IN_PROGRESS;
    }
    /*
     * In SRM mode we turn flow off until we get this request from the upper layer so we
     * need to turn flow on again.
     */
    if (connection->common.srm & OI_OBEX_SRM_ENABLED) {
        connection->common.lowerConnection->ifc->flowControl(connection->common.lowerConnection, TRUE /* ON */);
    }
    /*
     *  If the current operation is being aborted silently ignore the GET request.
     */
    if (connection->abort != NO_ABORT) {
        return OI_OK;
    }
    /*
     * Save the callback function.
     */
    connection->CB.get.recvData = getRecvData;
    /*
     * In SRM we don't send a GET command unless the server requested one in the response packet.
     */
    if (final && (connection->state == OBEX_GETTING) && (connection->common.srm & OI_OBEX_SRM_ENABLED)) {
        OI_DBGTRACE(("OBEX client doing SRM - GET supressed"));
        /*
         * Set a timeout in case the server goes unresponsive
         */
        SetCommandTimeout(connection, connection->responseTimeout);
        return OI_OK;
    }

    OI_DBGTRACE(("OBEX client issuing GET request"));

    OI_OBEXCOMMON_InitPacket(&connection->common, cmd, &pkt);
    /*
     * If a target was specified at connect time we must send the connection id
     * in the first get packet.
     */
    if (connection->state == OBEX_CONNECTED) {
        connection->srmRequested = FALSE;
        if (connection->cid != OI_OBEX_INVALID_CONNECTION_ID) {
            hdr = &headers[headerCount++];
            hdr->id = OI_OBEX_HDR_CONNECTION_ID;
            hdr->val.connectionId = connection->cid;
        }
        /*
         * SRM is not enabled unless the server responds with SRM_ENABLED
         */
        connection->common.srm &= ~OI_OBEX_SRM_ENABLED;
        SetState(connection, OBEX_GETTING);
        connection->common.progressBytes = 0;
    }
    /*
     * If SRM is supported for this connection request that it be enabled for this GET. Note that we
     * don't do this until the GET is final.
     */
    if (final && !connection->srmRequested && (connection->common.srm & OI_OBEX_SRM_SUPPORTED)) {
        hdr = &headers[headerCount++];
        hdr->id = OI_OBEX_HDR_SINGLE_RESPONSE_MODE;
        hdr->val.srmParam = OI_OBEX_SRM_ENABLED;
        /* Reset SRMP Headers on new Get Request */
        connection->common.srmpValid = false;
        connection->common.srmpWaitReceived = false;
        connection->srmRequested = TRUE;
        OI_DBGTRACE(("OBEX client requesing SRM for this GET"));
    }
    status = OI_OBEXCOMMON_MarshalPacket(&connection->common, &pkt, headers, headerCount, cmdHeaders);
    if (OI_SUCCESS(status)) {
        status = ClientSendPacket(connection, connection->responseTimeout);
    }
    if (!OI_SUCCESS(status)) {
        goto GetReqError;
    }
    /*
     * OBEX client is busy until we receive data from the remote peer.
     */
    connection->busy = TRUE;
    return OI_OK;

GetReqError:

    OI_SLOG_ERROR(status, ("OBEXCLI_GetReq"));
    SetState(connection, OBEX_CONNECTED);

    return status;

}


/**
 * Handle response to an OBEX setpath command
 */

static void SetpathResponse(OBEXCLI_CONNECTION *connection,
                            OI_BYTE_STREAM *rcvPacket,
                            OI_UINT8 rspCode)
{
    OI_STATUS           status;
    OI_OBEX_HEADER_LIST headers = { 0, 0 };

    /*
     * For interop - some non-compliant implementations return a CREATED
     * response instead of an OK response if a new folder was created.
     */
    if (rspCode == OI_OBEX_FINAL(OI_OBEX_RSP_CREATED)) {
        rspCode = OI_OBEX_FINAL(OI_OBEX_RSP_OK);
    }

    status = MapResponse(rspCode, OI_OBEX_ERROR);
    if (status == OI_OBEX_NOT_FOUND) {
        /*
         * Refine error message.
         */
        status = OI_OBEX_NO_SUCH_FOLDER;
    }

    OI_DBGTRACE(("OBEX client response to SETPATH %#x %d", rspCode, status));

    SetState(connection, OBEX_CONNECTED);
    /*
     * Save headers in common struct in case callback wants to fetch raw headers
     */
    OI_OBEXCOMMON_ParseHeaderList(&connection->common, &headers, rcvPacket);
    connection->common.pRawHeaders = &headers;
    /*
     * Confirm to the application that the path has been set or report an error.
     */
    connection->busy = FALSE;
    connection->CB.setpath.confirm(connection->common.connectionHandle, status);
    connection->common.pRawHeaders = NULL;
    OI_FreeIf(&headers.list);
}


/**
 * Handle response to an OBEX action command
 */

static void ActionResponse(OBEXCLI_CONNECTION *connection,
                           OI_BYTE_STREAM *rcvPacket,
                           OI_UINT8 rspCode)
{
    OI_STATUS           status = MapResponse(rspCode, OI_OBEX_ERROR);
    OI_OBEX_HEADER_LIST headers = { 0, 0 };

    OI_DBGTRACE(("OBEX client response to ACTION %d", status));

    SetState(connection, OBEX_CONNECTED);

    /*
     * Save headers in common struct in case callback wants to fetch raw headers
     */
    OI_OBEXCOMMON_ParseHeaderList(&connection->common, &headers, rcvPacket);
    connection->common.pRawHeaders = &headers;
    /*
     * Confirm to the application that the action has been completed or report an error.
     */
    connection->busy = FALSE;
    connection->CB.action.confirm(connection->common.connectionHandle, status);
    connection->common.pRawHeaders = NULL;
    OI_FreeIf(&headers.list);
}


/*
 * Handle response to an OBEX bulk put request
 */
static void BulkPutResponse(OBEXCLI_CONNECTION *connection,
                            OI_BYTE_STREAM *rcvPacket,
                            OI_UINT8 rspCode)
{
    OI_STATUS status;
    OI_OBEX_HEADER *srmHdr;
    OI_OBEX_HEADER *srmpHdr;
    OI_OBEX_HEADER_LIST headers;

    OI_DBGTRACE(("OBEX client response to bulk PUT request %d", rspCode));

    /*
     * Map the OBEX response into a OI_STATUS code.
     */
    status = OI_OBEXCOMMON_ParseHeaderList(&connection->common, &headers, rcvPacket);
    if (OI_SUCCESS(status)) {
        status = MapResponse(rspCode, OI_OBEX_PUT_RESPONSE_ERROR);
    }

    /*
     * Check if we are aborting this PUT
     */
    if (connection->abort != NO_ABORT) {
        /*
         * If we cannot confirm all data blocks the abort will be handed in BulkPutWriteCfm
         */
        if ((connection->abort == ABORT_PENDING) && CanConfirmAllBulkPuts(connection)) {
            OI_DBGTRACE(("Aborting BulkPUT %s", AbortStateTxt(connection->abort)));
            /*
             * If the PUT is complete we don't send an abort we just call the abort callback
             */
            if (connection->state == OBEX_BULK_PUTTING) {
                SendAbort(connection);
            } else {
                ConfirmBulkPuts(connection, OI_OBEX_CLIENT_ABORTED_COMMAND, CONFIRM_ALL);
                SetState(connection, OBEX_CONNECTED);
                SetAbortState(connection, NO_ABORT);
                connection->busy = FALSE;
                connection->abortConfirm(connection->common.connectionHandle);
            }
        }
        return;
    }
    srmHdr = OI_OBEX_FindHeader(&headers, OI_OBEX_HDR_SINGLE_RESPONSE_MODE);
    if (srmHdr) {
        if (srmHdr->val.srm == OI_OBEX_SRM_ENABLED) {
            connection->common.srm |= OI_OBEX_SRM_ENABLED;
            OI_DBGTRACE(("SRM enabled for this PUT"));
            /*
             * We don't want to pass the SRM header to the upper-layer.
             */
            OI_OBEXCOMMON_DeleteHeaderFromList(&headers, OI_OBEX_HDR_SINGLE_RESPONSE_MODE);
        }
    }
    srmpHdr = OI_OBEX_FindHeader(&headers, OI_OBEX_HDR_SINGLE_RESPONSE_PARAMETERS);
    if (srmpHdr) {
        if (srmHdr) {
            /* SRMP Header should be sent with first Get Response, otherwise it is invalid */
            OI_DBGTRACE(("SRMP parameter is %d, srm = %d",
                srmpHdr->val.srmParam, connection->common.srm));
            /* SRM Param received in first response, set SRMP as valid */
            connection->common.srmpValid = true;
            /* If client has set SRMP param to wait, we should disable SRM if enabled*/
            if (srmpHdr->val.srmParam == OI_OBEX_SRM_PARAM_WAIT &&
                connection->common.srm & OI_OBEX_SRM_ENABLED) {
                connection->common.srmpWaitReceived = true;
                connection->common.srm &= ~OI_OBEX_SRM_ENABLED;
                OI_DBGTRACE(("srmpWaitReceived %d, srm = %d",
                    connection->common.srmpWaitReceived, connection->common.srm));
            }
        } else {
            /* Remote is sending SRMP header in subsequent OBEX response */
            OI_DBGTRACE(("SRMP valid %d", connection->common.srmpValid));
            if (connection->common.srmpValid) {
                if (srmpHdr->val.srmParam == OI_OBEX_SRM_PARAM_WAIT) {
                    if (!connection->common.srmpWaitReceived) {
                        /* If client has set SRMP param to wait, we should disable SRM
                         * if enabled */
                        connection->common.srmpWaitReceived = true;
                        connection->common.srm &= ~OI_OBEX_SRM_ENABLED;
                    }
                } else if (srmpHdr->val.srmParam == OI_OBEX_SRM_PARAM_RSVP) {
                    /* If client has set SRMP param to remove wait, we should enable SRM
                     * if disabled */
                    if (connection->common.srmpWaitReceived) {
                        connection->common.srm |= OI_OBEX_SRM_ENABLED;
                        connection->common.srmpWaitReceived = false;
                    }
                }
                OI_DBGTRACE(("srmpWaitReceived %d, srm = %d",
                    connection->common.srmpWaitReceived, connection->common.srm));
            }
        }
        /*
         * We don't want to pass the SRMP header to the upper-layer.
         */
        OI_OBEXCOMMON_DeleteHeaderFromList(&headers, OI_OBEX_HDR_SINGLE_RESPONSE_PARAMETERS);
    } else {
        /* Re-enable SRM mode if disabled because of remote device has sent response without
         * SRMP Header */
        if (connection->common.srmpWaitReceived) {
            OI_DBGTRACE(("Re-enabling SRM as remote removed wait"));
            connection->common.srm |= OI_OBEX_SRM_ENABLED;
            connection->common.srmpWaitReceived = false;
            ConfirmBulkPuts(connection, status, CONFIRM_COMPLETED);
            connection->busy = FALSE;
            SendBulk(connection);
            return;
        }
    }    /*
     * Set busy flag to force any bulk puts during the confirm callback to be queued.
     */
    connection->busy = TRUE;

    /*
     * Potentially unsolicited results on a Bulk Put should abort the connection.
     * However, because this is may not be an explicit remote confirmation to a
     * truely sent block, clean-up should proceed normally.
     */
    if (!OI_SUCCESS(status) &&
        ((connection->common.srm & OI_OBEX_SRM_ENABLED) ||
         (status != OI_OBEX_CONTINUE))) {
        /*
         * In order to pass PTS TC_CLIENT_BCE_BV_06_I and TC_CLIENT_BCE_BV_07_I, we must not
         * disconnect if response is FORBIDDEN (aka OI_OBEX_ACCESS_DENIED)
         * Before we make this exception, we must make sure that we will be able to confirm
         * all transmits pending, otherwise that somehow leads to errant pointer crashes.
         */
        if ((status == OI_OBEX_ACCESS_DENIED) && CanConfirmAllBulkPuts(connection)) {
            OI_DBGPRINT2(("Received forbidden response, bulk puts will be confirmed"));
        } else {
            /*
             * Save error to report back to upper layers if prior error didn't exist.
             */
            if (OI_SUCCESS(connection->CB.bulkPut.status)) {
                connection->CB.bulkPut.status = status;
            }
            OI_SLOG_ERROR(status, ("Remote error during bulk PUT"));
            FatalClientError(connection, status);
            return;
        }
    } else {
        if (!(connection->common.srm & OI_OBEX_SRM_SUPPORTED)) {
            /*
             * Report progress if the upper layer has provided a progress callback
             */
            if (connection->progressCB && connection->common.progressBytes &&
                ((connection->state == OBEX_PUTTING) || (connection->state == OBEX_BULK_PUTTING))) {
                connection->progressCB(connection->common.connectionHandle, OI_OBEX_CMD_PUT,
                    connection->common.progressBytes);
            }
        }
    }

    if (ConfirmBulkPuts(connection, status, CONFIRM_COMPLETED)) {
        /*
         * We are done when the status is OI_OK or an ERROR and all blocks are confirmed
         */
        if (status != OI_OBEX_CONTINUE) {
            SetState(connection, OBEX_CONNECTED);
            connection->busy = FALSE;
            return;
        }
    } else {
        /*
         * We expect to continue if there are unconfirmed blocks
         */
        if (status != OI_OBEX_CONTINUE) {
            SetState(connection, OBEX_CONNECTED);
            if (OI_SUCCESS(status)) {
                status = OI_OBEX_ERROR;
            }
            connection->busy = FALSE;
            connection->CB.bulkPut.status = OI_OBEX_ERROR;
            return;
        }
    }
    /*
     * Now we can send any queued bulk puts
     */
    connection->busy = FALSE;
    SendBulk(connection);
}


/*
 * Handle response to an OBEX put request.
 */
static void PutResponse(OBEXCLI_CONNECTION *connection,
                        OI_BYTE_STREAM *rcvPacket,
                        OI_UINT8 rspCode)
{
    OI_OBEX_HEADER *srmHdr;
    OI_OBEX_HEADER *srmpHdr;
    OI_OBEX_HEADER_LIST headers;
    OI_STATUS status;

    OI_DBGTRACE(("OBEX client response to PUT request %d", rspCode));

    /*
     * Map the OBEX response into a OI_STATUS code.
     */
    status = OI_OBEXCOMMON_ParseHeaderList(&connection->common, &headers, rcvPacket);
    if (OI_SUCCESS(status)) {
        status = MapResponse(rspCode, OI_OBEX_PUT_RESPONSE_ERROR);
    }
    /*
     * No longer putting if the status is not CONTINUE.
     */
    if (status != OI_OBEX_CONTINUE) {
        SetState(connection, OBEX_CONNECTED);
    }
    /*
     * Check if we are aborting this PUT
     */
    if (connection->abort != NO_ABORT) {
        if (connection->abort == ABORT_PENDING) {
            OI_DBGTRACE(("Aborting PUT %s", AbortStateTxt(connection->abort)));
            /*
             * If the PUT is complete we don't send the abort we just call the abort callback
             */
            if (connection->state == OBEX_PUTTING) {
                SendAbort(connection);
            } else {
                SetState(connection, OBEX_CONNECTED);
                SetAbortState(connection, NO_ABORT);
                connection->busy = FALSE;
                connection->CB.put.confirm(connection->common.connectionHandle, NULL, OI_OBEX_CLIENT_ABORTED_COMMAND);
                connection->abortConfirm(connection->common.connectionHandle);
            }
        }
        return;
    }

    srmHdr = OI_OBEX_FindHeader(&headers, OI_OBEX_HDR_SINGLE_RESPONSE_MODE);
    if (srmHdr) {
        if (srmHdr->val.srm == OI_OBEX_SRM_ENABLED) {
            connection->common.srm |= OI_OBEX_SRM_ENABLED;
            OI_DBGTRACE(("SRM enabled for this PUT"));
            /*
             * We don't want to pass the SRM header to the upper-layer.
             */
            OI_OBEXCOMMON_DeleteHeaderFromList(&headers, OI_OBEX_HDR_SINGLE_RESPONSE_MODE);
            #ifdef OI_TEST_HARNESS
                if (OI_ObexTest.ignoreSRM) {
                    connection->common.srm &= ~OI_OBEX_SRM_ENABLED;
                    OI_SLOG_ERROR(OI_STATUS_TEST_HARNESS, ("Ignoring SRM!!!"));
                }
            #endif
        }
    }
    srmpHdr = OI_OBEX_FindHeader(&headers, OI_OBEX_HDR_SINGLE_RESPONSE_PARAMETERS);
    if (srmpHdr) {
        if (srmHdr) {
            /* SRMP Header should be sent with first Get Response, otherwise it is invalid */
            OI_DBGTRACE(("SRMP parameter is %d, srm = %d",
                srmpHdr->val.srmParam, connection->common.srm));
            /* SRM Param received in first response, set SRMP as valid */
            connection->common.srmpValid = true;
            /* If client has set SRMP param to wait, we should disable SRM if enabled*/
            if (srmpHdr->val.srmParam == OI_OBEX_SRM_PARAM_WAIT &&
                connection->common.srm & OI_OBEX_SRM_ENABLED) {
                connection->common.srmpWaitReceived = true;
                connection->common.srm &= ~OI_OBEX_SRM_ENABLED;
                OI_DBGTRACE(("srmpWaitReceived %d, srm = %d",
                    connection->common.srmpWaitReceived, connection->common.srm));
            }
        } else {
            /* Remote is sending SRMP header in subsequent OBEX response */
            OI_DBGTRACE(("SRMP valid %d", connection->common.srmpValid));
            if (connection->common.srmpValid) {
                if (srmpHdr->val.srmParam == OI_OBEX_SRM_PARAM_WAIT) {
                    if (!connection->common.srmpWaitReceived) {
                        /* If client has set SRMP param to wait, we should disable SRM
                         * if enabled */
                        connection->common.srmpWaitReceived = true;
                        connection->common.srm &= ~OI_OBEX_SRM_ENABLED;
                    }
                } else if (srmpHdr->val.srmParam == OI_OBEX_SRM_PARAM_RSVP) {
                    /* If client has set SRMP param to remove wait, we should enable SRM
                     * if disabled */
                    if (connection->common.srmpWaitReceived) {
                        connection->common.srm |= OI_OBEX_SRM_ENABLED;
                        connection->common.srmpWaitReceived = false;
                    }
                }
                OI_DBGTRACE(("srmpWaitReceived %d, srm = %d",
                    connection->common.srmpWaitReceived, connection->common.srm));
            }
        }
        /*
         * We don't want to pass the SRMP header to the upper-layer.
         */
        OI_OBEXCOMMON_DeleteHeaderFromList(&headers, OI_OBEX_HDR_SINGLE_RESPONSE_PARAMETERS);
    } else {
        /* Re-enable SRM mode if disabled because of remote device has sent response without
         * SRMP Header */
        if (connection->common.srmpWaitReceived) {
            OI_DBGTRACE(("Re-enabling SRM as remote removed wait"));
            connection->common.srm |= OI_OBEX_SRM_ENABLED;
            connection->common.srmpWaitReceived = false;
        }
    }
    /*
     * Keep sending body segments if there are more to send.
     */
    if ((connection->state == OBEX_PUTTING) &&
        OI_OBEX_IS_A_BODY_HEADER(connection->common.bodySegment.id)) {
        status = ClientSendBodySegment(connection);
        if (OI_SUCCESS(status)) {
            OI_FreeIf(&headers.list);
            return;
        }
    }
    connection->busy = FALSE;
    connection->CB.put.confirm(connection->common.connectionHandle, &headers, status);
    OI_FreeIf(&headers.list);
}


/**
 * Handle response to an OBEX get request.
 */

static void GetResponse(OBEXCLI_CONNECTION *connection,
                        OI_BYTE_STREAM *rcvPacket,
                        OI_UINT8 rspCode)
{
    OI_OBEX_HEADER *srmHdr;
    OI_OBEX_HEADER *srmpHdr;
    OI_OBEX_HEADER_LIST headers;
    OI_STATUS status;

    OI_DBGTRACE(("OBEX client response to GET request %d", MapResponse(rspCode, OI_OBEX_GET_RESPONSE_ERROR)));

    status = OI_OBEXCOMMON_ParseHeaderList(&connection->common, &headers, rcvPacket);
    if (OI_SUCCESS(status)) {
        status = MapResponse(rspCode, OI_OBEX_GET_RESPONSE_ERROR);
    }
    /*
     * No longer getting if the status is not CONTINUE.
     */
    if (status != OI_OBEX_CONTINUE) {
        SetState(connection, OBEX_CONNECTED);
        SetAbortState(connection, NO_ABORT);
    }
    /*
     * Check if we have an abort pending.
     */
    if (connection->abort != NO_ABORT) {
        OI_FreeIf(&headers.list);
        SendAbort(connection);
        return;
    }
    srmHdr = OI_OBEX_FindHeader(&headers, OI_OBEX_HDR_SINGLE_RESPONSE_MODE);
    if (srmHdr) {
        if (srmHdr->val.srm == OI_OBEX_SRM_ENABLED) {
            connection->common.srm |= OI_OBEX_SRM_ENABLED;
            OI_DBGTRACE(("SRM enabled for this GET"));
            #ifdef OI_TEST_HARNESS
                if (OI_ObexTest.ignoreSRM) {
                    connection->common.srm &= ~OI_OBEX_SRM_ENABLED;
                    OI_SLOG_ERROR(OI_STATUS_TEST_HARNESS, ("Ignoring SRM!!!"));
                }
            #endif
        }
        /*
         * We don't want to pass the SRM header to the upper-layer.
         */
        OI_OBEXCOMMON_DeleteHeaderFromList(&headers, OI_OBEX_HDR_SINGLE_RESPONSE_MODE);
    }
    srmpHdr = OI_OBEX_FindHeader(&headers, OI_OBEX_HDR_SINGLE_RESPONSE_PARAMETERS);
    if (srmpHdr) {
        if (srmHdr) {
            /* SRMP Header should be sent with first Get Response, otherwise it is invalid */
            OI_DBGTRACE(("SRMP parameter is %d, srm = %d",
                srmpHdr->val.srmParam, connection->common.srm));
            /* SRM Param received in first response, set SRMP as valid */
            connection->common.srmpValid = true;
            /* If client has set SRMP param to wait, we should disable SRM if enabled*/
            if (srmpHdr->val.srmParam == OI_OBEX_SRM_PARAM_WAIT &&
                connection->common.srm & OI_OBEX_SRM_ENABLED) {
                connection->common.srmpWaitReceived = true;
                connection->common.srm &= ~OI_OBEX_SRM_ENABLED;
                OI_DBGTRACE(("srmpWaitReceived %d, srm = %d",
                    connection->common.srmpWaitReceived, connection->common.srm));
            }
        } else {
            /* Remote is sending SRMP header in subsequent OBEX response */
            OI_DBGTRACE(("SRMP valid %d", connection->common.srmpValid));
            if (connection->common.srmpValid) {
                if (srmpHdr->val.srmParam == OI_OBEX_SRM_PARAM_WAIT) {
                    if (!connection->common.srmpWaitReceived) {
                        /* If client has set SRMP param to wait, we should disable SRM
                         * if enabled */
                        connection->common.srmpWaitReceived = true;
                        connection->common.srm &= ~OI_OBEX_SRM_ENABLED;
                    }
                } else if (srmpHdr->val.srmParam == OI_OBEX_SRM_PARAM_RSVP) {
                    /* If client has set SRMP param to remove wait, we should enable SRM
                     * if disabled */
                    if (connection->common.srmpWaitReceived) {
                        connection->common.srm |= OI_OBEX_SRM_ENABLED;
                        connection->common.srmpWaitReceived = false;
                    }
                }
                OI_DBGTRACE(("srmpWaitReceived %d, srm = %d",
                    connection->common.srmpWaitReceived, connection->common.srm));
            }
        }
        /*
         * We don't want to pass the SRMP header to the upper-layer.
         */
        OI_OBEXCOMMON_DeleteHeaderFromList(&headers, OI_OBEX_HDR_SINGLE_RESPONSE_PARAMETERS);
    } else {
        /* Re-enable SRM mode if disabled because of remote device has sent response without
         * SRMP Header */
        if (connection->common.srmpWaitReceived) {
            OI_DBGTRACE(("Re-enabling SRM as remote removed wait"));
            connection->common.srm |= OI_OBEX_SRM_ENABLED;
            connection->common.srmpWaitReceived = false;
        }
    }

    /*
     * Keep sending body segments if there are more to send.
     */
    if ((connection->state == OBEX_GETTING) && OI_OBEX_IS_A_BODY_HEADER(connection->common.bodySegment.id)) {
        status = ClientSendBodySegment(connection);
        if (OI_SUCCESS(status)) {
            OI_FreeIf(&headers.list);
            return;
        }
    }
    /*
     * In SRM mode we must turn flow off until we get another get request from the upper layer.
     */
    if ((status == OI_OBEX_CONTINUE) && (connection->common.srm & OI_OBEX_SRM_ENABLED)) {
        connection->common.lowerConnection->ifc->flowControl(connection->common.lowerConnection, FALSE /* OFF */);
    }
    connection->busy = FALSE;
    connection->CB.get.recvData(connection->common.connectionHandle, &headers, status);
    OI_FreeIf(&headers.list);

    /*
     * Report progress if the upper layer has provided a progress callback and has been progress
     */
    if (connection->progressCB && connection->common.progressBytes) {
        connection->progressCB(connection->common.connectionHandle, OI_OBEX_CMD_GET, connection->common.progressBytes);
    }
}



/**
 * Handle response to an OBEX connect request.
 */

static void ConnectResponse(OBEXCLI_CONNECTION *connection,
                            OI_BYTE_STREAM *rcvPacket,
                            OI_UINT8 rspCode)
{
    OI_OBEX_HEADER_LIST headers = { 0, 0 };
    OI_OBEX_HEADER *connectionId;
    OI_OBEX_HEADER *authChallenge;
    OI_OBEX_HEADER *authResponse;
    OI_BYTE *userIdLocal = NULL;
    OI_UINT8 userIdLocalLen = 0;
    OI_BYTE *userIdRemote = NULL;
    OI_UINT8 userIdRemoteLen = 0;
    OI_BOOL notAuthorized = FALSE;
    OI_STATUS status;
    OI_UINT8 version = 0;
    OI_UINT16 maxSendPktLen = 0;

    status = MapResponse(rspCode, OI_OBEX_CONNECT_FAILED);
    if (!OI_SUCCESS(status)) {
        /*
         * Server responds with NOT_AUTHORIZED if authentication is required.
         */
        if (status == OI_OBEX_UNAUTHORIZED) {
            OI_DBGTRACE(("OBEX Client authentication requested"));
            notAuthorized = TRUE;
        } else {
            OI_DBGTRACE(("OBEX Client connection rejected %d", status));
            goto ConnectFailed;
        }
    }

    ByteStream_GetUINT8_Checked(*rcvPacket, version);
    /*
     * Due to a misreading on the specification some implementations (including
     * earlier versions of BM3) send the OBEX spec version number instead of the
     * OBEX protocol version number. For interoperability we will only check the
     * major version number.
     */
    if (OI_OBEX_MAJOR_VERSION_NUMBER(version) != OI_OBEX_MAJOR_VERSION) {
        status = OI_OBEX_UNSUPPORTED_VERSION;
        OI_SLOG_ERROR(status, ("OBEX Client connect - version mismatch %d.%d", OI_OBEX_MAJOR_VERSION_NUMBER(version), OI_OBEX_MINOR_VERSION_NUMBER(version)));
        goto ConnectFailed;
    }
    OI_DBGPRINT2(("OBEX Client connect - version %d.%d", OI_OBEX_MAJOR_VERSION_NUMBER(version), OI_OBEX_MINOR_VERSION_NUMBER(version)));

    /* We don't currently use flags -- Skipping */
    /* ByteStream_GetUINT8_Checked(rcvPacket, flags); */
    ByteStream_Skip(*rcvPacket, 1);

    ByteStream_GetUINT16_Checked(*rcvPacket, maxSendPktLen, OI_OBEX_BO);

    if (ByteStream_Error(*rcvPacket)) {
        status = OI_OBEX_INCOMPLETE_PACKET;
        goto ConnectFailed;
    }

    OI_DBGPRINT2(("OBEX Client MaxSendPktLen was: %d, now %d", connection->common.maxSendPktLen, OI_MIN(maxSendPktLen, connection->common.maxSendPktLen)));
    connection->common.maxSendPktLen = OI_MIN(connection->common.maxSendPktLen, maxSendPktLen);

    /*
     * The OBEX specification defines a minimum packet length. Reject
     * connections that do not conform to the specification.
     */
    if (connection->common.maxSendPktLen < OI_OBEX_MIN_PACKET_SIZE) {
        status = OI_OBEX_VALUE_NOT_ACCEPTABLE;
        goto ConnectFailed;
    }

    status = OI_OBEXCOMMON_ParseHeaderList(&connection->common, &headers, rcvPacket);
    if (!OI_SUCCESS(status)) {
        goto ConnectFailed;
    }
    /*
     * If the server returned an unauthorized response we cannot complete the
     * connection at this time. Instead we call back the application to request
     * a password.
     */
    if (notAuthorized) {

        connection->common.authenticating = TRUE;

        authChallenge = OI_OBEX_FindHeader(&headers, OI_OBEX_HDR_AUTHENTICATION_CHALLENGE);
        /*
         * If the server returned an unauthorized response but did not return an
         * authentication challenge, something is broken with the server.
         */
        if (authChallenge == NULL) {
            status = OI_OBEX_REQUIRED_HEADER_NOT_FOUND;
            goto ConnectFailed;
        }
        OI_DBGTRACE(("OBEX client received authentication challenge"));
        if (connection->common.authentication == NULL) {
            /*
             * If the application is not allowing authentication we
             * have to reject the connnection.
             */
            status = OI_OBEX_NOT_IMPLEMENTED;
            goto ConnectFailed;
        }
        /*
         * Save the challenge digest
         */
        OI_OBEXAUTH_SaveChallenge(&connection->common, authChallenge);
        /*
         * Request a password and, if required, user id from the application.
         */
        if (connection->challengeCB != NULL) {
            connection->busy = FALSE;
            /* Save headers in common struct in case callback wants to fetch raw headers */
            connection->common.pRawHeaders = &headers;
            if (connection->common.authentication->realm.len > 0) {
                connection->challengeCB(connection->common.connectionHandle, connection->common.authentication->userIdRequested, &connection->common.authentication->realm);
            } else {
                connection->challengeCB(connection->common.connectionHandle, connection->common.authentication->userIdRequested, NULL);
            }
            connection->common.pRawHeaders = NULL;
        }
        /*
         * We will be resending the connection request so we are done with the
         * connection response.
         */
        OI_Free(headers.list);
        return;
    }

    /*
     * Get the connection id if there was one.
     */
    connectionId = OI_OBEX_FindHeader(&headers, OI_OBEX_HDR_CONNECTION_ID);
    if (connectionId != NULL) {
        connection->cid = connectionId->val.connectionId;
    }
    /*
     * If this connection is being authenticated we should have got an
     * authentication response from the server.
     */
    if (connection->authRequired) {
        authResponse = OI_OBEX_FindHeader(&headers, OI_OBEX_HDR_AUTHENTICATION_RESPONSE);
        if (authResponse == NULL) {
            status = OI_OBEX_REQUIRED_HEADER_NOT_FOUND;
            goto ConnectFailed;
        }
        OI_DBGTRACE(("OBEX client received authentication response"));
        OI_OBEXAUTH_SaveResponse(&connection->common, authResponse);

        userIdRemoteLen = connection->common.authentication->userIdRemoteLen;
        if (userIdRemoteLen > 0) {
            userIdRemote = &connection->common.authentication->userIdRemote[0];
            OI_DBGPRINT2(("Connection includes user id %s (%d)", userIdRemote, userIdRemoteLen));
        }

        /*
         * Does the client require a user id as well as a password?
         */
        if (connection->common.authentication->userIdRequired) {
            if (userIdRemoteLen) {
                userIdLocal = connection->common.authentication->userId;
                userIdLocalLen = connection->common.authentication->userIdLen;
                OI_DBGPRINT2(("Local user id %s (%d)", userIdLocal, userIdLocalLen));

                /* Check if we have a match fot User Id */
                if (!((userIdLocalLen == userIdRemoteLen) &&
                      (OI_Strncmp((OI_CHAR*)userIdLocal, (OI_CHAR*)userIdRemote, userIdLocalLen) == 0))) {
                    OI_DBGTRACE(("Incorrect user id"));
                    status = OI_OBEX_ACCESS_DENIED;
                }
            } else {
                OI_DBGTRACE(("Incorrect user id"));
                status = OI_OBEX_ACCESS_DENIED;
            }

        }

        if (OI_SUCCESS(status)) {
            status = OI_OBEXAUTH_Authenticate(&connection->common);
        }

        if (!OI_SUCCESS(status)) {

            status = OI_OBEX_ACCESS_DENIED;

            /*
             * Reject the connection attempt.
             */
            OI_OBEXAUTH_Reset(&connection->common);
            goto ConnectFailed;
        }
    }

    /*
     * Let the application know the connection has been established.
     */
    OI_DBGTRACE(("OBEX client connection confirmed%s", connection->common.srm & OI_OBEX_SRM_ENABLED ? " SRM enabled" : ""));
    OI_DBGTRACE(("   maxRecvPktLen %d, maxSendPktLen %d",
                connection->common.maxRecvPktLen, connection->common.maxSendPktLen));

    SetState(connection, OBEX_CONNECTED);
    connection->busy = FALSE;
    /* Save headers in common struct in case callback wants to fetch raw headers */
    connection->common.pRawHeaders = &headers;
    connection->CB.connect.confirm(connection->common.connectionHandle, connection->common.readOnly, OI_OK);
    connection->common.pRawHeaders = NULL;
    /*
     * All done with the received headers.
     */
    OI_FreeIf(&headers.list);

    return;

ConnectFailed:

    OI_FreeIf(&headers.list);
    OI_SLOG_ERROR(status, ("OBEX Client connection failed"));
    /*
     * Terminate the lower layer connection
     */
    FatalClientError(connection, status);
}


static OI_STATUS SendConnectRequest(OBEXCLI_CONNECTION  *connection,
                                    OI_INTERVAL         timeout,
                                    const OI_OBEX_REALM *realm)
{
    OI_OBEX_HEADER_LIST cmdHdrs;
    OI_OBEX_HEADER headers[3];
    OI_UINT16 headerCount = 0;
    OI_BYTE_STREAM pkt;
    OI_STATUS status;

    OI_ASSERT(connection->state == LOWER_LAYER_CONNECTED);

    OI_OBEXCOMMON_InitPacket(&connection->common, OI_OBEX_CMD_CONNECT, &pkt);

    /*
     * Write the command fields.
     */
    ByteStream_PutUINT8(pkt, OI_OBEX_VERSION_NUMBER);
    ByteStream_PutUINT8(pkt, OI_OBEX_CONNECT_FLAGS);
    ByteStream_PutUINT16(pkt, connection->common.maxRecvPktLen, OI_OBEX_BO);

    OI_DBGTRACE(("OBEX Client SendConnectRequest %2x %2x %4x", OI_OBEX_VERSION_NUMBER, OI_OBEX_CONNECT_FLAGS, connection->common.maxRecvPktLen));

    /*
     * The Bluetooth GOEP specification is very vague about OBEX authentication
     * it seems to imply that authentication is always mutual and is always
     * initiated by the server. So if we are authenticating it means we are
     * doing so in response to a challenge from the server.
     */

    if (connection->authRequired) {
        /*
         * Generate the client's challenge header.
         */
        OI_DBGTRACE(("OBEX Client calling OI_OBEXAUTH_ComposeChallenge"));
        OI_OBEXAUTH_ComposeChallenge(OI_CONFIG_TABLE_GET(OBEX_CLI)->privateKey, realm, &connection->common, &headers[headerCount++]);
    }

    if (connection->common.authenticating) {
        /*
         * Respond to the server's challenge.
         */
        OI_DBGTRACE(("OBEX Client calling OI_OBEXAUTH_ComposeResponse"));
        OI_OBEXAUTH_ComposeResponse(&connection->common, &headers[headerCount++]);
    }

    cmdHdrs.list = connection->connectHdrs;
    cmdHdrs.count = connection->connectHdrCount;

    status = OI_OBEXCOMMON_MarshalPacket(&connection->common, &pkt, headers, headerCount, &cmdHdrs);
    if (OI_SUCCESS(status)) {
        status = ClientSendPacket(connection, timeout);
    }
    if (!OI_SUCCESS(status)) {
        goto ConnectReqError;
    }
    /*
     * OBEX client is busy until we get a connect response from the remote peer
     */
    connection->busy = TRUE;
    return OI_OK;

ConnectReqError:

    return status;
}


/**
 * Handle authentication data to generate authentication challenge and/or response
 * in connection request.
 */

OI_STATUS OI_OBEXCLI_Authentication(OI_OBEXCLI_CONNECTION_HANDLE    connectionId,
                                    const OI_BYTE                   *userId,
                                    OI_UINT8                        userIdLen,
                                    const OI_CHAR                   *password,
                                    const OI_OBEX_REALM             *realm)
{
    OI_STATUS status;
    OBEXCLI_CONNECTION *connection;

    OI_DBGTRACE(("OI_OBEXCLI_Authentication (connectionId = %d, userId = \"%s\", userIdLen = %d, password = \"%s\")\n",
                 connectionId, userId, userIdLen, userIdLen, password ? password : "<NULL>"));

    connection = LookupConnection(connectionId);
    if (connection == NULL) {
        return OI_STATUS_INVALID_PARAMETERS;
    }

    if (connection->busy) {
        return OI_OBEX_OPERATION_IN_PROGRESS;
    }
    /*
     * Make sure we are doing authentication.
     */
    if ((!connection->common.authenticating) && (!connection->authRequired)) {
        return OI_OBEX_METHOD_NOT_ALLOWED;
    }

    if (password == NULL) {
        /*
         * Application is abandoning the connection attempt.
         */
        OI_DBGTRACE(("Application didn't specify password. aborting connection."));
        status = OI_OBEX_CLIENT_ABORTED_COMMAND;
        goto AuthResponseError;
    }

    status = OI_OBEXAUTH_SaveAuthInfo(&connection->common, userId, userIdLen, password);
    if (!OI_SUCCESS(status)) {
        OI_SLOG_ERROR(status, ("OI_OBEXAUTH_SaveAuthInfo"));
        goto AuthResponseError;
    }

    /*
     * Re-issue the connection request with authentication information.
     */
    status = SendConnectRequest(connection, OI_CONFIG_TABLE_GET(OBEX_CLI)->responseTimeout, realm);
    if (!OI_SUCCESS(status)) {
        OI_SLOG_ERROR(status, ("SendConnectRequest"));
        goto AuthResponseError;
    }
    /*
     * OBEX client is busy until we a response from the remote peer
     */
    connection->busy = TRUE;
    return OI_OK;

AuthResponseError:

    OI_OBEXAUTH_Reset(&connection->common);
    ReportError(connection, OI_OBEX_CLIENT_ABORTED_COMMAND);

    return status;
}


/**
 * Data received from an OBEX server.
 */

static void LowerRecvDataInd(OI_OBEX_LOWER_CONNECTION lowerConnection,
                             OI_BYTE *dataBuf,
                             OI_UINT16 dataLen)
{
    OI_BYTE_STREAM rcvPacket;
    OI_UINT8 rspCode;
    OBEXCLI_CONNECTION *connection = (OBEXCLI_CONNECTION*)lowerConnection->context;

    if (connection == NULL) {
        OI_SLOG_ERROR(OI_STATUS_INVALID_HANDLE, ("OBEX received data for unknown connection"));
        return;
    }

    OI_DBGPRINT2(("OBEX client %s received %d bytes", ClientStateTxt(connection->state), dataLen));

    /*
     * Cancel the timeout function.
     */
    if (connection->timeoutCB) {
        OI_Dispatch_CancelFunc(connection->timeoutCB);
        connection->timeoutCB = 0;
    }
    /*
     * Ignore any OBEX data we receive while we are dropping the lower layer link.
     */
    if (connection->disconnectingLowerLayer) {
        return;
    }

    ByteStream_Init(rcvPacket, dataBuf, dataLen);
    ByteStream_Open(rcvPacket, BYTESTREAM_READ);
    /*
     * Get the OBEX response code and skip over the packet length.
     */
    ByteStream_GetUINT8(rcvPacket, rspCode);
    ByteStream_Skip(rcvPacket, sizeof(OI_UINT16));

    switch (connection->state) {
    case LOWER_LAYER_CONNECTED:
        ConnectResponse(connection, &rcvPacket, rspCode);
        break;
    case OBEX_DISCONNECTING:
        DisconnectResponse(connection, &rcvPacket, rspCode);
        break;
    case OBEX_BULK_PUTTING:
        BulkPutResponse(connection, &rcvPacket,  rspCode);
        break;
    case OBEX_PUTTING:
        if (connection->abort != NO_ABORT) {
            SendAbort(connection);
        } else {
            PutResponse(connection, &rcvPacket, rspCode);
        }
        break;
    case OBEX_GETTING:
        if (connection->abort != NO_ABORT) {
            SendAbort(connection);
        } else {
            GetResponse(connection, &rcvPacket, rspCode);
        }
        break;
    case OBEX_SETTING_PATH:
        SetpathResponse(connection, &rcvPacket, rspCode);
        break;
    case OBEX_DO_ACTION:
        ActionResponse(connection, &rcvPacket, rspCode);
        break;
    case OBEX_ABORTING:
        /*
         * In SRM we can expect to continue receiving packets after we sent the ABORT.
         */
        if ((rspCode != OI_OBEX_FINAL(OI_OBEX_RSP_OK)) || (dataLen > ABORT_RSP_LEN)) {
            /*
             * Ignore anything that obviously isn't an ABORT response.
             */
            OI_DBGTRACE(("Aborting - discarding packet"));
            /*
             * This is not a response to the abort so restart the timeout
             */
            SetCommandTimeout(connection, OBEX_ABORT_TIMEOUT);
            break;
        }
        AbortConfirm(connection);
        break;
    default:
        /*
         * No data expected from server in this state.
         */
        OI_SLOG_ERROR(OI_OBEX_ERROR, ("Unexpected reponse packet"));
        FatalClientError(connection, OI_OBEX_ERROR);
    }
}


/**
 * Confirmation that an lower layer link has been established to a remote OBEX
 * server. Now we set up the OBEX connection over this link.
 */

static void LowerConnectCfm(OI_OBEX_LOWER_CONNECTION lowerConnection,
                            OI_UINT16 recvMtu,
                            OI_UINT16 sendMtu,
                            OI_STATUS result)
{
    OBEXCLI_CONNECTION *connection = (OBEXCLI_CONNECTION*)lowerConnection->context;

    OI_DBGTRACE(("LowerConnectCfm %d recvMtu:%d sendMtu:%d", result, recvMtu, sendMtu));

    /*
     * We expect to find a matching entry - with no match there is
     * no callback we can call so silently return.
     */
    if (connection == NULL) {
        OI_SLOG_ERROR(OI_STATUS_INVALID_HANDLE, ("Null connection handle!"));
        return;
    }

    OI_ASSERT(connection->state == LOWER_LAYER_CONNECTING);

    if (OI_SUCCESS(result)) {
        /*
         * Maximum OBEX packet size depends on the size of the lower layer MTU
         */
        connection->common.maxRecvPktLen = OI_MIN(recvMtu, connection->common.maxRecvPktLen);
        connection->common.maxSendPktLen = sendMtu;

        SetState(connection, LOWER_LAYER_CONNECTED);

        if(!connection->authRequired) {
            /*
             * Use the default timeout on a connect request rather than application
             * specified response timeout.
             */
            result = SendConnectRequest(connection, OI_CONFIG_TABLE_GET(OBEX_CLI)->responseTimeout, NULL);
        } else {
            /*
             * don't need to check for connection->challengeCB != NULL
             * since it was done when connection->authRequired was set to a nonzero value
             */
            connection->busy = FALSE;
            connection->challengeCB(connection->common.connectionHandle, connection->common.authentication->userIdRequired, NULL);
        }
    }
    /*
     * If the lower connection failed, free the entry and propogate the error
     * status to the application via the connect confirm callback.
     */
    if (!OI_SUCCESS(result)) {
        FatalClientError(connection, result);
    }
}


/**
 * Establishes a connection to an OBEX server by creating a lower layer connection and then sending
 * an OBEX connect request. The connect confirmation callback is not called until an OBEX connect
 * response is received from the remote lower layer.
 */

OI_STATUS OI_OBEXCLI_Connect(OI_BD_ADDR *addr,
                             OI_OBEX_LOWER_PROTOCOL *lowerProtocol,
                             OI_OBEX_CONNECTION_OPTIONS *connectOptions,
                             OI_OBEXCLI_AUTHENTICATION authentication,
                             const OI_OBEX_HEADER_LIST *cmdHeaders,
                             const OI_OBEXCLI_CB_LIST *callbacks,
                             OI_OBEXCLI_CONNECTION_HANDLE *connectionHandle,
                             const OI_CONNECT_POLICY *policy)

{
    OI_STATUS status;
    OBEXCLI_CONNECTION *connection = NULL;

    OI_ARGCHECK(addr);
    OI_ARGCHECK(callbacks);
    OI_ARGCHECK(callbacks->connectCfmCB);
    OI_ARGCHECK(callbacks->disconnectIndCB);
    OI_ARGCHECK(connectionHandle);
    OI_ARGCHECK(policy);

    OI_DBGTRACE(("OI_OBEXCLI_Connect addr:%: %s authentication:%d connectionHandle:%x",
                 addr, OI_OBEX_LowerProtocolTxt(lowerProtocol), authentication, connectionHandle));

    /*
     * Allocate a connection record
     */
    connection = OI_Calloc(sizeof(OBEXCLI_CONNECTION));
    if (connection == NULL) {
        return OI_STATUS_NO_RESOURCES;
    }
    /*
     * Allocate a connection handle and associate it with the connection record
     */
    connection->common.connectionHandle = OI_HANDLE_Alloc(ObexClientHandleType, connection);
    if (!connection->common.connectionHandle) {
        OI_Free(connection);
        return OI_STATUS_NO_RESOURCES;
    }
    /*
     * Default timeout can be overriden by OI_OBEXCLI_SetResponseTimeout()
     */
    connection->responseTimeout = OI_CONFIG_TABLE_GET(OBEX_CLI)->responseTimeout;
    /*
     * The maximum packet size we can receive is specified by a configuration
     * parameter. This may need to adjusted down after the lower layer transport has been
     * configured.
     */
    connection->common.maxRecvPktLen = OI_CONFIG_TABLE_GET(OBEX_CLI)->maxPktLen;
    /*
     * Global connection state
     */
    SetState(connection, LOWER_LAYER_CONNECTING);
    connection->disconnectInd = callbacks->disconnectIndCB;
    connection->common.authenticating = FALSE;
    connection->cid = OI_OBEX_INVALID_CONNECTION_ID;
    connection->progressCB = callbacks->progressIndCB;
    connection->bulkPutCfm = callbacks->bulkPutCfm;

    /*
     * Command specific state
     */
    connection->CB.connect.confirm = callbacks->connectCfmCB;

    /*
     * An application indicates that is supports authentication by passing
     * authentication challenge callback function. If authentication is required
     * allocate memory for managing the authentication state information.
     */
    connection->authRequired = authentication;

    if (callbacks->authChallengeIndCB != NULL || authentication) {
        connection->common.authentication = OI_Calloc(sizeof(OBEX_AUTHENTICATION));
        if (connection->common.authentication == NULL) {
            status = OI_STATUS_OUT_OF_MEMORY;
            goto ConnectErrorExit;
        }

        if(authentication == OI_OBEXCLI_AUTH_USERID_AND_PASSWORD) {
            connection->common.authentication->userIdRequired = TRUE;
        }

        if (callbacks->authChallengeIndCB != NULL) {
            connection->challengeCB = callbacks->authChallengeIndCB;
        } else {
            /*
             * If client requires authentication of a server, a callback must be provided
             */
            status = OI_OBEX_BAD_REQUEST;
            goto ConnectErrorExit;
        }
    }

    /*
     * Copy the connect command headers
     */
    if (cmdHeaders == NULL) {
        connection->connectHdrCount = 0;
    } else {
        OI_ASSERT(cmdHeaders->count <= OI_OBEX_MAX_CONNECT_HDRS);
        OI_MemCopy(connection->connectHdrs, cmdHeaders->list, sizeof(OI_OBEX_HEADER) * cmdHeaders->count);
        connection->connectHdrCount = cmdHeaders->count;
    }

    /*
     * Use the requested packet length for the maximum frame size.
     */
    status = OI_OBEX_LOWER_Connect(&lowerCallbacks,
                                   addr,
                                   lowerProtocol,
                                   OI_CONFIG_TABLE_GET(OBEX_CLI)->maxPktLen,
                                   policy,
                                   &connection->common.lowerConnection);

    if (!OI_SUCCESS(status)) {
        goto ConnectErrorExit;
    }
    /*
     * Set the context on the connection handle to point to the OBEXCLI connection struct
     */
    connection->common.lowerConnection->context = connection;
    /*
     * Check if the caller wants to use SRM for this connection
     */
    if (connectOptions && connectOptions->enableSRM && (lowerProtocol->protocol == OI_OBEX_LOWER_L2CAP)) {
        connection->common.srm = OI_OBEX_SRM_SUPPORTED;
    }
    #ifdef OI_TEST_HARNESS
        OI_OBEX_TestInit();
        if (OI_ObexTest.disableSRM) {
            connection->common.srm &= ~OI_OBEX_SRM_SUPPORTED;
        }
    #endif
    /*
     * Return the connection handle
     */
    *connectionHandle = connection->common.connectionHandle;

    OI_INIT_FLAG_INCREMENT(OBEX_CLI);

    return OI_OK;

ConnectErrorExit:

    if (connection) {
        if (connection->common.connectionHandle) {
            OI_HANDLE_Free(connection->common.connectionHandle);
        }
        OI_FreeIf(&connection->common.authentication);
        OI_Free(connection);
    }

    return status;
}



/**
 * Request to disconnect a link to an OBEX server. The disconnect will complete
 * asynchronously.
 */

OI_STATUS OI_OBEXCLI_Disconnect(OI_OBEXCLI_CONNECTION_HANDLE connectionId,
                                const OI_OBEX_HEADER_LIST    *cmdHeaders)
{
    OI_STATUS status;
    OBEXCLI_CONNECTION *connection = LookupConnection(connectionId);

    OI_DBGTRACE(("OI_OBEXCLI_Disconnect (connectionId = %d)\n", connectionId));

    if ((connection == NULL) || (connection->state < OBEX_CONNECTED)) {
        return OI_STATUS_NOT_CONNECTED;
    }
    /*
     * If there is an operation in progress disconnect the lower layer otherwise disconnect cleanly
     * at the OBEX layer.
     */
    if (connection->state > OBEX_CONNECTED) {
        status = OI_OBEX_OPERATION_IN_PROGRESS;
    } else {
        status = ClientSendCommand(connection, OI_OBEX_CMD_DISCONNECT, OBEX_DISCONNECT_TIMEOUT, cmdHeaders);
    }
    if (OI_SUCCESS(status)) {
        connection->busy = TRUE;
        SetState(connection, OBEX_DISCONNECTING);
    } else {
        FatalClientError(connection, status);
    }
    return OI_OK;
}


OI_STATUS OI_OBEXCLI_SetResponseTimeout(OI_OBEXCLI_CONNECTION_HANDLE connectionId,
                                        OI_INTERVAL timeout)
{
    OBEXCLI_CONNECTION *connection;

    connection = LookupConnection(connectionId);
    if ((connection == NULL) || (connection->state < OBEX_CONNECTED)) {
        OI_DBGTRACE(("OI_OBEXCLI_SetResponseTimeout: not connected"));
        return OI_OBEX_NOT_CONNECTED;
    }
    OI_ARGCHECK(timeout > 0);
    connection->responseTimeout = timeout;
    return OI_OK;
}


OI_STATUS OI_OBEXCLI_Abort(OI_OBEXCLI_CONNECTION_HANDLE connectionId,
                           OI_OBEXCLI_ABORT_CFM         abortCfm)
{
    OBEXCLI_CONNECTION *connection;

    OI_DBGTRACE(("OI_OBEXCLI_Abort connectionId:%d", connectionId));

    connection = LookupConnection(connectionId);
    if ((connection == NULL) || (connection->state < OBEX_CONNECTED)) {
        OI_DBGTRACE(("OI_OBEXCLI_Abort: not connected"));
        return OI_OBEX_NOT_CONNECTED;
    }
    /*
     * Can only abort PUT and GET operations
     */
    if ((connection->state != OBEX_GETTING) && (connection->state != OBEX_PUTTING) && (connection->state != OBEX_BULK_PUTTING)) {
        OI_DBGTRACE(("OI_OBEXCLI_Abort: no operation to abort"));
        return OI_STATUS_INVALID_STATE;
    }
    if (connection->busy) {
        /*
         * The upper layer has called PUT or GET and is expecting a callback. We cannot send an
         * ABORT immediately so we will return a pending status and send the ABORT as soon as we
         * have called the upper layer.
         */
        OI_DBGTRACE(("Abort of %s pending", ClientStateTxt(connection->state)));
        SetAbortState(connection, ABORT_PENDING);
    } else {
        /*
         * The upper layer is between operations and is not waiting for a PUT or GET callback so we
         * send the ABORT now and will call the abort confirm callback when we get a response.
         */
        OI_DBGTRACE(("Sending abort of %s", ClientStateTxt(connection->state)));
        SendAbort(connection);
    }
    /*
     * Save callback function.
     */
    connection->abortConfirm = abortCfm;
    return OI_OK;
}


OI_STATUS OI_OBEXCLI_SetPath(OI_OBEXCLI_CONNECTION_HANDLE   connectionId,
                             OI_OBEX_UNICODE const          *folder,
                             OI_BOOL                        dontCreate,
                             OI_BOOL                        upLevel,
                             OI_OBEXCLI_SETPATH_CFM         setpathCfm,
                             const OI_OBEX_HEADER_LIST      *cmdHeaders)
{
    OI_OBEX_HEADER headers[2];
    OI_UINT16 headerCount = 0;
    OI_OBEX_HEADER *hdr;
    OI_BYTE_STREAM pkt;
    OI_STATUS status;
    OI_UINT8 flags;
    const OI_UINT8 constants = 0;
    OBEXCLI_CONNECTION *connection;
#if defined(OI_DEBUG)
    const OI_CHAR16 nullStr16[] = { '<', 'n', 'u' ,'l', 'l', '>', '\0' };
#endif

    OI_DBGTRACE(("OI_OBEXCLI_SetPath connectionId:%d folder:\"%s\" dontCreate:%d upLevel:%d",
                 connectionId, (folder && folder->str) ? folder->str : nullStr16,
                 (folder && folder->str) ? folder->len : OI_ARRAYSIZE(nullStr16) - 1,
                 dontCreate, upLevel));

    OI_ARGCHECK(setpathCfm != NULL);

    /*
     * Must be connected to an OBEX server.
     */
    connection = LookupConnection(connectionId);
    if ((connection == NULL) || (connection->state < OBEX_CONNECTED)) {
        return OI_OBEX_NOT_CONNECTED;
    }

    OI_DBGPRINTSTR(("OBEX client issuing SETPATH request"));

    if (connection->busy) {
        return OI_OBEX_OPERATION_IN_PROGRESS;
    }
    /*
     * Make sure there are no PUT or GET operations in progress.
     */
    if (connection->state != OBEX_CONNECTED) {
        return OI_OBEX_OPERATION_IN_PROGRESS;
    }
    /*
     * Cannot specify a folder name when using uplevel.
     */
 #if 0
    if (upLevel && (folder != NULL)) {
        return OI_STATUS_INVALID_PARAMETERS;
    }
#endif
    /*
     * Save the callback function.
     */
    connection->CB.setpath.confirm = setpathCfm;
    /*
     * If a target was specified at connect time we must send the connection id.
     */
    if (connection->cid != OI_OBEX_INVALID_CONNECTION_ID) {
        hdr = &headers[headerCount++];
        hdr->id = OI_OBEX_HDR_CONNECTION_ID;
        hdr->val.connectionId = connection->cid;
    }


        hdr = &headers[headerCount++];
        hdr->id = OI_OBEX_HDR_NAME;
        /*
         * An empty name header sets the path to the root folder.
         */
        if (folder == NULL) {
            hdr->val.name.str = NULL;
            hdr->val.name.len = 0;
        } else {
            hdr->val.name = *folder;
        }


    OI_OBEXCOMMON_InitPacket(&connection->common, OI_OBEX_FINAL(OI_OBEX_CMD_SET_PATH), &pkt);
    flags = 0;
    if (dontCreate) {
        flags |= OI_OBEX_SETPATH_NO_CREATE;
    }
    if (upLevel) {
        flags |= OI_OBEX_SETPATH_UP_LEVEL;
    }
    ByteStream_PutUINT8(pkt, flags);
    ByteStream_PutUINT8(pkt, constants);

    SetState(connection, OBEX_SETTING_PATH);

    status = OI_OBEXCOMMON_MarshalPacket(&connection->common, &pkt, headers, headerCount, cmdHeaders);
    if (OI_SUCCESS(status)) {
        status = ClientSendPacket(connection, connection->responseTimeout);
    }
    if (!OI_SUCCESS(status)) {
        goto SetPathError;
    }
    /*
     * OBEX client is busy until we get a response from the remote peer
     */
    connection->busy = TRUE;
    return OI_OK;

SetPathError:

    SetState(connection, OBEX_CONNECTED);

    return status;
}


OI_STATUS OI_OBEXCLI_DoAction(OI_OBEXCLI_CONNECTION_HANDLE connectionId,
                              OI_UINT8 actionId,
                              OI_OBEX_UNICODE const *objectName,
                              OI_OBEX_HEADER_LIST const *actionHdrs,
                              OI_OBEXCLI_ACTION_CFM actionCfm)
{
    OI_OBEX_HEADER headers[3];
    OI_UINT16 headerCount = 0;
    OI_OBEX_HEADER *hdr;
    OI_BYTE_STREAM pkt;
    OI_STATUS status;
    OBEXCLI_CONNECTION *connection;

    OI_DBGTRACE(("OI_OBEXCLI_SDoAction connectionId:%d actionId:%d object:\"%s\" hdrs:%d=",
        connectionId, actionId, objectName, actionHdrs->list, actionHdrs->count));


    OI_ARGCHECK(actionHdrs != NULL);
    OI_ARGCHECK(actionCfm != NULL);

    /*
     * Must be connected to an OBEX server.
     */
    connection = LookupConnection(connectionId);
    if ((connection == NULL) || (connection->state < OBEX_CONNECTED)) {
        return OI_OBEX_NOT_CONNECTED;
    }

    OI_DBGPRINTSTR(("OBEX client issuing ACTION request"));

    if (connection->busy) {
        return OI_OBEX_OPERATION_IN_PROGRESS;
    }
    /*
     * Make sure there are no PUT or GET operations in progress.
     */
    if (connection->state != OBEX_CONNECTED) {
        return OI_OBEX_OPERATION_IN_PROGRESS;
    }
    /*
     * Save the callback function.
     */
    connection->CB.action.confirm = actionCfm;
    /*
     * If a target was specified at connect time we must send the connection id first.
     */
    if (connection->cid != OI_OBEX_INVALID_CONNECTION_ID) {
        hdr = &headers[headerCount++];
        hdr->id = OI_OBEX_HDR_CONNECTION_ID;
        hdr->val.connectionId = connection->cid;
    }
    hdr = &headers[headerCount++];
    hdr->id = OI_OBEX_HDR_ACTION_ID;
    hdr->val.actionId = actionId;
    hdr = &headers[headerCount++];
    hdr->id = OI_OBEX_HDR_NAME;
    hdr->val.name = *objectName;

    OI_OBEXCOMMON_InitPacket(&connection->common, OI_OBEX_FINAL(OI_OBEX_CMD_ACTION), &pkt);
    SetState(connection, OBEX_DO_ACTION);

    status = OI_OBEXCOMMON_MarshalPacket(&connection->common, &pkt, headers, headerCount, actionHdrs);
    if (OI_SUCCESS(status)) {
        status = ClientSendPacket(connection, connection->responseTimeout);
    }
    if (!OI_SUCCESS(status)) {
        goto DoActionError;
    }
    /*
     * OBEX client is busy until we get a response from the remote peer
     */
    connection->busy = TRUE;
    return OI_OK;

DoActionError:

    SetState(connection, OBEX_CONNECTED);

    return status;
}


OI_UINT16 OI_OBEXCLI_OptimalBodyHeaderSize(OI_OBEXCLI_CONNECTION_HANDLE connectionId)
{
    OBEXCLI_CONNECTION *connection;
    OI_UINT16   optimalSize;


    /*
     * We don't know the packet length until we are connected.
     */
    connection = LookupConnection(connectionId);
    if ((connection == NULL) || (connection->state < OBEX_CONNECTED)) {
        optimalSize = 0;
    } else {
        optimalSize = connection->common.maxSendPktLen - OI_OBEX_BODY_PKT_OVERHEAD;
    }
    OI_DBGPRINT2(("OI_OBEXCLI_OptimalBodyHeaderSize (connectionId %d) is %d\n", connectionId, optimalSize));
    return optimalSize;
}


OI_STATUS OI_OBEXCLI_SetConnectionContext(OI_OBEXCLI_CONNECTION_HANDLE connectionId,
                                          void *context)
{
    OBEXCLI_CONNECTION *connection = NULL;

    OI_DBGTRACE(("OI_OBEXCLI_SetConnectionContext connectionId:%#x context:%x", connectionId, context));

    if (connectionId) {
        connection = LookupConnection(connectionId);
    }
    if (connection == NULL) {
        return OI_STATUS_NOT_FOUND;
    } else {
        connection->context = context;
        return OI_OK;
    }

}


void* OI_OBEXCLI_GetConnectionContext(OI_OBEXCLI_CONNECTION_HANDLE connectionId)
{
    OBEXCLI_CONNECTION *connection = NULL;

    OI_DBGPRINT2(("OI_OBEXCLI_GetConnectionContext connectionId:%#x", connectionId));

    if (connectionId) {
        connection = LookupConnection(connectionId);
    }
    if (connection == NULL) {
        return NULL;
    } else {
        return connection->context;
    }
}


OI_STATUS OI_OBEXCLI_GetL2capCID(OI_OBEXCLI_CONNECTION_HANDLE connectionId,
                                 OI_L2CAP_CID *cid)
{
    OI_STATUS status = OI_STATUS_NOT_FOUND;
    OBEXCLI_CONNECTION *connection = NULL;

    OI_ARGCHECK(cid != NULL);

    if (connectionId) {
        connection = LookupConnection(connectionId);
        if (connection) {
            switch (connection->state) {
            case INVALID_CONNECTION:
            case LOWER_LAYER_CONNECTING:
            case OBEX_DISCONNECTING:
                status = OI_STATUS_NOT_CONNECTED;
                break;
            default:
                *cid = connection->common.lowerConnection->ifc->getCID(connection->common.lowerConnection);
                status = OI_OK;
                break;
            }
        }
    }
    return status;
}


OI_STATUS OI_OBEXCLI_GetRawHeaderList(OI_OBEXCLI_CONNECTION_HANDLE connectionId,
                                      OI_OBEX_HEADER_LIST          **pRawHeaderList)
{
    OBEXCLI_CONNECTION *connection = LookupConnection(connectionId);

    OI_ARGCHECK(pRawHeaderList);

    if (connection == NULL) {
        return OI_OBEX_INVALID_OPERATION;
    }

    *pRawHeaderList = connection->common.pRawHeaders;

    return OI_OK;
}
/*****************************************************************************/
