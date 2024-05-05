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

#define __OI_MODULE__ OI_MODULE_SOCKETS

#include "oi_thread.h"
#include "oi_eventloop.h"
#include "oi_wrapper.h"
#include "oi_assert.h"
#include "oi_utils.h"
#include "oi_list.h"
#include "oi_argcheck.h"
#include <unistd.h>
#include <fcntl.h>
#include <stdlib.h>
#include <errno.h>
#include <string.h>
#include "oi_debug.h"

// Indexes into the FD array returned by pipe()
#define READ_PIPE (0)
#define WRITE_PIPE (1)

typedef struct _EVENTLOOP_REGISTERED_ITEM {
    OI_EVENTLOOP_GET_DESCRIPTORS_CB getDescriptors;
    OI_EVENTLOOP_HANDLE_EVENTS_CB handleEvents;
    OI_EVENTLOOP_VALIDATE_CB validate;
    OI_LIST_ELEM links;
} EVENTLOOP_REGISTERED_ITEM;

static int WakeupFD[2];
static OI_BOOL loopExit;

static OI_LIST_ELEM callbackList;
static OI_THREAD thread;

void OI_EVENTLOOP_Wakeup(void)
{
    // Do not acquire the token in this function, it can cause deadlocks.
    int result;
    char aByte = '\0';

    result = write(WakeupFD[WRITE_PIPE], &aByte, 1);
}

static int getWakeupFds(fd_set* readFds, fd_set* writeFds, fd_set* exceptFds, int* maxFd)
{
    /*
     * Token already held by caller.  WakeupFD[] isn't modified after
     * init, so it is ok to access without the token.
     */

    int numFds = 1;

    // Always watch the wakeup pipe
    FD_SET(WakeupFD[READ_PIPE], readFds);
    *maxFd = WakeupFD[READ_PIPE];

    return numFds;
}

static int handleWakeup(int numSet, fd_set* readFds, fd_set* writeFds, fd_set* exceptFds)
{
    // Token already held by caller, but is only needed for logging/printing.
    int result;
    OI_BYTE readBuf[32];

    if (FD_ISSET(WakeupFD[READ_PIPE], readFds)) {
        /*
         * Try to empty the pipe - don't want to leave bytes queued
         * in the wakeup pipe.
         */
        result = read(WakeupFD[READ_PIPE], &readBuf, OI_ARRAYSIZE(readBuf));
        FD_CLR(WakeupFD[READ_PIPE], readFds);
        numSet--;
        OI_DBGPRINTSTR(("handleWakeup()"));
    }

    if (FD_ISSET(WakeupFD[READ_PIPE], writeFds)) {
        OI_SLOG_ERROR(OI_STATUS_INTERNAL_ERROR, ("Unexpected write flag on wakeup pipe"));
        FD_CLR(WakeupFD[READ_PIPE], writeFds);
        numSet--;
    }

    if (FD_ISSET(WakeupFD[READ_PIPE], exceptFds)) {
        OI_SLOG_ERROR(OI_STATUS_INTERNAL_ERROR, ("Exception on wakeup pipe"));
        FD_CLR(WakeupFD[READ_PIPE], exceptFds);
        numSet--;
    }

    return numSet;
}



static void* eventThread(void* arg)
{
    // "Main loop" integration

    while (!loopExit) {
        fd_set readFds;
        fd_set writeFds;
        fd_set exceptFds;
        int maxFd;
        int numFds;
        int numSet;
        OI_LIST_ELEM* head;
        EVENTLOOP_REGISTERED_ITEM marker;
        OI_DBGPRINT(("%s", __func__));
        marker.getDescriptors = NULL;
        marker.handleEvents = NULL;

        FD_ZERO(&readFds);
        FD_ZERO(&writeFds);
        FD_ZERO(&exceptFds);

        OI_Wrapper_GetToken();

        numFds = getWakeupFds(&readFds, &writeFds, &exceptFds, &maxFd);

        /*
         * Set up marker -- used for walking the linked list in a way
         * that allows on-the-fly modification.
         */
        OI_List_DynamicInit(&marker.links);
        OI_List_AddTail(&marker.links, &callbackList);

        head = OI_List_RemoveHead(&callbackList);
        while (head != &marker.links) {
            EVENTLOOP_REGISTERED_ITEM* item;
            int thisMaxFd;

            item = OI_LIST_ENTRY(head, EVENTLOOP_REGISTERED_ITEM, links);
            OI_ASSERT(NULL != item);
            /*
             * Rotate list element to the end of the linked list,
             * where it can be safely removed if the getDescriptors()
             * call chooses to do so.
             */
            OI_List_AddTail(&item->links, &callbackList);

            OI_ASSERT(NULL != item->getDescriptors);
            numFds += item->getDescriptors(&readFds, &writeFds, &exceptFds, &thisMaxFd);
            maxFd = OI_MAX(maxFd, thisMaxFd);

            head = OI_List_RemoveHead(&callbackList);
        }

        /*
         * ! Important - 'marker' is on the stack and cannot be in the
         * queue when the above while loop exits!
         */
        OI_ASSERT(head == &marker.links);

#ifndef ANDROID
        OI_DBGPRINT(("Waiting on %d FDs: read=%08x, write=%08x, except=%08x",
                      numFds,
                      __FDS_BITS(&readFds)[0],
                      __FDS_BITS(&writeFds)[0],
                      __FDS_BITS(&exceptFds)[0]));
#endif
        OI_DBGPRINTSTR(("Event loop about to call select()"));
        OI_Wrapper_ReleaseToken();

        numSet = select(maxFd+1,
                        &readFds,
                        &writeFds,
                        &exceptFds,
                        NULL);

        if (loopExit) {
            OI_DBGPRINTSTR(("Exiting Event loop"));
            // If another thread has shut this loop down, exit now.
            return NULL;
        }

        // Token is needed while calling handler functions.
        OI_Wrapper_GetToken();
        OI_DBGPRINTSTR(("Event loop running."));
#ifndef ANDROID
        OI_DBGPRINT(("Got %d events: read=%08x, write=%08x, except=%08x",
                      numSet,
                      __FDS_BITS(&readFds)[0],
                      __FDS_BITS(&writeFds)[0],
                      __FDS_BITS(&exceptFds)[0]));
#endif

        if (-1 == numSet) {
            switch (errno) {
            case EINTR:
                /*
                 * EINTR errors are benign - set numSet to 0 so processing happens
                 * normally.
                 */
                numSet = 0;
                break;
            case EBADF:
                /* select() returned a recoverable error.
                 * Run the dispatcher in case anything there is trying to
                 * clean up defunct sockets.
                 */
                while(OI_Dispatch_Run());
                break;
            default:
                /*
                 * Something is severely wrong.
                 */
                OI_SLOG_ERROR(OI_OS_ERROR, ("Unexpected select() error %d: %s", errno, strerror(errno)));
                break;
            }
        }

        /*
         * Set up marker -- used for walking the linked list in a way
         * that allows on-the-fly modification.
         */
        OI_List_DynamicInit(&marker.links);
        OI_List_AddTail(&marker.links, &callbackList);

        head = OI_List_RemoveHead(&callbackList);
        while (head != &marker.links) {
            EVENTLOOP_REGISTERED_ITEM* item;
            item = OI_LIST_ENTRY(head, EVENTLOOP_REGISTERED_ITEM, links);
            OI_ASSERT(NULL != item);

            /*
             * Rotate list element to the end of the linked list,
             * where it can be safely removed if the getDescriptors()
             * call chooses to do so.
             */
            OI_List_AddTail(&item->links, &callbackList);

            if (numSet >= 0) {
                /* File descriptors are valid, process them. */
                OI_ASSERT(NULL != item->handleEvents);
                numSet = item->handleEvents(numSet, &readFds, &writeFds, &exceptFds);
            }
            else {
                /* select() returned -1 for errors.  Trigger validation. */
                OI_ASSERT(NULL != item->validate);
                item->validate();
            }
            head = OI_List_RemoveHead(&callbackList);
        }

        /*
         * ! Important - 'marker' is on the stack and cannot be in the
         * queue when the above while loop exits!
         */
        OI_ASSERT(head == &marker.links);

        if (numSet > 0) {
            numSet = handleWakeup(numSet, &readFds, &writeFds, &exceptFds);
        }

        // All events should have been handled.
        if (0 != numSet) {
            OI_DBGPRINT(("Not all events were handled in the event loop (%d remaining).", numSet));
        }

        OI_Wrapper_ReleaseToken();
    }

    return NULL;
}

// Init
OI_STATUS OI_EVENTLOOP_Init(void)
{
    int ret;

    // Set up wakeup pipe
    if (-1 == pipe(WakeupFD)) {
        return OI_FAIL;
    }

    /* Do nonblocking reads & writes on the wakeup pipe, since it is
     * important to avoid blocking while the stack token or DBUS locks
     * are held.
     */
    fcntl(WakeupFD[READ_PIPE], F_SETFL, O_NONBLOCK);
    fcntl(WakeupFD[WRITE_PIPE], F_SETFL, O_NONBLOCK);

    // Set up handler list
    OI_List_DynamicInit(&callbackList);

    // Start thread.
    OI_DBGPRINTSTR(("Starting event loop"));
    loopExit = FALSE;
    ret = OI_Thread_Create(&thread, eventThread, NULL);
    OI_DBGPRINT(("OI_Thread_Create returned %d", ret));

    return OI_OK;
}


OI_STATUS OI_EVENTLOOP_Shutdown(void)
{
    OI_LIST_ELEM* elem;

    loopExit = TRUE;
    OI_EVENTLOOP_Wakeup();

    OI_Thread_Join(thread);

    close(WakeupFD[WRITE_PIPE]);
    close(WakeupFD[READ_PIPE]);

    // Free callback list elements.
    elem = OI_List_RemoveHead(&callbackList);
    while (NULL != elem) {
        free(OI_LIST_ENTRY(elem, EVENTLOOP_REGISTERED_ITEM, links));
        elem = OI_List_RemoveHead(&callbackList);
    }

    return OI_OK;
}

OI_STATUS OI_EVENTLOOP_Register(OI_EVENTLOOP_GET_DESCRIPTORS_CB getDescriptors,
                                OI_EVENTLOOP_HANDLE_EVENTS_CB handleEvents,
                                OI_EVENTLOOP_VALIDATE_CB validate,
                                OI_BOOL frequentEvents)
{
    EVENTLOOP_REGISTERED_ITEM* newItem;

    OI_ARGCHECK(getDescriptors);
    OI_ARGCHECK(handleEvents);
    OI_ARGCHECK(validate);

    newItem = malloc(sizeof(EVENTLOOP_REGISTERED_ITEM));
    if (NULL == newItem) {
        return OI_STATUS_OUT_OF_MEMORY;
    }

    newItem->getDescriptors = getDescriptors;
    newItem->handleEvents = handleEvents;

    if (frequentEvents) {
        OI_List_Add(&newItem->links, &callbackList);
    }
    else {
        OI_List_AddTail(&newItem->links, &callbackList);
    }

    return OI_OK;
}



OI_STATUS OI_EVENTLOOP_Deregister(OI_EVENTLOOP_GET_DESCRIPTORS_CB getDescriptors,
                                  OI_EVENTLOOP_HANDLE_EVENTS_CB handleEvents,
                                  OI_EVENTLOOP_VALIDATE_CB validate)
{
    OI_STATUS status = OI_STATUS_INVALID_PARAMETERS;
    OI_LIST_ELEM* elem;

    OI_LIST_FOREACH(elem, &callbackList) {
        EVENTLOOP_REGISTERED_ITEM* item;
        item = OI_LIST_ENTRY(elem, EVENTLOOP_REGISTERED_ITEM, links);

        if ((item->getDescriptors == getDescriptors) &&
            (item->handleEvents   == handleEvents) &&
            (item->validate       == validate)) {
            OI_List_Del(elem);
            free(item);
            status = OI_OK;
            break;
        }
    }

    return status;
}

OI_BOOL OI_EVENTLOOP_FdIsValid(int fd)
{
    struct timeval timeout;
    fd_set readFds;

    if (fd < 0) {
        return FALSE;
    }

    timeout.tv_sec = 0;
    timeout.tv_usec = 0;
    FD_ZERO(&readFds);
    FD_SET(fd, &readFds);

    /* Non-negative return value from select() indicates a valid FD */
    return (0 <= select(fd+1, &readFds, NULL, NULL, &timeout));
}
