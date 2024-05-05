#ifndef _OI_PBAP_SYS_H
#define _OI_PBAP_SYS_H

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
 * This file provides the file system abstraction layer used by
 * the Phonebook Access Profile client and server.
 *
 * This interface is described entirely as callback functions. The application
 * must provide the appropriate sets of functions to the PBAP client and PBAP
 * server when initializing these two services. A Bluetooth compliant
 * implementation of a PBAP server must return certain specific error codes
 * to report various operation failure modes.
 *
 * Callbacks and errors are mutually exclusive.  If you invoke a callback from
 * one of the functions below, that function must return OI_OK.  If you do not
 * invoke a callback, the function must return an error message.  Note that if
 * you wish your application to be BQB-compliant, you should generate error
 * codes consistent with BLUEmagic 3.0 best practices; see the sample code for
 * examples.
 */

#include "oi_status.h"
#include "oi_stddefs.h"
#include "oi_obex.h"
#include "oi_obexspec.h"
#include "oi_pbap_consts.h"

/** \addtogroup PBAP PBAP APIs */
/**@{*/

#ifdef __cplusplus
extern "C" {
#endif


typedef OI_OBEX_CONNECTION_HANDLE OI_PBAP_CONNECTION;

typedef void* OI_PBAP_HANDLE;


/*********************************************************************************
 * The file and folder functions are all asynchronous. However implementations
 * are permitted to call the callback function from within the function if the
 * file operation can be completed quickly. This will be the case for many
 * implementations.
 *********************************************************************************/




/**
 * A function of this type is called to confirm the success or failure of a file
 * open.
 *
 * @param handle          A handle that can be used to read the file contents.
 *                        The handle must not be NULL.
 *
 * @param status          OI_OK if the folder could be opened with the required
 *                        mode or an error if the file could not be opened.
 *
 * @param pbapConnection  identifies which PBAP client or server connection is
 *                        performing this operation.
 */
typedef void (*OI_PBAP_OPEN_CFM)(OI_PBAP_HANDLE handle,
                                 OI_STATUS status,
                                 OI_PBAP_CONNECTION pbapConnection);

/**
 * This function and corresponding callback is needed by a PBAP client and
 * server.  Whether the file is opened for read access or write access is
 * determined by if it is called from server code or client code.
 *
 * @param name            a unicode string name of the file to be opened.
 *
 * @param openCfm         the function that will be called when the open
 *                        completes.
 *
 * @param pbapConnection  identifies which PBAP client or server connection is
 *                        performing this operation.
 */
typedef OI_STATUS (*OI_PBAP_OPEN)(const OI_OBEX_UNICODE *name,
                                  OI_PBAP_OPEN_CFM openCfm,
                                  OI_PBAP_CONNECTION pbapConnection);

/**
 * A callback function of this type is called to indicate a server has
 * disconnected from the PBAP client. The client application provides a function
 * with this profile to OI_PBAPClient_Register()
 *
 * @param connectionId   handle representing the connection to an PBAP client.
 */
typedef void (*OI_PBAP_DISCONNECTION_IND)(OI_PBAP_CONNECTION connectionId);


/**
 * This function does not use a callback function. It is assumed that file
 * close will complete asynchronously or in the case of a failure will report
 * or log an error with the application.
 *
 * @param handle          a handle for an open file.
 *
 * @param pbapConnection  identifies which PBAP client or server connection is
 *                        performing this operation.
 *
 * @param status          OI_OK if the file operation completed succesfully. An
 *                        error status if the file operation terminated before it
 *                        was complete.
 */
typedef void (*OI_PBAP_CLOSE)(OI_PBAP_HANDLE handle,
                              OI_PBAP_CONNECTION pbapConnection,
                              OI_STATUS status);

/**
 * A function of this type is called to confirm the success or failure of a file
 * read.
 *
 * @param data            a pointer to a buffer containing the data read.
 *
 * @param len             the number of bytes read.
 *
 * @param status          a status code indicating if the read was succesful:
 *                        - OI_OK if the read completed
 *                        - OI_STATUS_END_OF_FILE if the read was successful and the
 *                          end of file has been reached.
 *                        - An error status indicating that the read failed.
 *
 * @param pbapConnection  identifies which PBAP client or server connection is
 *                        performing this operation.
 *
 */
typedef void (*OI_PBAP_READ_CFM)(OI_PBAP_HANDLE handle,
                                 OI_BYTE *data,
                                 OI_UINT16 len,
                                 OI_STATUS status,
                                 OI_PBAP_CONNECTION pbapConnection);

/**
 * This function and corresponding callback is needed by a PBAP server only.
 *
 * @param handle          a handle previously returned by an OI_PBAP_OPEN_CFM
 *                        function
 *
 * @param maxRead         the maximum number of bytes to read from the file on
 *                        this call.
 *
 * @param readCfm         the function that will be called when the read
 *                        completes.
 *
 * @param pbapConnection  identifies which PBAP client or server connection is
 *                        performing this operation.
 */
typedef OI_STATUS (*OI_PBAP_READ)(OI_PBAP_HANDLE handle,
                                  OI_UINT16 maxRead,
                                  OI_PBAP_READ_CFM readCfm,
                                  OI_PBAP_CONNECTION pbapConnection);

/**
 * A function of this type is called to confirm the success or failure of a
 * file write operation.
 *
 * @param handle          a handle previously returned by an OI_PBAP_OPEN_CFM
 *                        function
 *
 * @param status          indicates the success or failure of the write
 *                        operation.
 *
 * @param pbapConnection  identifies which PBAP client or server connection is
 *                        performing this operation.
 */
typedef void (*OI_PBAP_WRITE_CFM)(OI_PBAP_HANDLE handle,
                                  OI_STATUS status,
                                  OI_PBAP_CONNECTION pbapConnection);

/**
 * This function and corresponding callback is needed by a PBAP client.
 *
 * @param handle          a handle previously returned by an OI_PBAP_OPEN_CFM
 *                        function
 *
 * @param buffer          a pointer to a buffer containing the data to be written
 *                        to the file
 *
 * @param bufLen          the number of bytes to write to the file on this call.
 *
 * @param writeCfm        the function that will be called when the read
 *                        completes.
 *
 * @param pbapConnection  identifies which PBAP client or server connection is
 *                        performing this operation.
 */
typedef OI_STATUS (*OI_PBAP_WRITE)(OI_PBAP_HANDLE handle,
                                   const OI_BYTE *buffer,
                                   OI_UINT16 bufLen,
                                   OI_PBAP_WRITE_CFM writeCfm,
                                   OI_PBAP_CONNECTION pbapConnection);

/**
 * This function opens a handle for the PBAP server code to read the listing
 * of a phonebook.
 *
 * @param repository       The local or SIM1 repository to get the listing from.
 *
 * @param phonebook        The phonebook in the repository to get the listing from.
 *
 * @param order            The sorting order of the phonebook entries.
 *
 * @param searchAttribute  The type of criteria for finding specific phonebook entries.
 *
 * @param searchValue      Criteria for find specific entries (NULL for all entries).
 *
 * @param searchValueLen   Number of bytes in the search value.
 *
 * @param maxListCount     Maximum number of entries to return.
 *
 * @param listStartOffset  Index of first entry to include in listing.
 *
 * @param browsePbCfm      Callback indicating success or failure of this request.
 *
 * @param pbapConnection   Identifies which PBAP client or server connection is
 *                         performing this operation.
 */
typedef OI_STATUS (*OI_PBAP_BROWSE_PB)(OI_PBAP_REPOSITORY repository,
                                       OI_PBAP_PHONEBOOK phonebook,
                                       OI_PBAP_ORDER_TAG_VALUES order,
                                       OI_PBAP_SEARCH_ATTRIBUTE_TAG_VALUES searchAttribute,
                                       OI_BYTE *searchValue,
                                       OI_UINT8 searchValueLen,
                                       OI_UINT16 maxListCount,
                                       OI_UINT16 listStartOffset,
                                       OI_PBAP_OPEN_CFM browsePbCfm,
                                       OI_PBAP_CONNECTION pbapConnection);


/**
 * This function opens a handle for reading an entire phonebook.
 *
 * @param repository       The local or SIM1 repository to get the phonebook
 *                         contents from.
 *
 * @param phonebook        The phonebook in the repository to get the contents of.
 *
 * @param filter           A bit mask indicating the data fields to include in the
 *                         vCard data.
 *
 * @param format           The format of the vCard data: v2.1 or v3.0.  The server
 *                         must support both.
 *
 * @param maxListCount     Maximum number of entries to return.
 *
 * @param listStartOffset  Index of first entry to include in listing.
 *
 * @param openPbCfm        Callback indicating success or failure of this request.
 *
 * @param pbapConnection   Identifies which PBAP client or server connection is
 *                         performing this operation.
 */
typedef OI_STATUS (*OI_PBAP_OPEN_PB)(OI_PBAP_REPOSITORY repository,
                                     OI_PBAP_PHONEBOOK phonebook,
                                     const OI_UINT64 *filter,
                                     OI_PBAP_FORMAT_TAG_VALUES format,
                                     OI_UINT16 maxListCount,
                                     OI_UINT16 listStartOffset,
                                     OI_PBAP_OPEN_CFM openPbCfm,
                                     OI_PBAP_CONNECTION pbapConnection);

/**
 * This function opens a handle for reading an entire phonebook.
 *
 * @param repository       The local or SIM1 repository to get the vCard from.
 *
 * @param phonebook        The phonebook in the repository to get the vCard from.
 *
 * @param entry            Index of the desired entry in the phonebook.
 *
 * @param filter           A bit mask indicating the data fields to include in the
 *                         vCard data.
 *
 * @param format           The format of the vCard data: v2.1 or v3.0.  The server
 *                         must support both.
 *
 * @param openvCardCfm     Callback indicating success or failure of this request.
 *
 * @param pbapConnection   Identifies which PBAP client or server connection is
 *                         performing this operation.
 */
typedef OI_STATUS (*OI_PBAP_OPEN_VCARD)(OI_PBAP_REPOSITORY repository,
                                        OI_PBAP_PHONEBOOK phonebook,
                                        OI_UINT32 entry,
                                        const OI_UINT64 *filter,
                                        OI_PBAP_FORMAT_TAG_VALUES format,
                                        OI_PBAP_OPEN_CFM openvCardCfm,
                                        OI_PBAP_CONNECTION pbapConnection);


/*********************************************************************************
 *
 *                             File operations
 *
 *********************************************************************************/

/**
   This structure defines client filesystem functions.
 */
typedef struct {
    OI_PBAP_OPEN            open;
    OI_PBAP_CLOSE           close;
    OI_PBAP_WRITE           write;
} OI_PBAP_CLIENT_FILESYS_FUNCTIONS;

/**
   This structure defines server filesystem functions.
 */
typedef struct {
    OI_PBAP_BROWSE_PB       listPB;
    OI_PBAP_OPEN_PB         openPB;
    OI_PBAP_OPEN_VCARD      openvCard;
    OI_PBAP_CLOSE           close;
    OI_PBAP_READ            read;
} OI_PBAP_SERVER_FILESYS_FUNCTIONS;


#ifdef __cplusplus
}
#endif

/**@}*/

#endif /* _OI_PBAP_SYS_H */
