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
 * This file includes functions for fatal error handling, printing, and getting
 * a time value. This file is targeted to the Linux operating system.
 *
 * These functions are defined in oi_osinterface.h.
 */

#define __OI_MODULE__ OI_MODULE_SUPPORT

#ifndef LINUX
    #error This file is targeted for LINUX only!!!
#endif


#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <sys/stat.h>
#include <unistd.h>

#include "oi_time.h"
#include "oi_utils.h"
#include "oi_string.h"
#include "oi_debug.h"

/**
 *  Compute CRC16 of the program image.
 */
OI_UINT16  OI_ProgramImageCRC16(void)
{
    /* Not implemented */
    return (OI_UINT16)0;
}

/*
 * Fatal error handler - never returns
 */

void OI_FatalError(OI_STATUS reason) {
    OI_Printf("\nFatal error - %d\n", reason);
    exit(reason) ;
}


/**
 * Log error handler - indicate module, line number, and status code of error
 */
void OI_LogError(OI_MODULE module, OI_INT lineno, OI_STATUS status)
{
    /* Send error info to console */
    OI_Printf("\nOI_SLOG_ERROR: %d,%d,%d\n", module, lineno, status);
}

void OI_Time_Now(OI_TIME *now)
{
    struct timespec ts;

    clock_gettime(CLOCK_MONOTONIC, &ts);

    now->mseconds = ts.tv_nsec / 1000000;
    now->seconds = ts.tv_sec;
}


void OI_Sleep(OI_UINT32 milliseconds)
{
    struct timespec sleeptime;

    sleeptime.tv_sec = milliseconds / 1000;
    sleeptime.tv_nsec = (milliseconds % 1000) * 1000000;

    nanosleep(&sleeptime, NULL);
}

/*
 * Write a string to stdout
 */
void OI_Print(OI_CHAR const *str)
{
    if (str != NULL) {
        fputs(str, stdout);
    }
    fflush(stdout);
}

/*****************************************************************************/
