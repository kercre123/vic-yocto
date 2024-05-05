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

functions shared between OBEX server and OBEX client
*/

#define __OI_MODULE__ OI_MODULE_OBEX_SRV

#include "oi_obex_lower.h"
#include "oi_obexcommon.h"
#include "oi_obextext.h"
#include "oi_obexspec.h"
#include "oi_memmgr.h"
#include "oi_utils.h"
#include "oi_mbuf.h"
#include "oi_status.h"
#include "oi_assert.h"
#include "oi_text.h"
#include "oi_debug.h"
#include "oi_argcheck.h"

#define PRIORITY_HEADER(h) ((OI_OBEX_HDR_TARGET == h) || (OI_OBEX_HDR_CONNECTION_ID == h) || (OI_OBEX_HDR_SESSION_PARAMS == h))

#ifdef OI_DEBUG
static void DumpHeaderLists(OI_OBEX_HEADER *hdrs,
                            OI_INT count,
                            const OI_OBEX_HEADER_LIST* hdrList)
{
    OI_INT i;

    /*
     * First, dump priority headers.  Maintain same order
     * created in AddHeader() (priority headers are prepended).
     */
    if (NULL != hdrList) {
        for (i = hdrList->count-1; i >= 0; i--) {
            if (PRIORITY_HEADER(hdrList->list[i].id)) {
                OI_DBGPRINT2(("\t%=\n", &hdrList->list[i]));
            }
        }
    }
    for (i = count-1; i >= 0; i--) {
        if (PRIORITY_HEADER(hdrs[i].id)) {
            OI_DBGPRINT2(("\t%=\n", &hdrs[i]));
        }
    }

    /*
     * Now, dump normal headers in their expected order.
     */
    for (i = 0; i < count; i++) {
        if (!PRIORITY_HEADER(hdrs[i].id)) {
            OI_DBGPRINT2(("\t%=\n", &hdrs[i]));
        }
    }
    if (NULL != hdrList) {
        for (i = 0; i < hdrList->count; i++) {
            if (!PRIORITY_HEADER(hdrList->list[i].id)) {
                OI_DBGPRINT2(("\t%=\n", &hdrList->list[i]));
            }
        }
    }
}
#else
#define DumpHeaderLists(h, c, l)
#endif




static OI_UINT16 HeaderSize(OI_OBEX_HEADER *header)
{
    OI_UINT16 length = 0;

    switch (OI_OBEX_HDR_KIND(header->id)) {
        case OI_OBEX_HDR_ID_UNICODE:
            length = OI_OBEX_HEADER_PREFIX_LEN;
            if (header->val.unicode.len > 0) {
                /*
                 * Non-empty unicode strings must be NULL terminated.
                 */
                if (header->val.unicode.str[header->val.unicode.len - 1] != 0) {
                    OI_LOG_ERROR(("OBEX unicode must be NUL terminated"));
                }
                length += header->val.unicode.len * sizeof(OI_CHAR16);
            }
            break;
        case OI_OBEX_HDR_ID_BYTESEQ:
            length = OI_OBEX_HEADER_PREFIX_LEN + header->val.byteseq.len;
            break;
        case OI_OBEX_HDR_ID_UINT8:
            length = sizeof(OI_UINT8) + sizeof(OI_UINT8);
            break;
        case OI_OBEX_HDR_ID_UINT32:
            length = sizeof(OI_UINT8) + sizeof(OI_UINT32);
            break;
        default:
            OI_LOG_ERROR(("Bad OBEX header"));
            break;
    }
    return length;
}


OI_UINT16 OI_OBEXCOMMON_HeaderListSize(const OI_OBEX_HEADER_LIST *headers)
{
    OI_UINT i;
    OI_UINT16 size = 0;

    if (headers != NULL) {
        for (i = 0; i < headers->count; ++i) {
            size += HeaderSize(headers->list + i);
        }
    }
    return size;
}


static OI_STATUS AddHeader(OBEX_COMMON *common,
                           OI_MBUF *mbuf,
                           OI_OBEX_HEADER *header,
                           OI_UINT16 *pktLen)
{
    OI_UINT hdrId = header->id;
    OI_STATUS status;
    OI_BYTE tmp[8];
    OI_BYTE *buf;
    OI_BYTE_STREAM bs;
    OI_UINT i;
    OI_UINT16 pos;
    OI_UINT16 length = HeaderSize(header);
    OI_STATUS (*addToMbuf)(OI_MBUF*, const OI_BYTE*, OI_UINT16, OI_MBUF_DISPOSITION);

    if (length == 0) {
        return OI_STATUS_INVALID_PARAMETERS;
    }
    /*
     * Check the packet has room for this header. We will segment body and
     * end-of-body headers across multiple packets if necessary.
     */
    if ((*pktLen + length) > common->maxSendPktLen) {
        if (!OI_OBEX_IS_A_BODY_HEADER(header->id)) {
            return OI_OBEX_PACKET_OVERFLOW;
        }
        OI_DBGPRINT(("Segmenting body header total=%d", length + *pktLen));
        /*
         * How much data will fit in this packet?
         */
        length = common->maxSendPktLen - *pktLen;
        if (length <= OI_OBEX_HEADER_PREFIX_LEN) {
            OI_DBGPRINT(("Segmenting body header - no room to send body this time"));
            return OI_OK;
        }
        OI_DBGPRINT(("Segmenting size=%d", length));
        OI_MemCopy(&common->bodySegment, header, sizeof(OI_OBEX_HEADER));
        common->bodySegment.val.byteseq.data += (length - OI_OBEX_HEADER_PREFIX_LEN);
        common->bodySegment.val.byteseq.len -= (length - OI_OBEX_HEADER_PREFIX_LEN);
        /*
         * Always a body not end-of-body
         */
        hdrId = OI_OBEX_HDR_BODY;
    }

    *pktLen += length;

    /*
     * Certain headers are required to come first in the header list.
     */
    if (PRIORITY_HEADER(hdrId)) {
        addToMbuf = &OI_MBUF_Prepend;
    }
    else {
        addToMbuf = &OI_MBUF_Append;
    }

    switch (OI_OBEX_HDR_KIND(hdrId)) {
        case OI_OBEX_HDR_ID_UNICODE:
            /*
             * We want to preserve the byte order of the unicode string so we
             * have to allocate a buffer and copy the string over.
             */
            buf = OI_Malloc(length);
            if (buf == NULL) {
                return OI_STATUS_OUT_OF_MEMORY;
            }
            ByteStream_Init(bs, buf, length);
            ByteStream_Open(bs, BYTESTREAM_WRITE);
            ByteStream_PutUINT8(bs, hdrId);
            ByteStream_PutUINT16(bs, length, OI_OBEX_BO);
            for (i = 0; i < header->val.unicode.len; ++i) {
                ByteStream_PutUINT16(bs, header->val.unicode.str[i], OI_OBEX_BO);
            }
            /*
             * Unicode strings are supposed to be NUL terminated if this one
             * isn't HeaderSize() will have allocated space for one.
             */
            pos = ByteStream_GetPos(bs);
            if (pos < length) {
                ByteStream_PutUINT16(bs, 0, OI_OBEX_BO);
            }
            ByteStream_Close(bs);
            /*
             * MBUF will free the buffer when it is no longer needed.
             */
            status = addToMbuf(mbuf,
                               ByteStream_GetDataPointer(bs),
                               ByteStream_GetSize(bs),
                               MBUF_FREE);

            break;
        case OI_OBEX_HDR_ID_BYTESEQ:
            tmp[0] = hdrId;
            SetUINT16_BigEndian(&tmp[1], length);

            if (&OI_MBUF_Append == addToMbuf) {
                /*
                 * Appending, so add hdrId first
                 */
                status = OI_MBUF_Append(mbuf, tmp, OI_OBEX_HEADER_PREFIX_LEN, MBUF_COPY);
                if (OI_SUCCESS(status)) {
                    status = OI_MBUF_Append(mbuf, header->val.byteseq.data, (OI_UINT16)(length - OI_OBEX_HEADER_PREFIX_LEN), MBUF_KEEP);
                }
            }
            else {
                /*
                 * When prepending, reverse order of calls to make
                 * hdrId show up before data.
                 */
                status = OI_MBUF_Prepend(mbuf, header->val.byteseq.data, (OI_UINT16)(length - OI_OBEX_HEADER_PREFIX_LEN), MBUF_KEEP);
                if (OI_SUCCESS(status)) {
                    status = OI_MBUF_Prepend(mbuf, tmp, OI_OBEX_HEADER_PREFIX_LEN, MBUF_COPY);
                }
            }

            if (OI_SUCCESS(status)) {
                /*
                 * Track progress for body data
                 */
                if (OI_OBEX_IS_A_BODY_HEADER(hdrId)) {
                    common->progressBytes += length;
                }
            }
            break;
        case OI_OBEX_HDR_ID_UINT8:
            tmp[0] = hdrId;
            tmp[1] = header->val.uInt8;
            status = addToMbuf(mbuf, tmp, 2 * sizeof(OI_UINT8), MBUF_COPY);
            break;
        case OI_OBEX_HDR_ID_UINT32:
            tmp[0] = hdrId;
            SetUINT32_BigEndian(&tmp[1], header->val.uInt32);
            status = addToMbuf(mbuf, tmp, sizeof(OI_UINT8) + sizeof(OI_UINT32), MBUF_COPY);
            break;
        default:
            status = OI_FAIL;
    }
    return status;
}


/*
 * Marshall packet flags and headers into an mbuf.
 */
OI_STATUS OI_OBEXCOMMON_MarshalPacketMbuf(OBEX_COMMON       *common,
                                          OI_BYTE_STREAM    *pktHdr,
                                          OI_OBEX_HEADER    *hdrs,
                                          OI_UINT16         hdrCount,
                                          OI_OBEX_HEADER_LIST const *hdrList,
                                          OI_MBUF           **callersMbuf)
{
    OI_STATUS status;
    OI_UINT8 cells;
    OI_UINT16 pos;
    OI_UINT16 pktLen;
    OI_UINT16 hdrCount2 = (hdrList == NULL) ? 0 : hdrList->count;
    OI_UINT i;
    OI_MBUF *mbuf = NULL;

    OI_DBGPRINT2(("OI_OBEXCOMMON_MarshalPacketMbuf"));

    OI_ASSERT(callersMbuf != NULL);
    OI_ASSERT(common->bodySegment.id == 0);

#ifdef OI_DEBUG
    OI_DBGPRINT2(("Headers out {"));
    DumpHeaderLists(hdrs, hdrCount, hdrList);
    OI_DBGPRINT2(("}"));
#endif

    /*
     * One cell for the packet header + at least one cell per OBEX header.
     */
    cells = (OI_UINT8) (hdrCount + hdrCount2 + 1);
    /*
     * Byteseq headers use two mbuf cells.
     */
    for (i = 0; i < hdrCount; ++i) {
        if (OI_OBEX_HDR_KIND(hdrs[i].id) == OI_OBEX_HDR_ID_BYTESEQ) {
            ++cells;
        }
    }
    for (i = 0; i < hdrCount2; ++i) {
        if (OI_OBEX_HDR_KIND(hdrList->list[i].id) == OI_OBEX_HDR_ID_BYTESEQ) {
            ++cells;
        }
    }
    /*
     * Get the initial packet length.
     */
    pktLen = ByteStream_GetPos(*pktHdr);
    /*
     * Allocate and initialize the mbufs.
     */
    mbuf = OI_MBUF_Alloc(cells);
    OI_DBGPRINT2(("Allocating MBUF %lx", mbuf));
    if (mbuf == NULL) {
        status = OI_STATUS_OUT_OF_MEMORY;
        goto ErrorExit;
    }
    for (i = 0; i < hdrCount; ++i) {
        status = AddHeader(common, mbuf, &hdrs[i], &pktLen);
        if (!OI_SUCCESS(status)) {
            goto ErrorExit;
        }
    }
    for (i = 0; i < hdrCount2; ++i) {
        status = AddHeader(common, mbuf, &hdrList->list[i], &pktLen);
        if (!OI_SUCCESS(status)) {
            goto ErrorExit;
        }
    }

    pos = ByteStream_GetPos(*pktHdr);
    ByteStream_SetPos(*pktHdr, 0);
    /*
     * The packet is not "final" if a body header had to be segmented.
     */
    if (OI_OBEX_IS_A_BODY_HEADER(common->bodySegment.id)) {
        OI_UINT op = common->currentOpcode;
        if (op == OI_OBEX_FINAL(OI_OBEX_RSP_OK) || (op == OI_OBEX_FINAL(OI_OBEX_RSP_CONTINUE))) {
            op = OI_OBEX_FINAL(OI_OBEX_RSP_CONTINUE);
        } else {
            op &= ~OI_OBEX_FINAL_BIT;
        }
        ByteStream_PutUINT8(*pktHdr, op);
    } else {
        ByteStream_PutUINT8(*pktHdr, common->currentOpcode);
    }
    ByteStream_PutUINT16(*pktHdr, pktLen, OI_OBEX_BO);
    ByteStream_SetPos(*pktHdr, pos);
    ByteStream_Close(*pktHdr);

    status = OI_MBUF_Prepend(mbuf,
                             ByteStream_GetDataPointer(*pktHdr),
                             ByteStream_GetSize(*pktHdr),
                             MBUF_COPY);

    if (!OI_SUCCESS(status)) {
        goto ErrorExit;
    }

    OI_DBGPRINT2(("OI_OBEXCOMMON_MarshalPacketMbuf len = %d", pktLen));

    *callersMbuf = mbuf;

    return OI_OK;

ErrorExit:

    OI_DBGPRINT(("OI_OBEXCOMMON_MarshalPacketMbuf error exit %d", status));

    if (mbuf) {
        OI_MBUF_Free(mbuf);
    }
    *callersMbuf = NULL;
    return status;
}

/*
 * Marshall packet flags and headers into a the mbuf in common structure.
 */
OI_STATUS OI_OBEXCOMMON_MarshalPacket(OBEX_COMMON *common,
                                      OI_BYTE_STREAM *pktHdr,
                                      OI_OBEX_HEADER *hdrs,
                                      OI_UINT16 hdrCount,
                                      OI_OBEX_HEADER_LIST const *hdrList)
{
    OI_DBGPRINT2(("OBEXCOMMON_MarshalPacket"));

    OI_ASSERT(common->mbuf == NULL);
    return OI_OBEXCOMMON_MarshalPacketMbuf(common, pktHdr, hdrs, hdrCount, hdrList, &common->mbuf);
}


OI_STATUS OI_OBEXCOMMON_MarshalBodySegment(OBEX_COMMON *common)
{
    OI_BYTE_STREAM pkt;
    OI_OBEX_HEADER hdr;

    OI_ASSERT(OI_OBEX_IS_A_BODY_HEADER(common->bodySegment.id));

    OI_DBGPRINT(("MarshalBodySegment"));

    OI_MemCopy(&hdr, &common->bodySegment, sizeof(OI_OBEX_HEADER));
    common->bodySegment.id = 0;

    OI_OBEXCOMMON_InitPacket(common, common->currentOpcode, &pkt);
    return OI_OBEXCOMMON_MarshalPacket(common, &pkt, &hdr, 1, NULL);
}


OI_STATUS OI_OBEXCOMMON_SendBulk(OBEX_COMMON *common,
                                 OBEX_BULK_DATA_LIST *current,
                                 OI_BOOL *busy,
                                 OI_BOOL *final)
{
    OI_STATUS status = OI_OK;
    OI_UINT8 headerId = OI_OBEX_HDR_BODY;
    OI_UINT8 opcode = common->currentOpcode;
    OBEX_BULK_DATA_LIST bulkDataCurrent = *current;
    OI_UINT8 hdrBuf[BULK_PUT_HDR_SIZE];

    OI_DBGPRINT2(("OI_OBEXCOMMON_SendBulk current:%#08x", *current));

    if (final) {
        *final = FALSE;
    }
    /*
     * Check if we have anything to send
     */
    if (bulkDataCurrent == NULL) {
        return OI_OK;
    }

    OI_ASSERT(bulkDataCurrent->bytesSent <= bulkDataCurrent->blockSize);
    OI_ASSERT(bulkDataCurrent->bytesConfirmed <= bulkDataCurrent->blockSize);

    do {
        OI_BYTE_STREAM bs;
        OI_MBUF *mbuf;
        OI_UINT numBlocks = 0;
        OI_UINT32 bodyMax = common->maxSendPktLen - sizeof(hdrBuf);
        OI_UINT32 bodyLen = 0;
        OBEX_BULK_DATA_LIST bulkData;

        for (bulkData = bulkDataCurrent; bulkData != NULL; bulkData = bulkData->next) {
            OI_UINT32 toSend = (bulkData->blockSize - bulkData->bytesSent);
            ++numBlocks;
            /*
             * Can we only send a partial block?
             */
            if (toSend > bodyMax) {
                OI_DBGPRINT2(("%#08x SendBulk[%d] partial %d", bulkData, numBlocks, bodyMax));
                bodyLen += bodyMax;
                break;
            }
            /*
             * If the entire final data block can be sent this is the final send.
             */
            if (bulkData->final) {
                OI_DBGPRINT2(("%#08x SendBulk[%d] final %d", bulkData, numBlocks, toSend));
                /*
                 * The final response needs the final bit set and END_OF_BODY header.
                 */
                if (OI_OBEX_FINAL(opcode) == OI_OBEX_FINAL(OI_OBEX_RSP_CONTINUE)) {
                    opcode = OI_OBEX_RSP_OK;
                }
                headerId = OI_OBEX_HDR_END_OF_BODY;
                opcode = OI_OBEX_FINAL(opcode);
                if (final) {
                    *final = TRUE;
                }
                OI_ASSERT(bulkData->next == NULL);
            } else {
                OI_DBGPRINT2(("%#08x SendBulk[%d] complete %d", bulkData, numBlocks, toSend));
            }
            bodyMax -= toSend;
            bodyLen += toSend;
        }
        /*
         * We need one cell for the packet header + body header and one for each data block.
         */
        mbuf = OI_MBUF_Alloc(1 + numBlocks);
        OI_DBGPRINT2(("Allocating MBUF %lx", mbuf));
        if (mbuf == NULL) {
            status = OI_STATUS_OUT_OF_MEMORY;
            break;
        }
        ByteStream_Init(bs, hdrBuf, sizeof(hdrBuf));
        ByteStream_Open(bs, BYTESTREAM_WRITE);
        /*
         * Put the OBEX packet header
         */
        ByteStream_PutUINT8(bs, opcode);
        ByteStream_PutUINT16(bs, bodyLen + sizeof(hdrBuf), OI_OBEX_BO);
        /*
         * Put the Body packet header
         */
        ByteStream_PutUINT8(bs, headerId);
        ByteStream_PutUINT16(bs, bodyLen + OI_OBEX_HEADER_PREFIX_LEN, OI_OBEX_BO);
        ByteStream_Close(bs);

        OI_MBUF_Append(mbuf, ByteStream_GetDataPointer(bs), ByteStream_GetSize(bs), MBUF_COPY);

        OI_DBGPRINT2(("SendBulk numBlocks:%d bytes:%d", numBlocks, bodyLen));

        bulkData = bulkDataCurrent;
        while (numBlocks--) {
            OI_UINT32 len = bulkData->blockSize - bulkData->bytesSent;
            if (len > bodyLen) {
                len = bodyLen;
            }
            OI_MBUF_Append(mbuf, bulkData->blockBuffer + bulkData->bytesSent, (OI_UINT16)len, MBUF_KEEP);
            bodyLen -= len;
            bulkData->bytesSent += len;
            if (bulkData->bytesSent == bulkData->blockSize) {
                bulkData = bulkData->next;
            }
        }
        /*
         * Accelerate the write confirm if we have run out of data
         */
        status = common->lowerConnection->ifc->write(common->lowerConnection, mbuf, (bulkData == NULL), busy);
        if (OI_SUCCESS(status)) {
            /*
             * Set bulkDataCurrent as context on mbuf so it is available in the write confirm callback
             */
            mbuf->context.v = bulkDataCurrent;
        } else {
            OI_MBUF_Free(mbuf);
            OI_DBGPRINT(("OBEXCOMMON send bulk failed (%d)", status));
            break;
        }
        /*
         * Update current bulk data pointer
         */
        bulkDataCurrent = bulkData;
        /*
         * If we are not doing SRM sends are serialized.
         */
        if (!(common->srm & OI_OBEX_SRM_ENABLED)) {
            OI_DBGPRINT2(("SRM not enabled - writes serialized"));
            /*
             * This prevents any more data from being sent until the response packet is received.
             */
            *busy = TRUE;
            break;
        }
        /*
         * Continue while the queue is not full and we have data to send.
         */
    } while ((bulkDataCurrent != NULL) && !(*busy));

    OI_DBGPRINT2(("SendBulk exiting current:%#08x busy:%d", bulkDataCurrent, *busy));
    /*
     * Return the updated current
     */
    *current = bulkDataCurrent;
    return status;

}

/*
 * Allocate data structures to track the bulk data transfer
 */
OI_STATUS OI_OBEXCOMMON_AllocBulkData(OI_UINT8 numBuffers,
                                      OI_UINT8 *bulkDataBuffer[],
                                      OI_UINT32 bufferLength[],
                                      OBEX_BULK_DATA_LIST *bulkDataHead,
                                      OBEX_BULK_DATA_LIST *bulkDataTail)
{
    OI_STATUS status = OI_OK;
    OI_UINT i;

    *bulkDataHead = NULL;
    *bulkDataTail = NULL;

    /*
     * Allocate data structures to track the bulk data transfer
     */
    for (i = 0; i < numBuffers; ++i) {
        OBEX_BULK_DATA_LIST newBD = OI_Malloc(sizeof(*newBD));
        if (newBD == NULL) {
            status = OI_STATUS_NO_RESOURCES;
            break;
        }
        OI_DBGPRINT2(("Alloc bulk data %#08x", newBD));
        newBD->blockSize = bufferLength[i];
        if (newBD->blockSize > 0) {
            newBD->blockBuffer = bulkDataBuffer[i];
        } else {
            newBD->blockBuffer = NULL;
        }
        newBD->next = NULL;
        newBD->final = FALSE;
        newBD->bytesSent = 0;
        newBD->bytesConfirmed = 0;
        if (*bulkDataHead == NULL) {
            *bulkDataHead = newBD;
        }
        if (*bulkDataTail != NULL) {
            (*bulkDataTail)->next = newBD;
        }
        *bulkDataTail = newBD;
    }
    if (!OI_SUCCESS(status)) {
        OI_OBEXCOMMON_FreeBulkData(*bulkDataHead);
        *bulkDataHead = NULL;
        *bulkDataTail = NULL;
    }
    return status;
}


void OI_OBEXCOMMON_FreeBulkData(OBEX_BULK_DATA_LIST list)
{
    while (list) {
        OBEX_BULK_DATA_LIST next = list->next;
        OI_DBGPRINT2(("Free bulk data %#08x", list));
        OI_Free(list);
        list = next;
    }
}


void OI_OBEXCOMMON_InitPacket(OBEX_COMMON *common,
                              OI_UINT8 opcode,
                              OI_BYTE_STREAM *pkt)
{
    OI_DBGPRINT2(("OI_OBEXCOMMON_InitPacket %s%s", OI_OBEX_PktText(opcode), OI_OBEX_IS_FINAL(opcode) ? "(final)" : ""));

    common->currentOpcode = opcode;
    common->bodySegment.id = 0;

    ByteStream_Init(*pkt, common->hdrBuf, OI_OBEX_HDR_BUFSIZE);
    ByteStream_Open(*pkt, BYTESTREAM_WRITE);
    /*
     * Packet opcode and size will be filled in later.
     */
    ByteStream_Skip(*pkt, sizeof(OI_UINT8) + sizeof(OI_UINT16));
}


/**
 * Send a simple OBEX command packet - just an opcode with no headers.
 */
OI_STATUS OI_OBEXCOMMON_SendSimple(OBEX_COMMON *common,
                                   OI_UINT8 opcode)
{
    OI_STATUS status;
    OI_MBUF *mbuf;
    OI_BYTE cmdBuf[3];
    OI_BOOL queueFull;

    OI_DBGPRINT2(("OBEXCOMMON send %s%s", OI_OBEX_PktText(opcode), OI_OBEX_IS_FINAL(opcode) ? "(final)" : ""));

    cmdBuf[0] = opcode;
    cmdBuf[1] = 0;
    cmdBuf[2] = sizeof(cmdBuf);
    mbuf = OI_MBUF_Wrap(cmdBuf, sizeof(cmdBuf), MBUF_COPY);
    if (mbuf == NULL)
        return OI_STATUS_OUT_OF_MEMORY;
    status = common->lowerConnection->ifc->write(common->lowerConnection, mbuf, TRUE, &queueFull);
    if (!OI_SUCCESS(status)) {
        OI_MBUF_Free(mbuf);
        OI_SLOG_ERROR(status, ("lower layer write() failed"));
    }
    return status;
}

static OI_STATUS ScanHeader(OI_BYTE_STREAM *bs)
{
    OI_UINT8  kind;
    OI_UINT8  id = 0;
    OI_UINT16 len = 0;

    ByteStream_GetUINT8_Checked(*bs, id);
    kind = OI_OBEX_HDR_KIND(id);

    OI_DBGPRINT(("Header Type = %d  Id =%d\n",kind,id));
    switch (kind)
    {
    case OI_OBEX_HDR_ID_BYTESEQ:
    case OI_OBEX_HDR_ID_UNICODE:
        ByteStream_GetUINT16_Checked(*bs, len, OI_OBEX_BO);
        OI_DBGPRINT(("Len decoded = %d \n",len));
        len -= OI_OBEX_HEADER_PREFIX_LEN;
        OI_DBGPRINT(("Len decoded = %d \n",len));
        if ((len & 1) && (kind == OI_OBEX_HDR_ID_UNICODE)) {
            OI_LOG_ERROR(("Bad OBEX Unicode len: %d (odd)\n", len));
            return OI_OBEX_ERROR;
        }
        break;
    case OI_OBEX_HDR_ID_UINT8:
        len = sizeof(OI_UINT8);
        break;
    case OI_OBEX_HDR_ID_UINT32:
        len = sizeof(OI_UINT32);
        break;
    default:
        OI_LOG_ERROR(("Unrecognized OBEX Header %2x\n", id));
        return OI_OBEX_ERROR;
    }

	OI_DBGPRINT((" ByteStream size = %d Header length =%d Byte Stream Current pos = %d\n",bs->__size,len,bs->__pos));
    ByteStream_Skip_Checked(*bs, len);
    if (ByteStream_Error(*bs)) {
        OI_LOG_ERROR(("Bad OBEX Header:(%2x) len: (%d)\n", id, len));
        return OI_OBEX_ERROR;
    } else {
        return OI_OK;
    }
}


static void ParseHeader(OBEX_COMMON *common,
                        OI_BYTE_STREAM *bs,
                        OI_OBEX_HEADER *header)
{
    OI_BYTE *base;
    OI_UINT16 dchar;
    OI_UINT i;
    OI_UINT16 len;

    ByteStream_GetUINT8(*bs, header->id);

    switch (OI_OBEX_HDR_KIND(header->id)) {
    case OI_OBEX_HDR_ID_UNICODE:
        ByteStream_GetUINT16(*bs, len, OI_OBEX_BO);
        len -= OI_OBEX_HEADER_PREFIX_LEN;
        OI_ASSERT((len % sizeof(OI_CHAR16)) == 0);
        header->val.unicode.len = len / sizeof(OI_CHAR16);
        if (header->val.unicode.len) {
            /*
             * Some architecture require that arrays of 16 bit values are
             * aligned on a 16 bit boundary. We can safely adjust the start
             * address of the unicode string to a 16 bit boundary because we
             * no longer need the preceding length byte.
             */
            base = ByteStream_GetCurrentBytePointer(*bs);
            if (((OI_UINT32) base) & 1) {
                --base;
            }
            header->val.unicode.str = (OI_CHAR16*) base;
            /*
             * Do in-place byte order conversion for double byte characters.
             */
            for (i = 0; i < header->val.unicode.len; ++i) {
                ByteStream_GetUINT16(*bs, dchar, OI_OBEX_BO);
                header->val.unicode.str[i] = (OI_CHAR16)dchar;
            }
        } else {
            header->val.unicode.str = NULL;
        }
        break;
    case OI_OBEX_HDR_ID_BYTESEQ:
        ByteStream_GetUINT16(*bs, len, OI_OBEX_BO);
        len -= OI_OBEX_HEADER_PREFIX_LEN;
        header->val.byteseq.len = len;
        if (header->val.byteseq.len) {
            header->val.byteseq.data = (OI_BYTE*) ByteStream_GetCurrentBytePointer(*bs);
        } else {
            header->val.byteseq.data = NULL;
        }
        ByteStream_Skip(*bs, len);
        /*
         * Track progress for body data
         */
        if (OI_OBEX_IS_A_BODY_HEADER(header->id)) {
            common->progressBytes += len;
        }
        break;
    case OI_OBEX_HDR_ID_UINT8:
        ByteStream_GetUINT8(*bs, header->val.uInt8);
        break;
    case OI_OBEX_HDR_ID_UINT32:
        ByteStream_GetUINT32(*bs, header->val.uInt32, OI_OBEX_BO);
        break;
    default:
        OI_LOG_ERROR(("OBEX Common Parse Header corrupt header"));
        break;
    }
}


/*
 * Look for a specific OBEX header in a header list.
 */
OI_STATUS OI_OBEXCOMMON_DeleteHeaderFromList(OI_OBEX_HEADER_LIST *headers,
                                             OI_UINT8 headerId)
{
    OI_UINT i;

    /*
     * Find the header to remove
     */
    for (i = 0; i < headers->count; ++i) {
        if (headers->list[i].id == headerId) {
            break;
        }
    }
    if (i == headers->count) {
        return OI_STATUS_NOT_FOUND;
    } else {
        headers->count -= 1;
        /*
         * Close the gap
         */
        while (i < headers->count) {
            headers->list[i] = headers->list[i + 1];
            ++i;
        }
    }
    return OI_OK;
}


/*
 * Look for a specific OBEX header in a header list.
 */
OI_OBEX_HEADER* OI_OBEX_FindHeader(OI_OBEX_HEADER_LIST const *headers,
                                   OI_UINT8 headerId)
{
    OI_UINT i;

    for (i = 0; i < headers->count; ++i) {
        if (headers->list[i].id == headerId) {
            return headers->list + i;
        }
    }
    return NULL;
}


/**
 * Get all headers. This is done without copying data from the original
 * bytestream, so headers reference directly into the appropriate locations in
 * the bytestream. As a side-effect the byte stream may be modified.
 */

OI_STATUS OI_OBEXCOMMON_ParseHeaderList(OBEX_COMMON *common,
                                        OI_OBEX_HEADER_LIST *headers,
                                        OI_BYTE_STREAM *bs)
{
    OI_STATUS status;
    OI_UINT i;
    OI_UINT16 pos;

    /*
     * First count the headers.
     */
    headers->count = 0;
    headers->list = NULL;
    pos = ByteStream_GetPos(*bs);
    while (ByteStream_NumReadBytesAvail(*bs) > 0) {
        status = ScanHeader(bs);
        if (!OI_SUCCESS(status)) {
            headers->count = 0;
            return OI_OBEX_BAD_PACKET;
        }
        ++headers->count;
    }
    ByteStream_SetPos(*bs, pos);

    if (headers->count > 0) {
        headers->list = OI_Malloc(sizeof(OI_OBEX_HEADER) * headers->count);
        if (headers->list == NULL) {
            return OI_STATUS_OUT_OF_MEMORY;
        }
        /*
         * Now get the header values into the header list. We checked that
         * the headers were all valid in the scan pass.
         */
        for (i = 0; i < headers->count; ++i) {
            ParseHeader(common, bs, headers->list + i);
        }
    }
    ByteStream_Close(*bs);

#ifdef OI_DEBUG
    OI_DBGPRINT2(("Headers in {"));
    DumpHeaderLists(headers->list, headers->count, NULL);
    OI_DBGPRINT2(("}"));
#endif
    return OI_OK;
}

static OI_STATUS ScanAppParamsEntry(OI_BYTE_STREAM *bs)
{
    OI_UINT8 len = 0;

    ByteStream_Skip_Checked(*bs, sizeof(OI_UINT8)); /* The length doesn't depend on the value of the param tag */
    ByteStream_GetUINT8_Checked(*bs, len);
    ByteStream_Skip_Checked(*bs, len);

    if (ByteStream_Error(*bs)) {
        return OI_OBEX_ERROR;
    } else {
        return OI_OK;
    }
}

static void ParseAppParamsEntry(OI_BYTE_STREAM *bs,
                                OI_OBEX_APP_PARAM *param)
{
    /* Not using checked bytestream operations because ScanAppParamsEntry has already validated the field layout */
    ByteStream_GetUINT8(*bs, param->tag);
    ByteStream_GetUINT8(*bs, param->len);
    param->data = ByteStream_GetCurrentBytePointer(*bs);
    ByteStream_Skip(*bs, param->len);
}

OI_STATUS OI_OBEX_ParseAppParamsHeader(const OI_OBEX_BYTESEQ *rawData,
                                       OI_OBEX_APP_PARAM_LIST *params)
{
    OI_BYTE_STREAM bs;
    OI_STATUS status;
    OI_UINT i;

    ByteStream_Init(bs, rawData->data, rawData->len);
    ByteStream_Open(bs, BYTESTREAM_READ);

    params->count = 0;
    params->list = NULL;

    while (ByteStream_NumReadBytesAvail(bs) > 0) {
        status = ScanAppParamsEntry(&bs);
        if (!OI_SUCCESS(status)) {
            params->count = 0;
            return OI_OBEX_BAD_PACKET;
        }
        params->count++;
    }
    ByteStream_SetPos(bs, 0);

    if (params->count > 0) {
        params->list = OI_Malloc(sizeof(OI_OBEX_APP_PARAM) * params->count);
        if (params->list == NULL) {
            return OI_STATUS_OUT_OF_MEMORY;
        }
        /*
         * Now get the param values into the header list. We checked that
         * the values were all valid during the scan pass.
         */
        for (i = 0; i < params->count; ++i) {
            ParseAppParamsEntry(&bs, params->list + i);
        }
    }
    ByteStream_Close(bs);

    return OI_OK;
}

OI_STATUS OI_OBEXCOMMON_BuildAppParamsHeader(OI_OBEX_BYTESEQ *data,
                                             const OI_OBEX_APP_PARAM_LIST *params)
{
    OI_UINT i;
    OI_BYTE_STREAM bs;

    data->len = 0;
    for (i = 0; i < params->count; i++) {
        data->len += sizeof(params->list[i].tag) + sizeof(params->list[i].len) + params->list[i].len;
    }

    data->data = OI_Malloc(data->len);
    if (data->data == NULL) {
        return OI_STATUS_OUT_OF_MEMORY;
    }

    ByteStream_Init(bs, data->data, data->len);
    ByteStream_Open(bs, BYTESTREAM_WRITE);

    for (i = 0; i < params->count; i++) {
        ByteStream_PutUINT8(bs, params->list[i].tag);
        ByteStream_PutUINT8(bs, params->list[i].len);
        ByteStream_PutBytes(bs, params->list[i].data, params->list[i].len);
    }

    OI_ASSERT(ByteStream_NumWriteBytesAllowed(bs) == 0);

    return OI_OK;
}
