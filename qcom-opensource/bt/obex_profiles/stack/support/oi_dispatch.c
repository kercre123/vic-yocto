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
@file
@internal
This file handles deferred callbacks.
*/

#define __OI_MODULE__ OI_MODULE_DISPATCH

#include "oi_assert.h"
#include "oi_argcheck.h"
#include "oi_dispatch.h"
#include "oi_memmgr.h"
#include "oi_time.h"
#include "oi_osinterface.h"
#include "oi_debug.h"
#include "oi_init_flags.h"
#include "oi_config_table.h"
#include "oi_bt_stack_init.h"
#include "oi_utils.h"
#include "oi_bt_module_init.h"
#include "oi_dump.h"



/**
 * Space allocated directly in the dispatch table for storing copies of
 * arguments to be passed to the callback functions.
 * mallocArg memory is allocated to store arguments larger the this size.
 */

#define MAX_INLINE_ARG_SIZE   8

typedef enum {
    FREE_SLOT = 0,
    FUNC_RUNNING,    /** Function is running and the handle is no longer valid */
    FUNC_ACTIVE,     /** Function is registered and active */
    FUNC_RUNNABLE,   /** Function is read to run and will be running soon */
    FUNC_SUSPENDED   /** Function is registered and suspended */
} _ENTRY_STATE;

#ifdef OI_DEBUG
typedef _ENTRY_STATE ENTRY_STATE;
#else
typedef OI_UINT8 ENTRY_STATE;
#endif

/**
 * Type definition for entries in the dispatch table
 *
 * A dispatch table entry holds a pointer to a callback function, and arg struct
 * to pass to the callback function and if this is a timed callback, a timeout
 * interval.
 *
 * The dispatcher always makes a copy of the argument data. The copy is stored
 * inlineArg in the dispatch table if there is room, otherwise into dynamically
 * allocated memory.
 */

typedef struct {
    ENTRY_STATE      state;
    OI_INTERVAL      timeout;

    /*
     * Used to generate unique callback handles.
     */
    OI_UINT16        salt;
    OI_UINT16        argSize;
    DISPATCH_CB_FUNC CBFunc;
    union {
        OI_BYTE      *mallocArg;
        OI_BYTE      inlineArg[MAX_INLINE_ARG_SIZE];
    } arg;
} DISPATCH_ENTRY;


typedef struct {
    /*
     * DispatchRun must not be called recursively.
     */
    OI_UINT8 Running;

    /*
     * Flag Dispatches registered with a Zero Time-out as high priority, which
     * will be handled prior to any new incoming BT traffic.
     */
    OI_UINT8 ImmediateDispatch;

    /*
     * MaxUsedEntry is the high water mark for the dispatch table.
     */
    OI_INT16 MaxUsedEntry;

    /*
     * The number of entries that are active (i.e. not suspended)
     */
    OI_INT16 numActiveEntries;

    /*
     * The dispatch table is a statically sized array of dispatch entries
     */
    OI_UINT16 TableSize;

    /*
     * The last requested service timeout. (Timestamp of next-to-fire timer)
     */
    OI_INTERVAL nextTimeout;

    /*
     * The table of registered dispatch entries.
     */
    DISPATCH_ENTRY *Table;

    /*
     * The callback for service notifications.
     */
    OI_DISPATCH_SERVICE_REQUEST_HANDLER serviceCallback;

} DISPATCH_STATE;


/*
 * A callback handle is a 32 bit value that encodes the callback entry index
 * along with a unique salt.
 */
#define CB_INDEX(h)  ((OI_UINT16) ((h) >> 16))                   // Top 16 bits are the entry index
#define CB_SALT(h)   ((OI_UINT32) (((OI_UINT32) (h)) & 0xFFFF))  // Bottom 16 bits are the salt

/*
 * Generate a callback handle from an entry index.
 */
#define CB_HANDLE(i, s) ((DISPATCH_CB_HANDLE) ((((OI_UINT32) (i)) << 16) | CB_SALT(s)))


#define ENTRY_IN_USE(e)  ((e)->CBFunc != NULL)


/*
 * For debugging/calibration purposes, track high water mark
 */
#ifdef OI_DEBUG
    static OI_INT max_used_count;
#endif

/*
 * Pointer to the statically allocated dispatch state information
 */
static DISPATCH_STATE Dispatcher;

#ifdef EVAL

#define TIMEBOMB_DURATION OI_MINUTES(5)

/**
 * The handler for the timebomb timeout.  Print out an error message and generate a FatalError.
 * This code is only defined when EVAL is defined.
 */
static void TimebombHandler(DISPATCH_ARG *arg)
{
    OI_Print("****************************************************************************\n");
    OI_Print("* Thank you for using this evaluation version of the BLUEmagic 3.0 Bluetooth\n");
    OI_Print("*\n");
    OI_Print("****************************************************************************\n");
    OI_FatalError(OI_TIMEOUT);
}
#endif


/*
 * OI_Dispatch_Init
 *
 * Clears the dispatch table
 */
OI_STATUS OI_Dispatch_Init(OI_DISPATCH_SERVICE_REQUEST_HANDLER handler)
{
    if (OI_INIT_FLAG_VALUE(DISPATCH)) {
        OI_SLOG_ERROR(OI_STATUS_ALREADY_INITIALIZED, ("OI_Dispatch_Init"));
        return OI_STATUS_ALREADY_INITIALIZED;
    }

    // we require real configuration
    OI_CONFIG_DISPATCH *pConfigDispatch = OI_CONFIG_TABLE_GET(DISPATCH);
    OI_ASSERT(NULL != pConfigDispatch);

    OI_MemZero(&Dispatcher, sizeof(Dispatcher));

    Dispatcher.TableSize = OI_CONFIG_TABLE_GET(DISPATCH)->DispatchTableSize;
    Dispatcher.Table = (DISPATCH_ENTRY*) OI_StaticMalloc(sizeof(DISPATCH_ENTRY) * Dispatcher.TableSize);
    Dispatcher.MaxUsedEntry = -1;
    Dispatcher.serviceCallback = handler;
#ifdef OI_DEBUG
    max_used_count = -1;
#endif

    OI_INIT_FLAG_PUT_FLAG(TRUE, DISPATCH);

#ifdef EVAL
    /*
     * In the EVAL version print out an initial announcement.
     */
    OI_Print("\n");
    OI_Print("*******************************************************************\n");
    OI_Print("This version of BLUEmagic 3.0 is time limited, for evaluation only.\n");
    OI_Print(OI_Copyright());
    OI_Print("\n");
    OI_Print("*******************************************************************\n");
    OI_Print("\n");

    /*
     * In the EVAL version, register a timebomb.
     */
    OI_Dispatch_RegisterTimedFunc(TimebombHandler, NULL, TIMEBOMB_DURATION, NULL);
#endif

    return OI_OK;
}


/*
 * FreeCBEntry
 *
 * Frees any dynamically allocated memory associated with the callback entry and
 * marks the entry as unused.
 */

static void FreeCBEntry(OI_INT16 entryIndex)
{
    DISPATCH_ENTRY *entry = &(Dispatcher.Table[entryIndex]);

    OI_DBGPRINT2(("FreeCBEntry(%d)  handle: %x  TO: %d\n",
                  entryIndex, CB_HANDLE(entryIndex, entry->salt), entry->timeout));

    /*
     * Free any dynamically allocated memory
     */
    if (ENTRY_IN_USE(entry) && (entry->argSize > MAX_INLINE_ARG_SIZE)) {
        if (entry->arg.mallocArg != NULL) {
            OI_Free(entry->arg.mallocArg);
        }
    }
    OI_ASSERT(Dispatcher.numActiveEntries >= 0);
    /*
     * Adjust the active entries count
     */
    if (entry->state == FUNC_ACTIVE) {
        --Dispatcher.numActiveEntries;
    }
    /*
     * Flag that the entry is no longer in use
     */
    entry->CBFunc = NULL;
    entry->state = FREE_SLOT;

    /*
     * Adjust dispatcher high-water mark
     */
    if (entryIndex == Dispatcher.MaxUsedEntry) {
        while ((Dispatcher.MaxUsedEntry >= 0) && !ENTRY_IN_USE(&(Dispatcher.Table[Dispatcher.MaxUsedEntry]))) {
            --Dispatcher.MaxUsedEntry;
        }
    }
}


/*
 * OI_Dispatch_Terminate
 *
 * Clears any dynamically allocated callback argument data and clears
 * the dispatch table
 */

void OI_Dispatch_Terminate(void)
{
    OI_INT16 i;

    if (Dispatcher.Table != NULL) {
        /*
         * Frees up any registered callbacks
         */
        for (i = 0; i < Dispatcher.MaxUsedEntry; ++i) {
            FreeCBEntry(i);
        }
        Dispatcher.Table = NULL;
    }
}


/**
 * Request service, if required.
 */
static void Dispatch_RequestService(OI_INTERVAL timeout)
{
    if (NULL != Dispatcher.serviceCallback) {
        /*
         * convert OI_INTERVAL to milliseconds
         */
        Dispatcher.serviceCallback(OI_INTERVAL_TO_MILLISECONDS(timeout));
    }
}

/**
 * Free running interval timer based on OI_Time_Now
 */
static OI_INTERVAL Dispatch_timeNowInterval(void)
{
    OI_TIME     now;
    OI_INTERVAL ret_val;

    OI_Time_Now(&now);
    ret_val = OI_SECONDS(now.seconds);
    ret_val += OI_MSECONDS(now.mseconds);
    //OI_DBGPRINT2(("Dispatch_timeNowInterval: %d.%03d ==> %d\n", now.seconds, now.mseconds, ret_val));

    return ret_val;
}


/*
 * OI_Dispatch_RegisterFunc
 *
 * Registers a callback function in the dispatch table.
 */

OI_STATUS OI_Dispatch_RegisterFunc(DISPATCH_CB_FUNC CBFunction,
                                   DISPATCH_ARG *args,
                                   DISPATCH_CB_HANDLE *CBHandle)
{
    return OI_Dispatch_RegisterTimedFunc(CBFunction, args, 0, CBHandle);
}


/**
 * Helper routinte shared by internal and external functions to set the timeout
 * for a dispatcher entry.
 */
static void InternalSetTimeout(DISPATCH_ENTRY *entry,
                               OI_INTERVAL timeout)
{
    OI_INTERVAL intervalNow = Dispatch_timeNowInterval();
    OI_INT16 deltaTime = (OI_INT16)(Dispatcher.nextTimeout - intervalNow);

    OI_ASSERT(entry);

    entry->timeout = intervalNow + timeout;
    OI_DBGPRINT2(("InternalSetTimeout(%d), this TO: %d, old TO: %d, Cur time: %d\n",
                  timeout, entry->timeout, Dispatcher.nextTimeout, intervalNow));

    /*
     * Service request bookkeeping if this is the only or next entry needing dispatch.
     */
    if ((Dispatcher.numActiveEntries == 0) || (deltaTime >= timeout)) {
        Dispatcher.nextTimeout = entry->timeout;
        //OI_DBGPRINT(("Dispatch_RequestService (%d)\n", timeout));
        Dispatch_RequestService(timeout);
    }
    /*
     * If entry was suspended it is now active
     */
    if (entry->state == FUNC_SUSPENDED) {
        OI_DBGPRINT2(("Activating suspended entry %x numActiveEntries=%d", entry, Dispatcher.numActiveEntries));
        entry->state = FUNC_ACTIVE;
        ++Dispatcher.numActiveEntries;
    };

}


/*
 * OI_Dispatch_SetFuncTimeout
 *
 * Checks that the callback handle is valid, and if so updates the timeout value
 * on the callback function.
 */

OI_STATUS OI_Dispatch_SetFuncTimeout(DISPATCH_CB_HANDLE CBHandle,
                                     OI_INTERVAL        timeout)
{
    if (!OI_Dispatch_IsValidHandle(CBHandle)) {
        OI_DBGPRINT(("OI_Dispatch_SetFuncTimeout %!", OI_DISPATCH_INVALID_CB_HANDLE));
        return OI_DISPATCH_INVALID_CB_HANDLE;
    }

    OI_DBGPRINT(("OI_Dispatch_SetFuncTimeout(%d)  entry: %d  handle: %x  old TO: %d\n",
                 timeout, CB_INDEX(CBHandle), CBHandle, Dispatcher.Table[CB_INDEX(CBHandle)].timeout));


    OI_ASSERT(timeout <= OI_MAX_INTERVAL);

    if (timeout <= OI_MAX_INTERVAL) {
        InternalSetTimeout(&Dispatcher.Table[CB_INDEX(CBHandle)], timeout);
        return OI_STATUS_SUCCESS;
    } else {
        OI_SLOG_ERROR(OI_STATUS_INVALID_PARAMETERS, ("timeout > OI_MAX_INTERVAL"));
        return OI_STATUS_INVALID_PARAMETERS;
    }
}


/*
 * OI_Dispatch_SuspendFunc
 *
 * Checks that the callback handle is valid, and if so updates the timeout value
 * on the callback function.
 */

OI_STATUS OI_Dispatch_SuspendFunc(DISPATCH_CB_HANDLE CBHandle)
{
    DISPATCH_ENTRY *entry;

    if (!OI_Dispatch_IsValidHandle(CBHandle)) {
        OI_DBGPRINT(("OI_Dispatch_SuspendFunc %!", OI_DISPATCH_INVALID_CB_HANDLE));
        return OI_DISPATCH_INVALID_CB_HANDLE;
    }

    OI_DBGPRINT(("OI_Dispatch_SuspendFunc entry: %d  handle: %x", CB_INDEX(CBHandle), CBHandle));

    entry = &Dispatcher.Table[CB_INDEX(CBHandle)];
    if (entry->state == FUNC_ACTIVE) {
        entry->state = FUNC_SUSPENDED;
        OI_ASSERT(Dispatcher.numActiveEntries > 0);
        OI_DBGPRINT2(("Suspending entry %x numActiveEntries=%d", entry, Dispatcher.numActiveEntries));
        --Dispatcher.numActiveEntries;
    }
    return OI_OK;
}


/*
 * For storing information for an application callback registered by the public API OI_ScheduleCallbackFunction().
 */
typedef struct {
    void *arg;
    OI_SCHEDULED_CALLBACK cb;
} APPLICATION_CB;


static void AppCallback(DISPATCH_ARG *arg)
{
    APPLICATION_CB appCb = Dispatch_GetArg(arg, APPLICATION_CB);

    appCb.cb(appCb.arg);
}


/*
 * Public (SDK) interface for registering a timeout callback.
 */
OI_STATUS OI_ScheduleCallbackFunction(OI_SCHEDULED_CALLBACK callbackFunction,
                                      void                 *arg,
                                      OI_INTERVAL           timeout,
                                      OI_CALLBACK_HANDLE   *handle)
{
    APPLICATION_CB appCb;
    DISPATCH_ARG darg;

    OI_ARGCHECK(NULL != callbackFunction);

    appCb.arg = arg;
    appCb.cb = callbackFunction;

    Dispatch_SetArg(darg, appCb);
    return OI_Dispatch_RegisterTimedFunc(AppCallback, &darg, timeout, handle);
}

OI_STATUS OI_CancelCallbackFunction(OI_CALLBACK_HANDLE handle)
{
    return OI_Dispatch_CancelFunc(handle);
}


/*
 * OI_Dispatch_RegisterTimedFunc
 *
 * Calls OI_Dispatch_RegisterFunc to create the callback entry and then initializes
 * the callback timer
 */

OI_STATUS OI_Dispatch_RegisterTimedFunc(DISPATCH_CB_FUNC   CBFunction,
                                        DISPATCH_ARG       *args,
                                        OI_INTERVAL        timeout,
                                        DISPATCH_CB_HANDLE *CBHandle)
{
    OI_INT16 i;
    DISPATCH_ENTRY *entry = NULL;
    DISPATCH_CB_HANDLE handle = 0;

    OI_ASSERT(Dispatcher.Table != NULL);
    OI_ASSERT(CBFunction != NULL);
    OI_ASSERT(timeout <= OI_MAX_INTERVAL);

    OI_DBGPRINT(("OI_Dispatch_RegisterTimedFunc(%d)\n", timeout));


    /*
     * Dispatcher now has a public interface, so enable validity checks in
     * release mode.
     */
    if ((timeout > OI_MAX_INTERVAL) ||
        (CBFunction == NULL) ||
        (Dispatcher.Table == NULL)) {
        OI_SLOG_ERROR(OI_STATUS_INVALID_PARAMETERS, ("OI_Dispatch_RegisterTimedFunc"));
        return OI_STATUS_INVALID_PARAMETERS;
    }

    /*
     * Zero time-out registered functions are "High Priority" and must be run
     * prior to the handling of any new incoming BT traffic. Note that this
     * treatment is explicitly *not* extended to functions with > Zero timeouts
     * that subsequently reach Zero.
     */
    if (timeout == 0) {
        Dispatcher.ImmediateDispatch = TRUE;
    }

    /*
     * Find the first free entry in the dispatch table.
     */
    for (i = 0; i < Dispatcher.TableSize; ++i) {
        entry = &Dispatcher.Table[i];
        if (!ENTRY_IN_USE(entry)) {

            /*
             * We avoid checking the entire dispatch table for callbacks if we
             * know where the last entry is.
             */
            if (i > Dispatcher.MaxUsedEntry) {
                Dispatcher.MaxUsedEntry = i;
#ifdef OI_DEBUG
                if (i > max_used_count){
                    OI_DBGPRINT(("Dispatcher high water: %d\n", i + 1));
                    max_used_count = i;
                }
#endif
            }

            InternalSetTimeout(entry, timeout);
            entry->state = FUNC_ACTIVE;
            ++Dispatcher.numActiveEntries;

            /*
             * Set the callback function, this also flags the entry as used
             */
            entry->CBFunc = CBFunction;

            /*
             * Create a unique callback handle from the index and a salt.  The
             * index can be recovered using the CB_INDEX macro. Salt should
             * never be Zero, or a DISPATCH_CB_HANDLE of Zero is possible,
             * making handle intitializatons to Zero untenable.
             */
            if ((++entry->salt) == 0){
                OI_DBGPRINT(("Salt of Zero reached - index: %d\n", i));
                ++entry->salt;
            }

            handle = CB_HANDLE(i, entry->salt);

            OI_DBGPRINT2(("Function added to dispatcher - entry: %d  handle: %x  TO: %d\n",
                          i, handle, entry->timeout));

            break;
        }
    }

    OI_ASSERT(i < Dispatcher.TableSize);
    if (i == Dispatcher.TableSize) {
        OI_SLOG_ERROR(OI_DISPATCH_TABLE_OVERFLOW, ("OI_Dispatch_RegisterTimedFunc"));
        return OI_DISPATCH_TABLE_OVERFLOW;
    }

    if (args == NULL) {
        entry->argSize = 0;
    } else {
        /*
         * The dispatcher makes a copy of the callback arguments.
         */
        if (args->size <= MAX_INLINE_ARG_SIZE) {
            /*
             * Copy the callback arguments directly into the callback entry.
             */
            for (i = 0; i < args->size; ++i) {
                entry->arg.inlineArg[i] = ((OI_BYTE*) args->data)[i];
            }
        } else {
            /*
             * Malloc memory for the callback arguments.
             */
            if ((entry->arg.mallocArg = OI_Malloc(args->size)) == NULL) {
                FreeCBEntry(i);
                OI_SLOG_ERROR(OI_STATUS_OUT_OF_MEMORY, ("OI_Malloc %d", args->size));
                return OI_STATUS_OUT_OF_MEMORY;
            }
            OI_MemCopy(entry->arg.mallocArg, args->data, args->size);
        }
        entry->argSize = args->size;
    }

    if (CBHandle != NULL) {
        *CBHandle = handle;
    }

    return OI_STATUS_SUCCESS;
}




/*
 * DispatchCancelFunc
 *
 * Checks that the callback handle is valid and if so, deletes the associated
 * callback function from the dispatch table.
 */

OI_STATUS OI_Dispatch_CancelFunc(DISPATCH_CB_HANDLE CBHandle)
{
    if (!OI_Dispatch_IsValidHandle(CBHandle)) {
        OI_DBGPRINT(("OI_Dispatch_CancelFunc %!", OI_DISPATCH_INVALID_CB_HANDLE));
        return OI_DISPATCH_INVALID_CB_HANDLE;
    } else {
        OI_DBGPRINT2(("Cancelling dispatcher function (entry %d  handle: %x  TO: %d)\n",
                      CB_INDEX(CBHandle), CBHandle, Dispatcher.Table[CB_INDEX(CBHandle)].timeout));
        FreeCBEntry(CB_INDEX(CBHandle));
        return OI_STATUS_SUCCESS;
    }
}

/*
 * Remove this function from the dispatch queue and call it immediately.
 */
OI_STATUS OI_Dispatch_CallFunc(DISPATCH_CB_HANDLE CBHandle)
{
    DISPATCH_ARG arg;
    DISPATCH_ENTRY *entry;

    if (!OI_Dispatch_IsValidHandle(CBHandle)) {
        OI_SLOG_ERROR(OI_DISPATCH_INVALID_CB_HANDLE, ("OI_Dispatch_CallFunc"));
        return OI_DISPATCH_INVALID_CB_HANDLE;
    }
    entry = &(Dispatcher.Table[CB_INDEX(CBHandle)]);
    if (entry->state == FUNC_ACTIVE) {
        /*
         * This entry is no longer active
         */
        --Dispatcher.numActiveEntries;
    }
    /*
     * Mark that the function is now running so that the handle is no longer valid.
     */
    entry->state = FUNC_RUNNING;
    OI_DBGPRINT2(("Force call dispatcher function (entry: %d  handle: %x  TO: %d  current time: %d)\n",
                  CB_INDEX(CBHandle), CBHandle, entry->timeout, Dispatch_timeNowInterval()));
    if (entry->argSize == 0) {
        entry->CBFunc(NULL);
    } else {
        arg.size = entry->argSize;
        if (arg.size <= MAX_INLINE_ARG_SIZE) {
            arg.data = entry->arg.inlineArg;
        } else {
            arg.data = entry->arg.mallocArg;
        }
        entry->CBFunc(&arg);
    }
    FreeCBEntry(CB_INDEX(CBHandle));
    return OI_STATUS_SUCCESS;
}

/*
 * DispatchIsValidHandle
 *
 * Checks if this is a valid dispatch function handle.
 */

OI_BOOL OI_Dispatch_IsValidHandle(DISPATCH_CB_HANDLE CBHandle)
{
    OI_UINT16 index = CB_INDEX(CBHandle);

    OI_ASSERT(Dispatcher.Table != NULL);

    /*
     * Check index is in range
     */
    if (index >= Dispatcher.TableSize) {
        return FALSE;
    }
    /*
     * Check that there is a function registered in this table entry and the
     * function is not already running.
     */
    if ((Dispatcher.Table[index].state == FREE_SLOT) || (Dispatcher.Table[index].state == FUNC_RUNNING)) {
        return FALSE;
    }
    /*
     * Check the salt in the CBHandle is what we expect
     */
    return CB_SALT(Dispatcher.Table[index].salt) == CB_SALT(CBHandle);
}


/*
 * OI_Dispatch_Run
 *
 * Invokes currently registered callbacks that are runnable now.
 *
 * Callbacks that are registered or become runnable while OI_Dispatch_Run is
 * executing do not get called until next time.
 */

OI_INT16 OI_Dispatch_Run()
{
    OI_INT16 i;
    OI_INT16 calls = 0;
    DISPATCH_ARG arg;
    DISPATCH_ENTRY *entry;
    OI_INTERVAL smallestTimeout;
    OI_INTERVAL intervalNow;

    //OI_DBGPRINT2(("OI_Dispatch_Run"));

    /*
     * Silently ignore Run requests if the dispatcher has not been initialized or has been
     * terminated.
     */
    if (Dispatcher.Table == NULL) {
        return 0;
    }

    OI_ASSERT(!Dispatcher.Running);

    Dispatcher.Running = TRUE;


    /*
     * Request free-running interval time to test against registered timers.
     */
    intervalNow = Dispatch_timeNowInterval();

    /*
     * First pass over the list decrements the timeouts to figure out which
     * callbacks are runnable
     */
    for (i = 0; i <= Dispatcher.MaxUsedEntry; ++i) {
        entry = &Dispatcher.Table[i];
        if (ENTRY_IN_USE(entry) && (entry->state != FUNC_SUSPENDED)) {
            if (((OI_INT16)(entry->timeout - intervalNow)) <= 0) {
#ifdef OI_DEBUG
                if (((OI_INT16)(entry->timeout - intervalNow)) < -1) {
                    OI_SLOG_ERROR(OI_STATUS_NONE, ("Dispatcher function called late (entry: %d  handle %x  TO: %d  currTime: %d)\n",
                                                   i, CB_HANDLE(i, entry->salt), entry->timeout, intervalNow));
                }
#endif
                if (entry->state == FUNC_ACTIVE) {
                    /*
                     * This entry is no longer active
                     */
                    --Dispatcher.numActiveEntries;
                }
                entry->state = FUNC_RUNNABLE;
                calls++;
            }
        }
    }

    /*
     * Second passed over the list invokes callbacks that are runnable
     */
    if (calls) {
        OI_DBGPRINT2(("Dispatcher Running at %d\n", intervalNow));
        for (i = 0; i <= Dispatcher.MaxUsedEntry; ++i) {
            entry = &Dispatcher.Table[i];
            if (entry->state == FUNC_RUNNABLE) {
                /*
                 * Mark that the function is now running so that the handle is no longer valid.
                 */
                entry->state = FUNC_RUNNING;
                /*
                 * Call the callback function then free the entry
                 */
                OI_DBGPRINT2(("Calling dispatcher function (entry: %d  handle: %x  TO: %d\n",
                              i, CB_HANDLE(i, entry->salt), entry->timeout));
                if (entry->argSize == 0) {
                    entry->CBFunc(NULL);
                } else {
                    arg.size = entry->argSize;
                    if (arg.size <= MAX_INLINE_ARG_SIZE) {
                        arg.data = entry->arg.inlineArg;
                    } else {
                        arg.data = entry->arg.mallocArg;
                    }
                    entry->CBFunc(&arg);
                }
                FreeCBEntry(i);
            }
        }
    }

    /*
     * Find the pending task with the smallest timeout so know when
     * next to request service. We have to do this check after all current
     * tasks have completed.
     */
    smallestTimeout = (OI_INTERVAL)-1;
    for (i = 0; i <= Dispatcher.MaxUsedEntry; ++i) {
        entry = &Dispatcher.Table[i];
        if (ENTRY_IN_USE(entry) && (entry->state != FUNC_SUSPENDED)) {
            OI_INTERVAL timeout = entry->timeout - intervalNow;
            if (timeout < smallestTimeout) {
                smallestTimeout = timeout;
                Dispatcher.nextTimeout = entry->timeout;
            }
        }
    }

    /*
     * If there are no tasks that will become runnable in the near future there
     * is no need to callback to request service. Setting a timer to
     * OI_MAX_INTERVAL effectively puts the task in statis.
     */
    if (smallestTimeout < (OI_INTERVAL)-1) {
        //OI_DBGPRINT2(("Dispatch_RequestService (%d)\n", smallestTimeout));
        Dispatch_RequestService(smallestTimeout);
    }
    else {
        //OI_DBGPRINT2(("No service request: %d %d %d\n", smallestTimeout, OI_MAX_INTERVAL, Dispatcher.MaxUsedEntry));
    }
    Dispatcher.Running = FALSE;

    /*
     * If dispatch table has no ready to run callbacks, then by inference there
     * are no ImmediateDispatch functions.
     */
    if (calls == 0) {
        Dispatcher.ImmediateDispatch = FALSE;
    }

    return calls;
}

/*
 * OI_Dispatch_Immediate_Run
 *
 * Invokes OI_Dispatch_Run if Immediate callbacks have
 * been registered since the previous run.
 *
 */

void OI_Dispatch_Immediate_Run()
{
    /*
     * It is possible that Dispatched functions may themselves request an
     * immediate dispatch, so the flag must be repeatedly cleared and re-checked
     * to ensure that all immediate callbacks have been handled.
     */
    if (!Dispatcher.Running) {
        while (Dispatcher.ImmediateDispatch) {
            Dispatcher.ImmediateDispatch = FALSE;
            OI_DBGPRINT2(("OI_Dispatch_Immediate_Run"));
            OI_Dispatch_Run();
        }
    }
}

/*
 * OI_DISPATCH_Dump
 *
 * Dump dispatcher internal state
 *
 */

#ifdef OI_DEBUG

static OI_CHAR* stateToText(ENTRY_STATE state)
{
    switch (state) {
        case FREE_SLOT:         return "FREE_SLOT";
        case FUNC_RUNNING:      return "FUNC_RUNNING";
        case FUNC_ACTIVE:       return "FUNC_ACTIVE";
        case FUNC_RUNNABLE:     return "FUNC_RUNNABLE";
        case FUNC_SUSPENDED:    return "FUNC_SUSPENDED";
    }
    return "unknown state";
}


void OI_DISPATCH_Dump(void)
{
    DISPATCH_ENTRY  *entry;
    OI_INT           i;
    OI_INTERVAL      intervalNow = Dispatch_timeNowInterval();

    OI_Printf("Dispatcher dump:\n");
    OI_Printf("   Running          %d\n",  Dispatcher.Running);
    OI_Printf("   TableSize        %d\n",  Dispatcher.TableSize);
    OI_Printf("   numActiveEntries %d\n",  Dispatcher.numActiveEntries);
    OI_Printf("   MaxUsedEntry     %d\n",  Dispatcher.MaxUsedEntry);
    OI_Printf("   nextTimeout      %d\n",  Dispatcher.nextTimeout);
    OI_Printf("   max_used_count   %d\n",  max_used_count);
    for (i = 0; i < Dispatcher.TableSize; ++i) {
        entry = &Dispatcher.Table[i];
        if (ENTRY_IN_USE(entry)) {
            OI_Printf("entry %2d: %18s timeout %d\n",
                      i, stateToText(entry->state), (entry->state == FUNC_SUSPENDED) ? 0 : entry->timeout - intervalNow);
        }
    }
}
#endif

/*****************************************************************************/

