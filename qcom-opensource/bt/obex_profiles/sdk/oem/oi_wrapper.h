#ifndef _OI_WRAPPER_H
#define _OI_WRAPPER_H

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
 * This file provides the interface to the sample protocol stack
 * wrapper.
 *
 * For each platform, an example protocol stack wrapper is provided to demonstrate
 * application-level management of the BLUEmagic 3.0 protocol stack and support modules.
 * The stack wrapper initializes the protocol stack and support modules and provides
 * a stack access token (usually a mutex, but perhaps some other mechanism, depending
 * on the specific operating system in use) to enforce exclusive access to the protocol
 * stack.
 */

#include "oi_stddefs.h"
#include "oi_status.h"
#include "oi_bt_stack_init.h"

#ifdef __cplusplus
extern "C" {
#endif

/**
 * This function performs initialization of the stack wrapper module,
 * including initializing the stack synchronization scheme.
 * This function allocates the stack access token and initializes the lower and upper
 * layer protocol stack. The stack access token does not exist until this function
 * returns, so no calls that obtain or release the token should be called prior to
 * this function.
 *
 *  - OI_OBEX_Init &mdash; Legacy Synchronously Only
 *    form.
 *
 *
 * @param initComplete         Function to call upon completion of stack
 *                             initialization. If NULL is
 *                             passed, then the call is
 *                             completed synchronously.
 *
 * @param idleSleepMsecs       Amount of time to allocate to the
 *                             Dispatcher during calls to
 *                             OI_StackConsole_GetInput() and
 *                             OI_Wrapper_WaitForEvent().
 */
OI_STATUS OI_OBEX_Init(OI_UINT32           idleSleepMsecs);

/**
 * This function is an alternative to OI_Wrapper_Init() which provides the same functionality.
 * This function allows the application to customize the core
 * stack and profile configurations by providing a customization
 * callback. The customization callback will be invoked during
 * the initialization process before the core stack components
 * are initialized. The customization callback then has the
 * opportunity to specify the module configuration using
 * OI_CONFIG_TABLE_PUT(). Refer to file oi_config_table.h for
 * API descriptions.
 *
 *  - OI_Wrapper_Init_Custom $mdash; Legacy Synchronously
 *   Only form.
 *
 * @param initComplete         Function to call upon completion of stack
 *                             initialization. If NULL is
 *                             passed, then the call is
 *                             completed synchronously.
 *
 * @param idleSleepMsecs       Amount of time to allocate to the
 *                             Dispatcher during calls to
 *                             OI_StackConsole_GetInput() and
 *                             OI_Wrapper_WaitForEvent()
 *
 * @param customCB  Callback to be invoked at the appropriate
 *                  point during initialization.
 *
 */
OI_STATUS OI_Wrapper_Init_Custom(OI_UINT32 idleSleepMsecs,
                                     void (*customCB)(void));

/**
 * This function enters device-under-test mode when called, and is an alternative to both
 * OI_Wrapper_Init() and OI_Wrapper_Init_Custom().
 * This function does not have a return value.
 */
void OI_Wrapper_Init_DUT(void);

/**
 * A thread attempts to get the stack access token by calling this function.
 * This function must be called before calling into the protocol
 * stack. This function may be blocked if the token is not
 * available. When this function returns, the caller owns the
 * stack token and may call into the stack. It is incumbent on
 * the caller to release the stack token when it is no longer
 * needed.
 */
void    OI_Wrapper_GetToken(void);

/**
 * A thread releases the stack access token by calling this function.
 * This function returns the stack token. When this function returns to the caller,
 * the caller no longer owns the token and the caller may not call into the stack
 * without re-acquiring the token.
 */
void    OI_Wrapper_ReleaseToken(void);

/**
 * Wait for event.
 *   This function blocks until the indicated event is signaled or until the timeout
 *   (if any) occurs. This function must be called without the stack access token.
 *   Because this is a blocking call, it must not be called from within a BLUEmagic
 *   callback routine.
 *
 *   @param pEventFlag       Pointer to a Boolean value to which
 *                           OI_Wrapper_SignalEvent() also
 *                           has a pointer, allowing the wait
 *                           function to test whether the event
 *                           has occurred
 *   @param timeoutSeconds   Maximum time to wait on the event
 *                           (Zero indicates no timeout.)
 *   @return                 OI_OK if event occured; OI_TIMEOUT if timed out
 */
OI_STATUS OI_Wrapper_WaitForEvent(OI_BOOL *pEventFlag, OI_UINT timeoutSeconds) ;


/**
 * Signal event.
 *   This function signals that the event for which OI_Wrapper_WaitForEvent() is waiting
 *   has occurred. This function may be called at any time, from any context.
 *
 *   @param pEventFlag    Pointer to a Boolean value to which
 *                        OI_Wrapper_WaitForEvent() also
 *                        has a pointer, allowing the signal
 *                        function to signal whether the event
 *                        has occurred
 *   @return              OI_OK always
 */
OI_STATUS OI_Wrapper_SignalEvent (OI_BOOL *pEventFlag) ;


/**
 * Deinitializes the stack wrapper and cleans up all resources.
 * Because this function is intended to allow BLUEmagic 3.0
 * software to clean up after itself, this function must be
 * called without the stack access token.
 */
void OI_OBEX_Deinit(void) ;

#ifdef __cplusplus
}
#endif

/*****************************************************************************/
#endif /* _OI_WRAPPER_H */
