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

/** @file
 * This file provides the API for the client side of the Basic Imaging Profile.
 *
 * Most functions in this API involve a parameter block. These blocks are
 * explained by oi_bip_spec.h.
 *
 * A note about object lifetimes:
 *
 * In keeping with the usual conventions of BM3, all indirectly referenced data
 * passed to a BIP function must remain valid until the corresponding callback
 * is invoked. This means, for example, that storage for parameters should not
 * be allocated on the stack of the function calling into BIP.
 *
 * There is one specific class of exception to this rule for BIP. An
 * OI_OBEX_UNICODE or OI_OBEX_BYTESEQ entry in a parameter block may be safely
 * allocated on the stack because its entries are copied into an internal data
 * structre by the BIP call. However, the data pointed to by these structures must obey
 * the usual rules. If a programmer chooses to take advantage of this
 * exception, it is very important that the entry NOT be referenced by the
 * callback function because it will still contain a stale pointer to a stack
 * variable that has since gone out of scope. The application must have another
 * reference to the underlying data if it needs to free it or process it
 * further.
 */

#ifndef _OI_BIPCLI_H
#define _OI_BIPCLI_H

#include "oi_bip_spec.h"
#include "oi_obexspec.h"
#include "oi_obexcli.h"

/** \addtogroup BIP BIP APIs */
/**@{*/

#ifdef __cplusplus
extern "C" {
#endif


typedef OI_OBEXCLI_CONNECTION_HANDLE  OI_BIPCLI_HANDLE;

/**
 * Indicates an authentication request for a BIP connection
 * @param connection    Connection handle associated with the request; the
 * client should respond with OI_BIPCLI_AuthenticationResponse
 */
typedef void(*OI_BIPCLI_AUTH_IND)(OI_BIPCLI_HANDLE connection);

/**
 * Indicates a connection request has been completed
 * @param connection    Connection handle
 * @param status      Result of the connection: OI_OK or an error
 */
typedef void(*OI_BIPCLI_CONNECT_CFM)(OI_BIPCLI_HANDLE connection,
                                     OI_STATUS status);

/**
 * Indicates that a connection is no longer in place
 */
typedef void(*OI_BIPCLI_DISCONNECT_IND)(OI_BIPCLI_HANDLE connection);



/* *************
 * ************* GetCapabilities
 * *************
 */


/**
 * This callback function indicates the result of a GET_CAPABILITIES operation.
 * @param connection    Connection handle for this request
 * @param params        Pointer to the parameter block specified by the
 * application during the request
 * @param result        Response from the server; if this is anything other than
 * OI_OBEX_CONTINUE then it is safe for the application to deallocate or
 * otherwise destroy the parameter block
 */
typedef void(*OI_BIPCLI_GET_CAPABILITIES_CFM)(OI_BIPCLI_HANDLE connection,
                                              OI_BIP_GET_CAPABILITIES_PARAMS *params,
                                              OI_STATUS result);

/**
 * This function performs a BIP GET_CAPABILITIES operation.
 * @param connection    Connection handle for this request
 * @param cb            Completion callback
 * @param params        Pointer to a parameter block, allocated by the connection,
 * to be used for any requests and responses; this must remain valid until the
 * final confirmation callback
 */
OI_STATUS OI_BIPCLI_GetCapabilities(OI_BIPCLI_HANDLE connection,
                                    OI_BIPCLI_GET_CAPABILITIES_CFM cb,
                                    OI_BIP_GET_CAPABILITIES_PARAMS *params);

/* *************
 * ************* PutImage
 * *************
 */

/**
 * This callback function indicates the result of a PUT_IMAGE operation.
 * @param connection    Connection handle for this request
 * @param params        Pointer to the parameter block specified by the
 * application during the request
 * @param result        Response from the server; if this is anything other than
 * OI_OBEX_CONTINUE then it is safe for the application to deallocate or
 * otherwise destroy the parameter block
 */
typedef void(*OI_BIPCLI_PUT_IMAGE_CFM)(OI_BIPCLI_HANDLE connection,
                                       OI_BIP_PUT_IMAGE_PARAMS *params,
                                       OI_STATUS result);
/**
 * This function performs a BIP PUT_IMAGE operation.
 * @param connection    Connection handle for this request
 * @param cb            Completion callback
 * @param params        Pointer to a parameter block, allocated by the connection,
 * to be used for any requests and responses; this must remain valid until the
 * final confirmation callback
 */

OI_STATUS OI_BIPCLI_PutImage(OI_BIPCLI_HANDLE connection,
                             OI_BIPCLI_PUT_IMAGE_CFM cb,
                             OI_BIP_PUT_IMAGE_PARAMS *params);

/* *************
 * ************* PutLinkedThumbnail
 * *************
 */

/**
 * This callback function indicates the result of a PUT_LINKED_THUMBNAIL operation.
 * @param connection    Connection handle for this request
 * @param params        Pointer to the parameter block specified by the
 * application during the request
 * @param result        Response from the server; if this is anything other than
 * OI_OBEX_CONTINUE then it is safe for the application to deallocate or
 * otherwise destroy the parameter block
 */
typedef void(*OI_BIPCLI_PUT_LINKED_THUMBNAIL_CFM)(OI_BIPCLI_HANDLE connection,
                                                  OI_BIP_PUT_LINKED_THUMBNAIL_PARAMS *params,
                                                  OI_STATUS result);

/**
 * This function performs a BIP PUT_LINKED_THUMBNAIL operation.
 * @param connection    Connection handle for this request
 * @param cb            Completion callback
 * @param params        Pointer to a parameter block, allocated by the connection,
 * to be used for any requests and responses; this must remain valid until the
 * final confirmation callback
 */
OI_STATUS OI_BIPCLI_PutLinkedThumbnail(OI_BIPCLI_HANDLE connection,
                                       OI_BIPCLI_PUT_LINKED_THUMBNAIL_CFM cb,
                                       OI_BIP_PUT_LINKED_THUMBNAIL_PARAMS *params);

/* *************
 * ************* PutLinkedAttachment
 * *************
 */

/**
 * This callback function indicates the result of a PUT_LINKED_ATTACHMENT operation.
 * @param connection    Connection handle for this request
 * @param params        Pointer to the parameter block specified by the
 * application during the request
 * @param result        Response from the server; if this is anything other than
 * OI_OBEX_CONTINUE then it is safe for the application to deallocate or
 * otherwise destroy the parameter block
 */
typedef void(*OI_BIPCLI_PUT_LINKED_ATTACHMENT_CFM)(OI_BIPCLI_HANDLE connection,
                                                   OI_BIP_PUT_LINKED_ATTACHMENT_PARAMS *params,
                                                   OI_STATUS result);

/**
 * This function performs a BIP PUT_LINKED_ATTACHMENT operation.
 * @param connection    Connection handle for this request
 * @param cb            Completion callback
 * @param params        Pointer to a parameter block, allocated by the connection,
 * to be used for any requests and responses; this must remain valid until the
 * final confirmation callback
 */
OI_STATUS OI_BIPCLI_PutLinkedAttachment(OI_BIPCLI_HANDLE connection,
                                        OI_BIPCLI_PUT_LINKED_ATTACHMENT_CFM cb,
                                        OI_BIP_PUT_LINKED_ATTACHMENT_PARAMS *params);


/* *************
 * ************* RemoteDisplay
 * *************
 */

/**
 * This Callback function indicates the result of a REMOTE_DISPLAY operation.
 * @param connection    Connection handle for this request
 * @param params        Pointer to the parameter block specified by the
 * application during the request
 * @param result        Response from the server; if this is anything other than
 * OI_OBEX_CONTINUE then it is safe for the application to deallocate or
 * otherwise destroy the parameter block
 */
typedef void(*OI_BIPCLI_REMOTE_DISPLAY_CFM)(OI_BIPCLI_HANDLE connection,
                                            OI_BIP_REMOTE_DISPLAY_PARAMS *params,
                                            OI_STATUS result);


/**
 * This function performs a BIP REMOTE_DISPLAY operation.
 * @param connection    Connection handle for this request
 * @param cb            Completion callback
 * @param params        Pointer to a parameter block, allocated by the connection,
 * to be used for any requests and responses; this must remain valid until the
 * final confirmation callback
 */
OI_STATUS OI_BIPCLI_RemoteDisplay(OI_BIPCLI_HANDLE connection,
                                  OI_BIPCLI_REMOTE_DISPLAY_CFM cb,
                                  OI_BIP_REMOTE_DISPLAY_PARAMS *params);


/* *************
 * ************* GetImagesList
 * *************
 */

/**
 * This callback function indicates the result of a GET_IMAGES_LIST operation.
 * @param connection    Connection handle for this request
 * @param params        Pointer to the parameter block specified by the
 * application during the request
 * @param result        Response from the server; if this is anything other than
 * OI_OBEX_CONTINUE then it is safe for the application to deallocate or
 * otherwise destroy the parameter block
 */
typedef void(*OI_BIPCLI_GET_IMAGES_LIST_CFM)(OI_BIPCLI_HANDLE connection,
                                             OI_BIP_GET_IMAGES_LIST_PARAMS *params,
                                             OI_STATUS result);

/**
 * This function performs a BIP GET_IMAGES_LIST operation.
 * @param connection    Connection handle for this request
 * @param cb            Completion callback
 * @param params        Pointer to a parameter block, allocated by the connection,
 * to be used for any requests and responses; this must remain valid until the
 * final confirmation callback
 */
OI_STATUS OI_BIPCLI_GetImagesList(OI_BIPCLI_HANDLE connection,
                                  OI_BIPCLI_GET_IMAGES_LIST_CFM cb,
                                  OI_BIP_GET_IMAGES_LIST_PARAMS *params);

/* *************
 * ************* GetImageProperties
 * *************
 */


/**
 * This callback function indicates the result of a GET_IMAGE_PROPERTIES operation.
 * @param connection    Connection handle for this request
 * @param params        Pointer to the parameter block specified by the
 * application during the request.
 * @param result        Response from the server; if this is anything other than
 * OI_OBEX_CONTINUE then it is safe for the application to deallocate or
 * otherwise destroy the parameter block
 */
typedef void(*OI_BIPCLI_GET_IMAGE_PROPERTIES_CFM)(OI_BIPCLI_HANDLE connection,
                                                  OI_BIP_GET_IMAGE_PROPERTIES_PARAMS *params,
                                                  OI_STATUS result);

/**
 * This function performs a BIP GET_IMAGE_PROPERTIES operation.
 * @param connection    Connection handle for this request
 * @param cb            Completion callback
 * @param params        Pointer to a parameter block, allocated by the connection,
 * to be used for any requests and responses; this must remain valid until the
 * final confirmation callback
 */
OI_STATUS OI_BIPCLI_GetImageProperties(OI_BIPCLI_HANDLE connection,
                                       OI_BIPCLI_GET_IMAGE_PROPERTIES_CFM cb,
                                       OI_BIP_GET_IMAGE_PROPERTIES_PARAMS *params);

/* *************
 * ************* GetImage
 * *************
 */

/**
 * This callback function indicates the result of a GET_IMAGE operation.
 * @param connection    Connection handle for this request
 * @param params        Pointer to the parameter block specified by the
 * application during the request
 * @param result        Response from the server; if this is anything other than
 * OI_OBEX_CONTINUE then it is safe for the application to deallocate or
 * otherwise destroy the parameter block
 */
typedef void(*OI_BIPCLI_GET_IMAGE_CFM)(OI_BIPCLI_HANDLE connection,
                                       OI_BIP_GET_IMAGE_PARAMS *params,
                                       OI_STATUS result);

/**
 * This function performs a BIP GET_IMAGE operation.
 * @param connection    Connection handle for this request
 * @param cb            Completion callback
 * @param params        Pointer to a parameter block, allocated by the connection,
 * to be used for any requests and responses; this must remain valid until the
 * final confirmation callback
 */
OI_STATUS OI_BIPCLI_GetImage(OI_BIPCLI_HANDLE connection,
                             OI_BIPCLI_GET_IMAGE_CFM cb,
                             OI_BIP_GET_IMAGE_PARAMS *params);


/* *************
 * ************* GetLinkedThumbnail
 * *************
 */

/**
 * This callback function indicates the result of a GET_LINKED_THUMBNAIL operation.
 * @param connection    Connection handle for this request
 * @param params        Pointer to the parameter block specified by the
 * application during the request
 * @param result        Response from the server; if this is anything other than
 * OI_OBEX_CONTINUE then it is safe for the application to deallocate or
 * otherwise destroy the parameter block
 */
typedef void(*OI_BIPCLI_GET_LINKED_THUMBNAIL_CFM)(OI_BIPCLI_HANDLE connection,
                                                  OI_BIP_GET_LINKED_THUMBNAIL_PARAMS *params,
                                                  OI_STATUS result);

/**
 * This function performs a BIP GET_LINKED_THUMBNAIL operation.
 * @param connection    Connection handle for this request
 * @param cb            Completion callback
 * @param params        Pointer to a parameter block, allocated by the connection,
 * to be used for any requests and responses; this must remain valid until the
 * final confirmation callback
 */
OI_STATUS OI_BIPCLI_GetLinkedThumbnail(OI_BIPCLI_HANDLE connection,
                                       OI_BIPCLI_GET_LINKED_THUMBNAIL_CFM cb,
                                       OI_BIP_GET_LINKED_THUMBNAIL_PARAMS *params);


/* *************
 * ************* GetLinkedAttachment
 * *************
 */

/**
 * This callback function indicates the result of a GET_LINKED_ATTACHMENT operation.
 * @param connection    Connection handle for this request
 * @param params        Pointer to the parameter block specified by the
 * application during the request
 * @param result        Response from the server; if this is anything other than
 * OI_OBEX_CONTINUE then it is safe for the application to deallocate or
 * otherwise destroy the parameter block
 */
typedef void(*OI_BIPCLI_GET_LINKED_ATTACHMENT_CFM)(OI_BIPCLI_HANDLE connection,
                                                   OI_BIP_GET_LINKED_ATTACHMENT_PARAMS *params,
                                                   OI_STATUS result);

/**
 * This function performs a BIP GET_LINKED_ATTACHMENT operation.
 * @param connection    Connection handle for this request
 * @param cb            Completion callback
 * @param params        Pointer to a parameter block, allocated by the connection,
 * to be used for any requests and responses; this must remain valid until the
 * final confirmation callback
 */
OI_STATUS OI_BIPCLI_GetLinkedAttachment(OI_BIPCLI_HANDLE connection,
                                        OI_BIPCLI_GET_LINKED_ATTACHMENT_CFM cb,
                                        OI_BIP_GET_LINKED_ATTACHMENT_PARAMS *params);

/* *************
 * ************* DeleteImage
 * *************
 */

/**
 * This callback function indicates the result of a DELETE_IMAGE operation.
 * @param connection    Connection handle for this request
 * @param params        Pointer to the parameter block specified by the
 * application during the request
 * @param result        Response from the server; if this is anything other than
 * OI_OBEX_CONTINUE then it is safe for the application to deallocate or
 * otherwise destroy the parameter block
 */
typedef void(*OI_BIPCLI_DELETE_IMAGE_CFM)(OI_BIPCLI_HANDLE connection,
                                          OI_STATUS result);

/**
 * This function performs a BIP DELETE_IMAGE operation.
 * @param connection    Connection handle for this request
 * @param cb            Completion callback
 * @param params        Pointer to a parameter block, allocated by the connection,
 * to be used for any requests and responses; this must remain valid until the
 * final confirmation callback
 */
OI_STATUS OI_BIPCLI_DeleteImage(OI_BIPCLI_HANDLE connection,
                                OI_BIPCLI_DELETE_IMAGE_CFM cb,
                                OI_BIP_DELETE_IMAGE_PARAMS *params);

/* *************
 * ************* StartPrint
 * *************
 */

/**
 * This callback function indicates the result of a START_PRINT operation.
 * @param connection    Connection handle for this request
 * @param params        Pointer to the parameter block specified by the
 * application during the request
 * @param result        Response from the server; if this is anything other than
 * OI_OBEX_CONTINUE then it is safe for the application to deallocate or
 * otherwise destroy the parameter block
 */
typedef void(*OI_BIPCLI_START_PRINT_CFM)(OI_BIPCLI_HANDLE connection,
                                         OI_BIP_START_PRINT_PARAMS *params,
                                         OI_STATUS result);

/**
 * This function performs a BIP START_PRINT operation.
 * @param connection    Connection handle for this request
 * @param cb            Completion callback
 * @param params        Pointer to a parameter block, allocated by the connection,
 * to be used for any requests and responses; this must remain valid until the
 * final confirmation callback
 */
OI_STATUS OI_BIPCLI_StartPrint(OI_BIPCLI_HANDLE connection,
                               OI_BIPCLI_START_PRINT_CFM cb,
                               OI_BIP_START_PRINT_PARAMS *params);

/* *************
 * ************* GetPartialImage
 * *************
 */

/**
 * This callback function indicates the result of a GET_PARTIAL_IMAGE operation.
 * @param connection    Connection handle for this request
 * @param params        Pointer to the parameter block specified by the
 * application during the request
 * @param result        Response from the server; if this is anything other than
 * OI_OBEX_CONTINUE then it is safe for the application to deallocate or
 * otherwise destroy the parameter block
 */
typedef void(*OI_BIPCLI_GET_PARTIAL_IMAGE_CFM)(OI_BIPCLI_HANDLE connection,
                                               OI_BIP_GET_PARTIAL_IMAGE_PARAMS *params,
                                               OI_STATUS result);

/**
 * This function performs a BIP GET_PARTIAL_IMAGE operation.
 * @param connection    Connection handle for this request
 * @param cb            Completion callback
 * @param params        Pointer to a parameter block, allocated by the connection,
 * to be used for any requests and responses;t his must remain valid until the
 * final confirmation callback
 */
OI_STATUS OI_BIPCLI_GetPartialImage(OI_BIPCLI_HANDLE connection,
                                    OI_BIPCLI_GET_PARTIAL_IMAGE_CFM cb,
                                    OI_BIP_GET_PARTIAL_IMAGE_PARAMS *params);

/* *************
 * ************* StartArchive
 * *************
 */

/**
 * This callback function indicates the result of a START_ARCHIVE operation.
 * @param connection    Connection handle for this request
 * @param params        Pointer to the parameter block specified by the
 * application during the request
 * @param result        Response from the server; if this is anything other than
 * OI_OBEX_CONTINUE then it is safe for the application to deallocate or
 * otherwise destroy the parameter block
 */
typedef void(*OI_BIPCLI_START_ARCHIVE_CFM)(OI_BIPCLI_HANDLE connection,
                                           OI_STATUS result);

/**
 * This function performs a BIP START_ARCHIVE operation.
 * @param connection    Connection handle for this request
 * @param cb            Completion callback
 * @param params        Pointer to a parameter block, allocated by the connection,
 * to be used for any requests and responses; this must remain valid until the
 * final confirmation callback
 */
OI_STATUS OI_BIPCLI_StartArchive(OI_BIPCLI_HANDLE connection,
                                 OI_BIPCLI_START_ARCHIVE_CFM cb,
                                 OI_BIP_START_ARCHIVE_PARAMS *params);

/* *************
 * ************* GetMonitoringImage
 * *************
 */

/**
 * This callback function indicates the result of a GET_MONITORING_IMAGE operation.
 * @param connection    Connection handle for this request
 * @param params        Pointer to the parameter block specified by the
 * application during the request
 * @param result        Response from the server; if this is anything other than
 * OI_OBEX_CONTINUE then it is safe for the application to deallocate or
 * otherwise destroy the parameter block
 */
typedef void(*OI_BIPCLI_GET_MONITORING_IMAGE_CFM)(OI_BIPCLI_HANDLE connection,
                                                  OI_BIP_GET_MONITORING_IMAGE_PARAMS *params,
                                                  OI_STATUS result);

/**
 * This function performs a BIP GET_MONITORING_IMAGE operation.
 * @param connection    Connection handle for this request
 * @param cb            Completion callback
 * @param params        Pointer to a parameter block, allocated by the connection,
 * to be used for any requests and responses; this must remain valid until the
 * final confirmation callback
 */
OI_STATUS OI_BIPCLI_GetMonitoringImage(OI_BIPCLI_HANDLE connection,
                                       OI_BIPCLI_GET_MONITORING_IMAGE_CFM cb,
                                       OI_BIP_GET_MONITORING_IMAGE_PARAMS *params);

/* *************
 * ************* GetStatus
 * *************
 */
/**
 * This callback indicates the result of a GET_STATUS operation.
 * @param connection    Connection handle for this request
 * @param result        Status result from the server
 */
typedef void(*OI_BIPCLI_GET_STATUS_CFM)(OI_BIPCLI_HANDLE connection,
                                        OI_STATUS result);
/**
 * This function performs a BIP GET_STATUS operation.
 * @param connection    Connection handle for this request
 * @param cb            Completion callback
 */

OI_STATUS OI_BIPCLI_GetStatus(OI_BIPCLI_HANDLE connection,
                              OI_BIPCLI_GET_STATUS_CFM cb);



/* *************  */


/**
 * This function connects to a BIP server.
 *
 * @param addr              BD_ADDR of the remote BIP server
 * @param lowerProtocol     This identifies the RFCOMM channel number or the L2CAP PSM for the
 *                          FTP server running on the remote device.  the remote device. The caller
 *                          will normally perform service discovery on the remote device to obtain
 *                          the required channel number or PSM.
 * @param target            OBEX Target UUID of the server.  Constants for the UUID byte sequences are
 *                              exposed in the oi_bip_spec.h header file; e.g. OI_BIP_ImagePushObexTargetUUID
 * @param authentication    Specifies whether authentication is required when
 *                          connecting to a server
 * @param authInd           Callback handling server authentication challenges
 * @param connectCfm        Callback indicating completion of the connection attempt
 * @param disconnectInd     Callback indicating termination of the connection
 * @param handle            An out parameter receiving the handle for this connection, to
 *                          be used in subsequent function requests
 */
OI_STATUS OI_BIPCLI_Connect(OI_BD_ADDR *addr,
                            OI_OBEX_LOWER_PROTOCOL *lowerProtocol,
                            OI_OBEX_BYTESEQ const *target,
                            OI_OBEXCLI_AUTHENTICATION authentication,
                            OI_BIPCLI_AUTH_IND authInd,
                            OI_BIPCLI_CONNECT_CFM connectCfm,
                            OI_BIPCLI_DISCONNECT_IND disconnectInd,
                            OI_BIPCLI_HANDLE *handle);

/**
 * This function disconnects from a BIP server.
 * @param connection    Handle of the connection to be dropped
 */
OI_STATUS OI_BIPCLI_Disconnect(OI_BIPCLI_HANDLE connection);


/**
 * This function responds to an authentication challenge.
 * @param connection    Handle of the connection as specified by the
 *                      authentication challenge callback
 * @param pin           Pointer to a NULL-terminated pin, or NULL to refuse
 * authentication
 */
void OI_BIPCLI_AuthenticationResponse(OI_BIPCLI_HANDLE connection,
                                      const OI_CHAR *pin);


/**
 * This callback function is called when a cancel request completes.
 *
 * @param connection   Handle for the BIP connection
 */
typedef void (*OI_BIPCLI_CANCEL_CFM)(OI_BIPCLI_HANDLE connection);


/**
 * This function terminates the current BIP operation on this connection. If there is a
 * command in progress this function returns OI_STATUS_PENDING and the operation
 * will be canceled as soon as possible. If there is no command in progress
 * the operation will be canceled immediately and the cancel confirm callback
 * will be called when the cancel is complete. If there is no current BIP
 * operation this function returns OI_STATUS_INVALID_STATE.
 *
 * @param connection      Unique ID that represents an
 *                        established connection to an OBEX server
 *
 * @param cancelCfm       Function that will be invoked when the completes;
 *                        this function is only called if the return value is
 *                        OI_OK; if the return value is OI_STATUS_PENDING the
 *                        callback for the current command will be called
 *                        instead
 *
 * @return                - OI_OK if the operation was aborted
 *                        - OI_STATUS_PENDING if the abort is deferred
 *                        - OI_STATUS_INVALID_STATE if there is nothing to abort
 */
OI_STATUS OI_BIPCLI_Cancel(OI_BIPCLI_HANDLE connection,
                           OI_BIPCLI_CANCEL_CFM cancelCfm);


/**
 * This function associates a caller defined context with an BIP client connection. This
 * context can then be retrieved by calling OI_BIPCLI_GetContext().
 *
 * @param connection      Specifies the BIP connection with which to associate the context
 *
 * @param context         Value supplied by the caller
 *
 * @return                OI_OK if the context was set, OI_STATUS_NOT_FOUND if
 *                        the connection handle is not valid
 */
OI_STATUS OI_BIPCLI_SetContext(OI_BIPCLI_HANDLE connection,
                               void *context);


/**
 * This function gets a caller-defined context associated with an BIP client connection. This
 * is a value that was previously set by a call to OI_BIPCLI_SetContext().
 *
 * @param connection      BIP client connection from which to get the context
 *
 * @return                Context pointer or NULL if the handle is invalid or
 *                        there is no context associated with this connection
 */
void* OI_BIPCLI_GetContext(OI_BIPCLI_HANDLE connection);


#ifdef __cplusplus
}
#endif

/**@}*/


#endif /* _OI_BIP_CLI_H */


