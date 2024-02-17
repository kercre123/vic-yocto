#ifndef _OI_FTP_CLIENT_H
#define _OI_FTP_CLIENT_H

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

  File Transfer Profile client API

  The File Transfer Profile provides functions for establishing a connection to
  a remote device that support the File Transfer Profile over RFCOMM and
  functions for putting and getting files. This implementation currently only
  allows one connection at a time. A Bluetooth device address and an RFCOMM channel
  number are required for setting up the connection. Unless the application
  already knows the RFCOMM channel number, the application will need to perform
  service discovery to obtain the channel number.

  Once a connection has been established, the application can call
  OI_FTPClient_Put() to write files to the server and OI_FTPClient_Get() to read files
  from the server. If folder navigation is supported by the remote server, the
  application can call OI_FTPClient_SetPath() to navigate the folder hierarchy on
  the remote server. FTPClient_GetFolderList() queries the remote server for
  information about the files and folder in the current folder.

  The file system support on the local device is defined by oi_ftp_file.c and is
  implementation-dependent. On a client platform with a native file system,
  oi_ftp_file.c will likely be a thin wrapper on the native file system APIs. On
  other platforms, oi_ftp_file.c might implement a simple in-memory object manager.

 */

#include "oi_obexspec.h"
#include "oi_obex.h"
#include "oi_obexcli.h"
#include "oi_ftp_sys.h"

/** \addtogroup FTP FTP APIs */
/**@{*/

#ifdef __cplusplus
extern "C" {
#endif



/**
 * This enumeration is of completion and notification events returned to the application via the
 * @ref OI_FTP_CLIENT_EVENT_CB callback function. The event indicates completion, the
 * accompanying status code indictates if the operation succeeded or failed. The
 * completion of a get folder listing operation, OI_FTPClient_GetFolderListing(),
 * is not signalled by and event, rather a @ref OI_FTP_CLIENT_FOLDER_LIST_CB
 * function is called with the retrieved folder list.
 */
typedef enum {
    OI_FTP_CLIENT_CONNECTED,               /**< An event of this type indicates that a connection request has completed. */
    OI_FTP_CLIENT_CONNECTED_READ_ONLY,     /**< An event of this type indicates that a connection request has completed. */
    OI_FTP_CLIENT_DISCONNECT,              /**< An event of this type indicates that a connection has been terminated. */
    OI_FTP_CLIENT_GET_COMPLETE,            /**< A Put operation has completed. */
    OI_FTP_CLIENT_PUT_COMPLETE,            /**< A Get operation has completed. */
    OI_FTP_CLIENT_DELETE_COMPLETE,         /**< A file or folder delete operation has completed. */
    OI_FTP_CLIENT_SETPATH_COMPLETE,        /**< A Set path operation has completed. */
    OI_FTP_CLIENT_NEW_SUBFOLDER_COMPLETE,  /**< New sub-folder created on the remote server */
    OI_FTP_CLIENT_TREE_PUT_COMPLETE,       /**< A Put of a complete folder tree has completed. */
    OI_FTP_CLIENT_TREE_GET_COMPLETE,       /**< A Get of a complete folder tree has completed. */
    OI_FTP_CLIENT_RENAME_COMPLETE,         /**< A remote file or foler rename has completed */
    OI_FTP_CLIENT_COPY_COMPLETE,           /**< A remote file or folder copy has completed */
    OI_FTP_CLIENT_SET_PERMISSIONS_COMPLETE,/**< Permissions have been set on a remote file or folder */
    OI_FTP_CLIENT_PUT_STARTED,             /**< A New PUT file transfer has started */
    OI_FTP_CLIENT_GET_STARTED,             /**< A New GET file transfer has started */
    OI_FTP_CLIENT_PUT_PROGRESS,            /**< Periodic event delivered during FTP file transfer */
    OI_FTP_CLIENT_GET_PROGRESS,            /**< Periodic event delivered during FTP file transfer */
} OI_FTP_CLIENT_EVENT;

/**
 * Structure containing event code, and any related data
 */
typedef struct {
    OI_FTP_CLIENT_EVENT     event;
    union {
        struct {
            OI_OBEX_UNICODE *fileName;    /**< File name being pushed */
            OI_UINT32        totalBytes;  /**< Total bytes in file being sent (if known) */
        } putStarted;                     /**< Union structure valid for @ref OI_FTP_CLIENT_PUT_STARTED */
        struct {
            OI_OBEX_UNICODE *fileName;    /**< File name being pushed */
            OI_UINT32        totalBytes;  /**< Total bytes in file being received (if known) */
        } getStarted;                     /**< Union structure valid for @ref OI_FTP_CLIENT_GET_STARTED */
        struct {
            OI_UINT32        bytesTransferred; /**< Total bytes sent so far for this file */
        } putProgress;                    /**< Union structure valid for @ref OI_FTP_CLIENT_PUT_PROGRESS */
        struct {
            OI_UINT32        bytesTransferred; /**< Total bytes received so far for this file */
        } getProgress;                    /**< Union structure valid for @ref OI_FTP_CLIENT_GET_PROGRESS */
        struct {
            OI_UINT32        finalSize;   /**< Final bytes sent for this file */
        } putComplete;                    /**< Union structure valid for @ref OI_FTP_CLIENT_PUT_COMPLETE */
        struct {
            OI_UINT32        finalSize;   /**< Final bytes received for this file */
        } getComplete;                    /**< Union structure valid for @ref OI_FTP_CLIENT_GET_COMPLETE */
    } data;
} OI_FTP_CLIENT_EVENT_DATA;


/**
 * This value controls whether a set path request is setting the folder path to a specific
 * folder, to the parent of the current folder, or to the root folder as defined
 * by the remote server.
 */
typedef enum {
    OI_FTP_CLIENT_SETPATH_TO_FOLDER,
    OI_FTP_CLIENT_SETPATH_TO_PARENT,
    OI_FTP_CLIENT_SETPATH_TO_ROOT
} OI_FTP_CLIENT_SETPATH_CONTROL;


/**
 *  Represents an active connection between an FTP client and a remote FTP server
 */

typedef OI_FTP_CONNECTION OI_FTP_CLIENT_CONNECTION_HANDLE;


/**
 * The application must provide a function with this profile to OI_FTPClient_Connect()
 * when initializing the file transfer client. This function is called to signal
 * to the application when various operations completed, see @ref
 * OI_FTP_EVENT_DATA.
 *
 * @param connectionId handle representing the connection to the FTP server.
 *
 * @param event   identifies an event
 *
 * @param status  OI_OK if the operation succeeded, or an error code if the operation failed
 */
typedef void (*OI_FTP_CLIENT_EVENT_CB)(OI_FTP_CLIENT_CONNECTION_HANDLE connectionId,
                                       const OI_FTP_CLIENT_EVENT_DATA *event,
                                       OI_STATUS                       status);


/**
 * A callback function of this type asks the client to provide a password to authenticate the connection.
 * The client must provide a function with this profile to OI_FTPClient_Connect() if the client
 * supports OBEX authentication.
 *
 * @param connectionId handle representing the connection to the FTP server.
 *
 * @param userIdRequired  indicates that the server requires the client to
 *                        provide a user id in the authentication call.
 *
 * @param realm           The realm for the authentication or NULL if there is no realm specified
 */

typedef void (*OI_FTP_CLIENT_AUTHENTICATION_CB)(OI_FTP_CLIENT_CONNECTION_HANDLE connectionId,
                                                OI_BOOL userIdRequired,
                                                OI_OBEX_REALM *realm);


/**
 * The applicatiom must provide a function with this profile to
 * OI_FTPClient_GetFolderListing() so that FTP client can return the folder listing
 * results.
 *
 * @param connectionId handle representing the connection to the FTP server.
 *
 * @param folderData This points to a chunk of folder data. Folder data is sent
 *                   by the server as XML text in one or more packets.  This
 *                   parameter is NULL if this is the end of the folder data.
 *
 * @param status    Indicates if there was an error receiving the folder data.
 */
typedef void (*OI_FTP_CLIENT_FOLDER_LIST_CB)(OI_FTP_CLIENT_CONNECTION_HANDLE connectionId,
                                             OI_OBEX_BYTESEQ *folderData,
                                             OI_STATUS status);

/**
 * This function establishes an OBEX connection over RFCOMM to a
 * remote FTP server.  The client must provide callback functions of
 * types OI_FTP_CLIENT_EVENT_CB and OI_FTP_CLIENT_AUTHENTICATION_CB to
 * this function if the client supports OBEX authentication.
 *
 * @param addr              the address of a remote Bluetooth device that supports FTP
 *
 * @param lowerProtocol     This identifies the RFCOMM channel number or the L2CAP PSM for the
 *                          FTP server running on the remote device.  the remote device. The caller
 *                          will normally perform service discovery on the remote device to obtain
 *                          the required channel number or PSM.
 *
 * @param connectionId      [OUT] returns a handle representing the connection to the FTP server
 *
 * @param authentication    Specifies whether authentication is required when
 *                          connecting to a server.  Use OI_OBEXCLI_AUTH_NONE
 *                          when no authentication is required.
 *
 * @param eventCB           callback function with which FTP client events will be returned
 *
 * @param authenticationCB  authentication callback function
 *
 * @param fileOperations    pointer to a structure of platform-specific file operations
 *
 * @return                  OI_OK if the connection request can be issued
 */
OI_STATUS OI_FTPClient_Connect(OI_BD_ADDR *addr,
                               OI_OBEX_LOWER_PROTOCOL *lowerProtocol,
                               OI_FTP_CLIENT_CONNECTION_HANDLE *connectionId,
                               OI_OBEXCLI_AUTHENTICATION authentication,
                               OI_FTP_CLIENT_EVENT_CB eventCB,
                               OI_FTP_CLIENT_AUTHENTICATION_CB authenticationCB,
                               const OI_FTP_FILESYS_FUNCTIONS *fileOperations);

/**
 * This function terminate the current OBEX connection to a remote FTP server.
 *
 * @param connectionId handle representing the connection to the FTP server.
 *
 */
OI_STATUS OI_FTPClient_Disconnect(OI_FTP_CLIENT_CONNECTION_HANDLE connectionId);

/**
 * This function is called by the application in response to a password
 * indication to provide a password for OBEX authentication.
 *
 * @param connectionId handle representing the connection to the FTP server.
 *
 * @param userId       the user id for the connecting user. This parameter is
 *                     required if the server indicated that user id is
 *                     required. Otherwise is can be NULL. The client can
 *                     provide a user id even if the server does not require
 *                     one.
 *
 * @param userIdLen    Length of the userId.
 *
 * @param password     is a NULL terminated password.
 */
OI_STATUS OI_FTPClient_AuthenticationRsp(OI_FTP_CLIENT_CONNECTION_HANDLE connectionId,
                                         const OI_BYTE *userId,
                                         OI_UINT8 userIdLen,
                                         const OI_CHAR *password);


/**
 * This function copies a file from the local client to a remote server.
 *
 * @param connectionId  handle representing the connection to the FTP server.
 *
 * @param localName     the identifying name of the file on the local client that is
 *                      being sent to the server
 *
 * @param remoteName    the name of the file that will be created or written on the
 *                      remote server
 *
 * @return
 *                      - OI_OK if the put operation was started
 *                      - OI_OBEX_NOT_CONNECTED if the client is not connected to an OBEX server
 *                      - OI_OBEX_OPERATION_IN_PROGRESS if another put or get operation is in progress
 *                      - or other errors
 */
OI_STATUS OI_FTPClient_Put(OI_FTP_CLIENT_CONNECTION_HANDLE connectionId,
                           OI_OBEX_UNICODE *localName,
                           OI_OBEX_UNICODE *remoteName);

/**
 * This function deletes a file or folder on a remote server. Only empty folders can be deleted.
 *
 * @param connectionId  handle representing the connection to the FTP server.
 *
 * @param name          the identifying name of a file or foldder on the remote server.
 *
 * @return
 *                - OI_OK if the delete operation was sent
 *                - OI_OBEX_NOT_CONNECTED if the client is not connected to an OBEX server
 *                - OI_OBEX_OPERATION_IN_PROGRESS if another get operation is in progress
 *                - or other errors
 */
OI_STATUS OI_FTPClient_Delete(OI_FTP_CLIENT_CONNECTION_HANDLE connectionId,
                              OI_OBEX_UNICODE *name);


/**
 * This function copies a file from a remote server to the local client.
 *
 * @param connectionId  handle representing the connection to the FTP server.
 *
 * @param localName     the name of the file that will be created or written on the local client
 *
 * @param remoteName    the identifying name of the file that is being requested from the remote server
 *
 * @return
 *                    - OI_OK if the get operation was started
 *                    - OI_OBEX_NOT_CONNECTED if the client is not connected to an OBEX server
 *                    - OI_OBEX_OPERATION_IN_PROGRESS if another put or get operation is in progress
 *                    - or other errors
 */
OI_STATUS OI_FTPClient_Get(OI_FTP_CLIENT_CONNECTION_HANDLE connectionId,
                           OI_OBEX_UNICODE *localName,
                           OI_OBEX_UNICODE *remoteName);

/**
 * This callback is invoked when the cancellation of an operation has completed.
 *
 * @param connectionId  handle representing the connection to the FTP server.
 */
typedef void (*OI_FTP_CLIENT_CANCEL_CFM)(OI_FTP_CLIENT_CONNECTION_HANDLE connectionId);

/**
 * This function terminates the current put or get operation.
 *
 * @param connectionId  handle representing the connection to the FTP server.
 *
 * @param cancelCfm     Callback to indicate completion of the cancellation (may be NULL).
 *
 * @return an error status if there is no operation to abort.
 */
OI_STATUS OI_FTPClient_Cancel(OI_FTP_CLIENT_CONNECTION_HANDLE connectionId,
                              OI_FTP_CLIENT_CANCEL_CFM cancelCfm);


/**
 * This function sets the current folder on the remote server for subsequent put and get
 * operations.
 *
 * @param connectionId  handle representing the connection to the FTP server.
 *
 * @param folder        the name of a child folder of the current folder if the second
 *                      parameter is OI_FTP_CLIENT_SETPATH_TO_FOLDER; NULL otherwise
 *
 * @param setPathCtrl   determines the behavior of the setpath command, see @ref
 *                      OI_FTP_CLIENT_SETPATH_CONTROL for more information.
 *
 * @return
 *                    - OI_OK if the set path request was sent
 *                    - OI_OBEX_NOT_CONNECTED if the client is not connected to an OBEX server
 *                    - OI_OBEX_OPERATION_IN_PROGRESS if another operation is in progress
 *                    - or other errors
 */
OI_STATUS OI_FTPClient_SetPath(OI_FTP_CLIENT_CONNECTION_HANDLE connectionId,
                               OI_OBEX_UNICODE *folder,
                               OI_FTP_CLIENT_SETPATH_CONTROL setPathCtrl);

/**
 * This function creates a subfolder beneath the current folder on the remote
 * server if the server supports this function.
 *
 * @param connectionId  handle representing the connection to the FTP server.
 *
 * @param folder        the name of the new sub-folder
 *
 * @return
 *                    - OI_OK if the set path request was sent
 *                    - OI_OBEX_NOT_CONNECTED if the client is not connected to an OBEX server
 *                    - OI_OBEX_OPERATION_IN_PROGRESS if another operation is in progress
 *                    - or other errors
 */
OI_STATUS OI_FTPClient_NewSubfolder(OI_FTP_CLIENT_CONNECTION_HANDLE connectionId,
                                    OI_OBEX_UNICODE *folder);


/**
 * This function gets information about files and folders in the current folder or in a
 * specific subfolder of the current folder.
 *
 * @param connectionId  handle representing the connection to the FTP server.
 *
 * @param folder        a subfolder of the current folder, or NULL
 *
 * @param folderListCB  The callback function specified by this parameter is the function
 *                      called with the folder listing results. This function may be called
 *                      mutiple times if the results are sent as multiple packets.
 *
 * @return
 *                        - OI_OK if the folder listing request was sent
 *                        - OI_OBEX_NOT_CONNECTED if the client is not connected to an OBEX server
 *                        - OI_OBEX_OPERATION_IN_PROGRESS if another operation is in progress
 *                        - or other errors
 */
OI_STATUS OI_FTPClient_GetFolderListing(OI_FTP_CLIENT_CONNECTION_HANDLE connectionId,
                                        OI_OBEX_UNICODE *folder,
                                        OI_FTP_CLIENT_FOLDER_LIST_CB folderListCB);


/**
 * This function recursively copies a folder and all its subfolders and files
 * from the local client to a remote server.
 *
 * @param connectionId  handle representing the connection to the FTP server.
 *
 * @param localName     the identifying name of the folder on the local client that is
 *                      being sent to the server. This can be a path name that
 *                      is interpreted by the local file system.
 *
 * @param remoteName    the name of the folder that will be created or written on the
 *                      remote server
 *
 * @return
 *                      - OI_OK if the put operation was started
 *                      - OI_OBEX_NOT_CONNECTED if the client is not connected to an OBEX server
 *                      - OI_OBEX_OPERATION_IN_PROGRESS if another put or get operation is in progress
 *                      - or other errors
 *
 * @note   This function operates by recursively browsing folders on the server
 *         and client. In the event of an error the application must assume that
 *         the current local and remote folders are no longer known. It is
 *         therefore the repsonsibility of the  application to set the remote
 *         folder to the root folder and browese back to the desired folder. The
 *         local folder should be similarly reset to a starting point.
 */
OI_STATUS OI_FTPClient_TreePut(OI_FTP_CLIENT_CONNECTION_HANDLE connectionId,
                               OI_OBEX_UNICODE *localName,
                               OI_OBEX_UNICODE *remoteName);


/**
 * This function recursively copies a folder and all its subfolders and files
 * from a remote server to the local client.
 *
 * @param connectionId  handle representing the connection to the FTP server.
 *
 * @param localName     the name of the folder that will be created or written
 *                      on the local client
 *
 * @param remoteName    the identifying name of the folder that is being
 *                      requested from the remote server
 *
 * @return
 *                    - OI_OK if the get operation was started
 *                    - OI_OBEX_NOT_CONNECTED if the client is not connected to an OBEX server
 *                    - OI_OBEX_OPERATION_IN_PROGRESS if another put or get operation is in progress
 *                    - or other errors
 *
 * @note   This function operates by recursively browsing folders on the server
 *         and client. In the event of an error the application must assume that
 *         the current local and remote folders are no longer known. It is
 *         therefore the repsonsibility of the  application to set the remote
 *         folder to the root folder and browese back to the desired folder. The
 *         local folder should be similarly reset to a starting point.
 */
OI_STATUS OI_FTPClient_TreeGet(OI_FTP_CLIENT_CONNECTION_HANDLE connectionId,
                               OI_OBEX_UNICODE *localName,
                               OI_OBEX_UNICODE *remoteName);


/**
 * This function renames a file or folder on the remote device. The new name may specify an absolute
 * or relative file name in which case the file or folder will be moved to the new location.
 *
 * @param connectionId  Handle representing the connection to the FTP server.
 *
 * @param currentName   The name of thee remote file or folder to be renamed or moved.
 *
 * @param newName       The new name for the remote file or folder which may include a relative or
 *                      absolute path prefix if the file or folder is to be moved to another
 *                      location within the server folder tree.
 *
 * @return
 *                    - OI_OK if the rename operation was started
 *                    - OI_OBEX_NOT_CONNECTED if the client is not connected to an OBEX server
 *                    - OI_OBEX_OPERATION_IN_PROGRESS if another operation is in progress
 *                    - or other errors
 *
 */
OI_STATUS OI_FTPClient_Rename(OI_FTP_CLIENT_CONNECTION_HANDLE connectionId,
                              OI_OBEX_UNICODE *currentName,
                              OI_OBEX_UNICODE *newName);


/**
 * This function copies a file or folder on the remote device to another file or location on the
 * remote device. In the case of a folder the contents of the folder are copied recursively.
 *
 * @param connectionId  Handle representing the connection to the FTP server.
 *
 * @param srcName       The name of the remote file or folder to be copied.
 *
 * @param destName      The destination name for the copied file or folder.
 *
 * @return
 *                    - OI_OK if the copy operation was started
 *                    - OI_OBEX_NOT_CONNECTED if the client is not connected to an OBEX server
 *                    - OI_OBEX_OPERATION_IN_PROGRESS if another operation is in progress
 *                    - or other errors
 *
 */
OI_STATUS OI_FTPClient_Copy(OI_FTP_CLIENT_CONNECTION_HANDLE connectionId,
                            OI_OBEX_UNICODE *srcName,
                            OI_OBEX_UNICODE *destName);


/**
 * This function sets the permissions on a remote file or folder.
 *
 * @param connectionId     Handle representing the connection to the FTP server.
 *
 * @param name             The name of a remote file or folder
 *
 * @param groupPermission  Group permission bits as defined in oi_obexpec.h
 *
 * @param userPermission   User permission bits as defined in oi_obexpec.h
 *
 * @param otherPermission  Other permission bits as defined in oi_obexpec.h
 *
 * @return
 *                    - OI_OK if the set permission operation was started
 *                    - OI_OBEX_NOT_CONNECTED if the client is not connected to an OBEX server
 *                    - OI_OBEX_OPERATION_IN_PROGRESS if another operation is in progress
 *                    - or other errors
 *
 */
OI_STATUS OI_FTPClient_SetPermissions(OI_FTP_CLIENT_CONNECTION_HANDLE connectionId,
                                      OI_OBEX_UNICODE *name,
                                      OI_UINT8 groupPermission,
                                      OI_UINT8 userPermission,
                                      OI_UINT8 otherPermissions);
#ifdef __cplusplus
}
#endif

/**@}*/

#endif /* _OI_FTP_CLIENT_H */
