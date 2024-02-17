#ifndef _OI_OBEXCLI_H
#define _OI_OBEXCLI_H

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
 This file provides the API for the OBEX (GOEP) client
*/

#include "oi_stddefs.h"
#include "oi_status.h"
#include "oi_obex.h"
#include "oi_obexspec.h"
#include "oi_connect_policy.h"
#include "oi_bt_profile_config.h"

/** \addtogroup OBEX */
/**@{*/

#ifdef __cplusplus
extern "C" {
#endif



/**
 *  represents an active connection to a remote OBEX server
 */

typedef OI_OBEX_CONNECTION_HANDLE OI_OBEXCLI_CONNECTION_HANDLE;


/**
 * OBEX client required authentication
 */

typedef enum {
    OI_OBEXCLI_AUTH_NONE = 0,
    OI_OBEXCLI_AUTH_PASSWORD,
    OI_OBEXCLI_AUTH_USERID_AND_PASSWORD
} OI_OBEXCLI_AUTHENTICATION;

/**
 * A callback function of this type is called when an OBEX connection request completes.
 * The status code indicates if the connection was established.
 *
 * @param connectionId   a unique ID that represents a connection to an OBEX server
 *
 * @param readOnly       if TRUE, indicates that the server is only granting read access
 *                       to the server's resources to this client. Access rights
 *                       are only passed  on authenticated connections so this
 *                       parameter will only ever be TRUE for authenticated
 *                       connections. A FALSE value does not necessarily mean
 *                       that the client has write access to the server.
 *
 * @param status         OI_OK if successful; error status if the connection attempt failed
 *
 */

typedef void (*OI_OBEXCLI_CONNECT_CFM)(OI_OBEXCLI_CONNECTION_HANDLE connectionId,
                                       OI_BOOL readOnly,
                                       OI_STATUS status);

/**
 * A callback function of this type is called when an OBEX disconnect request completes,
 * whether requested by a call to OI_OBEXCLI_Disconnect() or because the connection was dropped.
 *
 * @param connectionId  the (now invalid) connection to an OBEX server
 */

typedef void (*OI_OBEXCLI_DISCONNECT_IND)(OI_OBEXCLI_CONNECTION_HANDLE connectionId);


/**
 * A callback function of this type is registered by OI_OBEXCLI_Put() and called when OBEX is ready for
 * more data to put.
 *
 * @param connectionId    This parameter is a unique ID that represents an established connection
 *                        to an OBEX server.
 *
 * @param rspHeaders      This parameter is a pointer to a list of OBEX headers received from the
 *                        OBEX server. Storage for rspHeaders is allocated by OBEX and must not be
 *                        freed by the caller. The caller should copy any data that is needed after
 *                        this function returns into a local buffer.
 *
 * @param status          OI_OK if the put transaction is complete.
 *                        OI_OBEX_CONTINUE if the application is being asked for more data to put;
 *                        OI_OBEX_NOT_READY if the put is incomplete and OBEX is not ready for
 *                        more data.
 *                        or an error status indicating that the transaction has been canceled and why
 */

typedef void (*OI_OBEXCLI_PUT_CFM)(OI_OBEXCLI_CONNECTION_HANDLE connectionId,
                                   OI_OBEX_HEADER_LIST *rspHeaders,
                                   OI_STATUS status);


/**
 * A callback function of this type is registered by OI_OBEXCLI_BulkPut() and called when OBEX has
 * put one or more bulk data buffers and is no longer using these buffers.
 *
 * @param connectionId    This parameter is a unique ID that represents an established connection
 *                        to an OBEX server.
 *
 * @param numBuffers      The number of buffers being confirmed by this call.
 *
 * @param bulkDataBuffer  This parameter is an array of pointers to data buffers that are not longer
 *                        being used by OBEX. Memory for these buffers was allocated by the caller
 *                        to OI_OBEXCLI_BulkPut() and can now be freed or reused by the caller.
 *
 * @param bufferLen       This parameter is an array of lengths of data buffers being returned.
 *
 * @param status        - OI_OK if the PUT transaction has sucesfully completed.
 *                      - OI_OBEX_CONTINUE if the PUT transaction is incomplete and the
 *                        application is permitted to put more data immediately.
 *                      - Error status code indicating that the PUT transaction is canceled.
 */

typedef void (*OI_OBEXCLI_BULK_PUT_CFM)(OI_OBEXCLI_CONNECTION_HANDLE connectionId,
                                        OI_UINT8 numBuffers,
                                        OI_UINT8 *bulkDataBuffer[],
                                        OI_UINT32 bufferLen[],
                                        OI_STATUS status);

/**
 * A callback function of this type is called when an OBEX get request returns data.
 *
 * @param connectionId    This parameter is a unique ID that represents an established connection
 *                        to an OBEX server.
 *
 * @param rspHeaders      This parameter is a pointer to a list of OBEX headers received from the
 *                        OBEX server. Storage for rspHeaders is allocated by OBEX and must not be
 *                        freed by the caller. The caller should copy any data that is needed after
 *                        this function returns into a local buffer.
 *
 * @param status          OI_OK if the put transaction is complete (if this is the last block of data);
 *                        OI_OBEX_CONTINUE if the application is being asked for more data to put;
 *                        or an error status indicating that the transaction has been canceled and why
 */

typedef void (*OI_OBEXCLI_GET_RECV_DATA)(OI_OBEXCLI_CONNECTION_HANDLE connectionId,
                                         OI_OBEX_HEADER_LIST *rspHeaders,
                                         OI_STATUS status);


/**
 * A callback function of this type is called when a setpath command completes.
 *
 * @param connectionId   This parameter is a unique ID that represents an established connection
 *                       to an OBEX server.
 *
 * @param status         OI_OK if the setpath operation succeeded; otherwise, error status indicating why the operation failed
 */

typedef void (*OI_OBEXCLI_SETPATH_CFM)(OI_OBEXCLI_CONNECTION_HANDLE connectionId,
                                       OI_STATUS status);


/**
 * A callback function of this type is called when an action command completes.
 *
 * @param connectionId   This parameter is a unique ID that represents an established connection
 *                       to an OBEX server.
 *
 * @param status         OI_OK if the action operation succeeded; otherwise, error status indicating why the operation failed
 */

typedef void (*OI_OBEXCLI_ACTION_CFM)(OI_OBEXCLI_CONNECTION_HANDLE connectionId,
                                      OI_STATUS status);


/**
 * A callback function of this type is made to the application to request a password and optional user id
 * if OBEX authentication is required on connections from this client.
 *
 * @param connectionId     a unique ID that represents the in-progress connection
 *
 * @param userIdRequired   depending on the calling context indicates either:
 *                          - that the server requested a user id as well
 *                            as a password. The client can provide a user id even
 *                            if the server does not require one. Whether the server
 *                            actually makes use of the user id in this case
 *                            depends entirely on the server implementation.
 *                          or
 *                          - that the client requires a user id to authorize the server
 *                            in client-initiated authentication scenario.
 *
 * @param realm            Indicates the OBEX server's realm for the password or NULL
 */
typedef void (*OI_OBEXCLI_AUTH_CHALLENGE_IND)(OI_OBEXCLI_CONNECTION_HANDLE  connectionId,
                                              OI_BOOL                       userIdRequired,
                                              OI_OBEX_REALM                 *realm);


/**
 * A callback function of this type can be registered when a connection is establishe to allow an
 * upper layer to monitor progress of PUT and GET operations.
 *
 * @param connectionId     a unique ID that rep2Yesents the in-progress connection
 *
 * @param obexCmd          Indicates if this is a put (OI_OBEX_CMD_PUT) or a get (OI_OBEX_CMD_GET)
 *
 * @param progressBytes    A running count of the number or bytes transferred.
 */
typedef void (*OI_OBEXCLI_PROGRESS_IND)(OI_OBEXCLI_CONNECTION_HANDLE connectionId,
                                        OI_UINT8 obexCmd,
                                        OI_UINT32 progressBytes);




/**
 * A callback function of this type is called when an OBEX abort request completes.
 *
 * @param connectionId  a unique ID that represents the in-progress connection
 */
typedef void (*OI_OBEXCLI_ABORT_CFM)(OI_OBEXCLI_CONNECTION_HANDLE connectionId);


/**
 * This function handles authentication data to generate authentication challenge and/or response
 * in connection request. A NULL password indicates that the application is rejecting the authentication challenge.
 *
 * @param connectionId    This parameter is a unique ID that represents an established connection
 *                        to an OBEX server.
 *
 * @param userId          A user id must be provided if the server indicated
 *                        that user ids are required in the authentication
 *                        challenge. A client can provide a user id even if the
 *                        server does not required one.
 *
 * @param userIdLen       The actual length of the user id.
 *
 * @param password        a NULL-terminated password string.
 *
 * @param realm           Indicates the Client's realm to be sent to server in Client's
 *                        authentication challenge or NULL
 *
 * @return                OI_OK if successful or error status if this function
 *                        failed or was called at the wrong time.
 */
OI_STATUS OI_OBEXCLI_Authentication(OI_OBEXCLI_CONNECTION_HANDLE    connectionId,
                                    const OI_BYTE                   *userId,
                                    OI_UINT8                        userIdLen,
                                    const OI_CHAR                   *password,
                                    const OI_OBEX_REALM             *realm);

typedef struct {
    OI_OBEXCLI_CONNECT_CFM connectCfmCB;              /**< Callback function called when the connection has been established */
    OI_OBEXCLI_DISCONNECT_IND disconnectIndCB;        /**< Callback function called when the OBEX connection is disconnected */
    OI_OBEXCLI_AUTH_CHALLENGE_IND authChallengeIndCB; /**< callback function called if a response to an authentication challenge is required */
    OI_OBEXCLI_PROGRESS_IND progressIndCB;            /**< Callback function called to monitor progress of PUTs and GETS, can be NULL */
    OI_OBEXCLI_BULK_PUT_CFM bulkPutCfm;               /**< Callback function called to confirm one or more bulk puts. */
} OI_OBEXCLI_CB_LIST;



/**
 * This function establishes a connection to an OBEX server. This includes setting up a lower layer
 * connection to the OBEX server.
 *
 * @param addr            Pointer to the Bluetooth device address of the OBEX server device
 *
 * @param loweProtocol    Specifies the lower layer protocol for the OBEX server and the
 *                        information required to connect to that server.
 *
 * @param authentication  Specifies whether authentication is required when
 *                        connecting to a server
 *
 * @param connectOptions  OBEX options for this connection. Can be NULL if no options are being
 *                        specified.
 *
 * @param cmdHeaders      This is a pointer to a list of optional OBEX headers to be included in the
 *                        connect request. This parameter can be NULL if there are no optional headers.
 *                        The caller must not free any data referenced by these headers until the
 *                        receiving the connect confirmation callback.
 *
 * @param callbacks       Callback functions for the OBEX client connection.
 *
 * @param connectionId    (OUT) pointer to the location to which a handle to the OBEX connection will be written
 *                         once the connection is established
 *
 * @param policy          The connection policy required by the OBEX client, NULL is not permitted
 *
 * @return                OI_OK if successful or error status if the connect request failed
 */
OI_STATUS OI_OBEXCLI_Connect(OI_BD_ADDR *addr,
                             OI_OBEX_LOWER_PROTOCOL *lowerProtocol,
                             OI_OBEX_CONNECTION_OPTIONS *connectOptions,
                             OI_OBEXCLI_AUTHENTICATION authentication,
                             const OI_OBEX_HEADER_LIST *cmdHeaders,
                             const OI_OBEXCLI_CB_LIST *callbacks,
                             OI_OBEXCLI_CONNECTION_HANDLE *connectionId,
                             const OI_CONNECT_POLICY *policy);


/**
 * This function disconnects from an OBEX server.
 *
 * @param connectionId    This parameter is a unique ID that represents an established connection
 *                        to an OBEX server.
 *
 * @param cmdHeaders      This is a pointer to a list of optional OBEX headers to be included in the
 *                        command. This parameter can be NULL if there are no optional headers.
 *                        The caller must not free any data referenced by these headers until the
 *                        receiving the response callback.
 *
 * @return                OI_OK if the disconnect request could be sent
 */
OI_STATUS OI_OBEXCLI_Disconnect(OI_OBEXCLI_CONNECTION_HANDLE connectionId,
                                const OI_OBEX_HEADER_LIST    *cmdHeaders);


/**
 * Put OBEX headers to an OBEX server. The putCfm() callback indicates that the server
 * has received and acknowledged receipt of the headers.
 *
 * @param connectionId    This parameter is a unique ID that represents an established connection
 *                        to an OBEX server.
 *
 * @param cmdHeaders      the headers being put. The memory allocated by the caller for these
 *                        headers cannot be freed until the putCfm callback is called.
 *
 * @param putCfm          callback function called to confirm that the put operation has
 *                        completed and the server has acknowedged receipt of the headers
 *
 * @param status          Status code to tell obex what is being put.
 *                        - OI_OK indicates this is the final put,
 *                        - OI_OBEX_CONTINUE if there are more puts to come
 *                        - OI_STATUS_INVALID_STATE if OI_OBEXCLI_BulkPut() has been called.
 *                        - An error status to terminate the put operation.
 *
 * @return                OI_OK if put complete, OI_OBEX_OPERATION_IN_PROGRESS if the last request has not
 *                        completed, or other errors from lower protocol layers.
 */
OI_STATUS OI_OBEXCLI_Put(OI_OBEXCLI_CONNECTION_HANDLE connectionId,
                         OI_OBEX_HEADER_LIST const *cmdHeaders,
                         OI_OBEXCLI_PUT_CFM putCfm,
                         OI_STATUS status);


/**
 * Send bulk data to an OBEX server. If used, this function is called following an initial
 * OI_OBEXCLI_Put() to send the body data for the put transaction. Unlike OI_OBEXCLI_Put() which
 * does not permit another Put until the putCfm callback is called, multiple OI_OBEXCLI_BulkPut()
 * calls an be made and OBEX will queue the data internally. This is of most value when the
 * connection has been configured for single response mode because it allows OBEX to keep data
 * flowing. The bulkPutCfm() callback is called as each bulk data buffer has been sent to the
 * server. For a specific PUT transaction, once OI_OBEXCLI_BulkPut() has been called, the
 * transaction must be completed using OI_OBEXCLI_BulkPut() calls.
 *
 * @param connectionId    This parameter is a unique ID that represents an established connection to
 *                        a remote OBEX server.
 *
 * @param numBuffers      The number of buffer to put.
 *
 * @param bulkDataBuffer  An array of bulk data buffers to be put. The data buffers must not be freed
 *                        until the buffer pointers are individually returned in the bulkPutCfm callback.
 *
 * @param bufferLength    An array of sizes for the bulk data buffers. The bulk data buffers are combined and
 *                        segmented internally into appropriately sized OBEX packets so this value
 *                        can be arbitrarily large or small.
 *
 * @param status          Status code to tell obex what is being put.
 *                        - OI_OK indicates this is the final put call,
 *                        - OI_OBEX_CONTINUE if there are more bulk data puts to come
 *                        - An error status to terminate the bulk put operation.
 *
 * @return                OI_OK if put was successful but no more puts are permitted at this time.
 *                        OI_OBEX_CONTINUE if the put was successful and more puts are permitted.
 *
 *                        or other errors indicating that the put operation failed and that the
 *                        caller should cleanup and disconnect the link.
 */
OI_STATUS OI_OBEXCLI_BulkPut(OI_OBEXCLI_CONNECTION_HANDLE connectionId,
                             OI_UINT8 numBuffers,
                             OI_UINT8 *bulkDataBuffer[],
                             OI_UINT32 bufferLength[],
                             OI_STATUS status);


/**
 * Get data from an OBEX server. This will cause one or more packets to be
 * retrieved from an OBEX server.
 *
 * @param connectionId    This parameter is a unique ID that represents an established connection
 *                        to an OBEX server.
 *
 * @param cmdHeaders      description of the object being requested.
 *
 * @param getRecvData     function that will be invoked when data is received from the server
 *
 * @param final           TRUE if this is the last packet in the get request,
 *                        FALSE otherwise. Most get requests are only one packet long, and should have
 *                        this parameter set to TRUE.
 *
 * @return                OI_OK if get successful, OI_OBEX_OPERATION_IN_PROGRESS
 *                        if the last request has not completed, or other errors from lower layers.
 */
OI_STATUS OI_OBEXCLI_Get(OI_OBEXCLI_CONNECTION_HANDLE connectionId,
                         OI_OBEX_HEADER_LIST const *cmdHeaders,
                         OI_OBEXCLI_GET_RECV_DATA getRecvData,
                         OI_BOOL final);


/**
 * Terminates the current put or get operation on this connection. If there is no current GET/PUT
 * operation this function returns OI_STATUS_INVALID_STATE. If a PUT/GET transaction is in progress,
 * i.e. the calling layer has called OI_OBEXCLI_Get, OI_OBEXCLI_Put, or OI_OBEXCLI_BulkPut, and is
 * waiting for a confirm callback, when the transaction completes the appropriate callback is called
 * with status OI_OBEX_CLIENT_ABORTED_COMMAND. The abort confirm callback will be called when the
 * remote OBEX server confirms that the abort is complete.
 *
 * @param connectionId    This parameter is a unique ID that represents an
 *                        established connection to an OBEX server.
 *
 * @param abortCfm        function that will be invoked when the abort completes.
 *
 * @retval                OI_OK if the operation was aborted.
 * @retval                OI_STATUS_INVALID_STATE if there is nothing to abort
 */
OI_STATUS OI_OBEXCLI_Abort(OI_OBEXCLI_CONNECTION_HANDLE connectionId,
                           OI_OBEXCLI_ABORT_CFM         abortCfm);


/**
 * Set the response timeout for an OBEX connection overriding the internal
 * default timeout. The timeout value indicates how long the OBEX client will
 * wait for a response from an OBEX server before aborting a PUT, GET or other
 * OBEX operation.
 *
 * @param connectionId    This parameter is a unique ID that represents an
 *                        established connection to an OBEX server.
 *
 * @param timeout         The timeout is specified in 1/10's of seconds. A
 *                        timeout of 0 is not allowed.
 *
 * @return                OI_OK if the connectionId and timeout value was valid.
 */
OI_STATUS OI_OBEXCLI_SetResponseTimeout(OI_OBEXCLI_CONNECTION_HANDLE connectionId,
                                        OI_INTERVAL timeout);

/**
 * Setpath can be used to set the current folder for getting and putting
 * objects and can be used to create new folders.
 *
 * @param connectionId    This parameter is a unique ID that represents an established connection
 *                        to an OBEX server.
 *
 * @param folder          This parameter is a unicode name specifying the new folder.
 *                        This must be NULL if upLevel is TRUE. A NULL folder
 *                        (and upLevel == FALSE) sets the folder to the FTP
 *                        server's root folder.
 *
 * @param dontCreate      This parameter indicates whether a new folder should be created if it does not already exist.
 *                        A value of TRUE indicates that the specified folder should not be created if it does not already exist.
 *
 * @param upLevel         This parameter indicates that the path is to be set to the parent folder of
 *                        the current folder. If upLevel is TRUE, folder must be NULL.
 *
 * @param                 setpathCfm is the callback function called with the server's response to the setpath command.
 *
 * @param cmdHeaders      This is a pointer to a list of optional OBEX headers to be included in the
 *                        command. This parameter can be NULL if there are no optional headers.
 *                        The caller must not free any data referenced by these headers until the
 *                        receiving the response callback.
 *
 * @return                OI_OK if successful; error if the command could not be sent
 */
OI_STATUS OI_OBEXCLI_SetPath(OI_OBEXCLI_CONNECTION_HANDLE   connectionId,
                             OI_OBEX_UNICODE const          *folder,
                             OI_BOOL                        dontCreate,
                             OI_BOOL                        upLevel,
                             OI_OBEXCLI_SETPATH_CFM         setpathCfm,
                             const OI_OBEX_HEADER_LIST      *cmdHeaders);



/**
 * This function sends an action command to the remote OBEX server.
 *
 * @param connectionId    This parameter is a unique ID that represents an established connection
 *                        to an OBEX server.
 *
 * @param actionId        Identifies the action to perform.
 *
 * @param remoteObject    This parameter is a unicode name specifying the remote object to perform
 *                        the action on.
 *
 * @param actionHdrs      This is a list of action-specific command headers.
 *
 * @param                 actionCfm is the callback function called with the server's response to the action command.
 *
 * @return                OI_OK if successful; error if the command could not be sent
 */
OI_STATUS OI_OBEXCLI_DoAction(OI_OBEXCLI_CONNECTION_HANDLE connectionId,
                              OI_UINT8 actionId,
                              OI_OBEX_UNICODE const *object,
                              OI_OBEX_HEADER_LIST const *actionHdrs,
                              OI_OBEXCLI_ACTION_CFM actionCfm);


/**
 * This function returns the optimal size for a body header for a connection.
 * Sending body headers that are smaller than this size or not an integer
 * multiple of this size will result in slower data transfer rates.
 *
 * @param connectionId    This parameter is a unique ID that represents an established connection
 *                        to an OBEX server.
 *
 * @returns               best body payload size or 0 if there is no connection
 */
OI_UINT16 OI_OBEXCLI_OptimalBodyHeaderSize(OI_OBEXCLI_CONNECTION_HANDLE connectionId);

/**
 * Associates a caller defined context with an OBEX client connection. This
 * context can then be retrieved by calling OI_OBEXCLI_GetConnectionContext().
 *
 * @param connectionId   The connection to associate the context with.
 *
 * @param context         A value supplied by the caller.
 *
 * @return                OI_OK if the context was set, OI_STATUS_NOT_FOUND if
 *                        the connection id is not valid.
 */
OI_STATUS OI_OBEXCLI_SetConnectionContext(OI_OBEXCLI_CONNECTION_HANDLE connectionId,
                                          void *context);


/**
 * Gets a caller defined context associate with an OBEX client connection. This is a value
 * that we previously set by a call to OI_OBEXCLI_SetConnectionContext().
 *
 * @param connectionId    The OBEX client connection to get the context from.
 *
 * @return                A context pointer or NULL if the handle is invalid or
 *                        there is no context associated with this connection.
 */
void* OI_OBEXCLI_GetConnectionContext(OI_OBEXCLI_CONNECTION_HANDLE connectionId);


/**
 * Given an OBEX client connection handle this function returns the L2CAP channel for this
 * connection. In the case of OBEX/RFCOMM this is the L2CAP channel for the underlying RFCOMM
 * session. In the case of OBEX/L2CAP this is the L2CAP channel for the OBEX connection.
 *
 * @param connectionId  The OBEX client connection to get the L2CAP channel for
 * @param cid           Pointer to out parameter for the L2CAP CID
 *
 * @return      - OI_OK if the context was set,
 *              - OI_STATUS_NOT_FOUND if the connection id is not valid.
 *              - OI_STATUS_NOT_CONNECTED if the connection is not up or is disconnecting
 *
 *
 */
OI_STATUS OI_OBEXCLI_GetL2capCID(OI_OBEXCLI_CONNECTION_HANDLE connectionId,
                                 OI_L2CAP_CID *cid);

/**
 * This function returns the raw header list associated with a callback.  This function
 * must only be called from within a callback, otherwise results are unpredictable.
 *
 * Some OBEX commands have optional headers (e.g. realm information in an authentication challenge);
 * this API makes those headers available to applications.
 *
 * @param connectionId  The OBEX connection

 * @param pRawHeaderList    Pointer where OBEX will store pointers to the raw header list.
 *
 * @return      - OI_OK, pRawHeaderList contains pointer to header list, may be NULL.
 *              - any other status, pRawHeaderList returned is undefined
 */
OI_STATUS OI_OBEXCLI_GetRawHeaderList(OI_OBEXCLI_CONNECTION_HANDLE connectionId,
                                      OI_OBEX_HEADER_LIST          **pRawHeaderList);


/*****************************************************************************/

#ifdef __cplusplus
}
#endif

/**@}*/

#endif /* _OI_OBEXCLI_H */
