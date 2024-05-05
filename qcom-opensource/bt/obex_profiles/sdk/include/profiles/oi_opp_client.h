#ifndef _OI_OPP_CLIENT_H_
#define  _OI_OPP_CLIENT_H_

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

  This file provides the API for the client side of the Object Push Profile.

  The Object Push Profile provides functions for establishing a connection to a
  remote device over RFCOMM and functions for pushing and pulling objects, such
  as vCard (business card) and vCal (calendar entry) objects. A Bluetooth device
  address and an RFCOMM channel number are required for setting up the
  connection. Unless the application already knows the RFCOMM channel number,
  the application will need to perform service discovery to obtain the channel
  number.

  After a connection has been established, the application can call
  OI_OPPClient_Push() to send objects to the server and OI_OPPClient_Pull() to
  retrieve the default object from the server. The default object is the owner's
  vCard business card.
 */

#include "oi_obexspec.h"
#include "oi_obex.h"
#include "oi_opp_sys.h"

/** \addtogroup OPP OPP APIs */
/**@{*/

#ifdef __cplusplus
extern "C" {
#endif



/**
 * Completion or notification events returned to the application via the @ref
 * OI_OPP_CLIENT_EVENT_CB callback function. The event indicates completion, the
 * accompanying status code indicates if the operation succeeded or failed. The
 * completion of a pull operation, OI_OPPClient_Pull(), is not signalled by and
 * event, rather a callback function is called with the pulled object.
 */
typedef enum {
    OI_OPP_CLIENT_CONNECTED,     /**< The client has connected to an OPP server */
    OI_OPP_CLIENT_DISCONNECT,    /**< The client is now disconnected from an OPP server */
    OI_OPP_CLIENT_PUSH_STARTED,  /**< A push operation was started. (Must call OI_OPPClient_NameOverride if enabled) */
    OI_OPP_CLIENT_PULL_STARTED,  /**< A pull operation was started. (Must call OI_OPPClient_NameOverride if enabled) */
    OI_OPP_CLIENT_PUSH_PROGRESS, /**< Push operation Progress Report */
    OI_OPP_CLIENT_PULL_PROGRESS, /**< Pull operation Progress Report */
    OI_OPP_CLIENT_PUSH_COMPLETE, /**< A push operation is complete */
    OI_OPP_CLIENT_PULL_COMPLETE  /**< A pull operation is complete */
} OI_OPP_CLIENT_EVENT;

/**
 * Structure for passing OPP Client events to application. This is a
 * tagged/typed structure which includes a union of structures who's meaning is
 * dependant on the actual OI_OPP_CLIENT_EVENT code.
 */

typedef struct {
    OI_OPP_CLIENT_EVENT         event;           /**< Event being posted */
    /// @cond
    union {
        struct {
            const OI_OBEX_UNICODE *fileName;        /**< File name being pushed */
            OI_UINT32              totalBytes;      /**< Total bytes in file being pushed (if known) */
        } pushStarted;                              /**< Union structure valid for @ref OI_OPP_CLIENT_PUSH_STARTED */
        struct {
            const OI_OBEX_UNICODE *fileName;        /**< File name being pulled */
            OI_UINT32              totalBytes;      /**< Total bytes in file being pulled (if known) */
        } pullStarted;                              /**< Union structure valid for @ref OI_OPP_CLIENT_PULL_STARTED */
        struct {
            OI_UINT32              bytesTransferred;/**< Total bytes pushed so far for this object */
        } pushProgress;                             /**< Union structure valid for @ref OI_OPP_CLIENT_PUSH_PROGRESS */
        struct {
            OI_UINT32              bytesTransferred;/**< Total bytes pulled so far for this object */
        } pullProgress;                             /**< Union structure valid for @ref OI_OPP_CLIENT_PULL_PROGRESS */
        struct {
            OI_UINT32              finalSize;       /**< Final bytes pulled for this object */
        } pushComplete;                             /**< Union structure valid for @ref OI_OPP_CLIENT_PUSH_COMPLETE */
        struct {
            OI_UINT32              finalSize;       /**< Final bytes pushed for this object */
        } pullComplete;                             /**< Union structure valid for @ref OI_OPP_CLIENT_PULL_COMPLETE */
    } data;
    /// @endcond
} OI_OPP_CLIENT_EVENT_DATA;

/**
 *  Represents an active connection between a OPP client and a remote OPP server
 */
typedef OI_OPP_CONNECTION OI_OPP_CLIENT_CONNECTION_HANDLE;


/**
 * The application must provide a function with this profile to
 * OI_OPPClient_Connect() when starting the object push client. This function
 * is called to signal to the application when various operations completed; see
 * @ref OI_OPP_CLIENT_EVENT_DATA.
 *
 * @param connectionId  handle representing the connection to the OPP server.

 * @param evtPtr        identifies the completion or notification event that is
 *                      being signalled.
 *
 * @param status        indicates if the operation succeeded, OI_OK, or failed
 *                      with an error.
 */
typedef void (*OI_OPP_CLIENT_EVENT_CB)(OI_OPP_CLIENT_CONNECTION_HANDLE connectionId,
                                       const OI_OPP_CLIENT_EVENT_DATA *evtPtr,
                                       OI_STATUS                       status);

/**
 * Connect to remote OBEX object push profile server.
 *
 * @param addr             the address of a remote Bluetooth device that supports OPP.
 *
 * @param lowerProtocol    This is identifies the RFCOMM channel number or the L2CAP PSM for the
 *                         FTP server running on the remote device.  the remote device. The caller
 *                         will normally perform service discovery on the remote device to obtain
 *                         the required channel number or PSM.
 *
 * @param connectionId    (OUT) pointer to return the OPP connection handle.
 *
 * @param eventCB         a callback function that is called when OPP client
 *                        operations complete.
 *
 * @param objectFunctions A set of functions that provide an interface to an
 *                        object management system that supports opening,
 *                        reading, writing of objects.
 *
 * @returns OI_OK if the connection request was sent.
 */
OI_STATUS OI_OPPClient_Connect(OI_BD_ADDR *addr,
                               OI_OBEX_LOWER_PROTOCOL *lowerProtocol,
                               OI_OPP_CLIENT_CONNECTION_HANDLE *connectionId,
                               OI_OPP_CLIENT_EVENT_CB eventCB,
                               const OI_OPP_OBJSYS_FUNCTIONS *objectFunctions);

/**
 * Terminate the current OBEX connection to a remote OPP server.
 *
 * @param connectionId  handle representing the connection to the OPP server.
 */
OI_STATUS OI_OPPClient_Disconnect(OI_OPP_CLIENT_CONNECTION_HANDLE connectionId);


/**
 * Push an object to an OBEX server. The object type must be one of the object
 * types supported by the server. The list of supported object types is obtained
 * from the service record for the server.
 *
 * @param connectionId  handle representing the connection to the OPP server.
 *
 * @param name          a NULL-terminated unicode name of the object to be pushed.
 *
 * @param type          object type
 *
 * @return OI_OK if the push command was sent, OI_OBEX_NOT_CONNECTED if a
 *               connection has not yet been established.
 */
OI_STATUS OI_OPPClient_Push(OI_OPP_CLIENT_CONNECTION_HANDLE connectionId,
                            const OI_OBEX_UNICODE *name,
                            const OI_CHAR *type);


/**
 * Pull the default object from an OBEX server. The default object is the
 * owner's business card which is an OBEX object of type @ref
 * OI_OBEX_VCARD_TYPE.
 *
 * @param connectionId  handle representing the connection to the OPP server.
 *
 * @return OI_OK if the pull request was sent, OI_OBEX_NOT_CONNECTED if a
 *               connection has not yet been established.
 */
OI_STATUS OI_OPPClient_Pull(OI_OPP_CLIENT_CONNECTION_HANDLE connectionId);




/**
 * Callback invoked when the cancellation of an operation is completed.
 *
 * @param connectionId  handle representing the connection to the OPP server.
 */
typedef void (*OI_OPP_CLIENT_CANCEL_CFM)(OI_OPP_CLIENT_CONNECTION_HANDLE connectionId);

/**
 * Terminate the current push or pull operation.
 *
 * @param connectionId  handle representing the connection to the OPP server.
 *
 * @param cancelCfm     Callback to call when cancellation is complete (may be NULL)
 *                      May or may not be called, depending on return value (see below)
 *
 * @return      OI_OK - The OBEX ABORT transaction has been initiated and the
 *                      cancelCfm callback will be called when the ABORT completes.
 *
 *              OI_STATUS_PENDING - There is an OPP PUSH or PULL in progress which must
 *                      complete before the OBEX ABORT can be sent.  When the ABORT
 *                      completes, the OI_OPP_CLIENT_EVENT_CB will be called with
 *                      PUSH (or PULL) COMPLETE event, status OI_OBEX_CLIENT_ABORTED_COMMAND.
 *                      Note that in this case, the cancelCfm callback is *NOT* called.
 *
 *              Any other value indicates error, no callbacks will be called.
 */
OI_STATUS OI_OPPClient_Cancel(OI_OPP_CLIENT_CONNECTION_HANDLE connectionId,
                              OI_OPP_CLIENT_CANCEL_CFM cancelCfm);

/**
 * This function sets name overriding. If name Overriding is set to TRUE, the
 * OI_OPPClient_NameOverride() function MUST be called upon delivery of the
 * OI_OPP_CLIENT_PULL_STARTED or OI_OPP_CLIENT_PUSH_STARTED event.
 *
 * NOTE: Until called for each connection, nameOverrideEnabled defaults to FALSE
 *       to maintain legacy functionality.
 *
 * @param connectionId         Handle representing the connection to the OPP server.
 *
 * @param nameOverrideEnabled  TRUE - This client connection requires the App to call
 *                                    OI_OPPClient_NameOverride to specify or confirm
 *                                    the name used locally (for PULLs) or remotely (for PUSHs).
 *                             FALSE - Default name will always be used.
 *
 * @return                     OI_OK - This command always succeeds if connected to
 *                                     a server, and fails otherwise.
 *
 */
OI_STATUS OI_OPPClient_EnableNameOverride(OI_OPP_CLIENT_CONNECTION_HANDLE connectionId,
                                          OI_BOOL                         nameOverrideEnabled);

/**
 * Supply Object Name. This call must be made after receiving the
 * OI_OPP_CLIENT_PULL_STARTED or OI_OPP_CLIENT_PUSH_STARTED event, if application
 * has previously specified Name Overriding with OI_OPPClient_EnableNameOverride().
 *
 * This call will fail if in an invalid state, or Name Override not enabled.
 *
 * On PULL, the name provided by this function is used to save the object in the
 * local storage system.
 *
 * On PUSH, the name provided is used in the OI_OBEX_HDR_NAME for the outgoing
 * object.
 *
 * @param connectionId  handle representing the connection to the OPP server.
 *
 * @param name          NULL to confirm name provided by OI_OPPClient_Push
 *                      or the incoming OBEX packet. Or a legal UNICODE name to
 *                      override the default.
 *
 * @return      OI_OK - If this command succeeds. Any other return indicates a
 *                      failure.
 *
 */
OI_STATUS OI_OPPClient_NameOverride(OI_OPP_CLIENT_CONNECTION_HANDLE connectionId,
                                    const OI_OBEX_UNICODE          *name);

/*************************************************************

    Debug print helpers

        Returns null string when compiled for release mode

*************************************************************/

extern OI_CHAR  *OI_OPPClient_eventText(OI_OPP_CLIENT_EVENT event) ;


#ifdef __cplusplus
}
#endif

/**@}*/

#endif /* _OPP_CLIENT_H */
