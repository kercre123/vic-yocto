#ifndef _OI_OBEX_LOWER_H
#define _OI_OBEX_LOWER_H

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

#include "oi_bt_spec.h"
#include "oi_obex.h"
#include "oi_stddefs.h"
#include "oi_mbuf.h"
#include "oi_status.h"
#include "oi_connect_policy.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef struct _OI_OBEX_LOWER_SERVER *OI_OBEX_LOWER_SERVER;

typedef struct _OI_OBEX_LOWER_CONNECTION *OI_OBEX_LOWER_CONNECTION;


/*
 * OBEX APIs called by the lower layer interface
 */


/**
 * This function is called by the lower layer to confirm a connection.
 *
 * @param lowerConnection  The handle for this connection.
 *
 * @param recvMtu           Lower layer imposed on maximum OBEX packet we can receive.
 *
 * @param sendMtu           Lower layer imposed on maximum OBEX packet we can send.
 *
 * @param result
 */
typedef void (*OI_OBEX_LOWER_CONNECT_CFM)(OI_OBEX_LOWER_CONNECTION lowerConnection,
                                          OI_UINT16 recvMtu,
                                          OI_UINT16 sendMtu,
                                          OI_STATUS result);


/**
 * This function is called by the lower layer to indicate an incoming connect request.
 *
 * @param serverHandle      The server handle for the lower server.
 *
 * @param lowerConnection   The handle for this connection.
 *
 * @param addr              The Bluetooth device address of the remote device making the
 *                          connection.
 */
typedef void (*OI_OBEX_LOWER_CONNECT_IND)(OI_OBEX_LOWER_SERVER serverHandle,
                                          OI_OBEX_LOWER_CONNECTION lowerConnection,
                                          OI_BD_ADDR *addr);


/**
 * This function is called by the lower layer to indicate a disconnect
 *
 * @param lowerConnection  The handle for the connection that is going away.
 *
 * @param reason            Status code indicating the reason for the disconnection.
 */
typedef void (*OI_OBEX_LOWER_DISCONNECT_IND)(OI_OBEX_LOWER_CONNECTION lowerConnection,
                                             OI_STATUS reason);


/**
 * This function is called by the lower layer to confirm completion of a write.
 *
 * @param lowerConnection  The handle for the connection for which a write has completed.
 *
 * @param mbuf              Pointer to the MBUF that is no longer in use by the lower layer.
 *
 * @param queueFull         If FALSE another write can be issued, if TRUE, subsequent calls to write will fail.
 *
 * @param result            OI_OK if the write completed succesfully.
 *                          Other status codes indicate the reason that the write failed.
 */
typedef void (*OI_OBEX_LOWER_WRITE_CFM)(OI_OBEX_LOWER_CONNECTION lowerConnection,
                                        OI_MBUF *mbuf,
                                        OI_BOOL queueFull,
                                        OI_STATUS result);


/**
 * This function is called by the lower layer to indicate data has been received.
 *
 * @param lowerConnection  The handle for the connection on which data has been received.
 *
 * @param dataBuf           Pointer to a lower layer buffer containing the received data.
 *
 * @param dataLen           The length of the received data.
 */
typedef void (*OI_OBEX_LOWER_RECV_DATA_IND)(OI_OBEX_LOWER_CONNECTION lowerConnection,
                                            OI_BYTE *dataBuf,
                                            OI_UINT16 dataLen);



/**
 * Callback functions passed in by OBEX client or SERVER
 */
typedef struct {
    OI_OBEX_LOWER_CONNECT_CFM    connectCfm;
    OI_OBEX_LOWER_CONNECT_IND    connectInd;
    OI_OBEX_LOWER_DISCONNECT_IND disconnectInd;
    OI_OBEX_LOWER_WRITE_CFM      writeCfm;
    OI_OBEX_LOWER_RECV_DATA_IND  recvDataInd;
} OI_OBEX_LOWER_CALLBACKS;


/*
 * Lower layer abstract interface function prototypes
 */

typedef OI_STATUS (*OI_OBEX_LOWER_REGISTER_SERVER)(OI_OBEX_LOWER_SERVER serverHandle,
                                                   OI_UINT16 mtu,
                                                   const OI_CONNECT_POLICY *policy,
                                                   OI_OBEX_LOWER_PROTOCOL* lowerProtocol,
                                                   btsock_interface_t *socket_interface);

typedef OI_STATUS (*OI_OBEX_LOWER_DEREGISTER_SERVER)(OI_OBEX_LOWER_SERVER serverHandle);


typedef OI_STATUS (*OI_OBEX_LOWER_CONNECT)(OI_OBEX_LOWER_CONNECTION lowerConnection,
                                           OI_BD_ADDR *addr,
                                           OI_OBEX_LOWER_PROTOCOL *serverIdentifier,
                                           OI_UINT16 mtu,
                                           const OI_CONNECT_POLICY *policy,
                                           btsock_interface_t *socket_interface);


typedef OI_STATUS (*OI_OBEX_LOWER_ACCEPT)(OI_OBEX_LOWER_CONNECTION lowerConnection,
                                          OI_BOOL accept);


typedef OI_STATUS (*OI_OBEX_LOWER_DISCONNECT)(OI_OBEX_LOWER_CONNECTION lowerConnection);


/**
 * Called by OBEX to write data via the lower layer transport
 */
typedef OI_STATUS (*OI_OBEX_LOWER_WRITE)(OI_OBEX_LOWER_CONNECTION lowerConnection,
                                         OI_MBUF *mbuf,
                                         OI_BOOL accleratedCfm,
                                         OI_BOOL *queueFull);


/*
 * Called by OBEX to turn lower layer flow control on/off
 */
typedef void (*OI_OBEX_LOWER_FLOW_CONTROL)(OI_OBEX_LOWER_CONNECTION lowerConnection,
                                           OI_BOOL flow);


/**
 * Get the protocol id for the lower protocol
 */
typedef OI_OBEX_LOWER_PROTOCOL_ID (*OI_OBEX_LOWER_GET_PROTOCOL)(
                                        OI_OBEX_LOWER_CONNECTION lowerConnection);


/**
 * Get the L2CAP channel for the lower connection
 */
typedef OI_L2CAP_CID (*OI_OBEX_LOWER_GET_CID)(OI_OBEX_LOWER_CONNECTION lowerConnection);


/*
 * Abstract interface to a lower layer OBEX transport
 */
typedef struct {

    OI_OBEX_LOWER_REGISTER_SERVER   regServer;   /**< Registers the lower layer transport */
    OI_OBEX_LOWER_DEREGISTER_SERVER deregServer; /**< Deregisters the lower layer transport */
    OI_OBEX_LOWER_CONNECT           connect;     /**< Connects to a remote OBEX server over a lower layer transport */
    OI_OBEX_LOWER_ACCEPT            accept;      /**< Accepts an inbound connection from a remote OBEX server */
    OI_OBEX_LOWER_DISCONNECT        disconnect;  /**< Disconnects a lower layer transport */
    OI_OBEX_LOWER_WRITE             write;       /**< Writes data to a remote OBEX server over a lower layer transport */
    OI_OBEX_LOWER_GET_PROTOCOL      getProtocol; /**< Returns the protocol id for this lower interface */
    OI_OBEX_LOWER_FLOW_CONTROL      flowControl; /**< Turns lower layer flow control on/off */
    OI_OBEX_LOWER_GET_CID           getCID;      /**< Get the L2CAP CID for the lower protocol */

} OI_OBEX_LOWER_INTERFACE;


/*
 * Connection information that is specific and private to the lower layer.
 */
struct _LOWER_CONNECTION_PRIVATE;


struct _OI_OBEX_LOWER_CONNECTION {
    void *context;
    OI_OBEX_LOWER_SERVER server;              /* server handle if there is one */
    const OI_OBEX_LOWER_INTERFACE *ifc;
    const OI_OBEX_LOWER_CALLBACKS *callbacks;
    struct _LOWER_CONNECTION_PRIVATE *lowerPrivate;
    void *lowerHandle;
};


/*
 * Server information that is specific and private to the lower layer.
 */
struct _LOWER_SERVER_PRIVATE;


struct _OI_OBEX_LOWER_SERVER {
    void *context;
    const OI_OBEX_LOWER_INTERFACE *ifc;
    const OI_OBEX_LOWER_CALLBACKS *callbacks;
    const OI_CONNECT_POLICY *policy;
    OI_UINT16 mtu;
    OI_OBEX_LOWER_PROTOCOL lowerProtocol;
    struct _LOWER_SERVER_PRIVATE* lowerPrivate;
};



/**
 * Registers an OBEX server on a specified lower layer protocol. An OBEX server can be registered on
 * more than one lower layer protocol.
 */
OI_STATUS OI_OBEX_LOWER_RegisterServer(const OI_OBEX_LOWER_CALLBACKS *callbacks,
                                       OI_UINT16 mtu,
                                       const OI_CONNECT_POLICY *policy,
                                       OI_OBEX_LOWER_PROTOCOL *lowerProtocol,
                                       OI_OBEX_LOWER_SERVER *lowerServer);


/**
 * Establish a connection to a remote OBEX server using a specified lower layer protocol.
 */
OI_STATUS OI_OBEX_LOWER_Connect(const OI_OBEX_LOWER_CALLBACKS *callbacks,
                                OI_BD_ADDR *addr,
                                OI_OBEX_LOWER_PROTOCOL *lowerProtocol,
                                OI_UINT16 mtu,
                                const OI_CONNECT_POLICY *policy,
                                OI_OBEX_LOWER_CONNECTION *lowerConnection);



/**
 * Returns a pointer to the lower interface function table
 */
const OI_OBEX_LOWER_INTERFACE* OI_OBEX_LowerInterface(void);

/**
 * This function sets the socket interface exposed by stack for communicating with underlying
 * transport (RFCOMM or L2CAP).
 *
 * @param socket_interface  Socket Interface to be used.

 * @return      - Null
 */
void OI_OBEX_LOWER_SetSocketInterface(btsock_interface_t *socket_interface);
#ifdef __cplusplus
}
#endif

#endif /* _OI_OBEX_LOWER_H */
