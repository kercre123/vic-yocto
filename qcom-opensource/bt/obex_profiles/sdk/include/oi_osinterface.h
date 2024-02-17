#ifndef _OI_OSINTERFACE_H
#define _OI_OSINTERFACE_H

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
 * This file provides the platform-independent interface for functions for which
 * implementation is platform-specific.
 *
 * The functions in this header file define the operating system or hardware
 * services needed by the BLUEmagic 3.0 protocol stack. The
 * actual implementation of these services is platform-dependent.
 *
 */

#include "oi_stddefs.h"
#include "oi_time.h"
#include "oi_status.h"
#include "oi_modules.h"

/** \addtogroup Misc Miscellaneous APIs */
/**@{*/

#ifdef __cplusplus
extern "C" {
#endif


/**
 * Terminates execution.
 *
 * @param reason  Reason for termination
 */
void OI_FatalError(OI_STATUS reason);

/**
 * This function logs an error.
 *
 * When built for release mode, BLUEmagic 3 errors are logged to
 * this function. (in debug mode, errors are logged via
 * OI_Print()).
 *
 * @param module Module in which the error was detected (see
 *                oi_modules.h)
 * @param lineno Line number of the C file OI_SLOG_ERROR called
 * @param status Status code associated with the error event
 */
void OI_LogError(OI_MODULE module, OI_INT lineno, OI_STATUS status);

/**
 * This function initializes the debug code handling.
 *
 * When built for debug mode, this function performs platform
 * dependent initialization to handle message codes passed in
 * via OI_SetMsgCode().
 */
void OI_InitDebugCodeHandler(void);


/**
 * This function reads the time from the real time clock.
 *
 * All timing in BM3 is relative, typically a granularity
 * of 5 or 10 msecs is adequate.
 *
 * @param[out] now  Pointer to the buffer to which the current
 *       time will be returned
 */
void OI_Time_Now(OI_TIME *now);

/**
 * This function causes the current thread to sleep for the
 * specified amount of time. This function must be called
 * without the stack access token.
 *
 * @note BM3 corestack and profiles never suspend and never call
 * OI_Sleep. The use of OI_Sleep is limited to applications and
 * platform-specific code.
 *
 * If your port and applications never use OI_Sleep, this function can be left unimplemented.
 *
 * @param milliseconds  Number of milliseconds to sleep
 */
void OI_Sleep(OI_UINT32 milliseconds);


/**
 * Defines for message type codes.
 */
#define OI_MSG_CODE_APPLICATION               0   /**< Application output */
#define OI_MSG_CODE_ERROR                     1   /**< Error message output */
#define OI_MSG_CODE_WARNING                   2   /**< Warning message output */
#define OI_MSG_CODE_TRACE                     3   /**< User API function trace output */
#define OI_MSG_CODE_PRINT1                    4   /**< Catagory 1 debug print output */
#define OI_MSG_CODE_PRINT2                    5   /**< Catagory 2 debug print output */
#define OI_MSG_CODE_HEADER                    6   /**< Error/Debug output header */

/**
 * This function is used to indicate the type of text being output with
 * OI_Print(). For the Linux and Win32 platforms, it will set
 * the color of the text. Other possible uses could be to insert
 * HTML style tags, add some other message type indication, or
 * be completely ignored altogether.
 *
 * @param code  OI_MSG_CODE_* indicating setting the message type.
 */
void OI_SetMsgCode(OI_UINT8 code);

/**
 * All output from OI_Printf() is sent to OI_Print.
 * Typically, if the platform has a console, OI_Print() is sent to stdout.
 * Embedded platforms typically send OI_Print() output to a serial port.
 *
 * @param str  String to print
 */
void OI_Print(OI_CHAR const *str);

/**
 * All BM3 debug output is sent to OI_PrintDebugOutput.  Typically, this
 * output is simply passed to OI_Print.  On some targets, it is desirable to 
 * filter debug output solely based on severity or verbosity in which case
 * the implementation would check message level before passing output to 
 * OI_Print.
 *
 * @param str       String to print
 *
 * @param msgType   Identifies message type (e.g. OI_MSG_CODE_ERROR or OI_MSG_CODE_PRINT1).
 */
void OI_PrintDebugOutput(OI_UINT8 msgType, OI_CHAR const *str);

/** 
 *  In cases where OI_Print() is sending output to a logfile in addition to console,
 *  it is desirable to also put console input into the logfile.
 *  This function can be called by the console input process.
 * 
 *  @note This is an optional API which is strictly
 *  between the platform-specific stack_console and osinterface
 *  modules. This API need only be implemented on those
 *  platforms where is serves a useful purpose, e.g., win32.
 *
 * @param str  Console input string
 */

void OI_Print_ConsoleInput(OI_CHAR const *str);

/**
 *  This function computes the CRC16 of the program image.
 */
OI_UINT16  OI_ProgramImageCRC16(void);

/**
 * Writes an integer to stdout in hex. This macro is intended
 * for selective use when debugging in small memory
 * configurations or other times when it is not possible to use
 * OI_DBGPRINT.
 *
 * @param n  Specifies the integer to print.
 */

#define OI_Print_Int(n) \
{ \
    static const OI_CHAR _digits[] = "0123456789ABCDEF"; \
    OI_CHAR _buf[9]; \
    OI_CHAR *_str = &_buf[8]; \
    OI_UINT32 _i = n; \
    *_str = 0; \
    do { *(--_str) = _digits[(_i & 0xF)]; _i >>= 4; } while (_i); \
    OI_Print(_str); \
}

/**
 * Get the value of an environment variable from the execution environment.
 *
 * @param key    Name of environment variable to get
 * @param value  Pointer at buffer to receive the environment variable value
 * @param len    Length of buffer pointed to value
 * @return       OI_OK if the value was found and was completely copied into buffer
 */
OI_STATUS OI_GetEnv(const OI_CHAR *key,
                             OI_CHAR *value,
                             OI_UINT len);


/**
 * Print the last system error. perror for POSIX enviroments
 *
 * @param str    String to prepend to output
 */
void OI_PrintLastError(const char *str);

/**
 * Get file size. stat in POSIX environments
 *
 * @param fileName    Full path of file
 * @param fileSize    pointer at variable to return size
 * @return       OI_OK on success
 */
OI_STATUS OI_GetFileSize(const char *fileName, OI_INT *fileSize);

/**
 *  Application Dynamic Memory allocation.
 *
 *  These APIs are provided for application use on those
 *  platforms which have no dynamic memory support. Memory is
 *  allocated from the pool-based heap managed by the stack's
 *  internal memory manager.
 */
void *OI_APP_Malloc(OI_INT32 size);
void OI_APP_Free(void *ptr);

/*****************************************************************************/
#ifdef __cplusplus
}
#endif

/**@}*/

#endif /* _OI_OSINTERFACE_H */

