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
 * @file
@internal
    This file contains the global initialization table and access routines.

 */

#define __OI_MODULE__ OI_MODULE_SUPPORT

#include "oi_core_common.h"
#include "oi_modules.h"
#include "oi_init_flags.h"
#include "oi_bt_module_init.h"

/* The initialization table is an array of initialization flags.  It is global in
    release mode, so that applications can efficiently access the flags (via macros). */

#ifdef OI_DEBUG

static OI_INIT_FLAG OI_GV_InitializationFlags[OI_NUM_STACK_MODULES] ;

#else

OI_INIT_FLAG OI_GV_InitializationFlags[OI_NUM_STACK_MODULES] ;

#endif

/*************************************************************

    Reset all initialization flags

    Know what you're doing if you call this function.

    It should only be called as part of a global, reset/restart process.

*************************************************************/


void OI_InitFlags_ResetAllFlags(void)
{
    //OI_DBGPRINT(("OI_InitFlags_ResetAllFlags - resetting all initialization flags")) ;
    OI_MemSet(OI_GV_InitializationFlags, 0, sizeof(OI_GV_InitializationFlags)) ;
}
/*************************************************************

    Initialization flag access

*************************************************************/

OI_INIT_FLAG OI_InitFlags_GetFlag(OI_MODULE module)
{
    OI_ASSERT(module < OI_ARRAYSIZE(OI_GV_InitializationFlags)) ;
    if (!(module < OI_ARRAYSIZE(OI_GV_InitializationFlags))) {
        return 0;
    }
    return (OI_GV_InitializationFlags[module]) ;
}

void OI_InitFlags_PutFlag(OI_INIT_FLAG flagValue, OI_MODULE module, OI_MODULE callingModule)
{
    OI_ASSERT(module == callingModule);
    OI_ASSERT(module < OI_ARRAYSIZE(OI_GV_InitializationFlags)) ;
    if (!(module < OI_ARRAYSIZE(OI_GV_InitializationFlags))) {
        return;
    }
    if (OI_GV_InitializationFlags[module] == OI_INIT_FLAG_UNINITIALIZED_VALUE) {
        if (flagValue != OI_INIT_FLAG_UNINITIALIZED_VALUE) {
            // was uninitialized, changing to initialized
            if (module != OI_MODULE_SUPPORT) { /* avoid an infinite recursion when initializing the debugprint code */
                /* OI_DBGPRINT(("%s init flag changed from uninitialized to %d\n", ModuleToString(module), flagValue)) ; */
            }
        }
    }
    else if (flagValue == OI_INIT_FLAG_UNINITIALIZED_VALUE) {
        // was initialized, changing to uninitialized
        if (module != OI_MODULE_SUPPORT) { /* avoid an infinite recursion when initializing the debugprint code */
            /* OI_DBGPRINT(("%s init flag changed from %d to uninitialized\n",_ModuleToString(module), OI_GV_InitializationFlags[module])) ; */
        }
    }
    OI_GV_InitializationFlags[module] = flagValue ;
}

void OI_InitFlags_Increment(OI_MODULE module)
{
    OI_ASSERT(module < OI_ARRAYSIZE(OI_GV_InitializationFlags)) ;
    if (!(module < OI_ARRAYSIZE(OI_GV_InitializationFlags))) {
        return;
    }
    ++OI_GV_InitializationFlags[module];
}

void OI_InitFlags_Decrement(OI_MODULE module)
{
    OI_ASSERT(module < OI_ARRAYSIZE(OI_GV_InitializationFlags)) ;
    if (!(module < OI_ARRAYSIZE(OI_GV_InitializationFlags))) {
        return;
    }
    OI_ASSERT(OI_GV_InitializationFlags[module]);
    --OI_GV_InitializationFlags[module];
}

OI_BOOL OI_InitFlags_AllUninitialized(void)
{
    OI_UINT  i ;

    for (i = 0; i < OI_ARRAYSIZE(OI_GV_InitializationFlags); ++i) {
        if (OI_INIT_FLAG_UNINITIALIZED_VALUE != OI_GV_InitializationFlags[i]) {
            return(FALSE) ;
        }
    }
    return(TRUE) ;
}
/*****************************************************************************/




