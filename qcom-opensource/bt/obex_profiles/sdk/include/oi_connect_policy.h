#ifndef _OI_CONNECT_POLICY_H
#define _OI_CONNECT_POLICY_H

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

This file defines the connection policy structure that is used to
communicate application and service requirements.
*/

#include "oi_stddefs.h"
#include "oi_l2cap_qos.h"
#include "oi_dataelem.h"


/** \addtogroup PolicyMgr Policy Manager APIs */
/**@{*/

#ifdef __cplusplus
extern "C" {
#endif


/* Bit definitions for the powerSavingDisables element. */
#define OI_POLICY_DISABLE_HOLD      OI_BIT0
#define OI_POLICY_DISABLE_SNIFF     OI_BIT1
#define OI_POLICY_DISABLE_PARK      OI_BIT2

    /**
       This structure defines the connection policy associated with
       the service.
     */
typedef struct  {

    /** UUID of the service. 16-bit, 32-bit, or 128 bit UUID as
        dataelement */
    OI_DATAELEM         serviceUuid;

    /** Indicates that the local device must be master of the ACL link */
    OI_BOOL             mustBeMaster ;

    /** Quality of service requirements for the link. This element
        must point to a static, constant structure, which must
        perist at least as long as the connection exists. The
        pointer may be NULL, in which case no L2CAP quality of
        service requirements will be enforced. Note that L2CAP
        guaranteed quality of service is not supported by this
        release of BLUEmagic software, nor does any profile
        specification require it. */
    OI_L2CAP_FLOWSPEC   *flowspec;

    /** The power-saving modes that should be disallowed on this link. This is a bit-wise OR of
        the possible power-saving mode is disabled. A value of 0
        define all power-saving modes are enabled. */
    OI_UINT8            powerSavingDisables ;

} OI_CONNECT_POLICY ;


/**
The callback is called when the policy manager has finished enforcing the specified policies.

@param  handle      Connection identifier which associates the callback with the original call to
                    OI_POLICYMGR_AddConnectionPolicy().

@param  status      Results of the attempt to enforce the specified policies.
                    Any value other than OI_SUCCESS indicates that policy enforcement failed.

@param  securityFailure If the status parameter is NOT OI_OK, this boolean indicates whether
                        the failure occurred as a result of enforcing security policy.
*/
typedef void(*OI_POLICYMGR_ADD_CONNECT_POLICY_CB)(
                            OI_UINT32   handle,
                            OI_STATUS   status,
                            OI_BOOL     securityFailure) ;


/**
This function requests the policy manager to enforce the specified
policies on the indicated connection.  If successful, the policy will
remain in effect until the acl link is disconnected or until the
application calls OI_POLICYMGR_RemoveConnectionPolicy().

@param    cb  Callback that will be called after the policies have been enforced.

@param    pAddr  Bluetooth device address identifies the connection.

@param    pPolicy Pointer to the policy that is to be applied to the connection. This may
          not be NULL.

@param    pHandle Identifier returned to caller. If the function
          succeeds, it will return a handle used to identify the connection
          policy. This handle will be passed to caller in the callback. The
          handle is also needed in order to remove the policy when it no longer
          needs to be in effect.

@return   OI_OK if the policy manager will attempt to enforce the given policy and will
          invoke the callback when done, or any other value, if the policy manager
          will not attempt to enforce the policy and will not call the callback.
*/
OI_STATUS    OI_POLICYMGR_AddConnectionPolicy(
                            OI_POLICYMGR_ADD_CONNECT_POLICY_CB cb,
                            const OI_BD_ADDR            *pAddr,
                            const OI_CONNECT_POLICY     *pPolicy,
                            OI_UINT32                   *pHandle) ;

/**
This function removes a policy. It is important that applications remove
policies when they are no longer needed.

This is a synchronous call which always succeeds, the actual removal
of policies is performed asynchronously.

@param  handle      Identifies the policy to be removed. This is the handle that was passed to
                    the application when the policy was added to the connection.
*/
void OI_POLICYMGR_RemoveConnectionPolicy(OI_UINT32   handle) ;


#ifdef __cplusplus
}
#endif

/**@}*/

#endif /* _OI_CONNECT_POLICY_H */


