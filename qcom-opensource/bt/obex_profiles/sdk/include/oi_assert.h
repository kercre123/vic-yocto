#ifndef _OI_ASSERT_H
#define _OI_ASSERT_H

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

/** @file
  This file provides macros and functions for compile-time and run-time assertions.

  When the OI_DEBUG preprocessor value is defined, the macro OI_ASSERT is compiled into
  the program, providing for a runtime assertion failure check.
  C_ASSERT is a macro that can be used to perform compile time checks.
*/

/** \addtogroup Debugging Debugging APIs */
/**@{*/

#ifdef __cplusplus
extern "C" {
#endif


#ifdef OI_DEBUG

/** The macro OI_ASSERT takes a condition argument. If the asserted condition
    does not evaluate to true, the OI_ASSERT macro calls the host-dependent function,
    OI_AssertFail(), which reports the failure and generates a runtime error.
*/
void OI_AssertFail(char* file, int line, char* reason);


#define OI_ASSERT(condition) \
    { if (!(condition)) OI_AssertFail(__FILE__, __LINE__, #condition); }

#define OI_ASSERT_FAIL(msg) \
    { OI_AssertFail(__FILE__, __LINE__, msg); }

#else


#define OI_ASSERT(condition)
#define OI_ASSERT_FAIL(msg)

#endif


/**
   C_ASSERT() can be used to perform many compile-time assertions: type sizes, field offsets, etc.
   An assertion failure results in compile time error C2118: negative subscript.
   Unfortunately, this elegant macro doesn't work with GCC, so it's all commented out
   for now. Perhaps later.....
*/

#ifndef C_ASSERT
// #define C_ASSERT(e) typedef char __C_ASSERT__[(e)?1:-1]
// #define C_ASSERT(e)
#endif


/*****************************************************************************/
#ifdef __cplusplus
}
#endif

/**@}*/

#endif /* _OI_ASSERT_H */

