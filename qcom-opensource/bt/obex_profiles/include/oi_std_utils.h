#ifndef _UTILS_H
#define _UTILS_H

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
 * This file contains utilities that are commonly found in stdlib libraries.
 */

#include "oi_stddefs.h"
#include "oi_bt_spec.h"
#include "oi_string.h"
#include "oi_utils.h"

/** \addtogroup Misc_Internal */
/**@{*/

#ifdef __cplusplus
extern "C" {
#endif


extern const OI_BD_ADDR OI_ZeroAddr;

void OI_StrToUpper(OI_CHAR *str);

/** Convert a character to upper case. */
#define OI_toupper(c) ( ((c) >= 'a') && ((c) <= 'z') ? ((c) - 32) : (c) )

/**
 * This macro initializes a BD_ADDR. This currently zeros all bytes.
 */

#define INIT_BD_ADDR(addr)      OI_MemZero((addr), OI_BD_ADDR_BYTE_SIZE)

#define OI_IS_ZERO_ADDR(addr)  SAME_BD_ADDR((addr), &OI_ZeroAddr)

/*****************************************************************************/
#ifdef __cplusplus
}
#endif

/**@}*/

#endif /* _UTILS_H */

