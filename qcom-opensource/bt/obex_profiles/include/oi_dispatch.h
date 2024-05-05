#ifndef _DISPATCH_H
#define _DISPATCH_H

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
 * This header file provides the interface with the CThru Dispatcher, the
 * scheduler for callback functions.
 *
 * The aynschronous CThru  architecture of the core BLUEmagic 3.0 stack
 * ensures that no API call into the core stack will ever block. All calls into
 * the stack take the form of a request for some action to be performed and pass
 * in a callback function that is called when the action completes. The
 * CThru Dispatcher's job is to invoke the callback functions at the appropriate time.
 *
 * Depending on the host platform the Dispatcher may be called periodically from
 * a worker thread or, in a single-threaded environment, the Dispatcher may be called
 * explicitly by an application.
 *
 * Callback functions may be registered with the Dispatcher to be called
 * immediately or to be called at some future time. The Dispatcher deregisters
 * callback functions immediately after calling them. Callback functions that
 * re-register themselves will not be called again until the Dispatcher is
 * called again.
 *
 * The Dispatcher returns a callback handle, a 32-bit value that uniquely
 * identifies the callback registration. This handle can be used later to cancel
 * the callback or to inquire if the callback has been called.
 *
 * For more information see @ref dispatch_docpage.
 */

#include "oi_common.h"
#include "oi_time.h"

/** \addtogroup Dispatcher_Internal */
/**@{*/

#ifdef __cplusplus
extern "C" {
#endif


/**
 * struct for describing an argument block used when registering a callback
 * function with the Dispatcher and later when the callback function is called
 * by the Dispatcher
 */

typedef struct {
    OI_INT16 size;     /**< Size of the argument block */
    const void *data;  /**< Pointer to the argument block */
} DISPATCH_ARG;


/* Two of the dispatcher functions have had their prototypes moved to the SDK header
    file oi_bt_stack_init.h because they need to be visible to applications.  We don't want this
    file visible because we don't applications calling any other dispatcher functions.

*/

/**
 * convenience macro to set a value in a DISPATCH_ARG struct.
 *
 * @param arg is a DISPATCH_ARG argument block
 *
 * @param val is the value to set in the argument block
 */

#define Dispatch_SetArg(arg, val) { (arg).data = &(val); (arg).size = sizeof(val); }


/**
 * convenience macro to get a value from a pointer to an arg struct given a
 * type
 *
 * @param argp is a pointer to a DISPATCH_ARG argument block.
 *
 * @param type is the target type into which to cast the data block
 */

#define Dispatch_GetArg(argp, type) *((type*) (argp)->data)

/**
 * convenience macro to get a pointer value from a pointer to an arg struct given a
 * type
 *
 * @param argp is a pointer to a DISPATCH_ARG argument block.
 *
 * @param type is the target type into which to cast the data block
 */

#define Dispatch_GetPointerFromArg(argp, type) ((type*) (argp)->data)

/**
 * This is a unique identifier returned by OI_Dispatch_RegisterFunc() and
 * OI_Dispatch_RegisterTimedFunc() that can be used to cancel a callback, test if a
 * callback function has been called, or change the timeout interval on a timed callback.
 */

typedef OI_UINT32 DISPATCH_CB_HANDLE;


/**
 * typedef for function profiles for callback functions registered and called
 * by the Dispatcher
 */

typedef void (*DISPATCH_CB_FUNC)(DISPATCH_ARG *arg);


/**
 * This function registers a callback function with the Dispatcher. The Dispatcher makes a
 * copy of the argument data, so the caller can free memory allocated for the
 * arguments after registering the callback.
 *
 * @param CBFunction is the callback function that will be called the next time
 * OI_Dispatch_Run() is called by the application.
 *
 * @param args is a pointer to the arguments structure that will be passed to
 * the callback function when it is called.
 *
 * @param CBHandle (OUT) If this input parameter is not NULL, the value  pointed to by the pointer
 * CBHandle will be assigned a unique 32-bit identifier handle that the caller can later use to
 * cancel the registration; if the input value of CBHandle is NULL, no callback handle is assigned.
 *
 * @return an error status if the Dispatcher has insufficient resources to
 * register the new callback function.
 */

OI_STATUS OI_Dispatch_RegisterFunc(DISPATCH_CB_FUNC    CBFunction,
                                   DISPATCH_ARG        *args,
                                   DISPATCH_CB_HANDLE  *CBHandle);


/**
 * This function registers a timed callback function with the Dispatcher. The
 * Dispatcher will make a copy of the argument data, so the caller is expected
 * to free any memory allocated for the arguments after registering the callback.
 *
 * The Dispatcher checks the timeout value of each timed callback function
 * whenever OI_Dispatch_Run() is called. Timer callbacks with expired timeouts are
 * called in the same way as those registered by calling OI_Dispatch_RegisterFunc().
 *
 * @param CBFunction is the callback function that will be called the next time
 * OI_Dispatch_Run() is called by the application after the timeout interval has expired.
 *
 * @param args is a pointer to the arguments structure that will be passed to
 * the callback function when it is called.
 *
 * @param timeout is a timeout interval specified in tenths of a second. A timeout value
 * of zero means zero time - the effect is the same as calling OI_Dispatch_RegisterFunc().
 * Timeouts may be in the range 0 - OI_MAX_INTERVAL intervals (max timeout 54 min 36.7 sec).
 *
 * @param CBHandle (OUT) If this input parameter is not NULL, the value  pointed to by the pointer
 * CBHandle  will be assigned a unique 32-bit identifier handle that the caller can
 * later use to cancel the registration or to change to timeout value by calling
 * OI_Dispatch_SetFuncTimeout(); if the input value of CBHandle is NULL, no callback handle is assigned.
 *
 * @return an error status if the Dispatcher has insufficient resources to
 * register the new callback function.
 */

OI_STATUS OI_Dispatch_RegisterTimedFunc(DISPATCH_CB_FUNC    CBFunction,
                                        DISPATCH_ARG        *args,
                                        OI_INTERVAL         timeout,
                                        DISPATCH_CB_HANDLE  *CBHandle);

/**
 * Checks that the callback handle is valid and, if so, updates the timeout value
 * on the callback function. The new timeout value replaces the previous timeout
 * value.
 *
 * @param CBHandle is a handle to a previously registered callback function.
 *
 * @param timeout is a timeout interval specified in tenths of a second.
 * Setting the timeout interval to zero results in the function being called the
 * next time the Dispatcher runs.
 *
 * @note A timeout can be set on any registered callback whether the callback was
 * originally registered as a timed function or not.
 *
 * @return an error status if CBHandle is invalid.
 */

OI_STATUS OI_Dispatch_SetFuncTimeout(DISPATCH_CB_HANDLE CBHandle,
                                     OI_INTERVAL        timeout);


/**
 * This function suspends a callback function by marking it as not callable. A callback is
 * un-suspened by calling OI_Dispatch_SetFuncTimeout() or OI_Dispatch_CancelFunc() or
 * OI_Dispatch_CallFunc().
 *
 * @param CBHandle This callback handle parameter identifies which callback
 *                 function is to be suspended.
 *
 * @return The OI_OK status code is returned if the callback function was
 *         successsfully suspended.  An error status code is returned if the
 *         callback handle does not identify a currently registered callback
 *         function.
 */

OI_STATUS OI_Dispatch_SuspendFunc(DISPATCH_CB_HANDLE CBHandle);


/**
 * This function de-registers a callback function and deletes any memory
 * allocated for the callback function's arguments.
 *
 * @param CBHandle This callback handle parameter identifies which callback
 *                 function is to be deregistered.
 *
 * @return The OI_OK status code is returned if the callback function was
 *         successsfully canceled.  An error status code is returned if the
 *         callback handle does not identify a currently registered callback
 *         function.
 */

OI_STATUS OI_Dispatch_CancelFunc(DISPATCH_CB_HANDLE CBHandle);


/**
 * This function removes a callback function from the dispatcher and calls it
 * immediately on the caller's thread. After this call the handle will no
 * longer be valid.
 *
 * @param CBHandle This callback handle parameter identifies which callback
 *                 function is to be called.
 *
 * @return The OI_OK status code is returned if the callback function was
 *         successsfully called.  An error status code is returned if the
 *         callback handle does not identify a currently registered callback
 *         function.
 */

OI_STATUS OI_Dispatch_CallFunc(DISPATCH_CB_HANDLE CBHandle);

/**
 * This function checks whether a callback handle is valid. A callback handle is only valid
 * if it corresponds to a registered callback function that has not been called.
 *
 * @param CBHandle is a handle to a previously registered callback function.
 *
 * @return a boolean value that indicates whether the callback associated with a
 * callback handle is currently registered with the Dispatcher.
 */

OI_BOOL OI_Dispatch_IsValidHandle(DISPATCH_CB_HANDLE CBHandle);

/*
 * OI_Dispatch_Immediate_Run
 *
 * Calls and clears all callback functions that were dispatched with a timeout
 * of Zero. This allows the HCI layer to verify that all processing associated
 * with an incoming HCI packet has been handled prior to handling any new
 * Baseband or Application level events.
 */

void OI_Dispatch_Immediate_Run(void);



/*****************************************************************************/
#ifdef __cplusplus
}
#endif

/**@}*/

#endif /* _DISPATCH_H */

