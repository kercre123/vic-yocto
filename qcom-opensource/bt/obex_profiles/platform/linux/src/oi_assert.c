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
Linux implementation of runtime error checking interface defined in oi_assert.h
*/

#define __OI_MODULE__ OI_MODULE_SUPPORT

#ifndef LINUX
    #error This file is targeted for LINUX only!!!
#endif


#include <stdlib.h>
#include <stdio.h>
#include "oi_assert.h"
#include "oi_utils.h"
#include "oi_debug.h"

int OI_AssertExpected = 0;

#define OI_ASSERT_FAILURE   (0)

void OI_AssertFail(char* file, int line, char* reason)
{
    if (OI_AssertExpected) {
        OI_Printf("AssertFail expected %s@%d : %s\n", file, line, reason);
        --OI_AssertExpected;
    } else {
        int i = 0;
        OI_Printf("AssertFail %s@%d : %s\n", file, line, reason);
        fflush(stdout);
        /*
         * Allow debugging
         */
        i = i / i;  /* Force division by zero to raise an exception */
    }
}






/*****************************************************************************/
