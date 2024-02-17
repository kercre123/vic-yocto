#ifndef _OI_FIFO_BUF_Q_H
#define _OI_FIFO_BUF_Q_H

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
 *FIFO buffer utility
 *
 * This utility provides a FIFO queue for arbitrary-length data buffers.
 *        - OI_Fifobuf_create()
 *        - OI_Fifobuf_enqueue()
 *        - OI_FifoBufQ_getEnqueueBuf()
 *        - OI_Fifobuf_dequeue()
 *        - OI_Fifobuf_dequeue_release()
 *
 *    Each element in the queue is a buffer that has been enqueued. The dequeue process returns
 *    buffers in the same order as that in which they were enqueued.
 *
 *   The queue is a block of data that is allocated by the queue user but is managed by this
 *    queue utility.
 */

#include "oi_common.h"

/** \addtogroup Misc_Internal */
/**@{*/

#ifdef __cplusplus
extern "C" {
#endif


/* Queue size and element length are each limited to 64kb.
  Also, a queue element cannot be larger than 1/2 the queue size. */

/** queue size, limited to 64kilobytes */
typedef OI_UINT16   OI_QSIZE ;

/** queue element length, limited to 1/2 the queue size  */
typedef OI_UINT16   OI_Q_ELEMENT_SIZE ;



/**
 * This function creates a FIFO buffer queue.
 *
 * @param pFifoQueue    This is a pointer to a block of memory that will managed by
 *                      this queue process. Total size of the queue should be large
 *                      enough to hold the maximum amount of data that may be queued
 *                      at any one time plus some.  The "plus some" should consider the
 *                      following:
 *                        - Each queued buffer requires a few bytes of overhead.
 *                        - At times, some buffer space may be wasted in order to keep a
 *                          queued item's data contiguous when the queue wraps.
 *
 *                    @note: The memory block must be 32-bit aligned.
 *
 * @param fifoQueueSize   total size of the queue
 */
OI_STATUS   OI_FifoBufQ_Create(
                    OI_BYTE             *pFifoQueue,
                    OI_QSIZE            fifoQueueSize) ;

/**
 * This function closes a FIFO buffer queue.
 *
 * This function marks the queue so that subsequent enqueue/dequeue requests will fail.
 *
 * @param pFifoQueue    Pointer to the queue.
 *
 */
OI_STATUS   OI_FifoBufQ_Close(
                    OI_BYTE             *pFifoQueue) ;

/**
 * This function enqueues an external data buffer.
 *
 * This function adds a copy of the caller's data buffer to the end of the FIFO queue.
 * In this context, external means that the caller's data buffer is owned by the caller (i.e.,
 * buffer was not obtained via OI_FifoBufQ_GetEnqueueBuf()).
 *        @note: Since a copy of the data is enqueued, the original data buffer is owned by the
 *               caller and there are no constraints on its used after enqueue().
 *
 * @param pFifoQueue pointer to the FIFO queue
 *
 * @param dataLen   length of the buffer to be enqueued
 *
 * @param pData     pointer to the data buffer to be enqueued
 */
OI_STATUS   OI_FifoBufQ_Enqueue_ExtBuf(
                    OI_BYTE             *pFifoQueue,
                    OI_Q_ELEMENT_SIZE   dataLen,
                    OI_BYTE             *pData) ;

/**
 * This function enqueues a data buffer that was previously obtained via OI_FifoBufQ_getEnqueueBuf().
 * Since the data is already in the queue buffer, no copying of data is needed.
 *
 *      @note: The caller must not access the data buffer after it has been enqueued;
 *             the buffer is owned by the queue process, not by the caller.
 *
 * There is no length parameter because the length was captured in the call to OI_FifoBufQ_GetEnqueueBuf().
 *
 * @param pFifoQueue pointer to the FIFO queue
 *
 * @param pData      pointer to the data buffer to be enqueued, previously obtained via OI_FifoBufQ_getEnqueueBuf()
 */
OI_STATUS   OI_FifoBufQ_Enqueue_QBuf(
                    OI_BYTE             *pFifoQueue,
                    OI_BYTE             *pData) ;

/**
 * This function gets a queue data buffer in preparation for enqueue operation.
 *
 * This function returns a pointer to the buffer that is the next buffer to be used to
 * enqueue data. Caller must subsequently call OI_FifoBufQ_enqueue() with this same pointer.
 *
 * @param pFifoQueue pointer to the FIFO queue
 *
 * @param dataLen    length of the buffer to be enqueud
 *      
 * @return           pointer to a data buffer, NULL if there is no buffer available
 */
OI_BYTE *OI_FifoBufQ_GetEnqueueBuf(
                    OI_BYTE             *pFifoQueue,
                    OI_Q_ELEMENT_SIZE   dataLen) ;

/**
 * This function dequeues a data buffer.
 *
 * This function returns the next queued buffer. For performance reasons, the dequeued buffer
 * is returned by reference; i.e., the dequeue process does not make a copy of the data. This
 * leads to some requirements that the caller must adhere to:
 *
 *      - Caller must call OI_Fifobuf_dequeue_release() after consuming the dequeued data.
 *      - Caller may not block between OI_Fifobuf_dequeue() and OI_Fifobuf_dequeue_release().
 *      - Caller may not have more than one dequeued packet outstanding at any one time.
 *
 * Failure to follow these conventions will result in a corrupt queue.
 *
 * @param pFifoQueue pointer to the FIFO queue
 *
 * @param pDataLen  This pointer points to the caller's variable where the length of the dequeued
 *                  buffer will be stored. If there is no buffer to dequeue, *pDataLen is set to zero.
 *
 * @return          pointer to the dequeued buffer, NULL if there is no buffer to dequeue
 */
OI_BYTE *OI_FifoBufQ_Dequeue(
                    OI_BYTE             *pFifoQueue,
                    OI_Q_ELEMENT_SIZE   *pDataLen) ;

/**
 * This function releases a dequeued data buffer.
 *
 * This function releases the buffer that was previously dequeued.
 *
 * @param pFifoQueue pointer to the FIFO queue
 *
 * @param pData     pointer to buffer previously dequeued
 *
 */
void    OI_FifoBufQ_Dequeue_Release(
                    OI_BYTE             *pFifoQueue,
                    OI_BYTE             *pData) ;

/**************************************************************************************/
#ifdef __cplusplus
}
#endif

/**@}*/

#endif /* _OI_FIFO_BUF_Q_H */

