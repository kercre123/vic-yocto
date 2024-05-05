#ifndef _OI_TEXT_H
#define _OI_TEXT_H

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
 * @file
 *
 * This file provides an API to functions for generation of text representations.
 *
 */

#include "oi_common.h"

/** \addtogroup Debugging Debugging APIs */
/**@{*/

#ifdef __cplusplus
extern "C" {
#endif


/**
 * This function generates a text string for a BDADDR for printing out Bluetooth addresses in
 * the SDK samples and in application test programs.
 *
 * @param addr  Pointer to a Bluetooth address.
 *
 * @return Pointer to text string in the form nn:nn:nn:nn:nn:nn.
 *
 * @note The buffer for the text string is statically allocated and should not be
 * freed. There is only one buffer so the following will not work:
 @code
   printf("addr1=%s adddr2=%s", OI_BDAddrText(&addr1), OI_BDAddrText(&addr2));
 @endcode
 */

OI_CHAR* OI_BDAddrText(const OI_BD_ADDR *addr);


/**
 * This function generates a text string for a number of bytes.
 *
 * @param bytes  Starting address of the bytes for which a text string is generated.
 * @param len  Number of bytes to output.
 *
 * @return Pointer to a text representation of the bytes.
 */

OI_CHAR* OI_HexText(const OI_BYTE* bytes,
                    OI_UINT16 len);


#ifdef __cplusplus
}
#endif

/**@}*/

#endif /* _OI_TEXT_H */

