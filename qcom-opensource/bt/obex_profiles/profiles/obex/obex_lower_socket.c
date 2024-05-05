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

#include "oi_obexspec.h"
#include "oi_obex_lower.h"
#include "oi_assert.h"
#include "oi_debug.h"
#include "oi_endian.h"
#include "oi_memmgr.h"
#include "oi_mbuf.h"
#include "oi_utils.h"
#include "oi_list.h"
#include "oi_eventloop.h"

#include <sys/socket.h>
#include <sys/un.h>
#include <unistd.h>
#include <errno.h>
#include <string.h>
#include <fcntl.h>
#include <errno.h>

typedef struct _SOCK_CONNECT_SIGNAL {
    OI_UINT16 size;
    OI_BD_ADDR bd_addr;
    int channel;
    int status;

    // The writer must make writes using a buffer of this maximum size
    // to avoid loosing data. (L2CAP only)
    OI_UINT16 max_tx_packet_size;

    // The reader must read using a buffer of at least this size to avoid
    // loosing data. (L2CAP only)
    OI_UINT16 max_rx_packet_size;
} __attribute__((packed)) SOCK_CONNECT_SIGNAL;

/*
 * (RFCOMM/L2CAP) state that is stored in OI_OBEX_LOWER_CONNECTION.lowerPrivate
 */
typedef struct _LOWER_CONNECTION_PRIVATE {
    btsock_interface_t *socket_interface;
    OI_OBEX_LOWER_CONNECTION handle;      /* Connection handle for making callbacks to upper OBEX */
    OI_LIST_ELEM links;    /* Linked list pointers */
    int socket;            /* Client connection socket */
    OI_BOOL connected;     /* Connection status */
    OI_CALLBACK_HANDLE disconnectCbHandle; /* Handle is set when there's a pending disconnect */
    OI_MBUF* writeMbuf;    /* Outgoing MBUF data */
    OI_BD_ADDR addr;       /* Address for remote device */
    OI_UINT16 channel;     /* transport (RFCOMM/L2CAP) channel for this connection */
    OI_UINT16 mtu;         /* Maximum OBEX packet length (read or write) */
    OI_UINT16 readLen;     /* The length of the reassembled OBEX packet */
    OI_UINT16 readPos;     /* The current reassembly position in the the packet */
    OI_UINT8* readBuffer;  /* Buffer for reassembling transport (RFCOMM/L2CAP) frames into a contiguous OBEX packet */
    OI_UINT16 writeLen;    /* The total length of the outgoing OBEX packet */
    OI_UINT16 writePos;    /* Where to begin the next write */
    OI_UINT8* writeBuffer; /* Buffer for staging write data for use with send() */
    OI_OBEX_LOWER_PROTOCOL_ID protocol;
    OI_BOOL clientSocket;
    SOCK_CONNECT_SIGNAL conn_params;
} LOWER_CONNECTION_PRIVATE;

typedef struct _LOWER_SERVER_PRIVATE {
    btsock_interface_t *socket_interface;
    OI_OBEX_LOWER_SERVER handle; /* Server handle for upper OBEX callbacks */
    OI_LIST_ELEM links;          /* Linked list pointers */
    int socket;                  /* Server socket (listening for connections) */
    OI_CALLBACK_HANDLE deregCbHandle; /* Handle is set when there's a pending disconnect */
    OI_OBEX_LOWER_PROTOCOL_ID protocol;
} LOWER_SERVER_PRIVATE;

/* List of all active server connections */
static OI_LIST(serverList);
/* List of all active client connections */
static OI_LIST(connectionList);

/*
 * Forward declarations.
 */
static OI_STATUS LowerDisconnect(OI_OBEX_LOWER_CONNECTION connectionHandle);
static void LowerTransportConnectInd(OI_OBEX_LOWER_SERVER serverHandle);
static OI_BOOL LowerTransportConnectCfm(OI_OBEX_LOWER_CONNECTION connectionHandle);
static OI_BOOL LowerTransportRecvDataInd(OI_OBEX_LOWER_CONNECTION connectionHandle);
static OI_BOOL LowerTransportSendMbuf(OI_OBEX_LOWER_CONNECTION connectionHandle);
static void LowerTransportDisconnectInd(OI_OBEX_LOWER_CONNECTION connectionHandle);
static void DeferredDeregServer(void* arg);



/*
 * Get fds for the event loop.  Active servers are always trying to
 * read (listening for incoming connections to accept).  Active
 * clients always listen for incoming data, and only set their write bit
 * when an MBUF is available to write.
 */
static int getLowerTransportFds(fd_set* readFds, fd_set* writeFds, fd_set* exceptFds, int* maxFd)
{
    OI_LIST_ELEM* elem;
    int numSet = 0;
    *maxFd = 0;

    OI_DBGPRINT(("getLowerTransportFds()"));
    /*
     * Connection fds are always readable, and writeable if there is
     * an MBUF waiting to be sent.  Sockets that are still connecting
     * become writeable when the connect procedure is complete.
     */
    OI_LIST_FOREACH(elem, &connectionList) {
        LOWER_CONNECTION_PRIVATE* lowerPrivate;
        lowerPrivate = OI_LIST_ENTRY(elem, LOWER_CONNECTION_PRIVATE, links);

        if (!lowerPrivate->connected) {
            OI_DBGPRINT(("Socket %d not connected, watching for write", lowerPrivate->socket));
            FD_SET(lowerPrivate->socket, writeFds);
            *maxFd = OI_MAX(*maxFd, lowerPrivate->socket);
            numSet++;
        }
        else {
            OI_DBGPRINT(("Socket %d watching for read", lowerPrivate->socket));
            FD_SET(lowerPrivate->socket, readFds);
            numSet++;
            *maxFd = OI_MAX(*maxFd, lowerPrivate->socket);

            if (NULL != lowerPrivate->writeMbuf) {
                OI_DBGPRINT(("Socket %d has pending write data", lowerPrivate->socket));
                FD_SET(lowerPrivate->socket, writeFds);
                numSet++;
                // No need to update *maxFd again
            }
        }
    }

    /*
     * Server fds become readable when an incoming connection is pending.
     */
    OI_LIST_FOREACH(elem, &serverList) {
        LOWER_SERVER_PRIVATE* lowerPrivate;
        lowerPrivate = OI_LIST_ENTRY(elem, LOWER_SERVER_PRIVATE, links);

        OI_DBGPRINT(("Server socket %d is listening for connections", lowerPrivate->socket));
        FD_SET(lowerPrivate->socket, readFds);
        numSet++;
        *maxFd = OI_MAX(*maxFd, lowerPrivate->socket);
    }

    OI_DBGPRINT(("Watching for %d fds", numSet));

    return numSet;
}

/*
 * Handle socket activity for servers and clients.
 */
static int handleLowerTransportEvents(int numSet, fd_set* readFds, fd_set* writeFds, fd_set* exceptFds)
{
    OI_LIST_ELEM* elem;
    OI_LIST_ELEM marker;

    OI_DBGPRINT(("handleLowerTransportEvents(numSet=%d)", numSet));

    // Process activity on data sockets
    OI_List_DynamicInit(&marker);
    OI_List_AddTail(&marker, &connectionList);
    while ((elem = OI_List_RemoveHead(&connectionList)) != &marker) {
        LOWER_CONNECTION_PRIVATE* lowerPrivate;

        // Rotate item to end of list.
        OI_List_AddTail(elem, &connectionList);

        if (numSet < 1) {
            OI_DBGPRINT(("No more FDs"));
            OI_List_Del(&marker);
            elem = &marker;
            break;
        }

        lowerPrivate = OI_LIST_ENTRY(elem, LOWER_CONNECTION_PRIVATE, links);
        if (lowerPrivate == NULL) {
            continue;
        }

        OI_DBGPRINT(("Checking socket %d", lowerPrivate->socket));

        if (FD_ISSET(lowerPrivate->socket, readFds)) {
            OI_DBGPRINT(("Socket %d has new data to read", lowerPrivate->socket));
            FD_CLR(lowerPrivate->socket, readFds);
            numSet--;
            if (!LowerTransportRecvDataInd(lowerPrivate->handle)) {
                OI_DBGPRINT(("Connection invalidated"));
                continue;
            }
        }
        if (FD_ISSET(lowerPrivate->socket, writeFds)) {
            FD_CLR(lowerPrivate->socket, writeFds);
            numSet--;
            if (lowerPrivate->connected) {
                OI_DBGPRINT(("Socket %d has write buffer space available", lowerPrivate->socket));
                if (!LowerTransportSendMbuf(lowerPrivate->handle)) {
                    /* In the current control structure, this is a
                     * no-op.  It is included to draw attention to
                     * the meaning of LowerTransportSendMbuf()'s return val
                     */
                    OI_DBGTRACE(("Connection invalidated"));
                    continue;
                }
            }
            else {
                OI_DBGPRINT(("Socket %d has a ready connection", lowerPrivate->socket));
                if (!LowerTransportConnectCfm(lowerPrivate->handle)) {
                    /* In the current control structure, this is a
                     * no-op.  It is included to draw attention to
                     * the meaning of LowerTransportConnectCfm()'s return val
                     */
                    OI_DBGTRACE(("Connection invalidated"));
                    continue;
                }
            }
        }
    }

    /*
     * ! Important - 'marker' is on the stack and cannot be in the
     * queue when the above while loop exits!
     */
    OI_ASSERT(elem == &marker);

    // Look for incoming connections or errors from server sockets.
    OI_List_DynamicInit(&marker);
    OI_List_AddTail(&marker, &serverList);
    while ((elem = OI_List_RemoveHead(&serverList)) != &marker) {
        LOWER_SERVER_PRIVATE* lowerPrivate;

        // Rotate item to end of list.
        OI_List_AddTail(elem, &serverList);

        if (numSet < 1) {
            OI_DBGPRINT(("No more FDs"));
            OI_List_Del(&marker);
            elem = &marker;
            break;
        }

        lowerPrivate = OI_LIST_ENTRY(elem, LOWER_SERVER_PRIVATE, links);
        if (lowerPrivate == NULL) {
            continue;
        }

        OI_DBGPRINT(("Checking server socket %d", lowerPrivate->socket));

        if (FD_ISSET(lowerPrivate->socket, readFds)) {
            OI_DBGPRINT(("Server socket %d has an incoming connection", lowerPrivate->socket));
            FD_CLR(lowerPrivate->socket, readFds);
            numSet--;
            LowerTransportConnectInd(lowerPrivate->handle);
        }
    }

    /*
     * ! Important - 'marker' is on the stack and cannot be in the
     * queue when the above while loop exits!
     */
    OI_ASSERT(elem == &marker);

    OI_DBGPRINT(("%d fds remaining", numSet));
    return numSet;
}

static void validateLowerTransportFds(void)
{
    OI_LIST_ELEM* elem;

    /*
     * Look through all server and client fds.  If any are invalid, clean
     * them up.
     */

    OI_LIST_FOREACH(elem, &connectionList) {
        LOWER_CONNECTION_PRIVATE* lowerPrivate;
        lowerPrivate = OI_LIST_ENTRY(elem, LOWER_CONNECTION_PRIVATE, links);

        if (!OI_EVENTLOOP_FdIsValid(lowerPrivate->socket)) {
            LowerDisconnect(lowerPrivate->handle);
        }
    }

    OI_LIST_FOREACH(elem, &serverList) {
        LOWER_SERVER_PRIVATE* lowerPrivate;
        lowerPrivate = OI_LIST_ENTRY(elem, LOWER_SERVER_PRIVATE, links);

        if (!OI_EVENTLOOP_FdIsValid(lowerPrivate->socket)) {
            OI_ScheduleCallbackFunction(DeferredDeregServer,
                                        (void*) lowerPrivate->handle,
                                        0,
                                        &lowerPrivate->deregCbHandle);
        }
    }
}

// Utility functions for handling callback registration w/ OI_EVENTLOOP
static void updateEventLoopRegistration(void)
{
    if (OI_List_IsEmpty(&connectionList) &&
        OI_List_IsEmpty(&serverList)) {
        OI_DBGPRINT(("Adding event handlers"));
        OI_EVENTLOOP_Register(getLowerTransportFds, handleLowerTransportEvents, validateLowerTransportFds, TRUE);
    }
}

static void updateEventLoopDeregistration(void)
{
    if (OI_List_IsEmpty(&connectionList) &&
        OI_List_IsEmpty(&serverList)) {
        OI_DBGPRINT(("Removing event handlers"));
        OI_EVENTLOOP_Deregister(getLowerTransportFds, handleLowerTransportEvents, validateLowerTransportFds);
    }
}

/*
 * Memory allocation and initialization for LOWER_CONNECTION_PRIVATE
 */
static LOWER_CONNECTION_PRIVATE* allocLowerConnectionPrivate(OI_UINT16 mtu)
{
    LOWER_CONNECTION_PRIVATE* lowerPrivate;

    lowerPrivate = OI_Calloc(sizeof(LOWER_CONNECTION_PRIVATE));
    if (NULL == lowerPrivate) {
        return NULL;
    }

    // Set up buffer pointers.
    lowerPrivate->readBuffer = OI_Malloc(mtu);
    lowerPrivate->writeBuffer = OI_Malloc(OI_UINT16_MAX);
    if ((NULL == lowerPrivate->readBuffer) ||
        (NULL == lowerPrivate->writeBuffer)) {
        OI_FreeIf(&lowerPrivate->readBuffer);
        OI_FreeIf(&lowerPrivate->writeBuffer);
        OI_Free(lowerPrivate);
        return NULL;
    }

    OI_List_DynamicInit(&lowerPrivate->links);
    lowerPrivate->mtu = mtu;

    // Other struct members are initialized to 0 or NULL by OI_Calloc()

    return lowerPrivate;
}

/*
 * Convenient freeing for LOWER_CONNECTION_PRIVATE
 */
static void deallocLowerConnectionPrivate(OI_OBEX_LOWER_CONNECTION handle)
{
    if (NULL != handle->lowerPrivate) {
        OI_DBGTRACE(("Deallocating private data at %08x", handle->lowerPrivate));

        if (0 != handle->lowerPrivate->disconnectCbHandle) {
            OI_CancelCallbackFunction(handle->lowerPrivate->disconnectCbHandle);
        }

        OI_FreeIf(&handle->lowerPrivate->readBuffer);
        OI_FreeIf(&handle->lowerPrivate->writeBuffer);
        OI_FreeIf(&handle->lowerPrivate);
    }
}

static int readAll(int sock_fd, unsigned char *b, int len)
{
    OI_DBGTRACE(("readAll: about to read %d bytes", len));
    int left = len;
    while(left > 0) {
        int ret = recv(sock_fd, b + (len - left), left, MSG_WAITALL);
        if(ret <= 0) {
            OI_DBGTRACE(("read failed, socket might closed or timeout, read ret %d: ",
                         ret));
            return ret;
        }
        left -= ret;
        if(left != 0)
            OI_DBGTRACE(("readAll() looping, read partial size: %d, expect size: %d" ,
                        (len- left) , len));
    }
    return len;
}

static int readInt(int sock_fd, unsigned char *b)
{
    if (sizeof(int) == readAll(sock_fd, b, sizeof(int))) {
        return (int)GetUINT32_LittleEndian(b);
    }
    return 0;
}

static int readMsg(int sock_fd, unsigned char *b, int len, int *new_fd)
{
    struct iovec iov[1];
    union {
        struct cmsghdr cm;
        char control[CMSG_SPACE(sizeof(int))];
    } control_un;
    struct msghdr msg = { NULL, 0, iov, 1, control_un.control,
                          sizeof(control_un), 0 };
    struct cmsghdr *cmsg = &control_un.cm;

    OI_DBGTRACE(("readMsg: about to read %d bytes from fd = %d", len, sock_fd));

    iov[0].iov_base = b;
    iov[0].iov_len = len;

    int ret = recvmsg(sock_fd, &msg, MSG_WAITALL);
    if(ret <= 0) {
        OI_DBGTRACE(("recvmsg failed, socket might closed or timeout, read ret %d: %s",
                     ret, strerror(errno)));
        return ret;
    }
    OI_DBGTRACE(("recvmsg received ret %d", ret));
    cmsg = CMSG_FIRSTHDR(&msg);
    while (cmsg) {
      if (cmsg->cmsg_level == SOL_SOCKET &&
         cmsg->cmsg_type == SCM_RIGHTS &&
         cmsg->cmsg_len == CMSG_LEN(sizeof(int))) {
         *new_fd = *((int *) CMSG_DATA(cmsg));
         break;
      }

      cmsg = CMSG_NXTHDR(&msg, cmsg);
    }
    return ret;
}

static void waitForConnectSignal(int sock_fd, SOCK_CONNECT_SIGNAL *conn_params,
    int *new_fd)
{
    unsigned char read_bytes[20];
    int ret;
    OI_DBGTRACE(("waitForConnectSignal fd:%d", sock_fd));
    /* Set status as error by default */
    conn_params->status = -1;
    if (new_fd == NULL) {
        ret = readAll(sock_fd, read_bytes, sizeof(*conn_params));
    } else {
        ret = readMsg(sock_fd, read_bytes, sizeof(*conn_params), new_fd);
    }
    if (ret == sizeof(*conn_params)) {
        conn_params->size = GetUINT16_LittleEndian(&read_bytes[0]);
        OI_MemCopy(&conn_params->bd_addr.addr[0], &read_bytes[2], sizeof(OI_BD_ADDR));
        conn_params->channel = GetUINT32_LittleEndian(&read_bytes[8]);
        conn_params->status = GetUINT32_LittleEndian(&read_bytes[12]);
        conn_params->max_rx_packet_size = GetUINT16_LittleEndian(&read_bytes[16]);
        conn_params->max_tx_packet_size = GetUINT16_LittleEndian(&read_bytes[18]);
        OI_DBGTRACE(("status = %d recvMtu = %d sendMtu = %d ", conn_params->status,
            conn_params->max_rx_packet_size, conn_params->max_tx_packet_size));
    }
}

static bool HandleLowerConnectCfmFailure(OI_OBEX_LOWER_CONNECTION connectionHandle)
{
    OI_DBGTRACE(("Lower Connect Failed"));
    /*
     * Remove the socket from the active list, and close the
     * socket, and let the upper layer know that the connect
     * failed.
     */
    OI_List_Del(&connectionHandle->lowerPrivate->links);
    updateEventLoopDeregistration();

    // Waking up the event loop to ensure that select is not blocking pre-close (BlueZ issue?).
    OI_EVENTLOOP_Wakeup();
    close(connectionHandle->lowerPrivate->socket);

    deallocLowerConnectionPrivate(connectionHandle);
    //TODO: Map the error from FD stack to BM3 Obex
    connectionHandle->callbacks->connectCfm(connectionHandle, 0, 0,
        OI_STATUS_INTERNAL_ERROR);
    OI_Free(connectionHandle);
    return FALSE;
}

/*
 * Socket-based analog for BM3 Lower transport (RFCOMM/L2CAP) connect confirm.
 * Should be called when a socket becomes writeable after a non-blocking
 * connect().
 *
 * Returns FALSE if there was an error on the connection -- and when
 * FALSE is returned, the caller must assume the connection handle
 * passed in to this function is no longer valid (it may have been
 * deallocated before this function returned).
 */
static OI_BOOL LowerTransportConnectCfm(OI_OBEX_LOWER_CONNECTION connectionHandle)
{
    LOWER_CONNECTION_PRIVATE* lowerPrivate;
    OI_STATUS status = OI_OK;
    OI_BOOL connectionOk = TRUE;

    OI_DBGTRACE(("LowerTransportConnectCfm(%08x)", connectionHandle));

    OI_ASSERT(NULL != connectionHandle);
    OI_ASSERT(!connectionHandle->lowerPrivate->connected);

    lowerPrivate = connectionHandle->lowerPrivate;

    if (connectionHandle->lowerPrivate->clientSocket) {
        unsigned char read_bytes[sizeof(int)];
        int channel = readInt(connectionHandle->lowerPrivate->socket, read_bytes);
        OI_DBGTRACE(("Channel = 0x%04x", channel));
        if (channel <= 0) {
            status = OI_STATUS_NOT_CONNECTED;
            connectionOk = HandleLowerConnectCfmFailure(connectionHandle);
            return connectionOk;
        }
        waitForConnectSignal(connectionHandle->lowerPrivate->socket,
            &connectionHandle->lowerPrivate->conn_params, NULL);
    }
    if (!connectionHandle->lowerPrivate->conn_params.status) {
        OI_DBGTRACE(("Lower Connect Successful"));
        lowerPrivate->connected = TRUE;
        /*
         * RFCOMM is a stream protocol so imposes no inherent limit on
         * the size of the OBEX packets.
         */
        if (OI_OBEX_LOWER_RFCOMM == connectionHandle->lowerPrivate->protocol)
            connectionHandle->callbacks->connectCfm(connectionHandle,
                OI_UINT16_MAX,
                OI_UINT16_MAX,
                OI_OK);
        else {
            connectionHandle->callbacks->connectCfm(connectionHandle,
                connectionHandle->lowerPrivate->conn_params.max_rx_packet_size,
                connectionHandle->lowerPrivate->conn_params.max_tx_packet_size,
                OI_OK);
        }
    } else {
        connectionOk = HandleLowerConnectCfmFailure(connectionHandle);
    }

    return connectionOk;
}

/*
 * Socket-based analog for BM3 Lower transport (RFCOMM/L2CAP) connect indication.
 * Should be called when a server socket becomes readable.
 */
static void LowerTransportConnectInd(OI_OBEX_LOWER_SERVER serverHandle)
{
    OI_OBEX_LOWER_CONNECTION connectionHandle;
    int remoteSocket;
    OI_BD_ADDR bdaddr;
    struct sockaddr_un cliaddr;
    int length;

    OI_DBGTRACE(("LowerTransportConnectInd(%08x)", serverHandle));

    OI_ASSERT(serverHandle);

    /*
     * Allocate a connection handle - disconnect if the allocation
     * fails.  Populate all relevant fields for a connection.  The
     * connection is not complete until it is confirmed by the upper
     * layer.
     */
    connectionHandle = OI_Calloc(sizeof(*connectionHandle));
    if (!connectionHandle) {
        // Waking up the event loop to ensure that select is not blocking pre-close (BlueZ issue?).
        OI_EVENTLOOP_Wakeup();
        close(serverHandle->lowerPrivate->socket);
        OI_SLOG_ERROR(OI_STATUS_OUT_OF_MEMORY, ("Failed to allocate connection handle"));
        return;
    }

    connectionHandle->lowerPrivate = allocLowerConnectionPrivate(serverHandle->mtu);

    if (NULL == connectionHandle->lowerPrivate) {
        // Waking up the event loop to ensure that select is not blocking pre-close (BlueZ issue?).
        OI_EVENTLOOP_Wakeup();
        close(serverHandle->lowerPrivate->socket);
        OI_Free(connectionHandle);
        OI_SLOG_ERROR(OI_STATUS_OUT_OF_MEMORY, ("Failed to allocate connection handle"));
        return;
    }

    waitForConnectSignal(serverHandle->lowerPrivate->socket,
        &connectionHandle->lowerPrivate->conn_params, &remoteSocket);
    if (!connectionHandle->lowerPrivate->conn_params.status) {
        connectionHandle->lowerPrivate->clientSocket = FALSE;
        connectionHandle->lowerPrivate->handle = connectionHandle;
        connectionHandle->lowerPrivate->addr = connectionHandle->lowerPrivate->conn_params.bd_addr;
        connectionHandle->lowerPrivate->channel = connectionHandle->lowerPrivate->conn_params.channel;
        connectionHandle->lowerPrivate->socket = remoteSocket;
        if ((connectionHandle->lowerPrivate->conn_params.max_rx_packet_size ==
            connectionHandle->lowerPrivate->conn_params.max_tx_packet_size) &&
            (connectionHandle->lowerPrivate->conn_params.max_rx_packet_size == 0))
            connectionHandle->lowerPrivate->protocol = OI_OBEX_LOWER_RFCOMM;
        else
            connectionHandle->lowerPrivate->protocol = OI_OBEX_LOWER_L2CAP;
        connectionHandle->ifc = serverHandle->ifc;
        connectionHandle->callbacks = serverHandle->callbacks;
        /*
         * Indicate the connection to OBEX
         */
        connectionHandle->callbacks->connectInd(serverHandle, connectionHandle,
            &connectionHandle->lowerPrivate->addr);

        /*
         * The connection will be added to the active list if it is
         * accepted by the app.
         */
    } else {
        // Waking up the event loop to ensure that select is not blocking pre-close (BlueZ issue?).
        OI_EVENTLOOP_Wakeup();
        close(serverHandle->lowerPrivate->socket);
        OI_Free(connectionHandle);
        OI_SLOG_ERROR(OI_STATUS_INTERNAL_ERROR, ("Connection status failed"));
        return;
    }

}

/*
 * Socket-based analog for BM3 Lower transport (RFCOMM/L2CAP) disconnect indication.  This is
 * responsible for both propagating the disconnect indication upward,
 * and for actually closing the socket & freeing resources.
 */
static void LowerTransportDisconnectInd(OI_OBEX_LOWER_CONNECTION connectionHandle)
{
    OI_CALLBACK_HANDLE deferredDisconnect;

    OI_DBGTRACE(("LowerTransportDisconnectInd(%08x)", connectionHandle));

    deferredDisconnect = connectionHandle->lowerPrivate->disconnectCbHandle;
    if (0 != deferredDisconnect) {
        OI_DBGTRACE(("Cancelling deferred disconnect"));
        OI_CancelCallbackFunction(deferredDisconnect);
    }

    /*
     * Remove the socket from the active list, and trigger a disconnect.
     */
    OI_List_Del(&connectionHandle->lowerPrivate->links);
    updateEventLoopDeregistration();

    if (connectionHandle->lowerPrivate->connected) {
        OI_DBGTRACE(("LowerTransportDisconnectInd: closing socket %d.", connectionHandle->lowerPrivate->socket));

        // Waking up the event loop to ensure that select is not blocking pre-close (BlueZ issue?).
        OI_EVENTLOOP_Wakeup();
        int closeRet = close(connectionHandle->lowerPrivate->socket);
        if (closeRet) {
            OI_DBGTRACE(("Could not close socket. Error info: ", strerror(errno)));
        }
    }

    connectionHandle->callbacks->disconnectInd(connectionHandle, OI_RFCOMM_LOCAL_DEVICE_DISCONNECTED);

    /*
     * Free the private connection data, and the handle itself.
     */
    deallocLowerConnectionPrivate(connectionHandle);
    OI_Free(connectionHandle);
}

/*
 * Propagate a disconnect indication from the dispatcher.
 */
static void DeferredLowerTransportDisconnectInd(void* arg) {
    OI_OBEX_LOWER_CONNECTION connection = (OI_OBEX_LOWER_CONNECTION) arg;

    connection->lowerPrivate->disconnectCbHandle = 0;

    LowerTransportDisconnectInd(connection);
}

/*
 * Socket-based analog for the BM3 Lower transport (RFCOMM/L2CAP) receive data indication.
 * This is responsible for reassembly of the OBEX packet, and only
 * propagates data upward when a complete packet is available.
 *
 * Returns FALSE if there was an error on the connection -- and when
 * FALSE is returned, the caller must assume the connection handle
 * passed in to this function is no longer valid (it may have been
 * deallocated before this function returned).
 */
static OI_BOOL LowerTransportRecvDataInd(OI_OBEX_LOWER_CONNECTION connectionHandle)
{
    OI_STATUS status = OI_OK;
    ssize_t recvBytes = 0;
    LOWER_CONNECTION_PRIVATE* lowerPrivate;
    int readLen;

    OI_DBGTRACE(("LowerTransportRecvDataInd(%08x)", connectionHandle));

    if (connectionHandle == NULL) {
        OI_SLOG_ERROR(OI_STATUS_INVALID_HANDLE, ("OBEX received data for unknown connection"));
        return FALSE;
    }

    lowerPrivate = connectionHandle->lowerPrivate;

    OI_DBGPRINT(("readPos = %d, readLen = %d",
                  lowerPrivate->readPos,
                  lowerPrivate->readLen));

    if (lowerPrivate->readLen == 0) {
        /*
         * Robustness testers may send tiny frames so we need to allow
         * for the possibility that we don't have enough of the header
         * to extract the packet length.
         */

        if (OI_OBEX_LOWER_RFCOMM == connectionHandle->lowerPrivate->protocol) {
            /*
             * RFCOMM sockets seem to have quirky behavior when reading.
             * If we just read the maximum buffer length, recv() will
             * block a long time on shorter packets and throughput will
             * suffer.  As a workaround, read a very small number of bytes
             * first - enough to determine the actual packet size - then
             * read the expected number of bytes remaining.
             */
            readLen = OI_OBEX_SMALLEST_PKT;
        } else {
            readLen = connectionHandle->lowerPrivate->conn_params.max_rx_packet_size;
        }
        recvBytes = recv(connectionHandle->lowerPrivate->socket,
                         lowerPrivate->readBuffer + lowerPrivate->readPos,
                         readLen - lowerPrivate->readPos,
                         MSG_DONTWAIT);

        if (recvBytes > 0) {
            lowerPrivate->readPos += recvBytes;
        }
        else {
            status = OI_STATUS_READ_ERROR;
            OI_SLOG_ERROR(status, ("Error %d reading socket %d",
                                   errno,
                                   connectionHandle->lowerPrivate->socket));
            // strerror() isn't thread safe, so let's not use it in release mode
            OI_DBGTRACE(("Error info: ", strerror(errno)));
            LowerTransportDisconnectInd(connectionHandle);
            return FALSE;
        }

        if (lowerPrivate->readPos < OI_OBEX_SMALLEST_PKT) {
            /*
             * recv() didn't get enough data to determine the packet
             * size, so return to the event loop.
             */
            OI_DBGTRACE(("Not enough data to get OBEX packet length"));
            return TRUE;
        }

        /*
         * Extract the packet length from the packet header.
         */
        lowerPrivate->readLen = GetUINT16_BigEndian(lowerPrivate->readBuffer +
                                                    sizeof(OI_UINT8));
        OI_DBGTRACE(("OBEX packet length is %d", lowerPrivate->readLen));

        /*
         * Check that the packet will fit our mtu
         */
        if (lowerPrivate->readLen > lowerPrivate->mtu) {
            status = OI_STATUS_MTU_EXCEEDED;
            OI_SLOG_ERROR(status, ("OBEX packet exceeds MTU - disconnecting"));
            LowerTransportDisconnectInd(connectionHandle);
            return FALSE;
        }
    }

    OI_DBGPRINT(("readPos = %d, readLen = %d",
                  lowerPrivate->readPos,
                  lowerPrivate->readLen));
    /*
     * Try to read the remainder of the packet if it's not already done.
     */
    if (lowerPrivate->readPos != lowerPrivate->readLen) {
        recvBytes = recv(connectionHandle->lowerPrivate->socket,
                         lowerPrivate->readBuffer + lowerPrivate->readPos,
                         lowerPrivate->readLen - lowerPrivate->readPos,
                         MSG_WAITALL);

        if (recvBytes > 0) {
            lowerPrivate->readPos += recvBytes;
        }
        else {
            status = OI_STATUS_READ_ERROR;
            OI_SLOG_ERROR(status, ("Error %d reading socket %d",
                                   errno,
                                   connectionHandle->lowerPrivate->socket));
            OI_DBGTRACE(("Error info: ", strerror(errno)));
            LowerTransportDisconnectInd(connectionHandle);
            return FALSE;
        }
    }


    OI_DBGPRINT(("readPos = %d, readLen = %d",
                  lowerPrivate->readPos,
                  lowerPrivate->readLen));

    /*
     * The OBEX packet is complete, pass it up the stack.
     */
    if (lowerPrivate->readPos == lowerPrivate->readLen) {
        OI_UINT16 readLen = lowerPrivate->readLen;

        /*
         * Clear readLen and readPos in preparation for next OBEX packet.
         */
        lowerPrivate->readPos = 0;
        lowerPrivate->readLen = 0;

        OI_DBGTRACE(("Passing %d new bytes to the upper layer", readLen));

        connectionHandle->callbacks->recvDataInd(connectionHandle,
                                                 lowerPrivate->readBuffer,
                                                 readLen);
    }

    return TRUE;
}

/*
 * Create a new server instance.
 */
static OI_STATUS LowerRegServer(OI_OBEX_LOWER_SERVER serverHandle,
                                OI_UINT16 mtu,
                                const OI_CONNECT_POLICY* policy,
                                OI_OBEX_LOWER_PROTOCOL* lowerProtocol,
                                btsock_interface_t *socket_interface)
{
    OI_UINT16 channel;
    OI_STATUS status = OI_OK;
    bt_status_t connectStatus = BT_STATUS_FAIL;
    btsock_type_t sock_type;

    OI_DBGTRACE(("LowerRegServer(%d)", lowerProtocol->protocol));

    // Set up private data.
    serverHandle->lowerPrivate = OI_Calloc(sizeof(LOWER_SERVER_PRIVATE));
    if (NULL != serverHandle->lowerPrivate) {

        serverHandle->mtu = mtu;
        if (lowerProtocol->protocol == OI_OBEX_LOWER_RFCOMM) {
            OI_DBGTRACE(("Listening on RFCOMM"));
            channel = lowerProtocol->svcId.rfcommChannel;
            sock_type = BTSOCK_RFCOMM;
        } else {
            OI_DBGTRACE(("Listening on L2CAP"));
            sock_type = BTSOCK_L2CAP;
            channel = lowerProtocol->svcId.l2capPSM;
        }
    }
    else {
        return OI_STATUS_OUT_OF_MEMORY;
    }

    int securityFlags = 0;
    OI_DATAELEM oppUuid = OI_ELEMENT_UUID32(OI_UUID_OBEXObjectPush);
    if ((NULL != policy) && !OI_DATAELEM_SameUUID(&policy->serviceUuid, &oppUuid)) {
        /* Set higher security for non-OPP servers */
        securityFlags = BTSOCK_FLAG_ENCRYPT | BTSOCK_FLAG_AUTH;
    }
    OI_DBGTRACE(("Listening on %s socket", OI_UUIDDataelemText(&policy->serviceUuid)));
    //TODO: check and update last argument of below call.
    //Added 0 to compile
    if (OI_SUCCESS(status) && socket_interface) {
        connectStatus = socket_interface->listen(sock_type,
                OI_UUIDDataelemText(&policy->serviceUuid),
                NULL,
                channel,
                &serverHandle->lowerPrivate->socket,
                securityFlags, 0);
    }

    if (BT_STATUS_SUCCESS == connectStatus) {
        serverHandle->lowerPrivate->handle = serverHandle;
        serverHandle->lowerPrivate->socket_interface = socket_interface;
        serverHandle->policy = policy;
        OI_DBGTRACE(("Waiting for channel on %d socket", serverHandle->lowerPrivate->socket));
        /* Wait for Connect Ind from Lower Layer */
        unsigned char read_bytes[sizeof(int)];
        int channel = readInt(serverHandle->lowerPrivate->socket, read_bytes);
        OI_DBGTRACE(("Channel = 0x%04x", channel));
        if (channel <= 0) {
            // Waking up the event loop to ensure that select is not blocking pre-close
            OI_EVENTLOOP_Wakeup();
            close(serverHandle->lowerPrivate->socket);
            status = OI_STATUS_INTERNAL_ERROR;
        } else {
            updateEventLoopRegistration();
            OI_List_Add(&serverHandle->lowerPrivate->links, &serverList);
            // Force an iteration of the eventloop so getLowerTransportFds() is called
            OI_EVENTLOOP_Wakeup();
        }
    } else {
        // Waking up the event loop to ensure that select is not blocking pre-close
        OI_EVENTLOOP_Wakeup();
        close(serverHandle->lowerPrivate->socket);
        status = OI_STATUS_INTERNAL_ERROR;
    }

    return status;
}

/*
 * Remove a server instance.
 */
static OI_STATUS LowerDeregServer(OI_OBEX_LOWER_SERVER serverHandle)
{
    OI_CALLBACK_HANDLE deferredDereg;

    OI_DBGTRACE(("LowerDeregServer(%08x)", serverHandle));

    OI_ASSERT(serverHandle->lowerPrivate);

    deferredDereg = serverHandle->lowerPrivate->deregCbHandle;
    if (0 != deferredDereg) {
        OI_DBGTRACE(("Cancelling deferred deregistration"));
        OI_CancelCallbackFunction(deferredDereg);
    }

    OI_List_Del(&serverHandle->lowerPrivate->links);
    updateEventLoopDeregistration();

    // Waking up the event loop to ensure that select is not blocking pre-close (BlueZ issue?).
    OI_EVENTLOOP_Wakeup();
    close(serverHandle->lowerPrivate->socket);
    OI_Free(serverHandle->lowerPrivate);
    OI_Free(serverHandle);

    return OI_STATUS_SUCCESS;
}

/*
 * Propagate a disconnect indication from the dispatcher.
 */
static void DeferredDeregServer(void* arg) {
    OI_OBEX_LOWER_SERVER server = (OI_OBEX_LOWER_SERVER) arg;

    server->lowerPrivate->deregCbHandle = 0;

    LowerDeregServer(server);
}

/*
 * Connect to a remote OBEX server.
 */
static OI_STATUS LowerConnect(OI_OBEX_LOWER_CONNECTION connectionHandle,
                              OI_BD_ADDR *addr,
                              OI_OBEX_LOWER_PROTOCOL *lowerProtocol,
                              OI_UINT16 mtu,
                              const OI_CONNECT_POLICY *policy,
                              btsock_interface_t *socket_interface)
{
    bt_status_t connectStatus = BT_STATUS_FAIL;
    OI_STATUS status = OI_OK;

    OI_DBGTRACE(("LowerConnect(%08x, %d)", connectionHandle, mtu));

    if (mtu < OI_OBEX_SMALLEST_PKT) {
        // LowerTransportRecvDataInd() relies on an mtu >= OI_OBEX_SMALLEST_PACKET
        return OI_OBEX_VALUE_NOT_ACCEPTABLE;
    }

    connectionHandle->lowerPrivate = allocLowerConnectionPrivate(mtu);

    if (NULL == connectionHandle->lowerPrivate) {
        status = OI_STATUS_OUT_OF_MEMORY;
        OI_SLOG_ERROR(status, ("Failed to allocate connection handle"));
        return status;
    }

    btsock_type_t sock_type;
    int securityFlags = 0;
    OI_DATAELEM oppUuid = OI_ELEMENT_UUID32(OI_UUID_OBEXObjectPush);
    if ((NULL != policy) && !OI_DATAELEM_SameUUID(&policy->serviceUuid, &oppUuid)) {
        /* Set higher security for non-OPP servers */
        securityFlags = BTSOCK_FLAG_ENCRYPT | BTSOCK_FLAG_AUTH;
    }
    if (lowerProtocol->protocol == OI_OBEX_LOWER_RFCOMM) {
        OI_DBGTRACE(("Connecting RFCOMM"));
        sock_type = BTSOCK_RFCOMM;
        connectionHandle->lowerPrivate->channel = lowerProtocol->svcId.rfcommChannel;
    } else {
        OI_DBGTRACE(("Connecting L2CAP"));
        sock_type = BTSOCK_L2CAP;
        connectionHandle->lowerPrivate->channel = lowerProtocol->svcId.l2capPSM;
    }
    connectionHandle->lowerPrivate->handle = connectionHandle;
    connectionHandle->lowerPrivate->addr = *addr;
    connectionHandle->lowerPrivate->socket_interface = socket_interface;
    connectionHandle->lowerPrivate->protocol = lowerProtocol->protocol;

    /*
     * Do a nonblocking connect to the nonblocking socket.  The
     * connection may not be complete when connect() returns, but the
     * socket will not become writeable until the connection is done.
     * When a completed connect is detected in handleLowerTransportEvents(),
     * lowerPrivate->connect will be updated.
     */

    //TODO: check and update last argument of below call.
    //Added 0 to compile
    if (socket_interface) {
        connectStatus = socket_interface->connect(addr, sock_type,
                NULL, /* UUID is set to NULL as channel has required info */
                connectionHandle->lowerPrivate->channel,
                &connectionHandle->lowerPrivate->socket,
                securityFlags, 0);
    }

    // Other fields in connectionHandle are populated by the caller

    // Link the connection to the list, and update the event loop.
    if (BT_STATUS_SUCCESS == connectStatus) {
        connectionHandle->lowerPrivate->clientSocket = TRUE;
        OI_DBGTRACE(("Adding connection to list"));
        updateEventLoopRegistration();
        OI_List_Add(&connectionHandle->lowerPrivate->links, &connectionList);
        OI_EVENTLOOP_Wakeup();
    } else {
        status = OI_STATUS_NOT_CONNECTED;
        // The caller will free the connection
        deallocLowerConnectionPrivate(connectionHandle);
    }

    OI_DBGTRACE(("%s returning %d", __func__, status));
    return status;
}

/*
 * Accept or reject an incoming transport (RFCOMM/L2CAP) connection, in response to a
 * connect indication.
 */
static OI_STATUS LowerAccept(OI_OBEX_LOWER_CONNECTION connectionHandle,
                             OI_BOOL accept)
{
    OI_STATUS status = OI_OK;

    OI_DBGTRACE(("LowerAccept accept(%08x,%b)", connectionHandle, accept));

    if (accept) {
        /*
         * The connection is already allocated and populated, just add
         * it to the list.
         */
        updateEventLoopRegistration();
        OI_List_Add(&connectionHandle->lowerPrivate->links, &connectionList);
        OI_EVENTLOOP_Wakeup();
    }
    else {
        /*
         * Must close the socket here, because the connection wasn't
         * completed in LowerTransportConnectCfm() (which would have been
         * called if this connection ever got added to the
         * connectionList).
         */

        // Waking up the event loop to ensure that select is not blocking pre-close (BlueZ issue?).
        OI_EVENTLOOP_Wakeup();
        close(connectionHandle->lowerPrivate->socket);
        status = LowerDisconnect(connectionHandle);
    }
    return status;
}

/*
 * Trigger a disconnect.
 */
static OI_STATUS LowerDisconnect(OI_OBEX_LOWER_CONNECTION connectionHandle)
{
    OI_DBGTRACE(("LowerDisconnect(%08x)", connectionHandle));

    // Call must be deferred, because LowerTransportDisconnectInd() calls back upward
    OI_ScheduleCallbackFunction(DeferredLowerTransportDisconnectInd,
                                (void*) connectionHandle,
                                0,
                                &connectionHandle->lowerPrivate->disconnectCbHandle);

    return OI_STATUS_SUCCESS;
}

/*
 * Send Lower transport (RFCOMM/L2CAP) data.  There's no guarantee that an
 * entire buffer will be accepted by send(), so partial  sends are handled.
 * When the socket becomes writeable in the future, the remaining data will be
 * sent.  When all data is sent, the write confirm callback is called.
 *
 * In practice, the entire buffer is accepted in one call.
 *
 * Returns FALSE if there was an error on the connection -- and when
 * FALSE is returned, the caller must assume the connection handle
 * passed in to this function is no longer valid (it may have been
 * deallocated before this function returned).  (Ok - the current
 * implementation never returns FALSE, but it is set up this way for
 * symmetry with LowerTransportRecvDataInd() and LowerTransportConnectCfm())
 */
static OI_BOOL LowerTransportSendMbuf(OI_OBEX_LOWER_CONNECTION connectionHandle)
{
    LOWER_CONNECTION_PRIVATE* lowerPrivate = connectionHandle->lowerPrivate;
    ssize_t sendBytes;
    OI_BOOL sendOk = FALSE;

    OI_DBGPRINT(("LowerTransportSendMbuf(%08x)", connectionHandle));

    // Do a non-blocking send of as many bytes as possible, and update position
    OI_DBGPRINT(("writePos = %d, writeLen = %d",
                  lowerPrivate->writePos,
                  lowerPrivate->writeLen));
    sendBytes = send(lowerPrivate->socket,
                     lowerPrivate->writeBuffer + lowerPrivate->writePos,
                     lowerPrivate->writeLen - lowerPrivate->writePos,
                     MSG_DONTWAIT);

    if (sendBytes > 0) {
        sendOk = TRUE;
        lowerPrivate->writePos += sendBytes;
    }
    else {
        sendOk = FALSE;
    }

    if (!sendOk || (lowerPrivate->writePos == lowerPrivate->writeLen)) {
        OI_STATUS status = sendOk ? OI_OK : OI_FAIL;
        OI_MBUF* mbuf = connectionHandle->lowerPrivate->writeMbuf;

        OI_DBGPRINT(("Done writing %d bytes", lowerPrivate->writeLen));

        connectionHandle->lowerPrivate->writeMbuf = NULL;
        connectionHandle->lowerPrivate->writePos = 0;
        connectionHandle->lowerPrivate->writeLen = 0;

        connectionHandle->callbacks->writeCfm(connectionHandle,
                                              mbuf,
                                              FALSE,
                                              status);
    }

    /*
     * If there are bytes remaining, the event loop will call this
     * function again when the socket is writeable.
     */
    return sendOk;
}

/*
 * Write data to an existing connection.
 */
static OI_STATUS LowerWrite(OI_OBEX_LOWER_CONNECTION connectionHandle,
                            OI_MBUF *mbuf,
                            OI_BOOL accelerateCfm,
                            OI_BOOL *queueFull)
{
    OI_STATUS status = OI_STATUS_SUCCESS;
    LOWER_CONNECTION_PRIVATE* lowerPrivate;

    OI_DBGTRACE(("LowerWrite(%08x,%08x,%d)", connectionHandle, mbuf, accelerateCfm));
    OI_DBGTRACE(("%d bytes to write", OI_MBUF_NumBytes(mbuf)));

    lowerPrivate = connectionHandle->lowerPrivate;

    if (NULL != lowerPrivate->writeMbuf) {
        /* Write is in progress, it must complete before another write starts */
        return OI_STATUS_WRITE_IN_PROGRESS;
    }

    /*
     * Copy MBUF data to a single, contiguous buffer.
     *
     * While each MBUF cell could be sent with a separate socket
     * send(), that would lead to more system calls and suboptimal
     * RFCOMM payload sizes.
     */
    lowerPrivate->writeLen = OI_MBUF_PullBytes(lowerPrivate->writeBuffer,
                                               mbuf,
                                               OI_UINT16_MAX);
    lowerPrivate->writePos = 0;

    if (0 != OI_MBUF_NumBytes(mbuf)) {
        // The mbuf isn't empty, the packet was too long.
        status = OI_STATUS_MTU_EXCEEDED;
        OI_SLOG_ERROR(status, ("OBEX tried to send an oversized packet. extra=%d", OI_MBUF_NumBytes(mbuf)));
        lowerPrivate->writeLen = 0;
        *queueFull = FALSE;
        // Force an RFCOMM disconnect so the OBEX session doesn't stall
        LowerDisconnect(connectionHandle);
    }
    else {
        OI_DBGPRINT(("Writing %d bytes", lowerPrivate->writeLen));

        lowerPrivate->writeMbuf = mbuf;
        *queueFull = TRUE;

        // Force the eventloop to re-check the FDs it's waiting for.
        OI_EVENTLOOP_Wakeup();
    }

    return status;
}

static OI_OBEX_LOWER_PROTOCOL_ID LowerProtocolId(OI_OBEX_LOWER_CONNECTION connectionHandle)
{
    if (connectionHandle && connectionHandle->lowerPrivate) {
        OI_DBGPRINT(("LowerProtocolId %d ", connectionHandle->lowerPrivate->protocol));
        return connectionHandle->lowerPrivate->protocol;
    }
    /* Return rfcomm by default */
    return OI_OBEX_LOWER_RFCOMM;
}

// Stub - RFCOMM flow control isn't supported.
static void LowerFlowControl(OI_OBEX_LOWER_CONNECTION connectionHandle,
                             OI_BOOL flow)
{
    return;
}

// Stub - Getting the L2CAP channel id is not supported
static OI_L2CAP_CID LowerGetCID(OI_OBEX_LOWER_CONNECTION lowerConnection)
{
    return 0;
}

/*
 * Inform the caller of our interface.
 */
const OI_OBEX_LOWER_INTERFACE* OI_OBEX_LowerInterface(void)
{
    static const OI_OBEX_LOWER_INTERFACE lowerInterface = {
        LowerRegServer,
        LowerDeregServer,
        LowerConnect,
        LowerAccept,
        LowerDisconnect,
        LowerWrite,
        LowerProtocolId,
        LowerFlowControl,
        LowerGetCID
    };
    return &lowerInterface;
}

/*
 * Tell other profile code that only L2CAP basic mode is supported, so
 * there are no attempts to use ERTM features (like OBEX over L2CAP).
 */
OI_BOOL OI_L2CAP_IsModeSupported(OI_L2CAP_MODE mode)
{
    return (OI_L2CAP_ENHANCED_RETRANSMISSION_MODE == mode);
}
