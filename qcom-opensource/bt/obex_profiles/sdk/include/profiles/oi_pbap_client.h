#ifndef _OI_PBAP_CLIENT_H
#define _OI_PBAP_CLIENT_H

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
 * Phonebook Access Profile client API
 *
 * The Phonebook Access Profile provides functions for establishing a
 * connection to a remote device that supports the Phonebook Access Profile
 * over RFCOMM and functions for pulling vCards. This implementation currently
 * only allows one connection at a time. A Bluetooth device address and an
 * RFCOMM channel number are required for setting up the connection. Unless
 * the application already knows the RFCOMM channel number, the application
 * will need to perform service discovery to obtain the channel number.
 *
 * All phonebook data (vCards and vCard listings) go through the callbacks
 * defined in oi_pbap_sys.h.  That is, when there is vCard data from the
 * server, the open callback will be called to establish reference handle
 * between the application and the PBAP client API.  The PBAP client API will
 * then use the write callback to send the actual vCard data to the
 * application.  The close callback lets the application know that get data
 * operation is now complete.
 */

#include "oi_obexspec.h"
#include "oi_obexcli.h"
#include "oi_sdp.h"
#include "oi_pbap_consts.h"
#include "oi_pbap_sys.h"

/** \addtogroup PBAP PBAP APIs */
/**@{*/

#ifdef __cplusplus
extern "C" {
#endif


/**
 * This function indicates the result of a connection attempt to a PBAP
 * server.
 *
 * @param connectionId Handle representing the connection to the PBAP server.
 *
 * @param status       OI_OK if the connection succeeded, or an error code if
 *                     the operation failed
 */
typedef void (*OI_PBAP_CONNECTION_CFM)(OI_PBAP_CONNECTION connectionId,
                                       OI_STATUS status);

/**
 * A callback function of this type is made to the application to request a password and
 * optional user id to authenticate connection. The client must provide this callback function
 * to OI_PBAPClient_ConnectAuthenticate() if the client supports OBEX authentication.
 * The implementation of this callback function must call OI_PBAPClient_AuthenticationRsp() before
 * return.
 *
 * @param connectionId     a unique ID that represents the in-progress connection to PBAP server.
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
 */
typedef void (*OI_PBAP_CLIENT_AUTHENTICATION_CB)(OI_PBAP_CONNECTION connectionId,
                                                 OI_BOOL userIdRequired);


/**
 * This function establishes an authorized OBEX connection over RFCOMM to a
 * remote PBAP server.  The client must provide a callback function of
 * type OI_PBAP_CLIENT_AUTHENTICATION_CB to this function if the client
 * supports OBEX authentication.
 *
 * @param addr              the address of a remote Bluetooth device that supports PBAP
 *
 * @param lowerProtocol     This identifies the RFCOMM channel number or the L2CAP PSM for the
 *                          PBAP server running on the remote device.  the remote device. The caller
 *                          will normally perform service discovery on the remote device to obtain
 *                          the required channel number or PSM.
 *
 * @param authentication    specifies whether authentication is required when
 *                          connecting to a server
 *
 * @param connectionId      [OUT] returns a handle representing the connection to the PBAP server
 *
 * @param supportedFeatures     specified supported features for PBAP Client.
 *
 * @param connectionCfm     callback function for indicating the connection result status.
 *
 * @param disconnectInd     callback function to indicate a disconnection.
 *
 * @param authenticationCB  authentication callback function
 *
 * @return                  OI_OK if the connection request can be issued
 */
OI_STATUS OI_PBAPClient_Connect(OI_BD_ADDR *addr,
                                OI_OBEX_LOWER_PROTOCOL *lowerProtocol,
                                OI_OBEXCLI_AUTHENTICATION authentication,
                                OI_PBAP_CONNECTION *connectionId,
                                OI_UINT32 supportedFeatures,
                                OI_PBAP_CONNECTION_CFM connectionCfm,
                                OI_PBAP_DISCONNECTION_IND disconnectInd,
                                OI_PBAP_CLIENT_AUTHENTICATION_CB authenticationCB,
                                const OI_PBAP_CLIENT_FILESYS_FUNCTIONS *fops);


/**
 * This function is called by the application in response to an authentication
 * request to provide a password and optional user id for OBEX authentication.
 *
 * @param connectionId handle representing the connection to the PBAP server.
 *
 * @param userId       the user id for the connecting client. This parameter is
 *                     required if the server indicated that user id is
 *                     required. Otherwise is can be NULL. The client can
 *                     provide a user id even if the server does not require
 *                     one.
 *
 * @param userIdLen    Length of the userId.
 *
 * @param password     is a NULL terminated client's password.
 */
OI_STATUS OI_PBAPClient_AuthenticationRsp(OI_PBAP_CONNECTION connectionId,
                                          const OI_BYTE *userId,
                                          OI_UINT8 userIdLen,
                                          const OI_CHAR *password);




/**
 * This function terminates the current OBEX connection to a remote PBAP server.
 *
 * @param connectionId handle representing the connection to the PBAP server.
 *
 */
OI_STATUS OI_PBAPClient_Disconnect(OI_PBAP_CONNECTION connectionId);

/**
 * This callback informs the application that the aborting of the current
 * operation has comleted.
 *
 * @param connectionId  handle representing the connection to the PBAP server.
 */
typedef void (*OI_PBAP_ABORT_CFM)(OI_PBAP_CONNECTION connectionId);

/**
 * This function terminates the current operation.
 *
 * @param connectionId  handle representing the connection to the PBAP server.
 *
 * @return an error status if there is no operation to abort.
 */
OI_STATUS OI_PBAPClient_Abort(OI_PBAP_CONNECTION connectionId, OI_PBAP_ABORT_CFM abortCfm);


/**
 * This callback function reports the number of phonebook entries for the
 * currently selected phonebook to the application.
 *
 * @param connectionId   Handle representing the connection to the PBAP server.
 *
 * @param phonebooksize  Total number of entries in the phonebook.
 *
 * @param status         OI_OK if the operation succeeded, or an error code if
 *                       the operation failed.
 */
typedef void (*OI_PBAP_CLIENT_GET_PHONEBOOK_SIZE_CB)(OI_PBAP_CONNECTION connectionId,
                                                     OI_UINT16 phonebookSize,
                                                     OI_STATUS status);



/**
 * This function gets the number of phonebook entries in the currently
 * selected phonebook.
 *
 * @param connectionId        Handle representing the connection to the PBAP server.
 *
 * @param repository          Phonebook repository to access
 *
 * @param phonebook           Phonebook to access
 *
 * @param getPhonebookSizeCb  Callback function to call with the size information.
 *
 * @return OI_OK if request for setting the phonebook was successfully sent.
 */
OI_STATUS OI_PBAPClient_GetPhonebookSize(OI_PBAP_CONNECTION connectionId,
                                         OI_PBAP_REPOSITORY repository,
                                         OI_PBAP_PHONEBOOK phonebook,
                                         OI_PBAP_CLIENT_GET_PHONEBOOK_SIZE_CB getPhonebookSizeCB);


/**
 * This callback function informs the application of the completion status of the
 * pull phonebook operation and the number of newly missed calls if the
 * current phonebook is the missed calls phonebook.
 *
 * @param connectionId    Handle representing the connection to the PBAP server.
 *
 * @param newMissedCalls  Number of missed calls not checked by the user (when in
 *                        the missed calls phonebook).
 *
 * @param status          OI_OK if the operation succeeded, or an error code if
 *                        the operation failed.
 */
typedef void (*OI_PBAP_CLIENT_PULL_PHONEBOOK_CB)(OI_PBAP_CONNECTION connectionId,
                                                 OI_UINT8 newMissedCalls,
                                                 OI_STATUS status);

/**
 * This function pulls the contents of the current phonebook.
 *
 * @param connectionId    Handle representing the connection to the PBAP server.
 *
 * @param repository      Phonebook repository to access
 *
 * @param phonebook       Phonebook to access
 *
 * @param filter          An OR'd list of all vCard attributes to include (0 retrieves
 *                        all avaliable attributes).
 *
 * @param format          Select vCard 2.1 or 3.0 format.
 *
 * @param maxListCount    Maximum number of phonebook entries to include.
 *
 * @param listStartOffset Starting position in list of phone book entries to retrieve.
 *
 * @param pullPhonebookCB Callback function to call when phonebook data is received.
 *
 * @return OI_OK if request for phonebook was successfully sent.
 */
OI_STATUS OI_PBAPClient_PullPhonebook(OI_PBAP_CONNECTION connectionId,
                                      OI_PBAP_REPOSITORY repository,
                                      OI_PBAP_PHONEBOOK phonebook,
                                      const OI_UINT64 *filter,
                                      OI_PBAP_FORMAT_TAG_VALUES format,
                                      OI_UINT16 maxListcount,
                                      OI_UINT16 listStartOffset,
                                      OI_PBAP_CLIENT_PULL_PHONEBOOK_CB pullPhonebookCB);



/**
 * This callback function provides the list of vCard entries that match the
 * requested criteria.
 *
 * @param connectionId   Handle representing the connection to the PBAP server.
 *
 * @param newMissedCalls Number of missed calls not checked by the user.
 *
 * @param status         OI_OK if the operation succeeded, or an error code if
 *                       the operation failed.
 */
typedef void (*OI_PBAP_CLIENT_PULL_VCARD_LISTING_CB)(OI_PBAP_CONNECTION connectionId,
                                                     OI_UINT8 newMissedCalls,
                                                     OI_STATUS status);

/**
 * This function pulls a list of vCards from the current phonebook.
 *
 * @param connectionId       Handle representing the connection to the PBAP server.
 *
 * @param repository         Phonebook repository to access
 *
 * @param phonebook          Phonebook to access
 *
 * @param order              The sorting order of the vCards.
 *
 * @param searchAttribute    Search criteria type.
 *
 * @param searchValue        String that specifies what to search for (NULL to match all vCards).
 *
 * @param searchValueLen     Number of bytes in the searchValue (0 to match all vCards).
 *
 * @param maxListCount       Maximum number of phonebook entries to include.
 *
 * @param listStartOffset    Starting position in list of phone book entries to retrieve.
 *
 * @param pullvCardListingCB Callback function to call when the vCard listing is received.
 *
 * @return OI_OK if request for the vCard listing was successfully sent.
 */
OI_STATUS OI_PBAPClient_PullvCardListing(OI_PBAP_CONNECTION connectionId,
                                         OI_PBAP_REPOSITORY repository,
                                         OI_PBAP_PHONEBOOK phonebook,
                                         OI_PBAP_ORDER_TAG_VALUES order,
                                         OI_PBAP_SEARCH_ATTRIBUTE_TAG_VALUES searchAttribute,
                                         OI_BYTE *searchValue,
                                         OI_UINT8 searchValueLen,
                                         OI_UINT16 maxListCount,
                                         OI_UINT16 listStartOffset,
                                         OI_PBAP_CLIENT_PULL_VCARD_LISTING_CB pullvCardListingCB);


/**
 * This callback function provides the requested vCard entry.
 *
 * @param connectionId Handle representing the connection to the PBAP server.
 *
 * @param status       OI_OK if the operation succeeded, or an error code if
 *                     the operation failed.
 */
typedef void (*OI_PBAP_CLIENT_PULL_VCARD_ENTRY_CB)(OI_PBAP_CONNECTION connectionId,
                                                   OI_STATUS status);

/**
 * This function pulls a specific vCard from the current phonebook.
 *
 * @param connectionId Handle representing the connection to the PBAP server.
 *
 * @param repository   Phonebook repository to access
 *
 * @param phonebook    Phonebook to access
 *
 * @param entry        The vCard entry to retrieve.
 *
 * @param filter       An OR'd list of all vCard attributes to include (0 retrieves
 *                     all avaliable attributes).
 *
 * @param format       Select vCard 2.1 or 3.0 format.
 *
 * @param pullvCardCB  Callback function to call when the vCard listing is received.
 *
 * @return OI_OK if request for the vCard entry was successfully sent.
 */
OI_STATUS OI_PBAPClient_PullvCardEntry(OI_PBAP_CONNECTION connectionId,
                                       OI_PBAP_REPOSITORY repository,
                                       OI_PBAP_PHONEBOOK phonebook,
                                       OI_UINT32 entry,
                                       const OI_UINT64 *filter,
                                       OI_PBAP_FORMAT_TAG_VALUES format,
                                       OI_PBAP_CLIENT_PULL_VCARD_ENTRY_CB pullvCardCB);

/**
 * This callback function provides the requested setpath.
 *
 * @param connectionId Handle representing the connection to the PBAP server.
 *
 * @param status       OI_OK if the operation succeeded, or an error code if
 *                     the operation failed.
 */
typedef void (*OI_PBAP_CLIENT_SETPATH_CB)(OI_PBAP_CONNECTION connectionId,
                                                   OI_STATUS status);

/**
 * This function set path to specified repository.
 *
 * @param connectionId Handle representing the connection to the PBAP server.
 *
 * @param repository   Phonebook repository to access
 *
 * @param phonebook    Phonebook to access
 *
 * @param setPathCB  Callback function to call when the set path is completed.
 *
 * @return OI_OK if set path is done successfully.
 */
OI_STATUS OI_PBAPClient_SetPath(OI_PBAP_CONNECTION connectionId,
                                       OI_PBAP_REPOSITORY repository,
                                       OI_PBAP_PHONEBOOK phonebook,
                                       OI_PBAP_CLIENT_SETPATH_CB setPathCB);

#ifdef __cplusplus
}
#endif

/**@}*/

#endif /* _OI_PBAP_CLIENT_H */
