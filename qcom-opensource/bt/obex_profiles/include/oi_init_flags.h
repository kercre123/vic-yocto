#ifndef _OI_INIT_FLAGS_H
#define _OI_INIT_FLAGS_H

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
 * Initialization flags and access routines
 *
 * This file contains initialization flags for each 'module' in the BM3 stack.  Modules are enumerated
 * in a separate file (oi_modules.h).
 *
 * This file also contains access routines and macros for the initialization flags.
 *
 * When compiled for debug, init flags are accessed by an accessor function which verifies
 * consistency of the initialization table.
 *
 * When compiled for release, init flags are accessed directly.
 */

#include "oi_stddefs.h"
#include "oi_modules.h"
#include "oi_debug.h"
#include "oi_assert.h"

/** \addtogroup Misc_Internal */
/**@{*/

#ifdef __cplusplus
extern "C" {
#endif


/*************************************************************

    What is an initialization flag?
    An init flag is a 32-bit value.
    Zero = not initialized
    Non-zero = initialized.  Any further meaning is up to the particular module to whom
        the init flag belongs.  It might be a pointer, a reference counter or simply a boolean.
*************************************************************/

typedef OI_UINT32   OI_INIT_FLAG;

#define OI_INIT_FLAG_UNINITIALIZED_VALUE     (0)

/*************************************************************

    void OI_InitFlags_ResetAllFlags(void) ;
    Reset all initialization flags
    Know what you're doing if you call this function.
    It should only be called as part of a global, reset/restart process.
    This function is defined in oi_bt_statck_init.h in order to be available to applications
*************************************************************/
/*************************************************************

    Initialization flag access
    OI_INIT_FLAG_VALUE (module)
        Returns the initialization flag for the specified module.  If the flag is non-zero,
        the actual type is known only to the caller.
        @param module   Specify module name without the OI_MODULE_ prefix.
                        Example:
                             OI_INIT_FLAG_VALUE (DEVMGR)
                        not:
                             OI_INIT_FLAG_VALUE (OI_MODULE_DEVMGR)

        @return         Value of the module's initialization flag

    OI_INIT_FLAG_PUT_FLAG (flagValue, module)
        Macro saves the caller's flag in the initialization table.
        @param flagValue    new initialization flag value.  Zero means that the module is
                            uninitialized, any non-zero value means module has been initialized
        @param module   Specify module name without the OI_MODULE_ prefix.
                        Example:
                             OI_INIT_FLAG_PUT_FLAG (TRUE, DEVMGR)
                        not:
                             OI_INIT_FLAG_PUT_FLAG (TRUE, OI_MODULE_DEVMGR)
*************************************************************/

/* Accessor functions */
extern OI_INIT_FLAG OI_InitFlags_GetFlag(OI_MODULE module) ;
extern void         OI_InitFlags_PutFlag(OI_INIT_FLAG flagValue, OI_MODULE module, OI_MODULE callingModule) ;
extern void         OI_InitFlags_Increment(OI_MODULE module) ;
extern void         OI_InitFlags_Decrement(OI_MODULE module) ;
extern OI_BOOL      OI_InitFlags_AllUninitialized(void) ;


#ifdef OI_DEBUG
    /* In debug mode, configuration table access is via an accessor function to allow for additional
        error checking. */
#define OI_INIT_FLAG_VALUE(module)               (OI_InitFlags_GetFlag(OI_MODULE_##module))
#define OI_INIT_FLAG_PUT_FLAG(flagValue, module) (OI_InitFlags_PutFlag(flagValue, OI_MODULE_##module, __OI_MODULE__))
#define OI_INIT_FLAG_INCREMENT(module)           (OI_InitFlags_Increment(OI_MODULE_##module))
#define OI_INIT_FLAG_DECREMENT(module)           (OI_InitFlags_Decrement(OI_MODULE_##module))

#else
    /* In release mode, configuration table is accessed directly.
        Init flags are in a global array, module # is index into the array */

extern OI_INIT_FLAG OI_GV_InitializationFlags[] ;

#define OI_INIT_FLAG_VALUE(module)               (OI_GV_InitializationFlags[OI_MODULE_##module])
#define OI_INIT_FLAG_PUT_FLAG(flagValue, module) OI_GV_InitializationFlags[OI_MODULE_##module] = (OI_INIT_FLAG)flagValue
#define OI_INIT_FLAG_INCREMENT(module)           (++OI_GV_InitializationFlags[OI_MODULE_##module])
#define OI_INIT_FLAG_DECREMENT(module)           (--OI_GV_InitializationFlags[OI_MODULE_##module])

#endif


/** Preamble for external APIs that do not initialize the module.
    Return an error if not initialized.
 */
#define OI_CHECK_INIT(x) if (!OI_INIT_FLAG_VALUE(x)) return OI_STATUS_INVALID_STATE;

/** Preamble for external APIs that initialize the module.
    Initialize the module if not initialized. */
#define OI_ENSURE_INIT(x) \
{ \
    OI_STATUS result; \
    if (!OI_INIT_FLAG_VALUE(x)) { \
       result = internalInit(); \
       if (!OI_SUCCESS(result)) { \
           return result; \
       } \
    } \
}
/*****************************************************************************/
#ifdef __cplusplus
}
#endif

/**@}*/

#endif /* _OI_INIT_FLAGS_H */

