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

#define __OI_MODULE__ OI_MODULE_OBEX_SRV

#include "oi_stddefs.h"
#include "oi_memmgr.h"
#include "oi_status.h"
#include "oi_connect_policy.h"
#include "oi_obex_lower.h"
#include "oi_obextest.h"
#include "oi_varstring.h"
#include "oi_argcheck.h"

static btsock_interface_t *sock_ifc;

#ifdef OI_DEBUG
OI_CHAR* OI_OBEX_LowerProtocolTxt(OI_OBEX_LOWER_PROTOCOL *lowerProtocol)
{
    static OI_CHAR txt[16];
    OI_VARSTRING vstr;

    vstr.Managed = FALSE;
    vstr.MaxLen = sizeof(txt) - 1;
    vstr.Len = 0;
    vstr.Buffer = txt;
    vstr.Overflow = FALSE;

    switch (lowerProtocol->protocol) {
        case OI_OBEX_LOWER_NONE:
            OI_FormatStr(&vstr, "NONE");
            break;
        case OI_OBEX_LOWER_RFCOMM:
            OI_FormatStr(&vstr, "RFCOMM channel=%d", lowerProtocol->svcId.rfcommChannel);
            break;
        case OI_OBEX_LOWER_L2CAP:
            OI_FormatStr(&vstr, "L2CAP psm=%d", lowerProtocol->svcId.l2capPSM);
            break;
        case OI_OBEX_LOWER_TCPIP:
            OI_FormatStr(&vstr, "TCPIP addr=%@", lowerProtocol->svcId.ipAddress, 8);
            break;
        default:
            OI_FormatStr(&vstr, "Unknown protocol %d", lowerProtocol->protocol);
            break;
    }
    return OI_VStrGetString(&vstr);

}
#endif


OI_STATUS OI_OBEX_LOWER_RegisterServer(const OI_OBEX_LOWER_CALLBACKS *callbacks,
                                       OI_UINT16 mtu,
                                       const OI_CONNECT_POLICY *policy,
                                       OI_OBEX_LOWER_PROTOCOL *lowerProtocol,
                                       OI_OBEX_LOWER_SERVER *lowerServer)
{
    OI_STATUS status;
    const OI_OBEX_LOWER_INTERFACE *ifc = NULL;
    OI_OBEX_LOWER_SERVER server;

    OI_ARGCHECK(callbacks);
    OI_ARGCHECK(lowerProtocol);
    OI_ARGCHECK(lowerServer);

    *lowerServer = NULL;

    switch (lowerProtocol->protocol) {
        case OI_OBEX_LOWER_RFCOMM:
        case OI_OBEX_LOWER_L2CAP:
            ifc = OI_OBEX_LowerInterface();
            break;
        case OI_OBEX_LOWER_TCPIP:
            // Not implemented.
            break;
        default:
            status = OI_STATUS_INVALID_PARAMETERS;
            OI_SLOG_ERROR(status, ("Invalid protocol %d", lowerProtocol->protocol));
            return status;
    }

    if (ifc == NULL) {
        return OI_STATUS_NOT_IMPLEMENTED;
    }

    server = OI_Calloc(sizeof(*server));
    if (server == NULL) {
        status = OI_STATUS_NO_RESOURCES;
    } else {
        status = ifc->regServer(server, mtu, policy, lowerProtocol, sock_ifc);
        if (OI_SUCCESS(status)) {
            server->callbacks = callbacks;
            server->ifc = ifc;
            *lowerServer = server;
            *lowerProtocol = server->lowerProtocol;
        } else {
            OI_Free(server);
        }
    }
    return status;
}



OI_STATUS OI_OBEX_LOWER_Connect(const OI_OBEX_LOWER_CALLBACKS *callbacks,
                                OI_BD_ADDR *addr,
                                OI_OBEX_LOWER_PROTOCOL *lowerProtocol,
                                OI_UINT16 mtu,
                                const OI_CONNECT_POLICY *policy,
                                OI_OBEX_LOWER_CONNECTION *lowerConnection)
{
    OI_STATUS status;
    const OI_OBEX_LOWER_INTERFACE *ifc;
    OI_OBEX_LOWER_CONNECTION connection;

    OI_ARGCHECK(callbacks);
    OI_ARGCHECK(addr);
    OI_ARGCHECK(lowerProtocol);
    OI_ARGCHECK(lowerConnection);

    *lowerConnection = NULL;

    switch (lowerProtocol->protocol) {
        case OI_OBEX_LOWER_RFCOMM:
        case OI_OBEX_LOWER_L2CAP:
            ifc = OI_OBEX_LowerInterface();
            break;
        case OI_OBEX_LOWER_TCPIP:
            return OI_STATUS_NOT_IMPLEMENTED;
        default:
            status = OI_STATUS_INVALID_PARAMETERS;
            OI_SLOG_ERROR(status, ("Invalid protocol %d", lowerProtocol->protocol));
            return status;
    }
    connection = OI_Calloc(sizeof(struct _OI_OBEX_LOWER_CONNECTION));
    if (connection == NULL) {
        status = OI_STATUS_NO_RESOURCES;
    } else {
        connection->callbacks = callbacks;
        connection->ifc = ifc;
        status = ifc->connect(connection, addr, lowerProtocol, mtu, policy, sock_ifc);
        if (OI_SUCCESS(status)) {
            *lowerConnection = connection;
        } else {
            OI_Free(connection);
        }
    }
    OI_LOG_ERROR(("%s returning %d", __func__, status));
    return status;
}

void OI_OBEX_LOWER_SetSocketInterface(btsock_interface_t *socket_interface)
{
    sock_ifc = socket_interface;
}
