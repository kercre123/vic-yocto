#ifndef _OI_MBUF_H
#define _OI_MBUF_H

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
@internal

 This module implements a non-linear buffering scheme called "mbufs"
 that allows data to be passed down the stack layers without requiring
 data copying at each layer. With an mbuf, a data packet is stored in
 multiple buffers rather than in a single contiguous data buffer. As
 each stack layer proceses data from the higher layers, data can be
 segmented and packetized without the need to actually copy any of the
 data. This improves performance by eliminating memory-to-memory
 copying and reduces total memory usage by elminating the need for
 each stack layer to allocate buffers to accomodate payloads passed in
 from the upper layers.

 Each contiguous "chunk" of data (or nested mbuf) associated with an
 mbuf occupies one cell within the mbuf. The maximum number of cells
 that an mbuf may contain must be specified when the mbuf is
 created. Mbufs that are allocated off of the heap using
 OI_MBUF_Alloc() can be initialized to contain any number of
 cells. Stack variables, globals and other non-heap allocated mbuf
 instances initialized with OI_MBUF_Init() may contain no more than
 DEFAULT_MBUF_CELLS cells.

 An Mbuf producer (an entity that creates an mbuf) fills the mbuf with
 data and/or data references using OI_MBUF_Append(), OI_MBUF_Prepend()
 and OI_MBUF_AppendMbuf(). Depending on the size of the data and the
 disposition (OI_MBUF_DISPOSITION) specified, the data pointer passed
 to OI_MBUF_Append() or OI_MBUF_Prepend() is either dereferenced and
 copied to heap-allocated or internal mbuf storage (OI_COPY) or the
 pointer is stored directly in the mbuf without copying (OI_KEEP or OI_FREE).

 Mbufs may be nested to allow appending/prepending of mbuf data without
 requiring the original mbuf to have unused cells. The current mbuf
 implementation does not manage the life-cycle of nested mbufs. This
 means that external logic must ensure that nested mbufs are
 freed once the containing (outer) mbuf is no longer in use.

 Consumers of Mbuf data do not need to be aware of the existence of
 cell boundaries within mbufs. Consumers access the contents of mbufs
 by calling OI_MBUF_PullBytes() and/or OI_MBUF_PeekBytes().

 MBuf provides a "windowing" feature that allows a producer to limit
 the region of the mbuf that is exposed to consumers. This is useful
 when you want to pass only a portion of a packet to some other entity
 for further processing. From the consumer's point of view, the
 windowed mbuf appears to be a normal mbuf that contains only a subset
 of the original mbuf's data. OI_MBUF_SetWindow(),
 OI_MBUF_SetWindowAt() and OI_MBUF_AdvanceWindow() are used to
 manipulate windows. When nesting mbufs, all windowing information is
 contained within the outermost mbuf. This allows for multiple
 simultaneous windows over the same mbuf data by creating one or more
 "outer" mbufs that all wrap the same "inner" mbuf data.
*/

#include "oi_stddefs.h"
#include "oi_debug.h"

/** \addtogroup Misc Miscellaneous APIs */
/**@{*/

#ifdef __cplusplus
extern "C" {
#endif


#ifdef OI_DEBUG

/**
 * OI_MBUF_DISPOSITION indicates how the ownership of data associated with
 * an mbuf's cell will be managed.
 */
typedef enum {
    MBUF_COPY   = 0,  /**< Data buffer is to be copied by mbuf into mbuf managed storage.
                         Mbuf will manage the freeing of the copied data if needed. */

    MBUF_FREE   = 1,  /**< Data buffer will be freed by mbuf when mbuf is freed, Data
                         must have been allocated wit OI_Malloc()*/

    MBUF_KEEP   = 2,  /**< Data buffer's life-time will be managed outside of mbuf.
                         Caller is responsible for ensuring the data remains valid
                         until mbuf is no longer in use. */

    MBUF_INLINE = 3,  /**< Internal use only. Data is inline in the mbuf and cannot
                         be freed. User cannot designate this disposition directly.
                         It is automatically chosen if the user selectes MBUF_COPY
                         and the size of the data fits within the cell structure. */

    MBUF_NESTED = 4   /**< Cell is a nested mbuf and cannot be freed. Mbuf implementation
                         is not responsible for managing the life-time of nested mbufs.
                         User is responsible for externally managing the life-time
                         of nested mbufs. */

} OI_MBUF_DISPOSITION;

#else

#define   MBUF_COPY    0
#define   MBUF_FREE    1
#define   MBUF_KEEP    2
#define   MBUF_INLINE  3
#define   MBUF_NESTED  4

typedef OI_UINT8 OI_MBUF_DISPOSITION;

#endif


struct _OI_MBUF;


typedef struct {
    OI_UINT16 winStart;    /** window start offset at time mbuf was nested */
    struct _OI_MBUF *mbuf; /** the nested mbuf */
} OI_NESTED_MBUF;


/**
 *
 */
#define MBUF_INLINE_BUF_SIZE  8


/**
 * Type definition for an mbuf cell. An mbuf cell has a pointer to a buffer and
 * a length.
 */
typedef struct {
    OI_MBUF_DISPOSITION disposition;
    OI_UINT16 len;
    union {
        const OI_BYTE *ptr;
        OI_BYTE data[MBUF_INLINE_BUF_SIZE];
        OI_NESTED_MBUF mcell;
    } buf;
} OI_MBUF_CELL;


#define DEFAULT_MBUF_CELLS    4

/**
 * Defines for casting function pointers to generic function pointers
 */
typedef void (*_F_PTR)(void);
#define MBUF_CONTEXT_FUNC(x) ((_F_PTR)x)

/**
 * Type definition for an mbuf.
 */

typedef struct _OI_MBUF {
    OI_UINT8 allocCells;                    /*< Number of cells allocated in this mbuf */
    OI_UINT8 currCell;                      /*< Current cell position for pull operations */
    OI_UINT8 usedCells;                     /*< Cells currently in use */
    OI_UINT16 numBytes;                     /*< Current number of bytes in all cells */
    OI_UINT16 currPos;                      /*< Current byte position in current cell for pull operations */

    OI_UINT16 winStart;                     /*< Current byte offset of window from start of MBUF */
    OI_UINT16 winSize;                      /*< Size of current window (0 means not windowed) */

    union {
        void     *v;                        /*< generic pointer value */
        OI_INT32  i;                        /*< generic signed value */
        OI_UINT32 u;                        /*< generic unsigned value */
        _F_PTR    f;                        /*< generic function pointer */
    } context;                              /*< context union data for users of mbuf */

    OI_MBUF_CELL cells[DEFAULT_MBUF_CELLS]; /*< mbuf cells reference packet data */
} OI_MBUF;


/**
 * An MBUF is clean if no bytes have been pulled from the MBUF. Otherwise is it dirty.
 *
 * The only operations that can be performed on a dirty MBUF are:
 *
 * OI_MBUF_PullBytes()
 * OI_MBUF_Clear()
 * OI_MBUF_NumBytes()
 * OI_MBUF_NumContigBytes()
 * OI_MBUF_PeekBytes()
 * OI_MBUF_Unwrap()
 *
 * Use OI_MBUF_Clear() restore nested MBUFs to a clean (un-pulled) state.
 */
#define OI_MBUF_IS_DIRTY(mbuf)   (((mbuf)->currCell != 0) || ((mbuf)->currPos != 0))


/**
 * Initializes a non-heap allocated mbuf. The caller is responsible for ensuring that the mbuf is
 * big enough to hold the number of cells specificed in the numCells parameter. The caller should
 * call OI_MBUF_Clear() when finished with this MBUF.
 *
 * @param  mbuf  A pointer to an unitialized mbuf. This can be a pointer to an
 *               mbuf allocated as an automatic variable.
 *
 * @param numCells  The number of cells in the mbuf. This must less than
 *                  DEFAULT_MBUF_CELLS.
 */

OI_STATUS OI_MBUF_Init(OI_MBUF *mbuf,
                       OI_UINT8 numCells);



/**
 * Allocate and initialize an mbuf sized to accomodate the requested number of cells. The caller
 * should call OI_MBUF_Free() when finished with this MBUF.
 *
 * @param numCells the number of cells the mbuf can hold.
 *
 * @returns A pointer to a dynmically allocted mbuf or NULL if the mbuf could
 *          not be allocated.
 */

OI_MBUF* OI_MBUF_Alloc(OI_UINT8 numCells);


/**
 * Helper function that wraps a buffer in an mbuf. Allocates a one cell mbuf initializes it to
 * reference a buffer. The mbuf must be freed by calling OI_MBUF_Free().
 *
 * @param  data  A data buffer to initialize the mbuf with.
 *
 * @param  len   The size of the data buffer.
 *
 * @param  disposition Specifies what mbuf should do with the data buffer when
  *                    the mbuf is freed.
 *
 * @return a pointer to a dynamically allocated mbuf and initialized mbuf
 *         or NULL if the mbuf could not be allocated.
 */

OI_MBUF* OI_MBUF_Wrap(const OI_BYTE *data,
                      OI_UINT16 len,
                      OI_MBUF_DISPOSITION disposition);


/**
 * Counterpart to OI_MBUF_Wrap() - removes and frees the MBUF created by OI_MBUF_Wrap() and returns
 * a pointer to the original data and len. Can only be used on MBUFs that were wrapped using the
 * disposition MBUF_KEEP.
 *
 * @param  mbuf   Mbuf to unwrap
 *
 * @param  data   Returned pointer to data wrapped by mbuf.
 *
 * @param  len    Returned length of data wrapped by mbuf.
 */
void OI_MBUF_Unwrap(OI_MBUF *mbuf,
                    OI_BYTE **data,
                    OI_UINT16 *len);

/**
 * Free an mbuf and any data that is not flagged to be kept.
 *
 * @param mbuf a pointer to a dynamically allocated mbuf.
 *
 * @return  The number of bytes that the MBUF used to contain.
 */
OI_UINT16 OI_MBUF_Free(OI_MBUF *mbuf);


/**
 * Add a cell to the front of an mbuf. This function will typically be used to
 * add a packet header to a payload passed from a higher layer. Prepend is only
 * allowed on a clean mbuf, that is an mbuf that has not had data pulled from
 * it.
 *
 * @param mbuf a pointer to an initialized mbuf.
 *
 * @param buf a pointer to a data buffer.
 *
 * @param len the length of the data buffer.
 *
 * @param disposition specifies what mbuf should do with the data buffer. There
 * are three possibilities:
 *
 * - MBUF_COPY indicates that mbuf must copy the buffer. Use this if the
 *   data buffer is an automatic variable. Mbuf will free any allocated memory
 *   when the mbuf itself is freed.
 *
 * - MBUF_FREE indicates that mbuf that caller want mbuf to eventually free
 *   the buffer. Obviously the buffer must not also be freed by the caller.
 *
 * - MBUF_KEEP indicates that mbuf must not free or otherwise modify the
 *   buffer.
 *
 * @return OI_OK is the buffer was succesfully added or
 *         OI_STATUS_MBUF_OVERFLOW if there are no free cells in the mbuf.
 */

OI_STATUS OI_MBUF_Prepend(OI_MBUF* mbuf,
                          const OI_BYTE* buf,
                          OI_UINT16 len,
                          OI_MBUF_DISPOSITION disposition);


/**
 * Add a cell to the end of an mbuf. This function will typically be used to add
 * a packet trailer to a payload passed from a higher layer. Append is only
 * allowed on a pristine mbuf, that is an mbuf that has not had data pulled from
 * it.
 *
 * @param mbuf a pointer to an initialized mbuf.
 *
 * @param buf a pointer to a data buffer.
 *
 * @param len the length of the data buffer.
 *
 * @param disposition specifies what mbuf should do with the data buffer. There
 * are three possibilities:
 *
 * - MBUF_COPY indicates that mbuf must copy the buffer. Use this if the data
 *   buffer is an automatic variable. Mbuf will free any allocated memory when
 *   the mbuf itself is freed.
 *
 * - MBUF_FREE indicates that mbuf that caller want mbuf to eventually free the
 *   buffer. Obviously the buffer must not also be freed by the caller.
 *
 * - MBUF_KEEP indicates that mbuf must not free or otherwise modify the buffer.
 *
 * @return OI_OK is the buffer was succesfully added or
 *         OI_STATUS_MBUF_OVERFLOW if there are no free cells in the mbuf.
 */

OI_STATUS OI_MBUF_Append(OI_MBUF* mbuf,
                         const OI_BYTE* buf,
                         OI_UINT16 len,
                         OI_MBUF_DISPOSITION disposition);


/**
 * Add a cell to the end of an mbuf where the contents of the appended cell is an MBUF. Note that
 * appended MBUFs must not be freed until the enclosing MBUF has been cleared by calling
 * OI_MBUF_Clear() or freed by calling OI_MBUF_Free().
 *
 * @param mbuf     a pointer to an initialized mbuf.
 *
 * @param added    a pointer to an initialized mbuf.
 *
 * @return OI_OK is the buffer was succesfully added or
 *         OI_STATUS_MBUF_OVERFLOW if there are no free cells in the mbuf.
 */
OI_STATUS OI_MBUF_AppendMbuf(OI_MBUF *mbuf,
                             OI_MBUF *added);


/**
 * Returns the pointer to a nested MBUF.
 *
 * @param mbuf   Pointer to an mbuf that has a nested MBUF cell.
 *
 * @param index  Cell index that contains the nested MBUF
 *
 * @return  Pointer to the nested MBUF or NULL
 */
OI_MBUF* OI_MBUF_GetNestedMbuf(OI_MBUF *mbuf,
                               OI_UINT8 index);


/**
 * Copies bytes from an mbuf info a contiguous buffer. The data in the source
 * mbuf is not modified by this operation but the state of the source mbuf is
 * updated to reflect the data that has been pulled. Sucessive pulls will
 * advance the internal state as if the data pulled were removed from the source
 * mbuf. Pulling bytes from an MBUF dirties the MBUF and any nested MBUFs.
 *
 * @param dest a pointer to a buffer at least numBytes long.
 *
 * @param src is a pointer to a segmented memory buffer.
 *
 * @param numBytes is the maximum number of bytes to be pulled from the mbuf.
 *
 * @returns  The number of bytes pulled.
 */

OI_UINT16 OI_MBUF_PullBytes(OI_BYTE *dest,
                            OI_MBUF *src,
                            OI_UINT16 numBytes);

/**
 * Returns a pointer into the mbuf's data at a specified offset. Peeking bytes
 * does not dirty the mbuf.
 *
 * @param src         Pointer to a segmented memory buffer.
 *
 * @param offset      Byte offset into mbuf.
 *
 * @param contigLen   Number of contiguous bytes that can be obtained from returned pointer.
 *
 * @returns  Pointer to data at requested offset or NULL if offset is past the end of mbuf.
 */
const OI_BYTE * OI_MBUF_PeekBytes(OI_MBUF *src,
                                  OI_UINT16 offset,
                                  OI_UINT16 *contigLen);

/**
 * Returns the aggregate number of bytes in the cells of an mbuf.
 *
 * @param mb is a pointer to an mbuf
 *
 */
#ifdef OI_DEBUG
OI_UINT16 OI_MBUF_NumBytes(OI_MBUF *mb);
#else
#define OI_MBUF_NumBytes(mb) ((mb)->numBytes)
#endif

/**
 * Projects a "window" on an mbuf defined by a length. A window sets the internal state on an mbuf
 * so that the calls to pull data from the mbuf will pull data from the range defined by the window.
 * If the window start position + the window length is beyond the actual end of the mbuf, the length
 * is set to its maximum possible value. If the start position is beyond the end of the mbuf the
 * window will be empty. After a window has been set, OI_MBUF_NumBytes() will return the size of the
 * window. The window can be advanced by calling OI_MBUF_AdvanceWindow().
 *
 * @param mbuf A pointer to an mbuf.
 *
 * @param  winSize   The size of the window.
 *
 * @returns  The actual window size, which may be smaller than the window size
 *           specified if there are fewer than winSize bytes in the mbuf.
 */
OI_UINT16 OI_MBUF_SetWindow(OI_MBUF *mbuf,
                            OI_UINT16 winSize);


/**
 * Projects a "window" on an mbuf defined by an offset and a length. A window sets the internal
 * state on an mbuf so that the calls to pull data from the mbuf will pull data from the range
 * defined by the window.  If the window start position + the window length is beyond the actual end
 * of the mbuf, the length is set to its maximum possible value. If the start position is beyond the
 * end of the mbuf the window will be empty. After a window has been set, OI_MBUF_NumBytes() will
 * return the size of the window. The window can be advanced by calling OI_MBUF_AdvanceWindow().
 *
 * @param mbuf      A pointer to an mbuf.
 *
 * @param winStart  Offset from the start of the MBUF for the window
 *
 * @param winSize   The size of the window.
 *
 * @returns  The actual window size, which may be smaller than the window size
 *           specified if there are fewer than winStart + winSize bytes in the
 *           mbuf.
 */
OI_UINT16 OI_MBUF_SetWindowAt(OI_MBUF *mbuf,
                              OI_UINT16 winStart,
                              OI_UINT16 winSize);


/**
 * This function projects a new window on an mbuf that starts where the previous window ended.
 * Prior to this call, OI_MBUF_SetWindow() must have been called to set an initial window. To
 * segment a packet into fixed-size frames, repeatedly call this function until it returns 0.
 *
 * Note that the all of the data from the previous window must have been pulled before the window
 * can be advanced. That is, OI_MBUF_NumBytes() must return zero before OI_MBUF_AdvanceWindow()
 *
 * @param mbuf     the mbuf
 * @param winSize  the size of the new window
 *
 * @returns  The actual window size, which may be smaller than the window size
 *           specified if there are fewer than winSize bytes between the current
 *           position and the end of the mbuf.
 */
OI_UINT16 OI_MBUF_AdvanceWindow(OI_MBUF *mbuf,
                                OI_UINT16 winSize);



/**
 * Frees up resources allocated for the cells of an MBUF and restores any nesteds MBUFs to a clean
 * state. Doesn not free the MBUF itself.
 *
 * @param mbuf     Pointer to the mbuf to clear
 *
 * @return  The number of bytes that the MBUF used to contain.
 */
OI_UINT16 OI_MBUF_Clear(OI_MBUF *mbuf);


/**
 * Compute a Frame Check Sequence over an MBUF, the MBUF is not modified by this operation. This FCS
 * is commonally know as CRC-16 and is used principally for L2CAP enhanced modes.
 *
 * @param mbuf        Pointer to an mbuf
 *
 * @param numBytes    The number of bytes to run the FCS over. Use OI_UINT16_MAX to run over the
 *                    entire mbuf.
 *
 * @patam runningFCS  Pointer to variable holding the running FCS. Initialize to zero to start
 *                    computing the FCS.
 *
 * @return   The number of bytes FCS was computed over.
 */
OI_UINT16 OI_MBUF_ComputeFCS(const OI_MBUF *mbuf,
                             OI_UINT16 numBytes,
                             OI_UINT16 *runningFCS);


/**
 * Returns the maximum cell capacity of an mbuf.
 */
#define OI_MBUF_NumAllocCells(mb) ((OI_UINT8) (((mb) == NULL) ? 0 : (mb)->allocCells))


/**
 * Returns the number of avaiable cells in an mbuf.
 */
#define OI_MBUF_NumFreeCells(mb) ((OI_UINT8) (((mb) == NULL) ? 0 : ((mb)->allocCells - ((mb)->usedCells))))


/**
 * Returns the number of used cells in an mbuf.
 */
#define OI_MBUF_NumUsedCells(mb) ((OI_UINT8) (((mb) == NULL) ? 0 : (mb)->usedCells))

#ifdef OI_DEBUG

/**
 * Print out contents of an mbuf.
 *
 * @param mbuf     the mbuf to print
 */
void OI_MBUF_Print(OI_MBUF *mbuf);


/**
 * Dump the structure of an mbuf.
 *
 * @param mbuf     the mbuf to dump
 * @param tag      a string to prefix to the dump info
 */
void OI_MBUF_Dump(OI_MBUF *mbuf,
                  OI_CHAR *tag);

#else

#define OI_MBUF_Print(m)
#define OI_MBUF_Dump(m, t)

#endif /* OI_DEBUG */

/*****************************************************************************/
#ifdef __cplusplus
}
#endif

/**@}*/

#endif /* _OI_MBUF_H */

