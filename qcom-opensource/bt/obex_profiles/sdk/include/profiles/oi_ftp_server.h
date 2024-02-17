#ifndef _OI_FTP_SERVER_H
#define _OI_FTP_SERVER_H

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
 */

/**
  @file

  This file provides the API for the server side of the File Transfer Profile.

  The File Transfer Profile provides functions for establishing a connection to
  a remote device that support the File Transfer Profile over RFCOMM and
  functions for putting and getting files. This implementation currently only
  allows one client connection at a time.

  The file system support on the local device is defined by oi_ftp_file.c and is
  implementation-dependent. On server platforms with a native file system,
  oi_ftp_file.c will likely be a thin wrapper on the native file system APIs. On
  other platforms, oi_ftp_file.c might implement a simple in-memory object manager.
 */

#include "oi_obex.h"
#include "oi_obexsrv.h"
#include "oi_status.h"
#include "oi_sdp.h"
#include "oi_ftp_sys.h"

/** \addtogroup FTP FTP APIs */
/**@{*/

#ifdef __cplusplus
extern "C" {
#endif


/**
 * Represents an active connection between the FTP server and a remote FTP
 * client
 */
typedef OI_FTP_CONNECTION OI_FTP_SERVER_CONNECTION_HANDLE;


/**
 * Represents a registered FTP server
 */
typedef OI_OBEX_SERVER_HANDLE OI_FTP_SERVER_HANDLE;

/*
 * List of events the FTP Server can notify the Server Application about
 */
typedef enum {
    OI_FTP_SERVER_EVENT_PUT,
    OI_FTP_SERVER_EVENT_GET,
    OI_FTP_SERVER_EVENT_PUT_PROGRESS,
    OI_FTP_SERVER_EVENT_GET_PROGRESS,
    OI_FTP_SERVER_EVENT_PUT_COMPLETE,
    OI_FTP_SERVER_EVENT_GET_COMPLETE
} OI_FTP_SERVER_EVENT_TYPE;

typedef struct {
    OI_FTP_SERVER_EVENT_TYPE    event;
    union {
        struct {
            const OI_OBEX_UNICODE *localName;   /* if known, else NULL */
            OI_UINT32              totalSize;   /* if known, else 0 */
            const OI_CHAR         *objType;     /* if known, else 0 */
        } put;
        struct {
            const OI_OBEX_UNICODE *localName;   /* if known, else NULL */
            OI_UINT32              totalSize;   /* if known, else 0 */
            const OI_CHAR         *objType;     /* if known, else 0 */
        } get;
        struct {
            OI_UINT32              bytesTransferred;
        } putProgress;
        struct {
            OI_UINT32              bytesTransferred;
        } getProgress;
        struct {
            OI_UINT32              finalSize;
            OI_STATUS              status;      /**< Status of the transfer upon completion */

        } putComplete;
        struct {
            OI_UINT32              finalSize;
            OI_STATUS              status;      /**< Status of the transfer upon completion */
        } getComplete;
    } data;
} OI_FTP_SERVER_EVENT_DATA;

/**
 * A callback function of this type is called to indicate a connection request.
 * The server application provides a function with this profile to
 * OI_FTPServer_Register()
 *
 * @param clientAddr     The Bluetooth device address of the client requesting
 *                       the connection.
 *
 * @param unauthorized   Indicates if the connection requires authentication. If
 *                       this parameter is TRUE, the server application must call
 *                       OI_FTPServer_AuthenticationRsp() to provide a password.
 *                       After successful authentication, this function will be
 *                       called again with unauthorized == FALSE indicating that
 *                       the connection has been established. If authentication
 *                       was attempted but failed, this unauthorized parameter
 *                       will still be TRUE.
 *
 * @param userId         The user ID of the user requesting the connection.
 *                       This parameter is only set for authenticated
 *                       connections and may be NULL in any case.
 *                       The application server may use this value to choose a
 *                       root directory or select a previously stored password.
 *
 * @param connectionId   Handle representing the connection to an FTP client.
 *
 * @param userIdLen      Length of the user ID.
 */

typedef void (*OI_FTP_CONNECTION_IND)(OI_BD_ADDR *clientAddr,
                                      OI_BOOL unauthorized,
                                      OI_BYTE *userId,
                                      OI_UINT8 userIdLen,
                                      OI_FTP_SERVER_CONNECTION_HANDLE connectionId);


/**
 * A callback function of this type is called to indicate that a client has
 * disconnected from the FTP server. The server application provides a function
 * with this profile to OI_FTPServer_Register()
 *
 * @param connectionId   Handle representing the connection to an FTP client
 */

typedef void (*OI_FTP_DISCONNECTION_IND)(OI_FTP_SERVER_CONNECTION_HANDLE connectionId);

/**
 * A callback function of this type is called to indicate a variety of events
 * have occured that the Server Application may be interested in. This pointer
 * may be NULL in which case no event delivery will be attempted.
 *
 * @param connectionId   Handle representing the connection to an FTP client
 *
 * @param event          Tagged Structure including the event, and whatever data
 *                       that needs to be delivered with it.
 */
typedef void (*OI_FTP_SERVER_EVENT_IND)(OI_FTP_SERVER_CONNECTION_HANDLE connectionId,
                                        const OI_FTP_SERVER_EVENT_DATA *event);

typedef struct {
    OI_FTP_CONNECTION_IND    connectInd;
    OI_FTP_DISCONNECTION_IND disconnectInd;
    OI_FTP_SERVER_EVENT_IND  eventInd;
} OI_FTP_SERVER_CALLBACKS;

/**
 * This function is called by the application in response to a connnection
 * indication that requires authentication.
 *
 * @param connectionId   Handle representing the connection to an FTP client
 *
 * @param password       NULL-terminated password
 *
 * @param readOnly       Indicates if the client is only being allowed read
 *                       access; a client that is only granted read access cannot
 *                       create, put, or delete files and folders
 *
 * @param allowBrowse    Indicates if the client is allowed to browse the file
 *                       system; a client that is not permitted to browse can
 *                       still get and put files from the root folder
 *
 */

OI_STATUS OI_FTPServer_AuthenticationRsp(OI_FTP_SERVER_CONNECTION_HANDLE connectionId,
                                         const OI_CHAR *password,
                                         OI_BOOL readOnly,
                                         OI_BOOL allowBrowse);


 /**
 * An application calls this function to accept or reject an indicated
 * connection.
 *
 * @param connectionId   Handle representing the connection to an FTP client
 *
 * @param accept         TRUE if the connection is being accepted, FALSE
 *                       otherwise; if FALSE, the values of the remaining
 *                       parameters are ignored
 *
 * @param rootFolder     Implementation-dependent root folder for this
 *                       connection
 *
 */

OI_STATUS OI_FTPServer_AcceptConnection(OI_FTP_SERVER_CONNECTION_HANDLE connectionId,
                                        OI_BOOL accept,
                                        const OI_OBEX_UNICODE *rootFolder);


/**
 * This function forcibly severs the connection from an FTP client to the FTP server. This
 * function should be called for all active connections before terminating the
 * FTP server.
 *
 * @param connectionId  Handle representing the connection between a remote
 *                      client and the local FTP server
 *
 * @return              OI_OK if the connection will be terminated
 */

OI_STATUS OI_FTPServer_ForceDisconnect(OI_FTP_SERVER_CONNECTION_HANDLE connectionId);


/**
 * This function initialzes the FTP server to accept connections and registers
 * it with the SDP database so that the service becomes discoverable.
 *
 * @param authentication Indicates whether connections to this
 *                       server must be authenticated using OBEX authentication;
 *                       if this parameter is FALSE, clients connecting to the
 *                       server may demand authentication
 *
 * @param readOnly       Default access for unauthenticated clients; applies to
 *                       all clients in authenticated == FALSE
 *
 * @param allowBrowse    Default browse permission for unauthenticated clients;
 *                       applies to all clients if authentication == FALSE
 *
 * @param callbacks      Callback function pointers for Connect, Disconnect and
 *                       Event Indication to server application
 *
 * @param fileOperations Interface to file system operations
 *
 * @param strings        Strings to register in the service record
 *
 * @param serverInstance [OUT] returns a handle for the FTP server instance. This handle
 *                       is required to deregsiter the FTP server, see OI_FTPServer_Deregister().
 *
 * @return               OI_OK if the service was successfully registered
 */

OI_STATUS OI_FTPServer_Register(OI_OBEXSRV_AUTHENTICATION       authentication,
                                OI_BOOL                         readOnly,
                                OI_BOOL                         allowBrowse,
                                const OI_FTP_SERVER_CALLBACKS  *callbacks,
                                const OI_FTP_FILESYS_FUNCTIONS *fileOperations,
                                const OI_SDP_STRINGS           *strings,
                                OI_FTP_SERVER_HANDLE           *serverInstance);


/**
 * This function gets the service record handle associated with this service.
 * For example, this can be used with OI_SDPDB_SetAttributeList to add
 * vendor-specific SDP attributes to the profile.
 *
 * @param serverInstance   Identifies the registered server instance.
 *
 * @param handle           Returns the service record's handle
 */
OI_STATUS OI_FTPServer_GetServiceRecord(OI_FTP_SERVER_HANDLE serverInstance,
                                        OI_UINT32 *handle);

/**
 * This function deregisters an FTP server.
 *
 * @param serverInstance  Specifies he server instance handle returned when the server was
 *                        registered.
 *
 * @return                OI_OK if the service was successfully deregistered
 *
 */
OI_STATUS OI_FTPServer_Deregister(OI_FTP_SERVER_HANDLE serverInstance);

/**
 * Accept/Reject incoming (PUT) object
 *
 * @param connectionId    The server connection handle that was returned when the server
 *                        connection was opened.
 *
 * @param filename        pointer to obex filename to save incoming file to,
 *                        or NULL to accept default incoming filename.
 *
 * @param accept          TRUE - Accept incoming file, FALSE - Reject
 *
 * @return                OI_OK if the rcvObject was successfuly opened for
 *                        writing (or if rejecting).
 *
 */
OI_STATUS OI_FTPServer_AcceptPut(OI_FTP_SERVER_CONNECTION_HANDLE connectionId,
                                 OI_OBEX_UNICODE                *filename,
                                 OI_BOOL                         accept);

/**
 * Request current directory level of the specified Server connection.
 * This function is intended for the use of the File System Abstration.
 *
 * @param connectionId    The server connection handle that of the server-client
 *                        connection being registered.
 *
 * @param folderLevel     [OUT] Location to return current folder level (0 == root)
 *
 * @return                OI_OK if connectionId is valid, and parameters are good
 *                        OI_STATUS_INVALID_PARAMETERS otherwise
 */
OI_STATUS OI_FTPServer_GetFolderLevel(OI_FTP_SERVER_CONNECTION_HANDLE connectionId,
                                      OI_UINT8                       *folderLevel);

/**
 * Request Root directory path of the specified Server connection.
 * This function is intended for the use of the File System Abstration.
 *
 * @param connectionId    The server connection handle that was returned when the server
 *                        connection was opened.
 *
 * @param rootFolder      [OUT] Location to return the pointer to the root Folder.
 *
 * @return                OI_OK if connectionId is valid, and parameters are good
 *                        OI_STATUS_INVALID_PARAMETERS otherwise
 *
 */
OI_STATUS OI_FTPServer_GetRootFolder(OI_FTP_SERVER_CONNECTION_HANDLE connectionId,
                                     OI_OBEX_UNICODE               **rootFolder);


#ifdef __cplusplus
}
#endif

/**@}*/

#endif /* _OI_FTP_SERVER_H */
