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

/** @file
 * This file contains a platform-independent interface for
 * synchronization and thread creation.
 */

#ifndef _OI_THREAD_H
#define _OI_THREAD_H

#include "oi_thread_platform.h"
#include "oi_status.h"
#include "oi_time.h"

/** \addtogroup Misc Miscellaneous APIs */
/**@{*/

#ifdef __cplusplus
extern "C" {
#endif


/*** Mutexes ***/

/** Initializes a mutex.
 * @param mut   Mutex to initialize
 */
OI_BOOL OI_Mutex_Init(OI_MUTEX *mut);

/** Deinitializes a mutex.
 * @param mut   Mutex to be destroyed
 */
OI_BOOL OI_Mutex_Destroy(OI_MUTEX *mut);

/**
 * This function locks a mutex. If the calling thread already
 * owns the mutex, this function will return immediately.
 * @param mut   Mutex to lock
 */
OI_BOOL OI_Mutex_Lock(OI_MUTEX *mut);

/**
 * This function unlocks a mutex.
 * @param mut   Mutex to unlock
 */
OI_BOOL OI_Mutex_Unlock(OI_MUTEX *mut);

/*** Threads ***/

/**
 * Function prototype for a thread start routine
 */
typedef void* (*OI_THREAD_START_ROUTINE)(void *);


/**
 * This function creates a new thread.
 *
 * @param t             Pointer to a thread type
 * @param startRoutine  Start routine for the thread
 * @param arg           Argument passed to the start routine
 */
OI_BOOL OI_Thread_Create(OI_THREAD *t,
                         OI_THREAD_START_ROUTINE startRoutine,
                         void *arg);

/**
 * This function waits for the thread specified by thread
 *     to terminate
 *
 * @param t            thread type
 */
void OI_Thread_Join(OI_THREAD t);

/**
 * Exits the current thread
 *
 * @param retval  Return code for the thread
 */
void OI_Thread_Exit(void *retval);

/*** Condition variables ***/

/**
 * This function initializes a condition variable.
 *
 * @param cond Condition variable to initialize
 */
OI_BOOL OI_Cond_Init(OI_COND *cond);

/**
 * This function wakes a single thread waiting on the specified
 * condition variable.
 *
 * @param cond Condition variable on which the thread is waiting
 */
OI_BOOL OI_Cond_Signal(OI_COND *cond);

/**
 * This function wakes all threads waiting on the specified
 * condition variable.
 *
 * @param cond Condition variable on which the threads are
 *             waiting
 */
OI_BOOL OI_Cond_Broadcast(OI_COND *cond);

/**
 * This function waits for the specified condition variable to
 * become signalled. The mutex parameter must be locked before
 * calling this function.
 *
 * @param cond   Condition variable
 * @param mutex  Mutex
 */
OI_BOOL OI_Cond_Wait(OI_COND *cond,
                     OI_MUTEX *mutex);

/**
 * This function waits for the specified condition variable to
 * become signalled or until the specified time has been
 * exceeded. The mutex parameter must be locked before calling
 * this function.
 *
 * @param cond   Condition variable
 * @param mutex  Mutex
 * @param msec   Number of milliseconds to wait
 *
 * @return  OI_OK if signaled, OI_TIMEOUT if a timeout occured, or another value if an error ocurred
 */
OI_STATUS OI_Cond_TimedWait(OI_COND *cond,
                            OI_MUTEX *mutex,
                            OI_UINT32 msec);

/**
 * This function destroys the specified condition variable
 *
 * @param cond Condition variable to be destroyed
 */
OI_BOOL OI_Cond_Destroy(OI_COND *cond);

/**
 * This function returns a free-running 32-bit ms counter which
 * rolls over about once every 50 days. The count may not
 * represent any particular time, and should only be used to
 * measure time differences between two different calls.
 *
 * @return The ms count
 */
OI_UINT32 OI_Milliseconds(void);


/**
 * This function enters a critical region that must not be
 * interrupted. This function may be nested.
 *
 * @return Value to be passed to matching OI_ExitCritical()
 */
OI_UINT32 OI_EnterCritical(void);

/**
 * This function exits a critical region that must not be
 * interrupted. This function must be called to match a previous
 * OI_EnterCritical().
 *
 * @param saved_state  Restores the previous critical region
 *                      state prior to matching
 *                      OI_EnterCritical()
 */
void OI_ExitCritical(OI_UINT32 saved_state);


#ifdef __cplusplus
}
#endif

/**@}*/

#endif /* _OI_THREAD_H */

