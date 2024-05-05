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
@file
@internal

This file implements memory management functions.
*/

#define __OI_MODULE__ OI_MODULE_MEMMGR

#include "oi_common.h"
#include "oi_debug.h"
#include "oi_std_utils.h"
#include "oi_memmgr.h"
#include "oi_time.h"
#include "oi_memprof.h"
#include "oi_debugcontrol.h"
#include "oi_support_init.h"
#include "oi_debug.h"
#include "oi_dump.h"

#include "oi_init_flags.h"

/*
 * Use OI_Printf instead of OI_DBGPRINT if we are not compiled for debugging.
 */
#ifndef OI_DEBUG
#undef OI_DBGPRINT
#undef OI_DBGPRINT2
#define OI_DBGPRINT(s)    OI_Printf s
#define OI_DBGPRINT2(s)   OI_Printf s
#endif


/*
 * Comment out the whole file if we are using the native malloc
 */
#ifndef OI_USE_NATIVE_MALLOC

/************************************************************************************

  Memory Heap

 ************************************************************************************/
/*
 *  All memory is managed by the memory manager resides in a heap which is
 *  passed to the mm in the initialization call.  Memory is staticMalloc'd from
 *  the heap and never returned.  Dynamic memory allocation is also from the
 *  heap, but via the pool mechanism.
 *
 *  The heap is managed with a 'current' and 'end' pointers.  The current
 *  pointer points at the next available memory location in the heap.  The end
 *  pointer points at the first memory location past the end of the heap.
 */

static OI_BYTE *heapCurrentPtr;
static OI_BYTE *heapEndPtr;

/************************************************************************************

  Memory Pools

 ************************************************************************************/

/*
 * A sanity check - no sane person would ever allocate 100 different size pools.
 */

#define MEMMGR_MAX_POOL_COUNT 100


/*
 * Forward declaration of ALLOC_BLOCK
 */

typedef struct _ALLOC_BLOCK ALLOC_BLOCK;

#ifdef MEMMGR_DEBUG

/*
 * The guard string helps to detect certain classes of memory bounds overrun
 * errors. It also helps to identify the start of memory blocks when debugging
 * raw memory.  The guard string length must be a multiple of 4 bytes to
 * maintain proper memory alignment of the ALLOC_BLOCK structure.
 */
static const OI_BYTE GUARD_STRING[] = "!QTI!!";

static OI_BOOL CheckGuardString(ALLOC_BLOCK const *blockPtr);

void *last_alloc;   /* the last block allocated */

void *last_free;    /* the last block freed */

/*
 * Add extra to end of each block to test for overshoot
 */
#define DEBUG_EXTRA 4

/*
 * Verify that allocated memory extends at least size bytes beyond addr.
 */
#define CHECK_EXTENT(addr, size)  OI_ASSERT(OI_MEMMGR_CheckSize(addr, size))

#else

#define CHECK_EXTENT(addr, size)
static OI_BYTE debug_out = 0;

/*
 * No Overshoot detection in Release Builds
 */
#define DEBUG_EXTRA 0

#endif /* MEMMGR_DEBUG */


/*
 * Type ALLOCBLOCK
 *
 * Raw memory blocks are cast to this type when linking and unlinking from the
 * free list. If MEMMGR_DEBUG is TRUE additional fields are allocated for
 * profiling and error checking.
 *
 * The actual allocated size is used for error checking when clearing or copying
 * memory and is also used to tune memory allocation for embedded applications.
 *
 * The Guard field is used to detect memory corruption errors due to bounds
 * errors. A fairly large guard field is more reliable at catching such errors
 * and use of an identifiable string makes it easier to debug other memory
 * allocation errors.
 */


struct _ALLOC_BLOCK {
#ifdef MEMMGR_DEBUG
    struct {
        OI_BYTE   guard[sizeof(GUARD_STRING)]; /* Detects memory corruption */
        OI_UINT32 allocSize;                   /* Actual allocation size */
        OI_TIME   allocTime;                   /* Timestamp for the time the block was allocated */
        OI_CHAR  *fname;                       /* File name of allocator */
        OI_CHAR  *freeFile;                    /* File name of last freer */
        OI_UINT16 lineNum;                     /* File line number of allocator */
        OI_UINT16 freeLine;                    /* File line number of last freer */
        OI_BYTE   free;                        /* Has the block been freed? */
        OI_UINT8  module;                      /* Module that last Malloc'd block */
        OI_UINT8  alignment[2];                /* (Unused) Force 32 bit alignment on 16 bit platforms */
        ALLOC_BLOCK *priorPool;                /* Prior Pool location for Guard String coruption */
    } MemDebug;
#endif
    ALLOC_BLOCK *next;     /* Link to next block in free list */
};


/*
 * The size of a block header (with debugging off this should be zero)
 */

#define BLOCK_HEADER_SIZE  (sizeof(ALLOC_BLOCK) - sizeof(ALLOC_BLOCK*))


/*
 * Macro to get the start of an alloc block
 */

#define GET_ALLOC_BLOCK(b) ((ALLOC_BLOCK*) ((OI_BYTE *) (b) - BLOCK_HEADER_SIZE))



/*
 * Type MEMPOOL
 *
 * A memory pool is a linked list of memory blocks of the same size. The
 * FreeList pointer is a pointer to a linked list of unallocated ALLOCBLOCKS.
 *
 * In production use OI_Malloc will always allocate the first block in the free
 * list, and OI_Free will always add a freed block to the head of the list. For
 * debugging we mix things up to help flush out memory allocation bugs.
 */

typedef struct {
    OI_BYTE     *LAddr;    /* Lowest memory address allocated to this pool */
    OI_BYTE     *HAddr;    /* Highest memory address allocated to this pool */
    ALLOC_BLOCK *freeList; /* Pointer to first free memory allocation block */
#ifdef MEMMGR_DEBUG
    OI_UINT32   poolSize;  /* Allocated size for every block in this pool */
#endif /* MEMMGR_DEBUG */
} MEM_POOL;


/*
 * The pool table is array of memory pools of various sizes as specified in the
 * pool configuration array. In a production system MEMMGR_MAX_POOL_COUNT should
 * be equal the exact number of bytes specified in the pool configuration table
 */

static MEM_POOL *PoolTable;


/*
 * The heap is a statically allocated contiguous block of memory. All memory
 * blocks are allocated from the heap. Same size blocks in memory pools are
 * allocated contiguously. This means that we can tell which pool a memory block
 * is allocated from by comparing the memory block address to the low and high
 * bounds of entries in the Pool_Table.
 */

static OI_BYTE *PoolHeap;

#ifdef MEMMGR_DEBUG

static OI_BYTE *PoolHeapEnd;

/*
 * Test if an address references data allocated from the pool heap
 */
#define IS_DYNAMIC_ALLOC(a)   ((((const OI_BYTE*)(a)) >= PoolHeap) && (((const OI_BYTE*)(a)) < PoolHeapEnd ))

static OI_UINT32 MEMMGR_ValidateAllocBlock(void const *block);
static OI_UINT32 MEMMGR_ValidateFreeBlock(ALLOC_BLOCK *blockPtr, OI_UINT32 poolsize);
#endif


/*
 * Test if an address references data allocated from a specific pool
 */

#define IS_FROM_POOL(a, p) ((((const OI_BYTE*)(a) >= PoolTable[p].LAddr)) && ((const OI_BYTE*)(a) < PoolTable[p].HAddr))


/*
 * Number of pools is established by pool configuragion parameter passed to OI_Memory_Manager_Init
 */

static OI_UINT16 PoolCount;


/*
 * The pool configuration for the pool heap as passed in to OI_MEMMGR_Init
 */

static const OI_MEMMGR_POOL_CONFIG *PoolConfiguration;


/*
 * OI_MEMMGR_Init
 *
 * Allocates blocks for each of the memory pools as specified in the pool
 * configuration parameter. The pool configuration is an array of tuples that
 * specify the block size for memory blocks in a pool and the number of blocks
 * in the pool. All blocks for all pools are allocated from a statically
 * allocated pool heap.
 */

OI_STATUS OI_MEMMGR_Init(const OI_CONFIG_MEMMGR *config)
{
    OI_INT16 numBlocks;
    OI_INT32 blockSize;
    OI_INT32 poolSize;
    OI_INT16 pool;
    OI_INT32 poolHeapSize = 0;
    OI_BYTE *poolHeapPtr;
#ifdef MEMMGR_DEBUG
    OI_INT32 prevBlockSize = 0;    /* to verify pool table ordering  */
    ALLOC_BLOCK *priorPool = NULL;

    /*
     * We want to ensure that the memory allocator always returns a pointer that
     * is aligned on a 32-bit boundary.  OI_Static_Malloc always returns a
     * 4-byte aligned block, so all we need to do is ensure that all block sizes
     * are a multiple of 4 and that the sizeof debug structure (if any) is a
     * multiple of 4.
     */

    OI_ASSERT(0 == (BLOCK_HEADER_SIZE & (3)));
#endif /* MEMMGR_DEBUG */

    OI_ASSERT(!OI_INIT_FLAG_VALUE(MEMMGR));
    if ( OI_INIT_FLAG_VALUE(MEMMGR) ){
        return OI_STATUS_ALREADY_INITIALIZED;
    }

    /*
     * Initialize the overall heap - it will be needed for subsequent allocation
     * of pool heaps.
     */

    OI_ASSERT(NULL != config->heapConfig);
    OI_ASSERT(NULL != config->heapConfig->heap);
    OI_ASSERT(0 != config->heapConfig->heapSize);

    heapCurrentPtr = (OI_BYTE*)config->heapConfig->heap;
    heapEndPtr = heapCurrentPtr + config->heapConfig->heapSize;

    /*
     * Establish the pool configuration that will be used
     */
    PoolConfiguration = config->poolConfig;

    /*
     * Count the pools and calculate the size of the pool heap
     */
    for (pool = 0; pool < MEMMGR_MAX_POOL_COUNT; ++pool) {

        numBlocks = PoolConfiguration[pool].numBlocks;
        blockSize = PoolConfiguration[pool].blockSize;

        /*
         * End of the config table is marked by a pair of zeroes.
         */
        if ((blockSize == 0) && (numBlocks == 0)) {
            PoolCount = pool;
            break;
        }

        OI_ASSERT( 0 == (blockSize & (3)));   /* must be a multiple of 4 */
        OI_ASSERT((blockSize > 0) && (numBlocks > 0));

#ifdef MEMMGR_DEBUG
        OI_ASSERT(blockSize > prevBlockSize); /* verify increasing block size order */
        prevBlockSize = blockSize;
#endif /* MEMMGR_DEBUG */

        /*
         * The pool heap space needed for a block is the block size defined for
         * this pool plus the block header (zero when debugging is off).
         */
        poolSize = blockSize + BLOCK_HEADER_SIZE + DEBUG_EXTRA;
        poolHeapSize += poolSize * numBlocks;
    }

    /*
     * Check Dynamic memory size
     */
    poolSize = poolHeapSize + (sizeof(MEM_POOL) * PoolCount);
    OI_ASSERT( poolSize <= ((OI_INT32) (config->heapConfig->heapSize)) );

    /*
     * Zero memory pool space
     */
    OI_MemZero( heapCurrentPtr, poolSize );

    /*
     * Reserve the memory allocator pool table
     */
    PoolTable = (void *) heapCurrentPtr;
    heapCurrentPtr += (sizeof(MEM_POOL) * PoolCount);
    heapCurrentPtr = (OI_BYTE*)((OI_UINT32)(heapCurrentPtr + 3) & (~3));


    /*
     * Reserve the memory for the pool heap
     */
    PoolHeap = (void *) heapCurrentPtr;
    heapCurrentPtr += poolHeapSize;
    heapCurrentPtr = (OI_BYTE*)((OI_UINT32)(heapCurrentPtr + 3) & (~3));


#ifdef MEMMGR_DEBUG
    PoolHeapEnd = PoolHeap + poolHeapSize;
#endif /* MEMMGR_DEBUG */

    /*
     * Initialize the pool tables
     */
    poolHeapPtr = PoolHeap;
    for (pool = 0; pool < PoolCount; ++pool) {
        OI_INT16 blockNum;
        ALLOC_BLOCK *freeList = NULL;

        numBlocks = PoolConfiguration[pool].numBlocks;
        blockSize = PoolConfiguration[pool].blockSize;

        poolSize = blockSize + BLOCK_HEADER_SIZE + DEBUG_EXTRA;

        /*
         * Set low and high addresses for blocks in this pool.
         */
        PoolTable[pool].LAddr = poolHeapPtr;
        PoolTable[pool].HAddr = poolHeapPtr + (numBlocks * poolSize);

#ifdef MEMMGR_DEBUG
        PoolTable[pool].poolSize = poolSize;
#endif /* MEMMGR_DEBUG */

        /*
         * Allocate space for each of the blocks in the pool and link them into
         * the free list for the pool
         */
        for (blockNum = 0; blockNum < numBlocks; ++ blockNum) {

            ALLOC_BLOCK *blockPtr = (ALLOC_BLOCK*) poolHeapPtr;

            /* Link this block into the pool's free list */
            blockPtr->next = freeList;
            freeList = blockPtr;

            poolHeapPtr += poolSize;

#ifdef MEMMGR_DEBUG
            {
                OI_UINT32  i;
                OI_UINT8  *fill;

                /* Initialize the guard string */
                for (i = 0; i < sizeof(GUARD_STRING); ++i) {
                    blockPtr->MemDebug.guard[i] = GUARD_STRING[i];
                }
                blockPtr->MemDebug.priorPool = priorPool;
                priorPool = blockPtr;
                blockPtr->MemDebug.free = TRUE;
                blockPtr->MemDebug.allocSize = 0;
                /*
                 * Fill block with odd value pattern to trap usage of freed blocks.
                 */
                fill = ((OI_UINT8 *)(void*) &(blockPtr->next)) + sizeof(blockPtr->next);
                for (; fill < poolHeapPtr; fill++) {
                    *fill = 0x55;
                }

            }
#endif /* MEMMGR_DEBUG */

        }

        OI_ASSERT(PoolTable[pool].HAddr == poolHeapPtr);

        /*
         * Initialize free list with the last allocated block
         */
        PoolTable[pool].freeList = freeList;

    }

    /* Mark MEMMGR as initialized */
    OI_INIT_FLAG_PUT_FLAG(TRUE, MEMMGR);


    MEMPROF_INIT( BLOCK_HEADER_SIZE ); /*Initialize after the pools are allocated */

    /*
     * NOTE: Using printf because we always want this information to be be displayed.
     */
    OI_Printf("MemMgr initialized: Dynamic pool heap size: %ld\n", (OI_UINT32)(heapCurrentPtr) - (OI_UINT32)config->heapConfig->heap);
    OI_Printf("MemMgr initialized: StaticMalloc heap available: %ld\n", (OI_UINT32)heapEndPtr - (OI_UINT32)heapCurrentPtr);

    return OI_STATUS_SUCCESS;
}

void OI_MEMMGR_DebugEnable(OI_BOOL enable)
{
#ifndef MEMMGR_DEBUG
    debug_out = enable ? TRUE : FALSE;
#endif
}


/*
 * Check if a call to OI_Malloc would fail
 */
OI_BOOL OI_MallocWillFail(OI_INT32 size)
{
    OI_UINT16 pool;

    OI_ASSERT(OI_INIT_FLAG_VALUE(MEMMGR));
    for (pool = 0; pool < PoolCount; ++pool) {
        if (PoolTable[pool].freeList != NULL) {
            if (PoolConfiguration[pool].blockSize >= (OI_UINT32)size) {
                return FALSE;
            }
        }
    }
    return TRUE;
}

/*
 * OI_Malloc
 *
 * Searches through the pool list to find a pool with block size >= the
 * requested size. If the best fit pool is empty, goes to the next pool. Assumes
 * that pools in the pool table are sorted by size.
 *
 * If a suitable pool is found, the block at the head of the free list is
 * removed from the free list and returned.
 */

#ifndef MEMMGR_DEBUG
/* Non-Debug version */

void* _OI_Malloc(OI_INT32 size)
{
    OI_INT16 pool;
    ALLOC_BLOCK *blockPtr;
    OI_UINT32 usize = size;
    void *retval;
    OI_UINT32 largest_available = 0;

    OI_ASSERT(OI_INIT_FLAG_VALUE(MEMMGR));
    for (pool = 0; pool < PoolCount; ++pool) {
        if (usize <= PoolConfiguration[pool].blockSize) {
            blockPtr = PoolTable[pool].freeList;
            if (blockPtr != NULL) {
                PoolTable[pool].freeList = blockPtr->next;
                /*
                 * Allocated memory starts where the next pointer used to be
                 */
                retval = (void*) &(blockPtr->next);
                /* Poor mans memprof --> print out details of memory allocation */
                if (debug_out) OI_Printf("\r+%d %x\n", usize, retval);
                return retval;
            }
        } else {
            /* Poor mans memprof --> remember largest available block size */
            if (PoolTable[pool].freeList) {
                largest_available = PoolConfiguration[pool].blockSize;
            }
        }
    }

    OI_DBGPRINT(("Dyn Req: %ld\nAvail: %ld\nStatic: %ld\n", usize, largest_available, (OI_UINT32)(heapEndPtr - heapCurrentPtr)));
    OI_LOG_ERROR(("OI_Malloc(%d) failed", usize));

    /* Poor mans memprof --> print out details of memory failure */
    if (debug_out){
        OI_Printf("\r+%d 88888888\n", usize);
        OI_FatalError(OI_STATUS_OUT_OF_MEMORY);
    }

    return 0;
}


void* _OI_Calloc(OI_INT32 size)
{
    void* block;
    OI_INT32 i;

    block = _OI_Malloc(size);
    if (block != NULL) {
        for (i = 0; i < size; ++i) {
            ((OI_BYTE*) block)[i] = 0;
        }
    }
    return block;

}

#else /* MEMMGR_DEBUG */

/* Debug version */

void* _OI_Malloc_Dbg(OI_INT32  size,
                     OI_UINT8 module,
                     OI_CHAR *fname,
                     OI_UINT16 lineNum)
{
    OI_INT16 pool;
    ALLOC_BLOCK *blockPtr;
    OI_UINT32 usize = size;
    void *retval;

    OI_DBGPRINT2(("%s@%d - malloc(%ld)", fname, lineNum, size));
    OI_ASSERT(OI_StackTokenHeld);
    OI_ASSERT(OI_INIT_FLAG_VALUE(MEMMGR));

    for (pool = 0; pool < PoolCount; ++pool) {
        if (usize <= PoolConfiguration[pool].blockSize) {
            blockPtr = PoolTable[pool].freeList;
            if (blockPtr != NULL) {
                PoolTable[pool].freeList = blockPtr->next;
                if( MEMMGR_ValidateFreeBlock(blockPtr, PoolConfiguration[pool].blockSize) != 0 ){
                    /* Additional dump of data if we are about to ASSERT */
                    OI_LOG_ERROR(("ERROR! - Block used after it was freed:\n"));
                    OI_LOG_ERROR(("Corruption at byte: %ld\n", MEMMGR_ValidateFreeBlock(blockPtr, PoolConfiguration[pool].blockSize)));
                    OI_LOG_ERROR(("Prior Alloc: %/@%d OI_Malloc(%ld)\n",blockPtr->MemDebug.fname,blockPtr->MemDebug.lineNum,blockPtr->MemDebug.allocSize));
                    OI_LOG_ERROR(("Prior Free: %/@%d OI_Free(%lx)\n",blockPtr->MemDebug.freeFile,blockPtr->MemDebug.freeLine,(OI_UINT32)&(blockPtr->next)));
                    OI_LOG_ERROR(("Cur Data: %@", &(blockPtr->next), 16));
                    OI_LOG_ERROR(("New Alloc: %/@%d OI_Malloc(%ld)\n",fname,lineNum,size));
                    OI_ASSERT_FAIL("OI_Malloc: block validation failed");
                }

                blockPtr->MemDebug.allocSize = size;  /* Record actual allocation size */
                blockPtr->MemDebug.free = FALSE;      /* This block is no longer free */
                blockPtr->MemDebug.fname = fname;     /* allocator's filename */
                blockPtr->MemDebug.lineNum = lineNum; /* allocator's line number */
                blockPtr->MemDebug.module = module;
                OI_Time_Now(&blockPtr->MemDebug.allocTime);
                MEMPROF_MALLOC(size, (void*) &(blockPtr->next), module, fname);

                if ( !IS_DYNAMIC_ALLOC((void*) &(blockPtr->next))){
                    OI_LOG_ERROR(("Alloc Error\n%/@%d OI_Malloc(%ld)\n",fname,lineNum,size));
                }
                last_alloc = ((void*) &(blockPtr->next));

                /*
                 * Allocated memory starts where the next pointer used to be
                 */
                retval = (void*) &(blockPtr->next);
                {
                    /*
                     * Write the entire block (not just the allocated size).
                     * Note that OI_Memset cannot do this because it checks the
                     * allocated size of the block.
                     */
                    OI_UINT32 *p = (OI_UINT32 *)((OI_UINT8 *) retval + PoolConfiguration[pool].blockSize + DEBUG_EXTRA);
                    while (p != (OI_UINT32 *)retval) {
                        *(--p) = 0x77777777;
                    }
                }
                return retval;
            }
        }
    }

    OI_MEMMGR_DumpUsedBlocks();
    OI_MEMMGR_Dump();
    OI_LOG_ERROR(("OI_Malloc(%ld) failed for %/@%d", size, fname, lineNum));
    return 0;
}


void* _OI_Calloc_Dbg(OI_INT32 size,
                     OI_UINT8 module,
                     OI_CHAR  *fname,
                     OI_UINT16 lineNum)
{
    void* block;
    OI_INT32 i;

    block = _OI_Malloc_Dbg(size, module, fname, lineNum);
    if (block != NULL) {
        for (i = 0; i < size; ++i) {
            ((OI_BYTE*) block)[i] = 0;
        }
    }
    return block;

}

#endif /* MEMMGR_DEBUG */


static void Free(void *block)
{
    ALLOC_BLOCK *blockPtr;
    OI_INT16 pool;

    OI_ASSERT(OI_INIT_FLAG_VALUE(MEMMGR));
    OI_ASSERT(block != NULL);

#ifdef MEMMGR_DEBUG
    OI_ASSERT(IS_DYNAMIC_ALLOC(block));
#endif /* MEMMGR_DEBUG */


    blockPtr = GET_ALLOC_BLOCK(block);

    if (block != &(blockPtr->next)) {
        OI_ASSERT_FAIL("OI_Free: pointer was not returned by OI_Malloc");
    }

    for (pool = 0; pool < PoolCount; ++pool) {
        if (IS_FROM_POOL(block, pool)) {
            blockPtr->next = PoolTable[pool].freeList;
            PoolTable[pool].freeList = blockPtr;
            return;
        }
    }
    OI_ASSERT_FAIL("OI_Free: memory not allocated by OI_Malloc");
}


/*
 * OI_Free
 *
 * Returns a memory block to the pool heap. First determines which pool the block
 * belongs by comparing the address of the block to be freed against the address
 * ranges for the pools. The block is then linked into the free list for the
 * block and becoming the first free block in the list.
 */

#ifndef MEMMGR_DEBUG

void _OI_Free(void *block)
{
    OI_ASSERT(OI_INIT_FLAG_VALUE(MEMMGR));
    if (debug_out) OI_Printf("\r-%x\n", block);
    Free(block);
}

#else /* MEMMGR_DEBUG */

void _OI_Free_Dbg(void *block,
                  OI_UINT8 module,
                  OI_CHAR *fname,
                  OI_UINT16 lineNum)
{
    OI_UINT32 position; /* test for writing past end of block */

    OI_DBGPRINT2(("%s@%d - free(%lx)", fname, lineNum, block));
    OI_ASSERT(OI_StackTokenHeld);
    OI_ASSERT(OI_INIT_FLAG_VALUE(MEMMGR));
    last_free = block;
    if (!IS_DYNAMIC_ALLOC(block)) {
        OI_LOG_ERROR(("Block Not Dynamic\n%/@%d OI_Free(%lx)\n", fname, lineNum, (OI_UINT32)block));
        OI_ASSERT(block != NULL);
    }
    position = MEMMGR_ValidateAllocBlock(block);
    Free(block);
    {
        ALLOC_BLOCK *blockPtr = GET_ALLOC_BLOCK(block);
        OI_UINT8 alloc_by = blockPtr->MemDebug.module;
        OI_UINT8 freed_by = module;
        OI_UINT32 blockSize = 0;
        OI_UINT8 *start = ((OI_UINT8 *)(void*)&blockPtr->next) + sizeof(blockPtr->next); /* skip past next since it was already set by Free() */
        OI_UINT8 *end;

        /* Check for prior freeing before checking overshoot, because data changes */
        if ( blockPtr->MemDebug.free ){
            OI_LOG_ERROR(("ERROR! - Block Freed Twice\n%/@%d OI_Free(%lx)\n",fname, lineNum, (OI_UINT32)block));
            OI_LOG_ERROR(("Prior Free: %/@%d \n",blockPtr->MemDebug.freeFile,blockPtr->MemDebug.freeLine));
            OI_LOG_ERROR(("Prior Alloc: %/@%d OI_Malloc(%ld)\n",blockPtr->MemDebug.fname,blockPtr->MemDebug.lineNum,blockPtr->MemDebug.allocSize));
            OI_ASSERT(!blockPtr->MemDebug.free);
        }

        /* overshooting must be checked prior to actual freeing and memory fill with 55 */
        if ( position ){
            OI_LOG_ERROR(("ERROR! - Block overshot prior to being freed at byte %ld\n", position));
            OI_LOG_ERROR(("This Free: %/@%d OI_Free(%lx)\n",fname, lineNum, (OI_UINT32)block));
            OI_LOG_ERROR(("Prior Alloc: %/@%d OI_Malloc(%ld)\n",blockPtr->MemDebug.fname,blockPtr->MemDebug.lineNum,blockPtr->MemDebug.allocSize));
            OI_LOG_ERROR(("Prior Free: %/@%d \n",blockPtr->MemDebug.freeFile,blockPtr->MemDebug.freeLine));
            OI_ASSERT(!position);
        }

        /*
         * Fill freed block with odd value pattern to trap usage of freed
         * blocks. Since block sizes must be 4-byte multiples, use UINT32 pointer arithmetic.
         */
        for (position = 0; position < PoolCount; ++position) {
            if (IS_FROM_POOL(block, position)) {
                blockSize = PoolConfiguration[position].blockSize;
                break;
            }
        }

        end = start + blockSize + DEBUG_EXTRA - sizeof (blockPtr->next); /* block size includes next */
        while (start < end) {
            *start++ = 0x55;
        }

        OI_ASSERT(CheckGuardString(blockPtr));

        blockPtr->MemDebug.freeFile = fname;
        blockPtr->MemDebug.freeLine = lineNum;
        blockPtr->MemDebug.free = TRUE;

        /* Always use the alloc_by module so we don't get out of sync. */
        MEMPROF_FREE(blockPtr->MemDebug.allocSize, block, blockPtr->MemDebug.module, blockPtr->MemDebug.fname);

        if ((alloc_by != OI_MODULE_MEMMGR) && (freed_by != OI_MODULE_MEMMGR)) {
            if (alloc_by != freed_by) {
                /*
                 * The following is DBGPRINT rather than LOG_ERROR because
                 * mismatch of allocating module and freeing module is at times
                 * a normal occurrence
                 */
                OI_DBGPRINT(("Memory allocated by %/ (line %d) was freed by %/ (line %d)\n", blockPtr->MemDebug.fname, blockPtr->MemDebug.lineNum, fname, lineNum));
            }
        }
    }
}

#endif /* MEMMGR_DEBUG */


/*
 * OI_FreeIf
 *
 * Calls free if the block is not null
 */

#ifndef MEMMGR_DEBUG
void _OI_FreeIf(const void *block)
{
    OI_ASSERT(OI_INIT_FLAG_VALUE(MEMMGR));

    if (block != NULL) {
        OI_Free((void *)block);
    }
}
#else /* MEMMGR_DEBUG */
void _OI_FreeIf_Dbg(const void *block,
                    OI_UINT8 module,
                    OI_CHAR *fname,
                    OI_UINT16 lineNum)
{
    OI_ASSERT(OI_INIT_FLAG_VALUE(MEMMGR));

    if (block != NULL) {
        _OI_Free_Dbg((void *)block, module, fname, lineNum);
    }
}
#endif /* MEMMGR_DEBUG */


/*
 * OI_StaticMalloc
 *
 * Allocates memory from the static allocation pool. This is data that is
 * required for the stack to function and will never explicitly freed.
 */

void* _OI_StaticMalloc(OI_INT32 size)
{
    OI_BYTE *pMalloc;
    OI_BYTE *pNewCur;

    OI_ASSERT(OI_INIT_FLAG_VALUE(MEMMGR));
    /*
     * Malloc'd space is current heap pointer aligned on 32-bit boundary
     */
    pMalloc = (OI_BYTE*)((OI_UINT32)(heapCurrentPtr + 3) & (~3));
    /*
     * Calculate new value for current heap pointer
     */
    pNewCur = pMalloc + size;

#ifndef MEMMGR_DEBUG
    /* Poor Mans memory profiler */
    if (debug_out) OI_Printf("\r=%d %x\n", size, pMalloc);
#endif

    /*
     * if the new value is past the end pointer, we don't have enough memory for
     * the current request (watch for possible memory wrap). This is a fatal
     * error because OI_StaticMalloc must always succeed.
     */
    if ((pNewCur > heapEndPtr) || (pNewCur < pMalloc)) {
        OI_DBGPRINT(("Static Req: %ld, Overflow: %ld\n", size, (OI_UINT32)(pNewCur - heapEndPtr)));
        OI_FatalError(OI_STATUS_OUT_OF_MEMORY);
    }
    heapCurrentPtr = pNewCur;
    /*
     * Zero the memory as required by api
     */
    OI_MemZero(pMalloc, size);
    return pMalloc;
}
#ifdef MEMMGR_DEBUG
void* _OI_StaticMalloc_Dbg(OI_INT32 size,
                           OI_UINT8 module,
                           OI_CHAR *fname,
                           OI_UINT16 lineNum)
{
    void *retVal;

    OI_ASSERT(OI_StackTokenHeld);
    OI_ASSERT(OI_INIT_FLAG_VALUE(MEMMGR));

    MEMPROF_STATIC_MALLOC(size, module, fname);
    retVal = _OI_StaticMalloc(size);

    OI_DBGPRINT(("%/@%d StaticMalloc(%ld)  - remaining: %ld", fname, lineNum, size, (OI_UINT32)(heapEndPtr - heapCurrentPtr)));

    return retVal;
}
#endif /* MEMMGR_DEBUG */


#ifndef OI_USE_NATIVE_MEMCPY

/*
 * OI_MemCopy
 */

void OI_MemCopy(void *To, void const *From, OI_UINT32 Size)
{
    /*
     * Do not OI_ASSERT(initialized) this function can be called before the
     * memory manager is initialized.
     */
    OI_ASSERT((To != NULL) || (Size == 0));
    OI_ASSERT((From != NULL) || (Size == 0));

    CHECK_EXTENT(To, Size);
    CHECK_EXTENT(From, Size);

    if (From < To) {
        /* copy starting at tail */
        while (Size > 0) {
            --Size;
            ((OI_BYTE*) To)[Size] = ((const OI_BYTE*) From)[Size];
        };
    } else {
        OI_UINT32 i;
        /* From > To ... copy starting at head */
        for (i = 0; i < Size; ++i) {
            ((OI_BYTE*) To)[i] = ((const OI_BYTE*) From)[i];
        }
    }
}


/*
 * OI_MemSet
 */

void OI_MemSet(void *Block, OI_UINT8 Val, OI_UINT32 Size)
{
    OI_UINT32 i;

    /*
     * Do not OI_ASSERT(initialized) this function can be called before the
     * memory manager is initialized.
     */
    OI_ASSERT((Block != NULL) || (Size == 0));
    CHECK_EXTENT(Block, Size);

    for (i = 0; i < Size; ++i) {
        ((OI_BYTE*) Block)[i] = Val;
    }
}


/*
 * OI_MemZero
 */

void OI_MemZero(void *Block, OI_UINT32 Size)
{
    OI_UINT32 i;

    /*
     * Do not OI_ASSERT(initialized) this function can be called before the
     * memory manager is initialized.
     */
    OI_ASSERT((Block != NULL) || (Size == 0));
    CHECK_EXTENT(Block, Size);

    for (i = 0; i < Size; ++i) {
        ((OI_BYTE*) Block)[i] = 0;
    }
}


/*
 * OI_MemCmp
 */

OI_INT OI_MemCmp(void const *s1, void const *s2, OI_UINT32 n) {

    OI_UINT32 i;

    /*
     * Do not OI_ASSERT(initialized) this function can be called before the
     * memory manager is initialized.
     */
    OI_ASSERT((s1 != NULL) && (s2 != NULL));
    CHECK_EXTENT(s1, n);
    CHECK_EXTENT(s2, n);

    for (i = 0; i < n; i++) {
        if ( ((const OI_BYTE *)s1)[i] < ((const OI_BYTE *)s2)[i]) return -1;
        if ( ((const OI_BYTE *)s1)[i] > ((const OI_BYTE *)s2)[i]) return 1;
    }

    return 0;
}

#endif /* OI_USE_NATIVE_MEMCPY */



/* **************************************************************************
 *
 * Debug and checking functions that are only used if MEMMGR_DEBUG is defined
 *
 * **************************************************************************/
#ifdef MEMMGR_DEBUG

/*
 * OI_MaxAllocBlock
 *
 * Searches through the pool list to find the smallest available block of
 * memory. First checks that a pool's free list is not empty then checks the
 * size of the blocks in that pool.
 */

OI_UINT32 OI_MaxAllocBlock(void)
{
    OI_UINT16 pool;
    OI_UINT32 maxSize = 0;

    OI_ASSERT(OI_INIT_FLAG_VALUE(MEMMGR));
    for (pool = 0; pool < PoolCount; ++pool) {
        if (PoolTable[pool].freeList != NULL) {
            if (PoolConfiguration[pool].blockSize > maxSize) {
                maxSize = PoolConfiguration[pool].blockSize;
            }
        }
    }

    return maxSize;
}


/*
 * OI_MinAllocBlock
 *
 * Searches through the pool list to find the smallest available block of memory.
 * First checks that a pool's free list is not empty then checks the size of the
 * blocks in that pool.
 */

OI_UINT32 OI_MinAllocBlock(void)
{
    OI_UINT16 pool;
    OI_UINT32 minSize = OI_INT32_MAX;

    OI_ASSERT(OI_INIT_FLAG_VALUE(MEMMGR));
    for (pool = 0; pool < PoolCount; ++pool) {
        if (PoolTable[pool].freeList != NULL) {
            if (PoolConfiguration[pool].blockSize < minSize) {
                minSize = PoolConfiguration[pool].blockSize;
            }
        }
    }
    return minSize;
}


static OI_BOOL CheckGuardString(ALLOC_BLOCK const *blockPtr)
{
    OI_UINT32 i;

    for (i = 0; i < sizeof(GUARD_STRING); ++i) {
        if (GUARD_STRING[i] != blockPtr->MemDebug.guard[i]) {
            OI_LOG_ERROR(("CheckGuardString failed"));
            if (blockPtr->MemDebug.priorPool) {
                blockPtr = blockPtr->MemDebug.priorPool;
                OI_LOG_ERROR(("Prior Block now free: %d\n",blockPtr->MemDebug.free));
                OI_LOG_ERROR(("Prior Last Alloc: %/@%d OI_Malloc(%ld)\n",blockPtr->MemDebug.fname,blockPtr->MemDebug.lineNum,blockPtr->MemDebug.allocSize));
                OI_LOG_ERROR(("Prior Last Free: %/@%d OI_Free(%lx)\n",blockPtr->MemDebug.freeFile,blockPtr->MemDebug.freeLine,(OI_UINT32)&(blockPtr->next)));
            }
            return FALSE;
        }
    }
    return TRUE;
}

/*
 * OI_MEMMGR_Check
 *
 * Checks that if the block was dynamically allocated that the guard string is
 * intact. Always returns TRUE for statically allocated memory.
 */

OI_BOOL OI_MEMMGR_Check(void const *Block)
{
    OI_ASSERT(OI_INIT_FLAG_VALUE(MEMMGR));
    if (IS_DYNAMIC_ALLOC(Block)) {
        return CheckGuardString(GET_ALLOC_BLOCK(Block));
    }
    return TRUE; /* not dynamically allocated memory */
}


/* Validate this alloc'd block hasn't overshot its valid range */
static OI_UINT32 MEMMGR_ValidateAllocBlock(void const *block)
{
    OI_UINT8 const *check;
    OI_UINT32 index;
    OI_UINT32 blockSize = 0;
    ALLOC_BLOCK *blockPtr = GET_ALLOC_BLOCK(block);

    for (index = 0; index < PoolCount; ++index) {
        if (IS_FROM_POOL(block, index)) {
            blockSize = PoolConfiguration[index].blockSize;
            break;
        }
    }

    check = block;
    for( index = blockPtr->MemDebug.allocSize; index < (blockSize + DEBUG_EXTRA); index++ ){
        if ( check[index] != 0x77 ){
            return index;
        }
    }

    /* Looks good */
    return 0;
}

/* Validate that this free block hasn't been written to */
static OI_UINT32 MEMMGR_ValidateFreeBlock(ALLOC_BLOCK *blockPtr, OI_UINT32 poolsize)
{
    OI_UINT8 *check;
    OI_UINT32 invalid;

    if( !blockPtr->MemDebug.free ||                                         /* block must be marked free */
        (!IS_DYNAMIC_ALLOC(blockPtr->next) && (blockPtr->next != NULL))){   /* next block must be dynamic */
        return 1;
    }

    /* Verify next block is properly alligned */
    if( ((OI_UINT32)(blockPtr->next)) & 0x03 ){
        return 3;
    }

    /* Verify that all bytes in block still have Free pattern */
    check = (OI_UINT8 *) &blockPtr[1];
    for (invalid = 0; invalid < (poolsize + DEBUG_EXTRA - sizeof(blockPtr->next)); invalid++){
        if ( check[invalid] != 0x55 ){
            return invalid + sizeof(blockPtr->next);
        }
    }

    /* Looks good */
    return 0;
}

/*
 * OI_MEMMGR_CheckSize
 *
 * For dynamically allocated memory, checks that there are at least "Size" bytes
 * between addr and the end of the appropriate allocation block.
 *
 * Always returns TRUE for statically allocated or stack memory.
 */

OI_BOOL OI_MEMMGR_CheckSize(void const *addr,
                            OI_UINT32 Size)
{
    ALLOC_BLOCK *blockPtr = NULL;
    OI_UINT32 poolSize;
    OI_UINT32 poolNum;
    OI_UINT16 pool;

    /*
     * There are a couple of situations during stack initialization that this
     * function is called before the memory manager has been initializted.
     */
    if (!OI_INIT_FLAG_VALUE(MEMMGR)) {
        return TRUE;
    }

    if (!IS_DYNAMIC_ALLOC(addr)) {
        return TRUE;
    }
    /*
     * Figure out which block this address is in
     */
    for (pool = 0; pool < PoolCount; ++pool) {
        if (IS_FROM_POOL(addr, pool)) {
            poolSize = PoolTable[pool].poolSize;
            poolNum = ((OI_UINT32) addr - (OI_UINT32) PoolTable[pool].LAddr) / poolSize;
            blockPtr = (ALLOC_BLOCK*) (PoolTable[pool].LAddr + poolSize * poolNum);
            break;
        }
    }
    OI_ASSERT(blockPtr != NULL);
    OI_ASSERT(CheckGuardString(blockPtr));

    /* Print out debugging info prior to assert fail if this block has been freed */
    if (blockPtr->MemDebug.free){
        OI_LOG_ERROR(("ERROR! - Block Checked after it was freed:\n"));
        OI_LOG_ERROR(("Prior Alloc: %/@%d OI_Malloc(%ld)\n",blockPtr->MemDebug.fname,blockPtr->MemDebug.lineNum,blockPtr->MemDebug.allocSize));
        OI_LOG_ERROR(("Prior Free: %/@%d OI_Free(%lx)\n",blockPtr->MemDebug.freeFile,blockPtr->MemDebug.freeLine,(OI_UINT32)&(blockPtr->next)));
        OI_ASSERT_FAIL("OI_MEMMGR_CheckSize: block free");
    }

    /*
     * Reduce available space by the offset of addr relative to blockPtr
     */
    if (Size <= (blockPtr->MemDebug.allocSize - ((const OI_BYTE*) addr - (const OI_BYTE*) &(blockPtr->next)))) {
        return TRUE;
    }
    OI_DBGPRINT(("CheckSize failed - requested size %ld, actual size %ld, file %s line %d\n",
                 Size, blockPtr->MemDebug.allocSize, blockPtr->MemDebug.fname, blockPtr->MemDebug.lineNum));
    return FALSE;
}


/*
 * OI_MEMMGR_DumpPools
 *
 * Prints out the current pool allocation for the pool heap
 */
void OI_MEMMGR_DumpPools(void)
{
    OI_INT16 pool;

    OI_ASSERT(OI_INIT_FLAG_VALUE(MEMMGR));
    OI_DBGPRINT(("Heap pool dump"));

    for (pool = 0; pool < PoolCount; ++pool) {
        ALLOC_BLOCK *blockPtr = PoolTable[pool].freeList;
        OI_INT16 count = 0;
        OI_UINT32 maxSize = 0;
        while (blockPtr != NULL) {
            ++count;
            if (blockPtr->MemDebug.allocSize > maxSize) {
                maxSize = blockPtr->MemDebug.allocSize;
            }
            blockPtr = blockPtr->next;
        }
        OI_DBGPRINT(("  [%lx .. %lx] alloc size %ld, max size %ld, free %d\n",
                     (OI_UINT32) PoolTable[pool].LAddr,
                     (OI_UINT32) PoolTable[pool].HAddr,
                     PoolConfiguration[pool].blockSize, maxSize, count));

    }
}

/*
 * OI_MEMMGR_DumpUsedBlocks
 *
 * Prints out all blocks that are currently in use
 */

void OI_MEMMGR_DumpUsedBlocks(void)
{
    OI_INT16 poolNum;

    OI_ASSERT(OI_INIT_FLAG_VALUE(MEMMGR));
    OI_Printf("MemHeap dump of all used blocks\n");
    OI_Printf("-----------------------------------------------\n");

    /*
     * for each pool, examine each block in the pool. If it is not free, print
     * out all the information about the block.
     */
    for (poolNum = 0; poolNum < PoolCount; ++poolNum) {
        MEM_POOL        *pPool = &PoolTable[poolNum];  /* ptr to pool */
        ALLOC_BLOCK     *pBlock;                       /* ptr to block */
        OI_UINT16       freeCount = 0;
        OI_UINT16       usedCount = 0;

        pBlock = (ALLOC_BLOCK*)(pPool->LAddr);         /* point at first block in this pool */
        while (pBlock < (ALLOC_BLOCK*)pPool->HAddr) {  /* while still within this pool */
            if (pBlock->MemDebug.free) {
                ++freeCount;
            } else {
                ++usedCount;
                OI_Printf("   Malloc(%d) at %T from %s@%d\n",
                          pBlock->MemDebug.allocSize,
                          &pBlock->MemDebug.allocTime,
                          pBlock->MemDebug.fname,
                          pBlock->MemDebug.lineNum);
            }
            pBlock = (ALLOC_BLOCK*)(((OI_UINT32)pBlock) + pPool->poolSize);
        }
        OI_Printf("[pool %4ld]  used:%4d free:%4d\n",
                   PoolConfiguration[poolNum].blockSize, usedCount, freeCount);
        OI_Printf("-----------------------------------------------\n");
    }
}


OI_UINT32 OI_MEMMGR_CurrentDynamicAllocation(void)
{
    OI_INT16    curPool;
    OI_UINT32   freeBlocksThisPool;
    OI_UINT32   allocThisPool;
    ALLOC_BLOCK *pFree;
    OI_UINT32   totalAlloc = 0;

    OI_ASSERT(OI_INIT_FLAG_VALUE(MEMMGR));
    for (curPool = 0; curPool < PoolCount; ++curPool) {
        // count number of blocks in the free list
        freeBlocksThisPool = 0;
        pFree = PoolTable[curPool].freeList;
        while (NULL != pFree) {
            ++freeBlocksThisPool;
            pFree = pFree->next;
        }
        // allocated memory from this pool = (total_blocks - free_blocks) * blocksize
        allocThisPool = (PoolConfiguration[curPool].numBlocks - freeBlocksThisPool) * PoolConfiguration[curPool].blockSize;
        totalAlloc += allocThisPool;
    }
    return(totalAlloc);
}

#endif /* MEMMGR_DEBUG */




#endif /* OI_USE_NATIVE_MALLOC */

/*****************************************************************************/
