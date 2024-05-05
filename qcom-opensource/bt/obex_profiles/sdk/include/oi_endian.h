#ifndef _OI_ENDIAN_H
#define _OI_ENDIAN_H

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

 This file contains macros for dealing with byte-wise endianness (byte order
 in multibyte data items).
*/

#include "oi_stddefs.h"
#include "oi_bt_spec.h"


/** \addtogroup Marshaller_Internal */
/**@{*/

#ifdef __cplusplus
extern "C" {
#endif


/* Write values in BIG endian order */

#define SetUINT32_BigEndian(dst,value)                                 \
do {                                                                   \
     ((OI_UINT8*)(dst))[3] = (OI_UINT8) ((value) & 0x000000ff);        \
     ((OI_UINT8*)(dst))[2] = (OI_UINT8)(((value) & 0x0000ff00) >> 8);  \
     ((OI_UINT8*)(dst))[1] = (OI_UINT8)(((value) & 0x00ff0000) >> 16); \
     ((OI_UINT8*)(dst))[0] = (OI_UINT8)(((value) & 0xff000000) >> 24); \
} while(0)

#define SetUINT24_BigEndian(dst, value)                                \
do {                                                                   \
     ((OI_UINT8*)(dst))[2] = (OI_UINT8) ((value) & 0x000000ff);        \
     ((OI_UINT8*)(dst))[1] = (OI_UINT8)(((value) & 0x0000ff00) >> 8);  \
     ((OI_UINT8*)(dst))[0] = (OI_UINT8)(((value) & 0x00ff0000) >> 16); \
} while(0)

#define SetUINT16_BigEndian(dst,value)                                 \
do {                                                                   \
     ((OI_UINT8*)(dst))[1] = (OI_UINT8) ((value) & 0x00ff);            \
     ((OI_UINT8*)(dst))[0] = (OI_UINT8)(((value) & 0xff00) >> 8);      \
} while(0)

#define SetUINT8_BigEndian(dst,value)                   \
do {                                                    \
     ((OI_UINT8*)(dst))[0] = (OI_UINT8) ((value));      \
} while(0)

/* Write values in LITTLE endian order */

#define SetUINT32_LittleEndian(dst,value)                              \
do {                                                                   \
     ((OI_UINT8*)(dst))[0] = (OI_UINT8) ((value) & 0x000000ff);        \
     ((OI_UINT8*)(dst))[1] = (OI_UINT8)(((value) & 0x0000ff00) >> 8);  \
     ((OI_UINT8*)(dst))[2] = (OI_UINT8)(((value) & 0x00ff0000) >> 16); \
     ((OI_UINT8*)(dst))[3] = (OI_UINT8)(((value) & 0xff000000) >> 24); \
} while(0)

#define SetUINT24_LittleEndian(dst, value)                             \
do {                                                                   \
     ((OI_UINT8*)(dst))[0] = (OI_UINT8) ((value) & 0x000000ff);        \
     ((OI_UINT8*)(dst))[1] = (OI_UINT8)(((value) & 0x0000ff00) >> 8);  \
     ((OI_UINT8*)(dst))[2] = (OI_UINT8)(((value) & 0x00ff0000) >> 16); \
} while(0)

#define SetUINT16_LittleEndian(dst,value)                              \
do {                                                                   \
     ((OI_UINT8*)(dst))[0] = (OI_UINT8) ((value) & 0x00ff);            \
     ((OI_UINT8*)(dst))[1] = (OI_UINT8)(((value) & 0xff00) >> 8);      \
} while(0)

#define SetUINT8_LittleEndian(dst,value)                \
do {                                                    \
     ((OI_UINT8*)(dst))[0] = (OI_UINT8) ((value));      \
} while(0)



/* Read values in BIG endian order */

#define GetUINT32_BigEndian(src)                         \
    ((OI_UINT32) (                                       \
        ((OI_UINT32)(((OI_UINT8 *)(src))[3])      )  |   \
        ((OI_UINT32)(((OI_UINT8 *)(src))[2]) << 8 )  |   \
        ((OI_UINT32)(((OI_UINT8 *)(src))[1]) << 16)  |   \
        ((OI_UINT32)(((OI_UINT8 *)(src))[0]) << 24) ))

#define GetUINT24_BigEndian(src)                         \
    ((OI_UINT32) (                                       \
        ((OI_UINT32)(((OI_UINT8 *)(src))[2])      )  |   \
        ((OI_UINT32)(((OI_UINT8 *)(src))[1]) << 8 )  |   \
        ((OI_UINT32)(((OI_UINT8 *)(src))[0]) << 16) ))

#define GetUINT16_BigEndian(src)                         \
    ((OI_UINT16) (                                       \
        ((OI_UINT16)(((OI_UINT8 *)(src))[1])      )  |   \
        ((OI_UINT16)(((OI_UINT8 *)(src))[0]) << 8 ) ))

#define GetUINT8_BigEndian(src)             \
    (* ((OI_UINT8 *)(src)) )

/* Read values in LITTLE endian order */

#define GetUINT32_LittleEndian(src)                      \
    ((OI_UINT32) (                                       \
        ((OI_UINT32)(((OI_UINT8 *)(src))[0])      )  |   \
        ((OI_UINT32)(((OI_UINT8 *)(src))[1]) << 8 )  |   \
        ((OI_UINT32)(((OI_UINT8 *)(src))[2]) << 16)  |   \
        ((OI_UINT32)(((OI_UINT8 *)(src))[3]) << 24) ))

#define GetUINT24_LittleEndian(src)                      \
    ((OI_UINT32) (                                       \
        ((OI_UINT32)(((OI_UINT8 *)(src))[0])      )  |   \
        ((OI_UINT32)(((OI_UINT8 *)(src))[1]) << 8 )  |   \
        ((OI_UINT32)(((OI_UINT8 *)(src))[2]) << 16) ))

#define GetUINT16_LittleEndian(src)                      \
    ((OI_UINT16) (                                       \
        ((OI_UINT16)(((OI_UINT8 *)(src))[0])      )  |   \
        ((OI_UINT16)(((OI_UINT8 *)(src))[1]) << 8 ) ))

#define GetUINT8_LittleEndian(src)          \
    (* ((OI_UINT8 *)(src)) )


/*
 * In-place endian conversion on an array of UINT16
 */


/* Convert array from CPU byte order to BigEndian */

void OI_SetUINT16Array_BigEndian(OI_UINT16 *data,
                                 OI_UINT16 len);

/* Convert array from BigEndian to CPU byte order */

void OI_GetUINT16Array_BigEndian(OI_UINT16 *data,
                                 OI_UINT16 len);

/*
 * BD_ADDR is an array of bytes - we have defined its native type as being a BIG
 * ENDIAN array of bytes.
 */

#define GetBDADDR_BigEndian(dest, src)     \
do {                                       \
        (dest)[0] = (src)[0];              \
        (dest)[1] = (src)[1];              \
        (dest)[2] = (src)[2];              \
        (dest)[3] = (src)[3];              \
        (dest)[4] = (src)[4];              \
        (dest)[5] = (src)[5];              \
} while(0)

#define GetBDADDR_LittleEndian(dest, src)      \
do {                                           \
        (dest)[0] = (src)[5];                  \
        (dest)[1] = (src)[4];                  \
        (dest)[2] = (src)[3];                  \
        (dest)[3] = (src)[2];                  \
        (dest)[4] = (src)[1];                  \
        (dest)[5] = (src)[0];                  \
} while(0)

#define SetBDADDR_BigEndian(dest, src)      \
do {                                        \
        (dest)[0] = (src)[0];               \
        (dest)[1] = (src)[1];               \
        (dest)[2] = (src)[2];               \
        (dest)[3] = (src)[3];               \
        (dest)[4] = (src)[4];               \
        (dest)[5] = (src)[5];               \
} while(0)

#define SetBDADDR_LittleEndian(dest, src)       \
do { \
        (dest)[0] = (src)[5];                   \
        (dest)[1] = (src)[4];                   \
        (dest)[2] = (src)[3];                   \
        (dest)[3] = (src)[2];                   \
        (dest)[4] = (src)[1];                   \
        (dest)[5] = (src)[0];                   \
} while(0)
/*
 * Natively, OI_UUID128 is a 32-bit integer followed by the remaining 96 bits as
 * an array of bytes in BIG ENDIAN order.
 */

#define GetUUID128_BigEndian(dest, src)             \
do {                                                \
        (dest).ms32bits = GetUINT32_BigEndian(src); \
        (dest).base[0]  = (src)[4];                 \
        (dest).base[1]  = (src)[5];                 \
        (dest).base[2]  = (src)[6];                 \
        (dest).base[3]  = (src)[7];                 \
        (dest).base[4]  = (src)[8];                 \
        (dest).base[5]  = (src)[9];                 \
        (dest).base[6]  = (src)[10];                \
        (dest).base[7]  = (src)[11];                \
        (dest).base[8]  = (src)[12];                \
        (dest).base[9]  = (src)[13];                \
        (dest).base[10] = (src)[14];                \
        (dest).base[11] = (src)[15];                \
} while(0)

#define GetUUID128_LittleEndian(dest, src)                                                        \
do {                                                                                              \
        (dest).ms32bits = GetUINT32_LittleEndian(src + (OI_BT_UUID128_SIZE - sizeof(OI_UINT32))); \
        (dest).base[0]  = (src)[11];                                                              \
        (dest).base[1]  = (src)[10];                                                              \
        (dest).base[2]  = (src)[9];                                                               \
        (dest).base[3]  = (src)[8];                                                               \
        (dest).base[4]  = (src)[7];                                                               \
        (dest).base[5]  = (src)[6];                                                               \
        (dest).base[6]  = (src)[5];                                                               \
        (dest).base[7]  = (src)[4];                                                               \
        (dest).base[8]  = (src)[3];                                                               \
        (dest).base[9]  = (src)[2];                                                               \
        (dest).base[10] = (src)[1];                                                               \
        (dest).base[11] = (src)[0];                                                               \
} while(0)

#define SetUUID128_BigEndian(dest, src)              \
do {                                                 \
        SetUINT32_BigEndian((dest), (src).ms32bits); \
        (dest)[4]  = (src).base[0];                  \
        (dest)[5]  = (src).base[1];                  \
        (dest)[6]  = (src).base[2];                  \
        (dest)[7]  = (src).base[3];                  \
        (dest)[8]  = (src).base[4];                  \
        (dest)[9]  = (src).base[5];                  \
        (dest)[10] = (src).base[6];                  \
        (dest)[11] = (src).base[7];                  \
        (dest)[12] = (src).base[8];                  \
        (dest)[13] = (src).base[9];                  \
        (dest)[14] = (src).base[10];                 \
        (dest)[15] = (src).base[11];                 \
} while(0)

#define SetUUID128_LittleEndian(dest, src)                                                           \
do {                                                                                                 \
        SetUINT32_LittleEndian(((dest) + (OI_BT_UUID128_SIZE - sizeof(OI_UINT32))), (src).ms32bits); \
        (dest)[0]  = (src).base[11];                                                                 \
        (dest)[1]  = (src).base[10];                                                                 \
        (dest)[2]  = (src).base[9];                                                                  \
        (dest)[3]  = (src).base[8];                                                                  \
        (dest)[4]  = (src).base[7];                                                                  \
        (dest)[5]  = (src).base[6];                                                                  \
        (dest)[6]  = (src).base[5];                                                                  \
        (dest)[7]  = (src).base[4];                                                                  \
        (dest)[8]  = (src).base[3];                                                                  \
        (dest)[9]  = (src).base[2];                                                                  \
        (dest)[10] = (src).base[1];                                                                  \
        (dest)[11] = (src).base[0];                                                                  \
} while(0)


/*****************************************************************************/
#ifdef __cplusplus
}
#endif

/**@}*/

#endif /* _OI_ENDIAN_H */

