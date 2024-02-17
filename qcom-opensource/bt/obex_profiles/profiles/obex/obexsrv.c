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

API for OBEX server
*/

#define __OI_MODULE__ OI_MODULE_OBEX_SRV

#include "oi_memmgr.h"
#include "oi_dispatch.h"
#include "oi_bytestream.h"
#include "oi_assert.h"
#include "oi_utils.h"
#include "oi_debug.h"
#include "oi_argcheck.h"
#include "oi_obextest.h"
#include "oi_handle.h"
#include "oi_list.h"

#include "oi_init_flags.h"
#include "oi_config_table.h"
#include "oi_bt_profile_config.h"

#include "oi_obex_lower.h"
#include "oi_obexcommon.h"
#include "oi_obexauth.h"
#include "oi_obexsrv.h"

/*
 * This is an internal state that tracks when SRM has been requested for a PUT but we haven't yet
 * actually enabled it because we need to send a reponse to the PUT request.
 */
#define OI_OBEX_SRM_REQUESTED 0x80

/*
 * An invalid SRM parameter value to indicate no parameter set.
 */
#define SRM_PARAM_NONE   255


/*
 * Number of failed authentication attempts allowed before the server drops the
 * lower layer connection.
 */

#define MAX_AUTHENTICATION_ATTEMPTS   3


/*
 * Order of the initial states matters.
 */
typedef enum {
    DISCONNECTED            = 0, /**< The connection is gone */
    LOWER_LAYER_CONNECTING  = 1, /**< A client is establishing a lower layer link to the server. */
    LOWER_LAYER_CONNECTED   = 2, /**< A client has established a lower layer link to the server. */
    OBEX_CONNECTING         = 3, /**< OBEX connection has been indicated to upper layer */
    CONNECT_COMPLETING      = 4, /**< Waiting for the connect to complete */
    OBEX_CONNECTED          = 5, /**< A OBEX connection has been established over the lower layer link. */
    GET_IN_PROGRESS,
    GET_COMPLETING,
    BULK_GET_IN_PROGRESS,
    BULK_GET_COMPLETING,
    BULK_GET_FAILED,
    PUT_IN_PROGRESS,
    PUT_COMPLETING,
    SETTING_PATH,
    DO_ACTION,
    OBEX_DISCONNECTING,          /**< Server has received a disconnect command */
    OBEX_DISCONNECT_COMPLETING   /**< Server has issued a disconnect request to the underlying layer */
} CONNECTION_STATE;


typedef struct _OBEXSRV_TARGET_INFO {
    OI_OBEX_BYTESEQ target;                 /**< Target for this connection */
    const OI_OBEXSRV_CB_LIST *CBList;       /**< Callbacks for this connection*/
    OI_OBEXSRV_AUTHENTICATION authRequired; /**< Level of authentication required on this server */
    struct _OBEXSRV_TARGET_INFO *next;
} OBEXSRV_TARGET_INFO;


typedef struct {

    void *context;                           /**< Application specific context.  */

    OI_OBEX_SERVER_HANDLE handle;            /**< Handle for this server */
    OI_LIST_ELEM connectionList;             /**< Connections using this server */
    OBEXSRV_TARGET_INFO targetInfo;          /**< Pimary target and list of any secondary targets and associated callbacks */
    OI_UINT8 srm;                            /**< SRM flags set at registration time */
    OI_OBEX_LOWER_SERVER *lowerProtocolList; /**< Null terminated list of lower layer server handles for this server */

} OBEX_SERVER;


typedef struct _OBEXSRV_CONNECTION {

    void *context;                          /**< Application specific context.  */

    OBEX_SERVER *obexServer;                /**< OBEX server for this connection */
    OI_OBEX_LOWER_SERVER lowerServer;       /**< Lower layer service for this client connection */
    OI_LIST_ELEM link;                      /**< Connection link into server connection list */

    OBEX_COMMON common;
    OI_BOOL clientSpecifiedTarget;          /**< Indicates if the client specified a target */
    OBEXSRV_TARGET_INFO *currentTarget;     /**< Pointer to currently connected target */

    DISPATCH_CB_HANDLE connectTimeout;      /**< Timeout to initiate disconnect if there is no
                                                 connect request to a multiplexed service above OBEX
                                                 after lower layer connection has been established */

    OI_BOOL  abortPending;                  /**< TRUE if an abort has been received and abort
                                                    response has not yet been sent to client */
    OI_BOOL  abortResponsePending;           /**< TRUE if abort has been signalled to callback and
                                                    server is waiting for response from application */
    OI_MBUF  *abortMbuf;                    /**< Contains abortResponse packet, may be NULL */
    OI_UINT8 authTries;                     /**< How many authentication attempts */
    OI_UINT8 forcedDisconnect;              /**< Application has forced a disconnect */
    OI_UINT8 unauthorized;                  /**< Connection in progress not authorized */
    OI_UINT8 authRequested;                 /**< Client requested authentication of the OBEX server */
    OI_UINT8 connectIndicated;              /**< Connection has been indicated to the upper-layer */
    OI_UINT8 final;                         /**< Final packet for a transaction has been received */
    CONNECTION_STATE state;

    /**
     * State for handling a GET that uses the bulk get APIs
     */
    struct {
        OI_BOOL busy;                /* TRUE if we are busy */
        OBEX_BULK_DATA_LIST current; /* Bulk data block that is currently being sent - will be partially sent */
        OBEX_BULK_DATA_LIST head;    /* Head of the bulk data list */
        OBEX_BULK_DATA_LIST tail;    /* Tailt of the bulk data list */
    } bulkGet;

    OI_BD_ADDR clientBdAddr;                /**< BDADDR of connected client */
} OBEXSRV_CONNECTION;




/***************** Forward function definition *************************/


static void LowerConnectCfm(OI_OBEX_LOWER_CONNECTION lowerConnection,
                            OI_UINT16 recvMtu,
                            OI_UINT16 sendMtu,
                            OI_STATUS result);


static void LowerConnectInd(OI_OBEX_LOWER_SERVER serverHandle,
                            OI_OBEX_LOWER_CONNECTION lowerConnection,
                            OI_BD_ADDR *addr);


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
    LowerConnectInd,
    LowerDisconnectInd,
    LowerWriteCfm,
    LowerRecvDataInd
};

static OI_STATUS sendSimpleWithHeaders(OBEX_COMMON   *common,
                                       OI_UINT8      opcode,
                                       const OI_OBEX_HEADER_LIST *headerList);

/************************************************************************/

/**
 * Type for an OBEX server connection handle
 */
static const OI_CHAR* ObexSrvConnectionHandleType = "OBEXSRV_CONNECTION";

/**
 * Type for an OBEX server handle
 */
static const OI_CHAR* ObexServerHandleType = "OBEX_SERVER";


/*
 * Get a connection struct from a connection handle
 */
#define LookupConnection(connectionHandle)  ((OBEXSRV_CONNECTION*)OI_HANDLE_Deref(connectionHandle))


#define SetState(connection, newState) \
    do { \
        OI_DBGPRINT(("State %s -> %s", ServerStateTxt((connection)->state), ServerStateTxt(newState))); \
        (connection)->state = (newState); \
    } while(0);


#ifdef OI_DEBUG
static const OI_CHAR* ServerStateTxt(CONNECTION_STATE state)
{
    switch (state) {
        case DISCONNECTED:               return "DISCONECTED";
        case LOWER_LAYER_CONNECTING:     return "LOWER_LAYER_CONNECTING";
        case LOWER_LAYER_CONNECTED:      return "LOWER_LAYER_CONNECTED";
        case OBEX_CONNECTING:            return "OBEX_CONNECTING";
        case CONNECT_COMPLETING:         return "CONNECT_COMPLETING";
        case OBEX_CONNECTED:             return "OBEX_CONNECTED";
        case GET_IN_PROGRESS:            return "GET_IN_PROGRESS";
        case GET_COMPLETING:             return "GET_COMPLETING";
        case BULK_GET_IN_PROGRESS:       return "BULK_GET_IN_PROGRESS";
        case BULK_GET_COMPLETING:        return "BULK_GET_COMPLETING";
        case BULK_GET_FAILED:            return "BULK_GET_FAILED";
        case PUT_IN_PROGRESS:            return "PUT_IN_PROGRESS";
        case PUT_COMPLETING:             return "PUT_COMPLETING";
        case SETTING_PATH:               return "SETTING_PATH";
        case DO_ACTION:                  return "DO_ACTION";
        case OBEX_DISCONNECTING:         return "OBEX_DISCONNECTING";
        case OBEX_DISCONNECT_COMPLETING: return "OBEX_DISCONNECT_COMPLETING";
        default:                         return "Invalid";
    }
}


const OI_CHAR* OI_OBEXSRV_AuthenticationText(OI_OBEXSRV_AUTHENTICATION authentication)
{
    switch (authentication) {
        case OI_OBEXSRV_AUTH_NONE:                return "NONE";
        case OI_OBEXSRV_AUTH_PASSWORD:            return "PASSWORD only";
        case OI_OBEXSRV_AUTH_USERID_AND_PASSWORD: return "USERID+PASSWORD";
        default:                                  return "Invalid";
    }
}

#else

const OI_CHAR* OI_OBEXSRV_AuthenticationText(OI_OBEXSRV_AUTHENTICATION authentication)
{
    return "";
}

#endif  /* OI_DEBUG */


/*
 * Map error status into an OBEX response code and send to the client.
 * Whatever transaction we were in has ended so clear SRM flag if it was set.
 */
#define ErrorResponse(connection, status) \
    do { \
        OI_DBGPRINT(("OBEX Server sending error response %d", status)); \
        connection->common.srm &= ~OI_OBEX_SRM_ENABLED; \
        OI_OBEXCOMMON_SendSimple(&connection->common, MapStatus(status)); \
    } while (0)

#define ErrorResponseWithHeaders(connection, status, hdrs) \
    do { \
        OI_DBGPRINT(("OBEX Server sending error response %d", status)); \
        connection->common.srm &= ~OI_OBEX_SRM_ENABLED; \
        sendSimpleWithHeaders(&connection->common, MapStatus(status), hdrs); \
    } while (0)





typedef enum {
    CONFIRM_COMPLETED,
    CONFIRM_ALL
} CONFIRM_FILTER;


/*
 * Maximum number of buffers we will confirm at one time
 */
#define MAX_CONFIRM 16

/**
 * Send a simple OBEX command packet, but append caller's optional headers.
 */
static OI_STATUS sendSimpleWithHeaders(OBEX_COMMON   *common,
                                       OI_UINT8      opcode,
                                       const OI_OBEX_HEADER_LIST *headerList)
{
    OI_BYTE_STREAM  pkt;
    OI_STATUS       status;
    OI_BOOL         queueFull;

    if ((NULL == headerList ) || (0== headerList->count)) {
        OI_DBGPRINT2(("sendSimpleWithHeaders, no headers provided"));
        return OI_OBEXCOMMON_SendSimple(common, opcode);
    }

    OI_DBGPRINT2(("sendSimpleWithHeaders, %d headers provided",
                        (NULL == headerList)? 0: headerList->count));

    OI_OBEXCOMMON_InitPacket(common, opcode, &pkt);

    status = OI_OBEXCOMMON_MarshalPacket(common, &pkt, NULL, 0, headerList);
    if (OI_SUCCESS(status)) {
        status = common->lowerConnection->ifc->write(common->lowerConnection, common->mbuf, TRUE, &queueFull);
        if (!OI_SUCCESS(status)) {
            OI_SLOG_ERROR(status, ("Lower layer write failed"));
            OI_MBUF_Free(common->mbuf);
            common->mbuf = NULL;
        }
    }

    return status;
}

/*
 *
 * Called to confirm incomplete BulkGet operations in the case of a disconnect or abort
 * Returns TRUE if all blocks are confirmed.
 */
static OI_BOOL ConfirmBulkGets(OBEXSRV_CONNECTION *connection,
                               OI_STATUS status,
                               CONFIRM_FILTER filter,
                               OI_BOOL *final)
{
    OBEX_BULK_DATA_LIST bulkData = connection->bulkGet.head;
    OI_UINT cfmCount = 0;
    OI_UINT8 *cfmBuffers[MAX_CONFIRM];
    OI_UINT32 cfmLengths[MAX_CONFIRM];
    OI_STATUS cfmStatus = (status == OI_OBEX_CONTINUE) ? status : OI_OK;

    OI_DBGPRINT(("ConfirmBulkGets %d", status));

    if (final) {
        *final = FALSE;
    }

    while (bulkData) {
        /*
         * Cannot confirm blocks that are being sent
         */
        if (bulkData->bytesConfirmed < bulkData->bytesSent) {
            break;
        }
        /*
         * If only confirming completed blocks exit if this block is incomplete
         */
        if ((filter == CONFIRM_COMPLETED) && (bulkData->bytesConfirmed < bulkData->blockSize)) {
            break;
        }
        cfmBuffers[cfmCount] = bulkData->blockBuffer;
        cfmLengths[cfmCount] = bulkData->blockSize;
        connection->bulkGet.head = bulkData->next;
        /*
         * If there is no head there is no tail
         */
        if (!connection->bulkGet.head) {
            connection->bulkGet.tail = NULL;
            connection->bulkGet.current = NULL;
        }
        OI_DBGPRINT2(("Free bulk data %#08x final:%d", bulkData, bulkData->final));
        if (final) {
            *final = bulkData->final;
        }
        OI_Free(bulkData);
        bulkData = connection->bulkGet.head;
        /*
         * Only report an error status when confirming last block
         */
        if (bulkData == NULL) {
            cfmStatus = status;
        }
        if (++cfmCount == MAX_CONFIRM) {
            OI_DBGPRINT2(("ConfirmBulkGets confirmed %d", cfmCount));
            connection->currentTarget->CBList->bulkGetInd(connection->common.connectionHandle, cfmCount, cfmBuffers, cfmLengths, cfmStatus);
            cfmCount = 0;
        }
    }
    if (cfmCount > 0) {
        OI_DBGPRINT2(("ConfirmBulkGets confirmed %s%d", bulkData ? "" : "all ", cfmCount));
        connection->currentTarget->CBList->bulkGetInd(connection->common.connectionHandle, cfmCount, cfmBuffers, cfmLengths, cfmStatus);
    }
    return (bulkData == NULL);
}


/*
 * Cleanup after a connection has terminated. This function is called during
 * normal and abnormal disconnects.
 */

static void ServerDisconnect(OBEXSRV_CONNECTION *connection,
                             OI_STATUS reason)
{
    CONNECTION_STATE state = connection->state;

    OI_DBGTRACE(("ServerDisconnect(reason=%d)\n", reason));

    SetState(connection, DISCONNECTED);

    /*
     * Stop connection timer if it is running
     */
    if (connection->connectTimeout) {
        OI_Dispatch_CancelFunc(connection->connectTimeout);
    }

    switch (state) {
        case PUT_IN_PROGRESS:
        case PUT_COMPLETING:
            connection->currentTarget->CBList->putInd(connection->common.connectionHandle, NULL, OI_OBEX_NOT_CONNECTED);
            break;
        case GET_IN_PROGRESS:
        case GET_COMPLETING:
            connection->currentTarget->CBList->getInd(connection->common.connectionHandle, NULL, OI_OBEX_NOT_CONNECTED);
            break;
        case BULK_GET_IN_PROGRESS:
        case BULK_GET_COMPLETING:
        case BULK_GET_FAILED:
            /*
             * We expect that the lower layer will have already confirmed all sends.
             */
            if (!ConfirmBulkGets(connection, reason, CONFIRM_ALL, NULL)) {
                OI_SLOG_ERROR(OI_STATUS_INVALID_STATE, ("Server disconnect with incomplete sends pending"));
            }
            break;
        default:
            break;
    }

    OI_FreeIf(&connection->common.authentication);
    /*
     * Unlink the connection from the server
     */
    OI_List_Del(&connection->link);
    /*
     * If the connection was indicated to the upper layer now indicate the disconnect
     */
    if (connection->connectIndicated) {
        connection->connectIndicated = FALSE;
        connection->currentTarget->CBList->disconnectInd(connection->common.connectionHandle);
    }
    OI_HANDLE_Free(connection->common.connectionHandle);
    OI_Free(connection);
}


static void ConnectTimeout(DISPATCH_ARG *darg)
{
    OI_OBEXSRV_CONNECTION_HANDLE connectionHandle = Dispatch_GetArg(darg, OI_OBEXSRV_CONNECTION_HANDLE);
    OBEXSRV_CONNECTION *connection = LookupConnection(connectionHandle);

    OI_SLOG_ERROR(OI_TIMEOUT, ("Connection timeout: lower layer disconnected"));

    if (connection) {
        OI_ASSERT(connection->state != OBEX_CONNECTED);
        connection->connectTimeout = 0;
        OI_OBEXSRV_ForceDisconnect(connectionHandle);
    }
}


static void LowerDisconnectInd(OI_OBEX_LOWER_CONNECTION lowerConnection,
                               OI_STATUS reason)
{
    OBEXSRV_CONNECTION *connection = (OBEXSRV_CONNECTION*)lowerConnection->context;

    OI_DBGPRINT(("LowerDisconnectInd %d", reason));

    if (connection != NULL) {
        if (connection->forcedDisconnect) {
            reason = OI_OBEX_SERVER_FORCED_DISCONNECT;
        }
        ServerDisconnect(connection, reason);
    }
}


/**
 * Map OI status into an OBEX response.
 */

static OI_UINT8 MapStatus(OI_STATUS status)
{
    OI_UINT8 rspCode;

    switch (status) {
        case OI_OK:
            rspCode = OI_OBEX_FINAL(OI_OBEX_RSP_OK);
            break;
        case OI_OBEX_ERROR:
            rspCode = OI_OBEX_FINAL(OI_OBEX_RSP_INTERNAL_SERVER_ERROR);
            break;
        case OI_OBEX_BAD_PACKET:
        case OI_OBEX_BAD_REQUEST:
            rspCode = OI_OBEX_FINAL(OI_OBEX_RSP_BAD_REQUEST);
            break;
        case OI_OBEX_OPERATION_IN_PROGRESS:
        case OI_OBEX_ACCESS_DENIED:
        case OI_OBEX_FORBIDDEN:
        case OI_STATUS_ACCESS_DENIED:
            rspCode = OI_OBEX_FINAL(OI_OBEX_RSP_FORBIDDEN);
            break;
        case OI_OBEX_NOT_FOUND:
            rspCode = OI_OBEX_FINAL(OI_OBEX_RSP_NOT_FOUND);
            break;
        case OI_OBEX_UNKNOWN_COMMAND:
        case OI_OBEX_NOT_IMPLEMENTED:
        case OI_STATUS_NOT_IMPLEMENTED:
            rspCode = OI_OBEX_FINAL(OI_OBEX_RSP_NOT_IMPLEMENTED);
            break;
        case OI_OBEX_LENGTH_REQUIRED:
            rspCode = OI_OBEX_FINAL(OI_OBEX_RSP_LENGTH_REQUIRED);
            break;
        case OI_OBEX_SERVICE_UNAVAILABLE:
            rspCode = OI_OBEX_FINAL(OI_OBEX_RSP_SERVICE_UNAVAILABLE);
            break;
        case OI_OBEX_VALUE_NOT_ACCEPTABLE:
            rspCode = OI_OBEX_FINAL(OI_OBEX_RSP_NOT_ACCEPTABLE);
            break;
        case OI_OBEX_REQUIRED_HEADER_NOT_FOUND:
        case OI_OBEX_UNSUPPORTED_VERSION:
        case OI_OBEX_INCOMPLETE_PACKET:
            rspCode = OI_OBEX_FINAL(OI_OBEX_RSP_BAD_REQUEST);
            break;
        case OI_OBEX_PRECONDITION_FAILED:
            rspCode = OI_OBEX_FINAL(OI_OBEX_RSP_PRECONDITION_FAILED);
            break;
        case OI_OBEX_DATABASE_FULL:
            rspCode = OI_OBEX_FINAL(OI_OBEX_RSP_DATABASE_FULL);
            break;
        case OI_OBEX_DATABASE_LOCKED:
            rspCode = OI_OBEX_FINAL(OI_OBEX_RSP_DATABASE_LOCKED);
            break;
        case OI_OBEX_UNSUPPORTED_MEDIA_TYPE:
            rspCode = OI_OBEX_FINAL(OI_OBEX_RSP_UNSUPPORTED_MEDIA_TYPE);
            break;
        case OI_OBEX_CONFLICT:
            rspCode = OI_OBEX_FINAL(OI_OBEX_RSP_CONFLICT);
            break;
        case OI_OBEX_UNAUTHORIZED:
            rspCode = OI_OBEX_FINAL(OI_OBEX_RSP_UNAUTHORIZED);
            break;
        case OI_OBEX_PARTIAL_CONTENT:
            rspCode = OI_OBEX_FINAL(OI_OBEX_RSP_PARTIAL_CONTENT);
            break;
        case OI_OBEX_METHOD_NOT_ALLOWED:
            rspCode = OI_OBEX_FINAL(OI_OBEX_RSP_METHOD_NOT_ALLOWED);
            break;
        case OI_OBEX_FOLDER_BROWSING_NOT_ALLOWED:
            rspCode = OI_OBEX_FINAL(OI_OBEX_RSP_UNAUTHORIZED);
            break;
        case OI_OBEX_NOT_MODIFIED:
            rspCode = OI_OBEX_FINAL(OI_OBEX_RSP_NOT_MODIFIED);
            break;
        default:
            rspCode = OI_OBEX_FINAL(OI_OBEX_RSP_INTERNAL_SERVER_ERROR);
            break;
    }
    return rspCode;
}


static OI_STATUS BulkGetWriteCfm(OBEXSRV_CONNECTION *connection,
                                 OI_UINT16 bytesConfirmed,
                                 OBEX_BULK_DATA_LIST bulkData,
                                 OI_BOOL queueFull,
                                 OI_STATUS status)
{
    OBEX_BULK_DATA_LIST list = bulkData;
    OI_STATUS continueStatus = (connection->state == BULK_GET_IN_PROGRESS) ? OI_OBEX_CONTINUE : OI_OK;

    OI_DBGPRINT(("BulkGetWriteCfm bytesConfirmed %d queueFull:%d %d", bytesConfirmed, queueFull, status));

    OI_ASSERT(bulkData->bytesConfirmed <= bulkData->bytesSent);

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
        OI_DBGPRINT2(("%#08x blockSize:%d bytesSent:%d bytesConfirmed:%d", list, list->blockSize, list->bytesSent, list->bytesConfirmed));
        if (list->bytesConfirmed < list->blockSize) {
            break;
        }
        list = list->next;
    }
    /*
     * Report that the operation was aborted if we are aborting the transaction.
     */
    if (connection->abortPending) {
        status = OI_OBEX_CLIENT_ABORTED_COMMAND;
    }
    /*
     * If we had an error confirm all outstanding bulk gets.
     */
    if (!OI_SUCCESS(status)) {
        /*
         * Try to confirm all unsent blocks
         */
        if (ConfirmBulkGets(connection, status, CONFIRM_ALL, NULL)) {
            /*
             * Send a response to the abort.  If we're waiting for application to
             * respond to abort request, keep waiting.
             */
            if (connection->abortPending && !connection->abortResponsePending) {
                OI_DBGPRINT(("Confirming abort"));
                OI_OBEXCOMMON_SendOk(&connection->common);
                connection->abortPending = FALSE;
            }
            connection->common.srm &= ~OI_OBEX_SRM_ENABLED;
            SetState(connection, OBEX_CONNECTED);
        }
        return OI_OK;
    }
    /*
     * If we are doing SRM or we are completely done confirm completed bulk data blocks otherwise
     * confirm when we receive the next GET command.
     */
    if ((connection->common.srm & OI_OBEX_SRM_ENABLED) || (continueStatus == OI_OK)) {
        OI_BOOL final;
        ConfirmBulkGets(connection, continueStatus, CONFIRM_COMPLETED, &final);
        /*
         * If we just confirmed the final bulk data block the GET is complete.
         */
        if (final) {
            OI_ASSERT(connection->bulkGet.head == NULL);
            connection->currentTarget->CBList->bulkGetInd(connection->common.connectionHandle, 0, NULL, NULL, OI_OBEX_CLEANUP);
            connection->common.srm &= ~OI_OBEX_SRM_ENABLED;
            SetState(connection, OBEX_CONNECTED);
        }
    }
    /*
     * If we are doing SRM and the queue is not full we can write data now. Otherwise we cannot
     * write data until we get a get command from the remote peer.
     */
    if (!queueFull && (connection->common.srm & OI_OBEX_SRM_ENABLED)) {
        status = OI_OBEXCOMMON_SendBulk(&connection->common, &connection->bulkGet.current, &queueFull, NULL);
        if (!OI_SUCCESS(status)) {
            OI_SLOG_ERROR(status, ("OI_OBEXCOMMON_SendBulk failed"));
            /*
             * Cannot report an error status if there is any data queued with lower layer
             */
            if (connection->bulkGet.head->bytesConfirmed < connection->bulkGet.head->bytesSent) {
                status = OI_OK;
            }
        }
        connection->bulkGet.busy = queueFull;
    }
    return status;
}


static void LowerWriteCfm(OI_OBEX_LOWER_CONNECTION lowerConnection,
                          OI_MBUF *mbuf,
                          OI_BOOL queueFull,
                          OI_STATUS result)
{
    OI_STATUS status;
    OBEXSRV_CONNECTION *connection = (OBEXSRV_CONNECTION*)lowerConnection->context;
    OI_UINT16 numBytes;
    void *context = mbuf->context.v;

    if (connection == NULL) {
        OI_SLOG_ERROR(result, ("Write confirmation for unknown connection"));
        return;
    }

    numBytes = OI_MBUF_Free(mbuf);
    if (mbuf == connection->common.mbuf) {
        connection->common.mbuf = NULL;
    } else if (mbuf == connection->abortMbuf) {
        connection->abortMbuf = NULL;
    }

    OI_DBGPRINT2(("Write completed %d bytes: freed mbuf %#x %d", numBytes, mbuf, result));

    /*
     * On an error drop the lower layer connection - it is probably already gone...
     */
    if (!OI_SUCCESS(result)) {
        lowerConnection->ifc->disconnect(lowerConnection);
    }

    /*
     * Check for operations that need to be completed.
     */
    switch (connection->state) {
    case CONNECT_COMPLETING:
        /*
         * No longer need the authentication state info.
         */
        OI_FreeIf(&connection->common.authentication);
        SetState(connection, OBEX_CONNECTED);
        OI_DBGPRINT(("Connected, maxRecvPktLen %d, maxSendPktLen %d",
                connection->common.maxRecvPktLen, connection->common.maxSendPktLen));

        /* Stop connection timer if it is running */
        if (0 != connection->connectTimeout) {
            OI_DBGPRINT2(("Cancel connect timeout"));
            OI_Dispatch_CancelFunc(connection->connectTimeout);
            connection->connectTimeout = 0;
        }
        break;
    case GET_COMPLETING:
        if (OI_OBEX_IS_A_BODY_HEADER(connection->common.bodySegment.id)) {
            /*
             * We are doing automatic body header segmentation so we are not done until the
             * entire body payload has been sent.
             */
            break;
        }
        SetState(connection, OBEX_CONNECTED);
        connection->common.srm &= ~OI_OBEX_SRM_ENABLED;
        if (OI_SUCCESS(result)) {
            result = OI_OBEX_CLEANUP;
        }
        connection->currentTarget->CBList->getInd(connection->common.connectionHandle, NULL, result);
        break;
    case PUT_COMPLETING:
        SetState(connection, OBEX_CONNECTED);
        connection->common.srm &= ~OI_OBEX_SRM_ENABLED;
        if (OI_SUCCESS(result)) {
            result = OI_OBEX_CLEANUP;
        }
        connection->currentTarget->CBList->putInd(connection->common.connectionHandle, NULL, result);
        break;
    case OBEX_DISCONNECTING:
        /*
         * Drop the lower layer connection.
         */
        SetState(connection, OBEX_DISCONNECT_COMPLETING);
        status = lowerConnection->ifc->disconnect(lowerConnection);
        if (!OI_SUCCESS(status)) {
            OI_SLOG_ERROR(status, ("Lower layer disconnect failed"));
            ServerDisconnect(connection, status);
        }
        break;
    case BULK_GET_FAILED:
        result = OI_OBEX_ERROR;
        /* Falling through */
    case BULK_GET_COMPLETING:
    case BULK_GET_IN_PROGRESS:
        BulkGetWriteCfm(connection, numBytes - BULK_GET_HDR_SIZE, (OBEX_BULK_DATA_LIST)context, queueFull, result);
        if (OI_SUCCESS(result)) {
            /*
             * Report progress (if any) if the upper layer has provided a progress callback
             */
            if (connection->common.progressBytes && connection->currentTarget->CBList->progressInd) {
                connection->currentTarget->CBList->progressInd(connection->common.connectionHandle, OI_OBEX_CMD_GET, connection->common.progressBytes);
            }
        }
        break;
    case GET_IN_PROGRESS:
        if (!OI_SUCCESS(result)) {
            SetState(connection, GET_COMPLETING);
            break;
        }
        if (connection->final) {
            /*
             * If we are through the initial GET phase switch over to the bulk GET APIs if the
             * upper profile supports bulk get.
             */
            if (connection->currentTarget->CBList->bulkGetInd) {
                SetState(connection, BULK_GET_IN_PROGRESS);
                /*
                 * If we are doing SRM we are not going to get a response packet so we need to
                 * call getInd to request data from the upper layer profile.
                 */
                if (connection->common.srm & OI_OBEX_SRM_ENABLED) {
                    connection->currentTarget->CBList->bulkGetInd(connection->common.connectionHandle, 0, NULL, NULL, OI_OBEX_CONTINUE);
                }
            } else {
                /*
                 * If we are doing SRM we are not going to get a response packet so we need to
                 * call getInd to request data from the upper layer profile.
                 */
                if (connection->common.srm & OI_OBEX_SRM_ENABLED) {
                    connection->currentTarget->CBList->getInd(connection->common.connectionHandle, NULL, OI_OBEX_CONTINUE);
                }
                /*
                 * Report progress (if any) if the upper layer has provided a progress callback
                 */
                if (connection->currentTarget->CBList->progressInd && connection->common.progressBytes) {
                    connection->currentTarget->CBList->progressInd(connection->common.connectionHandle, OI_OBEX_CMD_GET, connection->common.progressBytes);
                }
            }
        }
        break;
    default:
        /* Nothing to do */
        break;
    }
}


static OI_STATUS ServerSendPacket(OBEXSRV_CONNECTION *connection)
{
    OI_STATUS status;
    OI_BOOL queueFull;

    OI_DBGPRINT2(("Sending mbuf %#x", connection->common.mbuf));

    status = connection->common.lowerConnection->ifc->write(connection->common.lowerConnection, connection->common.mbuf, TRUE, &queueFull);
    if (!OI_SUCCESS(status)) {
        OI_SLOG_ERROR(status, ("Lower layer write failed"));
        OI_MBUF_Free(connection->common.mbuf);
        connection->common.mbuf = NULL;
    }
    return status;
}


static OI_STATUS ServerSendBodySegment(OBEXSRV_CONNECTION *connection)
{
    OI_STATUS status;

    OI_DBGPRINT(("ServerSendBodySegment"));

    status = OI_OBEXCOMMON_MarshalBodySegment(&connection->common);
    if (OI_SUCCESS(status)) {
        status = ServerSendPacket(connection);
    }
    else {
        OI_SLOG_ERROR(status, ("OI_OBEXCOMMON_MarshalBodySegment failed"));
    }

    return status;
}

/*
 * Cleanup and callback the application to indicate that the current operation
 * has been terminated by an ABORT command from the client.
 */

static OI_STATUS AbortOperation(OBEXSRV_CONNECTION *connection)
{
    OI_STATUS   status;

    OI_DBGPRINT(("Server got abort command in state %s", ServerStateTxt(connection->state)));

    switch (connection->state) {
    case PUT_IN_PROGRESS:
    case PUT_COMPLETING:
        SetState(connection, OBEX_CONNECTED);
        connection->common.srm &= ~OI_OBEX_SRM_ENABLED;
        connection->currentTarget->CBList->putInd(connection->common.connectionHandle, NULL, OI_OBEX_CLIENT_ABORTED_COMMAND);
        break;
    case GET_IN_PROGRESS:
    case GET_COMPLETING:
        SetState(connection, OBEX_CONNECTED);
        connection->currentTarget->CBList->getInd(connection->common.connectionHandle, NULL, OI_OBEX_CLIENT_ABORTED_COMMAND);
        break;
    case BULK_GET_IN_PROGRESS:
    case BULK_GET_COMPLETING:
    case BULK_GET_FAILED:
        if (!connection->bulkGet.head) {
            /*
             *  Nothing to confirm - just cleanup the state and indicate the abort
             */
            connection->common.srm &= ~OI_OBEX_SRM_ENABLED;
            SetState(connection, OBEX_CONNECTED);
            connection->currentTarget->CBList->bulkGetInd(connection->common.connectionHandle, 0, NULL, NULL, OI_OBEX_CLIENT_ABORTED_COMMAND);
        } else if (ConfirmBulkGets(connection, OI_OBEX_CLIENT_ABORTED_COMMAND, CONFIRM_ALL, NULL)) {
            connection->abortPending = FALSE;
            connection->common.srm &= ~OI_OBEX_SRM_ENABLED;
            SetState(connection, OBEX_CONNECTED);
        } else {
            /*
             * Unable to confirm all blocks - wait for the lower layer confirms to clean up
             */
            connection->abortPending = TRUE;
        }
        break;
    default:
        break;
    }
    if (connection->abortPending) {
        /*
         * We will respond to the abort after the cleanup.
         */
        status = OI_OK;
    } else {
        /*
         * Acknowledge receipt of ABORT command.
         */
        OI_DBGPRINT(("Confirming abort"));
        if (connection->abortMbuf) {
            OI_BOOL queueFull;

            /*
             *  Send abort response sitting waiting in the abortMbuf
             */
            status = connection->common.lowerConnection->ifc->write(connection->common.lowerConnection, connection->abortMbuf, TRUE, &queueFull);
            if (!OI_SUCCESS(status)) {
                OI_SLOG_ERROR(status, ("Lower layer write failed"));
                OI_MBUF_Free(connection->abortMbuf);
                connection->abortMbuf = NULL;
            }
        } else {
            /*
             *  Send standard OK abort response
             */
            status = OI_OBEXCOMMON_SendOk(&connection->common);
        }
    }
    return status;
}


static void AppendBulkData(OBEXSRV_CONNECTION *connection,
                           OBEX_BULK_DATA_LIST bulkDataHead,
                           OBEX_BULK_DATA_LIST bulkDataTail)
{
    if (connection->bulkGet.tail) {
        connection->bulkGet.tail->next = bulkDataHead;
    } else {
        OI_ASSERT(connection->bulkGet.head == NULL);
        connection->bulkGet.head = bulkDataHead;
    }
    connection->bulkGet.tail = bulkDataTail;
    if (!connection->bulkGet.current) {
        connection->bulkGet.current = bulkDataHead;
    }
}


OI_STATUS OI_OBEXSRV_BulkGetResponse(OI_OBEXSRV_CONNECTION_HANDLE connectionId,
                                     OI_UINT8 numBuffers,
                                     OI_UINT8 *bulkDataBuffer[],
                                     OI_UINT32 bufferLength[],
                                     OI_STATUS rspStatus)
{
    OI_STATUS status = (rspStatus != OI_OBEX_CONTINUE) ? rspStatus : OI_OK;
    OBEXSRV_CONNECTION* connection;
    OBEX_BULK_DATA_LIST bulkDataHead;
    OBEX_BULK_DATA_LIST bulkDataTail;

    connection = LookupConnection(connectionId);
    if (connection == NULL) {
        OI_DBGPRINT2(("GetResponse bad connectionId"));
        return OI_STATUS_INVALID_PARAMETERS;
    }

    OI_DBGPRINT(("OI_OBEXSRV_BulkGetResponse numBuffers:%d %d", numBuffers, rspStatus));

    /*
     * We don't expect to be called if a bulk put is failing
     */
    if (connection->state == BULK_GET_FAILED) {
        return OI_STATUS_INVALID_STATE;
    }
    /*
     * If the bulk put is completing we don't expect any new data
     */
    if (connection->state == BULK_GET_COMPLETING) {
        if (numBuffers > 0) {
            OI_DBGPRINT(("BulkGetResponse expected numBuffers == 0"));
            return OI_STATUS_INVALID_STATE;
        } else {
            return OI_OK;
        }
    }
    /*
     * This function can only be called if we indicated a bulk get
     */
    if (connection->state != BULK_GET_IN_PROGRESS) {
        OI_DBGPRINT(("BulkGetResponse when other operation in progress"));
        return OI_STATUS_INVALID_STATE;
    }
    /*
     * Reject data if we have an abort pending
     */
    if (connection->abortPending) {
        return OI_OBEX_CLIENT_ABORTED_COMMAND;
    }
    /*
     * No more get responses allowed after the final one.
     */
    if (connection->bulkGet.tail && connection->bulkGet.tail->final) {
        OI_DBGPRINT(("BulkGetResponse final already seen"));
        return OI_STATUS_INVALID_STATE;
    }
    /*
     * Cleanup if we got an error status
     */
    if (!OI_SUCCESS(status)) {
        goto BulkGetResponseError;
    }
    /*
     * Unless we got an error status we require at least one buffer to send.
     */
    if (numBuffers == 0) {
        return OI_STATUS_INVALID_PARAMETERS;
    }

#ifdef OI_TEST_HARNESS
    if (OI_ObexTest.SendErrorRsp) {
        status = OI_OBEX_ERROR;
        OI_ObexTest.SendErrorRsp = FALSE;
        goto BulkGetResponseError;
    }
#endif

    /*
     * Allocate data structures to track the bulk data transfer
     */
    status = OI_OBEXCOMMON_AllocBulkData(numBuffers, bulkDataBuffer, bufferLength, &bulkDataHead, &bulkDataTail);
    if (!OI_SUCCESS(status)) {
        goto BulkGetResponseError;
    }
    OI_ASSERT(bulkDataTail != NULL);
    if (rspStatus != OI_OBEX_CONTINUE) {
        bulkDataTail->final = TRUE;
        SetState(connection, BULK_GET_COMPLETING);
    }
    /*
     * We queue this bulk data if we are already putting a block or the lower layer is busy
     */
    if (connection->bulkGet.current || connection->bulkGet.busy) {
        OI_DBGPRINT(("OI_OBEXSRV_BulkGetResponse queuing bulk data block"));
        AppendBulkData(connection, bulkDataHead, bulkDataTail);
    } else {
        AppendBulkData(connection, bulkDataHead, bulkDataTail);
        status = OI_OBEXCOMMON_SendBulk(&connection->common, &connection->bulkGet.current, &connection->bulkGet.busy, NULL);
        if (!OI_SUCCESS(status)) {
            /*
             * Free the bulk data list if nothing was sent otherwise start a disconnect and let the
             * disconnect process cleanup for us.
             */
            if (bulkDataHead->bytesSent == bulkDataHead->bytesConfirmed) {
                OI_OBEXCOMMON_FreeBulkData(bulkDataHead);
                goto BulkGetResponseError;
            }
            status = OI_OBEXSRV_ForceDisconnect(connectionId);
        }
    }
    if (OI_SUCCESS(status)) {
        return OI_OK;
    }

BulkGetResponseError:

    SetState(connection, BULK_GET_FAILED);
    if (connection->common.srm & OI_OBEX_SRM_ENABLED) {
        /*
         * If there is nothing to confirm we are done.
         */
        if (connection->bulkGet.head == NULL) {
            connection->common.srm &= ~OI_OBEX_SRM_ENABLED;
            SetState(connection, OBEX_CONNECTED);
        }
        ErrorResponse(connection, status);
    }
    return OI_OBEX_ERROR;
}


/*
 * Called by application to write headers to satisfy a GET request.
 */

OI_STATUS OI_OBEXSRV_GetResponse(OI_OBEXSRV_CONNECTION_HANDLE connectionId,
                                 OI_OBEX_HEADER_LIST *rspHeaders,
                                 OI_STATUS rspStatus)
{
    OI_OBEX_HEADER headers[2];
    OI_UINT16 headerCount = 0;
    OI_UINT8 rsp;
    OI_STATUS status = OI_OK;
    OI_BYTE_STREAM pkt;
    OBEXSRV_CONNECTION* connection;

    OI_DBGTRACE(("OI_OBEXSRV_GetResponse (connectionId = %d, <*headers = %x>, rspStatus = %d)\n",
                 connectionId, rspHeaders, rspStatus));

    connection = LookupConnection(connectionId);
    if (connection == NULL) {
        OI_DBGPRINT2(("GetResponse bad connectionId"));
        return OI_STATUS_INVALID_PARAMETERS;
    }
    /*
     * Only one operation at a time on each connection.
     */
    if (connection->state != GET_IN_PROGRESS) {
        OI_DBGPRINT2(("GetResponse when other operation in progress"));
        return OI_OBEX_OPERATION_IN_PROGRESS;
    }

    OI_DBGPRINT2(("GetResponse %d", rspStatus));

    if ((rspStatus != OI_OK) && (rspStatus != OI_OBEX_CONTINUE)) {
        goto GetResponseError;
    }

#ifdef OI_TEST_HARNESS
    if (OI_ObexTest.SendErrorRsp) {
        status = OI_OBEX_ERROR;
        rspStatus = status;
        OI_ObexTest.SendErrorRsp = FALSE;
        goto GetResponseError;
    }
#endif

    if (rspStatus == OI_OBEX_CONTINUE) {
        rsp = OI_OBEX_FINAL(OI_OBEX_RSP_CONTINUE);
    } else {
        SetState(connection, GET_COMPLETING);
        rsp = OI_OBEX_FINAL(OI_OBEX_RSP_OK);
    }
    OI_OBEXCOMMON_InitPacket(&connection->common, rsp, &pkt);
    /*
     * Enable SRM if the client requested it
     */
    if (connection->common.srm & OI_OBEX_SRM_REQUESTED) {
        connection->common.srm &= ~OI_OBEX_SRM_REQUESTED;
        connection->common.srm |= OI_OBEX_SRM_ENABLED;
        headers[headerCount].id = OI_OBEX_HDR_SINGLE_RESPONSE_MODE;
        headers[headerCount].val.srm = OI_OBEX_SRM_ENABLED;
        headerCount++;
#ifdef OI_TEST_HARNESS
        if (OI_ObexTest.ignoreSRM) {
            connection->common.srm &= ~OI_OBEX_SRM_ENABLED;
            OI_SLOG_ERROR(OI_STATUS_TEST_HARNESS, ("Ignoring SRM!!!"));
        }
#endif
    }
    /*
     * If we are doing SRM request but we got an incomplete GET (i.e. !final) request another GET
     * command.
     */
    if (!connection->final && (connection->common.srm & OI_OBEX_SRM_ENABLED)) {
        headers[headerCount].id = OI_OBEX_HDR_SINGLE_RESPONSE_PARAMETERS;
        headers[headerCount].val.srmParam = OI_OBEX_SRM_PARAM_RSVP;
        headerCount++;
    }
    status = OI_OBEXCOMMON_MarshalPacket(&connection->common, &pkt, headers, headerCount, rspHeaders);
    if (OI_SUCCESS(status)) {
        status = ServerSendPacket(connection);
    }
    if (!OI_SUCCESS(status)) {
        OI_SLOG_ERROR(status, ("OI_OBEXSRV_GetResponse failed"));
        rspStatus = status;
        goto GetResponseError;
    }

    return OI_OK;

GetResponseError:

    /*
     * Report an error result to the client.
     */
    SetState(connection, OBEX_CONNECTED);
    ErrorResponse(connection, rspStatus);
    return status;
}


static OI_BOOL ResponseRequired(OBEXSRV_CONNECTION* connection)
{
    if (!(connection->common.srm & OI_OBEX_SRM_ENABLED)) {
        return TRUE;
    }
    /*
     * Final packet always requires a response.
     */
    if (connection->final) {
        connection->common.srmParam = SRM_PARAM_NONE;
        return TRUE;
    }
    /*
     * One time request for a response
     */
    if (connection->common.srmParam == OI_OBEX_SRM_PARAM_RSVP) {
        OI_DBGPRINT2(("SRM client requested response"));
        connection->common.srmParam = SRM_PARAM_NONE;
        return TRUE;
    }
    /*
     * Request for a response and wait for the next packet
     */
    if (connection->common.srmParam == OI_OBEX_SRM_PARAM_RSVP_AND_WAIT) {
        OI_DBGPRINT2(("SRM client requested response and wait"));
        return TRUE;
    }
    /*
     * We are in SRM mode so should not send a response packet
     */
    OI_DBGPRINT2(("SRM no response required"));
    return FALSE;
}


/*
 * Called by application to acknowledge a PUT request. Most servers will
 * respond with NULL headers, but the Sync profile requires Application
 * Parameter headers in some PUT responses.
 */

OI_STATUS OI_OBEXSRV_PutResponse(OI_OBEXSRV_CONNECTION_HANDLE connectionId,
                                 OI_OBEX_HEADER_LIST *rspHeaders,
                                 OI_STATUS rspStatus)
{
    OI_OBEX_HEADER headers[1];
    OI_UINT16 headerCount = 0;
    OI_UINT8 rsp;
    OI_STATUS status = OI_OK;
    OI_BYTE_STREAM pkt;
    OBEXSRV_CONNECTION* connection;

    OI_DBGTRACE(("OI_OBEXSRV_PutResponse connectionId:%d %d headers:%d=",
                 connectionId, rspStatus, rspHeaders ? rspHeaders->list : NULL, rspHeaders ? rspHeaders->count : 0));

    connection = LookupConnection(connectionId);
    if (connection == NULL) {
        return OI_STATUS_INVALID_PARAMETERS;
    }
    /*
     * Only one operation at a time on each connection.
     */
    if (connection->state != PUT_IN_PROGRESS) {
        return OI_OBEX_OPERATION_IN_PROGRESS;
    }

    OI_DBGPRINT2(("OI_OBEXSRV_PutResponse(%d, %8x, %d)", connectionId, headers, rspStatus));

    /*
     * In SRM mode we turned flow off until we get this confirmation call from the upper layer so
     * now we need to turn flow on again.
     */
    if (connection->common.srm & OI_OBEX_SRM_ENABLED) {
        connection->common.lowerConnection->ifc->flowControl(connection->common.lowerConnection, TRUE /* ON */);
    }

    if ((rspStatus != OI_OK) && (rspStatus != OI_OBEX_CONTINUE) && (rspStatus != OI_OBEX_PARTIAL_CONTENT)) {
        goto PutResponseError;
    }

    if (rspStatus == OI_OBEX_CONTINUE) {
        /*
         * We don't expect a continue after reporting the final put
         */
        if (connection->final) {
            OI_DBGPRINT(("OBEX Server in Invalid state\n "));
            status = OI_STATUS_INVALID_STATE;
            goto PutResponseError;
        }
        rsp = OI_OBEX_FINAL(OI_OBEX_RSP_CONTINUE);
    } else {
        SetState(connection, PUT_COMPLETING);
        connection->common.srm &= ~OI_OBEX_SRM_ENABLED;
        rsp = MapStatus(rspStatus);
    }

    /*
     * If we are in SRM mode we cannot send a response unless we are asked for one
     */
    if (ResponseRequired(connection)) {
        /*
         * Enable SRM if it has been requested and the PUT is continuing
         */
        if ((rspStatus == OI_OBEX_CONTINUE) && connection->common.srm & OI_OBEX_SRM_REQUESTED) {
            connection->common.srm &= ~OI_OBEX_SRM_REQUESTED;
            connection->common.srm |= OI_OBEX_SRM_ENABLED;
            headers[headerCount].id = OI_OBEX_HDR_SINGLE_RESPONSE_MODE;
            headers[headerCount].val.srm = OI_OBEX_SRM_ENABLED;
            headerCount++;
#ifdef OI_TEST_HARNESS
            if (OI_ObexTest.ignoreSRM) {
                connection->common.srm &= ~OI_OBEX_SRM_ENABLED;
                OI_SLOG_ERROR(OI_STATUS_TEST_HARNESS, ("Ignoring SRM!!!"));
            }
#endif
        }
        if ((NULL != rspHeaders) || (headerCount > 0)) {
            OI_OBEXCOMMON_InitPacket(&connection->common, rsp, &pkt);
            status = OI_OBEXCOMMON_MarshalPacket(&connection->common, &pkt, headers, headerCount, rspHeaders);
            if (OI_SUCCESS(status)) {
                status = ServerSendPacket(connection);
            }
        } else {
            status = OI_OBEXCOMMON_SendSimple(&connection->common, rsp);
        }
        if (!OI_SUCCESS(status)) {
            OI_SLOG_ERROR(status, ("OI_OBEXSRV_PutResponse failed"));
            rspStatus = status;
            goto PutResponseError;
        }
    }

    return OI_OK;

PutResponseError:

    SetState(connection, OBEX_CONNECTED);
    ErrorResponse(connection, rspStatus);
    return status;
}


OI_STATUS OI_OBEXSRV_ActionResponse(OI_OBEXSRV_CONNECTION_HANDLE    connectionId,
                                    OI_UINT8                        actionId,
                                    OI_STATUS                       rspStatus,
                                    const OI_OBEX_HEADER_LIST       *optHeaders)
{
    OI_STATUS status = OI_OK;
    OBEXSRV_CONNECTION* connection;

    OI_DBGTRACE(("OI_OBEXSRV_ActionResponse connectionId:%d %d", connectionId, rspStatus));

    connection = LookupConnection(connectionId);
    if (connection == NULL) {
        return OI_STATUS_INVALID_PARAMETERS;
    }
    /*
     * Only one operation at a time on each connection.
     */
    if (connection->state != DO_ACTION) {
        return OI_OBEX_OPERATION_IN_PROGRESS;
    }

    status = sendSimpleWithHeaders(&connection->common, MapStatus(rspStatus), optHeaders);
    if (!OI_SUCCESS(status)) {
        OI_SLOG_ERROR(status, ("OI_OBEXSRV_ActionResponse failed"));
        rspStatus = status;
        goto ActionResponseError;
    }

    SetState(connection, OBEX_CONNECTED);
    return OI_OK;

ActionResponseError:

    SetState(connection, OBEX_CONNECTED);
    ErrorResponse(connection, rspStatus);
    return status;
}


/**
 * Bluetooth specification 5.6 - if a Target was used to establish the
 * connection a ConnectionId is required in all subesquent requests.
 */
static OI_STATUS VerifyConnectionId(OBEXSRV_CONNECTION *connection,
                                    OI_OBEX_HEADER_LIST *headers)
{
    OI_STATUS status = OI_OK;
    OI_OBEX_HEADER *conId;

    if (connection->clientSpecifiedTarget) {
        conId = OI_OBEX_FindHeader(headers, OI_OBEX_HDR_CONNECTION_ID);
        if (conId == NULL) {
            status = OI_OBEX_REQUIRED_HEADER_NOT_FOUND;
        } else {
            if (conId->val.connectionId != (OI_UINT32) connection) {
                status = OI_OBEX_SERVICE_UNAVAILABLE;
            }
        }
    }
    return status;
}


/**
 *
 */

static OI_STATUS GetCommand(OBEXSRV_CONNECTION *connection,
                            OI_BYTE_STREAM *rcvPacket,
                            OI_BOOL final)
{
    OI_OBEX_HEADER_LIST headers = { NULL, 0 };
    OI_BOOL newRequest = FALSE;
    OI_OBEX_HEADER *srmParamHdr;
    OI_OBEX_HEADER *srmHdr;
    OI_STATUS status;

#ifdef OI_TEST_HARNESS
    if (OI_ObexTest.SendErrorRsp) {
        status = OI_OBEX_ERROR;
        OI_ObexTest.SendErrorRsp = FALSE;
        goto GetCommandError;
    }
#endif

    /*
     * Not busy after receiving a get request
     */
    connection->bulkGet.busy = FALSE;

    switch (connection->state) {
    case OBEX_CONNECTED:
        /*
         * This the start of a new GET request
         */
        OI_DBGPRINT2(("OBEX server received new get command"));
        newRequest = TRUE;
        SetState(connection, GET_IN_PROGRESS);
        connection->bulkGet.head = NULL;
        connection->bulkGet.tail = NULL;
        connection->bulkGet.busy = FALSE;
        connection->final = FALSE;
        connection->common.progressBytes = 0;
        break;
    case GET_IN_PROGRESS:
    case GET_COMPLETING:
        /*
         * Check that there is an upper layer server to handle the continuing get request.
         */
        if (connection->currentTarget->CBList->getInd == NULL) {
            status = OI_OBEX_SERVICE_UNAVAILABLE;
            OI_SLOG_ERROR(status, ("OBEX server received get without application to service it"));
            goto GetCommandError;
        }
        break;
    case BULK_GET_IN_PROGRESS:
    case BULK_GET_COMPLETING:
        /*
         * Check that there is an upper layer server to handle the continuing get request.
         */
        if (connection->currentTarget->CBList->bulkGetInd == NULL) {
            status = OI_OBEX_SERVICE_UNAVAILABLE;
            OI_SLOG_ERROR(status, ("OBEX server received get without application to service it"));
            goto GetCommandError;
        }
        break;
    case BULK_GET_FAILED:
        status = OI_OBEX_ERROR;
        goto GetCommandError;
    default:
        /*
         * Not expecting a get in this state.
         */
        status = OI_OBEX_OPERATION_IN_PROGRESS;
        goto GetCommandError;

    }
    /*
     * If SRM is enabled we don't expect GET commands after the GET is final.
     */
    if (connection->final && (connection->common.srm & OI_OBEX_SRM_ENABLED)) {
        status = OI_OBEX_OPERATION_IN_PROGRESS;
        OI_SLOG_ERROR(status, ("Unexpected GET received while doing SRM"));
        goto GetCommandError;
    }
    connection->final = final;
    /*
     * Parse the command into a list of headers.
     */
    status = OI_OBEXCOMMON_ParseHeaderList(&connection->common, &headers, rcvPacket);
    if (!OI_SUCCESS(status)) {
        OI_SLOG_ERROR(status, ("OI_OBEXCOMMON_ParseHeaderList failed"));
        goto GetCommandError;
    }
    /*
     * Check if client is requesting single response mode for this get
     */
    srmHdr = OI_OBEX_FindHeader(&headers, OI_OBEX_HDR_SINGLE_RESPONSE_MODE);
    if (srmHdr && (srmHdr->val.srm == OI_OBEX_SRM_ENABLED)) {
        if (connection->common.srm & OI_OBEX_SRM_SUPPORTED) {
            OI_DBGPRINT(("Client requesting SRM for this get"));
            connection->common.srm |= OI_OBEX_SRM_REQUESTED;
        } else {
            OI_DBGPRINT(("Client requesting SRM for this get (denied by server)"));
        }
        /*
         * We don't want to pass the SRM header to the upper-layer.
         */
        OI_OBEXCOMMON_DeleteHeaderFromList(&headers, OI_OBEX_HDR_SINGLE_RESPONSE_MODE);
    }
    /*
     * Look for an SRM parameter header
     */
    srmParamHdr = OI_OBEX_FindHeader(&headers, OI_OBEX_HDR_SINGLE_RESPONSE_PARAMETERS);
    if (srmParamHdr) {
        OI_DBGPRINT2(("SRM parameter %d", srmParamHdr->val.srmParam));
        /*
         * save the srm parameter, we will need it later
         */
        connection->common.srmParam = srmParamHdr->val.srmParam;
        /*
         * We don't want to pass the SRM header to the upper-layer.
         */
        OI_OBEXCOMMON_DeleteHeaderFromList(&headers, OI_OBEX_HDR_SINGLE_RESPONSE_PARAMETERS);
    } else {
        connection->common.srmParam = SRM_PARAM_NONE;
    }
    if (newRequest) {
        /*
         * Bluetooth specification 5.6 - either the Type header or the Name header
         * must be included in the GET request.
         */
        if (!OI_OBEX_FindHeader(&headers, OI_OBEX_HDR_NAME) && !OI_OBEX_FindHeader(&headers, OI_OBEX_HDR_TYPE)) {
            status = OI_OBEX_REQUIRED_HEADER_NOT_FOUND;
            goto GetCommandError;
        }
        /*
         * Only the first command packet has a connection id.
         */
        status = VerifyConnectionId(connection, &headers);
        if (!OI_SUCCESS(status)) {
            OI_SLOG_ERROR(status, ("VerifyConnectionId failed"));
            goto GetCommandError;
        }
        /*
         * Make sure the body segmentation header is cleared.
         */
        connection->common.bodySegment.id = 0;
    } else {
        /*
         * If we are sending a segmented body header we will not be calling the
         * application for more data until we are done with this body header.
         */
        if (OI_OBEX_IS_A_BODY_HEADER(connection->common.bodySegment.id)) {
            status = ServerSendBodySegment(connection);
            if (!OI_SUCCESS(status)) {
                goto GetCommandError;
            }
            OI_FreeIf(&headers.list);
            return OI_OK;
        }
    }
    if ((connection->state == BULK_GET_IN_PROGRESS) || (connection->state == BULK_GET_COMPLETING)) {
        OI_ASSERT(!(connection->common.srm & OI_OBEX_SRM_ENABLED));
        if (connection->bulkGet.head) {
            ConfirmBulkGets(connection, OI_OBEX_CONTINUE, CONFIRM_COMPLETED, NULL);
        } else {
            connection->currentTarget->CBList->bulkGetInd(connection->common.connectionHandle, 0, NULL, NULL, OI_OBEX_CONTINUE);
        }
        /*
         * If there is data queued to send, send it.
         */
        if (connection->bulkGet.current) {
            status = OI_OBEXCOMMON_SendBulk(&connection->common, &connection->bulkGet.current, &connection->bulkGet.busy, NULL);
        }
    } else {
        OI_STATUS indStatus = final ? OI_OBEX_CONTINUE : OI_OBEXSRV_INCOMPLETE_GET;
        status = connection->currentTarget->CBList->getInd(connection->common.connectionHandle, &headers, indStatus);
    }
    if (!OI_SUCCESS(status)) {
        goto GetCommandError;
    }

    OI_FreeIf(&headers.list);
    return OI_OK;

GetCommandError:

    OI_DBGPRINT(("OBEX get command %d", status));

    OI_ASSERT(!OI_SUCCESS(status));

    connection->common.srm &= ~OI_OBEX_SRM_ENABLED;
    /*
     * Cleanup depending on state we are in
     */
    switch (connection->state) {
    case OBEX_CONNECTED:
        break;
    case GET_IN_PROGRESS:
    case GET_COMPLETING:
        if (connection->currentTarget->CBList->getInd != NULL) {
            connection->currentTarget->CBList->getInd(connection->common.connectionHandle, NULL, status);
        }
        break;
    case BULK_GET_IN_PROGRESS:
    case BULK_GET_COMPLETING:
        if (connection->currentTarget->CBList->bulkGetInd != NULL) {
            connection->currentTarget->CBList->bulkGetInd(connection->common.connectionHandle, 0, NULL, NULL, status);
        }
        break;
    case BULK_GET_FAILED:
        if (!ConfirmBulkGets(connection, status, CONFIRM_ALL, NULL)) {
            OI_SLOG_ERROR(OI_STATUS_INVALID_STATE, ("Bulk get done with incomplete sends pending"));
        }
        break;
    default:
        break;
    }
    SetState(connection, OBEX_CONNECTED);
    OI_FreeIf(&headers.list);
    ErrorResponse(connection, status);

    return status;
}


/**
 *  Handle abort command, either immediately or via registered callback
 *
 */

static OI_STATUS handleAbortCommand(OBEXSRV_CONNECTION *connection, OI_BYTE_STREAM *rcvPacket)
{
    OI_STATUS           status;

    /*
     * Any previous abort command handling should have finished.  Don't assert this because
     * misbehaving remote device could trigger this logic.
     */
    if (NULL != connection->abortMbuf) {
        OI_LOG_ERROR(("Received abort command while still processing previous abort command"));
        connection->abortMbuf = NULL;
    }

    if (connection->currentTarget) {
        if (connection->currentTarget->CBList->abortInd) {
            /*
             * Defer handling abort command until application calls abortResponse API
             * As always, keep in mind that OBEXSRV_AbortResponse() might be called from within the callback.
             */
            OI_OBEX_HEADER_LIST headers;

            status = OI_OBEXCOMMON_ParseHeaderList(&connection->common, &headers, rcvPacket);
            if (OI_SUCCESS(status)) {
                connection->abortPending = TRUE;
                connection->abortResponsePending = TRUE;
                connection->abortMbuf = NULL;
                connection->currentTarget->CBList->abortInd(connection->common.connectionHandle, &headers);

            } else {
                ErrorResponse(connection, status);
            }
        } else {
            /*
             *  Process the abort command now.
             */
            status = AbortOperation(connection);
        }
    } else {
        status = OI_OBEX_NOT_CONNECTED;
    }
    return status;
}

/**
 * Handle a request from the client to perform a put operation.
 */

static OI_STATUS PutCommand(OBEXSRV_CONNECTION *connection,
                            OI_BYTE_STREAM *rcvPacket,
                            OI_BOOL final)
{
    OI_OBEX_HEADER_LIST headers;
    OI_OBEX_HEADER *srmParamHdr;
    OI_OBEX_HEADER *srmHdr;
    OI_STATUS status;

#ifdef OI_TEST_HARNESS
    if (OI_ObexTest.SendErrorRsp) {
        status = OI_OBEX_ERROR;
        OI_ObexTest.SendErrorRsp = FALSE;
        goto PutCommandError;
    }
#endif

    /*
     * Parse the command into a list of headers.
     */
    status = OI_OBEXCOMMON_ParseHeaderList(&connection->common, &headers, rcvPacket);

    if (OI_SUCCESS(status)) {
        /*
         * Is this the start of a new PUT request?
         */
        if (connection->state == OBEX_CONNECTED) {
            OI_DBGPRINT(("OBEX server received new PUT command final:%d", final));
            /*
             * Only the first command packet has a connection id.
             */
            status = VerifyConnectionId(connection, &headers);
            if (!OI_SUCCESS(status)) {
                goto PutCommandError;
            }
            SetState(connection, PUT_IN_PROGRESS);
            connection->common.progressBytes = 0;
            /*
             * Check if client is requesting single response mode for this put
             */
            srmHdr = OI_OBEX_FindHeader(&headers, OI_OBEX_HDR_SINGLE_RESPONSE_MODE);
            if (srmHdr && (srmHdr->val.srm == OI_OBEX_SRM_ENABLED)) {
                if (connection->common.srm & OI_OBEX_SRM_SUPPORTED) {
                    OI_DBGPRINT(("Client requesting SRM for this put"));
                    connection->common.srm |= OI_OBEX_SRM_REQUESTED;
                } else {
                    OI_DBGPRINT(("Client requesting SRM for this put (denied by server)"));
                }
                /*
                 * We don't want to pass the SRM header to the upper-layer.
                 */
                OI_OBEXCOMMON_DeleteHeaderFromList(&headers, OI_OBEX_HDR_SINGLE_RESPONSE_PARAMETERS);
            }
        } else {
            if (connection->state != PUT_IN_PROGRESS) {
                status = OI_OBEX_OPERATION_IN_PROGRESS;
                goto PutCommandError;
            }
            OI_DBGPRINT2(("OBEX server received PUT command final:%d", final));
        }
        connection->final = final;
        /*
         * Check that there is a server to handle the put request.
         */
        if (connection->currentTarget->CBList->putInd == NULL) {
            status = OI_OBEX_SERVICE_UNAVAILABLE;
            goto PutCommandError;
        }
        /*
         * Look for an SRM parameter header
         */
        srmParamHdr = OI_OBEX_FindHeader(&headers, OI_OBEX_HDR_SINGLE_RESPONSE_PARAMETERS);
        if (srmParamHdr) {
            OI_DBGPRINT2(("SRM parameter %d", srmParamHdr->val.srmParam));
            /*
             * save the srm parameters, we will need them later
             */
            connection->common.srmParam = srmParamHdr->val.srmParam;
            /*
             * We don't want to pass the SRM header to the upper-layer.
             */
            OI_OBEXCOMMON_DeleteHeaderFromList(&headers, OI_OBEX_HDR_SINGLE_RESPONSE_PARAMETERS);
        } else {
            connection->common.srmParam = SRM_PARAM_NONE;
        }
        /*
         * In SRM mode we need to turn flow off until we get a confirm from the upper layer. Because
         * of buffering in the lower layer this will not normally stop the flow of data from the
         * remote peer unless the upper layer delays calling the confirm API.
         */
        if (connection->common.srm & OI_OBEX_SRM_ENABLED) {
            connection->common.lowerConnection->ifc->flowControl(connection->common.lowerConnection, FALSE /* OFF */);
        }
        /*
         * Call upper layer server with the PUT request headers.
         */
        status = connection->currentTarget->CBList->putInd(connection->common.connectionHandle, &headers, final ? OI_OK : OI_OBEX_CONTINUE);
        if (OI_SUCCESS(status)) {

            /*
             * Report progress if the upper layer has provided a progress callback and there has been progress
             */
            if (connection->currentTarget->CBList->progressInd && connection->common.progressBytes) {
                connection->currentTarget->CBList->progressInd(connection->common.connectionHandle, OI_OBEX_CMD_PUT, connection->common.progressBytes);
            }
        } else {
            /*
             * Turn flow on again if it was turned off above
             */
            if (connection->common.srm & OI_OBEX_SRM_ENABLED) {
                connection->common.lowerConnection->ifc->flowControl(connection->common.lowerConnection, TRUE /* ON */);
            }
            /*
             * Application reported an error - we need to return this error status
             * to the OBEX client.
             */
            SetState(connection, OBEX_CONNECTED);
            ErrorResponse(connection, status);
        }
        OI_FreeIf(&headers.list);
        return OI_OK;
    }

PutCommandError:

    OI_SLOG_ERROR(status, ("PutCommand error"));

    OI_ASSERT(!OI_SUCCESS(status));

    OI_FreeIf(&headers.list);
    if ((connection->state == OBEX_CONNECTED) || (connection->state == PUT_IN_PROGRESS) || (connection->state == PUT_COMPLETING)) {
        connection->currentTarget->CBList->putInd(connection->common.connectionHandle, NULL, status);
        SetState(connection, OBEX_CONNECTED);
    }
    ErrorResponse(connection, status);
    return status;
}


/**
 *
 */

static OI_STATUS SetPathCommand(OBEXSRV_CONNECTION *connection,
                                OI_BYTE_STREAM *rcvPacket)
{
    OI_OBEX_HEADER_LIST headers = { NULL, 0 };
    OI_OBEX_HEADER *hdr;
    OI_OBEX_UNICODE *name = NULL;
    OI_UINT8 flags = 0;
    OI_BOOL uplevel;
    OI_BOOL create;
    OI_STATUS status;

    OI_DBGPRINT(("OBEX Server - SetPathCommand"));

    if (connection->state != OBEX_CONNECTED) {
        status = OI_OBEX_OPERATION_IN_PROGRESS;
        goto SetPathCommandError;
    }

    /*
     * Check that there is a server to handle the setpath request.
     */
    if (connection->currentTarget->CBList->setPathInd == NULL) {
        status = OI_OBEX_SERVICE_UNAVAILABLE;
        goto SetPathCommandError;
    }


    ByteStream_GetUINT8_Checked(*rcvPacket, flags);

    /* We don't currently use constants -- Skipping*/
    /* ByteStream_GetUINT8_Checked(rcvPacket, constants); */
    ByteStream_Skip_Checked(*rcvPacket, 1);

    if (ByteStream_Error(*rcvPacket)) {
        status = OI_OBEX_BAD_REQUEST;
        goto SetPathCommandError;
    }

    status = OI_OBEXCOMMON_ParseHeaderList(&connection->common, &headers, rcvPacket);
    if (!OI_SUCCESS(status)) {
        goto SetPathCommandError;
    }

    status = VerifyConnectionId(connection, &headers);
    if (!OI_SUCCESS(status)) {
        goto SetPathCommandError;
    }

    uplevel = (flags & OI_OBEX_SETPATH_UP_LEVEL) != 0;
    create = (flags & OI_OBEX_SETPATH_NO_CREATE) == 0;

    hdr = OI_OBEX_FindHeader(&headers, OI_OBEX_HDR_NAME);
    if ((hdr != NULL) && (hdr->val.name.len != 0)) {
        name = &hdr->val.name;
    }

    /*
     * Reject illegal argument combinations.
     */
    if (((name != NULL) && uplevel) || ((name == NULL) && create)) {
        status = OI_OBEX_VALUE_NOT_ACCEPTABLE;
        goto SetPathCommandError;

    }

    SetState(connection, SETTING_PATH);

    /*
     * Call the application to set the path.
     */
    connection->common.pRawHeaders = &headers;
    status = connection->currentTarget->CBList->setPathInd(connection->common.connectionHandle, name, uplevel, create);
    connection->common.pRawHeaders = NULL;
    if (!OI_SUCCESS(status)) {
        goto SetPathCommandError;
    }

    OI_FreeIf(&headers.list);

    return OI_OK;

SetPathCommandError:

    OI_SLOG_ERROR(status, ("SetPathCommand error"));

    if (connection->state == SETTING_PATH) {
        SetState(connection, OBEX_CONNECTED);
    }
    OI_FreeIf(&headers.list);
    ErrorResponse(connection, status);
    return status;
}


/**
 *
 */

static OI_STATUS ActionCommand(OBEXSRV_CONNECTION *connection,
                               OI_BYTE_STREAM *rcvPacket)
{
    OI_OBEX_HEADER_LIST headers = { NULL, 0 };
    OI_OBEX_HEADER *hdr;
    OI_OBEX_UNICODE *name = NULL;
    OI_UINT8 actionId;
    OI_STATUS status;

    OI_DBGPRINT(("OBEX Server - ActionCommand"));

    if (connection->state != OBEX_CONNECTED) {
        status = OI_OBEX_OPERATION_IN_PROGRESS;
        goto ActionCommandError;
    }

    /*
     * Check that there is a server to handle the action request.
     */
    if (connection->currentTarget->CBList->actionInd == NULL) {
        status = OI_OBEX_NOT_IMPLEMENTED;
        goto ActionCommandError;
    }

    status = OI_OBEXCOMMON_ParseHeaderList(&connection->common, &headers, rcvPacket);
    if (!OI_SUCCESS(status)) {
        goto ActionCommandError;
    }

    status = VerifyConnectionId(connection, &headers);
    if (!OI_SUCCESS(status)) {
        goto ActionCommandError;
    }

    hdr = OI_OBEX_FindHeader(&headers, OI_OBEX_HDR_ACTION_ID);
    if (hdr == NULL) {
        status = OI_OBEX_REQUIRED_HEADER_NOT_FOUND;
        goto ActionCommandError;
    }
    actionId = hdr->val.actionId;

    hdr = OI_OBEX_FindHeader(&headers, OI_OBEX_HDR_NAME);
    if (hdr == NULL) {
        status = OI_OBEX_REQUIRED_HEADER_NOT_FOUND;
        goto ActionCommandError;
    }
    name = &hdr->val.name;

    SetState(connection, DO_ACTION);

    /*
     * Call the application with the action command.
     */
    status = connection->currentTarget->CBList->actionInd(connection->common.connectionHandle, actionId, name, &headers);
    if (!OI_SUCCESS(status)) {
        SetState(connection, OBEX_CONNECTED);
        goto ActionCommandError;
    }

    OI_FreeIf(&headers.list);

    return OI_OK;

ActionCommandError:

    OI_SLOG_ERROR(status, ("ActionCommand error"));

    OI_FreeIf(&headers.list);
    ErrorResponse(connection, status);
    return status;
}


static OI_STATUS SessionCommand(OBEXSRV_CONNECTION *connection,
                                OI_BYTE_STREAM *rcvPacket)
{
    OI_OBEX_HEADER_LIST headers = { NULL, 0 };
    OI_OBEX_HEADER *hdr;
    OI_UINT16 paramlen;
    OI_UINT8 *params;
    OI_UINT8 opcode;
    OI_STATUS status;

    OI_DBGPRINT(("OBEX Server - SessionCommand"));

    if (connection->state != OBEX_CONNECTED) {
        status = OI_OBEX_OPERATION_IN_PROGRESS;
        goto SessionCommandError;
    }

    status = OI_OBEXCOMMON_ParseHeaderList(&connection->common, &headers, rcvPacket);
    if (!OI_SUCCESS(status)) {
        goto SessionCommandError;
    }
    hdr = OI_OBEX_FindHeader(&headers, OI_OBEX_HDR_SESSION_PARAMS);
    if (hdr == NULL) {
        status = OI_OBEX_REQUIRED_HEADER_NOT_FOUND;
        goto SessionCommandError;
    }
    /*
     * Parse the session parameters
     */
    params = hdr->val.sessionParams.data;
    paramlen = hdr->val.sessionParams.len;

    if (paramlen < 3) {
        status = OI_OBEX_INCOMPLETE_PACKET;
        OI_SLOG_ERROR(status, ("Session parameters too short"));
        goto SessionCommandError;
    }
    /*
     * First TLV value is the one byte session opcode
     */
    if ((params[0] != OI_OBEX_SESSION_PARAM_OPCODE) || (params[1] != 1)) {
        status = OI_STATUS_INVALID_PARAMETERS;
        OI_SLOG_ERROR(status, ("First session parameter must be the session opcode"));
        goto SessionCommandError;
    }
    opcode = params[2];
    paramlen -= 3;
    params += 3;

    /*
     * Parse the remaining session parameters
     */
    while (paramlen >= 2) {
        OI_UINT8 tag = params[0];
        OI_UINT8 len = params[1];

        paramlen -= 2;
        if (paramlen < len) {
            OI_SLOG_ERROR(OI_OBEX_INCOMPLETE_PACKET, ("Session parameters too short"));
            break;
        }
        params += 2;
        switch (tag) {
        case OI_OBEX_SESSION_PARAM_DEVICE_ADDR:
            break;
        case OI_OBEX_SESSION_PARAM_NONCE:
            break;
        case OI_OBEX_SESSION_PARAM_ID:
            break;
        case OI_OBEX_SESSION_PARAM_NEXT_SEQ_NUM:
            break;
        case OI_OBEX_SESSION_PARAM_TIMEOUT:
            break;
        default:
            OI_SLOG_ERROR(OI_STATUS_INVALID_PARAMETERS, ("Invalid session parameter %02#x", params[0]));
            break;
        }
        paramlen -= len;
        params += len;
    }

    /*
     * Sessions are not yet implemented so all responses are error responses
     */
    switch (opcode) {
    case OI_OBEX_SESSION_CREATE:
        status = OI_OBEX_SERVICE_UNAVAILABLE;
        break;
    case OI_OBEX_SESSION_CLOSE:
        status = OI_OBEX_FORBIDDEN;
        break;
    case OI_OBEX_SESSION_SUSPEND:
        status = OI_OBEX_NOT_IMPLEMENTED;
        break;
    case OI_OBEX_SESSION_RESUME:
        status = OI_OBEX_SERVICE_UNAVAILABLE;
        break;
    case OI_OBEX_SESSION_SET_TIMEOUT:
        status = OI_OBEX_NOT_IMPLEMENTED;
        break;
    }

    if (!OI_SUCCESS(status)) {
        goto SessionCommandError;
    }

    OI_FreeIf(&headers.list);

    return OI_OBEX_SERVICE_UNAVAILABLE;

SessionCommandError:

    OI_SLOG_ERROR(status, ("SessionCommand error"));

    OI_FreeIf(&headers.list);
    ErrorResponse(connection, status);
    return status;
}


static OI_STATUS acceptConnect(OI_OBEXSRV_CONNECTION_HANDLE connectionId,
                               const OI_OBEX_REALM          *realm,
                               OI_BOOL                      accept,
                               OI_STATUS                    status,
                               const OI_OBEX_HEADER_LIST    *optHeaders)
{

    OBEXSRV_CONNECTION *connection;
    OI_OBEX_HEADER headers[4];
    OI_UINT16 headerCount = 0;
    OI_UINT8 rsp = OI_OBEX_FINAL(OI_OBEX_RSP_OK);
    OI_OBEX_HEADER *hdr;
    OI_BYTE_STREAM pkt;
    OI_STATUS inStatus = status;

    OI_DBGPRINT2(("acceptConnect connectionId %x accept %d %d", connectionId, accept, status));

    connection = LookupConnection(connectionId);

    if (connection == NULL) {
        OI_SLOG_ERROR(OI_OBEX_INVALID_OPERATION, ("Null connection handle!"));
        return OI_OBEX_INVALID_OPERATION;
    }

    if (accept) {
        if (connection->unauthorized) {
            /*
             * The OBEX connection has not been established
             */
            SetState(connection, LOWER_LAYER_CONNECTED);
            /*
             * Reject the connection request and send an authentication challenge
             * header in the response.
             */
            rsp = OI_OBEX_FINAL(OI_OBEX_RSP_UNAUTHORIZED);
            OI_OBEXAUTH_ComposeChallenge(OI_CONFIG_TABLE_GET(OBEX_SRV)->privateKey, realm, &connection->common, &headers[headerCount++]);
        } else {
            /*
             * The connection will be complete when the connect response packet has
             * been sent.
             */
            SetState(connection, CONNECT_COMPLETING);
            /*
             * If the connect request specified a target we must respond with a
             * ConnectionId header and a Who header.
             */
            if (connection->clientSpecifiedTarget) {
                hdr = &headers[headerCount++];
                hdr->id = OI_OBEX_HDR_CONNECTION_ID;
                hdr->val.connectionId = (OI_UINT32) connection;
                hdr = &headers[headerCount++];
                hdr->id = OI_OBEX_HDR_WHO;
                hdr->val.who.len = connection->currentTarget->target.len;
                hdr->val.who.data = connection->currentTarget->target.data;
            }
            /*
             * If authentication is required include an authentication challenge
             * in the connection response.
             */
            if (connection->authRequested) {
                OI_OBEXAUTH_ComposeResponse(&connection->common, &headers[headerCount++]);
            }
            /*
             * If the upper layer server wants to use SRM include the SRM header in the response.
             */
            if ((connection->common.srm & OI_OBEX_SRM_SUPPORTED) &&
                (connection->common.srm & OI_OBEX_SRM_REQUESTED)) {
                headers[headerCount].id = OI_OBEX_HDR_SINGLE_RESPONSE_MODE;
                headers[headerCount].val.srm = OI_OBEX_SRM_SUPPORTED;
                headerCount++;
                OI_DBGPRINT(("SRM supported for this connection"));
            }
        }
    } else {
        /*
         * Lower layer connection is still alive.
         */
        SetState(connection, LOWER_LAYER_CONNECTED);
        rsp = MapStatus(inStatus);
        /*
         * The upper did not accept so forget we indicated the connection
         */
        connection->connectIndicated = FALSE;
    }

    /*
     * Initialize the packet header.
     */
    OI_OBEXCOMMON_InitPacket(&connection->common, rsp, &pkt);
    ByteStream_PutUINT8(pkt, OI_OBEX_VERSION_NUMBER);
    ByteStream_PutUINT8(pkt, 0); /* flags */
    ByteStream_PutUINT16(pkt, connection->common.maxRecvPktLen, OI_OBEX_BO);

    status = OI_OBEXCOMMON_MarshalPacket(&connection->common, &pkt, headers, headerCount, optHeaders);

    if (OI_SUCCESS(status)) {
        status = ServerSendPacket(connection);
    }
    if (!OI_SUCCESS(status)) {
        goto ConnectRspError;
    }

    return OI_OK;

ConnectRspError:

    OI_SLOG_ERROR(status, ("acceptConnect error"));
    /*
     * The lower layer connection is still up, the OBEX connection is not.
     */
    SetState(connection, LOWER_LAYER_CONNECTED);
    OI_FreeIf(&connection->common.authentication);
    connection->connectIndicated = FALSE;
    return status;

}

OI_STATUS OI_OBEXSRV_AcceptConnect(OI_OBEXSRV_CONNECTION_HANDLE connectionId,
                                   OI_BOOL                      accept,
                                   OI_STATUS                    status,
                                   const OI_OBEX_HEADER_LIST    *optHeaders)
{
    OBEXSRV_CONNECTION *connection;

    OI_DBGTRACE(("OI_OBEXSRV_AcceptConnect (connectionId = %d, accept = %d, status = %d)\n",
                 connectionId, accept, status));

    if (!OI_INIT_FLAG_VALUE(OBEX_SRV)) {
        return OI_STATUS_NOT_INITIALIZED;
    }

    connection = LookupConnection(connectionId);
    if ((connection == NULL) || (connection->state != OBEX_CONNECTING)) {
        return OI_OBEX_INVALID_OPERATION;
    }

    if ((accept && !OI_SUCCESS(status)) || (!accept && OI_SUCCESS(status))) {
        OI_SLOG_ERROR(OI_STATUS_INVALID_PARAMETERS, ("Invalid combination of accept and status"));
        return OI_STATUS_INVALID_PARAMETERS;
    }

    status = acceptConnect(connectionId, NULL, accept, status, optHeaders);

    return status;
}

/*
 * Dispatch function for re-indicating a connection to the upper layer. This has
 * to be done via the dispatcher because a call into the stack cannot generate
 * a call out.
 */

static void ReIndicateConnect(DISPATCH_ARG *darg)
{
    OBEXSRV_CONNECTION *connection = Dispatch_GetArg(darg, OBEXSRV_CONNECTION*);

    OI_ASSERT(!connection->unauthorized);

    OI_DBGPRINT(("Re-indicating connection"));

    connection->connectIndicated = TRUE;
    connection->currentTarget->CBList->connectInd(connection->common.connectionHandle, FALSE, NULL, 0, NULL);
}


OI_STATUS OI_OBEXSRV_AuthenticationResponse(OI_OBEXSRV_CONNECTION_HANDLE connectionId,
                                            const OI_BYTE *userId,
                                            OI_UINT8 userIdLen,
                                            const OI_CHAR *password,
                                            OI_BOOL readOnly)
{
    OI_STATUS status;
    DISPATCH_ARG darg;
    OBEXSRV_CONNECTION *connection;

    OI_DBGTRACE(("OI_OBEXSRV_AuthenticationResponse(connectionId = %d, userId =  \"%s\", userIdLen = %d, password = \"%s\", readOnly = %s)",
                 connectionId, userId ? (OI_CHAR *)userId : "<null>", userIdLen, password ? password : "<null>", readOnly ? "TRUE" : "FALSE"));

    if (!OI_INIT_FLAG_VALUE(OBEX_SRV)) {
        return OI_STATUS_NOT_INITIALIZED;
    }

    /*
     * Check that we are connecting.
     */
    connection = LookupConnection(connectionId);
    if ((connection == NULL) || (connection->state != OBEX_CONNECTING)) {
        OI_DBGPRINT(("Authentication response not expected"));
        return OI_OBEX_INVALID_OPERATION;
    }

    if (!connection->unauthorized && !connection->authRequested) {
        OI_DBGPRINT(("Connection did not required authentication"));
        return OI_OBEX_INVALID_OPERATION;
    }

    /*
     * Password must be a null terminated string.
     */
    if (password == NULL) {
        password = "\0";
    }

    /*
     * Readonly is only passed to the client if the connection is authenticated.
     */
    connection->common.readOnly = readOnly;

    status = OI_OBEXAUTH_SaveAuthInfo(&connection->common, userId, userIdLen, password);
    if (OI_SUCCESS(status)) {
        /*
         * If authentication succeeds. re-indicate the connection to the
         * upper layer so that the connection can be accepted.
         */
        if (connection->unauthorized) {
            if (OI_OBEXAUTH_Authenticate(&connection->common) == OI_OK) {
                connection->unauthorized = FALSE;
                Dispatch_SetArg(darg, connection);
                OI_Dispatch_RegisterFunc(ReIndicateConnect, &darg, NULL);
            } else {
                if (connection->authTries < MAX_AUTHENTICATION_ATTEMPTS) {
                    /*
                     * Accept the connection to allow the authentication to proceed.
                     */
                    status = OI_OBEXSRV_AcceptConnect(connectionId, TRUE, OI_OK, NULL);
                } else {
                    OI_DBGPRINT(("Reached maximum number of authentication attempts"));
                    status = OI_OBEX_ACCESS_DENIED;
                }
            }
        } else {
            /*
             * Re-indicate the connection so the upperlayer can call OI_OBEXSRV_AcceptConnect to
             * accept or reject the connect request. We do this for consistency with the case of no
             * authentication.
             */
            Dispatch_SetArg(darg, connection);
            OI_Dispatch_RegisterFunc(ReIndicateConnect, &darg, NULL);
        }
    }

    /*
     * Reset the OBEX connection if we get an error.
     */
    if (!OI_SUCCESS(status)) {

        OI_STATUS connectStatus;

        OI_SLOG_ERROR(status, ("OI_OBEXSRV_AuthenticationResponse error"));

        connectStatus = acceptConnect(connectionId, NULL, FALSE, status, NULL);
        if (!OI_SUCCESS(connectStatus)) {
            OI_SLOG_ERROR(connectStatus, ("OI_OBEXSRV_AuthenticationResponse: Failure to send error reponse"));
        }
        SetState(connection, LOWER_LAYER_CONNECTED);
        OI_OBEXAUTH_Reset(&connection->common);
    }

    return status;

}

/**
 * Given two (possibly null) byteseq pointers that represent OBEX targets,
 * determine if they match. Two byteseqs match if they both have the same data,
 * or if both are NULL or have a NULL data pointer
 */

static OI_BOOL MatchTarget(const OI_OBEX_BYTESEQ *t1,
                           const OI_OBEX_BYTESEQ *t2)
{
    if (t1 && t2 && (t1->len == t2->len)) {
        return (OI_MemCmp(t1->data, t2->data, t1->len) == 0);
    }

    return ((t1 == NULL || t1->len == 0) && (t2 == NULL || t2->len == 0));
}


/**
 * Given a server connection block and the target header from the client (if
 * present), determine if a matching connection target can be found. If so, set
 * the currentTarget field, otherwise return an error.
 */

static OI_STATUS FindTarget(OBEX_SERVER *server,
                            const OI_OBEX_BYTESEQ *clientTarget,
                            OBEXSRV_TARGET_INFO **foundTarget)
{
    OBEXSRV_TARGET_INFO *target;

    for (target = &server->targetInfo; target != NULL; target = target->next) {
        if (MatchTarget(clientTarget, &target->target)) {
            *foundTarget = target;
            return OI_OK;
        }
    }
    return OI_OBEX_SERVICE_UNAVAILABLE;
}


/**
 * Handle an OBEX connect request.
 */

static OI_STATUS ConnectCommand(OBEXSRV_CONNECTION *connection,
                                OI_BYTE_STREAM *rcvPacket,
                                OI_UINT8 cmdCode)
{
    OI_BYTE *userIdRemote = NULL;
    OI_UINT8 userIdRemoteLen = 0;
    OI_OBEX_HEADER_LIST headers = { NULL, 0 };
    OI_OBEX_HEADER *target;
    OI_OBEX_HEADER *authChallenge = NULL;
    OI_OBEX_HEADER *authResponse = NULL;
    OI_STATUS status;
    OI_STATUS connectStatus;
    OI_UINT8 version = 0;
    OI_UINT8 flags = 0;
    OI_BOOL havePassword;
    OI_UINT16 maxSendPktLen = 0;

    /*
     * Reject a connect command if we are already connected.
     */
    if (connection->state != LOWER_LAYER_CONNECTED) {
        status = OI_OBEX_SERVICE_UNAVAILABLE;
        goto ConnectError;
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
        OI_SLOG_ERROR(status, ("OBEX Server connect - version mismatch %d.%d", OI_OBEX_MAJOR_VERSION_NUMBER(version), OI_OBEX_MINOR_VERSION_NUMBER(version)));
        goto ConnectError;
    }
    OI_DBGPRINT2(("OBEX Server connect - version %d.%d", OI_OBEX_MAJOR_VERSION_NUMBER(version), OI_OBEX_MINOR_VERSION_NUMBER(version)));

    ByteStream_GetUINT8_Checked(*rcvPacket, flags);
    if (flags != 0) {
        status = OI_OBEX_COMMAND_ERROR;
        goto ConnectError;
    }
    ByteStream_GetUINT16_Checked(*rcvPacket, maxSendPktLen, OI_OBEX_BO);
    if (ByteStream_Error(*rcvPacket)) {
        status = OI_OBEX_INCOMPLETE_PACKET;
        goto ConnectError;
    }

    OI_DBGPRINT2(("OBEX Server MaxSendPktLen was: %d, now %d", connection->common.maxSendPktLen, OI_MIN(maxSendPktLen, connection->common.maxSendPktLen)));
    connection->common.maxSendPktLen = OI_MIN(maxSendPktLen, connection->common.maxSendPktLen);

    /*
     * The OBEX specification defines a minimum packet length. Reject
     * connections that do not conform to the specification.
     */
    if (connection->common.maxSendPktLen < OI_OBEX_MIN_PACKET_SIZE) {
        status = OI_OBEX_VALUE_NOT_ACCEPTABLE;
        OI_SLOG_ERROR(status, ("OBEX Server connect - packet size too small %d", connection->common.maxSendPktLen));
        goto ConnectError;
    }

    OI_DBGPRINT2(("OBEX Server connect - requested packet len = %d", connection->common.maxSendPktLen));
    status = OI_OBEXCOMMON_ParseHeaderList(&connection->common, &headers, rcvPacket);
    if (!OI_SUCCESS(status)) {
        goto ConnectError;
    }

    /*
     * Cache headers before invoking any callbacks.
     */
    connection->common.pRawHeaders = &headers;

    /*
     * If the client has specified a target check that the server supports the
     * target. For the purposes of the comparison, NULL is a valid target and
     * is valid if the server supports the NULL target.
     */
    target = OI_OBEX_FindHeader(&headers, OI_OBEX_HDR_TARGET);
    status = FindTarget(connection->obexServer, target ? &target->val.target : NULL, &connection->currentTarget);
    if (!OI_SUCCESS(status)) {
        OI_SLOG_ERROR(status, ("Unable to find OBEX target %@", target ? target->val.target.data : NULL, target ? target->val.target.len : 0));
        goto ConnectError;
    }
    /*
     * We need to know if the client specified a target so we know whether to
     * use a connection id subsequent response packets.
     */
    connection->clientSpecifiedTarget = (target != NULL);
    /*
     * Authentication is required if the server requires authentication or if
     * the client included an authentication challenge in the connection
     * request.
     *
     * Check if we received authentication challenge from the client
     */
    authChallenge = OI_OBEX_FindHeader(&headers, OI_OBEX_HDR_AUTHENTICATION_CHALLENGE);

    if (authChallenge != NULL) {
        OI_DBGPRINT(("OBEX server received authentication challenge"));

        /*
         * If this this is the first authentication attempt we need to
         * allocate memory to manage the authentication state information.
         */
        if (connection->common.authentication == NULL) {
            connection->common.authentication = OI_Calloc(sizeof(OBEX_AUTHENTICATION));
            if (connection->common.authentication == NULL) {
                status = OI_STATUS_NO_RESOURCES;
                OI_SLOG_ERROR(status, ("OBEX server could not allocate authentication state"));
                goto ConnectError;
            }
        }

        OI_OBEXAUTH_SaveChallenge(&connection->common, authChallenge);

        connection->authRequested = TRUE;

    } else {
        connection->authRequested = FALSE;
    }

    if (connection->currentTarget->authRequired) {
        connection->common.authenticating = TRUE;
        /*
         * Until we know better assume that the connection is not authorized.
         */
        connection->unauthorized = TRUE;
        /*
         * If this this is the first authentication attempt we need to
         * allocate memory to manage the authentication state information.
         */
        if (connection->common.authentication == NULL) {
            connection->common.authentication = OI_Calloc(sizeof(OBEX_AUTHENTICATION));
            if (connection->common.authentication == NULL) {
                status = OI_STATUS_NO_RESOURCES;
                OI_SLOG_ERROR(status, ("OBEX server could not allocate authentication state"));
                goto ConnectError;
            }
        }

        if (connection->currentTarget->authRequired == OI_OBEXSRV_AUTH_USERID_AND_PASSWORD) {
            connection->common.authentication->userIdRequired = TRUE;
        }

        /*
         * We count the number of times authentication has been attempted on to
         * establish this connection. Reject the connection request is there are
         * too many failed attempts.
         */
        if (connection->authTries >= MAX_AUTHENTICATION_ATTEMPTS) {
            OI_DBGPRINT(("Reached maximum number of authentication attempts"));
            OI_OBEXAUTH_Reset(&connection->common);
            status = OI_OBEX_ACCESS_DENIED;
            goto ConnectError;
        }

        authResponse = OI_OBEX_FindHeader(&headers, OI_OBEX_HDR_AUTHENTICATION_RESPONSE);
        if (authResponse != NULL) {
            OI_OBEXAUTH_SaveResponse(&connection->common, authResponse);

            userIdRemoteLen = connection->common.authentication->userIdRemoteLen;
            if (userIdRemoteLen > 0) {
                userIdRemote = &connection->common.authentication->userIdRemote[0];
                OI_DBGPRINT2(("Connection includes user id %s (%d)", userIdRemote, userIdRemoteLen));
            }
            /*
             * Does the server require a user id as well as a password?
             */
            if (connection->common.authentication->userIdRequired && (userIdRemote == NULL)) {
                /*
                 * Reject the connection attempt.
                 */
                OI_DBGPRINT(("Server requires user id to authenticate"));
                OI_OBEXAUTH_Reset(&connection->common);
                status = OI_OBEX_ACCESS_DENIED;
                goto ConnectError;
            }
        }
    } else {
        /*
         * If the connection does not require authentication then it is
         * implicitly authorized.
         */
        connection->common.authenticating = FALSE;
        connection->unauthorized = FALSE;
    }
    /*
     * No longer need the received headers.
     */
    OI_FreeIf(&headers.list);
    /*
     * We are connecting.
     */
    SetState(connection, OBEX_CONNECTING);
    if (connection->unauthorized) {
        /*
         * Check if we have all the information we need to authenticate.
         */
        havePassword = (authResponse != NULL);
        if (!havePassword) {
            /*
             * If this is the first connect request and the upper layer provides an authentication
             * indication call it now.
             */
            if ((connection->authTries == 0) && connection->currentTarget->CBList->authInd) {
                OI_DBGPRINT(("Indicate auth"));
                connection->currentTarget->CBList->authInd(connection->common.connectionHandle, &connection->clientBdAddr);
                status = OI_OK;
            } else {
                /*
                 * Continue the connection process to get all the authentication information.
                 */
                status = OI_OBEXSRV_AcceptConnect(connection->common.connectionHandle, TRUE, OI_OK, NULL);
            }
            if (OI_SUCCESS(status)) {
                return status;
            } else {
                OI_SLOG_ERROR(status, ("OBEX server could not accept connection"));
                goto ConnectError;
            }
        }
    }
    ++connection->authTries;
    /*
     * Let application know that a connect request has been received. If
     * unauthorized is TRUE the upper layer must call back with a password.
     */
    connection->connectIndicated = TRUE;
    {
        OI_OBEX_REALM *pRealm = NULL;

        if ((NULL != connection->common.authentication) && (connection->common.authentication->realm.len > 0)) {
            pRealm = &connection->common.authentication->realm;
        }
        connection->currentTarget->CBList->connectInd(connection->common.connectionHandle,
                                                      connection->unauthorized | connection->authRequested,
                                                      userIdRemote,
                                                      userIdRemoteLen,
                                                      pRealm);
    }
    connection->common.pRawHeaders = NULL;
    return OI_OK;

ConnectError:

    OI_SLOG_ERROR(status, ("ConnectCommand error"));

    connectStatus = acceptConnect(connection->common.connectionHandle, NULL, FALSE, status, NULL);
    if (!OI_SUCCESS(connectStatus)) {
        OI_SLOG_ERROR(connectStatus, ("OBEX server failed to accept connection"));
    }
    OI_FreeIf(&connection->common.authentication);
    OI_FreeIf(&headers.list);
    connection->common.pRawHeaders = NULL;
    return status;
}


OI_STATUS OI_OBEXSRV_AuthAccept(OI_OBEXSRV_CONNECTION_HANDLE connectionId,
                                const OI_OBEX_REALM          *realm,
                                OI_BOOL                      accept)
{
    OBEXSRV_CONNECTION *connection;
    OI_STATUS status;

    OI_DBGTRACE(("OI_OBEXSRV_AuthAccept (connectionHandle = %d, accept = %d)", connectionId, accept));

    connection = LookupConnection(connectionId);
    if ((connection == NULL) || (connection->state != OBEX_CONNECTING)) {
        return OI_OBEX_INVALID_OPERATION;
    }
    if (realm && (realm->len > 254)) {
        OI_DBGPRINT(("Maximum length of realm string is 254 bytes"));
        return OI_STATUS_INVALID_PARAMETERS;
    }
    status = acceptConnect(connectionId, realm, accept, OI_OK, NULL);
    if (OI_SUCCESS(status)) {
        ++connection->authTries;
    } else {
        OI_SLOG_ERROR(status, ("OBEX server failed to accept connection"));
    }
    return status;
}


/**
 * Returns the current target associated with an OI_OBEXSRV_CONNECTION_HANDLE
 */

OI_STATUS OI_OBEXSRV_GetTarget(OI_OBEXSRV_CONNECTION_HANDLE connectionHandle,
                               OI_OBEX_BYTESEQ *target)
{
    OBEXSRV_CONNECTION *connection = LookupConnection(connectionHandle);

    OI_DBGTRACE(("OI_OBEXSRV_GetTarget (connectionHandle = %d, <*target = %x>)\n",
                 connectionHandle, target));

    if (connection == NULL) {
        OI_DBGPRINT(("No connection for handle %x", connectionHandle));
        return OI_OBEX_NOT_CONNECTED;
    }
    *target = connection->currentTarget->target;
    return OI_OK;
}


OI_OBEX_SERVER_HANDLE OI_OBEXSRV_GetServerHandle(OI_OBEXSRV_CONNECTION_HANDLE connectionHandle)
{
    OBEXSRV_CONNECTION *connection = LookupConnection(connectionHandle);

    if (connection) {
        return connection->obexServer->handle;
    } else {
        return NULL;
    }
}


/**
 * Returns the BD_ADDR associated with an OI_OBEXSRV_CONNECTION_HANDLE
 */

OI_STATUS OI_OBEXSRV_GetClientAddr(OI_OBEXSRV_CONNECTION_HANDLE connectionHandle,
                                   OI_BD_ADDR *pBdAddr)
{
    OBEXSRV_CONNECTION *connection = LookupConnection(connectionHandle);

    OI_DBGPRINT2(("OI_OBEXSRV_GetClientAddr (connectionHandle = %d, <*pBdAddr = %x>)\n",
                 connectionHandle, pBdAddr));

    if (connection == NULL) {
        OI_SLOG_ERROR(OI_OBEX_NOT_CONNECTED, ("No connection for handle %x", connectionHandle));
        return OI_OBEX_NOT_CONNECTED;
    }
    *pBdAddr = connection->clientBdAddr;
    return OI_OK;
}


/*
 * Data received from an OBEX client.
 */
static void LowerRecvDataInd(OI_OBEX_LOWER_CONNECTION lowerConnection,
                             OI_BYTE *dataBuf,
                             OI_UINT16 dataLen)
{
    OBEXSRV_CONNECTION *connection = (OBEXSRV_CONNECTION*)lowerConnection->context;
    OI_STATUS status;
    OI_UINT8 cmdCode;
    OI_BYTE_STREAM rcvPacket;

    if (connection == NULL) {
        OI_SLOG_ERROR(OI_OBEX_NOT_CONNECTED, ("OBEX received data for unknown connection"));
        return;
    }

    OI_DBGPRINT2(("OBEX server received data"));

    /* We already issued a disconnect, nothing to do */
    if (connection->state == OBEX_DISCONNECT_COMPLETING) {
        OI_SLOG_ERROR(OI_OBEX_NOT_CONNECTED, ("Received command after server issued a disconnect request"));
        return;
    }

    /*
     * The client should not be sending any OBEX commands after issuing a disconnect. If we get
     * spurious commands we will force a disconnect of the underlying lower layer connection and let
     * the badly behaving remote client deal with the consequences.
     */
    if (connection->state == OBEX_DISCONNECTING) {
        OI_SLOG_ERROR(OI_STATUS_INVALID_STATE, ("Received command after a disconnect"));
        SetState(connection, OBEX_DISCONNECT_COMPLETING);
        status = connection->common.lowerConnection->ifc->disconnect(connection->common.lowerConnection);
        if (!OI_SUCCESS(status)) {
            OI_SLOG_ERROR(status, ("Lower layer disconnect failed"));
            ServerDisconnect(connection, status);
        }
        return;
    }


    ByteStream_Init(rcvPacket, dataBuf, dataLen);
    ByteStream_Open(rcvPacket, BYTESTREAM_READ);
    /*
     * Get the OBEX response code and skip over the packet length.
     */
    ByteStream_GetUINT8(rcvPacket, cmdCode);
    ByteStream_Skip(rcvPacket, sizeof(OI_UINT16));

    switch (OI_OBEX_FINAL(cmdCode)) {
    case OI_OBEX_FINAL(OI_OBEX_CMD_CONNECT):
        status = ConnectCommand(connection, &rcvPacket, cmdCode);
        break;
    case OI_OBEX_FINAL(OI_OBEX_CMD_DISCONNECT):
        OI_DBGPRINT(("OBEX server received disconnect request"));
        /*
         * Acknowledge the disconnect request. The disconnect will complete
         * when the lower layer indicates that the response has been sent.
         */
        status = OI_OBEXCOMMON_SendOk(&connection->common);
        if (OI_SUCCESS(status)) {
            SetState(connection, OBEX_DISCONNECTING);
        } else {
            /*
             * Even if we are unable to respond to the disconnect request we
             * need to act on it.
             */
            OI_SLOG_ERROR(status, ("OI_OBEXCOMMON_SendOk failed"));
            SetState(connection, OBEX_DISCONNECT_COMPLETING);
            status = connection->common.lowerConnection->ifc->disconnect(connection->common.lowerConnection);
            if (!OI_SUCCESS(status)) {
                OI_SLOG_ERROR(status, ("Lower layer disconnect failed"));
                ServerDisconnect(connection, status);
            }
        }
        break;
    case OI_OBEX_FINAL(OI_OBEX_CMD_PUT):
        OI_DBGPRINT2(("PUT"));
        status = PutCommand(connection, &rcvPacket, OI_OBEX_IS_FINAL(cmdCode));
        break;
    case OI_OBEX_FINAL(OI_OBEX_CMD_GET):
        OI_DBGPRINT2(("GET"));
        status = GetCommand(connection, &rcvPacket, OI_OBEX_IS_FINAL(cmdCode));
        break;
    case OI_OBEX_FINAL(OI_OBEX_CMD_SET_PATH):
        OI_DBGPRINT2(("SETPATH"));
        status = SetPathCommand(connection, &rcvPacket);
        break;
    case OI_OBEX_FINAL(OI_OBEX_CMD_ACTION):
        OI_DBGPRINT2(("ACTION"));
        status = ActionCommand(connection, &rcvPacket);
        break;
    case OI_OBEX_FINAL(OI_OBEX_CMD_SESSION):
        OI_DBGPRINT2(("SESSION"));
        status = SessionCommand(connection, &rcvPacket);
        break;
    case OI_OBEX_FINAL(OI_OBEX_CMD_ABORT):
        OI_DBGPRINT2(("ABORT"));
        status = handleAbortCommand(connection, &rcvPacket);
        break;
    default:
        status = OI_OBEX_BAD_REQUEST;
        ErrorResponse(connection, status);
    }

    if (!OI_SUCCESS(status)) {
        OI_DBGPRINT(("OBEX request error %d", status));
    }
}


static void LowerConnectCfm(OI_OBEX_LOWER_CONNECTION lowerConnection,
                            OI_UINT16 recvMtu,
                            OI_UINT16 sendMtu,
                            OI_STATUS result)
{
    OBEXSRV_CONNECTION *connection = (OBEXSRV_CONNECTION*)lowerConnection->context;
    DISPATCH_ARG darg;
    OI_INTERVAL timeout;

    OI_DBGTRACE(("LowerConnectCfm %d recvMtu:%d sendMtu:%d", result, recvMtu, sendMtu));

    if (connection == NULL) {
        OI_SLOG_ERROR(OI_STATUS_INVALID_STATE, ("Null connection handle!"));
        return;
    }

    if (OI_SUCCESS(result)) {
        OI_DBGPRINT(("OBEX server lower layer connection confirmed"));
        /*
         * Maximum OBEX packet size depends on the size of the lower layer MTU
         */
        connection->common.maxRecvPktLen = OI_MIN(recvMtu, connection->common.maxRecvPktLen);
        connection->common.maxSendPktLen = sendMtu;
        OI_DBGPRINT(("maxRecvPktLen = %d, maxSendPktLen = %d", connection->common.maxRecvPktLen, connection->common.maxSendPktLen));

        /*
         * SRM is supported if it was enabled when the server was registered and the lower protocol
         * for the connection is OBEX/L2CAP
         */
        if ((connection->obexServer->srm & OI_OBEX_SRM_SUPPORTED) &&
            (connection->common.lowerConnection->ifc->getProtocol(
            connection->common.lowerConnection) == OI_OBEX_LOWER_L2CAP)) {
            connection->common.srm = OI_OBEX_SRM_SUPPORTED;
        } else {
            connection->common.srm &= ~OI_OBEX_SRM_SUPPORTED;
        }
        /*
         * Finalize the connection. Most of the intialization was performed in
         * LowerConnectInd() when the connection was indicated.
         */
        SetState(connection, LOWER_LAYER_CONNECTED);
        /*
         * If there is an existing timeout cancel it.
         */
        if (0 != connection->connectTimeout) {
            OI_Dispatch_CancelFunc(connection->connectTimeout);
        }
        /*
         * Allow for longer timeout for password entry if authentication is required.
         */
        timeout = (connection->obexServer->targetInfo.authRequired) ? OI_CONFIG_TABLE_GET(OBEX_SRV)->authTimeout : OI_CONFIG_TABLE_GET(OBEX_SRV)->connectTimeout;

        Dispatch_SetArg(darg, connection->common.connectionHandle);
        OI_Dispatch_RegisterTimedFunc(ConnectTimeout, &darg, timeout, &connection->connectTimeout);

    } else {
        /*
         * Free any dynamically allocated memory and release the connection
         * entry.
         */
        OI_SLOG_ERROR(result, ("OBEX server lower layer connection failed"));
        /*
         * Unlink the connection from the server and release the handle
         */
        OI_List_Del(&connection->link);
        OI_HANDLE_Free(connection->common.connectionHandle);
        OI_Free(connection);
    }
}


/**
 * Called by the lower layer when a client attempts to establish a connection to a registered OBEX
 * server. We have to either accept or reject the request.
 */

static void LowerConnectInd(OI_OBEX_LOWER_SERVER lowerServer,
                            OI_OBEX_LOWER_CONNECTION lowerConnection,
                            OI_BD_ADDR *addr)
{
    OBEX_SERVER *server = (OBEX_SERVER*)lowerServer->context;
    OBEXSRV_CONNECTION *connection;
    OI_STATUS status;

    OI_DBGTRACE(("LowerConnectInd %:", addr));


    connection = OI_Calloc(sizeof(*connection));
    if (!connection) {
        OI_SLOG_ERROR(OI_STATUS_NO_RESOURCES, ("Failed to accepting connection from %:", addr));
        goto RejectConnection;
    }
    /*
     * Allocate a connection handle and associate it with the connection record
     */
    connection->common.connectionHandle = OI_HANDLE_Alloc(ObexSrvConnectionHandleType, connection);
    if (!connection->common.connectionHandle) {
        OI_SLOG_ERROR(OI_STATUS_NO_RESOURCES, ("Failed to accepting connection from %:", addr));
        goto RejectConnection;
    }
    /*
     * Save connecting device's bdaddr and the lower layer connection
     */
    connection->clientBdAddr = *addr;
    connection->common.lowerConnection = lowerConnection;
    connection->obexServer = server;
    /*
     * Inherit SRM flags from the server registration
     */
    connection->common.srm = server->srm;
    /*
     * Until we negotiate maximum  packet size for sending on this connection
     * use the default minimum packet sizes.
     */
    connection->common.maxSendPktLen = OI_OBEX_MIN_PACKET_SIZE;
    /*
     * The maximum packet size we can receive is specified by a configuration
     * parameter.
     */
    connection->common.maxRecvPktLen = OI_CONFIG_TABLE_GET(OBEX_SRV)->maxPktLen;
    /*
     * Accept the link request.
     */
    connection->state = LOWER_LAYER_CONNECTING;
    status = lowerServer->ifc->accept(lowerConnection, TRUE);
    if (!OI_SUCCESS(status)) {
        OI_SLOG_ERROR(status, ("OBEX server error accepting link request from %:", addr));
        goto RejectConnection;
    }
    /*
     * Associate this OBEX connection with the lower connection.
     */
    lowerConnection->context = connection;
    /*
     * Add the connection to the server's connection list
     */
    OI_List_Add(&connection->link, &server->connectionList);
    return;


RejectConnection:

    if (connection) {
        if (connection->common.connectionHandle) {
            OI_HANDLE_Free(connection->common.connectionHandle);
        }
        OI_Free(connection);
    }
    /*
     * Reject the connection request.
     */
    OI_DBGPRINT(("OBEX server rejecting link request from %:", addr));
    (void) lowerConnection->ifc->accept(lowerConnection, FALSE);
}


OI_STATUS OI_OBEXSRV_ConfirmSetpath(OI_OBEXSRV_CONNECTION_HANDLE    connectionId,
                                    OI_STATUS                       status,
                                    const OI_OBEX_HEADER_LIST       *optHeaders)
{
    OBEXSRV_CONNECTION *connection = LookupConnection(connectionId);

    OI_DBGTRACE(("OI_OBEXSRV_ConfirmSetpath (connectionId = %d, status = %d)\n",
                 connectionId, status));

    if ((connection == NULL) || (connection->state != SETTING_PATH)) {
        return OI_OBEX_INVALID_OPERATION;
    }
    SetState(connection, OBEX_CONNECTED);
    if (OI_SUCCESS(status)) {
        status = sendSimpleWithHeaders(&connection->common, OI_OBEX_FINAL(OI_OBEX_RSP_OK), optHeaders);
    } else {
        ErrorResponse(connection, status);
        status = OI_OK;
    }
    return status;
}


OI_UINT16 OI_OBEXSRV_OptimalBodyHeaderSize(OI_OBEXSRV_CONNECTION_HANDLE connectionId)
{
    OI_UINT16   optimalSize;
    OBEXSRV_CONNECTION *connection = LookupConnection(connectionId);

    /*
     * We don't know the packet length until we are connected.
     */
    if ((connection == NULL) || (connection->state < OBEX_CONNECTING)) {
        optimalSize = 0;
    } else {
        optimalSize = connection->common.maxSendPktLen - OI_OBEX_BODY_PKT_OVERHEAD;
    }
    OI_DBGPRINT2(("OI_OBEXSRV_OptimalBodyHeaderSize (connectionId %d) is %d\n", connectionId, optimalSize));
    return optimalSize;
}


/**
 * Registers the OBEX server to accept put and/or get requests from OBEX
 * clients.
 */

OI_STATUS OI_OBEXSRV_RegisterServer(const OI_OBEX_BYTESEQ *target,
                                    const OI_OBEXSRV_CB_LIST *CBList,
                                    OI_OBEXSRV_AUTHENTICATION authentication,
                                    OI_OBEX_CONNECTION_OPTIONS *connectOptions,
                                    OI_OBEX_LOWER_PROTOCOL *lowerProtocolList,
                                    OI_UINT8 numProtocols,
                                    const OI_CONNECT_POLICY *policy,
                                    OI_OBEX_SERVER_HANDLE *serverHandle)
{
    OI_UINT i;
    OI_STATUS status = OI_STATUS_INVALID_PARAMETERS;
    OBEX_SERVER *server = NULL;
    OI_UINT numRegistered = 0;

    OI_ARGCHECK(policy);
    OI_ARGCHECK(CBList);
    OI_ARGCHECK(lowerProtocolList);
    OI_ARGCHECK(serverHandle);
    OI_ARGCHECK(numProtocols > 0);

    OI_DBGTRACE(("OI_OBEXSRV_RegisterServer target:%x authentication:%s", target, OI_OBEXSRV_AuthenticationText(authentication)));

    if ((CBList == NULL) || ((target != NULL) && (target->data == NULL))) {
        return OI_STATUS_INVALID_PARAMETERS;
    }

    OI_DBGPRINT2(("Target: %@", target ? target->data : NULL, target ? target->len : 0));

    server = OI_Calloc(sizeof(OBEX_SERVER));
    if (server == NULL) {
        status = OI_STATUS_NO_RESOURCES;
        goto RegistrationFailed;
    }
    server->handle = OI_HANDLE_Alloc(ObexServerHandleType, server);
    if (!server->handle) {
        status = OI_STATUS_NO_RESOURCES;
        goto RegistrationFailed;
    }
    OI_List_DynamicInit(&server->connectionList);

    server->lowerProtocolList = OI_Calloc(sizeof(OI_OBEX_LOWER_SERVER) * (numProtocols + 1));

    if (server->lowerProtocolList == NULL) {
        status = OI_STATUS_NO_RESOURCES;
        goto RegistrationFailed;
    }

    /*
     * Is the upper layer server requesting SRM?
     */
    if (connectOptions && connectOptions->enableSRM) {
        server->srm = OI_OBEX_SRM_SUPPORTED;
    }
#ifdef OI_TEST_HARNESS
    OI_OBEX_TestInit();
    if (OI_ObexTest.disableSRM) {
        server->srm &= ~OI_OBEX_SRM_SUPPORTED;
    }
#endif
    /*
     * Registration with one or more lower protocols
     */
    for (i = 0; i < numProtocols; ++i) {
        /*
         * Register the server with the lower layer handler.
         */
        status = OI_OBEX_LOWER_RegisterServer(&lowerCallbacks,
                                              OI_CONFIG_TABLE_GET(OBEX_SRV)->maxPktLen,
                                              policy,
                                              &lowerProtocolList[i],
                                              &server->lowerProtocolList[i]);


        if (OI_SUCCESS(status)) {
            /*
             * Store the OBEX server connection struct in the lower layer server handle.
             */
            server->lowerProtocolList[i]->context = server;
            ++numRegistered;
        } else {
            lowerProtocolList[i].protocol = OI_OBEX_LOWER_NONE;
        }
    }
    /*
     * Check that we successfully registered at least one lower layer protocol.
     */
    if (numRegistered == 0) {
        status = OI_STATUS_NO_RESOURCES;
        OI_SLOG_ERROR(status, ("Failed to register lower layer protocol"));
        goto RegistrationFailed;
    }

    if (target != NULL) {
        server->targetInfo.target.data = target->data;
        server->targetInfo.target.len = target->len;
    }

    server->targetInfo.CBList = CBList;
    server->targetInfo.authRequired = authentication;

    OI_INIT_FLAG_INCREMENT(OBEX_SRV);

    *serverHandle = server->handle;

    return OI_OK;

RegistrationFailed:

    OI_SLOG_ERROR(status, ("OI_OBEXSRV_RegisterServer error"));

    if (server) {

        if (server->lowerProtocolList) {
            /*
             * Deregister any lower servers that were registered.
             */
            for (i = 0; i < numProtocols; ++i) {
                if (server->lowerProtocolList[i]) {
                    server->lowerProtocolList[i]->ifc->deregServer(server->lowerProtocolList[i]);
                }
            }
            OI_Free(server->lowerProtocolList);
        }

        if (server->handle) {
            OI_HANDLE_Free(server->handle);
        }
        OI_Free(server);
    }
    return status;
}


/**
 * This function registers an additional target on an existing OBEX server.
 */
OI_STATUS OI_OBEXSRV_RegisterSecondaryTarget(const OI_OBEX_BYTESEQ *target,
                                             const OI_OBEXSRV_CB_LIST *CBList,
                                             OI_OBEXSRV_AUTHENTICATION authentication,
                                             OI_OBEX_SERVER_HANDLE serverHandle)
{
    OBEX_SERVER *server = (OBEX_SERVER*)OI_HANDLE_Deref(serverHandle);
    OBEXSRV_TARGET_INFO *targetInfo;

    OI_DBGTRACE(("OI_OBEXSRV_RegisterSecondaryTarget target:%x authentication:%s", target, OI_OBEXSRV_AuthenticationText(authentication)));

    OI_ARGCHECK(target);
    OI_ARGCHECK(CBList);

    if (server == NULL) {
        return OI_STATUS_INVALID_HANDLE;
    }

    OI_DBGPRINT2(("Target: %@", target ? target->data : NULL, target ? target->len : 0));

    if (FindTarget(server, target, &targetInfo) == OI_OK) {
        return OI_STATUS_ALREADY_REGISTERED;
    }
    targetInfo = OI_Calloc(sizeof(*targetInfo));
    if (targetInfo == NULL) {
        return OI_STATUS_NO_RESOURCES;
    }
    targetInfo->target.data = target->data;
    targetInfo->target.len = target->len;

    targetInfo->CBList = CBList;
    targetInfo->authRequired = authentication;
    targetInfo->next = server->targetInfo.next;
    server->targetInfo.next = targetInfo;

    return OI_OK;
}


/*
 * Forcibly sever the connection to an OBEX client.
 */
OI_STATUS OI_OBEXSRV_ForceDisconnect(OI_OBEXSRV_CONNECTION_HANDLE connectionHandle)
{
    OBEXSRV_CONNECTION *connection;

    OI_DBGTRACE(("OI_OBEXSRV_ForceDisconnect"));

    connection = LookupConnection(connectionHandle);
    if (connection == NULL) {
        return OI_STATUS_INVALID_HANDLE;
    }
    if (connection->state == DISCONNECTED) {
        return OI_OBEX_NOT_CONNECTED;
    }
    /*
     * This will let us report the reason for the disconnect in any callbacks
     * that are pending.
     */
    connection->forcedDisconnect = TRUE;

    SetState(connection, OBEX_DISCONNECT_COMPLETING);
    return connection->common.lowerConnection->ifc->disconnect(connection->common.lowerConnection);
}


/*
 * Deregisters the OBEX server so that it will no longer accept put and/or get
 * requests from OBEX clients. All targets for the service are deregistered.
 *
 * The server cannot be deregistered if there is a client connected. The
 * application must first force a disconnect and then deregister the server.
 */

OI_STATUS OI_OBEXSRV_DeregisterServer(OI_OBEX_SERVER_HANDLE serverHandle)
{
    OBEX_SERVER *server = (OBEX_SERVER*)OI_HANDLE_Deref(serverHandle);
    OI_STATUS status = OI_OK;
    OI_OBEX_LOWER_SERVER *list;
    OBEXSRV_TARGET_INFO *target;

    OI_DBGTRACE(("OI_OBEXSRV_DeregisterServer handle:%x", serverHandle));

    if (server == NULL) {
        return OI_STATUS_INVALID_HANDLE;
    }
    /*
     * Check there are no connections using this server
     */
    if (!OI_List_IsEmpty(&server->connectionList)) {
        status = OI_STATUS_STILL_CONNECTED;
        OI_SLOG_ERROR(status, ("OI_OBEXSRV_DeregisterServer failed"));
        return status;
    }
    /*
     * Deregister all the lower protocols for this OBEX server
     */
    for (list = server->lowerProtocolList; *list != NULL; ++list) {
        status = (*list)->ifc->deregServer(*list);
        if (!OI_SUCCESS(status)){
            OI_SLOG_ERROR(status, ("Lower layer deregister failed"));
        }
    }
    OI_Free(server->lowerProtocolList);
    /*
     * Free all secondary targets associated with this server
     */
    target = server->targetInfo.next;
    while (target != NULL) {
        OBEXSRV_TARGET_INFO *goner;
        goner = target;
        target = target->next;
        OI_Free(goner);
    }
    OI_HANDLE_Free(serverHandle);
    OI_Free(server);
    OI_INIT_FLAG_DECREMENT(OBEX_SRV);
    return status;
}


OI_STATUS OI_OBEXSRV_GetLowerProtocolInfo(OI_OBEXSRV_CONNECTION_HANDLE connectionHandle,
                                          OI_OBEX_LOWER_PROTOCOL *lowerProtocol)
{
    OBEXSRV_CONNECTION *connection = LookupConnection(connectionHandle);

    OI_ARGCHECK(lowerProtocol);

    if ((connection == NULL) || (connection->state < LOWER_LAYER_CONNECTED)) {
        return OI_STATUS_NOT_CONNECTED;
    } else {
        *lowerProtocol = connection->common.lowerConnection->server->lowerProtocol;
        return OI_OK;
    }
}


OI_BOOL OI_OBEXSRV_IsServerBusy(OI_OBEX_SERVER_HANDLE serverHandle)
{
    OBEX_SERVER *server = (OBEX_SERVER*)OI_HANDLE_Deref(serverHandle);

    if (server == NULL) {
        OI_DBGTRACE(("OI_OBEXSRV_IsServerBusy invalid server handle"));
        return FALSE;
    } else {
        OI_DBGTRACE(("OI_OBEXSRV_IsServerBusy %d", !OI_List_IsEmpty(&server->connectionList)));
        return !OI_List_IsEmpty(&server->connectionList);
    }
}


void* OI_OBEXSRV_GetConnectionContext(OI_OBEXSRV_CONNECTION_HANDLE connectionId)
{
    OBEXSRV_CONNECTION *connection = LookupConnection(connectionId);


    if (connection == NULL) {
        OI_DBGPRINT2(("OI_OBEXSRV_GetConnectionContext invalid connection handle"));
        return NULL;
    } else {
        OI_DBGPRINT2(("OI_OBEXSRV_GetConnectionContext connectionId:%d context:%x", connectionId, connection->context));
        return connection->context;
    }
}

OI_STATUS OI_OBEXSRV_SetConnectionContext(OI_OBEXSRV_CONNECTION_HANDLE connectionId,
                                          void *context)
{
    OBEXSRV_CONNECTION *connection = LookupConnection(connectionId);

    if (connection == NULL) {
        OI_SLOG_ERROR(OI_STATUS_INVALID_HANDLE, ("OI_OBEXSRV_SetConnectionContext invalid connection handle"));
        return OI_STATUS_INVALID_HANDLE;
    } else {
        OI_DBGPRINT2(("OI_OBEXSRV_SetConnectionContext connectionId:%d context:%x", connectionId, context));
        connection->context = context;
        return OI_OK;
    }
}


OI_STATUS OI_OBEXSRV_SetServerContext(OI_OBEX_SERVER_HANDLE serverHandle,
                                      void *context)
{
    OBEX_SERVER *server = (OBEX_SERVER*)OI_HANDLE_Deref(serverHandle);

    if (server) {
        OI_DBGPRINT2(("OI_OBEXSRV_SetServerContext serverHandle:%d context:%x", serverHandle, context));
        server->context = context;
        return OI_OK;
    } else {
        OI_SLOG_ERROR(OI_STATUS_INVALID_HANDLE, ("OI_OBEXSRV_SetServerContext invalid server handle"));
        return OI_STATUS_INVALID_HANDLE;
    }

}


void* OI_OBEXSRV_GetServerContext(OI_OBEX_SERVER_HANDLE serverHandle)
{
    OBEX_SERVER *server = (OBEX_SERVER*)OI_HANDLE_Deref(serverHandle);

    OI_DBGPRINT2(("OI_OBEXSRV_GetServerContext"));

    if (server) {
        OI_DBGPRINT2(("OI_OBEXSRV_GetServerContext serverHandle:%d context:%x", serverHandle, server->context));
        return server->context;
    } else {
        OI_DBGPRINT2(("OI_OBEXSRV_GetServerContext invalid server handle"));
        return NULL;
    }
}


OI_UINT OI_OBEXSRV_GetNumConnections(OI_OBEX_SERVER_HANDLE serverHandle)
{
    OBEX_SERVER *server = (OBEX_SERVER*)OI_HANDLE_Deref(serverHandle);
    if (server) {
        return OI_List_CountElements(&server->connectionList);
    } else {
        return 0;
    }
}


OI_STATUS OI_OBEXSRV_GetL2capCID(OI_OBEXSRV_CONNECTION_HANDLE connectionId,
                                 OI_L2CAP_CID *cid)
{
    OI_STATUS status = OI_STATUS_NOT_FOUND;
    OBEXSRV_CONNECTION *connection = LookupConnection(connectionId);

    OI_ARGCHECK(cid != NULL);

    if (connection) {
        switch (connection->state) {
        case DISCONNECTED:
        case LOWER_LAYER_CONNECTING:
        case OBEX_DISCONNECTING:
        case OBEX_DISCONNECT_COMPLETING:
            status = OI_STATUS_NOT_CONNECTED;
            break;
        default:
            *cid = connection->common.lowerConnection->ifc->getCID(connection->common.lowerConnection);
            status = OI_OK;
            break;
        }
    }
    return status;
}


OI_STATUS OI_OBEXSRV_GetRawHeaderList(OI_OBEXSRV_CONNECTION_HANDLE connectionId,
                                      OI_OBEX_HEADER_LIST          **pRawHeaderList)
{
    OBEXSRV_CONNECTION *connection = LookupConnection(connectionId);

    OI_ARGCHECK(pRawHeaderList);

    if (connection == NULL) {
        return OI_OBEX_INVALID_OPERATION;
    }

    *pRawHeaderList = connection->common.pRawHeaders;

    return OI_OK;
}


static void dispatchedAbortCommand(DISPATCH_ARG *darg)
{
    OI_OBEXSRV_CONNECTION_HANDLE connectionHandle = Dispatch_GetArg(darg, OI_OBEXSRV_CONNECTION_HANDLE);
    OBEXSRV_CONNECTION *connection = LookupConnection(connectionHandle);

    if (connection) {
        connection->abortPending = FALSE;       /* Unblock bulk confirms */
        AbortOperation(connection);
    } else {
        OI_SLOG_ERROR(OI_OBEX_NOT_CONNECTED, ("dispatchedAbortCommand"));
    }
}


OI_STATUS OI_OBEXSRV_AbortResponse(OI_OBEXSRV_CONNECTION_HANDLE connectionId,
                                   OI_OBEX_HEADER_LIST          *rspHeaders,
                                   OI_STATUS                    rspStatus)
{
    OI_STATUS           status = OI_OK;
    OBEXSRV_CONNECTION  *connection = LookupConnection(connectionId);

    if (connection == NULL) {
        return OI_OBEX_INVALID_OPERATION;
    }
    if (!connection->abortResponsePending) {
        return OI_OBEX_INVALID_OPERATION;
    }

    /*
     *  We have been waiting for this.
     */
    connection->abortResponsePending = FALSE;           /* No longer waiting for response */
    if (rspHeaders) {
        /*
         *  Application wants optional headers sent with the abort response.
         *  Build the abort response packet for later use by the abort handler
         */
        OI_BYTE_STREAM  pkt;

        OI_OBEXCOMMON_InitPacket(&connection->common, OI_OBEX_FINAL(rspStatus), &pkt);
        status = OI_OBEXCOMMON_MarshalPacketMbuf(&connection->common, &pkt, NULL, 0, rspHeaders, &connection->abortMbuf);
        if (!OI_SUCCESS(status)) {
            OI_SLOG_ERROR(status, ("OI_OBEXCOMMON_MarshalPacketMbuf"));
        }
    } else {
        /*
         *  Build simple response, no headers
         */
        OI_BYTE cmdBuf[3];

        cmdBuf[0] = OI_OBEX_FINAL(rspStatus);
        cmdBuf[1] = 0;
        cmdBuf[2] = sizeof(cmdBuf);
        connection->abortMbuf = OI_MBUF_Wrap(cmdBuf, sizeof(cmdBuf), MBUF_COPY);
    }

    if (OI_SUCCESS(status)) {
        /*
         *  Now we want to actually process the abort command, but that needs to
         *  run on dispatcher thread because we may be calling other application callbacks
         *  in the course of executing abort.
         */
        DISPATCH_ARG darg;

        Dispatch_SetArg(darg, connection->common.connectionHandle);
        status = OI_Dispatch_RegisterFunc(dispatchedAbortCommand, &darg, NULL);
    }
    return status;
}
/*****************************************************************************/
