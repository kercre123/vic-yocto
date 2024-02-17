#ifndef _OI_BT_MODULE_INIT_H
#define _OI_BT_MODULE_INIT_H

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
 * This file provides the interface for initialization and control of the protocol stack.
 *
 * Initialization functions for the individual protocol stack and support modules of
 * BLUEmagic 3.0 software are collected together by this file (oi_bt_stack_init.h).
 * In the default implementation, module initialization is performed by oi_bt_stack_init_default.c.
 * Configuration parameters for all modules primarily control the amount of data memory space (RAM)
 * that will be allocated for buffers, queues, et cetera. The initialization parameter values
 * should be modified according to the needs of the application.
 *
 * Configuration data structures are defined in oi_bt_stack_config.h. Configuration values are set
 * in oi_bt_stack_config_default.c; documentation for this file contains recommendations for parameter settings.
 */

#include "oi_status.h"
#include "oi_stddefs.h"
#include "oi_support_init.h"
#include "oi_bt_stack_config.h"
#include "oi_bt_stack_init.h"

/** \addtogroup InitConfig */
/**@{*/

#ifdef __cplusplus
extern "C" {
#endif


/**
 *  This function resets all initialization flags.
 *
 *  Do not call this function unless you are sure that you know what you are doing.
 *
 *  This function should only be called as part of a global, reset/restart process.
 */
extern void OI_InitFlags_ResetAllFlags(void);


/**
 * This function initializes the configuration table.
 *
 * Do not call this function unless you are sure that you know what you are doing.
 *
 * Initialization sets all configuration pointers to their default values.
 * This function should be called before initializing any profiles or core protocol stack
 * components.
 */
extern void OI_ConfigTable_Init(void);

#ifdef __cplusplus
}
#endif

/**@}*/

/*****************************************************************************/
#endif /* _OI_BT_MODULE_INIT_H */

