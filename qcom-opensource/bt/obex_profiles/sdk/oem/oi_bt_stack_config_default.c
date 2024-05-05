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
 *  This file defines configuration parameters for various modules, including the CThru Dispatcher,
 *  Security Manager, security database, Device Manager, Policy Manager, and core protocol
 *  stack modules.
 *
 *  Values in this file may be changed; these values will be used to populate instances of the data structures
 *  defined in oi_bt_stack_config.h that will be sent as arguments to initialization and configuration functions.
 *  See the the @ref init_docpage section for details of recommended and required usage of these values.
 *  Do not change the file oi_bt_stack_config.h.
 *
 *  Many of the parameters in this file affect the amount of RAM memory that the stack requires.
 *  To accomodate embedded platforms where RAM may be limited, some configurations in this file
 *  will depend on whether SMALL_RAM_MEMORY is defined or not. If SMALL_RAM_MEMORY is defined,
 *  configuration will be defined for less memory usage.
 */

#include "oi_stddefs.h"
#include "oi_time.h"
#include "oi_bt_stack_config.h"
#include "oi_bt_assigned_nos.h"
#include "oi_dataelem.h"
#include "oi_l2cap.h"

/**************************************************
  common configuration
 **************************************************/

/*  This constant structure configures parameters used by several modules of the
    BLUEmagic 3.0 Bluetooth protocol stack and supporting layers. */
const OI_CONFIG_COMMON_CONFIG oi_default_config_COMMON_CONFIG =  {
#ifdef SMALL_RAM_MEMORY
    1,                    /* OI_UINT8    maxAclConnections */

    1,                    /* OI_UINT8    maxLogicalConnections */

    1,                    /* OI_UINT8    maxScoConnections */

    26,                   /* OI_UINT8    deviceNameMaxLen (includes NUL terminator) */

    672,                  /* OI_UINT16   maxMTU */

    672,                  /* OI_UINT16   maxMPS */
#else
    7,                    /* OI_UINT8    maxAclConnections */

    14,                   /* OI_UINT8    maxLogicalConnections */

    7,                    /* OI_UINT8    maxScoConnections */

    61,                   /* OI_UINT8    deviceNameMaxLen (includes NUL terminator) */

    OI_L2CAP_SDU_SAFE_MAX,/* OI_UINT16   maxMTU */

    8158,                 /* OI_UINT16   maxMPS */
#endif

    1,                    /* OI_UINT8    automaticUnPark */

    0                     /* OI_UINT8    automaticUnSniff */
};

/**************************************************
  Dispatcher configuration
 **************************************************/

/*  This constant structure configures the size of the table used by the CThru Dispatcher
 *  to store callback function registration entries. The parameter indicates the size of
 *  the table in 16-bit units, not number of entries. An entry consumes between 20 and 24 bytes.
 *  A reasonable parameter setting for many applications would be 64. A feasible minimum might
 *  be between 16 and 32.
 */
const OI_CONFIG_DISPATCH oi_default_config_DISPATCH = {
#ifdef SMALL_RAM_MEMORY
    16                   /* OI_UINT16 DispatchTableSize */
#else
    64                   /* OI_UINT16 DispatchTableSize */
#endif
};


/*****************************************************************************/
