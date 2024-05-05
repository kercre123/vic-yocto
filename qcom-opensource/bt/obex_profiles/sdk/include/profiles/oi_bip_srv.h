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

This is the Basic Imaging Profile server API.

For more information on BIP functions, see the documentation for the
file oi_bip_spec.h.

 */

#ifndef _OI_BIPSRV_SRV_H
#define _OI_BIPSRV_SRV_H

#include "oi_bip_spec.h"
#include "oi_sdp.h"
#include "oi_obexsrv.h"
#include "oi_obexspec.h"

/** \addtogroup BIP BIP APIs */
/**@{*/

#ifdef __cplusplus
extern "C" {
#endif


/**
 * A type of state parameter used in combination with the OI_STATUS code to indicate the phase of a
 * BIP request. Depending on the request type the state transitions are as follows, where the
 *
 * For a multi-part request:
 *
 * OI_BIPSRV_REQUEST_INITIAL/OI_OBEX_CONTINUE -> OI_BIPSRV_REQUEST_CONTINUE/OI_OBEX_CONTINUE) -> OI_BIPSRV_REQUEST_CONTINUE/OI_OK -> OI_BIPSRV_REQUEST_CLEANUP/OI_OK
 *
 * For a 2-part request:
 *
 * OI_BIPSRV_REQUEST_INITIAL/OI_OBEX_CONTINUE -> OI_BIPSRV_REQUEST_CONTINUE/OI_OK -> OI_BIPSRV_REQUEST_CLEANUP/OI_OK
 *
 * For an atomic request:
 *
 * OI_BIPSRV_REQUEST_INITIAL/OI_OK -> OI_BIPSRV_REQUEST_CLEANUP/OI_OK
 *
 * In the case of an error OI_BIPSRV_REQUEST_CLEANUP/<error> can be indicated at any time and
 * completes the request.
 *
 * For all state except OI_BIPSRV_REQUEST_CLEANUP the application must issues an explicit PUT
 * or GET response or return an error status from the request indication.
 *
 * Some requests return request-specic status codes.
 *
 * Note that OI_BIPSRV_REQUEST_CLEANUP is always reported and completes the request.
 */
typedef enum {
    OI_BIPSRV_REQUEST_INITIAL,  /**< Start of a request */
    OI_BIPSRV_REQUEST_CONTINUE, /**< Continuation of a multi-part request */
    OI_BIPSRV_REQUEST_CLEANUP,  /**< Indicates either an error or that the last response has been sent */
} OI_BIPSRV_REQUEST_STATE;


typedef OI_OBEXSRV_CONNECTION_HANDLE OI_BIPSRV_CONNECTION_HANDLE;


/**
 * Represents a registered FTP server
 */
typedef OI_OBEX_SERVER_HANDLE OI_BIP_SERVER_HANDLE;


/**
 * Indicates a request for a BIP connection. The server should call @ref
 * OI_BIPSRV_AcceptConnection to allow the connection or to reject it.
 *
 * @param connectionId  Connection handle to be used in conjunction with this
 * connection in future calls.
 */
typedef void (*OI_BIPSRV_CONNECT_IND)(OI_BIPSRV_CONNECTION_HANDLE connectionId);

/**
 * Indicates that a BIP client has terminated its connection to the server
 *
 * @param connection Connection handle to the connection which is being dropped.
 */
typedef void (*OI_BIPSRV_DISCONNECT_IND)(OI_BIPSRV_CONNECTION_HANDLE connectionId);

/**
 * This callback is invoked prior to a connection indication when the server has
 * been configured to require authentication. The server should respond by
 * calling OI_BIPSRV_AuthenticationResponse() with the pin to use for this
 * connection.
 @param connectionId Connection handle to the connection on which authentication is being performed.
 */
typedef void (*OI_BIPSRV_AUTH_IND)(OI_BIPSRV_CONNECTION_HANDLE connectionId);

/**
 * The server responds to an authentication request with this function.
 * @param connectionId  Connection handle to the connection on which authentication is being performed.
 */
OI_STATUS OI_BIPSRV_AuthenticationResponse(OI_BIPSRV_CONNECTION_HANDLE connectionId,
                                           OI_CHAR *pin);


/* ************* GetCapabilities */

/**
  @param connectionId Connection handle to the connection in question.
  @param params Parameters specified by the BIP client.
  @param state The phase of the response
  @param status Whether the client has more data (deferred result from last response sent)
  @return OI_OK if the operation was successful.
  */
typedef OI_STATUS (*OI_BIPSRV_GET_CAPABILITIES_IND)(OI_BIPSRV_CONNECTION_HANDLE connectionId,
                                                    OI_BIP_GET_CAPABILITIES_PARAMS *params,
                                                    OI_BIPSRV_REQUEST_STATE state,
                                                    OI_STATUS status);


/**
  @param connectionId Connection handle to the connection in question.
  @param params Parameters specified by the BIP client.
  @param result Returned by callback function; always OI_OK except in case of catastrophic failure.
  @return OI_OK if the operation was successful.
  */
OI_STATUS OI_BIPSRV_GetCapabilitiesResponse(OI_BIPSRV_CONNECTION_HANDLE connectionId,
                                            OI_BIP_GET_CAPABILITIES_PARAMS *params,
                                            OI_STATUS result);
/* ************* PutImage */

/**
  @param connectionId Connection handle to the connection in question.
  @param params Parameters specified by the BIP client.
  @param state The phase of the response
  @param status Whether the client has more data (deferred result from last response sent)
  @return OI_OK if the operation was successful.
  */
typedef OI_STATUS (*OI_BIPSRV_PUT_IMAGE_IND)(OI_BIPSRV_CONNECTION_HANDLE connectionId,
                                             OI_BIP_PUT_IMAGE_PARAMS *params,
                                             OI_BIPSRV_REQUEST_STATE state,
                                             OI_STATUS status);

/**
  @param connectionId Connection handle to the connection in question.
  @param params Parameters specified by the BIP client.
  @param result Returned by callback function; always OI_OK except in case of catastrophic failure.
  @return OI_OK if the operation was successful.
  */
OI_STATUS OI_BIPSRV_PutImageResponse(OI_BIPSRV_CONNECTION_HANDLE connectionId,
                                     OI_BIP_PUT_IMAGE_PARAMS *params,
                                     OI_STATUS result);

/* ************* PutLinkedThumbnail */

/**
  @param connectionId Connection handle to the connection in question.
  @param params Parameters specified by the BIP client.
  @param state The phase of the response
  @param status Whether the client has more data (deferred result from last response sent)
  @return OI_OK if the operation was successful.
  */
typedef OI_STATUS (*OI_BIPSRV_PUT_LINKED_THUMBNAIL_IND)(OI_BIPSRV_CONNECTION_HANDLE connectionId,
                                                        OI_BIP_PUT_LINKED_THUMBNAIL_PARAMS *params,
                                                        OI_BIPSRV_REQUEST_STATE state,
                                                        OI_STATUS status);
/**
  @param connectionId Connection handle to the connection in question.
  @param params Parameters specified by the BIP client.
  @param result Returned by callback function; always OI_OK except in case of catastrophic failure.
  @return OI_OK if the operation was successful.
  */
OI_STATUS OI_BIPSRV_PutLinkedThumbnailResponse(OI_BIPSRV_CONNECTION_HANDLE connectionId,
                                               OI_BIP_PUT_LINKED_THUMBNAIL_PARAMS *params,
                                               OI_STATUS result);


/* ************* PutLinkedAttachment */

/**
  @param connectionId Connection handle to the connection in question.
  @param params Parameters specified by the BIP client.
  @param state The phase of the response
  @param status Whether the client has more data (deferred result from last response sent)
  @return OI_OK if the operation was successful.
  */
typedef OI_STATUS (*OI_BIPSRV_PUT_LINKED_ATTACHMENT_IND)(OI_BIPSRV_CONNECTION_HANDLE connectionId,
                                                         OI_BIP_PUT_LINKED_ATTACHMENT_PARAMS *params,
                                                         OI_BIPSRV_REQUEST_STATE state,
                                                         OI_STATUS status);

/**
  @param connectionId Connection handle to the connection in question.
  @param params Parameters specified by the BIP client.
  @param result Returned by callback function; always OI_OK except in case of catastrophic failure.
  @return OI_OK if the operation was successful.
  */
OI_STATUS OI_BIPSRV_PutLinkedAttachmentResponse(OI_BIPSRV_CONNECTION_HANDLE connectionId,
                                                OI_BIP_PUT_LINKED_ATTACHMENT_PARAMS *params,
                                                OI_STATUS result);


/* ************* RemoteDisplay */

/**
  @param connectionId Connection handle to the connection in question.
  @param params Parameters specified by the BIP client.
  @param state The phase of the response
  @param status Whether the client has more data (deferred result from last response sent)
  @return OI_OK if the operation was successful.
  */
typedef OI_STATUS (*OI_BIPSRV_REMOTE_DISPLAY_IND)(OI_BIPSRV_CONNECTION_HANDLE connectionId,
                                                  OI_BIP_REMOTE_DISPLAY_PARAMS *params,
                                                  OI_BIPSRV_REQUEST_STATE state,
                                                  OI_STATUS status);

/**
  @param connectionId Connection handle to the connection in question.
  @param params Parameters specified by the BIP client.
  @param result Returned by callback function; always OI_OK except in case of catastrophic failure.
  @return OI_OK if the operation was successful.
  */
OI_STATUS OI_BIPSRV_RemoteDisplayResponse(OI_BIPSRV_CONNECTION_HANDLE connectionId,
                                          OI_BIP_REMOTE_DISPLAY_PARAMS *params,
                                          OI_STATUS result);


/* ************* GetImagesList */

/**
  @param connectionId Connection handle to the connection in question.
  @param params Parameters specified by the BIP client.
  @param state The phase of the response
  @param status Whether the client has more data (deferred result from last response sent)
  @return OI_OK if the operation was successful.
  */
typedef OI_STATUS (*OI_BIPSRV_GET_IMAGES_LIST_IND)(OI_BIPSRV_CONNECTION_HANDLE connectionId,
                                                   OI_BIP_GET_IMAGES_LIST_PARAMS *params,
                                                   OI_BIPSRV_REQUEST_STATE state,
                                                   OI_STATUS status);
/**
  @param connectionId Connection handle to the connection in question.
  @param params Parameters specified by the BIP client.
  @param result Returned by callback function; always OI_OK except in case of catastrophic failure.
  @return OI_OK if the operation was successful.
  */
OI_STATUS OI_BIPSRV_GetImagesListResponse(OI_BIPSRV_CONNECTION_HANDLE connectionId,
                                          OI_BIP_GET_IMAGES_LIST_PARAMS *params,
                                          OI_STATUS result);

/* ************* GetImageProperties */



/**
  @param connectionId Connection handle to the connection in question.
  @param params Parameters specified by the BIP client.
  @param state The phase of the response
  @param status Whether the client has more data (deferred result from last response sent)
  @return OI_OK if the operation was successful.
  */
typedef OI_STATUS (*OI_BIPSRV_GET_IMAGE_PROPERTIES_IND)(OI_BIPSRV_CONNECTION_HANDLE connectionId,
                                                        OI_BIP_GET_IMAGE_PROPERTIES_PARAMS *params,
                                                        OI_BIPSRV_REQUEST_STATE state,
                                                        OI_STATUS status);
/**
  @param connectionId Connection handle to the connection in question.
  @param params Parameters specified by the BIP client.
  @param result Returned by callback function; always OI_OK except in case of catastrophic failure.
  @return OI_OK if the operation was successful.
  */
OI_STATUS OI_BIPSRV_GetImagePropertiesResponse(OI_BIPSRV_CONNECTION_HANDLE connectionId,
                                               OI_BIP_GET_IMAGE_PROPERTIES_PARAMS *params,
                                               OI_STATUS result);

/* ************* GetImage */

/**
  @param connectionId Connection handle to the connection in question.
  @param params Parameters specified by the BIP client.
  @param state The phase of the response
  @param status Whether the client has more data (deferred result from last response sent)
  @return OI_OK if the operation was successful.
  */
typedef OI_STATUS (*OI_BIPSRV_GET_IMAGE_IND)(OI_BIPSRV_CONNECTION_HANDLE connectionId,
                                             OI_BIP_GET_IMAGE_PARAMS *params,
                                             OI_BIPSRV_REQUEST_STATE state,
                                             OI_STATUS status);

/**
  @param connectionId Connection handle to the connection in question.
  @param params Parameters specified by the BIP client.
  @param result Returned by callback function; always OI_OK except in case of catastrophic failure.
  @return OI_OK if the operation was successful.
  */
OI_STATUS OI_BIPSRV_GetImageResponse(OI_BIPSRV_CONNECTION_HANDLE connectionId,
                                     OI_BIP_GET_IMAGE_PARAMS *params,
                                     OI_STATUS result);

/* ************* GetLinkedThumbnail */


/**
  @param connectionId Connection handle to the connection in question.
  @param params Parameters specified by the BIP client.
  @param state The phase of the response
  @param status Whether the client has more data (deferred result from last response sent)
  @return OI_OK if the operation was successful.
  */
typedef OI_STATUS (*OI_BIPSRV_GET_LINKED_THUMBNAIL_IND)(OI_BIPSRV_CONNECTION_HANDLE connectionId,
                                                        OI_BIP_GET_LINKED_THUMBNAIL_PARAMS *params,
                                                        OI_BIPSRV_REQUEST_STATE state,
                                                        OI_STATUS status);
/**
  @param connectionId Connection handle to the connection in question.
  @param params Parameters specified by the BIP client.
  @param result Returned by callback function; always OI_OK except in case of catastrophic failure.
  @return OI_OK if the operation was successful.
  */
OI_STATUS OI_BIPSRV_GetLinkedThumbnailResponse(OI_BIPSRV_CONNECTION_HANDLE connectionId,
                                               OI_BIP_GET_LINKED_THUMBNAIL_PARAMS *params,
                                               OI_STATUS result);


/* ************* GetLinkedAttachment */


/**
  @param connectionId Connection handle to the connection in question.
  @param params Parameters specified by the BIP client.
  @param state The phase of the response
  @param status Whether the client has more data (deferred result from last response sent)
  @return OI_OK if the operation was successful.
  */
typedef OI_STATUS (*OI_BIPSRV_GET_LINKED_ATTACHMENT_IND)(OI_BIPSRV_CONNECTION_HANDLE connectionId,
                                                         OI_BIP_GET_LINKED_ATTACHMENT_PARAMS *params,
                                                         OI_BIPSRV_REQUEST_STATE state,
                                                         OI_STATUS status);

/**
  @param connectionId Connection handle to the connection in question.
  @param params Parameters specified by the BIP client.
  @param result Returned by callback function; always OI_OK except in case of catastrophic failure.
  @return OI_OK if the operation was successful.
  */
OI_STATUS OI_BIPSRV_GetLinkedAttachmentResponse(OI_BIPSRV_CONNECTION_HANDLE connectionId,
                                                OI_BIP_GET_LINKED_ATTACHMENT_PARAMS *params,
                                                OI_STATUS result);

/* ************* DeleteImage */


/**
  @param connectionId Connection handle to the connection in question.
  @param params Parameters specified by the BIP client.
  @param state The phase of the response
  @param status Whether the client has more data (deferred result from last response sent)
  @return OI_OK if the operation was successful.
  */
typedef OI_STATUS (*OI_BIPSRV_DELETE_IMAGE_IND)(OI_BIPSRV_CONNECTION_HANDLE connectionId,
                                                OI_BIP_DELETE_IMAGE_PARAMS *params,
                                                OI_BIPSRV_REQUEST_STATE state,
                                                OI_STATUS status);

/**
  @param connectionId Connection handle to the connection in question.
  @param params Parameters specified by the BIP client.
  @param result Returned by callback function; always OI_OK except in case of catastrophic failure.
  @return OI_OK if the operation was successful.
  */
OI_STATUS OI_BIPSRV_DeleteImageResponse(OI_BIPSRV_CONNECTION_HANDLE connectionId,
                                        OI_BIP_DELETE_IMAGE_PARAMS *params,
                                        OI_STATUS result);

/* ************* StartPrint */


/**
  @param connectionId Connection handle to the connection in question.
  @param params Parameters specified by the BIP client.
  @param state The phase of the response
  @param status Whether the client has more data (deferred result from last response sent)
  @return OI_OK if the operation was successful.
  */
typedef OI_STATUS (*OI_BIPSRV_START_PRINT_IND)(OI_BIPSRV_CONNECTION_HANDLE connectionId,
                                               OI_BIP_START_PRINT_PARAMS *params,
                                               OI_BIPSRV_REQUEST_STATE state,
                                               OI_STATUS status);

/**
  @param connectionId Connection handle to the connection in question.
  @param params Parameters specified by the BIP client.
  @param result Returned by callback function; always OI_OK except in case of catastrophic failure.
  @return OI_OK if the operation was successful.
  */
OI_STATUS OI_BIPSRV_StartPrintResponse(OI_BIPSRV_CONNECTION_HANDLE connectionId,
                                       OI_BIP_START_PRINT_PARAMS *params,
                                       OI_STATUS result);


/* ************* GetPartialImage */


/**
  @param connectionId Connection handle to the connection in question.
  @param params Parameters specified by the BIP client.
  @param state The phase of the response
  @param status Whether the client has more data (deferred result from last response sent)
  @return OI_OK if the operation was successful.
  */
typedef OI_STATUS (*OI_BIPSRV_GET_PARTIAL_IMAGE_IND)(OI_BIPSRV_CONNECTION_HANDLE connectionId,
                                                     OI_BIP_GET_PARTIAL_IMAGE_PARAMS *params,
                                                     OI_BIPSRV_REQUEST_STATE state,
                                                     OI_STATUS status);

/**
  @param connectionId Connection handle to the connection in question.
  @param params Parameters specified by the BIP client.
  @param result Returned by callback function; always OI_OK except in case of catastrophic failure.
  @return OI_OK if the operation was successful.
  */
OI_STATUS OI_BIPSRV_GetPartialImageResponse(OI_BIPSRV_CONNECTION_HANDLE connectionId,
                                            OI_BIP_GET_PARTIAL_IMAGE_PARAMS *params,
                                            OI_STATUS result);


/* ************* StartArchive */


/**
  @param connectionId Connection handle to the connection in question.
  @param params Parameters specified by the BIP client.
  @param state The phase of the response
  @param status Whether the client has more data (deferred result from last response sent)
  @return OI_OK if the operation was successful.
  */
typedef OI_STATUS (*OI_BIPSRV_START_ARCHIVE_IND)(OI_BIPSRV_CONNECTION_HANDLE connectionId,
                                                 OI_BIP_START_ARCHIVE_PARAMS *params,
                                                 OI_BIPSRV_REQUEST_STATE state,
                                                 OI_STATUS status);

/**
  @param connectionId Connection handle to the connection in question.
  @param params Parameters specified by the BIP client.
  @param result Returned by callback function; always OI_OK except in case of catastrophic failure.
  @return OI_OK if the operation was successful.
  */
OI_STATUS OI_BIPSRV_StartArchiveResponse(OI_BIPSRV_CONNECTION_HANDLE connectionId,
                                         OI_BIP_START_ARCHIVE_PARAMS *params,
                                         OI_STATUS result);

/* ************* GetMonitoringImage */


/**
  @param connectionId Connection handle to the connection in question.
  @param params Parameters specified by the BIP client.
  @param state The phase of the response
  @param status Whether the client has more data (deferred result from last response sent)
  @return OI_OK if the operation was successful.
  */
typedef OI_STATUS (*OI_BIPSRV_GET_MONITORING_IMAGE_IND)(OI_BIPSRV_CONNECTION_HANDLE connectionId,
                                                        OI_BIP_GET_MONITORING_IMAGE_PARAMS *params,
                                                        OI_BIPSRV_REQUEST_STATE state,
                                                        OI_STATUS status);

/**
  @param connectionId Connection handle to the connection in question.
  @param params Parameters specified by the BIP client.
  @param result Returned by callback function; always OI_OK except in case of catastrophic failure.
  @return OI_OK if the operation was successful.
  */
OI_STATUS OI_BIPSRV_GetMonitoringImageResponse(OI_BIPSRV_CONNECTION_HANDLE connectionId,
                                               OI_BIP_GET_MONITORING_IMAGE_PARAMS *params,
                                               OI_STATUS result);


/* ************* GetStatus  */


/**
  @param connectionId Connection handle to the connection in question.
  @param status Whether the client has more data (deferred result from last response sent)
  @return OI_OK if the operation was successful.
  */
typedef OI_STATUS (*OI_BIPSRV_GET_STATUS_IND)(OI_BIPSRV_CONNECTION_HANDLE connectionId,
                                              OI_STATUS status);

/**
  @param connectionId Connection handle to the connection in question.
  @param result Returned by callback function; always OI_OK except in case of catastrophic failure.
  @return OI_OK if the operation was successful.
  */
OI_STATUS OI_BIPSRV_GetStatusResponse(OI_BIPSRV_CONNECTION_HANDLE connectionId,
                                      OI_STATUS result);



/**
 * A application calls thsi function to accept or reject an indicated
 * connection.
 *
 * @param connectionId   handle representing the connection to an BIP client.
 *
 * @param accept         TRUE if the connection is being accepted, FALSE
 *                       otherwise.
 */

OI_STATUS OI_BIPSRV_AcceptConnection(OI_BIPSRV_CONNECTION_HANDLE connectionId,
                                     OI_BOOL accept);


/* *************  */

typedef struct {
    OI_BIPSRV_CONNECT_IND connectInd;
    OI_BIPSRV_DISCONNECT_IND disconnectInd;
    OI_BIPSRV_AUTH_IND authInd;

    OI_BIPSRV_GET_CAPABILITIES_IND getCapabilities;
    OI_BIPSRV_PUT_IMAGE_IND putImage;
    OI_BIPSRV_PUT_LINKED_ATTACHMENT_IND putLinkedAttachment;
    OI_BIPSRV_PUT_LINKED_THUMBNAIL_IND putLinkedThumbnail;
    OI_BIPSRV_REMOTE_DISPLAY_IND remoteDisplay;
    OI_BIPSRV_GET_IMAGES_LIST_IND getImagesList;
    OI_BIPSRV_GET_IMAGE_PROPERTIES_IND getImageProperties;
    OI_BIPSRV_GET_IMAGE_IND getImage;
    OI_BIPSRV_GET_LINKED_THUMBNAIL_IND getLinkedThumbnail;
    OI_BIPSRV_GET_LINKED_ATTACHMENT_IND getLinkedAttachment;
    OI_BIPSRV_DELETE_IMAGE_IND deleteImage;
    OI_BIPSRV_START_PRINT_IND startPrint;
    OI_BIPSRV_START_ARCHIVE_IND startArchive;
    OI_BIPSRV_GET_STATUS_IND getStatus;
    OI_BIPSRV_GET_MONITORING_IMAGE_IND getMonitoringImage;
} OI_BIPSRV_IMAGING_RESPONDER_CALLBACKS;

typedef struct {
    OI_BIPSRV_CONNECT_IND connectInd;
    OI_BIPSRV_DISCONNECT_IND disconnectInd;
    OI_BIPSRV_AUTH_IND authInd;

    OI_BIPSRV_GET_CAPABILITIES_IND getCapabilities;
    OI_BIPSRV_GET_PARTIAL_IMAGE_IND getPartialImage;
} OI_BIPSRV_REFERENCED_OBJECTS_CALLBACKS;

typedef struct {
    OI_BIPSRV_CONNECT_IND connectInd;
    OI_BIPSRV_DISCONNECT_IND disconnectInd;
    OI_BIPSRV_AUTH_IND authInd;

    OI_BIPSRV_GET_CAPABILITIES_IND getCapabilities;
    OI_BIPSRV_GET_IMAGES_LIST_IND getImagesList;
    OI_BIPSRV_GET_IMAGE_PROPERTIES_IND getImageProperties;
    OI_BIPSRV_GET_IMAGE_IND getImage;
    OI_BIPSRV_GET_LINKED_THUMBNAIL_IND getLinkedThumbnail;
    OI_BIPSRV_GET_LINKED_ATTACHMENT_IND getLinkedAttachment;
    OI_BIPSRV_DELETE_IMAGE_IND deleteImage;
} OI_BIPSRV_ARCHIVED_OBJECTS_CALLBACKS;

typedef struct {
    OI_UINT8 supportedCapabilities;
    OI_UINT16 supportedFeatures;
    OI_UINT32 supportedFunctions;
    OI_UINT64 imagingCapacity;
} OI_BIPSRV_IMAGING_RESPONDER_INFO;

typedef struct {
    OI_UUID128 serviceId;
    OI_UINT32 supportedFunctions;
} OI_BIPSRV_OBJECTS_SERVER_INFO;

/**
 * This function registers an imaging responder server.
 *
 * @param cb               pointer to a struct specifying callbacks for the functions
 *                         implemented by this server
 *
 * @param info             pointer to a struct containing information about the server, to
 *                         be published via SDP. The value of
 *                         info->supportedFeatures determines which OBEX targets
 *                         will be registered. At least one of its bits must be
 *                         set, or registration will fail.
 *
 * @param strings          name to register in service record
 * @param authentication   TRUE if authentication is to be required of clients,
 *                         FALSE otherwise
 *
 * @param serverHandle     [OUT] returns the handle of the registered server
 *
 * @return                 OI_OK if registration completed successfully, an error otherwise
 */
OI_STATUS OI_BIPSRV_RegisterImagingResponder(const OI_BIPSRV_IMAGING_RESPONDER_CALLBACKS *cb,
                                             OI_BIPSRV_IMAGING_RESPONDER_INFO *info,
                                             const OI_SDP_STRINGS *strings,
                                             OI_BOOL authentication,
                                             OI_BIP_SERVER_HANDLE *serverHandle);

/**
 * Registers a referenced objects server.
 *
 * @param cb   Pointer to a struct specifying callbacks for the functions
 * implemented by this server.
 *
 * @param info  Pointer to a struct containing information about the server, to
 * be published via SDP.
 *
 * @param strings           name to register in service record
 * @param authentication    TRUE if authentication is to be required of clients,
 * FALSE otherwise
 *
 * @param serverHandle     [OUT] returns the handle of the registered server
 *
 * @return  OI_OK if registration completed successfully, an error otherwise.
 */
OI_STATUS OI_BIPSRV_RegisterReferencedObjects(const OI_BIPSRV_REFERENCED_OBJECTS_CALLBACKS *cb,
                                              OI_BIPSRV_OBJECTS_SERVER_INFO *info,
                                              const OI_SDP_STRINGS *strings,
                                              OI_BOOL authentication,
                                              OI_BIP_SERVER_HANDLE *serverHandle);

/**
 * Registers an archived objects server.
 *
 * @param cb   Pointer to a struct specifying callbacks for the functions
 * implemented by this server.
 *
 * @param info  Pointer to a struct containing information about the server, to
 * be published via SDP.
 *
 * @param strings           name to register in service record
 *
 * @param authentication    TRUE if authentication is to be required of clients,
 * FALSE otherwise
 *
 * @param serverHandle   Out parameter assigned the handle of the registered server.
 *
 * @return  OI_OK if registration completed successfully, an error otherwise.
 */
OI_STATUS OI_BIPSRV_RegisterArchivedObjects(const OI_BIPSRV_ARCHIVED_OBJECTS_CALLBACKS *cb,
                                            OI_BIPSRV_OBJECTS_SERVER_INFO *info,
                                            const OI_SDP_STRINGS *strings,
                                            OI_BOOL authentication,
                                            OI_BIP_SERVER_HANDLE *serverHandle);

/**
 * Get the service record handle associated with this service.
 * This can be used with e.g. OI_SDPDB_SetAttributeList to add
 * vendor-specific SDP attributes to the profile.
 *
 * @param serverHandle   The handle of the server
 *
 * @param handle  return the service record's handle
 */
OI_STATUS OI_BIPSRV_GetServiceRecord(OI_BIP_SERVER_HANDLE serverHandle,
                                     OI_UINT32 *handle);

/**
 * Deregisters a previously registered BIP server.
 *
 * @param serverHandle   The handle of the server to deregister.
 *
 * @return  OI_OK if registration completed successfully, an error otherwise.
 */
OI_STATUS OI_BIPSRV_DeregisterServer(OI_BIP_SERVER_HANDLE serverHandle);


/**
 * Associates a caller defined context with an BIP server connection. This
 * context can then be retrieved by calling OI_BIPSRV_GetContext().
 *
 * @param connection     The connection to associate the context with.
 *
 * @param context         A value supplied by the caller.
 *
 * @return                OI_OK if the context was set, OI_STATUS_NOT_FOUND if
 *                        the connection handle is not valid.
 */
OI_STATUS OI_BIPSRV_SetContext(OI_BIPSRV_CONNECTION_HANDLE connection,
                               void *context);


/**
 * Gets a caller defined context associate with an BIP server connection. This
 * is a value that was previously set by a call to OI_BIPSRV_SetContext().
 *
 * @param connection      The BIP server connection to get the context from.
 *
 * @return                A context pointer or NULL if the handle is invalid or
 *                        there is no context associated with this connection.
 */
void* OI_BIPSRV_GetContext(OI_BIPSRV_CONNECTION_HANDLE connection);

/**
 * This function forcibly severs the connection from a BIP client to the OBEX
 * server. The disconnect indication callback will be called when the disconnect
 * is complete.
 *
 * An BIP server may need to forcibly terminate a connection during
 * deregistration, since deregistration will fail if a connection is in place.
 *
 * @param connectionId     a unique identifier generated by the BIP server that
 *                         identifies the connection.
 *
 * @return                 OI_OK if the connectionId is valid and the connection
 *                         will be terminated.
 */
OI_STATUS OI_BIPSRV_ForceDisconnect(OI_BIPSRV_CONNECTION_HANDLE connectionId);


#ifdef __cplusplus
}
#endif

/**@}*/

#endif /* _OI_BIPSRV_SRV_H */


