#ifndef _OI_BT_STACK_INIT_H
#define _OI_BT_STACK_INIT_H

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
 *
 * This file provides the interface for initialization and control of the protocol stack.
 *
 * Initialization functions for the entire BLUEmagic 3.0 protocol
 * stack software are collected together by this file.
 *
 * Configuration data structures are defined in oi_bt_stack_config.h. Configuration values are set
 * in oi_bt_stack_config_default.c; documentation for this file contains recommendations for parameter settings.
 */

#include "oi_status.h"
#include "oi_stddefs.h"

/** \addtogroup InitConfig */
/**@{*/

#ifdef __cplusplus
extern "C" {
#endif


/**
    A callback function of this type is used to indicate that BM3 initialization has completed.

    @param status    status code
*/
typedef void(*OI_INIT_COMPLETE_CB)(OI_STATUS status);

/**
 * A callback function of this type is used to indicate when the BLUEmagic 3.0 protocol stack
 * requires service by the application's dispatch thread/routine.
 *
 * @param msec      number of milliseconds before the protocol stack requires service
 */
typedef void(*OI_DISPATCH_SERVICE_REQUEST_HANDLER)(OI_UINT msec);

/**
 *
 * This function invokes all callback functions that are ready to run.
 * See the @ref dispatch_docpage section for details.
 *
 * @return               a count of the number of callback functions that were called, a value that can
 *                       be used to test if any callback functions were called
 */
extern OI_INT16 OI_Dispatch_Run(void);

/**
 *
 * This function initializes the entire BLUEmagic 3.0 protocol stack and its support modules.
 *
 * @param initComplete   This is a function in the application that is called
 *                       when initialization has completed. No BM3 calls should
 *                       be made until this callback occurs with a successful
 *                       result code.  Callback may not be NULL.
 *
 * @param handler   This is a function in the runtime environment that is called by the BLUEmagic 3.0
 *                  protocol stack to request immediate or future service from the runtime environment.
 *                  The stack initialization function simply passes this function on to OI_Dispatch_Init().
 * @note This function is not part of the BLUEmagic 3.0 protocol stack per se; this function implementation
 *          must be provided by the user of the stack.  A default implementation of this function
 *          is provided in the SDK.
 */
OI_STATUS OI_BT_StackInit(OI_INIT_COMPLETE_CB                 initComplete,
                          OI_DISPATCH_SERVICE_REQUEST_HANDLER handler);

/**
 *
 * This function initializes the entire BLUEmagic 3.0 protocol stack and its support modules.
 *
 *  Customization:
 *      The customCB is called after the configuration table is initialized with the default values (typically
 *      specified in oi_bt_stack_config_default.c), but before any core stack components (HCI, L2CAP, etc)
 *      have been initialized.
 *
 *      In the customCB, the application may specify its custom configuration(s) with the API's provided in
 *      oi_config_table.h.  The customCB must not call any BM3 api other than OI_CONFIG_TABLE_PUT() and/or
 *      OI_CONFIG_TABLE_GET().  Please refer to oi_config_table.h file for details of these api's.
 *
 * @param initComplete   This is a function in the application that is called
 *                       when initialization has completed. No further BM3 calls should
 *                       be made until this callback occurs with a successful
 *                       result code.  Callback may not be NULL.
 *
 * @param handler   This is a function in the runtime environment that is called by the BLUEmagic 3.0
 *                  protocol stack to request immediate or future service from the runtime environment.
 *                  The stack initialization function simply passes this function on to OI_Dispatch_Init().
 *
 * @param customCB  This is the callback that is to be invoked at the appropriate point during initialization.
 *                  Specify NULL if there is no customization required
 *
 * @return      OI_OK, if initialization request is accepted, callback will be called.
 *              Any other value, initialization request failed, callback will not be called.
 *
 * @note This function is not part of the BLUEmagic 3.0 protocol stack, per se; this function implementation
 *          must be provided by the user of the stack. A default implementation of this function
 *          is provided in the SDK.
 */
OI_STATUS OI_BT_StackInit_Custom(OI_INIT_COMPLETE_CB                 initComplete,
                                 OI_DISPATCH_SERVICE_REQUEST_HANDLER handler,
                                 void (*customCB)(void));

/**
 * This function terminates the entire stack and profiles.
 *
 *  The stack is terminated by removing any pending callbacks from the dispatcher
 *  and marking all stack modules as un-initialized.  Any subsequent calls into the
 *  will fail.
 *
 *  Prior to stack termination, both the radio and transports should be closed by
 *  calling :
 *      OI_HCI_RadioClose()
 *      OI_HCI_CloseAllTransports()
 *
 *  Closing the radio will cleanly disconnect all existing connections and will prevent
 *  new operations while the stack is being terminated.
 *
 *  Closing the transports relinquishes any OS resources that may have been allocated
 *  when opening transports.
 *
 *  @Note OI_BT_Stack_Terminate() must be called an application thread!!  It must not be
 *  called from a stack callback.  Failure to follow the sequence described above might
 *  result in race or deadlock condition.
 *
 * @return      termination result.
 */

OI_STATUS OI_BT_Stack_Terminate(void);

#ifdef __cplusplus
}
#endif

/**@}*/

#endif /* _OI_BT_STACK_INIT_H */

