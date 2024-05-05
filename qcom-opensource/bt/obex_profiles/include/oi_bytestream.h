#ifndef _OI_BYTESTREAM_H
#define _OI_BYTESTREAM_H
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
 * This file provides byte stream read and write macros.
 *
 *   Provides macros for reading and writing data to a
 *   byte buffer. A byte stream can be opened for either reading or writing.
 *
 *   The size and the position for reading and writing to the byte
 *   stream are maintained in an OI_BYTE_STREAM struct. This structure holds a
 *   pointer to a byte buffer and the field mazSize is the allocated size of this
 *   byte array.  The size field keeps track of how much data is currently in the
 *   byte array. The pos field is used the by the Put and Get functions to keep
 *   track of the where to read or write the next value.
 *
 * @code
 *
 *    -----------------------------------------------------------------------
 *    |                                                 XXXXXXXXXXXXXXXXXXX |
 *    -----------------------------------------------------------------------
 *                 +                                   +                    +
 *                 |                                   |                    |
 *                POS                                 SIZE                MAXSIZE
 *
 *         current read/write position            data size            buffer size
 *
 *
 * @endcode
 *
 *   Position is adjusted as data is read or written from the byte stream and can be
 *   explicitly set using ByteStream_SetPos.
 *
 *   Size is set when the byte stream is initialized and can be explicitly set using
 *   ByteStream_SetSize to truncate the data (for reading) or set a limit on the number
 *   of bytes that can be written.
 *
 */

#include "oi_stddefs.h"
#include "oi_memmgr.h"
#include "oi_cpu_dep.h"
#include "oi_endian.h"
#include "oi_assert.h"

/** \addtogroup Misc_Internal */
/**@{*/

#ifdef __cplusplus
extern "C" {
#endif


/** Data types to be used with ByteStream_Print and ByteStream_Parse. */
enum {
    TYPE_INT8,
    TYPE_UINT8,
    TYPE_INT16,
    TYPE_UINT16,
    TYPE_INT24,
    TYPE_UINT24,
    TYPE_INT32,
    TYPE_UINT32,
    TYPE_INT64,
    TYPE_UINT64,
    TYPE_DATA,
    TYPE_BD_ADDR,
    TYPE_UUID16,
    TYPE_UUID32,
    TYPE_UUID128,
    TYPE_OBEX_UNICODE,
    TYPE_IPV6_ADDR
};

/**
 * When writing integer data to a byte stream you must specify if the data is to
 * be written in little-endian (least significant bytes first) or big-endian (most significant bytes first)
 * byte order. The Bluetooth specification indicates which order is required for
 * each profile or protocol.
 *
 * The following constants are defined in cpu_def.h:
 * @code
 * OI_BIG_ENDIAN_BYTE_ORDER
 * OI_LITTLE_ENDIAN_BYTE_ORDER
 * OI_CPU_BYTE_ORDER
 * @endcode
 */

#define NETWORK_BYTE_ORDER  OI_BIG_ENDIAN_BYTE_ORDER

/** Indicates the byte stream being opened is to be read from. */
#define BYTESTREAM_READ          1

/** Indicates the byte stream being opened is to be written to. */
#define BYTESTREAM_WRITE         2

/** Indicates the byte stream being opened is locked. */
#define BYTESTREAM_LOCKED        15

/**
 * This is the structure definition for a byte stream. The actual byte array must be
 * allocated separately from the byte stream struct.
 *
 * The members of the bytestream are *PRIVATE* to this file, and must not be
 * used, except as accessed through the Macros in this header file.
 */
typedef struct {
    OI_BYTE *   __bsdata;      // Pointer to an array of bytes of size trueSize
    OI_UINT16   __trueSize;  // Allocated size of data
    OI_UINT16   __size;      // Current size of the data
    OI_UINT16   __pos;       // Current read/write pointer
    OI_UINT8    __RWFlag;    // Are we reading or writing to the byte stream
    OI_UINT8    __error;     // Error reading or writing to byte stream
} OI_BYTE_STREAM;


/**
 * Allocates a bytestream and the data buffer inside.
 *
 * @param size  the size in bytes of the data buffer to allocate
 */
OI_BYTE_STREAM *ByteStream_Alloc(OI_UINT16 size);


/**
 * Frees an allocated bytestream including the internal buffer.
 *
 * @param bs  The bytestream to free.
 */
void ByteStream_Free(OI_BYTE_STREAM *bs);


#ifdef OI_DEBUG /* Only used by internal test programs */
/**
 * Prints to a bytestream using the specified format.
 *
 * @param bs         the byte stream to print
 * @param byteorder  the byte order to use when printing
 * @param format     string indicating the format for the byte stream
 * @param formatLen  length of the byte stream format string
 * @param ...        the arguments matching format
 */
OI_STATUS ByteStream_Print(OI_BYTE_STREAM * bs,
                           OI_INT           byteorder,
                           const OI_CHAR    format[],
                           OI_UINT          formatLen,
                           ...);
#endif

/**
 * Parses a bytestream using the specified format.
 *
 * @param bs         the byte stream to parse
 * @param byteorder  the byte order to use when parsing
 * @param format     string indicating the format for the byte stream
 * @param formatLen  length of the byte stream format string
 * @param ...        the arguments matching format
 */
OI_STATUS ByteStream_Parse(OI_BYTE_STREAM * bs,
                           OI_INT           byteorder,
                           const OI_CHAR    format[],
                           OI_UINT          formatLen,
                           ...);

/**
 * Initializes a byte stream.
 * Must be called before opening the stream.
 *
 * @param bs The byte stream to initialize
 * @param buf The data buffer to use with the stream
 * @param size The size of the data buffer
 *
 * void ByteStream_Init(OI_BYTE_STREAM  bs,
 *                      OI_BYTE *       buf,
 *                      OI_UINT16       size);
 *
 */

#define ByteStream_Init(bs, buf, size) \
do { \
    OI_ASSERT((size) > 0); \
    (bs).__error = 0; \
    (bs).__bsdata = (buf); \
    (bs).__trueSize = (size); \
    (bs).__size = (size); \
} while(0)



/**
 * Initializes a byte stream for reading or writing.
 *
 * Clears the byte stream if initializing for writing and sets size to trueSize.
 * Size will be set to the correct size when the stream is closed.
 *
 * @param bs The byte stream to initialize
 * @param rw Whether to read from or write to the byte stream.
 *
 * void ByteStream_Open(OI_BYTE_STREAM  bs,
 *                      OI_UINT8        rw);
 */

#define ByteStream_Open(bs, rw) \
do { \
    OI_ASSERT(((bs).__bsdata != NULL) && ((bs).__trueSize > 0)); \
    (bs).__pos = 0; \
    (bs).__RWFlag = (rw); \
    (bs).__size = (bs).__trueSize; \
} while(0)

/**
 * Closes a byte stream.
 *
 * @param bs The byte stream to close.
 *
 * void ByteStream_Close(OI_BYTE_STREAM  bs);
 */

#define ByteStream_Close(bs) \
do { \
    if ((bs).__RWFlag == BYTESTREAM_WRITE)  { (bs).__size = (bs).__pos; } \
    (bs).__RWFlag = BYTESTREAM_LOCKED; \
} while(0)


/**
 * Skips the next n bytes in the byte stream.
 *
 * @param bs The byte stream in which to skip bytes
 * @param count The number of bytes to skip
 *
 * void ByteStream_Skip(OI_BYTE_STREAM  bs,
 *                      OI_UINT16       count);
 */

#define ByteStream_Skip(bs, count) \
do { \
    OI_ASSERT(((bs).__pos  + (count)) <= (bs).__size); \
    (bs).__pos += (count); \
} while(0)



/**
 * Skips the next n bytes in the byte stream, checking for validity
 * of the operation.
 *
 * @param bs The byte stream in which to skip bytes
 * @param count The number of bytes to skip
 *
 * void ByteStream_Skip_Checked(OI_BYTE_STREAM  bs,
 *                              OI_UINT16       count);
 */
#define ByteStream_Skip_Checked(bs, count) \
do { \
    if (((bs).__pos  + (count)) <= (bs).__size) { \
        (bs).__pos += (count); \
     } else { \
         (bs).__error = 1; \
     } \
} while(0)



/**
 * Advances to the next byte in the buffer.
 *
 * @param bs Buffer in which to advance
 *
 * void ByteStream_Next(OI_BYTE_STREAM  bs);
 */
#define ByteStream_Next(bs) \
do { \
    OI_ASSERT(((bs).__pos  + 1) <= (bs).__size); \
    ++(bs).__pos; \
} while(0)

/**
 * Sets the position in the byte stream.
 *      (We want to make sure p isn't out of range. If p is 0, the compiler
 *       warns that 0 <= {unsigned} is always true. Using a temporary fails
 *       when release mode defines OI_ASSERT to nothing; the compiler warns of
 *       an unused variable. Adding one to both sizes avoids both these traps.)
 *
 * @param bs The bytestream in which to set the position
 * @param p The position to set
 *
 * void ByteStream_SetPos(OI_BYTE_STREAM    bs,
 *                        OI_UINT16         pos);
 */

#define ByteStream_SetPos(bs, p) \
do { \
    OI_ASSERT((OI_INT)((p) + 1) <= (OI_INT)((bs).__size + 1)); \
    (bs).__pos = (p); \
} while(0)

/**
 * Gets the position in the byte stream.
 *
 * @param bs The bytestream in which to get the position
 * @return The position
 *
 * OI_UINT16 ByteStream_GetPos(OI_BYTE_STREAM    bs);
 */

#define ByteStream_GetPos(bs) ((bs).__pos)

/**
 * Sets the size of the byte stream. The size must be less than or equal to the
 * maximum size of the byte stream buffer and greater than or equal to the
 * current read or write position.
 *
 * @param bs The byte stream whose size to set
 * @param sz The size to which to set the byte stream
 *
 * void ByteStream_SetSize(OI_BYTE_STREAM   bs,
 *                         OI_UINT16        sz);
 */

#define ByteStream_SetSize(bs, sz) \
do { \
    OI_ASSERT(((sz) <= (bs).__trueSize) && ((sz) >= (bs).__pos)); \
    (bs).__size = (sz);                                           \
} while(0)

/**
 * Gets the current size of the byte stream.
 *
 * @param bs The byte stream whose size to get
 * @return The size of the byte stream
 *
 * OI_UINT16 ByteStream_GetSize(OI_BYTE_STREAM    bs);
 */

#define ByteStream_GetSize(bs) ((bs).__size)

/**
 * Gets the maximum size (buffer size) of the byte stream.
 *
 * @param bs The byte stream whose maximum size to get
 * @return The maximum size of the byte stream
 *
 * OI_UINT16 ByteStream_GetMaxSize(OI_BYTE_STREAM   bs);
 */
#define ByteStream_GetMaxSize(bs) ((bs).__trueSize)



/**
 * Gets byte pointer to current position.
 *
 * @param bs The byte stream in which to get the byte pointer
 * @return The pointer to current position
 *
 * OI_BYTE * ByteStream_GetCurrentBytePointer(OI_BYTE_STREAM   bs);
 */
#define ByteStream_GetCurrentBytePointer(bs) (OI_BYTE*)(&((bs).__bsdata[(bs).__pos]))


/**
 * Gets ByteStream data pointer
 *
 * @param bs The byte stream in which to get the byte pointer
 * @return The pointer to start of stream data
 *
 * OI_BYTE * ByteStream_GetDataPointer(OI_BYTE_STREAM   bs);
 */
#define ByteStream_GetDataPointer(bs) (OI_BYTE*)((bs).__bsdata)


/**
 * Tests for a byte stream error.
 *
 * @param bs The byte stream to test for errors
 * @return FALSE(0) if there were no errors
 *
 * OI_BOOL ByteStream_Error(OI_BYTE_STREAM   bs);
 */
#define ByteStream_Error(bs) ((bs).__error != 0)


/**
 * Forces a byte stream error.
 *
 * @param bs The byte stream for which to force an error
 *
 * void ByteStream_SetError(OI_BYTE_STREAM   bs);
 */
#define ByteStream_SetError(bs) {((bs).__error) = 1;}


/**
 * Clears a byte stream error.
 *
 * @param bs The byte stream for which to clear an error
 *
 * void ByteStream_ClearError(OI_BYTE_STREAM   bs);
 */
#define ByteStream_ClearError(bs) {((bs).__error) = 0;}


/*******************************************************************
 * Byte stream read functions
 ******************************************************************/

/**
 * Finds how many bytes are still available for reading.
 *
 * @code
 * Example:
 *   var = ByteStream_NumReadBytesAvail(bs);
 * @endcode
 *
 * @param bs The byte stream to check
 * @return The number of bytes still available for reading
 *
 * OI_UINT16 ByteStream_NumReadBytesAvail(OI_BYTE_STREAM   bs);
 */
#define ByteStream_NumReadBytesAvail(bs) ((bs).__size - (bs).__pos)


/**
 * Extracts N bits from current byte stream starting at bit B.
 *
 * @param bs The byte stream from which to extract bits
 * @param i [out] The bits extracted
 * @param B The bit at which to start extraction
 * @param N The number of bits to extract
 *
 * void ByteStream_Extract(OI_BYTE_STREAM   bs,
 *                         OI_BYTE          i,
 *                         OI_BYTE          B,
 *                         OI_BYTE          N);
 */

#define ByteStream_Extract(bs, i, B, N) \
do { \
    OI_ASSERT((bs).__RWFlag == BYTESTREAM_READ); \
    OI_ASSERT(((B) + (N)) <= 8); \
    (i) = (OI_BYTE) (((bs).__bsdata[(bs).__pos] >> (B)) & (0xFF >> (8 - (N)))); \
} while(0)


/**
 * Gets N bytes from a byte stream.
 *
 * @param bs The byte stream from which to get the bytes
 * @param bytes [out] The bytes obtained
 * @param N Number of bytes to obtain
 *
 * void ByteStream_GetBytes(OI_BYTE_STREAM  bs,
 *                          void *          bytes,
 *                          OI_UINT16       N);
 */
#define ByteStream_GetBytes(bs, bytes, N) \
do { \
    OI_BYTE *__p = (OI_BYTE*) (bytes);                      \
    OI_BYTE *__q = (OI_BYTE*) ((bs).__bsdata + (bs).__pos); \
    OI_BYTE * const __e = __p + (N); \
    OI_ASSERT((bs).__RWFlag == BYTESTREAM_READ); \
    OI_ASSERT(((bs).__pos + (N)) <= (bs).__size); \
    while (__p < __e) { *__p++ = *__q++; } \
    (bs).__pos += N; \
} while(0)


/**
 * Gets N bytes from a byte stream, checking them for validity.
 *
 * @param bs The byte stream from which to get the bytes
 * @param bytes [out] The bytes obtained
 * @param N Number of bytes to obtain
 *
 * void ByteStream_GetBytes_Checked(OI_BYTE_STREAM  bs,
 *                                  void *          bytes,
 *                                  OI_UINT16       N);
 */
#define ByteStream_GetBytes_Checked(bs, bytes, N) \
do { \
    OI_BYTE *__p = (OI_BYTE*) (bytes);                      \
    OI_BYTE *__q = (OI_BYTE*) ((bs).__bsdata + (bs).__pos); \
    OI_BYTE * const __e = __p + (N); \
    OI_ASSERT((bs).__RWFlag == BYTESTREAM_READ); \
    if (((bs).__pos + (N)) <= (bs).__size) { \
        while (__p < __e) { *__p++ = *__q++; } \
        (bs).__pos += N; \
    } else { \
        (bs).__error = 1; \
    } \
} while(0)


/**
 * Gets an object from a byte stream.
 *
 * @param bs The byte stream from which to get the object
 * @param val [out] The object obtained
 */
#define ByteStream_GetObject(bs, val) \
   ByteStream_GetBytes((bs), ((OI_BYTE*) &(val)), sizeof(val))

/**
 * Gets an object from a byte stream, checking it for validity.
 *
 * @param bs The byte stream from which to get the object
 * @param val [out] The object obtained
 */
#define ByteStream_GetObject_Checked(bs, val) \
   ByteStream_GetBytes_Checked((bs), ((OI_BYTE*) &(val)), sizeof(val))



#define ByteStream_GetINT8 ByteStream_GetUINT8
#define ByteStream_GetBYTE ByteStream_GetUINT8

/**
 * Reads 8 bits from a byte stream.
 *
 * @param bs The byte stream from which to read the UINT8
 * @param i [out] The integer obtained
 * @param t The type of the value
 *
 * @code
 * void ByteStream_GetTyped8(OI_BYTE_STREAM   bs,
 *                           OI_BYTE          i,
 *                           <type>           t);
 * @endcode
 */

#define ByteStream_GetTyped8(bs, i, t) \
do { \
    OI_ASSERT((bs).__RWFlag == BYTESTREAM_READ); \
    OI_ASSERT(((bs).__pos + 1) <= (bs).__size); \
    (i) = (t) (bs).__bsdata[(bs).__pos++]; \
} while(0)

/**
 * Reads an unsigned 8-bit integer from a byte stream.
 *
 * @param bs The byte stream from which to read the UINT8
 * @param i [out] The integer obtained
 *
 * void ByteStream_GetUINT8(OI_BYTE_STREAM   bs,
 *                          OI_BYTE          i);
 */

#define ByteStream_GetUINT8(bs, i) ByteStream_GetTyped8(bs, i, OI_UINT8)



#define ByteStream_GetINT8_Checked ByteStream_GetUINT8_Checked
#define ByteStream_GetBYTE_Checked ByteStream_GetUINT8_Checked

/**
 * Reads 8 bits from a byte stream, checking it for validity.
 *
 * @param bs The byte stream from which to read the integer
 * @param i [out] The integer obtained
 * @param t The type of the value
 *
 * @code
 * void ByteStream_GetTyped8_Checked(OI_BYTE_STREAM   bs,
 *                                   OI_BYTE          i,
 *                                   <type>           t);
 * @endcode
 */
#define ByteStream_GetTyped8_Checked(bs, i, t) \
do { \
    OI_ASSERT((bs).__RWFlag == BYTESTREAM_READ); \
    if (((bs).__pos + 1) <= (bs).__size) { \
        (i) = (t) (bs).__bsdata[(bs).__pos++]; \
    } else { \
        (bs).__error = 1; \
    } \
} while(0)

/**
 * Reads an unsigned 8-bit integer from a byte stream, checking it for validity.
 *
 * @param bs The byte stream from which to read the integer
 * @param i [out] The integer obtained
 *
 * void ByteStream_GetUINT8_Checked(OI_BYTE_STREAM   bs,
 *                                  OI_BYTE          i);
 */
#define ByteStream_GetUINT8_Checked(bs, i)  ByteStream_GetTyped8_Checked(bs, i, OI_UINT8)



#define ByteStream_GetINT16 ByteStream_GetUINT16

/*
 * Reads a 16-bit integer from a byte stream, specifying whether the byte
 * order in the stream is big- or little-endian.
 *
 * @param bs The byte stream
 * @param i [out] The integer obtained
 * @param bo Byte order (endianness) of the stream
 *
 * void ByteStream_GetUINT16(OI_BYTE_STREAM   bs,
 *                           OI_UINT16        i,
 *                           OI_UINT8         bo);
 */
#define ByteStream_GetUINT16(bs, i, bo) \
do { \
    OI_ASSERT((bs).__RWFlag == BYTESTREAM_READ); \
    OI_ASSERT( ((bo) == OI_BIG_ENDIAN_BYTE_ORDER) || ((bo) == OI_LITTLE_ENDIAN_BYTE_ORDER) ); \
    OI_ASSERT(((bs).__pos + 2) <= (bs).__size); \
    if ((bo) == OI_BIG_ENDIAN_BYTE_ORDER) { \
        (i) = GetUINT16_BigEndian((bs).__bsdata + (bs).__pos); \
    } else { \
        (i) = GetUINT16_LittleEndian((bs).__bsdata + (bs).__pos); \
    } \
    (bs).__pos += 2; \
} while(0)


#define ByteStream_GetINT16_Checked ByteStream_GetUINT16_Checked


/*
 * Reads an 16-bit integer from a byte stream, checking it for validity and
 * specifying whether the byte
 * order in the stream is big- or little-endian.
 *
 * @param bs The byte stream from which to get the integer
 * @param i [out] The integer obtained
 * @param bo Byte order (endianness) of the stream
 *
 * void ByteStream_GetUINT16_Checked(OI_BYTE_STREAM   bs,
 *                                   OI_UINT16        i,
 *                                   OI_UINT8         bo);
 */
#define ByteStream_GetUINT16_Checked(bs, i, bo) \
do { \
    OI_ASSERT((bs).__RWFlag == BYTESTREAM_READ); \
    OI_ASSERT( ((bo) == OI_BIG_ENDIAN_BYTE_ORDER) || ((bo) == OI_LITTLE_ENDIAN_BYTE_ORDER) ); \
    if (((bs).__pos + 2) <= (bs).__size) { \
        if ((bo) == OI_BIG_ENDIAN_BYTE_ORDER) { \
            (i) = GetUINT16_BigEndian((bs).__bsdata + (bs).__pos); \
        } else { \
            (i) = GetUINT16_LittleEndian((bs).__bsdata + (bs).__pos); \
        } \
        (bs).__pos += 2; \
    } else { \
        (bs).__error = 1; \
    } \
} while(0)




#define ByteStream_GetINT24 ByteStream_GetUINT24

/**
 * Reads a 24-bit integer from a byte stream, specifying whether the byte
 * order in the stream is big- or little-endian.
 *
 * @param bs The byte stream from which to get the integer
 * @param i [out] The integer obtained
 * @param bo Byte order (endianness) of the stream
 *
 * void ByteStream_GetUINT24(OI_BYTE_STREAM   bs,
 *                           OI_UINT32        i,
 *                           OI_UINT8         bo);
 */
#define ByteStream_GetUINT24(bs, i, bo) \
do { \
    OI_ASSERT( ((bo) == OI_BIG_ENDIAN_BYTE_ORDER) || ((bo) == OI_LITTLE_ENDIAN_BYTE_ORDER) ); \
    OI_ASSERT(((bs).__pos + 3) <= (bs).__size); \
    OI_ASSERT((bs).__RWFlag == BYTESTREAM_READ); \
    OI_ASSERT(sizeof(i) >= 3); \
    if ((bo) == OI_BIG_ENDIAN_BYTE_ORDER) { \
        (i) = GetUINT24_BigEndian((bs).__bsdata + (bs).__pos); \
    } else { \
        (i) = GetUINT24_LittleEndian((bs).__bsdata + (bs).__pos); \
    } \
    (bs).__pos += 3; \
} while(0)



#define ByteStream_GetINT24_Checked ByteStream_GetUINT24

/**
 * Reads a 24-bit integer from a byte stream, checking it for validity, and
 * specifying whether the byte
 * order in the stream is big- or little-endian.
 *
 * @param bs The byte stream from which to get the integer
 * @param i [out] The integer obtained
 * @param bo Byte order (endianness) of the stream
 *
 * void ByteStream_GetUINT24_Checked(OI_BYTE_STREAM   bs,
 *                                   OI_UINT32        i,
 *                                   OI_UINT8         bo);
 */
#define ByteStream_GetUINT24_Checked(bs, i, bo) \
do { \
    OI_ASSERT((bs).__RWFlag == BYTESTREAM_READ); \
    OI_ASSERT( ((bo) == OI_BIG_ENDIAN_BYTE_ORDER) || ((bo) == OI_LITTLE_ENDIAN_BYTE_ORDER) ); \
    OI_ASSERT(sizeof(i) >= 3); \
    if (((bs).__pos + 3) <= (bs).__size) { \
        if ((bo) == OI_BIG_ENDIAN_BYTE_ORDER) { \
            (i) = GetUINT24_BigEndian((bs).__bsdata + (bs).__pos); \
        } else { \
            (i) = GetUINT24_LittleEndian((bs).__bsdata + (bs).__pos); \
        } \
        (bs).__pos += 3; \
    } else { \
        (bs).__error = 1; \
    } \
} while(0)



#define ByteStream_GetINT32 ByteStream_GetUINT32

/**
 * Reads a 32-bit integer from a byte stream, specifying whether the byte
 * order in the stream is big- or little-endian.
 *
 * @param bs The byte stream from which to get the integer
 * @param i [out] The integer obtained
 * @param bo Byte order (endianness) of the stream
 *
 * void ByteStream_GetUINT32(OI_BYTE_STREAM   bs,
 *                           OI_UINT32        i,
 *                           OI_UINT8         bo);
 */
#define ByteStream_GetUINT32(bs, i, bo) \
do { \
    OI_ASSERT( ((bo) == OI_BIG_ENDIAN_BYTE_ORDER) || ((bo) == OI_LITTLE_ENDIAN_BYTE_ORDER) ); \
    OI_ASSERT(((bs).__pos + 4) <= (bs).__size); \
    OI_ASSERT((bs).__RWFlag == BYTESTREAM_READ); \
    if ((bo) == OI_BIG_ENDIAN_BYTE_ORDER) { \
        (i) = GetUINT32_BigEndian((bs).__bsdata + (bs).__pos); \
    } else { \
        (i) = GetUINT32_LittleEndian((bs).__bsdata + (bs).__pos); \
    } \
    (bs).__pos += 4; \
} while(0)


#define ByteStream_GetINT32_Checked ByteStream_GetUINT32

/**
 * Reads a 32-bit integer from a byte stream, checking it for validity, and
 * specifying whether the byte
 * order in the stream is big- or little-endian.
 *
 * @param bs The byte stream from which to get the integer
 * @param i [out] The integer obtained
 * @param bo Byte order (endianness) of the stream
 *
 * void ByteStream_GetUINT32_Checked(OI_BYTE_STREAM   bs,
 *                                   OI_UINT32        i,
 *                                   OI_UINT8         bo);
 */
#define ByteStream_GetUINT32_Checked(bs, i, bo) \
do { \
    OI_ASSERT((bs).__RWFlag == BYTESTREAM_READ); \
    OI_ASSERT( ((bo) == OI_BIG_ENDIAN_BYTE_ORDER) || ((bo) == OI_LITTLE_ENDIAN_BYTE_ORDER) ); \
    if (((bs).__pos + 4) <= (bs).__size) { \
        if ((bo) == OI_BIG_ENDIAN_BYTE_ORDER) { \
            (i) = GetUINT32_BigEndian((bs).__bsdata + (bs).__pos); \
        } else { \
            (i) = GetUINT32_LittleEndian((bs).__bsdata + (bs).__pos); \
        } \
        (bs).__pos += 4; \
    } else { \
        (bs).__error = 1; \
    } \
} while(0)



/**
 * Reads a Bluetooth address from a byte stream,
 * specifying whether the byte order in the stream is big- or little-endian.

@code
        ByteStream_GetBDADDR(
            OI_BYTE_STREAM  bs,
            OI_BD_ADDR      ba,
            BYTE_ORDER);
@endcode

 * @param bs The byte stream from which to read the Bluetooth address
 * @param ba [out] The Bluetooth address
 * @param bo Byte order (endianness) of the stream
 */
#define ByteStream_GetBDADDR(bs, ba, bo) \
do { \
    OI_ASSERT( ((bo) == OI_BIG_ENDIAN_BYTE_ORDER) || ((bo) == OI_LITTLE_ENDIAN_BYTE_ORDER) ); \
    OI_ASSERT(((bs).__pos + OI_BD_ADDR_BYTE_SIZE) <= (bs).__size); \
    OI_ASSERT((bs).__RWFlag == BYTESTREAM_READ); \
    if ((bo) == OI_BIG_ENDIAN_BYTE_ORDER) { \
        GetBDADDR_BigEndian((ba).addr, &((bs).__bsdata[(bs).__pos]));\
    } else { \
        GetBDADDR_LittleEndian((ba).addr, &((bs).__bsdata[(bs).__pos]));\
    } \
    (bs).__pos += OI_BD_ADDR_BYTE_SIZE; \
} while(0)

/**
 * Reads a Bluetooth address from a byte stream, checking it for validity,
 * and specifying whether the byte order in the stream is big- or little-endian.

@code
        ByteStream_GetBDADDR_Checked(
            OI_BYTE_STREAM  bs,
            OI_BD_ADDR      ba,
            BYTE_ORDER);
@endcode

 * @param bs The byte stream from which to read the Bluetooth address
 * @param ba [out] The Bluetooth address
 * @param bo Byte order (endianness) of the stream
 */
#define ByteStream_GetBDADDR_Checked(bs, ba, bo) \
do { \
    OI_ASSERT((bs).__RWFlag == BYTESTREAM_READ); \
    OI_ASSERT( ((bo) == OI_BIG_ENDIAN_BYTE_ORDER) || ((bo) == OI_LITTLE_ENDIAN_BYTE_ORDER) ); \
    if (((bs).__pos + OI_BD_ADDR_BYTE_SIZE) <= (bs).__size) { \
        if ((bo) == OI_BIG_ENDIAN_BYTE_ORDER) { \
            GetBDADDR_BigEndian((ba).addr, &((bs).__bsdata[(bs).__pos]));\
        } else { \
            GetBDADDR_LittleEndian((ba).addr, &((bs).__bsdata[(bs).__pos]));\
        } \
        (bs).__pos += OI_BD_ADDR_BYTE_SIZE; \
    } else { \
        (bs).__error = 1; \
    } \
} while(0)


/**********************************************************************
 * Byte stream write functions
 **********************************************************************/

/**
 * Finds how many bytes can still be written.
 *
@code
 * Example:
 *   var = ByteStream_NumWriteBytesAllowed(bs);
@endcode
 *
 * @param bs The byte stream to write to
 * @return The number of bytes that can still be written
 *
 * OI_UINT16 ByteStream_NumWriteBytesAllowed(OI_BYTE_STREAM   bs);
 */
#define ByteStream_NumWriteBytesAllowed(bs) ((bs).__size - (bs).__pos)


/**
 * Inserts N bits into current byte stream starting at bit B.
 *
 * @param bs The byte stream into which the bits should be inserted
 * @param i The bits to insert
 * @param B The position of the bit in the byte stream starting at which the bits should be inserted
 * @param N The number of bits to insert
 *
 * void ByteStream_Insert(OI_BYTE_STREAM   bs,
 *                        OI_BYTE          i
 *                        OI_UINT8         B
 *                        OI_UINT8         N);
 */
#define ByteStream_Insert(bs, i, B, N) \
do { \
    OI_ASSERT(((B) + (N)) <= 8); \
    OI_ASSERT((bs).__RWFlag == BYTESTREAM_WRITE); \
    (bs).__bsdata[(bs).__pos] |= ((i) & (0xFF >> (8 - (N)))) << (B); \
} while(0)



/**
 * Appends N bytes to a byte stream.
 *
 * @param bs The byte stream to which the bytes should be appended
 * @param bytes The bytes to insert into the byte stream
 * @param N The number of bytes to insert
 *
 * void ByteStream_PutBytes(OI_BYTE_STREAM   bs,
 *                          OI_BYTE *        bytes
 *                          OI_UINT16        N);
 */
#define ByteStream_PutBytes(bs, bytes, N) \
do { \
    OI_BYTE *__p = (OI_BYTE*) ((bs).__bsdata + (bs).__pos); \
    OI_BYTE *__q = (OI_BYTE*) (bytes); \
    OI_BYTE * const __e = __p + (N); \
    OI_ASSERT(((bs).__pos + (N) ) <= (bs).__size); \
    OI_ASSERT((bs).__RWFlag == BYTESTREAM_WRITE); \
    while (__p < __e) { *__p++ = *__q++; }        \
    (bs).__pos += N; \
} while(0)

/**
 * Copies N bytes from one byte stream to another.
 *
 * @param dst_bs The destination byte stream
 * @param src_bs The source byte stream
 * @param N The number of bytes to copy
 *
 * void ByteStream_Copy(OI_BYTE_STREAM   dst_bs,
 *                      OI_BYTE_STREAM   src_bs,
 *                      OI_UINT16        N);
 */
#define ByteStream_Copy(dst_bs, src_bs, N) \
do { \
    OI_BYTE *__d = (OI_BYTE*) ((dst_bs).__bsdata + (dst_bs).__pos); \
    OI_BYTE *__s = (OI_BYTE*) ((src_bs).__bsdata + (src_bs).__pos); \
    OI_BYTE * const __e = __s + (N); \
    OI_ASSERT(ByteStream_NumReadBytesAvail(src_bs) >= (N)); \
    OI_ASSERT(ByteStream_NumWriteBytesAllowed(dst_bs) >= (N)); \
    OI_ASSERT((dst_bs).__RWFlag == BYTESTREAM_WRITE); \
    OI_ASSERT((src_bs).__RWFlag == BYTESTREAM_READ); \
    while (__s < __e) { *__d++ = *__s++; } \
    (src_bs).__pos += N; \
    (dst_bs).__pos += N; \
} while(0)



/**
 * Appends an object to a byte stream.
 *
 * @param bs The byte stream to which to append the object
 * @param val The object to append
 */
#define ByteStream_PutObject(bs, val) \
   ByteStream_PutBytes((bs), ((OI_BYTE*) &(val)), sizeof(val))




#define ByteStream_PutINT8 ByteStream_PutUINT8
#define ByteStream_PutBYTE ByteStream_PutUINT8

/**
 * Writes an unsigned 8-bit integer to a byte stream.
 *
 * @param bs The byte stream to which to write the integer
 * @param i The integer to write to the byte stream
 *
 * void ByteStream_PutUINT8(OI_BYTE_STREAM   bs,
 *                          OI_UINT8         i);
 */
#define ByteStream_PutUINT8(bs, i) \
{ \
    OI_ASSERT(((bs).__pos + 1) <= (bs).__size); \
    OI_ASSERT((bs).__RWFlag == BYTESTREAM_WRITE); \
    (bs).__bsdata[(bs).__pos++] = (OI_BYTE) (i); \
}



#define ByteStream_PutINT16 ByteStream_PutUINT16

/**
 * Writes a 16-bit integer to a byte stream, specifying whether the byte
 * order in the stream is big- or little-endian.
 *
 * @param bs The byte stream to which to write the integer
 * @param i The integer to write to the byte stream
 * @param bo The byte order (endianness) of the stream
 *
 * void ByteStream_PutUINT16(OI_BYTE_STREAM   bs,
 *                           OI_UINT16        i,
 *                           OI_UINT8         bo);
 */
#define ByteStream_PutUINT16(bs, i, bo) \
{ \
    OI_ASSERT( ((bo) == OI_BIG_ENDIAN_BYTE_ORDER) || ((bo) == OI_LITTLE_ENDIAN_BYTE_ORDER) ); \
    OI_ASSERT(((bs).__pos + 2) <= (bs).__size); \
    OI_ASSERT((bs).__RWFlag == BYTESTREAM_WRITE); \
    OI_ASSERT(sizeof(i) >= 2); \
    if ((bo) == OI_BIG_ENDIAN_BYTE_ORDER) { \
        SetUINT16_BigEndian((bs).__bsdata + (bs).__pos, (i)); \
    } else { \
        SetUINT16_LittleEndian((bs).__bsdata + (bs).__pos, (i)); \
    } \
    (bs).__pos += sizeof(OI_UINT16); \
}


#define ByteStream_PutINT24 ByteStream_PutUINT24

/**
 * Writes a 24-bit integer to a byte stream, specifying whether the byte
 * order in the stream is big- or little-endian.
 *
 * @param bs The byte stream to which to write the integer
 * @param i The integer to write to the byte stream
 * @param bo The byte order (endianness) of the stream
 *
 * void ByteStream_PutUINT24(OI_BYTE_STREAM   bs,
 *                           OI_UINT32        i,
 *                           OI_UINT8         bo);
 */
#define ByteStream_PutUINT24(bs, i, bo) \
{ \
    OI_ASSERT( ((bo) == OI_BIG_ENDIAN_BYTE_ORDER) || ((bo) == OI_LITTLE_ENDIAN_BYTE_ORDER) ); \
    OI_ASSERT(((bs).__pos + 3) <= (bs).__size); \
    OI_ASSERT((bs).__RWFlag == BYTESTREAM_WRITE); \
    OI_ASSERT(sizeof(i) >= 3); \
    OI_ASSERT(0 == ((i) & 0xFF000000)); \
    if ((bo) == OI_BIG_ENDIAN_BYTE_ORDER) { \
        SetUINT24_BigEndian((bs).__bsdata + (bs).__pos, (i)); \
    } else { \
        SetUINT24_LittleEndian((bs).__bsdata + (bs).__pos, (i)); \
    } \
    (bs).__pos += 3; \
}


#define ByteStream_PutINT32 ByteStream_PutUINT32

/**
 * Writes a 32-bit integer to a byte stream, specifying whether the byte
 * order in the stream is big- or little-endian.
 *
 * @param bs The byte stream to which to write the integer
 * @param i The integer to write to the byte stream
 * @param bo The byte order (endianness) of the stream
 *
 * void ByteStream_PutUINT32(OI_BYTE_STREAM   bs,
 *                           OI_UINT32        i,
 *                           OI_UINT8         bo);
 */
#define ByteStream_PutUINT32(bs, i, bo) \
{ \
    OI_ASSERT( ((bo) == OI_BIG_ENDIAN_BYTE_ORDER) || ((bo) == OI_LITTLE_ENDIAN_BYTE_ORDER) ); \
    OI_ASSERT(((bs).__pos + 4) <= (bs).__size); \
    OI_ASSERT((bs).__RWFlag == BYTESTREAM_WRITE); \
    OI_ASSERT(sizeof(i) >= 4); \
    if ((bo) == OI_BIG_ENDIAN_BYTE_ORDER) { \
        SetUINT32_BigEndian((bs).__bsdata + (bs).__pos, (i)); \
    } else { \
        SetUINT32_LittleEndian((bs).__bsdata + (bs).__pos, (i)); \
    } \
    (bs).__pos += sizeof(OI_UINT32); \
}

/**
 * Writes a Bluetooth address to a byte stream, specifying whether the byte
 * order in the stream is big- or little-endian.

@code
        ByteStream_PutBDADDR(OI_BYTE_STREAM  bs,
                             OI_BD_ADDR      ba,
                             OI_UINT8        bo);
@endcode

 *
 * @param bs The byte stream to which to write the Bluetooth address
 * @param ba The Bluetooth address to write to the byte stream
 * @param bo The byte order (endianness) of the stream
 */
#define ByteStream_PutBDADDR(bs, ba, bo) \
{ \
    OI_ASSERT( ((bo) == OI_BIG_ENDIAN_BYTE_ORDER) || ((bo) == OI_LITTLE_ENDIAN_BYTE_ORDER) ); \
    OI_ASSERT(((bs).__pos + OI_BD_ADDR_BYTE_SIZE) <= (bs).__size); \
    OI_ASSERT((bs).__RWFlag == BYTESTREAM_WRITE); \
    if ((bo) == OI_BIG_ENDIAN_BYTE_ORDER) { \
        SetBDADDR_BigEndian(&((bs).__bsdata[(bs).__pos]), (ba).addr);\
    } else { \
        SetBDADDR_LittleEndian(&((bs).__bsdata[(bs).__pos]), (ba).addr);\
    } \
    (bs).__pos += OI_BD_ADDR_BYTE_SIZE; \
}


/*****************************************************************************/
#ifdef __cplusplus
}
#endif

/**@}*/

#endif /* _OI_BYTESTREAM_H */

