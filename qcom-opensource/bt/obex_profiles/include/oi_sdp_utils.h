#ifndef _OI_SDP_UTILS_H
#define _OI_SDP_UTILS_H

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

#include "oi_stddefs.h"
#include "oi_obex.h"
#include "oi_sdpdb.h"

/** \addtogroup SvcDisc_Internal */
/**@{*/

#ifdef __cplusplus
extern "C" {
#endif


/*
 * Initialize SDP attributes for an OBEX profile
 *
 * @param lowerProtocols     A list of lower layer protocols.
 *
 * @param numLowerProtocols  The length of the lower protocols list.
 *
 * @param pdlAttribute       Pointer the attribute structure to be allocated and initialized
 *                           according to the protocol list. This structure must eventually be freed
 *                           by calling OI_DataElement_Free().
 *
 * @param numAttributes      [in/out] Passes the size of the attributeList to the function and
 *                           returns the number of attributes set.
 */
OI_STATUS OI_SDP_UTILS_OBEX_MakeAttributes(OI_OBEX_LOWER_PROTOCOL *lowerProtocols,
                                           OI_UINT8 numLowerProtocols,
                                           OI_SDPDB_ATTRIBUTE *attributeList,
                                           OI_UINT8 *numAttributes);


/*
 * Verify that a data element conforms to an expected PDL structure and parses it.
 *
 * @param   Pointer to a data element. This is expected to be a data element sequence representing a
 *          protocol stack as it should appear in a protocol descriptor list.
 *
 * @param   Pointer to an OBEX lower protocol structure
 */
OI_STATUS OI_SDP_UTILS_OBEX_ParsePDL(OI_DATAELEM *elem,
                                     OI_OBEX_LOWER_PROTOCOL *lowerProtocol);

/*
 * Initialize a protocol descriptor list attribute for an RFCOMM profile
 *
 * @param channelNumber      The rfcomm channel number to include in the PDL.
 *
 * @param pdlAttribute       Pointer the attribute structure to be allocated and initialized.
 *                           This structure must eventually be freed by calling
 *                           OI_DataElement_Free().
 */
OI_STATUS OI_SDP_UTILS_RFCOMM_PDLAttribute(OI_UINT8 channelNumber,
                                           OI_SDPDB_ATTRIBUTE *pdlAttribute);


/*
 * Verify that a data element conforms to an expected PDL structure and parses it.
 *
 * @param   Pointer to a data element. This is expected to be a data element sequence representing a
 *          protocol stack as it should appear in a protocol descriptor list.
 *
 * @param   Pointer to return the RFCOMM channel number
 */
OI_STATUS OI_SDP_UTILS_RFCOMM_ParsePDL(OI_DATAELEM *elem,
                                       OI_UINT8 *channel);


/*****************************************************************************/
#ifdef __cplusplus
}
#endif

/**@}*/

#endif /* _OI_SDP_UTILS_H */

