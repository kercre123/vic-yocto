#ifndef _OI_OBEX_SD_H
#define _OI_OBEX_SD_H

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
 * This file provides the API for a service discovery helper function that gets the information
 * required by an OBEX client to establish a connection to a remote OBEX server.
 *
 */

#include "oi_common.h"
#include "oi_status.h"
#include "oi_obex.h"

/** \addtogroup OBEX OBEX APIs */
/**@{*/

#ifdef __cplusplus
extern "C" {
#endif



#define OI_OBEX_SD_QUERY_MAX_PROTOCOLS         3

/**
 * This structure includes the device address and service UUID fields that are filled in by the
 * caller, then OI_OBEX_SD_Query() is called. When the query completes successfully, the protocols
 * list is filled in.
 */

typedef struct {
    OI_BD_ADDR devAddr;                                               /**< Bluetooth device address of the device with the SDP database */
    OI_UUID32 serviceUUID;                                            /**< UUID identifying the service to query for */
    OI_OBEX_LOWER_PROTOCOL protocols[OI_OBEX_SD_QUERY_MAX_PROTOCOLS]; /**< A list of protocols and protocol specific link information */
    OI_UINT8 numProtocols;                                            /**< The number of protocols supported */
    OI_UINT16 profileVersion;                                         /**< The profile version number */
} OI_OBEX_SD_INFO;


/**
 * A callback function of this type is called to return the result of the service database query.
 *
 * @param query        Pointer to the structure containing return parameters.  This is the same structure
 *                      that was originally passed to OI_OBEX_SD_Query().
 *
 * @param status       OI_OK if query successful; failure code otherwise
 */

typedef void (*OI_OBEX_SD_QUERY_RESPONSE_CB)(OI_OBEX_SD_INFO *query,
                                            OI_STATUS status);

/**
 * This function issues a query to an Service Discovery Protocol database of services
 * in order to obtain information about an OBEX service.
 *
 * To use an OBEX service registered in the SDP database, a client application needs to know lower
 * layer protocol (RFCOMM, L2CAP, etc) information for the service. This helper function composes
 * the appropriate SDP request to obtain this information for a service identified by its UUID. The
 * service name and service description are also retrieved by this function.
 *
 * @param preferred     The preferred protocol. The query reponses will be sorted so that the
 *                      preferred protocol entries appear first in the list.
 *
 * @param query         Pointer to the structure containing the inquiry parameters
 *                      (Bluetooth device address and service UUID)
 *                      @note   Since the query process fills in structure elements, this
 *                              structure must persist at least until the queryRsp callback is called.
 *
 * @param queryRsp      The callback function that will be called when the inquiry has completed
 *
 * @return              OI_OK if the query was sumbitted successfully; failure code otherwise
 *
 */
OI_STATUS OI_OBEX_SD_Query(OI_OBEX_LOWER_PROTOCOL_ID preferred,
                           OI_OBEX_SD_INFO *query,
                           OI_OBEX_SD_QUERY_RESPONSE_CB queryRsp);


#ifdef __cplusplus
}
#endif

/**@}*/

#endif /* _OI_OBEX_SD_H */

