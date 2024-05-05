#ifndef _OI_PBAP_SERVER_H
#define _OI_PBAP_SERVER_H

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
 *  @file
 *
 *  This file provides the API for the server side of the Phonebook Access
 *  Profile.
 *
 *  The Phonebook Access Profile provides functions for establishing a
 *  connection to a remote device that supports the Phonebook Access Profile
 *  over RFCOMM and functions for putting and getting files. This
 *  implementation currently only allows one client connection at a time.
 */

#include "oi_obexsrv.h"
#include "oi_status.h"
#include "oi_sdp.h"
#include "oi_pbap_sys.h"

/** \addtogroup PBAP PBAP APIs */
/**@{*/

#ifdef __cplusplus
extern "C" {
#endif


/**
 * Type of a registered PBAP server instance.
 */
typedef OI_OBEX_SERVER_HANDLE OI_PBAP_SERVER_HANDLE;


/**
 * A callback function of this type is called to indicate a connection request.
 * The server application provides a function with this profile to
 * OI_PBAPServer_Register().
 *
 * @param clientAddr     The Bluetooth device address of the client requesting
 *                       the connection.
 *
 * @param unauthorized   Indicates if the connection requires authentication. If
 *                       this parameter is TRUE the server application must call
 *                       OI_PBAPServer_AuthenticationResponse() to provide a password
 *                       to PBAP server.
 *                       After succesful authentication this function will be
 *                       called again wuth unauthorized == FALSE indicating that
 *                       the connection has been established and the server application
 *                       must call OI_PBAPServer_AcceptConnection(). If authentication
 *                       was attempted but failed this parameter will still be TRUE.
 *
 * @param userId         The user id  of the client requesting the connection.
 *                       If this parameter is NULL and unauthorized is TRUE this
 *                       means either that the server received authentication
 *                       challenge from the connecting client or, in case of handling
 *                       authentication response, client's user id was not required.
 *                       If userId is nonzero and the user id match is found, the server
 *                       application must call OI_PBAPServer_AuthenticationResponse()
 *                       to provide a password to PBAP server. The application server
 *                       may use this value to choose a root directory or select
 *                       a previously stored password.
 *                       If the user id match is not found, the server application must call
 *                       OI_PBAPServer_AcceptConnection() to either disconnect or
 *                       request another authentication attempt from the client.
 *
 * @param userIdLen      Length of the user id.
 *
 * @param connectionId   handle representing the connection to an PBAP client.
 */
typedef void (*OI_PBAP_CONNECTION_IND)(OI_BD_ADDR *clientAddr,
                                       OI_BOOL unauthorized,
                                       OI_BYTE *userId,
                                       OI_UINT8 userIdLen,
                                       OI_PBAP_CONNECTION connectionId);

/**
 * This function is called by the application in response to a connnection
 * indication that requires authentication.
 *
 * @param connectionId handle representing the connection to an PBAP client.
 *
 * @param userId       the user id for the connecting user. If the routine is called in response
 *                     to a authentication challenge from a client, this parameter should be set
 *                     to a User Id provided by the server.
 *                     If the routine is called to handle authentication response from a client,
 *                     this parapemter would hold a User Id provided by the client.
 *
 * @param userIdLen    Length of the userId.
 *
 * @param password     NULL-teminated password
 */
OI_STATUS OI_PBAPServer_AuthenticationResponse(OI_PBAP_CONNECTION connectionId,
                                               const OI_BYTE *userId,
                                               OI_UINT8 userIdLen,
                                               const OI_CHAR *password);
/**
 * An application calls this function to accept or reject an indicated
 * connection.
 *
 * @param connectionId   handle representing the connection to an PBAP client.
 *
 * @param newMissedCalls  Number of missed calls not yet checked by the user.
 *
 * @param accept         TRUE if the connection is being accepted, FALSE
 *                       otherwise. If FALSE the values of the remaining
 *                       parameters is ignored.
 *
 */
OI_STATUS OI_PBAPServer_AcceptConnection(OI_PBAP_CONNECTION connectionId,
                                         OI_UINT8 newMissedCalls,
                                         OI_BOOL accept);


/**
 * Forcibly severs the connection from a PBAP client to the PBAP server. This
 * function should be called for all active connections before terminating the
 * PBAP server.
 *
 * @param connectionId  handle representing the connection between a remote
 *                      client and the local PBAP server.
 *
 * @return              OI_OK if the connection will be terminated.
 */
OI_STATUS OI_PBAPServer_ForceDisconnect(OI_PBAP_CONNECTION connectionId);




/**
 * This function updates the number newly missed calls when for when the
 * client makes a request of the missed call history phonebook.
 *
 * @param newMissedCalls    Number of newly missed calls in the missed call
 *                          history.
 */
void OI_PBAPServer_SetNewMissedCalls(OI_UINT8 newMissedCalls);


/**
 * The function gets called when the client needs to know the number of
 * entries in a phonebook.
 *
 * @param connectionId  handle representing the connection between a remote
 *                      client and the local PBAP server.
 *
 * @param repository    The local or SIM1 repository to get the phonebook size from.
 *
 * @param phonebook     The phonebook in the repository to get the size of.
 *
 */
typedef void (*OI_PBAP_SERVER_GET_PHONEBOOK_SIZE_IND)(OI_PBAP_CONNECTION connectionId,
                                                      OI_PBAP_REPOSITORY repository,
                                                      OI_PBAP_PHONEBOOK phonebook);


/**
 * This function is for the reponse to the get phonebook size request.
 *
 * @param connectionId  handle representing the connection between a remote
 *                      client and the local PBAP server.
 *
 * @param size          Total number of entries in the phonebook.
 *
 * @param status        OI_OK if the determination of size was successful.
 *
 * @return              OI_OK if sending response to client succeeded.
 */
OI_STATUS OI_PBAPServer_GetPhonebookSizeRsp(OI_PBAP_CONNECTION connectionId,
                                            OI_UINT16 size,
                                            OI_STATUS status);



/**
 * This function initialzes the PBAP server to accept connections and registers
 * it with the SDP database so that the service becomes discoverable.
 *
 * @param authentication This parameter indicates whether connections to this
 *                       server must be authenticated using OBEX authentication.
 *                       If this parameter is FALSE, clients connecting to the
 *                       server may demand authentication.
 *
 * @param connectInd     a callback function for indicating an incoming client
 *                       connection to the server application.
 * @param disconnectInd  a callback function for indicating a disconnection
 *
 * @param fileOperations interface to file system operations.
 *
 * @param strings        strings to register in service record
 *
 * @param srecHandle     [OUT] returns the handle to the SDP record
 *
 * @param serverHandle   [OUT] returns the server handle for
 *                       the server. The server handle is required for
 *                       OI_PBAPServer_Deregister().
 *
 * @param suppRepositories  Bit mask describing the repositories supported by the
 *                          server.
 *
 * @return               OI_OK if the service was successfully registered.
 */

OI_STATUS OI_PBAPServer_Register(OI_OBEXSRV_AUTHENTICATION authentication,
                                 OI_PBAP_CONNECTION_IND connectInd,
                                 OI_PBAP_DISCONNECTION_IND disconnectInd,
                                 OI_PBAP_SERVER_GET_PHONEBOOK_SIZE_IND getPhonebookSize,
                                 const OI_PBAP_SERVER_FILESYS_FUNCTIONS *fileOperations,
                                 const OI_SDP_STRINGS *strings,
                                 OI_UINT32 *srecHandle,
                                 OI_UINT8 suppRepositories,
                                 OI_PBAP_SERVER_HANDLE *serverHandle);


/**
 * Deregisters a PBAP server.
 *
 * @param serverHandle    The server handle returned when the server was registered.
 *
 * @return                OI_OK if the service was successfully deregistered.
 *
 */
OI_STATUS OI_PBAPServer_Deregister(OI_PBAP_SERVER_HANDLE serverHandle);


#ifdef __cplusplus
}
#endif

/**@}*/

#endif /* _OI_PBAP_SERVER_H */
