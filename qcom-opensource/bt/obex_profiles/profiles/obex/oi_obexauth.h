#ifndef _OBEXAUTH_H
#define _OBEXAUTH_H

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

OBEX authentication support
*/

#include "oi_stddefs.h"
#include "oi_status.h"
#include "oi_obexspec.h"
#include "oi_obexcommon.h"
#ifdef __cplusplus
extern "C" {
#endif



/**
 * Resets OBEX authentication state and frees any dynamically allocated memory.
 */

void OI_OBEXAUTH_Reset(OBEX_COMMON *common);


/**
 * Saves the password string.
 */

OI_STATUS OI_OBEXAUTH_SaveAuthInfo(OBEX_COMMON *common,
                                   const OI_BYTE userId[OI_OBEX_MAX_USERID_LEN],
                                   OI_UINT8 userIdLen,
                                   const OI_CHAR *password);

/**
 * Saves an OBEX authentication challenge digest string.
 */

OI_STATUS OI_OBEXAUTH_SaveChallenge(OBEX_COMMON *common,
                                    OI_OBEX_HEADER *challenge);


/**
 * Saves an OBEX authentication response digest string.
 */

OI_STATUS OI_OBEXAUTH_SaveResponse(OBEX_COMMON *common,
                                   OI_OBEX_HEADER *response);


/**
 * Generate the authentication response to an previous authentication challenge.
 */

void OI_OBEXAUTH_ComposeResponse(OBEX_COMMON *common,
                                 OI_OBEX_HEADER *response);


/**
 * Composes an authentication challenge header.
 */

void OI_OBEXAUTH_ComposeChallenge(const OI_CHAR* privateKey,
                                  const OI_OBEX_REALM* realm,
                                  OBEX_COMMON *common,
                                  OI_OBEX_HEADER *challenge);


/**
 * Checks that correct response was received to an OBEX authentication
 * challenge.
 */

OI_STATUS OI_OBEXAUTH_Authenticate(OBEX_COMMON *common);

#ifdef __cplusplus
}
#endif

#endif

/**@}*/
