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

 This module implements a non-linear buffering scheme called "mbufs" that allows
 data to be passed down the stack layers without requiring data copying at each
 layer. The simple concept is that with an mbuf, a data packet is stored in
 multiple buffers rather than in a single contiguous data buffer. As each stack
 layer proceses data from the higher layers, data can be segmented and
 packetized without the need to actually copy any of the data. This improves
 performance by eliminating memory-to-memory copying and reduces total memory
 usage by elminating the need for each stack layer to allocate buffers to
 accomodate payloads passed in from the upper layers.
*/

#define __OI_MODULE__ OI_MODULE_MEMMGR

#include "oi_debug.h"
#include "oi_assert.h"
#include "oi_memmgr.h"
#include "oi_status.h"
#include "oi_std_utils.h"
#include "oi_mbuf.h"
#include "oi_fcs.h"



/*
 * An mbuf is considered to be windowed if there is a non-zero window size set or if it has a
 * non-zero window start position.
 */
#define MBUF_IS_WINDOWED(m) ((m)->winSize || (m)->winStart)


/*
 * Allocate and initialize an mbuf sized to accomodate the requested number of cells.
 */

OI_MBUF* OI_MBUF_Alloc(OI_UINT8 numCells)
{
    OI_UINT size = sizeof(OI_MBUF) + sizeof(OI_MBUF_CELL) * (numCells - DEFAULT_MBUF_CELLS);
    OI_MBUF *mbuf;

    mbuf = OI_Calloc(size);
    if (mbuf) {
        mbuf->allocCells = numCells;
    }
    return mbuf;
}


OI_STATUS OI_MBUF_Init(OI_MBUF *mbuf,
                       OI_UINT8 numCells)
{
    OI_UINT size = sizeof(OI_MBUF) + sizeof(OI_MBUF_CELL) * (numCells - DEFAULT_MBUF_CELLS);
    OI_ASSERT(mbuf != NULL);
    OI_ASSERT(numCells > 0);

    if (numCells > DEFAULT_MBUF_CELLS) {
        OI_SLOG_ERROR(OI_STATUS_MBUF_OVERFLOW, ("OI_MBUF_Init %d", OI_STATUS_MBUF_OVERFLOW)) ;
        return OI_STATUS_MBUF_OVERFLOW;
    }
    OI_MemZero(mbuf, size);
    mbuf->allocCells = numCells;
    return OI_OK;
}


/*
 * Recursively cleans an MBUF by clearing the currPos and currCell values and resetting numBytes.
 * All information related to windowing is preserved.
 */
static OI_UINT16 Clean(OI_MBUF *mbuf)
{
    OI_DBGPRINT2(("MBUF clean %lx", mbuf));

    if (OI_MBUF_IS_DIRTY(mbuf)) {
        OI_UINT i;
        OI_UINT16 len = 0;
        /*
         * Note that the windowing information is preserved
         */
        mbuf->currPos = 0;
        mbuf->currCell = 0;

        for (i = 0; i < mbuf->usedCells; ++i) {
            OI_MBUF_CELL *cell = &mbuf->cells[i];
            if (MBUF_NESTED == cell->disposition) {
                Clean(cell->buf.mcell.mbuf);
            }
            len += cell->len;
        }
        /*
         * If the mbuf is windowed numBytes is the window size, otherwise it is the un-windowed size
         */
        if (MBUF_IS_WINDOWED(mbuf)) {
            OI_ASSERT(mbuf->winSize <= len);
            mbuf->numBytes = mbuf->winSize;
        } else {
            mbuf->numBytes = len;
        }
    }
    return mbuf->numBytes;
}


/*
 * Not currently a public API but this function is used for testing.
 */
void OI_MBUF_Reset(OI_MBUF *mbuf)
{
    Clean(mbuf);
}


OI_UINT16 OI_MBUF_Clear(OI_MBUF *mbuf)
{
    OI_UINT i;
    OI_UINT16 len = 0;

#ifdef OI_DEBUG
    OI_UINT allocCells = mbuf->allocCells;
    OI_UINT size = sizeof(OI_MBUF) + sizeof(OI_MBUF_CELL) * (allocCells - DEFAULT_MBUF_CELLS);
#endif

    OI_DBGPRINT2(("MBUF Clear %lx", mbuf));

    for (i = 0; i < mbuf->usedCells; ++i) {
        OI_MBUF_CELL *cell = &mbuf->cells[i];
        switch (cell->disposition) {
            case MBUF_KEEP:
            case MBUF_INLINE:
                len += cell->len;
                break;
            case MBUF_COPY:
            case MBUF_FREE:
                OI_Free((void *)cell->buf.ptr);
                len += cell->len;
                break;
            case MBUF_NESTED:
                len += Clean(cell->buf.mcell.mbuf);
                break;
        }
    }
#ifdef OI_DEBUG
    OI_MemZero(mbuf, size);
    mbuf->allocCells = allocCells;
#else
    mbuf->usedCells = 0;
    mbuf->currPos = 0;
    mbuf->currCell = 0;
    mbuf->numBytes = 0;
    mbuf->winSize = 0;
    mbuf->winStart = 0;
#endif
    return len;
}


/**
 * Free an mbuf previously allocated by OI_MBUF_Alloc.
 */

OI_UINT16 OI_MBUF_Free(OI_MBUF *mbuf)
{
    OI_UINT16 len;

    OI_DBGPRINT2(("MBUF Free %lx", mbuf));
    len = OI_MBUF_Clear(mbuf);
    OI_Free(mbuf);
    return len;
}


static OI_STATUS InitCell(OI_MBUF_CELL *currCell,
                          const OI_BYTE* buf,
                          OI_UINT16 len,
                          OI_MBUF_DISPOSITION disposition)
{
    OI_INT i;
    OI_BYTE *ptr;

    /*
     * Verify data being added to this MBUF is valid (see Bug2627)
     */
    OI_ASSERT(OI_MEMMGR_CheckSize(buf, len));

    if (disposition == MBUF_COPY) {
        /*
         * Avoid malloc and free for small fragments by copying data directly into mbuf
         */
        if (len <= MBUF_INLINE_BUF_SIZE) {
            for (i = 0; i < len; ++i) {
                currCell->buf.data[i] = buf[i];
            }
            disposition = MBUF_INLINE;
        } else {
            /*
             * Malloc memory for the mbuf data.
             */
            if ((ptr = OI_Malloc(len)) == NULL) {
                /*
                 * Clear the cell to keep the enclosing mbuf consistent.
                 */
                currCell->buf.ptr = NULL;
                currCell->len = 0;
                currCell->disposition = MBUF_KEEP;
                return OI_STATUS_OUT_OF_MEMORY;
            }
            OI_MemCopy(ptr, buf, len);
            currCell->buf.ptr = ptr;
        }
    } else {
        currCell->buf.ptr = buf;
    }
    currCell->disposition = disposition;
    currCell->len = len;
    return OI_OK;
}



void OI_MBUF_Unwrap(OI_MBUF *mbuf,
                    OI_BYTE **data,
                    OI_UINT16 *len)
{
    OI_ASSERT(mbuf && data && len);
    OI_ASSERT(mbuf->allocCells <= 1);
    OI_ASSERT(mbuf->usedCells == mbuf->allocCells);

    if (mbuf->allocCells == 1) {
        OI_ASSERT(mbuf->cells[0].disposition == MBUF_KEEP);
        *data = (OI_BYTE*)mbuf->cells[0].buf.ptr;
        *len = mbuf->cells[0].len;
    } else {
        OI_ASSERT(mbuf->allocCells == 0);
        *data = NULL;
        *len = 0;
    }
    OI_Free(mbuf);
}


OI_MBUF* OI_MBUF_Wrap(const OI_BYTE *data,
                      OI_UINT16 len,
                      OI_MBUF_DISPOSITION disposition)
{
    OI_MBUF *mbuf;
    OI_STATUS result;

    if ((data == NULL) || (len == 0)) {
        mbuf = OI_MBUF_Alloc(0);
    } else {
        mbuf = OI_MBUF_Alloc(1);
        if (mbuf != NULL) {
            mbuf->usedCells = 1;
            mbuf->numBytes = len;
            result = InitCell(mbuf->cells, data, len, disposition);
            if (!OI_SUCCESS(result)) {
                OI_SLOG_ERROR(result, ("OI_MBUF_Wrap - InitCell failed %d", result));
                OI_MBUF_Free(mbuf);
                mbuf = NULL;
            }
        }
    }
    OI_DBGPRINT2(("OI_MBUF_Wrap %lx", mbuf));
    return mbuf;
}


/**
 * Add a new cell to the front of an mbuf. This function will typically be used to add a packet
 * header to a payload passed from a higher layer.
 */

OI_STATUS OI_MBUF_Prepend(OI_MBUF* mbuf,
                          const OI_BYTE* buf,
                          OI_UINT16 len,
                          OI_MBUF_DISPOSITION disposition)
{
    OI_UINT i;
    OI_STATUS status;

    OI_ASSERT(mbuf);
    OI_ASSERT(buf);
    OI_ASSERT(!OI_MBUF_IS_DIRTY(mbuf));
    /*
     * Cannot prepend to an mbuf that is windowed
     */
    OI_ASSERT(!MBUF_IS_WINDOWED(mbuf));

    if (disposition > MBUF_KEEP) {
        return OI_STATUS_INVALID_PARAMETERS;
    }
    if (len == 0) {
        return OI_OK;
    }
    if (mbuf->usedCells >= mbuf->allocCells) {
        OI_SLOG_ERROR(OI_STATUS_MBUF_OVERFLOW, ("MBUF Prepend overflow: alloc %d", mbuf->allocCells));
        OI_MBUF_Dump(mbuf, "overflow in prepend");
        return OI_STATUS_MBUF_OVERFLOW;
    }
    /*
     * Make room for the new cell
     */
    for (i = mbuf->usedCells; i > 0; --i) {
        mbuf->cells[i] = mbuf->cells[i - 1];
    }
    /*
     * Increment used cells first to avoid potential memory leak if InitCell fails
     */
    mbuf->usedCells++;
    status = InitCell(&mbuf->cells[0], buf, len, disposition);
    if (OI_SUCCESS(status)) {
        mbuf->numBytes += len;
    }

    return status;
}


/**
 * Add a cell to the end of an mbuf. This function will typically be used to add
 * a packet trailer to a payload passed from a higher layer.
 */

OI_STATUS OI_MBUF_Append(OI_MBUF* mbuf,
                         const OI_BYTE* buf,
                         OI_UINT16 len,
                         OI_MBUF_DISPOSITION disposition)
{
    OI_STATUS status;

    OI_ASSERT(mbuf);
    OI_ASSERT(!OI_MBUF_IS_DIRTY(mbuf));

    /*
     * Cannot append to an mbuf that is windowed
     */
    OI_ASSERT(!MBUF_IS_WINDOWED(mbuf));

    if (disposition > MBUF_KEEP) {
        return OI_STATUS_INVALID_PARAMETERS;
    }
    if (len == 0) {
        return OI_OK;
    }
    if (mbuf->usedCells >= mbuf->allocCells) {
        OI_SLOG_ERROR(OI_STATUS_MBUF_OVERFLOW, ("MBUF Append overflow: alloc %d", mbuf->allocCells));
        OI_MBUF_Dump(mbuf, "overflow in append");
        return OI_STATUS_MBUF_OVERFLOW;
    }
    status = InitCell(&mbuf->cells[mbuf->usedCells], buf, len, disposition);
    if (OI_SUCCESS(status)) {
        mbuf->usedCells++;
        mbuf->numBytes += len;
    }
    return status;
}


OI_MBUF* OI_MBUF_GetNestedMbuf(OI_MBUF *mbuf,
                               OI_UINT8 index)
{
    OI_MBUF_CELL *cell;

    if (index >= mbuf->usedCells) {
        return NULL;
    }
    cell = &mbuf->cells[index];
    if (cell->disposition != MBUF_NESTED) {
        OI_SLOG_ERROR(OI_STATUS_INVALID_STATE, ("Cell does not hold a nested MBUF"));
        return NULL;
    }
    return cell->buf.mcell.mbuf;
}


OI_STATUS OI_MBUF_AppendMbuf(OI_MBUF *mbuf,
                             OI_MBUF *added)
{
    OI_MBUF_CELL *cell;

    OI_ASSERT(!OI_MBUF_IS_DIRTY(mbuf));
    OI_ASSERT(!OI_MBUF_IS_DIRTY(added));

    OI_DBGPRINT(("Append MBUF %lx to %lx", added, mbuf));

    if (mbuf->usedCells >= mbuf->allocCells) {
        OI_SLOG_ERROR(OI_STATUS_MBUF_OVERFLOW, ("MBUF Append Mbuf overflow: alloc %d", mbuf->allocCells));
        OI_MBUF_Dump(mbuf, "overflow in append mbuf");
        return OI_STATUS_MBUF_OVERFLOW;
    }
    cell = &mbuf->cells[mbuf->usedCells];

    mbuf->usedCells++;
    mbuf->numBytes += added->numBytes;
    cell->disposition = MBUF_NESTED;
    cell->buf.mcell.mbuf = added;
    /*
     * Save the windowing information from the inner MBUF
     */
    cell->buf.mcell.winStart = added->winStart;
    cell->len = added->numBytes;

    return OI_OK;
}


static OI_UINT16 PullBytes(OI_BYTE *dest,
                           OI_MBUF *src,
                           OI_UINT16 numBytes,
                           OI_UINT16 winPos)
{
    OI_BYTE *destStart = dest;
    OI_UINT8 currCell = src->currCell;
    OI_UINT16 currPos = src->currPos;

    /*
     * Immediately following a reset the mbuf tree is clean and data before the start of the window is
     * skipped. After the initial bytes have been pulled the currCell and currPos will be positioned
     * beyond the start of the window so winPos no longer applies.
     */
    if (OI_MBUF_IS_DIRTY(src)) {
        winPos = 0;
    }

    while (currCell < src->usedCells) {
        OI_MBUF_CELL *cell = &src->cells[currCell];
        OI_UINT16 len = cell->len;
        /*
         * If the current cell is completely outside the window skip it
         */
        if (len < winPos) {
            OI_ASSERT(currPos == 0);
            winPos -= len;
        } else {
            /*
             * Skip bytes that are outside the window
             */
            currPos += winPos;
            winPos = 0;
            /*
             * Cannot pull more bytes from the cell than available
             */
            OI_ASSERT(len >= currPos);
            len = OI_MIN(numBytes, len - currPos);

            if (cell->disposition == MBUF_NESTED) {
                /*
                 * Use the window position that size in effect when the mbuf was nested
                 */
                len = PullBytes(dest, cell->buf.mcell.mbuf, len, cell->buf.mcell.winStart + currPos);
            } else {
                const OI_BYTE *data = (cell->disposition == MBUF_INLINE) ? cell->buf.data : cell->buf.ptr;
                OI_MemCopy(dest, data + currPos, len);
            }
            currPos += len;
            dest += len;
            OI_ASSERT(numBytes >= len);
            numBytes -= len;
            /*
             * If there are no more bytes to pull we are done
             */
            if (numBytes == 0) {
                break;
            }
        }
        currPos = 0;
        ++currCell;
    }

    src->currCell = currCell;
    src->currPos = currPos;

    return (OI_UINT16)(dest - destStart);
}



/**
 * Moves bytes from an mbuf into a contiguous buffer.
 */

OI_UINT16 OI_MBUF_PullBytes(OI_BYTE *dest,
                            OI_MBUF *src,
                            OI_UINT16 numBytes)
{
    OI_UINT16 pulled = 0;

    OI_ASSERT(dest);
    OI_ASSERT(src);

    if (numBytes > 0) {
        pulled = PullBytes(dest, src, OI_MIN(src->numBytes, numBytes), src->winStart);
        OI_ASSERT(src->numBytes >= pulled);
        src->numBytes -= pulled;
    }
    return pulled;
}


static OI_UINT16 ComputeFCS(const OI_MBUF *mbuf,
                            OI_UINT16 numBytes,
                            OI_UINT16 winPos,
                            OI_UINT16 *runningFCS)
{
    OI_UINT16 checkedLen = 0;
    OI_UINT8 currCell;

    OI_ASSERT(!OI_MBUF_IS_DIRTY(mbuf));

    for (currCell = 0; currCell < mbuf->usedCells; ++currCell) {
        const OI_MBUF_CELL *cell = &mbuf->cells[currCell];
        OI_UINT16 len = cell->len;
        /*
         * If the current cell is outside the window skip it
         */
        if (len < winPos) {
            winPos -= len;
        } else {
            len = OI_MIN(numBytes, len - winPos);
            if (cell->disposition == MBUF_NESTED) {
                len = ComputeFCS(cell->buf.mcell.mbuf, len, winPos + cell->buf.mcell.winStart, runningFCS);
            } else {
                const OI_BYTE *data = (cell->disposition == MBUF_INLINE) ? cell->buf.data : cell->buf.ptr;
                OI_FCS_Compute(data + winPos, len, runningFCS);
            }
            checkedLen += len;
            numBytes -= len;
            if (numBytes == 0) {
                break;
            }
            winPos = 0;
        }
    }
    return checkedLen;
}


OI_UINT16 OI_MBUF_ComputeFCS(const OI_MBUF *mbuf,
                             OI_UINT16 numBytes,
                             OI_UINT16 *runningFCS)
{
    OI_ASSERT(mbuf);
    return ComputeFCS(mbuf, OI_MIN(numBytes, mbuf->numBytes), mbuf->winStart, runningFCS);
}


OI_UINT16 OI_MBUF_SetWindowAt(OI_MBUF *mbuf,
                              OI_UINT16 winStart,
                              OI_UINT16 winSize)
{
    OI_UINT16 i;
    OI_UINT16 len = 0;

    OI_ASSERT(mbuf);
    OI_ASSERT(!OI_MBUF_IS_DIRTY(mbuf));

    OI_DBGPRINT(("SetWindowAt %lx (current pos %d) at %d size %d", mbuf, mbuf->winStart, winStart, winSize));

    /*
     * Recalculate the total number of bytes in the mbuf
     */
    for (i = 0; i < mbuf->usedCells; ++i) {
        len += mbuf->cells[i].len;
    }
    /*
     * Apply the windowing
     */
    mbuf->winStart = OI_MIN(len, winStart);
    mbuf->winSize = OI_MIN(len - mbuf->winStart, winSize);
    mbuf->numBytes = mbuf->winSize;

    return mbuf->winSize;
}


OI_UINT16 OI_MBUF_SetWindow(OI_MBUF *mbuf,
                            OI_UINT16 winSize)
{
    return OI_MBUF_SetWindowAt(mbuf, 0, winSize);
}


OI_UINT16 OI_MBUF_AdvanceWindow(OI_MBUF *mbuf,
                                OI_UINT16 winSize)
{
    OI_ASSERT(mbuf);
    return OI_MBUF_SetWindowAt(mbuf, mbuf->winStart + mbuf->winSize, winSize);
}


const OI_BYTE * OI_MBUF_PeekBytes(OI_MBUF *mbuf,
                                  OI_UINT16 offset,
                                  OI_UINT16 *contigLen)
{
    const OI_BYTE *ret = NULL;
    OI_INT i;

    /* Make offset relative to any defined window */
    offset += mbuf->winStart;

    for (i = 0; i < mbuf->usedCells; ++i) {
        OI_MBUF_CELL *cell = &mbuf->cells[i];
        if (offset >= cell->len) {
            offset -= cell->len;
        }
        else {
            if (MBUF_INLINE == cell->disposition) {
                *contigLen = cell->len - offset;
                ret = &cell->buf.data[0] + offset;
            }
            else if (MBUF_NESTED == cell->disposition) {
                ret = OI_MBUF_PeekBytes(cell->buf.mcell.mbuf, offset, contigLen);
            }
            else {
                *contigLen = cell->len - offset;
                ret = cell->buf.ptr + offset;
            }
            break;
        }
    }
    return ret;
}

#ifdef OI_DEBUG
OI_UINT16 OI_MBUF_NumBytes(OI_MBUF *mb)
{
    OI_ASSERT(mb);

    return (mb ? mb->numBytes : 0);
}
#endif


#ifdef OI_DEBUG

/**
 * Print out contents of an mbuf.
 */

static OI_UINT16 MBUF_Print(OI_UINT indent,
                     OI_MBUF *mbuf,
                     OI_UINT16 numBytes,
                     OI_UINT16 winPos)
{
    OI_UINT i;
    OI_UINT16 currPos = mbuf->currPos;
    OI_UINT8 currCell = mbuf->currCell;
    OI_UINT16 totalLen = 0;
    OI_CHAR inTxt[33];

    OI_ASSERT(indent < OI_ARRAYSIZE(inTxt));

    for (i = 0; i < indent; ++i) {
        inTxt[i] = ' ';
    }
    inTxt[indent] = 0;

    if (MBUF_IS_WINDOWED(mbuf)) {
        OI_Printf("MBUF window @%d len:%d ", winPos, mbuf->winSize);
    } else {
        OI_Printf("MBUF ");
    }
    if (OI_MBUF_IS_DIRTY(mbuf)) {
        winPos = 0;
        OI_Printf("(dirty) ");
    }
    if (numBytes == 0) {
        OI_Printf("empty\n");
    } else {
        OI_Printf("\n");
        while (currCell < mbuf->usedCells) {
            OI_MBUF_CELL *cell = &mbuf->cells[currCell];
            OI_UINT16 len = cell->len;
            /*
             * If the current cell is outside the window skip it
             */
            if (len < winPos) {
                OI_ASSERT(currPos == 0);
                winPos -= len;
            } else {
                /*
                 * Skip bytes that are outside the window
                 */
                currPos += winPos;
                winPos = 0;
                OI_ASSERT(len >= currPos);
                len = OI_MIN(numBytes, len - currPos);

                if (cell->disposition == MBUF_NESTED) {
                    OI_Printf("%s[%d] ", inTxt, currCell);
                    /*
                     * Use the window position and size in effect when the mbuf was nested
                     */
                    len = MBUF_Print(indent + 4, cell->buf.mcell.mbuf, len, cell->buf.mcell.winStart + currPos);
                } else {
                    const OI_BYTE *data = (cell->disposition == MBUF_INLINE) ? cell->buf.data : cell->buf.ptr;
                    /*
                     * Skip bytes that are outside the window
                     */
                    OI_Printf("%s[%d] %@", inTxt, currCell, data + currPos, len);
                    currPos += len;
                }
                currPos += len;
                totalLen += len;
                numBytes -= len;
                if (numBytes == 0) {
                    break;
                }
            }
            currPos = 0;
            ++currCell;
        }
    }
    return totalLen;
}


void OI_MBUF_Print(OI_MBUF *mbuf)
{
    MBUF_Print(0, mbuf, mbuf->numBytes, mbuf->winStart);
}

/**
 * Print out structure of an mbuf.
 */

static void MBUF_Dump(OI_UINT indent,
                      OI_MBUF *mbuf,
                      OI_CHAR *tag)
{
    OI_UINT i;
    OI_CHAR inTxt[33];

    OI_ASSERT(indent < OI_ARRAYSIZE(inTxt));

    for (i = 0; i < indent; ++i) {
        inTxt[i] = ' ';
    }
    inTxt[indent] = 0;

    OI_Printf("%sMBUF%s %lx %s:\n", inTxt, OI_MBUF_IS_DIRTY(mbuf) ? " (dirty)" : "", mbuf, tag);
    if (MBUF_IS_WINDOWED(mbuf)) {
        OI_Printf("%sis windowed winStart=%d winSize=%d\n", inTxt, mbuf->winStart, mbuf->winSize);
    }
    OI_Printf("%salloc=%d used=%d\n", inTxt, mbuf->allocCells, mbuf->usedCells);
    OI_Printf("%snumBytes=%d\n", inTxt, mbuf->numBytes);
    OI_Printf("%scurrCell=%d currPos=%d\n", inTxt, mbuf->currCell, mbuf->currPos);

    for (i = 0; i < mbuf->usedCells; ++i) {
        OI_MBUF_CELL *cell = &mbuf->cells[i];
        const OI_BYTE *p;
        if (cell->disposition == MBUF_NESTED) {
            OI_Printf("%scell[%d] NESTED len=%d winStart=%d\n", inTxt, i, cell->len, cell->buf.mcell.winStart);
            MBUF_Dump(indent + 4, cell->buf.mcell.mbuf, "");
            continue;
        }
        if (cell->disposition == MBUF_INLINE) {
            p = cell->buf.data;
        } else {
            p = cell->buf.ptr;
        }
        if (cell->len) {
            OI_UINT16 max = OI_MIN(cell->len, 8);
            OI_Printf("%scell[%d] len=%d data=%@", inTxt, i, cell->len, p, max);
        } else {
            OI_Printf("%scell[%d] EMPTY\n", inTxt, i);
        }
    }
}


void OI_MBUF_Dump(OI_MBUF *mbuf,
                  OI_CHAR *tag)
{
    MBUF_Dump(0, mbuf, tag);
}

#endif /* OI_DEBUG */


