#ifndef _OI_UNICODESTREAM_H
#define _OI_UNICODESTREAM_H

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
 * This file provides unicode stream read and write macros.  Unicode streams consist
 * of an array of encoded bytes, where the encoding may be UTF-8, UTF-16, etc.  A
 * unicode stream is a superset of a bytestream, and all the bytestream macros may be
 * used on unicode streams as well.
 */

#include "oi_stddefs.h"
#include "oi_memmgr.h"
#include "oi_cpu_dep.h"
#include "oi_endian.h"
#include "oi_assert.h"
#include "oi_bytestream.h"
#include "oi_unicode.h"

/** \addtogroup Misc_Internal */
/**@{*/

#ifdef __cplusplus
extern "C" {
#endif


/**
 * This is the structure definition for an unicode encoded text stream.  This structure
 * is identical to a byte stream, with the addition of an encoding member added.  This
 * means that all bytestream macros listed in this file can also be used on the text
 * stream.
 *
 * The members of the unicode stream are *PRIVATE* to this file, and must not be
 * used, except as accessed through the Macros in this header file.
 */
typedef struct {
    OI_UNICODE_ENCODING __encoding; // The encoding of the bytes
    OI_BYTE *           __bsdata;     // Pointer to an array of bytes of size trueSize
    OI_UINT16           __trueSize; // Allocated size of data
    OI_UINT16           __size;     // Current size of the data
    OI_UINT16           __pos;      // Current read/write pointer
    OI_UINT8            __RWFlag;   // Are we reading or writing to the byte stream
    OI_UINT8            __error;    // Error reading or writing to byte stream
} OI_UNICODE_STREAM;

/**
 * Initializes a unicode stream.
 * Must be called before opening the stream with ByteStream_Open.
 *
 * @param bs The unicode stream to initialize
 * @param buf The data buffer to use with the stream
 * @param size The size of the data buffer
 * @param encoding The encoding of the data buffer
 *
 * void UnicodeStream_Init(OI_UNICODE_STREAM    bs,
 *                         OI_BYTE *            buf,
 *                         OI_UINT16            size,
 *                         OI_UNICODE_ENDOCDING encoding);
 *
 */

#define UnicodeStream_Init(bs, buf, size, encoding) \
{                                                   \
    OI_ASSERT(buf);                                 \
    OI_ASSERT((size) > 0);                          \
    (bs).__error = 0;                               \
    (bs).__bsdata = (buf);                            \
    (bs).__trueSize = (size);                       \
    (bs).__size = (size);                           \
    (bs).__encoding = (encoding);                   \
}

/**
 * Get the encoding of the specified stream.
 *
 * @param bs  The unicode stream to query
 * @return  The encoding of the stream
 *
 * OI_UNICODE_ENCODING UnicodeStream_GetEncoding(OI_UNICODE_STREAM bs);
 */

#define UnicodeStream_GetEncoding(bs) (bs).__encoding

/**
 * Skips the next n characters in the unicode stream.
 *
 * @param bs The stream in which to skip characters
 * @param count The number of characters to skip
 *
 * void UnicodeStream_SkipChar(OI_UNICODE_STREAM  bs,
 *                             OI_UINT16       count);
 */

#define UnicodeStream_SkipChar(bs, count)       \
{                                               \
    OI_UINT32 ch;                               \
    OI_UINT16 n;                                \
    for (n = (count); n; --n) {                 \
        UnicodeStream_GetChar(bs, ch);          \
    }                                           \
}

/**
 * Skips the next n characters in the stream, checking for validity
 * of the operation.
 *
 * @param bs The stream in which to skip characters
 * @param count The number of characters to skip
 *
 * void UnicodeStream_Skip_Checked(OI_UNICODE_STREAM  bs,
 *                                 OI_UINT16       count);
 */
#define UnicodeStream_SkipChar_Checked(bs, count)   \
{                                                   \
    OI_UINT32 ch;                                   \
    OI_UINT16 n;                                    \
    for (n = (count); n; --n) {                     \
        UnicodeStream_GetChar_Checked(bs, ch);      \
    }                                               \
}

/*******************************************************************
 * Unicode stream read functions
 ******************************************************************/

/**
 * Reads a unicode character encoded as UTF-8 from a byte stream.
 *
 * @param bs The byte stream from which to read the UTF-8 encoded character
 * @param ch [out] The character obtained
 *
 * void UnicodeStream_GetUTF8(OI_UNICODE_STREAM   bs,
 *                            OI_UINT32        ch);
 */

#define UnicodeStream_GetUTF8(bs, ch)                                       \
{                                                                           \
    OI_INT count;                                                           \
    OI_BYTE mask;                                                           \
    OI_ASSERT((bs).__RWFlag == BYTESTREAM_READ);                            \
    OI_ASSERT(((bs).__pos + 1) <= (bs).__size);                             \
    (ch) = (OI_UINT32) (bs).__bsdata[(bs).__pos++];                           \
    if ((ch) >= 0xc0) {                                                     \
        for (count = 5, mask = 0xfc; (ch) <= mask; --count, mask <<= 1) {   \
        }                                                                   \
        OI_ASSERT(((bs).__pos + count) <= (bs).__size);                     \
        (ch) &= ~mask;                                                      \
        while (count--) {                                                   \
            (ch) = ((ch) << 6) | ((bs).__bsdata[(bs).__pos++] & 0x3f);        \
        }                                                                   \
    }                                                                       \
}

/**
 * Reads a unicode character encoded as UTF-8 from a byte stream, checking it
 * for validity.
 *
 * @param bs The byte stream from which to read the UTF-8 encoded character
 * @param ch [out] The character obtained
 *
 * void UnicodeStream_GetUTF8_Checked(OI_UNICODE_STREAM   bs,
 *                                    OI_UINT32        ch);
 */

#define UnicodeStream_GetUTF8_Checked(bs, ch)                                   \
{                                                                               \
    OI_INT count;                                                               \
    OI_BYTE mask;                                                               \
    OI_ASSERT((bs).__RWFlag == BYTESTREAM_READ);                                \
    if (((bs).__pos + 1) <= (bs).__size) {                                      \
        (ch) = (OI_UINT32) (bs).__bsdata[(bs).__pos++];                           \
        if ((ch) >= 0xc0) {                                                     \
            for (count = 5, mask = 0xfc; (ch) <= mask; --count, mask <<= 1) {   \
            }                                                                   \
            if (((bs).__pos + count) <= (bs).__size) {                          \
                (ch) &= ~mask;                                                  \
                while (count--) {                                               \
                    (ch) = ((ch) << 6) | ((bs).__bsdata[(bs).__pos++] & 0x3f);    \
                }                                                               \
            } else {                                                            \
                (bs).__error = 1;                                               \
            }                                                                   \
        }                                                                       \
    } else {                                                                    \
        (bs).__error = 1;                                                       \
    }                                                                           \
}

/*
 * Reads a unicode character encoded as UTF-16 from a byte stream.
 *
 * @param bs The byte stream
 * @param ch [out] The character obtained
 * @param bo Byte order (endianness) of the stream
 *
 * void UnicodeStream_GetUTF16(OI_UNICODE_STREAM   bs,
 *                             OI_UINT32        ch,
 *                             OI_UINT8         bo);
 */
#define UnicodeStream_GetUTF16(bs, ch, bo)                                                      \
{                                                                                               \
    OI_ASSERT((bs).__RWFlag == BYTESTREAM_READ);                                                \
    OI_ASSERT( ((bo) == OI_BIG_ENDIAN_BYTE_ORDER) || ((bo) == OI_LITTLE_ENDIAN_BYTE_ORDER) );   \
    OI_ASSERT(((bs).__pos + 2) <= (bs).__size);                                                 \
    if ((bo) == OI_BIG_ENDIAN_BYTE_ORDER) {                                                     \
        (ch) = GetUINT16_BigEndian((bs).__bsdata + (bs).__pos);                                   \
    } else {                                                                                    \
        (ch) = GetUINT16_LittleEndian((bs).__bsdata + (bs).__pos);                                \
    }                                                                                           \
    (bs).__pos += 2;                                                                            \
    if ((0xd800 <= (ch)) && ((ch) <= 0xdbff)) {                                                 \
        (ch) = ((ch) & 0x3ff) << 10;                                                            \
        OI_ASSERT(((bs).__pos + 2) <= (bs).__size);                                             \
        if ((bo) == OI_BIG_ENDIAN_BYTE_ORDER) {                                                 \
            (ch) |= (GetUINT16_BigEndian((bs).__bsdata + (bs).__pos) & 0x3ff);                    \
        } else {                                                                                \
            (ch) |= (GetUINT16_LittleEndian((bs).__bsdata + (bs).__pos) & 0x3ff);                 \
        }                                                                                       \
        (ch) += 0x10000;                                                                        \
        (bs).__pos += 2;                                                                        \
    }                                                                                           \
}

/*
 * Reads a unicode character encoded as UTF-16 from a byte stream, checking it
 * for validity.
 *
 * @param bs The byte stream
 * @param ch [out] The character obtained
 * @param bo Byte order (endianness) of the stream
 *
 * void UnicodeStream_GetUTF16_Checked(OI_UNICODE_STREAM   bs,
 *                                     OI_UINT32        ch,
 *                                     OI_UINT8         bo);
 */
#define UnicodeStream_GetUTF16_Checked(bs, ch, bo)                                              \
{                                                                                               \
    OI_ASSERT((bs).__RWFlag == BYTESTREAM_READ);                                                \
    OI_ASSERT( ((bo) == OI_BIG_ENDIAN_BYTE_ORDER) || ((bo) == OI_LITTLE_ENDIAN_BYTE_ORDER) );   \
    if (((bs).__pos + 2) <= (bs).__size) {                                                      \
        if ((bo) == OI_BIG_ENDIAN_BYTE_ORDER) {                                                 \
            (ch) = GetUINT16_BigEndian((bs).__bsdata + (bs).__pos);                               \
        } else {                                                                                \
            (ch) = GetUINT16_LittleEndian((bs).__bsdata + (bs).__pos);                            \
        }                                                                                       \
        (bs).__pos += 2;                                                                        \
        if ((0xd800 <= (ch)) && ((ch) <= 0xdbff)) {                                             \
            (ch) = ((ch) & 0x3ff) << 10;                                                        \
            if (((bs).__pos + 2) <= (bs).__size) {                                              \
                if ((bo) == OI_BIG_ENDIAN_BYTE_ORDER) {                                         \
                    (ch) |= (GetUINT16_BigEndian((bs).__bsdata + (bs).__pos) & 0x3ff);            \
                } else {                                                                        \
                    (ch) |= (GetUINT16_LittleEndian((bs).__bsdata + (bs).__pos) & 0x3ff);         \
                }                                                                               \
                (ch) += 0x10000;                                                                \
                (bs).__pos += 2;                                                                \
            } else {                                                                            \
                (bs).__error = 1;                                                               \
            }                                                                                   \
        }                                                                                       \
    } else {                                                                                    \
        (bs).__error = 1;                                                                       \
    }                                                                                           \
}

/*
 * Reads a unicode character from a stream.
 *
 * @param bs The stream
 * @param ch [out] The character obtained
 *
 * void UnicodeStream_GetChar(OI_UNICODE_STREAM   bs,
 *                            OI_UINT32        ch)
 */
#define UnicodeStream_GetChar(bs, ch)                                           \
{                                                                               \
    OI_ASSERT((bs).__encoding != OI_UNICODE_UNKNOWN);                           \
    switch ((bs).__encoding) {                                                  \
        case OI_UNICODE_UTF8:                                                   \
            UnicodeStream_GetUTF8((bs), (ch));                                  \
            break;                                                              \
        case OI_UNICODE_UTF16_BE:                                               \
            UnicodeStream_GetUTF16((bs), (ch), OI_BIG_ENDIAN_BYTE_ORDER);       \
            break;                                                              \
        case OI_UNICODE_UTF16_LE:                                               \
            UnicodeStream_GetUTF16((bs), (ch), OI_LITTLE_ENDIAN_BYTE_ORDER);    \
            break;                                                              \
        default:                                                                \
            break;                                                              \
    }                                                                           \
}

/*
 * Reads a unicode character from a stream, checking it for validity.
 *
 * @param bs The stream
 * @param ch [out] The character obtained
 *
 * void UnicodeStream_GetChar_Checked(OI_UNICODE_STREAM   bs,
 *                                    OI_UINT32        ch)
 */
#define UnicodeStream_GetChar_Checked(bs, ch)                                           \
{                                                                                       \
    switch ((bs).__encoding) {                                                          \
        case OI_UNICODE_UTF8:                                                           \
            UnicodeStream_GetUTF8_Checked((bs), (ch));                                  \
            break;                                                                      \
        case OI_UNICODE_UTF16_BE:                                                       \
            UnicodeStream_GetUTF16_Checked((bs), (ch), OI_BIG_ENDIAN_BYTE_ORDER);       \
            break;                                                                      \
        case OI_UNICODE_UTF16_LE:                                                       \
            UnicodeStream_GetUTF16_Checked((bs), (ch), OI_LITTLE_ENDIAN_BYTE_ORDER);    \
            break;                                                                      \
        default:                                                                        \
            (bs).__error = 1;                                                           \
            break;                                                                      \
    }                                                                                   \
}

/*
 * Reads a unicode character from a stream, without updating the stream
 * position.
 *
 * @param bs The stream
 * @param ch [out] The character obtained
 *
 * void UnicodeStream_PeekChar(OI_UNICODE_STREAM   bs,
 *                             OI_UINT32        ch)
 */
#define UnicodeStream_PeekChar(bs, ch)          \
{                                               \
    OI_UINT16 pos = (bs).__pos;                 \
    UnicodeStream_GetChar((bs), (ch))           \
    (bs).__pos = pos;                           \
}

/*
 * Reads a unicode character from a stream, checking it for validity.  This
 * does not update the stream position.
 *
 * @param bs The stream
 * @param ch [out] The character obtained
 *
 * void UnicodeStream_PeekChar_Checked(OI_UNICODE_STREAM   bs,
 *                                     OI_UINT32        ch)
 */
#define UnicodeStream_PeekChar_Checked(bs, ch)  \
{                                               \
    OI_UINT16 pos = (bs).__pos;                 \
    UnicodeStream_GetChar_Checked((bs), (ch))   \
    (bs).__pos = pos;                           \
}


/*****************************************************************************/
#ifdef __cplusplus
}
#endif

/**@}*/

#endif /* _OI_UNICODESTREAM_H */

