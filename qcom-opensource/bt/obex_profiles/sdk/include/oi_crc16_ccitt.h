#ifndef _OI_CRC16_CCIT_H
#define _OI_CRC16_CCIT_H

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

    This file defines the interface to CCIT CRC16 computations as specified in the
    HCI 3-wire specification.
*/

#include "oi_stddefs.h"

/** \addtogroup Misc Miscellaneous APIs */
/**@{*/

#ifdef __cplusplus
extern "C" {
#endif


/**
 * This function computes a 16-bit CRC on a buffer, using the caller's supplied seed as
 * a running CRC. The result is returned to the caller in the caller's running CRC variable.
 *
 * @param buffer         Buffer for which to compute the CRC
 * @param bufLen         Length of the buffer in bytes
 * @param runningCrc     Input -  Pointer to seed for CRC computation
 *                       Putput - Pointer to result of CRC computation
 */
void OI_CRC16_CCITT_Compute(const OI_BYTE *buffer,
                            OI_UINT16 bufLen,
                            OI_UINT16 *runningCrc);

/**
 * This function completes the CCIT CRC computation by rearranging the CRC bits and bytes
 * into the correct order.
 *
 * @param crc            Computed CRC as calculated by OI_CCIT_CRC_Compute()
 * @param crcBlock       Pointer to a 2-byte buffer where the resulting CRC will be stored
 */

void OI_CRC16_CCITT_Complete(OI_UINT16 crc,
                             OI_BYTE   *crcBlock);

/*****************************************************************************/
#ifdef __cplusplus
}
#endif

/**@}*/

#endif /* _OI_CRC16_CCIT_H */

