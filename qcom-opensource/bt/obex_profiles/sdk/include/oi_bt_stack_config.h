#ifndef _OI_BT_STACK_CONFIG_H
#define _OI_BT_STACK_CONFIG_H

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
 * @file
 * This file provides type definitions for configuration of data structures for various subsystems
 * of the BLUEmagic 3.0 protocol stack, used for configuring memory usage
 *
 * The comments in this file contain details on recommended and required usage of the
 * parameters defined herein. The values of these parameters may be changed in the file
 * oi_bt_stack_config_default.c.
 *
 */

#include "oi_stddefs.h"
#include "oi_time.h"
#include "oi_bt_spec.h"
#include "oi_connect_policy.h"
#include "oi_sdpdb.h"
#include "oi_support_init.h"

/** \addtogroup InitConfig */
/**@{*/

#ifdef __cplusplus
extern "C" {
#endif


/**************************************************
    Common Configuration
**************************************************/
/**
    This constant structure is used to configure parameters used by several
    modules of the BLUEmagic 3.0 Bluetooth protocol stack.
 */
typedef struct  {
    OI_UINT8    maxAclConnections ; /**< This parameter indicates how many concurrent ACL connections the protocol
                                         stack should support.
                                         Memory usage - possibly significant, this value is used to size control
                                         data structures a number of modules. */

    OI_UINT8    maxLogicalConnections ; /**< This parameter indicates how many concurrent logical connections the protocol
                                         stack should support.
                                         Memory usage - possibly significant, this value is used to size control
                                         data structures a number of modules. */

    OI_UINT8    maxScoConnections ; /**< This parameter indicates how many concurrent
                                         SCO connections the protocol stack should support.
                                         This value is used to size control data structures. */

    OI_UINT8    deviceNameMaxLen ;  /**< The Bluetooth specification v2.0+EDR allows device names
                                         to be up to 248 characters. This definition allows the local
                                         device name to be limited to something less than 248 bytes.
                                         Note that applications can write name of any length, but a read of local
                                         device will be limited by this parameter.
                                         OI_DEVMGR_EnumerateDevices() uses this same parameter to limit remote
                                         device names in the enumeration database.
                                         Length is specified as a number of bytes and INCLUDES (!!) the
                                         NULL terminator. */

    OI_UINT16 maxMTU;               /**< This parameter configures the maximum incoming L2CAP MTU that will be supported.
                                         Memory usage - significant impact, L2CAP allocates a buffer this size for each
                                         of the 'maxAclConnections'. */

    OI_UINT16 maxMPS;               /**< This parameter configures the maximum incoming L2CAP MPS that will be supported.
                                         This parameter only applies in ERTM and STREAMING mode. */

    OI_UINT8 automaticUnPark;       /**< TRUE/FALSE controls Unpark behavior if outbound data is queued for transmission */

    OI_UINT8 automaticUnSniff;      /**< TRUE/FALSE controls UnSniff behavior if outbound data is queued for transmission */

}   OI_CONFIG_COMMON_CONFIG ;

/** The common system configuration is global, accessible to all modules at all times. */
extern const OI_CONFIG_COMMON_CONFIG oi_default_config_COMMON_CONFIG ;

/**************************************************
    Dispatcher Configuration
**************************************************/

/** This sructure configures the size of the table used by the
    CThru Dispatcher to store callback function registration
    entries. The parameter indicates the size of the table in
    16-bit units, not number of entries. An entry consumes
    between 16 and 20 bytes. A reasonable parameter setting for
    many applications would be 64. A feasible minimum might be
    between 16 and 32.
 */

typedef struct {
    OI_UINT16  DispatchTableSize;   /**< size of Dispatcher table */

} OI_CONFIG_DISPATCH;

extern const OI_CONFIG_DISPATCH oi_default_config_DISPATCH ;

/*****************************************************
 * Memory Manager configuration parameters
 *****************************************************/

#ifndef OI_USE_NATIVE_MALLOC
extern const OI_CONFIG_MEMMGR oi_default_config_MEMMGR ;
#endif

#ifdef __cplusplus
}
#endif

/**@}*/

/*****************************************************************************/
#endif  /* _OI_BT_STACK_CONFIG_H */

