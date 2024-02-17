#ifndef _OI_EVENT_LOOP_H
#define _OI_EVENT_LOOP_H

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
 * Event loop API: platform-specific loop for Linux.
 */

#include <sys/select.h>

#ifdef __cplusplus
extern "C" {
#endif

/**
 * A callback function of this type is used by the event loop to find
 * out which file descriptors to wait on.  It is run on every
 * iteration of the event loop - possibly very frequently if the loop
 * is used by any part of the system to monitor sockets - so the
 * function should not take long to execute.
 *
 * The provided fd_sets may already contain file descriptors and are
 * modified in-place.
 *
 * A callback function of this type must be provided in calls to
 * OI_EVENTLOOP_Register().
 *
 * @param  readFds   File descriptors to watch for new incoming data
 *
 * @param  writeFds  File descriptors to watch for available output buffer
 *                   space.
 *
 * @param  exceptFds File descriptors to watch for exceptions
 *
 * @param  maxFd     Highest FD number set for read/write/except
 *
 * @return           The number of file descriptors added (does not count
 *                   descriptors that were already included in the lists)
 */
typedef int (*OI_EVENTLOOP_GET_DESCRIPTORS_CB)(fd_set* readFds,
                                               fd_set* writeFds,
                                               fd_set* exceptFds,
                                               int* maxFd);

/**
 * A callback function of this type is used by the event loop to
 * process I/O when an event is detected.  It will run for every event
 * that triggers the loop, even events not requested by the companion
 * OI_EVENTLOOP_GET_DESCRIPTORS_CB function, so this function should
 * be as quick and efficient as possible.  All operations must be
 * *non-blocking*, except for locking required for multi-threaded
 * operation.
 *
 * When an event for a given file descriptor is handled, the
 * descriptor should be cleared from the relevant fd_set.
 *
 * A callback function of this type must be provided in calls to
 * OI_EVENTLOOP_Register().
 *
 * @param  numSet    Total number of events in all fd_sets
 *
 * @param  readFds   File descriptors with data available for reading
 *
 * @param  writeFds  File descriptors with output buffer space available
 *
 * @param  exceptFds File descriptors with exceptions
 *
 * @return           The number of unprocessed events remaining in
 *                   all fd_sets.
 */
typedef int (*OI_EVENTLOOP_HANDLE_EVENTS_CB)(int numSet,
                                             fd_set* readFds,
                                             fd_set* writeFds,
                                             fd_set* exceptFds);

/**
 * A validate callback function is invoked only when a bad file
 * descriptor has been detected.  All file descriptors being tracked
 * by the registered module should be checked for validity by calling
 * OI_EVENTLOOP_FdIsValid() on the file descriptor.  If
 * OI_EVENTLOOP_FdIsValid() returns FALSE, the module should clean up
 * any data associated with that file descriptor.
 */
typedef void (*OI_EVENTLOOP_VALIDATE_CB)(void);

/**
 * This function initializes the event loop module and starts the event
 * thread.
 *
 * @return OI_OK if init is successful; various error codes
 *         indicating reason for failure otherwise
 */
OI_STATUS OI_EVENTLOOP_Init(void);

/**
 * This function closes down the event loop module and stops the event
 * thread.
 *
 * @return OI_OK if shutdown is successful; various error codes
 *         indicating reason for failure otherwise
 */
OI_STATUS OI_EVENTLOOP_Shutdown(void);

/**
 * This function registers pairs of callbacks for the event loop to
 * call to request events to wait on, and to process those events.
 * These callbacks are used on every iteration of the event loop.
 *
 * Callbacks can be registered only once.
 *
 * Registrations flagged with "frequentEvents" will have their
 * handleEvents functions called first.  This way, those functions
 * remove their events from the fd_sets early, and later handlers
 * can skip their processing if there are no remaining events to
 * process.
 *
 * @param  getDescriptors   Callback function to get expected events.
 *
 * @param  handleEvents     Callback function to process events.
 *
 * @param  frequentEvents   Set to TRUE if this pair of functions is
 *                          expected to trigger and handle lots of
 *                          events (like socket I/O).
 *
 * @return                  OI_OK if the functions are successfully
 *                          registered; various error codes indicating
 *                          reason for failure otherwise
 */

OI_STATUS OI_EVENTLOOP_Register(OI_EVENTLOOP_GET_DESCRIPTORS_CB getDescriptors,
                                OI_EVENTLOOP_HANDLE_EVENTS_CB handleEvents,
                                OI_EVENTLOOP_VALIDATE_CB validate,
                                OI_BOOL frequentEvents);

/**
 * This function removes pairs of callbacks from the event loop.
 *
 * @param  getDescriptors   Callback function to remove.
 *
 * @param  handleEvents     Callback function to remove.
 *
 * @return                  OI_OK if the functions are successfully
 *                          deregistered; various error codes indicating reason
 *                          for failure otherwise
 */

OI_STATUS OI_EVENTLOOP_Deregister(OI_EVENTLOOP_GET_DESCRIPTORS_CB getDescriptors,
                                  OI_EVENTLOOP_HANDLE_EVENTS_CB handleEvents,
                                  OI_EVENTLOOP_VALIDATE_CB validate);

/**
 * This function forces an iteration of the event loop, which is useful to
 * update the descriptors.
 *
 */

void OI_EVENTLOOP_Wakeup(void);

/**
 * This function may be called to determine if a file descriptor is still 
 * valid.  It is typically called within a module's OI_EVENTLOOP_VALIDATE_CB
 * function.
 *
 * @return       TRUE if FD is valid, FALSE if it is not.
 */
OI_BOOL OI_EVENTLOOP_FdIsValid(int fd);

#ifdef __cplusplus
}
#endif

#endif /* _OI_EVENT_LOOP_H */
