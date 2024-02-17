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

#define __OI_MODULE__ OI_MODULE_SUPPORT

#include "oi_thread.h"
#include <pthread.h>
#include <time.h>
#include <errno.h>

OI_BOOL OI_Mutex_Init(OI_MUTEX *mutex)
{
    pthread_mutexattr_t attr;
    int retval;

    retval = pthread_mutexattr_init(&attr);

    if (retval == 0) {
        retval = pthread_mutexattr_settype(&attr, PTHREAD_MUTEX_RECURSIVE_NP);
        if (retval == 0) {
            retval = pthread_mutex_init(mutex, &attr);
        }
        pthread_mutexattr_destroy(&attr);
    }

    return (retval == 0);
}

/** Destroy a mutex */
OI_BOOL OI_Mutex_Destroy(OI_MUTEX *mutex)
{
    int retval;

    retval = pthread_mutex_destroy(mutex);
    return (retval == 0);
}

/** Lock a mutex */
OI_BOOL OI_Mutex_Lock(OI_MUTEX *mutex)
{
    int retval;

    retval = pthread_mutex_lock(mutex);
    return (retval == 0);
}

/** Unlock a mutex */
OI_BOOL OI_Mutex_Unlock(OI_MUTEX *mutex)
{
    int retval;

    retval = pthread_mutex_unlock(mutex);
    return (retval == 0);
}


OI_BOOL OI_Thread_Create(OI_THREAD *t,
                         void *(*startRoutine)(void *),
                         void *arg)
{
    int retval;

    retval = pthread_create(t, NULL, startRoutine, arg);
    if (retval != 0) {
        return FALSE;
    }
    else {
        return TRUE;
    }
}

void OI_Thread_Join(OI_THREAD t)
{
    pthread_join(t, NULL);

}

void OI_Thread_Exit(void *retval)
{
    pthread_exit(retval);

}

/**
 * Initialize a condition variable
 *
 * @param  the condition variable to initialize
 */
OI_BOOL OI_Cond_Init(OI_COND *cond)
{
    int ret;

    ret = pthread_cond_init(cond, NULL);

    if (0 == ret) {
        return TRUE;
    }
    else {
        return FALSE;
    }
}

/**
 * Wake a single thread waiting on the specified condition variable.
 *
 * @param  the condition variable
 */
OI_BOOL OI_Cond_Signal(OI_COND *cond)
{
    int ret;

    ret = pthread_cond_signal(cond);
    if (0 == ret) {
        return TRUE;
    }
    else {
        return FALSE;
    }
}

/**
 * Wake all threads waiting on the specified condition variable.
 *
 * @param  the condition variable
 */
OI_BOOL OI_Cond_Broadcast(OI_COND *cond)
{
    int ret;

    ret = pthread_cond_broadcast(cond);
    if (0 == ret) {
        return TRUE;
    }
    else {
        return FALSE;
    }
}

/**
 * Wait for the specified condition variable to become signalled.  The
 * mutex paramter must be locked before calling this function.
 *
 * @param  the condition variable
 * @param  the mutex
 */
OI_BOOL OI_Cond_Wait(OI_COND *cond,
                     OI_MUTEX *mutex)
{
    int ret;

    ret = pthread_cond_wait(cond, mutex);
    if (0 == ret) {
        return TRUE;
    }
    else {
        return FALSE;
    }
}

/**
 * Wait for the specified condition variable to become signalled or
 * until the specified time has been exceeded.  The mutex paramter
 * must be locked before calling this function.
 *
 * @param  the condition variable
 * @param  the mutex
 * @param  the number of milliseconds to wait
 *
 * @return  OI_OK if signaled, OI_TIMEOUT if a timeout occured, or another value if an error ocurred
 */
OI_STATUS OI_Cond_TimedWait(OI_COND *cond,
                            OI_MUTEX *mutex,
                            OI_UINT32 msec)
{
    int ret;
    struct timespec ts;

    clock_gettime(CLOCK_MONOTONIC, &ts);
    ts.tv_sec += (msec / 1000);
    ts.tv_nsec += ((msec % 1000) * 1000000);

    ret = pthread_cond_timedwait(cond, mutex, &ts);

    if (0 == ret) {
        return OI_OK;
    }
    else if (ETIMEDOUT == ret) {
        return OI_TIMEOUT;
    }
    else {
        return ret;
    }
}

/**
 * Destroy the specified condition variable
 *
 * @param  the condition variable
 */
OI_BOOL OI_Cond_Destroy(OI_COND *cond)
{
    int ret;

    ret = pthread_cond_destroy(cond);
    if (0 == ret) {
        return TRUE;
    }
    else {
        return FALSE;
    }
}

/* Unsigned divide by 1000. See http://www.hackersdelight.org/divcMore.pdf for
 * details. Explicitly verified correct for 0 <= x <= 0xFFFFFFFF.
 * */
static __inline OI_UINT32 divu1000(OI_UINT32 n)
{
    OI_UINT32 q, r, t;

    t = (n >> 7) + (n >> 8) + (n >> 12);
    q = (n >> 1) + t + (n >> 15) + (t >> 11) + (t >> 14);
    q >>= 9;
    r = n - q*1000;
    return q + ((r + 24) >> 10);
}

/**
 * OI_Milliseconds provides the caller with a 32 bit free running ms time base.
 */
OI_UINT32 OI_Milliseconds( void )
{
    struct timespec ts;
    OI_UINT32       ret_val;

    clock_gettime(CLOCK_MONOTONIC, &ts);

    ret_val = ((OI_UINT32)ts.tv_sec) * 1000;
    ret_val += (OI_UINT32)ts.tv_nsec / 1000000;

    return ret_val;
}
