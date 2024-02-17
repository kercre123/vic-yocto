#ifndef _OI_L2CAP_QOS
#define _OI_L2CAP_QOS

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
 * This file contains values from the flow specification for guaranteed Quality of Service.
 *
 * Management of guaranteed quality of service at the L2CAP level is not supported
 * in the present BLUEmagic 3.0 software implementation of L2CAP.
 * This file contains some of the infrastructure for management of L2CAP guaranteed
 * quality of service. The flow specification structures defined in this file do not
 * presently have any functional effect at the L2CAP level. L2CAP management of
 * guaranteed quality of service will be supported in a future version of BLUEmagic software.
 */

#include "oi_stddefs.h"

/** \addtogroup L2CAP L2CAP APIs */
/**@{*/

#ifdef __cplusplus
extern "C" {
#endif


/** This structure indicates the flow specification for an L2CAP
 *  connection. */
typedef struct _OI_L2CAP_FLOWSPEC {

    /** level of service required */
    OI_UINT8   serviceType;

    /** rate at which traffic credits are granted in bytes/sec */
    OI_UINT32   tokenRate;

    /** size of the token bucket in bytes */
    OI_UINT32   tokenBucketSize;

    /** limit in bytes/second on how fast packets may be sent back-to-back from applications */
    OI_UINT32   peakBandwidth;

    /** maximum delay between tranmission by sender and tranmission over air, in microseconds */
    OI_UINT32   latency;

    /** difference between maximum and minimum delay that a packet will experience */
    OI_UINT32   delayVariation;

} OI_L2CAP_FLOWSPEC;

#define OI_L2CAP_SERVICE_TYPE_NO_TRAFFIC   0x00                             /**< service type: no traffic  */
#define OI_L2CAP_SERVICE_TYPE_BEST_EFFORT  0x01                             /**< service type: best effort  */
#define OI_L2CAP_SERVICE_TYPE_GUARANTEED   0x02                             /**< service type: guaranteed  */
#define OI_L2CAP_SERVICE_TYPE_DEFAULT      OI_L2CAP_SERVICE_TYPE_BEST_EFFORT   /**< The default service type is best effort.  */

#define OI_L2CAP_TOKEN_RATE_NONE           0                                /**< No token rate is specified.  */
#define OI_L2CAP_TOKEN_RATE_MAX            0xffffffff                       /**< Token rate is maximum available.  */
#define OI_L2CAP_TOKEN_RATE_DEFAULT        OI_L2CAP_TOKEN_RATE_NONE            /**< The default is for no token rate to be specified.  */

#define OI_L2CAP_TOKEN_BUCKET_NONE         0                                /**< No token bucket is needed.  */
#define OI_L2CAP_TOKEN_BUCKET_MAX          0xffffffff                       /**< Token bucket is maximum available.  */
#define OI_L2CAP_TOKEN_BUCKET_DEFAULT      OI_L2CAP_TOKEN_BUCKET_NONE          /**< The default is for no token rate to be specified.  */

#define OI_L2CAP_PEAK_BANDWIDTH_UNKNOWN    0                                /**< Maximum bandwidth is unknown. */
#define OI_L2CAP_PEAK_BANDWIDTH_DEFAULT    OI_L2CAP_PEAK_BANDWIDTH_UNKNOWN     /**< The default is for the maximum bandwidth setting to indicate that maximum bandwidth is unknown. */

#define OI_L2CAP_LATENCY_DONT_CARE         0xffffffff                       /**< The application does not care about latency. */
#define OI_L2CAP_LATENCY_DEFAULT           OI_L2CAP_LATENCY_DONT_CARE          /**< The default is for the latency setting to indicate that the application does not care about latency. */

#define OI_L2CAP_DELAY_VARIATION_DONT_CARE 0xffffffff                       /**< The application does not care about delay variation. */
#define OI_L2CAP_DELAY_VARIATION_DEFAULT   OI_L2CAP_DELAY_VARIATION_DONT_CARE  /**< The default is for the delay variation setting to indicate that the application does not care about delay variation. */

#ifdef __cplusplus
}
#endif

/**@}*/

#endif /* _OI_L2CAP_QOS */

