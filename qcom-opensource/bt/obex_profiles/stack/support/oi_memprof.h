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
 * @internal
 *
 * functions to record the use of memory by the Memory Manager
 *
 */

#ifndef _MEMPROF_H
#define _MEMPROF_H

#include "oi_stddefs.h"

/** \addtogroup MemMgr_Internal */
/**@{*/

/*
 * Cannot do memory profiling if using native malloc.
 */
#ifdef OI_USE_NATIVE_MALLOC
#undef MEMMGR_PROFILE
#endif


#ifdef MEMMGR_PROFILE

#define MEMPROF_INIT(x)                 OI_MemProf_Init(x)
#define MEMPROF_STATIC_MALLOC(s, m, f)  OI_MemProf_StaticMalloc(s, m, f)
#define MEMPROF_MALLOC(s, a, m, f)      OI_MemProf_Malloc(s, a, m, f)
#define MEMPROF_FREE(s, a, m, f)        OI_MemProf_Free(s, a, m, f)

OI_STATUS OI_MemProf_Init(OI_INT debug_overhead);


void OI_MemProf_StaticMalloc(OI_INT32 size,
                             OI_UINT8 module,
                             OI_CHAR *fileName);


void OI_MemProf_Malloc(OI_INT32 size,
                       void* addr,
                       OI_UINT8 module,
                       OI_CHAR *fileName);


void OI_MemProf_Free(OI_INT32 size,
                     void* addr,
                     OI_UINT8 module,
                     OI_CHAR *filename);

#else

#define MEMPROF_INIT(x)
#define MEMPROF_STATIC_MALLOC(s, m, f)
#define MEMPROF_MALLOC(s, a, m, f)
#define MEMPROF_FREE(s, a, m, f)

#endif /* MEMMG_PROFILE */


#endif /* _MEMPROF_H */

/**@}*/
