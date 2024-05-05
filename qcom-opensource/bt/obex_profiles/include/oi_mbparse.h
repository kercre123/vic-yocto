#ifndef _OI_MBPARSE_H
#define _OI_MBPARSE_H


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

#include "oi_common.h"
#include "oi_mbuf.h"

#include "oi_mbuf.h"

#ifdef __cplusplus
extern "C" {
#endif

extern OI_UINT8  OI_MBPARSE_UINT8_value;
extern OI_UINT16 OI_MBPARSE_UINT16_value;
extern OI_UINT32 OI_MBPARSE_UINT32_value;

/**
 * Pull a UINT8 from an mbuf.
 *
 * @param  mbuf     MBUF source
 * @return   UINT8 value from mbuf
 *
 * OI_UINT8 OI_MBParse_GetUINT8(OI_MBUF *mbuf);
 */
#define OI_MBParse_GetUINT8(mbuf)  (OI_MBUF_PullBytes((OI_BYTE *)&OI_MBPARSE_UINT8_value, mbuf, sizeof(OI_UINT8)), OI_MBPARSE_UINT8_value)

/**
 * Pull a UINT16 from an mbuf.
 *
 * @param  mbuf     MBUF source
 * @param  endian   OI_LITTLE_ENDIAN_BYTE_ORDER or OI_BIG_ENDIAN_BYTE_ORDER
 * @return  OI_UINT16 value from mbuf
 *
 * OI_UINT16 OI_MBParse_GetUINT16(OI_MBUF *mbuf, OI_BYTE endian);
 */
#define OI_MBParse_GetUINT16(mbuf, endian) \
    (OI_MBUF_PullBytes((OI_BYTE *)&OI_MBPARSE_UINT16_value, (mbuf), sizeof(OI_UINT16)), \
    (endian == OI_LITTLE_ENDIAN_BYTE_ORDER) ? GetUINT16_LittleEndian(&OI_MBPARSE_UINT16_value) : GetUINT16_BigEndian(&OI_MBPARSE_UINT16_value))

/**
 * Pull a UINT32 from an mbuf.
 *
 * @param  mbuf     MBUF source
 * @param  endian   OI_LITTLE_ENDIAN_BYTE_ORDER or OI_BIG_ENDIAN_BYTE_ORDER
 * @return  OI_UINT32 value from mbuf
 *
 * OI_UINT32 OI_MBParse_GetUINT32(OI_MBUF *mbuf, OI_BYTE endian);
 */
#define OI_MBParse_GetUINT32(mbuf, endian) \
    (OI_MBUF_PullBytes((OI_BYTE *)&OI_MBPARSE_UINT32_value, (mbuf), sizeof(OI_UINT32)), \
    (endian == OI_LITTLE_ENDIAN_BYTE_ORDER) ? GetUINT32_LittleEndian(&OI_MBPARSE_UINT32_value) : GetUINT32_BigEndian(&OI_MBPARSE_UINT32_value))


#ifdef __cplusplus
}
#endif

#endif      /* _OI_MBPARSE_H */
