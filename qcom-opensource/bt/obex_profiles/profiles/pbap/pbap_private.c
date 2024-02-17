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

  Phonebook Access Profile internals
  */

#define __OI_MODULE__ OI_MODULE_PBAP_SRV

#include "oi_bytestream.h"
#include "oi_debug.h"
#include "oi_stddefs.h"
#include "oi_obexspec.h"
#include "oi_pbap_consts.h"
#include "oi_unicode.h"
#include "oi_utils.h"


const OI_CHAR16 OI_PBAP_usim1[] = { 'S', 'I', 'M', '1', 0 };
const OI_CHAR16 OI_PBAP_utelecom[] = { 't', 'e', 'l', 'e', 'c', 'o', 'm', 0 };
const OI_CHAR16 OI_PBAP_vcf[] = { '.', 'v', 'c', 'f', 0 };

static const OI_CHAR16 pb[] = { 'p','b', 0 };
static const OI_CHAR16 ich[] = { 'i', 'c', 'h', 0 };
static const OI_CHAR16 och[] = { 'o', 'c', 'h', 0 };
static const OI_CHAR16 mch[] = { 'm', 'c', 'h', 0 };
static const OI_CHAR16 cch[] = { 'c', 'c', 'h', 0 };
const OI_CHAR16 * OI_PBAP_upbdirs[] = {
    pb,
    ich,
    och,
    mch,
    cch
};
const OI_UINT16 OI_PBAP_upbdirsizes[] = {
    OI_ARRAYSIZE(pb),
    OI_ARRAYSIZE(ich),
    OI_ARRAYSIZE(och),
    OI_ARRAYSIZE(mch),
    OI_ARRAYSIZE(cch)
};


/***********************************************************************
 *
 * Debug Print code for reporting application paramters and OBEX headers
 *
 ***********************************************************************/

#ifdef OI_DEBUG

/**
 * DumpAppParams()
 *
 * Helper debug function for nicely displaying PBAP APPLICATION PARMATERs.
 */
static void DumpAppParams(OI_BYTE *buf, OI_INT bufLen)
{
    OI_BYTE_STREAM bs;
    OI_UINT8 tagId = 0;
    OI_UINT8 tagLen = 0;

    ByteStream_Init(bs, buf, (OI_UINT16)bufLen);
    ByteStream_Open(bs, BYTESTREAM_READ);

    OI_DBG_PRINT2(("PBAP OBEX header APPLICATION PARAMETERS: %@\n",
                      buf, bufLen));

    while (bufLen > 0) {
        ByteStream_GetUINT8_Checked(bs, tagId);
        ByteStream_GetUINT8_Checked(bs, tagLen);
        bufLen -= (2 + tagLen);

        if (bufLen >= 0) {
            switch (tagId) {
            case OI_PBAP_TAG_ID_ORDER:
            {
                OI_UINT8 order = 0;
                const OI_CHAR *orderStr[] = { "Indexed",
                                              "Alphabetical",
                                              "Phonetical",
                                              "Invalid" };
                DEBUG_ONLY((void) orderStr[0];)  /* Suppresses GCC warning for specialized debug builds. */

                ByteStream_GetUINT8_Checked(bs, order);
                OI_DBG_PRINT2(("    PBAP tag Order: %d (%s)\n",
                                  order, orderStr[OI_MIN(order, OI_ARRAYSIZE(orderStr) - 1)]));
                break;
            }

            case OI_PBAP_TAG_ID_SEARCH_VALUE:
                OI_DBG_PRINT2(("    PBAP tag Search Value = \"%s\"\n",
                                  ByteStream_GetCurrentBytePointer(bs), tagLen));
                ByteStream_Skip_Checked(bs, tagLen);
                break;

            case OI_PBAP_TAG_ID_SEARCH_ATTRIBUTE:
            {
                OI_UINT8 searchAttr = 0;
                const OI_CHAR *searchAttrStr[] = { "Name",
                                                   "Number",
                                                   "Sound",
                                                   "Invalid" };
                DEBUG_ONLY((void) searchAttrStr[0];)  /* Suppresses GCC warning for specialized debug builds. */

                ByteStream_GetUINT8_Checked(bs, searchAttr);
                OI_DBG_PRINT2(("    PBAP tag Search Attribute: %d (%s)\n", searchAttr,
                                  searchAttrStr[OI_MIN(searchAttr,
                                                       OI_ARRAYSIZE(searchAttrStr) - 1)]));
                break;
            }

            case OI_PBAP_TAG_ID_MAX_LIST_COUNT:
            {
                OI_UINT16 maxListCount = 0;

                ByteStream_GetUINT16_Checked(bs, maxListCount, OI_BIG_ENDIAN_BYTE_ORDER);
                OI_DBG_PRINT2(("    PBAP tag Max List Count: %d\n", maxListCount));
                break;
            }

            case OI_PBAP_TAG_ID_LIST_START_OFFSET:
            {
                OI_UINT16 listStartOffset = 0;

                ByteStream_GetUINT16_Checked(bs, listStartOffset, OI_BIG_ENDIAN_BYTE_ORDER);
                OI_DBG_PRINT2(("    PBAP tag List Start Offset: %d\n", listStartOffset));
                break;
            }

            case OI_PBAP_TAG_ID_FILTER:
            {
                OI_UINT64 filter = {0,0};

                ByteStream_GetUINT32_Checked(bs, filter.I1, OI_BIG_ENDIAN_BYTE_ORDER);
                ByteStream_GetUINT32_Checked(bs, filter.I2, OI_BIG_ENDIAN_BYTE_ORDER);
                OI_DBG_PRINT2(("    PBAP tag Filter: %08x-%08x\n", filter.I1, filter.I2));
                break;
            }

            case OI_PBAP_TAG_ID_FORMAT:
            {
                OI_UINT8 format = 0;
                const OI_CHAR *formatStr[] = { "2.1",
                                               "3.0",
                                               "Invalid" };
                DEBUG_ONLY((void) formatStr[0];)  /* Suppresses GCC warning for specialized debug builds. */

                ByteStream_GetUINT8_Checked(bs, format);
                OI_DBG_PRINT2(("    PBAP tag Format: %d (vCard version %s)\n",
                                  format, formatStr[OI_MIN(format, OI_ARRAYSIZE(formatStr) - 1)]));
                break;
            }

            case OI_PBAP_TAG_ID_PHONEBOOK_SIZE:
            {
                OI_UINT16 size = 0;

                ByteStream_GetUINT16_Checked(bs, size, OI_BIG_ENDIAN_BYTE_ORDER);
                OI_DBG_PRINT2(("    PBAP tag Phonebook Size: %d\n", size));
                break;
            }

            case OI_PBAP_TAG_ID_NEW_MISSED_CALLS:
            {
                OI_UINT8 newMissedCalls = 0;

                ByteStream_GetUINT8_Checked(bs, newMissedCalls);
                OI_DBG_PRINT2(("    PBAP tag New Missed Calls: %d\n", newMissedCalls));
                break;
            }

            default:
                OI_DBG_PRINT2(("    PBAP tag UNKNOWN: %02x (%d bytes in size)\n", tagId, tagLen));
                ByteStream_Skip_Checked(bs, tagLen);
            }
        }
    }

    if ((bufLen < 0) || ByteStream_Error(bs)) {
        OI_LOG_ERROR(("Invalid application parameter length\n"));
    }

    ByteStream_Close(bs);
}


/**
 * OI_PBAP_DumpObexHeaders()
 *
 * Helper debug function for nicely displaying OBEX headers relevant to PBAP.
 */
void OI_PBAP_DumpObexHeaders(const OI_OBEX_HEADER_LIST *hdrList)
{
    OI_INT i;

    if (!hdrList || hdrList->count == 0) {
        OI_DBG_PRINT2(("No PBAP OBEX headers.\n"));
        return;
    }

    for (i = 0; i < hdrList->count; i++) {
        OI_OBEX_HEADER *hdr = &hdrList->list[i];

        switch (hdr->id) {
        case OI_OBEX_HDR_NAME:
            OI_DBG_PRINT2(("PBAP OBEX header NAME: \"%s\"\n",
                              hdr->val.name.str, hdr->val.name.len));
            break;

        case OI_OBEX_HDR_TYPE:
            OI_DBG_PRINT2(("PBAP OBEX header TYPE: \"%s\"\n",
                              hdr->val.type.data, hdr->val.type.len));
            break;

        case OI_OBEX_HDR_APPLICATION_PARAMS:


            if((NULL != hdr->val.applicationParams.data) && (0 != hdr->val.applicationParams.len)) {
                DumpAppParams(hdr->val.applicationParams.data, hdr->val.applicationParams.len);
            } else {
                OI_DBG_PRINT2(("PBAP OBEX header APPLICATION_PARAMS empty\n"));
            }
            break;

        case OI_OBEX_HDR_BODY:
            OI_DBG_PRINT2(("PBAP OBEX header BODY: %@", hdr->val.body.data,
                              OI_MIN(hdr->val.body.len, 16)));
            break;

        case OI_OBEX_HDR_END_OF_BODY:
            OI_DBG_PRINT2(("PBAP OBEX header END OF BODY: %@", hdr->val.body.data,
                              OI_MIN(hdr->val.body.len, 16)));
            break;

        default:
            OI_DBG_PRINT2(("PBAP OBEX header UNRECOGNIZED: %x (ignored)\n", hdr->id));
        }
    }
}
#endif
