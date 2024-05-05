#ifndef _MEMMGR_H
#define _MEMMGR_H

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
 *  This header file provides memory management functions.
 *  The memory allocator assumes that all dynamic memory is allocated from a fixed
 *  size pool. For a desktop environment this pool can be large, but for embedded
 *  applications this pool may be quite small. A block-based allocation scheme is used to
 *  prevent memory fragmentation. The size and number of blocks of each allocation
 *  size will be tuned to the needs of the specific embedded application.
 *
 *  See @ref memMgr_docpage for more information on the basic function of the Memory Manager.
*/

#include "oi_stddefs.h"
#include "oi_status.h"
#include "oi_debug.h"
#include "oi_string.h"
#include "oi_cpu_dep.h"

#ifdef OI_USE_NATIVE_MALLOC
#include <stdlib.h>
#endif

/** \addtogroup MemMgr_Internal */
/**@{*/

#ifdef __cplusplus
extern "C" {
#endif


/*
 * IF OI_DEBUG is defined then MEMMGR is compiled for debugging and memory
 * profiling. If OI_DEBUG is not defined memmgr debugging and profiling can be
 * turned on indepdentently by defining MEMMGR_DEBUG and MEMMGR_PROFILE
 */

#ifdef OI_DEBUG
    #define MEMMGR_DEBUG
    #ifndef SMALL_MEMORY
        #define MEMMGR_PROFILE
    #endif
#endif

/*
 * Cannot use memory manager debugging or profiling is using native malloc
 * and using native malloc implies use native memcpy.
 */
#ifdef OI_USE_NATIVE_MALLOC
#undef MEMMGR_DEBUG
#undef MEMMGR_PROFILE
#endif

/*
 * OI_Malloc, OI_Calloc
 *
 * Allocates the smallest available memory block that is large enough to hold
 * the specified number of bytes. Returns 0 if there is no memory block of the
 * required size available
 *
 * For debugging, OI_Malloc is a macro which adds file and line number to the
 * malloc call so that we can trace memory hogs and memory leaks.
 *
 * OI_Calloc does the allocation and then zeroes the memory before returning it.
 */

#ifdef OI_USE_NATIVE_MALLOC

#define OI_Malloc(size)        malloc((size))
#define OI_Calloc(size)        calloc((size), 1)
#define OI_Free(x)             free((x))
#define OI_StaticMalloc(size)  calloc((size),1)
#define OI_MallocWillFail(x)   (FALSE)
#define OI_FreeIf(x)           do{if (*(x) != NULL) { free(*(x)); *(x) = NULL;}} while(0)

#else /* OI_USE_NATIVE_MALLOC */

#ifdef MEMMGR_DEBUG

#define OI_Malloc(size) _OI_Malloc_Dbg((size), OI_CURRENT_MODULE, __FILE__, (OI_UINT16)(__LINE__))
#define OI_Calloc(size) _OI_Calloc_Dbg((size), OI_CURRENT_MODULE, __FILE__, (OI_UINT16)(__LINE__))

void* _OI_Malloc_Dbg(OI_INT32 size, OI_UINT8 module, OI_CHAR *fname, OI_UINT16 lineNum);
void* _OI_Calloc_Dbg(OI_INT32 size, OI_UINT8 module, OI_CHAR *fname, OI_UINT16 lineNum);

#else

#define OI_Malloc(size) _OI_Malloc(size)
void* _OI_Malloc(OI_INT32 size);

#define OI_Calloc(size) _OI_Calloc(size)
void* _OI_Calloc(OI_INT32 size);

#endif /* MEMMGR_DEBUG */

/**
 * A memory allocation failure is usually a fatal internal stack error
 * but there are some cases where the size of the block being
 * allocated is based on data received from a remote device and a NO
 * RESOURCES error should be reported.
 *
 * It is still important to check OI_Malloc or OI_Calloc return values for
 * NULL, for correct error handling when BM3 is built with OI_USE_NATIVE_MALLOC.
 *
 * Check if a call to OI_Malloc (or OI_Calloc) would fail.
 *
 * @param size  The size of the memory block to check
 *
 * @return TRUE if a call to OI_Malloc (or OI_Calloc) would fail
 */
OI_BOOL OI_MallocWillFail(OI_INT32 size);

/*
 * OI_StaticMalloc
 *
 * Allocates memory from the static allocation pool. This is data that is
 * required for the stack to function and will never explicitly freed.
 *
 * The memory allocated will be zero'd before returning to caller.
 */

#ifdef MEMMGR_DEBUG

#define OI_StaticMalloc(size) _OI_StaticMalloc_Dbg((size), OI_CURRENT_MODULE, __FILE__, (OI_UINT16)(__LINE__))

void* _OI_StaticMalloc_Dbg(OI_INT32 size, OI_UINT8 module, OI_CHAR *fname, OI_UINT16 lineNum);

#else

#define OI_StaticMalloc(size) _OI_StaticMalloc(size)
void* _OI_StaticMalloc(OI_INT32 size);

#endif /* MEMMGR_DEBUG */


/*
 * OI_Free
 *
 * Returns a memory block to the memory pool and makes it available for
 * subsequent allocations. Fails with an assertion error if the memory was not
 * allocated by OI_Malloc or if the memory block has become corrupt.
 *
 * OI_FreeIf
 *
 * Passed a reference to a pointer to a dynamic memory block. If the value
 * referenced is non-null, frees the memory and sets the referenced value to
 * null.
 */

#ifdef MEMMGR_DEBUG

#define OI_Free(block) _OI_Free_Dbg((block), OI_CURRENT_MODULE, __FILE__, (OI_UINT16)(__LINE__))
void _OI_Free_Dbg(void *block, OI_UINT8 module, OI_CHAR *fname, OI_UINT16 lineNum);

#define OI_FreeIf(p) \
    do { \
        _OI_FreeIf_Dbg(*(p), OI_CURRENT_MODULE, __FILE__, (OI_UINT16)(__LINE__)); \
        *(p) = NULL; \
    } while (0)
void _OI_FreeIf_Dbg(const void *blockRef, OI_UINT8 module, OI_CHAR *fname, OI_UINT16 lineNum);

#else

#define OI_Free(block) _OI_Free(block)
void _OI_Free(void *block);

#define OI_FreeIf(p)  do { _OI_FreeIf(*(p)); *(p) = NULL; } while (0)
void _OI_FreeIf(const void *blockRef);

#endif /* MEMMGR_DEBUG */

#endif /* OI_USE_NATIVE_MALLOC */


/**
 * OI_MEMMGR_Check, OI_MEMMGR_CheckSize
 *
 * For debugging only
 *
 * Checks the memory address is a valid memory block address and that has been
 * allocated (i.e. not on the free list) and checks that the contents at that
 * memory block has not been corrupted.
 *
 * Returns TRUE if the address and contents are ok, FALSE otherwise.
 *
 * OI_MEMMGR_CheckSize also checks that the allocated storage for the memory block
 * is >= Size.
 *
 * Intended for use with the ASSERT macro, for example:
 *
 *     OI_ASSERT(OI_MEMMGR_Check(ptr));
 *     OI_ASSERT(OI_MEMMGR_CheckSize(buffer, 96));
 */

#ifdef MEMMGR_DEBUG

OI_BOOL OI_MEMMGR_Check(void const *Block);

OI_BOOL OI_MEMMGR_CheckSize(void const *Block,
                            OI_UINT32 Size);

/**
 * How much dynamic memory is currently allocated
 */
OI_UINT32 OI_MEMMGR_CurrentDynamicAllocation(void);

/*
 * OI_MaxAllocBlock
 *
 * Returns the size of the largest memory block currently available for
 * allocation by a OI_Malloc
 */
OI_UINT32 OI_MaxAllocBlock(void);


/*
 * OI_MinAllocBlock
 *
 * Returns the size of the smallest memory block currently available for
 * allocation by a OI_Malloc
 */
OI_UINT32 OI_MinAllocBlock(void);

#else

#define OI_MEMMGR_Checksum(B)        (0)

#define OI_MEMMGR_Check(B)        (TRUE)

#define OI_MEMMGR_CheckSize(B, S) (TRUE)

#define OI_MEMMGR_CurrentDynamicAllocation()  (0)

#define OI_MaxAllocBlock() (0)

#define OI_MinAllocBlock() (0)

#endif /* MEMMGR_DEBUG */

#ifdef __cplusplus
}
#endif

/**@}*/

/*****************************************************************************/
#endif /* _MEMMGR_H */

