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

This file provides the Basic Imaging Profile definitions.

Each BIP function (except GET_STATUS) has an associated parameter
block defined in this file. Although the parameter blocks differ for
the various functions, they are substantially similar. A parameter
block has a 'request' sub-struct, a 'response' sub-struct, or both. If
a function involves the transfer of body data, one of these
sub-structs will have 'data' and 'final' fields, depending on the
direction of the data transfer.

A BIP client implementation is responsible for setting the values in
the request field. A server implementation sets the values in the
response field.  If body data is present in a request or response, the
'data' field points to an OI_OBEX_BYTESEQ describing the data, and
'final' field indicates whether this is the final data packet.

If body data entails a function with more than one request-response
round-trip then the 'data' field (if present) should be the only
request field present after the first request. Similarly, the 'data'
field (if present) should be the only response field present before
the final response.
*/

#ifndef _OI_BIP_SPEC_H
#define _OI_BIP_SPEC_H

#include "oi_bt_spec.h"
#include "oi_stddefs.h"
#include "oi_obexspec.h"

/** \addtogroup BIP BIP APIs */
/**@{*/

#ifdef __cplusplus
extern "C" {
#endif


#define OI_BIP_VERSION 0x0100



#define OI_BIP_SDP_CAPABILITY_GENERIC_IMAGING       OI_BIT0 /**< BIP SDP constant for generic imaging capability.
                                                              BIP specification version 1.0 final, section 6.1.1. */
#define OI_BIP_SDP_CAPABILITY_CAPTURING             OI_BIT1 /**< BIP SDP constant for capturing capability.
                                                              BIP specification version 1.0 final, section 6.1.1. */
#define OI_BIP_SDP_CAPABILITY_PRINTING              OI_BIT2 /**< BIP SDP constant for printing capability.
                                                              BIP specification version 1.0 final, section 6.1.1. */
#define OI_BIP_SDP_CAPABILITY_DISPLAYING            OI_BIT3 /**< BIP SDP constant for displaying capability.
                                                              BIP specification version 1.0 final, section 6.1.1. */

#define OI_BIP_SDP_FEATURE_IMAGE_PUSH               OI_BIT0 /**< BIP SDP constant for ImagePush feature.
                                                              BIP specification version 1.0 final, section 6.1.1. */
#define OI_BIP_SDP_FEATURE_IMAGE_PUSH_STORE         OI_BIT1 /**< BIP SDP constant for ImagePush-Store feature.
                                                              BIP specification version 1.0 final, section 6.1.1. */
#define OI_BIP_SDP_FEATURE_IMAGE_PUSH_PRINT         OI_BIT2 /**< BIP SDP constant for ImagePush-Print feature.
                                                              BIP specification version 1.0 final, section 6.1.1. */
#define OI_BIP_SDP_FEATURE_IMAGE_PUSH_DISPLAY       OI_BIT3 /**< BIP SDP constant for ImagePush-Display feature.
                                                              BIP specification version 1.0 final, section 6.1.1. */
#define OI_BIP_SDP_FEATURE_IMAGE_PULL               OI_BIT4 /**< BIP SDP constant for ImagePull feature.
                                                              BIP specification version 1.0 final, section 6.1.1. */
#define OI_BIP_SDP_FEATURE_ADVANCED_IMAGE_PRINTING  OI_BIT5 /**< BIP SDP constant for AdvancedImagePrinting feature.
                                                              BIP specification version 1.0 final, section 6.1.1. */
#define OI_BIP_SDP_FEATURE_AUTOMATIC_ARCHIVE        OI_BIT6 /**< BIP SDP constant for AutomaticArchive feature.
                                                              BIP specification version 1.0 final, section 6.1.1. */
#define OI_BIP_SDP_FEATURE_REMOTE_CAMERA            OI_BIT7 /**< BIP SDP constant for RemoteCamera feature.
                                                              BIP specification version 1.0 final, section 6.1.1. */
#define OI_BIP_SDP_FEATURE_REMOTE_DISPLAY           OI_BIT8 /**< BIP SDP constant for RemoteDisplay feature.
                                                              BIP specification version 1.0 final, section 6.1.1. */

#define OI_BIP_SDP_FUNCTION_GET_CAPABILITIES        OI_BIT0 /**< GetCapabilities function.
                                                              BIP specification version 1.0 final, section 4.5.1. */
#define OI_BIP_SDP_FUNCTION_PUT_IMAGE               OI_BIT1 /**< PutImage function.
                                                              BIP specification version 1.0 final, section 4.5.2. */
#define OI_BIP_SDP_FUNCTION_PUT_LINKED_ATTACHMENT   OI_BIT2 /**< PutLinkedAttachment function.
                                                              BIP specification version 1.0 final, section 4.5.4. */
#define OI_BIP_SDP_FUNCTION_PUT_LINKED_THUMBNAIL    OI_BIT3 /**< PutLinkedThumbnail function.
                                                              BIP specification version 1.0 final, section 4.5.3. */
#define OI_BIP_SDP_FUNCTION_REMOTE_DISPLAY          OI_BIT4 /**< RemoteDisplay function.
                                                              BIP specification version 1.0 final, section 4.5.5. */
#define OI_BIP_SDP_FUNCTION_GET_IMAGES_LIST         OI_BIT5 /**< GetImagesList function.
                                                              BIP specification version 1.0 final, section 4.5.6. */
#define OI_BIP_SDP_FUNCTION_GET_IMAGE_PROPERTIES    OI_BIT6 /**< GetImageProperties function.
                                                              BIP specification version 1.0 final, section 4.5.7. */
#define OI_BIP_SDP_FUNCTION_GET_IMAGE               OI_BIT7 /**< GetImage function.
                                                              BIP specification version 1.0 final, section 4.5.8. */
#define OI_BIP_SDP_FUNCTION_GET_LINKED_THUMBNAIL    OI_BIT8 /**< GetLinkedThumbnail function.
                                                              BIP specification version 1.0 final, section 4.5.9. */
#define OI_BIP_SDP_FUNCTION_GET_LINKED_ATTACHMENT   OI_BIT9 /**< GetLinkedAttachment function.
                                                              BIP specification version 1.0 final, section 4.5.10. */
#define OI_BIP_SDP_FUNCTION_DELETE_IMAGE            OI_BIT10 /**< DeleteImage function.
                                                               BIP specification version 1.0 final, section 4.5.11. */
#define OI_BIP_SDP_FUNCTION_START_PRINT             OI_BIT11 /**< StartPrint function.
                                                               BIP specification version 1.0 final, section 4.5.12. */
#define OI_BIP_SDP_FUNCTION_GET_PARTIAL_IMAGE       OI_BIT12 /**< GetPartialImage function.
                                                               BIP specification version 1.0 final, section 4.5.13. */
#define OI_BIP_SDP_FUNCTION_START_ARCHIVE           OI_BIT13 /**< StartArchive function.
                                                               BIP specification version 1.0 final, section 4.5.14. */
#define OI_BIP_SDP_FUNCTION_GET_MONITORING_IMAGE    OI_BIT14 /**< GetMonitoringImage function.
                                                               BIP specification version 1.0 final, section 4.5.16. */
    /* RESERVED OI_BIT15, */
#define OI_BIP_SDP_FUNCTION_GET_STATUS              OI_BIT16 /**< GetStatus function.
                                                               BIP specification version 1.0 final, section 4.5.15. */

/**
RemoteDisplay screen control commands.
See Bluetooth BIP specification version 1.0 final, section 4.5.5.
*/
typedef enum {
    OI_BIP_REMOTE_DISPLAY_NEXT_IMAGE = 1,
    OI_BIP_REMOTE_DISPLAY_PREVIOUS_IMAGE = 2,
    OI_BIP_REMOTE_DISPLAY_SELECT_IMAGE = 3,
    OI_BIP_REMOTE_DISPLAY_CURRENT_IMAGE = 4
} OI_BIP_REMOTE_DISPLAY_COMMAND;


/* BIP operation parameter structures */

/** This structure defines the prameter block for the
GetCapabilities function OI_BIPCLI_GetCapabilities(). See
Bluetooth BIP specification version 1.0 final, section 4.5.1. */
typedef struct {
    struct {
        /** See section 4.4.6.3 of the Bluetooth BIP specification, version 1.0. */
        OI_OBEX_BYTESEQ *data;
        OI_BOOL final;
    } response;
} OI_BIP_GET_CAPABILITIES_PARAMS;


/** This structure defines the parameter block for the
GetMonitoringImage function OI_BIPCLI_GetMonitoringImage(). See
Bluetooth BIP specification version 1.0 final, section
4.5.16. */
typedef struct {
    struct {
        OI_BOOL storeImage;
    } request;

    struct {
        /** See section 4.4.4 of the Bluetooth BIP specification, version 1.0. */
        OI_OBEX_UNICODE *imageHandle;
        /** See section 4.4.6.5 of the Bluetooth BIP specification, version 1.0. */
        OI_OBEX_BYTESEQ *data;
        OI_BOOL final;
    } response;
} OI_BIP_GET_MONITORING_IMAGE_PARAMS;


/** This structure defines the parameter block for the PutImage
function OI_BIPCLI_PutImage(). See Bluetooth BIP specification
version 1.0 final, section 4.5.2. */
typedef struct {
    struct {
        OI_OBEX_UNICODE *imageName;
        /** See section 4.4.7.2 of the Bluetooth BIP specification, version 1.0. */
        OI_OBEX_BYTESEQ *imageDescriptor;
        OI_OBEX_BYTESEQ *data;
        OI_BOOL final;
    } request;

    struct {
        /** See section 4.4.4 of the Bluetooth BIP specification, version 1.0. */
        OI_OBEX_UNICODE *imageHandle;
    } response;
} OI_BIP_PUT_IMAGE_PARAMS;


/** This structure defines the parameter block for the
PutLinkedAttachment function OI_BIPCLI_PutLinkedAttachment().
See Bluetooth BIP specification version 1.0 final, section
4.5.4. */
typedef struct {
    struct {
        /** See section 4.4.4 of the Bluetooth BIP specification, version 1.0. */
        OI_OBEX_UNICODE *imageHandle;
        /** See section 4.4.7.3 of the Bluetooth BIP specification, version 1.0. */
        OI_OBEX_BYTESEQ *attachmentDescriptor;
        OI_OBEX_BYTESEQ *data;
        OI_BOOL final;
    } request;
} OI_BIP_PUT_LINKED_ATTACHMENT_PARAMS;


/** This structure defines the prameter block for the
PutLinkedThumbnail function OI_BIPCLI_PutLinkedThumbnail(). See
Bluetooth BIP specification version 1.0 final, section 4.5.3. */
typedef struct {
    struct {
        /** See section 4.4.4 of the Bluetooth BIP specification, version 1.0. */
        OI_OBEX_UNICODE *imageHandle;
        /** See section 4.4.3 of the Bluetooth BIP specification, version 1.0. */
        OI_OBEX_BYTESEQ *data;
        OI_BOOL final;
    } request;
} OI_BIP_PUT_LINKED_THUMBNAIL_PARAMS;


/** This structure defines the parameter block for the
GetImagesList function OI_BIPCLI_GetImagesList(). See Bluetooth
BIP specification version 1.0 final, section 4.5.6. */
typedef struct {
    struct {
        OI_UINT16 handleCount;
        OI_UINT16 handleOffset;
        OI_UINT8 latest;
        /** See section 4.4.7.1 of the Bluetooth BIP specification, version 1.0. */
        OI_OBEX_BYTESEQ *handlesDescriptor;
    } request;

    struct {
        OI_UINT16 nbReturnedHandles;
        /** See section 4.4.7.1 of the Bluetooth BIP specification, version 1.0. */
        OI_OBEX_BYTESEQ *imageHandlesDescriptor;
        /** See section 4.4.6.1 of the Bluetooth BIP specification, version 1.0. */
        OI_OBEX_BYTESEQ *data;
        OI_BOOL final;
    } response;
} OI_BIP_GET_IMAGES_LIST_PARAMS;


/** This structure defines the parameter block for the GetImage
function OI_BIPCLI_GetImage(). See Bluetooth BIP specification
version 1.0 final, section 4.5.8. */
typedef struct {
    struct {
        /** See section 4.4.4 of the Bluetooth BIP specification, version 1.0. */
        OI_OBEX_UNICODE *imageHandle;
        /** See section 4.4.7.2 of the Bluetooth BIP specification, version 1.0. */
        OI_OBEX_BYTESEQ *imageDescriptor;
    } request;

    struct {
        OI_UINT32 imageFileSize;
        OI_OBEX_BYTESEQ *data;
        OI_BOOL final;
    } response;
} OI_BIP_GET_IMAGE_PARAMS;


/** This structure defines the prameter block for the
GetLinkedThumbnail function OI_BIPCLI_GetLinkedThumbnail(). See
Bluetooth BIP specification version 1.0 final, section 4.5.9. */
typedef struct {
    struct {
        /** See section 4.4.4 of the Bluetooth BIP specification, version 1.0. */
        OI_OBEX_UNICODE *imageHandle;
    } request;

    struct {
        /** See section 4.4.3 of the Bluetooth BIP specification, version 1.0. */
        OI_OBEX_BYTESEQ *data;
        OI_BOOL final;
    } response;
} OI_BIP_GET_LINKED_THUMBNAIL_PARAMS;


/** This structure defines the prameter block for the
GetLinkedAttachment function OI_BIPCLI_GetLinkedAttachment().
See Bluetooth BIP specification version 1.0 final, section
4.5.10. */
typedef struct {
    struct {
        /** See section 4.4.4 of the Bluetooth BIP specification, version 1.0. */
        OI_OBEX_UNICODE *imageHandle;
        OI_OBEX_UNICODE *attachmentName;
    } request;

    struct {
        OI_OBEX_BYTESEQ *data;
        OI_BOOL final;
    } response;
} OI_BIP_GET_LINKED_ATTACHMENT_PARAMS;


/** This structure defines the parameter block for the
StartPrint function OI_BIPCLI_StartPrint(). See Bluetooth BIP
specification version 1.0 final, section 4.5.12. */
typedef struct {
    struct {
        OI_UUID128 secondaryServiceId;
        /** See section 4.4.6.4 of the Bluetooth BIP specification, version 1.0. */
        OI_OBEX_BYTESEQ *data;
        OI_BOOL final;
    } request;
} OI_BIP_START_PRINT_PARAMS;


/** This structure defines the prameter block for the
StartArchive function OI_BIPCLI_StartArchive(). See Bluetooth
BIP specification version 1.0 final, section 4.5.14. */
typedef struct {
    struct {
        OI_UUID128 secondaryServiceId;
    } request;
} OI_BIP_START_ARCHIVE_PARAMS;


/** This structure defines the prameter block for the
DeleteImage function OI_BIPCLI_DeleteImage(). See Bluetooth BIP
specification version 1.0 final, section 4.5.11. */
typedef struct {
    struct {
        /** See section 4.4.4 of the Bluetooth BIP specification, version 1.0. */
        OI_OBEX_UNICODE *imageHandle;
    } request;
} OI_BIP_DELETE_IMAGE_PARAMS;


/** This structure defines the parameter block for the
GetPartialImage function OI_BIPCLI_GetPartialImage(). See
Bluetooth BIP specification version 1.0 final, section
4.5.13. */
typedef struct {
    struct {
        OI_OBEX_UNICODE *imageName;
        OI_UINT32 partialLength;
        OI_UINT32 partialOffset;
    } request;

    struct {
        OI_UINT32 partialLength;
        OI_UINT32 totalFileSize;
        OI_BOOL endFlag;
        OI_OBEX_BYTESEQ *data;
        OI_BOOL final;
    } response;
} OI_BIP_GET_PARTIAL_IMAGE_PARAMS;


/** This structure defines the prameter block for the
GetImageProperties function OI_BIPCLI_GetImageProperties(). See
Bluetooth BIP specification version 1.0 final, section 4.5.7. */
typedef struct {
    struct {
        /** See section 4.4.4 of the Bluetooth BIP specification, version 1.0. */
        OI_OBEX_UNICODE *imageHandle;
    } request;

    struct {
        /** See section 4.4.6.2 of the Bluetooth BIP specification, version 1.0. */
        OI_OBEX_BYTESEQ *data;
        OI_BOOL final;
    } response;
} OI_BIP_GET_IMAGE_PROPERTIES_PARAMS;


/** This structure defines the prameter block for the
RemoteDisplay function OI_BIPCLI_RemoteDisplay(). See Bluetooth
BIP specification version 1.0 final, section 4.5.5. */
typedef struct {
    struct {
        /** See section 4.4.4 of the Bluetooth BIP specification, version 1.0. */
        OI_OBEX_UNICODE *imageHandle;
        OI_UINT8 displayCommand;
    } request;

    struct {
        /** See section 4.4.4 of the Bluetooth BIP specification, version 1.0. */
        OI_OBEX_UNICODE *imageHandle;
    } response;
} OI_BIP_REMOTE_DISPLAY_PARAMS;


/** This structure is a union of all of the BIP parameter
    blocks. */
typedef union {
    OI_BIP_GET_CAPABILITIES_PARAMS      getCapabilities;
    OI_BIP_GET_MONITORING_IMAGE_PARAMS  getMonitoringImage;
    OI_BIP_PUT_IMAGE_PARAMS             putImage;
    OI_BIP_PUT_LINKED_ATTACHMENT_PARAMS putLinkedAttachment;
    OI_BIP_PUT_LINKED_THUMBNAIL_PARAMS  putLinkedThumbnail;
    OI_BIP_GET_IMAGES_LIST_PARAMS       getImagesList;
    OI_BIP_GET_IMAGE_PARAMS             getImage;
    OI_BIP_GET_LINKED_THUMBNAIL_PARAMS  getLinkedThumbnail;
    OI_BIP_GET_LINKED_ATTACHMENT_PARAMS getLinkedAttachment;
    OI_BIP_START_PRINT_PARAMS           startPrint;
    OI_BIP_START_ARCHIVE_PARAMS         startArchive;
    OI_BIP_DELETE_IMAGE_PARAMS          deleteImage;
    OI_BIP_GET_PARTIAL_IMAGE_PARAMS     getPartialImage;
    OI_BIP_GET_IMAGE_PROPERTIES_PARAMS  getImageProperties;
    OI_BIP_REMOTE_DISPLAY_PARAMS        remoteDisplay;
} OI_BIP_PARAM_UNION;


/* *************** Primary target UUIDs *******************/

extern const OI_OBEX_BYTESEQ OI_BIP_ImagePushObexTargetUUID;
extern const OI_OBEX_BYTESEQ OI_BIP_ImagePullObexTargetUUID;
extern const OI_OBEX_BYTESEQ OI_BIP_AdvancedImagePrintingObexTargetUUID;
extern const OI_OBEX_BYTESEQ OI_BIP_AutomaticArchiveObexTargetUUID;
extern const OI_OBEX_BYTESEQ OI_BIP_RemoteCameraObexTargetUUID;
extern const OI_OBEX_BYTESEQ OI_BIP_RemoteDisplayObexTargetUUID;

#define OI_BIP_IMAGE_PUSH_OBEX_TARGET_UUID \
    { 0xE3, 0x3D, 0x95, 0x45, 0x83, 0x74, 0x4A, 0xD7, 0x9E, 0xC5, 0xC1, 0x6B, 0xE3, 0x1E, 0xDE, 0x8E }

#define OI_BIP_IMAGE_PULL_OBEX_TARGET_UUID \
    { 0x8E, 0xE9, 0xB3, 0xD0, 0x46, 0x08, 0x11, 0xD5, 0x84, 0x1A, 0x00, 0x02, 0xA5, 0x32, 0x5B, 0x4E }

#define OI_BIP_ADVANCED_IMAGE_PRINTING_OBEX_TARGET_UUID \
    { 0x92, 0x35, 0x33, 0x50, 0x46, 0x08, 0x11, 0xD5, 0x84, 0x1A, 0x00, 0x02, 0xA5, 0x32, 0x5B, 0x4E }

#define OI_BIP_AUTOMATIC_ARCHIVE_OBEX_TARGET_UUID \
    { 0x94, 0x01, 0x26, 0xC0, 0x46, 0x08, 0x11, 0xD5, 0x84, 0x1A, 0x00, 0x02, 0xA5, 0x32, 0x5B, 0x4E }

#define OI_BIP_REMOTE_CAMERA_OBEX_TARGET_UUID \
    { 0x94, 0x7E, 0x74, 0x20, 0x46, 0x08, 0x11, 0xD5, 0x84, 0x1A, 0x00, 0x02, 0xA5, 0x32, 0x5B, 0x4E }

#define OI_BIP_REMOTE_DISPLAY_OBEX_TARGET_UUID \
    { 0x94, 0xC7, 0xCD, 0x20, 0x46, 0x08, 0x11, 0xD5, 0x84, 0x1A, 0x00, 0x02, 0xA5, 0x32, 0x5B, 0x4E }

/* *************** Secondary target UUIDs *******************/

extern const OI_OBEX_BYTESEQ OI_BIP_ReferencedObjectsObexTargetUUID;
extern const OI_OBEX_BYTESEQ OI_BIP_ArchivedObjectsObexTargetUUID;

#define OI_BIP_REFERENCED_OBJECTS_OBEX_TARGET_UUID \
    { 0x8E, 0x61, 0xF9, 0x5D, 0x1A, 0x79, 0x11, 0xD4, 0x8E, 0xA4, 0x00, 0x80, 0x5F, 0x9B, 0x98, 0x34 }

#define OI_BIP_ARCHIVED_OBJECTS_OBEX_TARGET_UUID \
    { 0x8E, 0x61, 0xF9, 0x5E, 0x1A, 0x79, 0x11, 0xD4, 0x8E, 0xA4, 0x00, 0x80, 0x5F, 0x9B, 0x98, 0x34 }



#ifdef __cplusplus
}
#endif

/**@}*/

#endif /* _OI_BIP_SPEC_H */
