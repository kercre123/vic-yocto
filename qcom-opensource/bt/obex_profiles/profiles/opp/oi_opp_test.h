#ifndef _OPP_TEST_H
#define _OPP_TEST_H

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

#include "oi_stddefs.h"

#ifdef __cplusplus
extern "C" {
#endif

#ifdef OI_TEST_HARNESS

/**
 * OPP Test Names. Used to trigger test behavior for qualification testing.
 */
typedef enum {
    TEST_OPP_NONE = 0,
    TEST_OPP_MISSING_SERVER_PULL_HEADER
} OI_OPP_TESTNAME;

/**
 * Set the current OPP test
 */
void OI_OPP_SetTestName(OI_OPP_TESTNAME testname);

/**
 * Get the current OPP test
 */
OI_OPP_TESTNAME OI_OPP_GetTestName(void);

/*
 * Init the test param
 */
OI_STATUS OI_OPP_Test_Init(void);


#endif

#ifdef __cplusplus
}
#endif

#endif /* _OPP_TEST_H */
