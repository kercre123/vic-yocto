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

functions common to OBEX server and client
*/

#ifndef _OBEXCOMMON_H
#define _OBEXCOMMON_H

#include "oi_obex.h"
#include "oi_obexcli.h"
#include "oi_obexsrv.h"
#include "oi_obexspec.h"
#include "oi_obex_lower.h"
#include "oi_mbuf.h"
#include "oi_bytestream.h"

/** \addtogroup OBEX_Internal */
/**@{*/

#ifdef __cplusplus
extern "C" {
#endif


/**
 * size of a small buffer used for composing OBEX packet headers
 */
#define OI_OBEX_HDR_BUFSIZE 8

/**
 * the maximum number of headers passed in to a connect request
 */
#define OI_OBEX_MAX_CONNECT_HDRS  4


typedef struct {
    /**
     * password entered by the user
     */
    OI_BYTE password[OI_OBEX_MAX_PASSWORD_LEN];
    OI_UINT8 passwordLen;
    /**
     * user id of client
     */
    OI_BYTE userId[OI_OBEX_MAX_USERID_LEN];
    OI_UINT8 userIdLen;
    /**
     * remote user id
     */
    OI_BYTE userIdRemote[OI_OBEX_MAX_USERID_LEN];
    OI_UINT8 userIdRemoteLen;
    /**
     * realm
     */
    OI_OBEX_REALM realm;
    /*
     * indicates if server/client requires a user id in the authentication response.
     */
    OI_BOOL userIdRequired;
    /*
     * indicates if server/client was requested to provide a user id to establish an authorized connection
     */
    OI_BOOL userIdRequested;
    /**
     * nonce received in a challenge
     */
    OI_BYTE challengeDigest[OI_OBEX_AUTH_DIGEST_LEN];
    /**
     * nonce received in a response
     */
    OI_BYTE responseDigest[OI_OBEX_AUTH_DIGEST_LEN];
    /**
     * This is the buffer for sending an authentication response header. This is the entire
     * header byte sequence, so we need two bytes for the authentication field tag and length
     * and two bytes + the maximum user id length for the user id field.
     */
    OI_BYTE responseHeader[5 + OI_OBEX_AUTH_DIGEST_LEN + OI_OBEX_MAX_USERID_LEN];
    /**
     * This is the buffer for sending an authentication challenge header. This is the entire
     * header byte sequence, so we need two bytes for the authentication field tag and length.
     * three bytes for the option field, and 257 bytes for the realm
     */
    OI_BYTE challengeHeader[2 + OI_OBEX_AUTH_DIGEST_LEN + 3 + 257];

} OBEX_AUTHENTICATION;


typedef struct {

    /**
     * Pointer to the lower level protocol connection
     */
    OI_OBEX_LOWER_CONNECTION lowerConnection;
    /**
     * Connection handle exposed to upper layer
     */
    OI_OBEX_CONNECTION_HANDLE connectionHandle;
    /**
     * mbuf for sending packets.
     */
    OI_MBUF *mbuf;
    /**
     * Small buffer for composing the packet headers and simple response
     * packets.
     */
    OI_BYTE hdrBuf[OI_OBEX_HDR_BUFSIZE];
    /**
     * Are we authenticating connections?
     */
    OI_UINT8 authenticating;
    /**
     * indicates read-only access is being granted by server.
     */
    OI_UINT8 readOnly;
    /**
     * maintains state during OBEX authentication
     */
    OBEX_AUTHENTICATION *authentication;
    /**
     * Single Response mode flags
     */
    OI_UINT8 srm;
    /**
     * Single response mode parameter for the current operation
     */
    OI_UINT8 srmParam;
    /**
     * flag indicating if srmp is valid or not
     */
    OI_BOOL srmpValid;
    /**
     * flag indicating if srmpWait is received from remote device
     */
    OI_BOOL srmpWaitReceived;
    /**
     * maximum packet size we can receive (based on configuration parameter)
     */
    OI_UINT16 maxRecvPktLen;
    /**
     * maximum packet size we can send (negotiated at connect time)
     */
    OI_UINT16 maxSendPktLen;
    /**
     * opcode for the current operation.
     */
    OI_UINT8 currentOpcode;
    /**
     * body header that had to be segmented because it would not fit in one
     * packet.
     */
    OI_OBEX_HEADER bodySegment;
    /**
     * current PUT/GET progress
     */
    OI_UINT32 progressBytes;
    /**
     * raw received headers, only available during callbacks.
     */
    OI_OBEX_HEADER_LIST *pRawHeaders;

} OBEX_COMMON;


/**
 * Type definition for bulk data blocks passed to OI_OBEXCOMMON_SendBulk().
 */
typedef struct OBEX_BULK_DATA *OBEX_BULK_DATA_LIST;


/**
 * Structure definition for data blocks passed to OI_OBEXCOMMON_SendBulk().
 */
struct OBEX_BULK_DATA {
    OI_UINT8 *blockBuffer;            /* The bulk data block buffer */
    OI_UINT32 blockSize;              /* Size of the bulk data block */
    OI_UINT32 bytesSent;              /* Number of bytes that have been sent */
    OI_UINT32 bytesConfirmed;         /* Number of bytes that have been confirmed */
    OBEX_BULK_DATA_LIST next;         /* Forms a linked list of written but no confirmed bulk data blocks */
    OI_BOOL final;                    /* TRUE if this is the final block for the put transation */
};

/**
 * This function initializes and opens a byte stream for composing a generic OBEX packet.
 *
 * @param common   pointer to common OBEX connection information
 *
 * @param opcode   the OBEX command or response code
 *
 * @param pkt      pointer to byte stream to be initialized
 */
void OI_OBEXCOMMON_InitPacket(OBEX_COMMON *common,
                              OI_UINT8 opcode,
                              OI_BYTE_STREAM *pkt);


/**
 * This function marshals OBEX headers into common->mbuf ready for sending as series
 * of lower layer packets.
 *
 * @param common     pointer to common OBEX connection information
 *
 * @param pktHdr     byte stream previously initialized by
 *                   OI_OBEXCOMMON_InitPacket()
 *
 * @param hdrs       OBEX headers to be marshaled.
 *
 * @param hdrCount   how many headers.
 *
 * @param hdrList    list of headers provided by application.
 *
 * @return           OI_OK or OI_STATUS_OUT_OF_MEMORY
 */
OI_STATUS OI_OBEXCOMMON_MarshalPacket(OBEX_COMMON *common,
                                      OI_BYTE_STREAM *pktHdr,
                                      OI_OBEX_HEADER *hdrs,
                                      OI_UINT16 hdrCount,
                                      OI_OBEX_HEADER_LIST const *hdrList);

/**
 * This function is identical to OI_OBEXCOMMON_MarshalPacket() except that
 * pointer to marshalled mbuf is returned to caller rather than placed
 * directly into common->mbuf.
 *
 * @param common     pointer to common OBEX connection information
 *
 * @param pktHdr     byte stream previously initialized by
 *                   OI_OBEXCOMMON_InitPacket()
 *
 * @param hdrs       OBEX headers to be marshaled.
 *
 * @param hdrCount   how many headers.
 *
 * @param hdrList    list of headers provided by application.
 *
 * @param mbuf       caller's pointer where this function will return pointer
 *                   to mbuf containing marshalled packet.  Undefined if return
 *                   status is not successful.
 *
 * @return           OI_OK or OI_STATUS_OUT_OF_MEMORY
 */
OI_STATUS OI_OBEXCOMMON_MarshalPacketMbuf(OBEX_COMMON       *common,
                                          OI_BYTE_STREAM    *pktHdr,
                                          OI_OBEX_HEADER    *hdrs,
                                          OI_UINT16         hdrCount,
                                          OI_OBEX_HEADER_LIST const *hdrList,
                                          OI_MBUF           **mbuf);



/**
 * This function marshals bulk data into one or more OBEX packets depending on whether
 * single-response mode is enabled or not.
 *
 * @param common     A pointer to common OBEX connection information
 *
 * @param bulkData   The current the bulk data list to send.
 *
 * @param busy       If TRUE indicates that bulk sends cannot be performed now.
 *
 * @param final      Returns TRUE if we completely put the final bulk data block
 */
OI_STATUS OI_OBEXCOMMON_SendBulk(OBEX_COMMON *common,
                                 OBEX_BULK_DATA_LIST *bulkData,
                                 OI_BOOL *busy,
                                 OI_BOOL *final);


/**
 * Allocate and initialize a linked list of bulk data blocks
 *
 * @param numBuffer        The number of bulk data blocks to allocate
 *
 * @param bulkDataBuffer   An array of bulk data buffers
 *
 * @param bufferLength     An array of lengths for the bulk data buffers
 *
 * @param bulkDataHead     Returns the head of the allocated bulk data list
 *
 * @param bulkDataTail     Returns the tail of the allocated bulk data list
 */
OI_STATUS OI_OBEXCOMMON_AllocBulkData(OI_UINT8 numBuffers,
                                      OI_UINT8 *bulkDataBuffer[],
                                      OI_UINT32 bufferLength[],
                                      OBEX_BULK_DATA_LIST *bulkDataHead,
                                      OBEX_BULK_DATA_LIST *bulkDataTail);


/**
 * Free a bulk data list
 */
void OI_OBEXCOMMON_FreeBulkData(OBEX_BULK_DATA_LIST list);


/**
 * This function marshals segmented body headers.
 *
 * @param common     A pointer to common OBEX connection information
 *
 */
OI_STATUS OI_OBEXCOMMON_MarshalBodySegment(OBEX_COMMON *common);


/**
 * Send a simple OBEX packet - just an opcode with no headers
 *
 * @param common     A pointer to common OBEX connection information
 *
 * @param opcode     The opcode to send
 */

OI_STATUS OI_OBEXCOMMON_SendSimple(OBEX_COMMON *common,
                                   OI_UINT8 opcode);
/**
 * Send an OBEX OK response packet.
 *
 * @param common     A pointer to common OBEX connection information
 */
#define OI_OBEXCOMMON_SendOk(common) \
    OI_OBEXCOMMON_SendSimple((common), OI_OBEX_FINAL(OI_OBEX_RSP_OK))


/**
 * Send an OBEX CONTINUE response packet.
 *
 * @param common     A pointer to common OBEX connection information
 */
#define OI_OBEXCOMMON_SendContinue(common) \
    OI_OBEXCOMMON_SendSimple((common), OI_OBEX_FINAL(OI_OBEX_RSP_CONTINUE))


/**
 * Header overhead for a bulk PUT request issued by an OBEX client
 */
#define BULK_PUT_HDR_SIZE (sizeof(OI_UINT16) + sizeof(OI_UINT8) + sizeof(OI_UINT16) + sizeof(OI_UINT8))


/**
 * Header overhead for a bulk GET response issued by an OBEX server
 */
#define BULK_GET_HDR_SIZE (sizeof(OI_UINT16) + sizeof(OI_UINT8) + sizeof(OI_UINT16) + sizeof(OI_UINT8))


/**
 * Data received from an OBEX server.
 */

OI_STATUS OI_OBEXCOMMON_ReassemblePacket(OBEX_COMMON *common,
                                         OI_BYTE *dataBuf,
                                         OI_UINT16 dataLen);


/**
 * Returns the size in bytes that the headers will occupy in an OBEX packet.
 */
OI_UINT16 OI_OBEXCOMMON_HeaderListSize(const OI_OBEX_HEADER_LIST *headers);


/**
 * Get all headers.
 */
OI_STATUS OI_OBEXCOMMON_ParseHeaderList(OBEX_COMMON *common,
                                        OI_OBEX_HEADER_LIST *headers,
                                        OI_BYTE_STREAM *bs);

/**
 * Remove a specific header form the header list and free any memory allocated for the header.
 *
 * @param headerList    The header list.
 *
 * @poaram headerId     The header id of the header to to remove from the header list.
 */
OI_STATUS OI_OBEXCOMMON_DeleteHeaderFromList(OI_OBEX_HEADER_LIST *headerList,
                                             OI_UINT8 headerId);


/**
 * Builds an application paramers header based on a list of elements. The caller
 * is responsible for calling OI_Free(data->data) when it is finished with this
 * information. The underlying application parameter entry list data is not
 * affected by this operation.
 */
OI_STATUS OI_OBEXCOMMON_BuildAppParamsHeader(OI_OBEX_BYTESEQ *data,
                                             const OI_OBEX_APP_PARAM_LIST *params);




#ifdef __cplusplus
}
#endif

/**@}*/

#endif /* _OBEXCOMMON_H */

