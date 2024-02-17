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
 */

#define __OI_MODULE__ OI_MODULE_SUPPORT

#include "oi_argcheck.h"
#include "oi_stddefs.h"
#include "oi_utils.h"
#include "oi_unicode.h"
#include "oi_assert.h"
#include "oi_unicodestream.h"
#include "oi_std_utils.h"

#include "../unicode/ConvertUTF.h"


/* This table translates the result status from the conversion code developed
 * by Unicode, Inc., to standard OI_STATUS codes.
 */
static const OI_STATUS OI_CvtUnicodeResult[] = {
    OI_OK,
    OI_UNICODE_SOURCE_EXHAUSTED,
    OI_UNICODE_DESTINATION_EXHAUSTED,
    OI_UNICODE_INVALID_SOURCE
};

OI_STATUS OI_Utf8ToUtf16(const OI_UTF8 *src, OI_INT srcLen,
                         OI_UTF16 *dst, OI_INT dstLen)
{
    OI_STATUS status;
    OI_UTF16 *dstend = dst + dstLen;

    OI_DBGPRINT2(("OI_Utf8ToUtf16 (<*src = %x>, srcLen = %d, <*dst = %x>, dstLen = %d)\n",
                  src, srcLen, dst, dstLen));

    OI_ARGCHECK(src != NULL);
    OI_ARGCHECK(srcLen > 0);
    OI_ARGCHECK(dst != NULL);
    OI_ARGCHECK(dstLen > 0);

    OI_DBGPRINT2(("\tConverting \"%s\" from UTF8 to UTF16...\n", src, srcLen));

    status = OI_CvtUnicodeResult[ConvertUTF8toUTF16(&src, src + srcLen,
                                                    &dst, dstend,
                                                    strictConversion)];
    if(OI_SUCCESS(status)) {
        if (dst < dstend) {
            *dst = 0;  /* Ensure last character is nul terminator. */
        } else if (dst == dstend && (dst[-1] != 0)) {
            OI_DBGPRINT2(("Failed to add final nul terminator character.\n"));
            status = OI_UNICODE_DESTINATION_EXHAUSTED;
        }
    } else {
        OI_DBGPRINT2(("UTF conversion failed: %d\n", status));
    }
    return status;
}

OI_STATUS OI_Utf16ToUtf8(const OI_UTF16 *src, OI_INT srcLen,
                         OI_UTF8 *dst, OI_INT dstLen)
{
    OI_STATUS status;
    OI_UTF8 *dstend = dst + dstLen;

    OI_DBGPRINT2(("OI_Utf8ToUtf16 (<*src = %x>, srcLen = %d, <*dst = %x>, dstLen = %d)\n",
                  src, srcLen, dst, dstLen));

    OI_ARGCHECK(src != NULL);
    OI_ARGCHECK(srcLen > 0);
    OI_ARGCHECK(dst != NULL);
    OI_ARGCHECK(dstLen > 0);

    OI_DBGPRINT2(("\tConverting \"%s\" from UTF16 to UTF8...\n", src, srcLen));

    status = OI_CvtUnicodeResult[ConvertUTF16toUTF8(&src, src + srcLen,
                                                    &dst, dstend,
                                                    strictConversion)];
    if(OI_SUCCESS(status)) {
        if (dst < dstend) {
            *dst = 0;  /* Ensure last character is nul terminator. */
        } else  if (dst == dstend && (dst[-1] != 0)) {
            OI_DBGPRINT2(("Failed to add final nul terminator character.\n"));
            status = OI_UNICODE_DESTINATION_EXHAUSTED;
        }
    } else {
        OI_DBGPRINT2(("UTF conversion failed: %d\n", status));
    }
    return status;
}


/**
 * OI_StrLenUtf16()
 *
 * This function returns the number of 16-bit words used by the string, not
 * the number of Unicode characters as some Unicode characters may occupy
 * multiple 16-bit words.  In other words, it operates the same way that the
 * standard C function strlen() would on UTF-8 strings (which also have
 * multibyte characters).
 */
OI_UINT OI_StrLenUtf16(const OI_UTF16 *str)
{
    OI_UINT len = 0;

    OI_ASSERT(str != NULL);

    while (str[len] != 0) {
        ++len;
    }
    return len;
}


/**
 * OI_StrcmpUtf16()
 *
 * This function compare 2 UTF-16 strings in the same manner that the standard
 * C function strcmp() would compare UTF-8 strings.  That is, it does an exact
 * 16-bit word for 16-bit word compare rather than check for possible
 * eqivalent character encodings for those characters that may have different,
 * but equivalent encodings.
 */
OI_INT OI_StrcmpUtf16(const OI_UTF16 *str1,
                      const OI_UTF16 *str2)
{
    OI_ASSERT(str1 != NULL);
    OI_ASSERT(str2 != NULL);

    for (;;) {
        if (*str1 < *str2) return -1;
        if (*str1 > *str2) return 1;
        if (*str1 == 0) return 0;
        str1++;
        str2++;
    }
}


OI_INT OI_StrncmpUtf16(const OI_UTF16 *str1,
                       const OI_UTF16 *str2,
                       OI_UINT16 len)
{
    OI_ASSERT(str1 != NULL);
    OI_ASSERT(str2 != NULL);

    while (len--) {
        if (*str1 < *str2) return -1;
        if (*str1 > *str2) return 1;
        if (*str1 == 0) return 0;
        str1++;
        str2++;
    }
    return 0;
}







OI_INT OI_PStrcmp(const OI_PSTR *s1,
                  OI_UNICODE_ENCODING e1,
                  const OI_CHAR *s2)
{
    OI_UNICODE_STREAM bs;
    OI_UINT32 ch = 0;

    OI_ASSERT(s1 != NULL);
    OI_ASSERT(s2 != NULL);

    if (s1->sz) {
        UnicodeStream_Init(bs, s1->p, s1->sz, e1);
        ByteStream_Open(bs, BYTESTREAM_READ);
        for (;;) {
            if (!ByteStream_NumReadBytesAvail(bs)) {
                if (*s2 == '\0') {
                    return 0;
                } else {
                    return -1;
                }
            }
            UnicodeStream_GetChar(bs, ch);
            if (ch < (OI_UINT32)*s2) return -1;
            if (ch > (OI_UINT32)*s2) return 1;
            s2++;
        }
    } else {
        if (*s2) {
            return -1;
        } else {
            return 0;
        }
    }
}

OI_INT OI_PStrcmpInsensitive(const OI_PSTR *s1,
                             OI_UNICODE_ENCODING e1,
                             const OI_CHAR *s2)
{
    OI_UNICODE_STREAM bs;
    OI_UINT32 ch = 0;
    OI_UINT32 uc1;
    OI_UINT32 uc2;

    OI_ASSERT(s1 != NULL);
    OI_ASSERT(s2 != NULL);

    if (s1->sz) {
        UnicodeStream_Init(bs, s1->p, s1->sz, e1);
        ByteStream_Open(bs, BYTESTREAM_READ);
        for (;;) {
            if (!ByteStream_NumReadBytesAvail(bs)) {
                if (*s2 == '\0') {
                    return 0;
                } else {
                    return -1;
                }
            }
            UnicodeStream_GetChar(bs, ch);
            uc1 = OI_toupper(ch);
            uc2 = OI_toupper(*s2);
            if (uc1 < uc2) return -1;
            if (uc1 > uc2) return 1;
            s2++;
        }
    } else {
        if (*s2) {
            return -1;
        } else {
            return 0;
        }
    }
}

OI_INT OI_PStrncmp(const OI_PSTR *s1,
                   OI_UNICODE_ENCODING e1,
                   const OI_CHAR *s2,
                   OI_UINT16 len)
{
    OI_UNICODE_STREAM bs;
    OI_UINT32 ch = 0;

    OI_ASSERT(s1 != NULL);
    OI_ASSERT(s2 != NULL);

    if (s1->sz) {
        UnicodeStream_Init(bs, s1->p, s1->sz, e1);
        ByteStream_Open(bs, BYTESTREAM_READ);
        while (len--) {
            if (!ByteStream_NumReadBytesAvail(bs)) {
                if (*s2 == '\0') {
                    return 0;
                } else {
                    return -1;
                }
            }
            UnicodeStream_GetChar(bs, ch);
            if (ch < (OI_UINT32)*s2) return -1;
            if (ch > (OI_UINT32)*s2) return 1;
            s2++;
        }
        return 0;
    } else {
        if (*s2) {
            return -1;
        } else {
            return 0;
        }
    }
}

OI_INT OI_PStrcmp2(const OI_PSTR *s1,
                      OI_UNICODE_ENCODING e1,
                      const OI_PSTR *s2,
                      OI_UNICODE_ENCODING e2)
{
    OI_UNICODE_STREAM bs1;
    OI_UNICODE_STREAM bs2;
    OI_UINT32 ch1 = 0;
    OI_UINT32 ch2 = 0;
    OI_INT endOfString;

    OI_ASSERT(s1 != NULL);
    OI_ASSERT(s2 != NULL);

    if (s1->sz && s2->sz) {

        UnicodeStream_Init(bs1, s1->p, s1->sz, e1);
        ByteStream_Open(bs1, BYTESTREAM_READ);

        UnicodeStream_Init(bs2, s2->p, s2->sz, e2);
        ByteStream_Open(bs2, BYTESTREAM_READ);

        for (;;) {
            endOfString = ByteStream_NumReadBytesAvail(bs1) ? 0 : 1;
            endOfString |= ByteStream_NumReadBytesAvail(bs2) ? 0 : 2;
            switch (endOfString) {
                case 1: return -1;
                case 2: return 1;
                case 3: return 0;
            }
            UnicodeStream_GetChar(bs1, ch1);
            UnicodeStream_GetChar(bs2, ch2);
            if (ch1 < ch2) return -1;
            if (ch1 > ch2) return 1;
        }
    } else if (s2->sz) {
        return -1;
    } else if (s1->sz) {
        return 1;
    } else {
        return 0;
    }
}

OI_INT OI_PStrIndexOf(const OI_PSTR *s1,
                      OI_UNICODE_ENCODING e1,
                      const OI_CHAR *s2,
                      OI_INT pos)
{
    OI_UNICODE_STREAM bs;
    const OI_CHAR *s;
    OI_UINT32 ch = 0;

    OI_ASSERT(s1 != NULL);
    OI_ASSERT(s2 != NULL);

    if (*s2 == 0) {
        return pos;
    }

    UnicodeStream_Init(bs, s1->p, s1->sz, e1);
    ByteStream_Open(bs, BYTESTREAM_READ);
    ByteStream_SetPos(bs, pos);
    while (!ByteStream_Error(bs) && ByteStream_NumReadBytesAvail(bs)) {
        pos = ByteStream_GetPos(bs);

        for (s = s2; (*s != 0) && ByteStream_NumReadBytesAvail(bs); ++s) {
            UnicodeStream_GetChar_Checked(bs, ch);
            if (ch != (OI_UINT32)(*s)) {
                break;
            }
        }
        if (*s == 0) {
            return pos;
        }

        ByteStream_SetPos(bs, pos);
        UnicodeStream_SkipChar_Checked(bs, 1);
    }
    return -1;
}

OI_INT OI_PStrLastIndexOf(const OI_PSTR *s1,
                          OI_UNICODE_ENCODING e1,
                          const OI_CHAR *s2,
                          OI_INT pos)
{
    OI_UNICODE_STREAM bs;
    const OI_CHAR *s;
    OI_UINT32 ch = 0;
    OI_INT result = -1;

    OI_ASSERT(s1 != NULL);
    OI_ASSERT(s2 != NULL);

    if (*s2 == 0) {
        return 0;
    }

    UnicodeStream_Init(bs, s1->p, s1->sz, e1);
    ByteStream_Open(bs, BYTESTREAM_READ);
    ByteStream_SetPos(bs, pos);
    while (!ByteStream_Error(bs) && ByteStream_NumReadBytesAvail(bs)) {
        pos = ByteStream_GetPos(bs);

        for (s = s2; (*s != 0) && ByteStream_NumReadBytesAvail(bs); ++s) {
            UnicodeStream_GetChar_Checked(bs, ch);
            if (ch != (OI_UINT32)(*s)) {
                break;
            }
        }
        if (*s == 0) {
            result = pos;
        }

        ByteStream_SetPos(bs, pos);
        UnicodeStream_SkipChar_Checked(bs, 1);
    }
    return result;
}

OI_INT OI_PStrLen(const OI_PSTR *s1,
                  OI_UNICODE_ENCODING e1)
{
    OI_INT len = 0;
    OI_UINT16 i;

    /*
     * Count each multi-byte sequence only once.
     */
    switch (e1) {
        case OI_UNICODE_UTF8:
            for (i = 0; i < s1->sz; ++i) {
                if ((s1->p[i] & 0xc0) != 0x80) {
                    ++len;
                }
            }
            break;
        case OI_UNICODE_UTF16_LE:
            for (i = 0; i < s1->sz; i += 2) {
                if ((s1->p[i + 1] & 0xfc) != 0xdc) {
                    ++len;
                }
            }
            break;
        case OI_UNICODE_UTF16_BE:
            for (i = 0; i < s1->sz; i += 2) {
                if ((s1->p[i] & 0xfc) != 0xdc) {
                    ++len;
                }
            }
            break;
        default:
            break;
    }
    return len;
}

void OI_PStrToUStr(OI_CHAR16 *s1,
                   const OI_PSTR *s2,
                   OI_UNICODE_ENCODING e2)
{
    OI_UNICODE_STREAM bs;
    OI_UINT32 ch = 0;

    UnicodeStream_Init(bs, s2->p, s2->sz, e2);
    ByteStream_Open(bs, BYTESTREAM_READ);
    while (ByteStream_NumReadBytesAvail(bs)) {
        UnicodeStream_GetChar(bs, ch);
        if (ch > 0xffff) {
            OI_LOG_ERROR(("Truncating extended unicode character!"));
        }
        *s1++ = (OI_CHAR16)ch;
    }
    *s1 = 0;
}




