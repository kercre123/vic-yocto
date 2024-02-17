#ifndef _OI_POLICY_MGR_H
#define _OI_POLICY_MGR_H

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
 * Policy manager API
 *
 * This file defines the policy manager api.
 *
 * Basically, all multiplexing protocols report when a new connection is created and
 * when a connection is disconnected.  When a new connection is created, the policy
 * manager will enforce the acl link policies defined by the various services using
 * the same ACL link.
 */

#include "oi_status.h"
#include "oi_connect_policy.h"

/** \addtogroup PolicyMgr_Internal */
/**@{*/

#ifdef __cplusplus
extern "C" {
#endif


/*************************************************************

    Policy Manager API uses handles to connections.
    The handle is an opaque 32-bit value, but zero will never be a valid handle

*************************************************************/

#define OI_POLICYMGR_NULL_HANDLE     0

/*************************************************************

    New connection callback.

    The callback is called when the policy manager has finished enforcing the specified policies.
    If the policy manager was unsuccessful (i.e. status != OI_OK), the multiplexing protocol
    must reject the connection.

    @param  handle      Identifier which associates the callback with the original call to
                        OI_POLICYMGR_NewConnection().

    @param  status      Results of the attempt to enforce the specified policies.

    @param  securityFailure If the status parameter is NOT OI_OK, this boolean indicates whether
                        the failure occurred as a result of enforcing security policy.
*************************************************************/

typedef void(*OI_POLICYMGR_NEW_CONNECT_CB)(
                            OI_UINT32   handle,                 /**< connection identifier */
                            OI_STATUS   status,                 /**< policy enforcement result */
                            OI_BOOL     securityFailure) ;

/*************************************************************

    New connection

    The multiplexing protocol calls the policy manager when a new connection is being created.
    The policy manager will attempt to enforce the various policies on the connection and then
    call back the multiplexing protocol.  If the policy manager is not successful in enforcing
    the policies, the multiplexing protocol must reject the connection attempt.

    @param  cb          Callback that will be called after the policies have been enforced.

    @param  pAddr       Device address which is being connected

    @param  pPolicy     Ptr to the policy that is to be applied to the connection.

    @param  pHandle     If the function succeeds, it will return a handle used to identify the
                        connection in subsequent interactions with the policy manager.

    @param incoming    Incoming (TRUE) or outgoing (FALSE) connection.

    @return             OI_OK, policy manager will attempt to enforce the given policy and will
                            invoke the callback when done.
                        Any other value, the policy manager will not attempt to enforce the policy
                            and will not call the callback.

*************************************************************/

extern OI_STATUS    OI_POLICYMGR_NewConnection(
                            OI_POLICYMGR_NEW_CONNECT_CB cb,         /**< callback when done */
                            const OI_BD_ADDR            *pAddr,     /**< Bluetooth device address */
                            const OI_CONNECT_POLICY     *pPolicy,   /**< policy to be applied to the connection */
                            OI_UINT32                   *pHandle,   /**< identifier returned to caller */
                            OI_BOOL                     incoming);  /**< incoming/outgoing connection */

/*************************************************************

    Disconnect

    The multiplexing protocol calls the policy manager when a disconnect occurs.  The handle
    is the handle that was returned by OI_POLICYMGR_NewConnection().

    This is a synchronous call which always succeeds.

    @param  handle      Identifies the connection which has been disconnected.  Although 0 is not a valid
                        handle, for caller's convenience, this function will accept a 0 handle without
                        major complaint (Issues DBGPRINT msg).

*************************************************************/

extern void OI_POLICYMGR_Disconnect(
                            OI_UINT32   handle) ;

/*************************************************************

    Get the current devmgr policy for an ACL link.

    Devmgr does not cache the policy, but rather uses this accessor function when it needs
    to query the current devmgr policy.  This query is used when a devmgr api is called which
    might potentially conflict with the current policy (e.g. setRole).

    The power-saving link policy returned by this function is in HCI format,
    i.e. a bit mask of power-saving enables.

*************************************************************/

extern void OI_POLICYMGR_GetDevmgrPolicy(OI_BD_ADDR  *pAddr,
                                  OI_BOOL     *pMustBeMaster,
                                  OI_UINT16   *pPowerSavePolicy);

/*************************************************************

    Device Manager Disconnect

    This policy manager function is called by the device manager when an ACL link has been dropped.
    Normally, the policy manager will already have been informed by the multiplexing protocols that
    services have been disconnected.  This function serves as a double check.

    @param  pAddr      Identifies the device which has been disconnected.
*************************************************************/

extern void OI_POLICYMGR_DevmgrDisconnect(OI_BD_ADDR *pAddr) ;

/*************************************************************

    L2CAP policy removal notification

        This is a private api for L2CAP.
        Note that this callback may be invoked from an application thread.

*************************************************************/

typedef void(*OI_POLICYMGR_REMOVAL_CB)(OI_HCI_CONNECTION_HANDLE    connectionHandle);


void OI_POLICYMGR_RegisterL2capPolicyRemoval(OI_POLICYMGR_REMOVAL_CB Cb);

/*****************************************************************************/
#ifdef __cplusplus
}
#endif

/**@}*/

#endif /* _OI_POLICY_MGR_H */

