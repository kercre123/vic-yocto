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
 * Sample protocol stack manager for Linux
 *
 * This example protocol stack wrapper for Linux is provided to demonstrate
 * application-level management of the BLUEmagic 3.0 protocol stack and support modules.
 * The stack wrapper initializes the protocol stack and support modules and provides
 * a stack access token to enforce exclusive access to the protocol stack.
 *
 * The stack access token is an operating-system-specific mechanism used to ensure that
 * only one process at a time attempts to make function calls into the protocol stack code.
 * Since the Linux operating system provides a mutex capability, the stack access token
 * for this platform is a mutex. On some other operating systems, other schemes using
 * binary sempahores may be used.
 *
 * The application must call OI_Wrapper_Init() to initialize the stack wrapper module,
 * including the synchronization scheme.
 *
 * The application must get the stack access token with OI_Wrapper_GetToken()
 * in order to make calls into the BLUEmagic 3.0 protocol stack. The application must
 * release the stack access token from time to time with OI_Wrapper_ReleaseToken()
 * in order to give other processes (the transport thread(s) and the dispatch thread)
 * an opportunity to run.
 */

#ifndef LINUX
    #error This file is targeted for LINUX only!!!
#endif

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <pthread.h>
#include <errno.h>

#include "oi_bt_stack_init.h"
#include "oi_bt_module_init.h"

#include "oi_debug.h"
#include "oi_osinterface.h"
#include "oi_time.h"
#include "oi_statustext.h"
#include "oi_wrapper.h"
#include "oi_stddefs.h"
#include "oi_status.h"
#include "oi_utils.h"

/*
 * Globals and defines
 */
#define LONG_WAIT (60 * 60)     /**< Wait this long (in seconds) while we are idle. */
#define THREAD_STACK_SIZE 4096  /**< The stack size of the dispatch thread. */

static OI_UINT32 idleLoopSleepTime;
static pthread_t StackThread;                  /**< AKA the dispatcher thread. */
static pthread_cond_t idler;                   /**< A condition variable to block the stack loop when the stack has nothing to do. */
static pthread_mutex_t idlerMutex;             /**< The mutex belonging to the idler condition variable, which also protects ServiceTime. */
static OI_TIME ServiceTime;                    /**< The time when the Dispatcher next requires service. */
static OI_BOOL runStackLoop = FALSE;           /**< Semaphore to communicate to the dispatch thread. */
static OI_BOOL someInitCompleted;              /**< Callback flag signals device configuration completion. */
static pthread_mutex_t stackAccessMutex;       /**< The stack access mutex is the token that determines which process may enter
                                                    the protocol stack. */

/**
 * This function initializes the synchronization mechanism.
 */
static OI_STATUS OI_WrapperToken_Init(void)
{
    int initStatus;

    pthread_mutexattr_t mutexAttr;
    pthread_mutexattr_init(&mutexAttr);
    /*
     * Initialize a recursive mutex.
     *
     * When pthread_mutex_lock() is called on a mutex that is already
     * locked by the calling thread, it will increment an internal
     * counter and return immediately.  pthread_mutex_unlock() will
     * decrement that internal counter, releasing the lock when it has
     * been unlocked the same number of times it was locked.
     *
     * Recursive mutexes also include error checking when unlocking a
     * mutex owned by another thread, or an already-unlocked mutex.
     */
    pthread_mutexattr_settype(&mutexAttr, PTHREAD_MUTEX_RECURSIVE);

    initStatus = pthread_mutex_init(&stackAccessMutex, &mutexAttr);
    pthread_mutexattr_destroy(&mutexAttr);
    if (initStatus != 0) {
        OI_DBGTRACE(("Failed to intialize the stack sync token: %s", strerror(initStatus)));
        return OI_STATUS_INITIALIZATION_FAILED;
    } else {
        return OI_OK;
    }
}


void OI_Wrapper_GetToken(void)
{
    int lockStatus;

    lockStatus = pthread_mutex_lock(&stackAccessMutex);
    if (lockStatus != 0) {
        OI_DBGTRACE(("Error getting the stack sync token on thread %x: %s\n",
            pthread_self(), strerror(lockStatus)));
    }
}


void OI_Wrapper_GetTokenRecursive(void)
{
    /*
     * Since stackAccessMutex is configured as a recursive mutex, the
     * normal "get token" operation is already recursive.
     */
    OI_Wrapper_GetToken();
}


void OI_Wrapper_ReleaseToken(void)
{
    int unlockStatus;

    unlockStatus = pthread_mutex_unlock(&stackAccessMutex);
    if (unlockStatus != 0) {
        OI_DBGTRACE(("Error releasing the stack sync token on thread %x: %s\n",
            pthread_self(), strerror(unlockStatus)));
    }
}


void OI_Wrapper_ReleaseTokenRecursive(void)
{
    /*
     * Since stackAccessMutex is configured as a recursive mutex, the
     * normal "release token" operation is already recursive.
     */
    OI_Wrapper_ReleaseToken();
}

static void stackInitCb(OI_STATUS status)
{
    if (!OI_SUCCESS(status)) {
        OI_DBGTRACE(("stackInitCb, status %d\n", status) );
        OI_FatalError(status);
    }
    OI_Wrapper_SignalEvent(&someInitCompleted) ;
}


/**
 * This callback function handles service requests.  This function saves
 * requested service time, while the StackLoop() function checks the value.
 */
static void ServiceRequestHandler(OI_UINT msec)
{
   OI_TIME update;
   OI_UINT secs;

   if (msec == 0) {
        /*
         * The Dispatcher wants service now.
         */
       pthread_mutex_lock(&idlerMutex);
       OI_Time_Now(&ServiceTime);
       pthread_cond_signal(&idler);
       pthread_mutex_unlock(&idlerMutex);
   } else {
       OI_Time_Now(&update);
       msec += update.mseconds;
       secs = msec / 1000;
       update.seconds += secs;
       update.mseconds = msec - (secs * 1000);
       /*
        * If the updated service time is earlier than the previous service time
        * we need to let the stack loop know about it.
        * The mutex must be locked to protect accesses to ServiceTime, but
        * the condition variable is only signaled if ServiceTime is changed.
        */
       pthread_mutex_lock(&idlerMutex);
       if (OI_Time_Compare(&update, &ServiceTime) < 0) {
           ServiceTime = update;
           pthread_cond_signal(&idler);
       }
       pthread_mutex_unlock(&idlerMutex);
   }
}


/**
 * This is the thread in which the real work gets done. This function uses a
 * mutex to synchronize execution between the application and the stack. This
 * function also demonstrates handling of the requested service time.
 * ServiceRequestHandler() saves the requested value, while this function
 * decrements the requested time by the amount of time elapsed since the last
 * run. Once the time has expired, this function calls into the stack. This
 * implementation is designed to present the basic usage model for the service
 * request without complex system calls.
 */
static void* StackLoop(void *arg)
{
    struct timespec timeout;

    while (runStackLoop) {
        /*
         * If the service timeout has already expired or we have waited and it has
         * expired, then the Dispatcher has work to do.
         */
        pthread_mutex_lock(&idlerMutex);

        timeout.tv_sec = ServiceTime.seconds;
        timeout.tv_nsec = 1000000 * ServiceTime.mseconds;

        pthread_cond_timedwait(&idler, &idlerMutex, &timeout);

        if (!runStackLoop) {
            OI_DBGTRACE(("Exiting StackLoop\n"));
            pthread_mutex_unlock(&idlerMutex);
            return NULL;
        }

        /*
         * The Dispatcher will reset this value if there is more work
         * to be done.  Like all access to ServiceTime, this needs to
         * be done while idlerMutex is locked by this thread.
         */
        ServiceTime.seconds += LONG_WAIT;

        pthread_mutex_unlock(&idlerMutex);

        OI_Wrapper_GetToken() ;
        while(OI_Dispatch_Run());
        OI_Wrapper_ReleaseToken();
    }
    return NULL;
}


/**
 * Allow the Dispatcher to run. The period that the idle loop sleeps is set in
 * the call to OI_Wrapper_Init().
 */
static void idleLoop(void)
{
    if (idleLoopSleepTime) {
        OI_Sleep(idleLoopSleepTime);
    } else {
        OI_Sleep(1);
    }
}


OI_STATUS OI_OBEX_Init(OI_UINT32 idleSleepMsecs)
{
    return(OI_Wrapper_Init_Custom(idleSleepMsecs, NULL)) ;
}


OI_STATUS OI_Wrapper_Init_Custom(OI_UINT32 idleSleepMsecs, void (*customCB)(void))
{
    OI_STATUS retVal = OI_OK ;

    idleLoopSleepTime = idleSleepMsecs;

    runStackLoop = TRUE;

    /*
     * Initialize the stack synchronization module.
     */
    retVal = OI_WrapperToken_Init();
    if (!OI_SUCCESS(retVal)) {
        OI_Print("OI_WrapperToken_Init failed\n");
        OI_FatalError(retVal);
    }
    /*
     * Get token to access stack. This will block other threads from
     * accessing the stack for now.
     */
    OI_Wrapper_GetToken() ;

    /*
     * Initialize a condition variable (and associated mutex) that will be used
     * to signal to the stack loop when to call the Dispatcher.
     */
    pthread_cond_init(&idler, NULL);

    pthread_mutex_init(&idlerMutex, NULL);

    /*
     * Create and start the stack (Dispatcher) thread. It will not actually be
     * able to run until we let go of the stack token.
     */
    if (pthread_create(&StackThread, NULL, StackLoop, NULL) != 0) {
        perror("Failed to CreateThread ");
        OI_FatalError(OI_STATUS_INITIALIZATION_FAILED);
    }
    /*
     * Request stack (Dispatcher) service immediately.
     */
    ServiceRequestHandler(0);
    /*
     * Initialize the protocol stack.
     */
    someInitCompleted = FALSE;
    retVal = OI_BT_StackInit_Custom(stackInitCb, ServiceRequestHandler, customCB);
    if (!OI_SUCCESS(retVal)) {
        OI_Print("OI_BT_StackInit_Custom failed\n");
        OI_FatalError(retVal);
    }
    OI_Wrapper_ReleaseToken();

    return(OI_OK) ;
}

static OI_BOOL terminateDone = FALSE;
static void terminateBM3(void)
{
    OI_STATUS   status;

    terminateDone = FALSE;
    OI_DBGTRACE(("Terminating stack\n"));
    status = OI_BT_Stack_Terminate();
    if (!OI_SUCCESS(status)) {
        OI_DBGTRACE(("OI_BT_Stack_Terminate failed: %d\n", status));
    }
    OI_DBGTRACE(("Terminating stack done\n"));
}

void OI_OBEX_Deinit(void)
{
    OI_STATUS retVal;

    OI_DBGTRACE(("OI_OBEX_Deinit\n"));
    /*
     * First set non-discoverable
     */
    OI_Wrapper_GetToken();
    terminateDone = FALSE;
    /*
     * Terminate the stack itself.
     */
    terminateBM3();
    OI_Wrapper_ReleaseToken();     /* Done with token forever */
    /*
     * Tell the loop thread to terminate and wait until it has done so.
     */
    runStackLoop = FALSE;

    /*
     * Request stack (Dispatcher) service immediately.
     */
    ServiceRequestHandler(0);

    pthread_join(StackThread, NULL);

    pthread_mutex_destroy(&stackAccessMutex);
    pthread_mutex_destroy(&idlerMutex);
    pthread_cond_destroy(&idler);

    OI_DBGPRINTSTR(("OI_Wrapper_Terminate done\n")) ;
}


OI_STATUS OI_Wrapper_WaitForEvent(OI_BOOL *pEventFlag,
                                       OI_UINT timeoutSeconds)
{
    OI_BOOL timedOut = FALSE ;
    OI_TIME endTime ;
    OI_TIME timeNow ;

    /*
     * Set up end time.
     */
    OI_Time_Now(&endTime) ;
    endTime.seconds += timeoutSeconds;

    /*
     * Wait for event or timeout.
     */
    while (!(*pEventFlag) && !timedOut) {
        /*
         * Take a break so that the CPU is not hoarded.
         */
        idleLoop() ;
        /*
         * Check for timeout.
         */
        if (timeoutSeconds) {
            OI_Time_Now(&timeNow) ;
            if (OI_Time_Compare(&timeNow, &endTime) >= 0) {
                timedOut = TRUE;
            }
        }
    }

    /*
     * Return appropriate status to caller.  Note that is possible that both termination
     * conditions may have occurred contemporaneously.
     */
    if (*pEventFlag) {
        *pEventFlag = FALSE;
        return OI_OK;
    } else {
        return OI_TIMEOUT;
    }
}


OI_STATUS OI_Wrapper_SignalEvent (OI_BOOL *pEventFlag)
{
    *pEventFlag = TRUE ;
    return OI_OK  ;
}
