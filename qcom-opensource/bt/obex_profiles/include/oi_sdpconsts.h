#ifndef _SDPCONSTS_H
#define _SDPCONSTS_H

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

#include "oi_sdp.h"
#include "oi_bt_assigned_nos.h"
#include "oi_status.h"


/** \addtogroup SvcDisc_Internal */
/**@{*/

#ifdef __cplusplus
extern "C" {
#endif


/*
 * SDP PDU ID codes
 */

#define SDP_ERROR_RESPONSE                     0x01
#define SDP_SERVICE_SEARCH_REQUEST             0x02
#define SDP_SERVICE_SEARCH_RESPONSE            0x03
#define SDP_SERVICE_ATTRIBUTE_REQUEST          0x04
#define SDP_SERVICE_ATTRIBUTE_RESPONSE         0x05
#define SDP_SERVICE_SEARCH_ATTRIBUTE_REQUEST   0x06
#define SDP_SERVICE_SEARCH_ATTRIBUTE_RESPONSE  0x07

/*
 * An SDP PDU header is 5 bytes long:
 *
 *  -----------------------------------------------------------------------------
 *  |   PDU ID   |     TRANSACTION ID     |    Parameter Length   |  parameters...
 *  -----------------------------------------------------------------------------
 *  <== 1 byte ==><======= 2 bytes =======><====== 2 bytes =======>
 */

#define TRANSACTION_ID_OFFSET  1   // Where the transaction id starts
#define PARAM_LENGTH_OFFSET    3   // Where the parameter length starts

#define SDP_PDU_HEADER_SIZE    5

/**
 * An attributes ID is represented in an SDP PDU by a one byte header and two
 * bytes of value.
 */
#define SDP_ATTRIBUTE_ID_BYTES   (sizeof(OI_UINT8) + sizeof(OI_UINT16))

/**
 * The size of an error response packet (assumes no additional error info)
 */

#define SDP_ERROR_RESPONSE_SIZE  (SDP_PDU_HEADER_SIZE + 2)

/**
 * The SDP specification defines a set of error codes. We map the SDP error
 * codes into OI_STATUS codes. This macro is the reverse mapping.
 */

#define SDP_ERROR_CODE(e)    ((e) - OI_SDP_SPEC_ERROR)


/**
 * Maximum number of bytes in a continuation state parameter.
 *
 * Defined to be 16 bytes for SDP V1.0
 */
#define SDP_CONTINUATION_STATE_MAX_LEN 16

/**
 * SDP transfer byte order is big endian
 */
#define SDP_BO  OI_BIG_ENDIAN_BYTE_ORDER

/**
 * As defined in the SDP 1.1 specification: the maximum number of service Ids
 * that can be specified in a single service request
 */
#define SDP_MAX_SERVICE_IDS  12

/**
 * SDP spec says that service record handles from 0x0001 to 0xFFFF are reserved.
 *
 * Service record handle 0 is the service record handle for the SDP service
 * itself.
 */

#define SDP_MIN_SERVICE_RECORD_HANDLE   ((OI_SDP_SERVICE_RECORD_HANDLE) 0x4F490000)


/**
 * The minimum size of a MaxAttributeByteCount is defined by the SDP
 * specification to be 7 bytes. This means that the SDP server must be able to
 * support at least a 7 byte attribute response.
 */

#define SDP_MIN_ATTRIBUTE_BYTE_COUNT   7


/**
 * The zero service record handle (0x00000000) is always interpreted by an SDP
 * server to mean the SDP server itself. The service record for an SDP server
 * lists all of the attributes supported by the SDP server including the
 * protocol versions, etc.
 */

#define SDPSERVICE_SERVICE_RECORD_HANDLE 0x00000000

/*****************************************************************************/
#ifdef __cplusplus
}
#endif

/**@}*/

#endif /* _SDPCONSTS_H */

