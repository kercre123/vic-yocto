#ifndef _ARGCHECK_H
#define _ARGCHECK_H

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

This file provides a macro for validating function arguments.
*/

#include "oi_assert.h"
#include "oi_status.h"

/** \addtogroup Debugging */
/**@{*/

#ifdef __cplusplus
extern "C" {
#endif


/**
 * The OI_ARGCHECK macro is to be used for checking function arguments in
 * public interfaces. It takes a conditional as an argument and
 * succeeds if the conditional is TRUE. If the conditional is FALSE,
 * an assertion is perfomed, and OI_STATUS_INVALID_PARAMETERS is returned.
 * Assertions can be disabled by not defining OI_DEBUG.
 *
 * If OI_SUPPRESS_ARGCHECK is defined, no argument checking is performed.
 * This allows us to use strong argument checking when the caller may
 * not be known (e.g., under Windows) and use assertions for debugging,
 * but leave the code out of the release build, when the stack and
 * application are tightly coupled (e.g., a deeply embedded system).
 *
 * Note that the documentation for BLUEmagic 3.0 software assumes that
 * OI_SUPPRESS_ARGCHECK is NOT defined. To leave argument checking code
 * out of build, pass -DOI_SUPPRESS_ARGCHECK on your compiler command line.
 */

#ifndef OI_SUPPRESS_ARGCHECK
#define OI_ARGCHECK(x) {OI_ASSERT((x)); if (!(x)) return OI_STATUS_INVALID_PARAMETERS; }
#else
#define OI_ARGCHECK(x)
#endif

#ifdef __cplusplus
}
#endif

/**@}*/

#endif /* _ARGCHECK_H */


