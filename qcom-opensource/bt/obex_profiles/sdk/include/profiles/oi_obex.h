#ifndef _OI_OBEX_H
#define _OI_OBEX_H

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
 This file contains typedefs required by all OBEX profiles..
 */

#include "oi_bt_assigned_nos.h"
#include "oi_stddefs.h"
#include "oi_debug.h"
#include "oi_status.h"
#include <hardware/bluetooth.h>
#include <hardware/bt_sock.h>

/** \addtogroup OBEX */
/**@{*/

#ifdef __cplusplus
extern "C" {
#endif


/**
 * Opaque type for an OBEX connection (client or server)
 */
typedef OI_HANDLE OI_OBEX_CONNECTION_HANDLE;


/**
 * Opaque type for an OBEX server instance
 */
typedef OI_HANDLE OI_OBEX_SERVER_HANDLE;


/**
 * Lower layer protocols identifiers for OBEX (mapped to the 16 bit UUID values)
 */
typedef enum {
    OI_OBEX_LOWER_NONE   = 0,
    OI_OBEX_LOWER_RFCOMM = OI_UUID_RFCOMM,
    OI_OBEX_LOWER_L2CAP  = OI_UUID_L2CAP,
    OI_OBEX_LOWER_TCPIP  = OI_UUID_TCP
} OI_OBEX_LOWER_PROTOCOL_ID;


/**
 * Options to be used for an OBEX connection
 */
typedef struct {
    OI_BOOL enableSRM;       /**< TRUE is single response mode is to be enabled for this connection */
} OI_OBEX_CONNECTION_OPTIONS;


/**
 * Tagged union for the different lower layer protocol service identifiers.
 */
typedef struct {
    OI_OBEX_LOWER_PROTOCOL_ID protocol; /**< The lower layer protocol for the OBEX service */
    union {
        OI_UINT8 rfcommChannel;         /**< The channel number if the lower protocol is RFCOMM */
        OI_UINT16 l2capPSM;             /**< The PSM if the lower protocol is L2CAP */
        void *ipAddress;                /**< The IP address and port is the lower protocol is TCP/IP */
    } svcId;
} OI_OBEX_LOWER_PROTOCOL;

#ifdef OI_DEBUG
/**
 * Returns a string describing the lower protocol for an OBEX server
 */
OI_CHAR* OI_OBEX_LowerProtocolTxt(OI_OBEX_LOWER_PROTOCOL *lowerProtocol);
#else
#define OI_OBEX_Lower_ProtocolTxt "lower-protocol"
#endif


#ifdef __cplusplus
}
#endif


/**@}*/

/*****************************************************************************/
#endif /* _OI_OBEX_H */
