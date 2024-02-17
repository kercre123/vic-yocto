#ifndef _OI_BPP_SENDER_H
#define _OI_BPP_SENDER_H

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
 * @file
 *
 * This file provides the interface for a Basic Printing Profile sender
 * application.
 */

#include "oi_bpp.h"
#include "oi_sdp.h"
#include "oi_connect_policy.h"
#include "oi_obexcli.h"

/** \addtogroup BPP BPP APIs */
/**@{*/

#ifdef __cplusplus
extern "C" {
#endif


/**
 * Type for a register BPP sender server
 */
typedef OI_OBEX_SERVER_HANDLE OI_BPP_SENDER_SERVER_HANDLE;


/** This function indicates a connection request by a printer for a referenced objects service.
 * The sender application should check to see if the connecting printer is
 * currently or has recently been the target of a printing operation before
 * deciding to accept the connection.
 *
 * @param handle    Connection handle for this session
 * @param addr      Address of printer requesting access
 * @return  OI_OK to accept the connection, OI_OBEX_ACCESS_DENIED or another
 * error status to refuse
 */
typedef OI_STATUS (*OI_BPP_SENDER_CONNECT_IND)(OI_OBEXSRV_CONNECTION_HANDLE handle,
                                               OI_BD_ADDR *addr);

/** This function indicates that a printer has disconnected.
 *
 * @param handle    Handle of the dropped connection
 */
typedef void (*OI_BPP_SENDER_DISCONNECT_IND)(OI_OBEXSRV_CONNECTION_HANDLE handle);

/** This function indicates a request by a connected printer for an object.
 * @param handle        Connection handle on which the request is being made
 * @param name          URI of the requested object
 * @param offset        Offset into the requested object
 * @param count         Number of bytes, starting at the offset, being requested
 * @param getFileSize   TRUE if the printer is requesting the total size of the
 * indicated object
 */
typedef OI_STATUS (*OI_BPP_SENDER_GET_REFERENCED_OBJECTS_IND)(OI_OBEXSRV_CONNECTION_HANDLE handle,
                                                              OI_OBEX_UNICODE *name,
                                                              OI_UINT32 offset,
                                                              OI_INT32 count,
                                                              OI_BOOL getFileSize,
                                                              OI_STATUS status);
/** This function responds to a get referenced objects request.
 * @param handle    Connection handle on which the request is being made
 * @param data      Body of the reply
 * @param fileSize  Pointer to file size if requested, NULL if not
 * @param status    OI_OBEX_CONTINUE if more data follows this response, OI_OK or
 * an error otherwise.
 */
OI_STATUS OI_BPP_SENDER_GetReferencedObjectsResponse(OI_OBEXSRV_CONNECTION_HANDLE handle,
                                                     OI_OBEX_BYTESEQ *data,
                                                     OI_INT32 *fileSize,
                                                     OI_STATUS status);



/** Connection establishment */

/** This function provides confirmation of a connection attempt.
 * @param handle    Connection handle, as returned by OI_BPP_SENDER_Connect
 * @param status    OI_OK if connection was established, error otherwise
 */
typedef void (*OI_BPP_SENDER_CONNECT_CFM)(OI_OBEXSRV_CONNECTION_HANDLE handle,
                                          OI_STATUS status);

/** This function provides an authentication challenge during connection attempt. The application should
 * respond with OI_OBEXCLI_Authentication.
 * @param handle    Connection handle, as returned by OI_BPP_SENDER_Connect
 * @param userIdRequired    Indicates whether the authentication response should
 * include a user ID
 */
typedef void (*OI_BPP_SENDER_AUTH_CHALLENGE_IND)(OI_OBEXSRV_CONNECTION_HANDLE handle,
                                                 OI_BOOL userIdRequired);

/** This function connects to a BPP printer. At most, one job-channel connection, one
 * rui-channel connection, and one status connection may be in progress at any given time.
 *
 * @param addr          Address of the remote BPP printer server.
 *
 * @param lowerProtocol This identifies the RFCOMM channel number or the L2CAP PSM for the
 *                      BPP printer server running on the remote device.  the remote device. The
 *                      caller will normally perform service discovery on the remote device to
 *                      obtain the required channel number or PSM.
 *
 * @param target        Enumeration constant specifying the type of service to
 *                      which to connect
 *
 * @param authentication Specifies whether authentication is required when
 *                       connecting to a server

 * @param connectCfm    Callback invoked indicating the success or failure of
 *                      the connection request
 *
 * @param disconnectInd Callback to be invoked when this connection terminates
 *
 * @param authInd       Callback to be invoked to get authentication data; may be
 *                      NULL if authentication is not to be used
 *
 * @param handle        Out parameter specifying the connection handle associated
 *                      with the new connection
 *
 * @return  OI_OK if connection request was successful, error otherwise; the connection is not
 *          complete until the connectCfm callback is called with OI_OK
 */
OI_STATUS OI_BPP_SENDER_Connect(OI_BD_ADDR *addr,
                                OI_OBEX_LOWER_PROTOCOL *lowerProtocol,
                                OI_BPP_TARGET target,
                                OI_OBEXCLI_AUTHENTICATION authentication,
                                OI_BPP_SENDER_CONNECT_CFM connectCfm,
                                OI_BPP_SENDER_DISCONNECT_IND disconnectInd,
                                OI_BPP_SENDER_AUTH_CHALLENGE_IND authInd,
                                OI_OBEXCLI_CONNECTION_HANDLE *handle);

/** This function disconnects the specified BPP connection.
 * @param handle    Handle of the connection to drop
 */
OI_STATUS OI_BPP_SENDER_Disconnect(OI_OBEXCLI_CONNECTION_HANDLE handle);


/** This callback function is invoked in response to OI_BPP_SENDER_SendFile. If the status code
 * is OI_OBEX_CONTINUE, the client should respond by invoking
 * OI_BPP_SENDER_SendFile(handle, [callback], NULL, NULL, NULL, NULL, [pdata],
 * [final]);
 *
 * @param handle    Handle to the connection over which the file was sent
 *
 * @param status    OI_OBEX_CONTINUE if the server expects more data, OI_OK if
 * the transaction completed without error, otherwise an error code
 */
typedef void (*OI_BPP_SENDER_SEND_FILE_CFM)(OI_OBEXCLI_CONNECTION_HANDLE handle,
                                            OI_STATUS status);

/** This function performs the FilePush or SendDocument BPP operations.
 *
 * @param handle    Handle to the connection over which to send the file
 *
 * @param putCfm    Callback indicating the result of the operation
 *
 * @param type      MIME type of the file being sent
 *
 * @param description   Optional document-type specific information
 *
 * @param name      Optional document name
 *
 * @param jobId     Pointer to the job ID if using job-based data transfer, NULL for
 * simple push operation
 *
 * @param data      File data to send
 *
 * @param final     TRUE if this is the final block of data in this operation,
 * FALSE otherwise.
 */
OI_STATUS OI_BPP_SENDER_SendFile(OI_OBEXCLI_CONNECTION_HANDLE handle,
                                 OI_BPP_SENDER_SEND_FILE_CFM putCfm,
                                 OI_OBEX_BYTESEQ *type,
                                 OI_OBEX_UNICODE *description,
                                 OI_OBEX_UNICODE *name,
                                 OI_BPP_JOB_ID *jobId,
                                 OI_OBEX_BYTESEQ *data,
                                 OI_BOOL final);

/** This callback function is invoked in response to OI_BPP_SENDER_SendReference.
 *
 * @param handle    Handle to the connection over which the reference was sent
 * @param url       In the event that not all references could be retrieved, the URL is
 * the first to fail
 * @param httpChallenge If the URL failed due to an authentication challenge, this
 * parameter includes the authentication challenge HTTP header
 * @param status    OI_OBEX_CONTINUE if the server expects more data, OI_OK if
 * the transaction completed without error, otherwise an error code
 */
typedef void (*OI_BPP_SENDER_SEND_REFERENCE_CFM)(OI_OBEXCLI_CONNECTION_HANDLE handle,
                                                 OI_OBEX_BYTESEQ *url,
                                                 OI_OBEX_BYTESEQ *httpChallenge,
                                                 OI_STATUS status);

/** This function performs the SimpleReferencePush or SendReference operation.
 *
 * @param handle    Handle to the connection over which to send the reference
 * @param cb        Callback receiving the result of the operation
 * @param type      Enumeration value indicating the type of reference (simple,
 * list, or xml)
 * @param httpHeaders Authentication credentials (optional)
 * @param jobId Pointer to the job ID if using job-based transfer, NULL for simple
 * reference push
 * @param data  Reference data to send
 * @param final TRUE if this is the final block of data for this operation,
 * FALSE otherwise.
 */
OI_STATUS OI_BPP_SENDER_SendReference(OI_OBEXCLI_CONNECTION_HANDLE handle,
                                      OI_BPP_SENDER_SEND_REFERENCE_CFM cb,
                                      OI_BPP_REF_TYPE type,
                                      OI_OBEX_BYTESEQ *httpHeaders,
                                      OI_BPP_JOB_ID *jobId,
                                      OI_OBEX_BYTESEQ *data,
                                      OI_BOOL final);

/** This callback function is invoked in response to SOAP and RUI requests.
 *
 * @param handle    Handle to the connection over which the request was made
 * @param reply     Body of the reply
 * @param jobId     If a job ID was sent as part of the SOAP response, this
 * value will point to it; otherwise, it is NULL
 * @param status    OI_OBEX_CONTINUE if there is more response data, OI_OK if
 * the transaction completed without error, otherwise an error code
 * @param final     TRUE if the get response contains end-of-body header.
 */
typedef void (*OI_BPP_SENDER_GET_CFM)(OI_OBEXCLI_CONNECTION_HANDLE handle,
                                      OI_OBEX_BYTESEQ *reply,
                                      OI_BPP_JOB_ID *jobId,
                                      OI_STATUS status,
                                      OI_BOOL final);

/** This function sends a SOAP message to a BPP printer.
 * @param handle    Handle to the connection over which to send the request
 * @param cb    Callback invoked with the result of the request
 * @param body  Body of the request
 * @param final TRUE if this is the final (or only) portion of the request
 */
OI_STATUS OI_BPP_SENDER_SOAPRequest(OI_OBEXCLI_CONNECTION_HANDLE handle,
                                    OI_BPP_SENDER_GET_CFM cb,
                                    OI_OBEX_BYTESEQ *body,
                                    OI_BOOL final);

/** This function sends a Get RUI request to a BPP printer.
 * @param handle       Handle to the connection over which to send the request
 * @param cb           Callback invoked with the result of the request
 * @param type         MIME type of the request
 * @param name         Name
 * @param httpHeaders  HTTP headers
 * @param body         Body of the request
 * @param final        TRUE if this is the final (or only) portion of the request
 */
OI_STATUS OI_BPP_SENDER_GetRUI(OI_OBEXCLI_CONNECTION_HANDLE handle,
                               OI_BPP_SENDER_GET_CFM cb,
                               OI_OBEX_BYTESEQ *type,
                               OI_OBEX_UNICODE *name,
                               OI_OBEX_BYTESEQ *httpHeaders,
                               OI_OBEX_BYTESEQ *body,
                               OI_BOOL final);


/** This callback function is invoked when a cancel is complete.
 *
 * @param handle      Handle to the connection with the recently canceled operation
 */
typedef void (*OI_BPP_SENDER_CANCEL_CFM)(OI_OBEXCLI_CONNECTION_HANDLE handle);

/** This function cancels the current OBEX BPP operation.
 *
 * @param handle      Handle to the connection to cancel
 * @param cancelCfm   Callback indicating completion of cancel (may be NULL)
 */
OI_STATUS OI_BPP_SENDER_Cancel(OI_OBEXCLI_CONNECTION_HANDLE handle,
                               OI_BPP_SENDER_CANCEL_CFM cancelCfm);

/**
 * OI_BPP_SENDER_AuthenticationRsp()
 *
 * This function provides authentication information to the BPP printer.
 *
 * @param connectionId Handle representing the connection to the BPP printer
 *
 * @param userId       User ID for the connecting sender; this parameter is
 *                     required if the printer indicated that the user ID is
 *                     required; otherwise is can be NULL; the client can
 *                     provide a user ID even if the printer does not require
 *                     one
 *
 * @param userIdLen    Length of the userId
 *
 * @param password     NULL-terminated client password
 */
OI_STATUS OI_BPP_SENDER_AuthenticationRsp(OI_OBEXCLI_CONNECTION_HANDLE handle,
                                          const OI_BYTE *userId,
                                          OI_UINT8 userIdLen,
                                          const OI_CHAR *password);

/**
  This structure groups together the callbacks associated with a
  sender.
  */
typedef struct {
    OI_BPP_SENDER_CONNECT_IND connectInd;
    OI_BPP_SENDER_DISCONNECT_IND disconnectInd;
    OI_BPP_SENDER_GET_REFERENCED_OBJECTS_IND getObj;
} OI_BPP_SENDER_CALLBACKS;

/** This function registers the BPP sender, optionally registering referenced objects servers.
 * This call must be made even if no servers are to be registered.
 *
 * @param strings         SDP strings for the service record created if servers are to
 *                        be registered; this may be NULL
 *
 * @param objectCallbacks Pointer to server callbacks for referenced objects service; if this
 *                        pointer is NULL, then no referenced objects service will be registered
 *
 * @param ruiRefCallbacks Pointer to server callbacks for RUI referenced objects service; if this
 *                        pointer is NULL, then no RUI referenced objcets service will be registered
 *
 * @param serverHandle    Out parameter handle for the BPP sender server
 */
OI_STATUS OI_BPP_SENDER_Register(const OI_SDP_STRINGS *strings,
                                 const OI_BPP_SENDER_CALLBACKS *objectCallbacks,
                                 const OI_BPP_SENDER_CALLBACKS *ruiRefCallbacks,
                                 OI_BPP_SENDER_SERVER_HANDLE *serverHandle);

/**
 * This function deregisters the BPP sender.
 *
 * @param serverHandle   The handle returned when the BPP printer server was registered.
 *
 * @returns     OI_OK if deregistration was successful, an error otherwise.
 */
OI_STATUS OI_BPP_SENDER_Deregister(OI_BPP_SENDER_SERVER_HANDLE serverHandle);


/**
 * This function forcibly severs the connection from a BPP client to the OBEX
 * server. The disconnect indication callback will be called when the
 * disconnect is complete.
 *
 * A BPP server may need to forcibly terminate a connection during
 * deregistration, since deregistration will fail if a connection is in place.
 *
 * @param handle     Unique identifier generated by the BIP server that
 *                   identifies the connection
 *
 * @return           OI_OK if the connectionId is valid and the connection
 *                   will be terminated
 */
OI_STATUS OI_BPP_SENDER_ForceDisconnect(OI_OBEXSRV_CONNECTION_HANDLE handle);


#ifdef __cplusplus
}
#endif

/**@}*/

#endif /* _OI_BPP_SENDER_H */
