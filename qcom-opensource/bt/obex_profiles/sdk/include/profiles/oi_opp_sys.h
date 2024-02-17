#ifndef _OI_OPP_SYS_H
#define _OI_OPP_SYS_H

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

 This file provides the object system abstraction layer used by
 the  Object Push Profile client and server.

 This interface is described entirely as callback functions. The application
 must provide the sets of functions to the OPP client and OPP server
 when initializing these two services.

 Callbacks and errors are mutually exclusive.
    If a function below returns OI_OK, the callback must be called,
    either from within the function or on a later thread of execution.

    If a function below returns anything other than OI_OK, the callback
    must NOT be called.

 Note that if you wish your application to be
 BQB-compliant, you should generate error codes consistent with
 BLUEmagic 3.0 best practices; see the sample code for examples.

*/

#include "oi_status.h"
#include "oi_stddefs.h"
#include "oi_obex.h"
#include "oi_obexspec.h"


/** \addtogroup OPP OPP APIs */
/**@{*/

#ifdef __cplusplus
extern "C" {
#endif


/**
 * This type represents an opaque object that uniquely identifies an object that
 * has been opened for either reading or writing.
 */
typedef void* OI_OPP_HANDLE;


/**
 * This type represents a connection id for an OPP client or server connection
 * that is making calls into the object system abstraction layer.
 */
typedef OI_OBEX_CONNECTION_HANDLE OI_OPP_CONNECTION;


/* ********************************************************************************
 * The object functions are all asynchronous. However implementations are
 * permitted to call the callback function from within the function if the
 * operation can be completed quickly. This will be the case for many
 * implementations.
 * ********************************************************************************/



/**
 * A function of this type is called to confirm the success or failure opening
 * an object for reading
 *
 * @param objHandle   A handle that can be used to read the object contents.
 *
 * @param name        A null-terminated unicode string name of the object that was opened.
 *
 * @param type        A null-terminated ascii string for the object type. For example
 *                    for a vCard the type string is "text/x-vCard"
 *
 * @param size        If the object was opened for reading, size is the number
 *                    of bytes that can be read from the object. If the object
 *                    was opened for writing the value is undefined.
 *
 * @param data           A pointer to a buffer containing the data read.
 *
 * @param len            The number of bytes read.
 *
 * @param status      OI_OK if the object could be opened, an error if the object
 *                    could not be opened.
 *
 * @param oppConnection identifies which OPP client or server connection is
 *                   performing this operation.
 */
typedef void (*OI_OPP_OPEN_READ_CFM)(OI_OPP_HANDLE objHandle,
                                     const OI_OBEX_UNICODE *name,
                                     const OI_CHAR *type,
                                     OI_UINT32 size,
                                     OI_BYTE *data,
                                     OI_UINT32 len,
                                     OI_STATUS status,
                                     OI_OPP_CONNECTION oppConnection);

/**
 * This function and corresponding callback is needed by an OPP client and server.
 *
 * @param name           a null-terminated unicode string name of the object to
 *                       be opened. A NULL name means open the default object.
 *                       The default object is the owner's business card.
 *
 * @param type           a null-terminated ascii string for the object type.
 *
 * @param maxRead        the maximum number of bytes to read from the object on this
 *                       call.
 *
 * @param openCfm        the function that will be called when the open completes.
 *
 * @param oppConnection  identifies the OPP client or server connection that is
 *                       performing this operation.
 */
typedef OI_STATUS (*OI_OPP_OPEN_READ)(const OI_OBEX_UNICODE *name,
                                      const OI_CHAR *type,
                                      OI_UINT32 maxRead,
                                      OI_OPP_OPEN_READ_CFM openCfm,
                                      OI_OPP_CONNECTION oppConnection);


/**
 * A function of this type is called to confirm the success or failure opening
 * an object for writing.
 *
 * @param objHandle      A handle that can be used to write the object contents.
 *
 * @param status         OI_OK if the object could be opened to write or
 *                       an error if the object could not be opened.
 *
 * @param oppConnection  identifies the OPP client or server connection that is
 *                       performing this operation.
 */
typedef void (*OI_OPP_OPEN_WRITE_CFM)(OI_OPP_HANDLE objHandle,
                                      OI_STATUS status,
                                      OI_OPP_CONNECTION oppConnection);

/**
 * This function and corresponding callback is needed by an OPP client and server.
 *
 * @param name           a NULL terminated unicode string name of the object to
 *                       be opened. A NULL name means that the remote OPP device
 *                       did not provide a file name for the object.  (This can
 *                       happen when pulling a default object from a device that
 *                       follows the IrDA OBEX spec over the Bluetooth OPP spec.)
 *
 * @param type           a NULL terminated ascii string for the object type.
 *
 * @param objSize        a hint about the size of the object.
 *
 * @param openCfm        the function to be called when the open completes.
 *
 * @param oppConnection  identifies the OPP client or server connection that is
 *                       performing this operation.
 */
typedef OI_STATUS (*OI_OPP_OPEN_WRITE)(const OI_OBEX_UNICODE *name,
                                       const OI_CHAR *type,
                                       OI_UINT32 objSize,
                                       OI_OPP_OPEN_WRITE_CFM openCfm,
                                       OI_OPP_CONNECTION oppConnection);

/**
 * This function does not use a callback function. It is assumed that object
 * close will complete asynchronously or in the case of a failure will report or
 * log an error with the application.
 *
 * @param handle         A handle for an open object.
 *
 * @param status         OI_OK or an error status if the object is being closed
 *                       because of an error. In this case if the object was
 *                       being written the state of the object is unknown and
 *                       the application may choose to delete the object.
 *
 * @param oppConnection  identifies the OPP client or server connection that is
 *                       performing this operation.
 */
typedef void (*OI_OPP_CLOSE)(OI_OPP_HANDLE objHandle,
                             OI_STATUS status,
                             OI_OPP_CONNECTION oppConnection);

/**
 * A function of this type is called to confirm the success or failure of a
 * object read.
 *
 * @param objHandle      handle passed to the read call.
 *
 * @param data           A pointer to a buffer containing the data read.
 *
 * @param len            The number of bytes read.
 *
 * @param status         a status code indicating if the read was succesful:
 *                       - OI_OK if the read completed
 *                       - OI_STATUS_END_OF_FILE if the read was successful and
 *                         the end of object has been reached.
 *                       - An error status indicating that the read failed.
 *
 * @param oppConnection  identifies the OPP client or server connection that is
 *                       performing this operation.
 *
 */
typedef void (*OI_OPP_READ_CFM)(OI_OPP_HANDLE objHandle,
                                OI_BYTE *data,
                                OI_UINT32 len,
                                OI_STATUS status,
                                OI_OPP_CONNECTION oppConnection);

/**
 * This function and corresponding callback is needed by an OPP client that
 * supports object push and and opp server that supports object pull.
 *
 * @param objHandle   a handle previously returned by an OI_OPP_OPEN_CFM function
 *
 * @param maxRead     the maximum number of bytes to read from the object on this
 *                    call.
 *
 * @param readCfm     the function that will be called when the read completes.
 *
 * @param oppConnection  identifies the OPP client or server connection that is
 *                       performing this operation.
 */
typedef OI_STATUS (*OI_OPP_READ)(OI_OPP_HANDLE objHandle,
                                 OI_UINT32 maxRead,
                                 OI_OPP_READ_CFM readCfm,
                                 OI_OPP_CONNECTION oppConnection);


/**
 * A function of this type is called to confirm the success or failure of a object
 * read multiple.
 *
 * @param numBuffers     The number of reads buffers returned by this call
 *
 * @param data           An array of buffers containing the data read.
 *
 * @param len            An array of lengths for the number of bytes in each buffer
 *
  * @param status         a status code indicating if the read was succesful:
 *                       - OI_OK if the read completed
 *                       - OI_STATUS_END_OF_FILE if the read was successful and
 *                         the end of object has been reached.
 *                       - An error status indicating that the read failed.
 *
 * @param oppConnection  Identifies which OPP client or server connection is
 *                       performing this operation.
 *
 */

typedef void (*OI_OPP_READ_MULTIPLE_CFM)(OI_OPP_HANDLE handle,
                                         OI_UINT8 numBuffers,
                                         OI_BYTE *data[],
                                         OI_UINT32 len[],
                                         OI_STATUS status,
                                         OI_OPP_CONNECTION oppConnection);


/**
 * This function and corresponding callback is optional for an OPP client or server.
 *
 * @param handle          A handle previously returned by an OI_OPP_OPEN_CFM function
 *
 * @param numReads        The maximum number of mutiple reads that can be made on this call. May be
 *                        zero in which case the readMultipleCfm callback will not be called.
 *
 * @param releaseBuf      Buffer previously passed in readMultipleCfm that can now be released. Will
 *                        be NULL on the first ReadMultiple call.
 *
 * @param releaseBufLen   Length of the buffer that is being released.
 *
 * @param readMultipleCfm The function that will be called as each read completes. This parameter
 *                        will be NULL if OPP is simply releasing buffers and does not require to be
 *                        called back.
 *
 * @param oppConnection   Identifies which OPP client or server connection is performing this
 *                        operation.
 */
typedef OI_STATUS (*OI_OPP_READ_MULTIPLE)(OI_OPP_HANDLE handle,
                                          OI_UINT8 numReads,
                                          OI_BYTE *releaseBuf,
                                          OI_UINT32 releaseBufLen,
                                          OI_OPP_READ_MULTIPLE_CFM readMultipleCfm,
                                          OI_OPP_CONNECTION oppConnection);

/**
 * A function of this type is called to confirm the success or failure of a
 * object write operation.
 *
 * @param objHandle      handle passed to the write call.
 *
 * @param status         Indicates the success or failure of the write operation.
 *
 * @param oppConnection  identifies which OPP client or server connection is
 *                       performing this operation.
 */
typedef void (*OI_OPP_WRITE_CFM)(OI_OPP_HANDLE objHandle,
                                 OI_STATUS status,
                                 OI_OPP_CONNECTION oppConnection);


/**
 * This function and corresponding callback is only required by servers that
 * support object push and clients that support object pull.
 *
 * @param objHandle      a handle previously returned by an OI_OPP_OPEN_CFM function
 *
 * @param buffer         a pointer to a buffer containing the data to be written to
 *                       the object
 *
 * @param bufLen         the number of bytes to write to the object on this call.
 *
 * @param writeCfm       function that will be called when the read completes.
 *
 * @param oppConnection  identifies which OPP client or server connection is
 *                       performing this operation.
 */
typedef OI_STATUS (*OI_OPP_WRITE)(OI_OPP_HANDLE objHandle,
                                  const OI_BYTE *buffer,
                                  OI_UINT32 bufLen,
                                  OI_OPP_WRITE_CFM writeCfm,
                                  OI_OPP_CONNECTION oppConnection);


/* ********************************************************************************
 *
 *               Object operations
 *
 * ********************************************************************************/

typedef struct {

    OI_OPP_OPEN_READ       OpenRead;
    OI_OPP_OPEN_WRITE      OpenWrite;
    OI_OPP_CLOSE           Close;
    OI_OPP_READ            Read;
    OI_OPP_WRITE           Write;
    OI_OPP_READ_MULTIPLE   ReadMultiple;

} OI_OPP_OBJSYS_FUNCTIONS;


#ifdef __cplusplus
}
#endif

/**@}*/

#endif /* _OI_OPP_SYS_H */
