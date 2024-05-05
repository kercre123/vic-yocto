#ifndef _OI_DEBUGCONTROL_H
#define _OI_DEBUGCONTROL_H

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
 * @file
 *
 * The debugging level for each module is controlled through this header file.
 *
 * Additional information is available in the @ref debugging_docpage section.
 */

#include "oi_modules.h"
#include "oi_stddefs.h"
#include "oi_status.h"
#include "oi_osinterface.h"

/** \addtogroup Debugging Debugging APIs */
/**@{*/

#ifdef __cplusplus
extern "C" {
#endif


/**
 * This macro converts the message code to bit mask flag used in the encoding
 * of the levelEnableFlags member of the OI_DBG_OUTPUT_CTRL structure.
 */
#define OI_DBG_MSG_ENABLE(_code) (1 << (_code))

/**
 * Convenience macros for selecting common sets of debug output classes.
 */
#define OI_DBG_MSG_ERROR_ENABLE (OI_DBG_MSG_ENABLE(OI_MSG_CODE_ERROR) | OI_DBG_MSG_ENABLE(OI_MSG_CODE_WARNING))
#define OI_DBG_MSG_TRACE_ENABLE (OI_DBG_MSG_ENABLE(OI_MSG_CODE_TRACE))
#define OI_DBG_MSG_PRINT_ENABLE (OI_DBG_MSG_ENABLE(OI_MSG_CODE_PRINT1) | OI_DBG_MSG_ENABLE(OI_MSG_CODE_PRINT2))
#define OI_DBG_MSG_ALL_ENABLE (OI_DBG_MSG_ERROR_ENABLE | OI_DBG_MSG_TRACE_ENABLE | OI_DBG_MSG_PRINT_ENABLE)


/**
 * This function sets the debug output control for a given module.  The output
 * is controlled by a sequence of control characters.  The control characters
 * are as follows:
 *
 * - L     - Enable Line Number output
 * - U     - Enable Trace output
 * - N     - Only application output and Errors are reported. It will also override
 *           any settings to OI_DEBUG_ALL.
 * - ON    - Enable debug logging using the default settings.  OI_DEBUG_DEFAULT
 *           can be used to customize default settings.  This must be set alone.
 * - OFF   - This is similar to N, however it will not override any settings to
 *           OI_DEBUG_ALL.  It must be set alone.
 * - 1     - Enable Print 1 and Trace.
 * - 2     - Enable Print 1, Print 2, and Trace.
 * - 123   - Same as ON, but using line numbers. It must be alone.
 *
 * @param module    the module for which to set the debugging level
 * @param ctrlStr   the debug output control for the specified module
 */
OI_STATUS OI_SetDebugControl(OI_MODULE module,
                             const OI_CHAR *ctrlStr);

/**
 * This function sets the debug output control for a given module by the
 * textual name of the module.  See the documentation for @ref
 * OI_SetDebugControl for details on the control character codes.
 *
 * @param name      the string name for the module on which to set the debug output level
 * @param ctrlStr   the debug output control for the specified module
 */
OI_STATUS OI_SetDebugControlByName(const OI_CHAR *name,
                                   const OI_CHAR *ctrlStr);

/**
 * This function checks if specified debug messages are enabled or not for the
 * specified module.  It will consider the settings for ALL modules as well.
 * This function allows for fairly flexible combinations of checks:
 * - Check both PRINT messages enabled:
 *    OI_CheckDebugControl(module, OI_DBG_MSG_PRINT_ENABLE, 0)
 *
 * - Check either PRINT messages enabled:
 *    OI_CheckDebugControl(module, 0, OI_DBG_MSG_PRINT_ENABLE)
 *
 * - Check both PRINT and either TRACE messages enabled:
 *    OI_CheckDebugControl(module, OI_DBG_MSG_PRINT_ENABLE, OI_DBG_MSG_TRACE_ENABLE)
 *
 *
 * @param module            module to look up
 * @param allMsgEnable      all specified messages must be enabled
 * @param oneMsgEnable      at least one of the specified messages must be enabled
 *
 * @returns TRUE if at least one of the specified flags are enabled
 */
OI_BOOL OI_CheckDebugControl(OI_MODULE module,
                             OI_UINT8 allMsgEnable,
                             OI_UINT8 oneMsgEnable);

/**
 * Parses a debug, dump or sniff command string and performs appropriate action.
 *
 * @param command  A null terminated string possibly containing a debug, dump
 *                 test or sniff command.
 *
 * @return  OI_OK if the command was parsed
 *          OI_STATUS_INVALID_COMMAND if not a dump, sniff, or debug command
 *          OI_STATUS_INVALID_PARAMETERS if the debug module name is unknown
 *          OI_STATUS_PARSE_ERROR if command was recognized but badly formed.
 *
 *
 */
OI_STATUS OI_Parse_DebugCommand(const OI_CHAR *command);


/**  ******  THIS API IS FOR INTERNAL TESTING USE ONLY ********
 *
 * Parses a test command string and performs appropriate action.
 *
 *
 * @param command  A null terminated string possibly containing a test command.
 *
 * @return  OI_OK if the command was parsed/consumed
 *          OI_STATUS_INVALID_COMMAND if was not a test command
 */
OI_STATUS OI_Parse_TestCommand(const OI_CHAR *command);


#ifdef __cplusplus
}
#endif

/**@}*/

#endif /* _OI_DEBUGCONTROL_H */

