#ifndef _OI_BPP_H
#define _OI_BPP_H

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
 * This file provides the interface for a Basic Printing Profile appliation.
 */

#include "oi_common.h"
#include "oi_obexspec.h"
#include "oi_obexsrv.h"

/** \addtogroup BPP BPP APIs */
/**@{*/

#ifdef __cplusplus
extern "C" {
#endif


#define OI_BPP_VERSION 0x0100

typedef OI_UINT32 OI_BPP_JOB_ID;

typedef enum {
    BPP_TARGET_DPS = 1, /**< Direct printing */
    BPP_TARGET_PBR,     /**< Print by reference */
    BPP_TARGET_RUI,     /**< Reflected user interface */
    BPP_TARGET_STS,     /**< Status */
    REF_TARGET_OBJ      /**< Referenced objects */
} OI_BPP_TARGET;

typedef enum {
    OI_BPP_REF_SIMPLE = 1,
    OI_BPP_REF_XML,
    OI_BPP_REF_LIST
} OI_BPP_REF_TYPE;

#ifdef __cplusplus
}
#endif

/**@}*/

#endif /* _OI_BPP_H */
