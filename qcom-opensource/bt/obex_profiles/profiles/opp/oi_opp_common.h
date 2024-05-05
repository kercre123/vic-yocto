#ifndef _OPPCOMMON_H
#define _OPPCOMMON_H

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

Object Push Profile common definitions and functions
*/

#include "oi_status.h"
#include "oi_obexspec.h"
#ifdef __cplusplus
extern "C" {
#endif


/**
 * struct for describing a generic OBEX object. The struct may contain a
 * complete object or partial object data.
 */
typedef struct {
    OI_OBEX_UNICODE name;     /**< Unicode name of the object */
    OI_OBEX_BYTESEQ type;     /**< NULL-terminated ASCII type for the object */
    OI_UINT32 objSize;        /**< total size of the object */
    OI_OBEX_BYTESEQ objData;  /**< object data */
} OI_OPP_GENERIC_OBJECT;

OI_STATUS OPPCommon_ParseHeaders(OI_OPP_GENERIC_OBJECT *obj,
                                 OI_OBEX_HEADER_LIST *headers);




#ifdef __cplusplus
}
#endif

#endif /* _OPPCOMMON_H */

/**@}*/
