#ifndef _TEST_H
#define _TEST_H

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
@file
@internal

 This file defines internal module test APIs.
*/

#include "oi_status.h"
#include "oi_assert.h"

/** \addtogroup Debugging_Internal */
/**@{*/

#ifdef __cplusplus
extern "C" {
#endif

#ifdef OI_TEST_HARNESS

/**
 * Pass a test command to core stack module.
 * All modules take same arguments and return same values.
 *
 * @param cmd  Pointer to character string for the command
 *
 * @return OI_OK or
 *         OI_STATUS_PARSE_ERROR if the command is recognized but invalid
 */
OI_STATUS OI_L2CAP_Test(const OI_CHAR *cmd);
OI_STATUS OI_SDPSRV_Test(const OI_CHAR *cmd);
OI_STATUS OI_HCI_Test(const OI_CHAR *cmd);

#endif /* OI_TEST_HARNESS */

/**
 *  Test function type for dynamically-registered tests
 *  Same calling conventions as core stack test modules above
 */

typedef OI_STATUS (*OI_TEST_COMMAND_HANDLER)(const OI_CHAR *cmd);


/**
 *  Register a test command handler
 *
 *  @param  cmdHandler  the callback to be called for input "test <moduleId> ....."
 *
 *  @param  moduleId    string uniquely identifying the command handler, typically
 *                      same as module name.
 *
 *  @return OI_OK if handler was registered successfully.
 */

OI_STATUS OI_TEST_RegisterHandler(OI_TEST_COMMAND_HANDLER   cmdHandler,
                                  const OI_CHAR             *moduleId);


#ifdef __cplusplus
}
#endif

/**@}*/

#endif /* _TEST_H */

