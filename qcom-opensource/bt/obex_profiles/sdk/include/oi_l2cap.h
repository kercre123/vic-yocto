#ifndef _OI_L2CAP_H
#define _OI_L2CAP_H

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
 */

/**
 * @file
 * This file provides the L2CAP (Logical Link Controller and Adaptation Protocol) API.
 *
 * See the @ref L2CAP_docpage section of the BLUEmagic 3.0 SDK documentation for more information.
 *
 */

#include "oi_common.h"
#include "oi_connect_policy.h"
#include "oi_bt_spec.h"

/** \addtogroup L2CAP L2CAP APIs */
/**@{*/

#ifdef __cplusplus
extern "C" {
#endif


/** AMP Device Identifier. ID = 0 refers to the BR/EDR controller */
typedef OI_UINT8 OI_AMP_ID;

/** type for a channel identifier (CID) */
typedef OI_UINT16 OI_L2CAP_CID;

/** type for a protocol/service multiplexer (PSM) */
typedef OI_UINT16 OI_L2CAP_PSM;


/******************************************************************************

  Connection parameters

 ******************************************************************************/

/**
 * The L2CAP connection modes. The connection mode is specified when making an outbound connection or
 * reported when an inbound connection is accepted.
 *
 * Note: FLOW_CONTROL_MODE and RETRANSMISSION_MODE are only listed for documentation purposes.
 * Because they have are superceded by ENHANCED_RETRANSMISSION and STREAMING mode they
 * are not implemented in BLUEmagic 3.0.
 */
typedef enum {
    OI_L2CAP_BASIC_MODE                    = 0,
    OI_L2CAP_FLOW_CONTROL_MODE             = 1,
    OI_L2CAP_RETRANSMISSION_MODE           = 2,
    OI_L2CAP_ENHANCED_RETRANSMISSION_MODE  = 3,
    OI_L2CAP_STREAMING_MODE                = 4
} OI_L2CAP_MODE;


/**
 * The L2CAP extended feature mask is a logical OR of the features bits defined below. The
 * extended feature mask was originally defined in the Bluetooth specification.
 *
 * The extended feature mask is one of the stack configuration parameters defined in
 * the OI_CONFIG_L2CAP structure in oi_bt_stack_config.h
 */
#define OI_L2CAP_FEATURE_VALID_BITS             (0x000003FF)

#define OI_L2CAP_FEATURE_FLOW_CONTROL_MODE                     OI_BIT0
#define OI_L2CAP_FEATURE_RETRANSMISSION_MODE                   OI_BIT1
#define OI_L2CAP_FEATURE_BI_DIRECTIONAL_QOS                    OI_BIT2
#define OI_L2CAP_FEATURE_ENHANCED_RETRANSMISSION_MODE          OI_BIT3
#define OI_L2CAP_FEATURE_STREAMING_MODE                        OI_BIT4
#define OI_L2CAP_FEATURE_FCS_OPTION                            OI_BIT5
#define OI_L2CAP_FEATURE_EXTENDED_FLOW_SPEC                    OI_BIT6
#define OI_L2CAP_FEATURE_FIXED_CHANNELS                        OI_BIT7
#define OI_L2CAP_FEATURE_EXTENDED_WINDOW_SIZE                  OI_BIT8
#define OI_L2CAP_FEATURE_UNICAST_CONNECTIONLESS_DATA_RECEPTION OI_BIT9

/**
 * FCS option values
 */
#define OI_L2CAP_FCS_NONE       0   /** No FCS check requested */
#define OI_L2CAP_FCS_DEFAULT    1   /** Use the 16 bit FCS defined in section 3.3.5 of the L2CAP spec */

/**
 *  Info type requests for use in OI_L2CAP_InfoReq()
 */
#define OI_L2CAP_INFO_TYPE_CONNECTIONLESS_MTU         1
#define OI_L2CAP_INFO_TYPE_EXTENDED_FEATURE_MASK      2
#define OI_L2CAP_INFO_TYPE_FIXED_CHANNELS             3

/**
 * L2CAP Fixed channels.
 * Fixed channels (CIDs) exist in the reserved range 0x03-0x3f.
 * The OI_L2CAP_INFO_TYPE_FIXED_CHANNELS_MASK infoType is used to
 * communicate which fixed (reserved) channels are in use.
 */
#define OI_L2CAP_NULL_CID                   0
#define OI_L2CAP_SIGNALLING_CID             1
#define OI_L2CAP_CONNECTIONLESS_CID         2
#define OI_AMP_SIGNALING_CID                3
#define OI_ATTRIBUTE_PROTOCOL_CID           4
#define OI_AMP_TEST_MGR_CID              0x3f

/**
 * Enumeration type for flow control parameter used in OI_L2CAP_Flow()
 */
typedef enum {
    OI_L2CAP_FLOW_ON,   /**< Turn flow on - allow packets to be sent to the upper layer */
    OI_L2CAP_FLOW_OFF,  /**< Turn flow off - prevent packets from being sent to the upper layer */
    OI_L2CAP_FLOW_RNR   /**< Immediately send an RNR to stop remote peer from sending data */
} OI_L2CAP_FLOW_CONTROL;


/**
 * The service type values for the serviceType field of an extended flow specification.
 */
#define OI_L2CAP_SERVICE_TYPE_NO_TRAFFIC    0x00
#define OI_L2CAP_SERVICE_TYPE_BEST_EFFORT   0x01
#define OI_L2CAP_SERVICE_TYPE_GUARANTEED    0x02


#define OI_L2CAP_FLAGS_NONE                             0

/*
 * If set, the L2CAP write is permitted to complete synchronously without calling the callback
 * function. Completion means queued internally, it does not mean received by the remote device.
 */
#define OI_L2CAP_FLAGS_ALLOW_SYNCHRONOUS_COMPLETION   BIT0

/*
 * If set, indicates to L2CAP that the upper layer needs a quick write confirmation. In ERTM mode,
 * L2CAP may choose to accelerate the acknowledgement of the packet by sending a poll packet or copy
 * the packet and queue it internally. This is a no-op in BASIC and STREAMING mode.
 *
 * Note that care must be taken when using this flag:
 *
 * - This flag may cause L2CAP to make a copy of the data passed to OI_L2CAP_QueueMBUF().
 * - The accelerated write confirm callbacks may happen before callbacks for earlier writes to the
 *   same L2CAP channel that were not accelerated. Applications using this function should not
 *   depend on the ordering of write confirm callbacks in this case.
 */
#define OI_L2CAP_FLAGS_ACCELERATE_WRITE_CFM           BIT1


/**
 * An extended flow specification is used for requesting a desired Quality of Service (QoS) on a
 * channel. The flow specifications specify parameters for outgoing traffic.
 */
typedef struct {
    OI_UINT8 identifier;            /**< Used internally by L2CAP, should be initialized to zero */
    OI_UINT8 serviceType;           /**< Identifies the service type for the flow spec */
    OI_UINT16 maximumSduSize;
    OI_UINT32 sduInterArrivalTime;  /**< This value is specified in microseconds */
    OI_UINT32 accessLatency;        /**< This value is specified in microseconds */
    OI_UINT32 flushTimeout;         /**< This value is specified in microseconds */
} OI_L2CAP_EXTENDED_FLOW_SPEC;


/**
 * A structure of this type is passed as an argument to OI_L2CAP_Connect() or OI_L2CAP_Accept() and
 * carries the connection and configuration parameters used when creating or accepting a connection.
 */
typedef struct {

    /**
     * Maximum transmission unit (MTU): greatest acceptable size for incoming data packets (SDUs)
     * in bytes (sec 5.1 of BT spec v2.0+EDR vol 3 part A).  Because of buffer size limitations, it
     * may be advantageous to use the value returned by OI_L2CAP_MaxSupportedMTU if the largest
     * possible buffer size is required.
     */
    OI_UINT16 inMTU;

    /**
     * Flush timeout: number of milliseconds to wait before an L2CAP packet that can not be
     * acknowledged at the physical layer is dropped (sec 5.2 of BT spec v2.0+EDR vol 3 part A)
     */
    OI_UINT16 outFlushTO;

    /**
     * Link supervision timeout: number of milliseconds to wait before terminating an unresponsive
     * link
     */
    OI_UINT16 linkTO;

    /**
     * Mode that is preferred or required for this link.
     */
    OI_L2CAP_MODE mode;

    /**
     * If TRUE, the requested mode is mandatory for this connection, otherwise other modes are
     * acceptable for this connection. If FALSE and mode == OI_L2CAP_BASIC_MODE this is
     * treated as "don't care" and the mode from the global configuration parameters is used.
     */
    OI_BOOL modeMandatory;

    /**
     * Configuration parameters for ENHANCED RETRANSMISSSION and STREAMING mode. This value must be
     * in the range 1..16383. Window size values > 63 indicate a request to use Extended Control Fields.
     * For ENHANCED RETRANSMISSSION mode the actual window size is negotiated and may be lower than
     * the configured value. For STREAMING mode, the value is used solely as a request to use
     * Extended Control Fields.
     */
    OI_UINT16 txWindowSize;  /**< Maximum local tx window size */

    /**
     * Configuration parameters for ENHANCED RETRANSMISSSION mode only, This value is ignored
     * and should be zero for BASIC and STREAMING mode.
     */
    OI_UINT8 maxTransmit;    /**< Maximum number of retransmit attempts */

    /**
     * Configuration parameters for ENHANCED RETRANSMISSSION or STREAMING mode, These values are ignored
     * and should be zero for BASIC and STREAMING mode.
     */
    OI_UINT16 mps;          /**< Maximum PDU payload size */
    OI_UINT8 fcsOption;     /**< Frame Check Sequence option, currently this is either OI_L2CAP_FCS_NONE or OI_L2CAP_FCS_DEFAULT */

    OI_AMP_ID ampId;        /**< AMP controller ID, or 0 for the BR/EDR controller */

    /*
     * An extended flow spec of NULL if none is provided. Note that the extended flow spec overrides
     * the flush timeout.
     */
    OI_L2CAP_EXTENDED_FLOW_SPEC *extFlowSpec;

} OI_L2CAP_CONNECT_PARAMS;


/*
 * ************ MTU constants ************
 */


/**
 * The L2CAP specification requires that the
 *  maximum transmission unit (MTU) for an L2CAP connection be at least 48.
 */

#define OI_L2CAP_MTU_MIN           48

/**
 * The default maximum transmission unit (MTU) for an L2CAP connection
 */
#define OI_L2CAP_MTU_DEFAULT       672

/**
 * The largest possible value for a maximum transmission unit (MTU) for a BASIC mode L2CAP
 * connection as defined by the Bluetooth specification. This value should only be used by
 * applications that know that the L2CAP channel will be configured for BASIC mode.
 */
#define OI_L2CAP_MTU_MAX           65535


/**
 * In ENHANCED_RETRANSMISSION and STREAMING modes, additional header fields and an optional FCS
 * reduce the maximum number of payload bytes. The following value is the maximum number of payload
 * bytes that can be assumed by an application that has no knowledge about the L2CAP channel mode or
 * if the connection uses AMP (Alternative MAC/PHY)
 */
#define OI_L2CAP_SDU_SAFE_MAX       65527



/*
 * ************* Flush timeout constants ************
 */


/**
 * This flush timeout value indicates to the L2CAP layer that the caller does not care
 * what the flush timeout is. The flush timeout will be whatever it was previously.
 */
#define OI_L2CAP_FLUSHTO_DONT_CARE      0x0000

/**
 * This flush timeout value indicates that the L2CAP layer should perform no retransmissions.
 * This value represents a number of milliseconds but, since 1 millisecond is smaller than a
 * baseband timeslot pair, the interpretation is that no retransmissions will occur.
 */
#define OI_L2CAP_FLUSHTO_NO_RETRANSMIT  0x0001


/**
 * This flush timeout value indicates that the L2CAP layer should perform retransmissions until
 * the link timeout terminates the channel. This is referred to as 'reliable channel'. Note that
 * this is the default value.
 */
#define OI_L2CAP_FLUSHTO_INFINITE       0xFFFF


/**
 * The L2CAP APIs specify the flush timeout in milliseconds; this value is the maximum flush timeout
 * expressed as milliseconds.  This is computed from the following formula  0x7FF * 0.625ms where
 * 0x7FF is the maximum value allowed by the underlying Write Automatic Flush Timeout HCI command
 * (expressed as Bluetooth ticks, tick = 0.625ms).
 */
#define OI_L2CAP_FLUSHTO_MAX            1279


/**
 * ************** Link timeout constants ***********************
 */


/**
 * This is a special value to indicate to the L2CAP layer that the caller does not care what
 * the link supervision timeout is. The link supervision timeout will be whatever it was previously.
 */
#define OI_L2CAP_LINKTO_DONT_CARE       0xffff

/**
 * This is a special value to indicate that no link supervision timeout should be set.
 */
#define OI_L2CAP_LINKTO_INFINITE        0xfffe

/**
 * The L2CAP APIs specify the link supervision timeout in milliseconds; this value is the maximum
 * link supervision timeout expressed as milliseconds. This is computed from the following formula
 * 0xFFFF * 0.625ms where 0xFFFF is the maximum value allowed by the underlying Write Link
 * Supervision Timeout HCI command (expressed as Bluetooth ticks, tick = 0.625ms).
 * time).
 */
#define OI_L2CAP_LINKTO_MAX            40959



/******************************************************************************

  indication callback function type definitions

 ******************************************************************************/

/**
 * A callback function of this type indicates that a connection request has been received from a remote device.
 * This request should be responded to by calling OI_L2CAP_Accept().
 *
 * @param psm             Specifies the protocol/service multiplexer of the protocol or service requesting the connection
 * @param addr            Pointer to the Bluetooth device address of the remote device
 * @param localAmpId      ID of local AMP device receiving the connection attempt or 0 for BR/EDR
 * @param cid             Channel identifier (CID) of the connection
 */
typedef void (*OI_L2CAP_CONNECT_IND)(OI_L2CAP_PSM psm,
                                     OI_BD_ADDR *addr,
                                     OI_AMP_ID localAmpId,
                                     OI_L2CAP_CID cid);

/**
 * A callback function of this type informs a higher layer of the success or failure of a
 * call to OI_L2CAP_MoveChannel().
 *
 * @param cid     Channel ID affected by the move operation.
 * @param ampId   New local AMP ID for the L2CAP channel.
 * @param result  Indicates success or failure of the move operation.
 */
typedef void (*OI_L2CAP_MOVE_CFM)(OI_L2CAP_CID cid,
                                  OI_AMP_ID localAmpId,
                                  OI_STATUS result);

/**
 * A callback function of this type indicates an incoming request to move an existing L2CAP channel
 * has been received. The implementation of OI_L2CAP_MOVE_IND should call OI_L2CAP_AcceptMoveChannel()
 * to allow (or disallow) the move operation and to establish a move complete callback.
 *
 * @param cid             Channel identifier (CID) of the connection
 * @param addr            Pointer to the Bluetooth device address of the remote device
 * @param currentAmpId    ID of the local AMP device currently (before move) associated with channel.
 * @param finalAmpId      ID of the local AMP device that will be associated with channel if move is successful.
 */
typedef void (*OI_L2CAP_MOVE_IND)(OI_L2CAP_CID cid,
                                  const OI_BD_ADDR *addr,
                                  OI_AMP_ID currentAmpId,
                                  OI_AMP_ID finalAmpId);

/**
 * A callback function of this type indicates that the QoS agreement between the
 * local device and a remote device has been violated. This indication does not require a response.
 * Since quality of service management is not supported in the present BLUEmagic 3.0 software implementation
 * of L2CAP, this callback function will never be called.
 *
 * @param addr            Pointer to the address of the remote Bluetooth device
 */
typedef void (*OI_L2CAP_QOS_VIOLATION_IND)(OI_BD_ADDR *addr);


typedef enum {
    OI_L2CAP_RECV_OK,           /**< Packet was received successfully */
    OI_L2CAP_RECV_INCOMPLETE,   /**< In STREAMING mode: packet was incomplete */
    OI_L2CAP_RECV_CORRUPT,      /**< In STREAMING mode: packet was corrupt */
    OI_L2CAP_RECV_PKT_DISCARDED /**< In STREAMING mode: packet was received but discarded */
} OI_L2CAP_RECV_STATUS;


/**
 * A callback function of this type indicates that data was received on the specified connection.
 *
 * @param cid            Channel identifier for the channel on which data was received
 * @param buf            Pointer to buffer holding the received data
 * @param length         Number of bytes received in the buffer
 * @param recvStatus     Indicates the status of the received packet
 */
typedef void(*OI_L2CAP_RECV_DATA_IND)(OI_L2CAP_CID cid,
                                      OI_UINT8 *buf,
                                      OI_UINT16 length,
                                      OI_L2CAP_RECV_STATUS recvStatus);

/**
 * A callback function of this type indicates that the specified connection was disconnected.
 *
 * @param cid            Specifies the local channel identifier for the connection being disconnected
 * @param reason         Specifies the reason for the disconnection
 */
typedef void (*OI_L2CAP_DISCONNECT_IND)(OI_L2CAP_CID cid,
                                        OI_STATUS reason);

/******************************************************************************

  confirmation callback function type definitions

 ******************************************************************************/

/**
 * A callback function of this type informs a higher program layer of the success or failure of an
 * OI_L2CAP_Connect() or OI_L2CAP_Accept() command. A status of OI_OK
 * indicates that a connection corresponding to the specified CID has been created and configured
 * and is therefore in the open state. A callback function of this type will also be
 * called if a reconfiguration of the connection takes place. The fields inMTU and outMTU
 * carry the parameters for maximum transmission unit (MTU) for incoming data and outgoing data.
 *
 * @param cid         Local channel identifier for the connection being created
 * @param result      Success or failure status code for the connection request
 * @param inMTU       MTU for incoming packets
 * @param outMTU      MTU for outgoing packets
 */
typedef void(*OI_L2CAP_CONNECT_COMPLETE_CALLBACK)(OI_L2CAP_CID cid,
                                                  OI_UINT16 inMTU,
                                                  OI_UINT16 outMTU,
                                                  OI_STATUS result);

/**
 * A callback function of this type informs a higher program layer that L2CAP no longer needs access
 * to the data buffer and report the success or failure of an earlier call to OI_L2CAP_Write().
 *
 * @param cid        Local channel identifier for the connection to which the data is to be written
 * @param buf        Pointer to the buffer from which data was written
 * @param sendCount  Number of bytes written from the send buffer
 * @param result     Success or failure status code for the write command
 */
typedef void(*OI_L2CAP_DATA_WRITE_CFM)(OI_L2CAP_CID cid,
                                       OI_UINT8 *buf,
                                       OI_UINT16 sendCount,
                                       OI_STATUS result);


/**
 * A callback function of this type informs a higher program layer that L2CAP no longer needs access
 * to the data buffer and report the success or failure of an earlier call to OI_L2CAP_QueueWrite().
 *
 * @param cid        Local channel identifier for the connection to which the data is to be written
 * @param buf        Pointer to the buffer from which data was written
 * @param sendCount  Number of bytes written from the send buffer
 * @param queueFull  Returns TRUE if the queue is full, FALSE if the queue is not full. If the queue
 *                   is not full, another write can be queued by calling OI_L2CAP_QueueWrite().
 * @param result     Success or failure status code for the write command
 */
typedef void(*OI_L2CAP_DATA_QUEUE_WRITE_CFM)(OI_L2CAP_CID cid,
                                             OI_UINT8 *buf,
                                             OI_UINT16 sendCount,
                                             OI_BOOL queueFull,
                                             OI_STATUS result);

/**
 * A callback function of this type informs a higher program layer of the results of
 * a call to OI_L2CAP_EchoReq().
 *
 * @param addr        Bluetooth device address for the responding device
 * @param context     Context pointer provided by the caller of the echo function
 * @param sentData    Pointer to the data buffer provided by the caller of the echo request function
 * @param sentLength  Number of bytes passed by the caller or the echo request function
 * @param echoData    Pointer to the buffer holding data returned by the responding device
 * @param echoLength  Number of bytes of data returned by the responding device
 * @param result      Success or failure status code for the echo command
 */
typedef void(*OI_L2CAP_ECHO_RSP)(const OI_BD_ADDR *addr,
                                 void *context,
                                 OI_UINT8 *sentData,
                                 OI_UINT16 sendLength,
                                 OI_UINT8  *echoData,
                                 OI_UINT16 echoLength,
                                 OI_STATUS result);

/**
 * A callback function of this type informs a higher program layer of the success or failure of
 * a call to OI_L2CAP_InfoReq().
 *
 * @param addr       Address of device to receive the information request
 * @param context    Opaque user-defined context
 * @param infoType   Specifies the type of data requested
 * @param infoData   Pointer to the buffer holding response data returned by the remote device
 * @param size       Number of bytes of information response data
 * @param result     Success or failure status code for the information request
 */
typedef void(*OI_L2CAP_INFO_RSP)(const OI_BD_ADDR *addr,
                                 void             *context,
                                 OI_UINT16         infoType,
                                 OI_UINT8         *infoData,
                                 OI_UINT16         size,
                                 OI_STATUS         result);


/****************************************************************

  Functions

 *****************************************************************/

/**
 * This function registers a client protocol or service PSM (protocol/service multiplexer)
 * and callback functions with the L2CAP layer. This allows the L2CAP layer to
 * notify the protocol or service when remote hosts attempt to connect to it. If this particular
 * protocol or service will not respond to incoming connection requests, then registration is unnecessary.
 *
 * @param psm            Specifies the PSM (Protocol/Service Multiplexer) of the client protocol or service
 * @param connectInd     Callback used to notify the application of incoming connections
 * @param moveInd        Callback used to notify the application of incoming requests to move a channel
 *
 * @return               Status of registration request: OI_OK, if successful; failure code otherwise
 */
OI_STATUS OI_L2CAP_Register(OI_L2CAP_PSM psm,
                            OI_L2CAP_CONNECT_IND connectInd,
                            OI_L2CAP_MOVE_IND moveInd);

/**
 * This function deregisters a protocol registered on a specified PSM.
 *
 * @param psm  Specifies the PSM to deregister
 */
OI_STATUS OI_L2CAP_Deregister(OI_L2CAP_PSM psm);

/**
 * This function obtains a dynamically allocated PSM (Protocol/Service Multiplexer) for
 * an application that does not have a static PSM. Once a PSM has been allocated, the
 * application should call OI_L2CAP_Register() to register the newly allocated PSM.
 *
 * @param psm            Pointer to a PSM to hold the newly allocated PSM
 */
OI_STATUS OI_L2CAP_AllocPSM(OI_L2CAP_PSM *psm);


/**
 * This function returns the PSM for a given L2CAP channel.
 *
 * @param cid  Specifies the L2CAP channel from which to obtain the PSM
 *
 * @return  The PSM for the channel or 0 if the channel ID is invalid.
 */
OI_L2CAP_PSM OI_L2CAP_GetPSM(OI_L2CAP_CID cid);


/**
 * This function requests that a connection be created and configured. The cid parameter is a pointer
 * to a location where the channel identifier for the connection will be written once the
 * OI_L2CAP_CONNECT_COMPLETE_CALLBACK callback function is called with a success status code, confirming
 * that the connection has been successfully created. Configuration of the channel will be negotiated
 * with the remote device. If the remote device agrees, then the configuration parameters used will be
 * those specified by the connectSpec parameter; a null connectSpec pointer specifies the default configuration
 * values. If the remote device does not agree with the configuration parameters specified, then
 * the parameters requested by the remote side will be used.
 *
 * @param confirmCB       Callback function to be called with the result of this connection request
 * @param disconnectInd   Callback function to indicate that the connection created by this request was disconnected
 * @param recvDataInd     Callback function to indicate that data was received on the connection created by this request
 * @param targetPSM       PSM (Protocol/Service Multiplexer) of the remote protocol to which to connect
 * @param addr            Pointer to the Bluetooth device address of the remote device to which to connect
 * @param params          Pointer to the structure containing the configuration parameters requested by the
 *                        local device for this connection; a NULL value for this pointer indicates that the
 *                        default values are to be used.
 * @param cid             [out] Pointer to where the connection's channel identifier will be written once the connection
 *                        has been created
 * @param policy          Pointer to the connection policy required on this connection, which will be managed
 *                        and enforced by the Policy Manager.  This parameter must not be NULL.
 *
 * @return                Status of connection request: OI_OK, if successful; failure code otherwise
 */
OI_STATUS OI_L2CAP_Connect(OI_L2CAP_CONNECT_COMPLETE_CALLBACK confirmCB,
                           OI_L2CAP_DISCONNECT_IND disconnectInd,
                           OI_L2CAP_RECV_DATA_IND recvDataInd,
                           OI_L2CAP_PSM targetPSM,
                           OI_BD_ADDR *addr,
                           const OI_L2CAP_CONNECT_PARAMS *params,
                           OI_L2CAP_CID *cid,
                           const OI_CONNECT_POLICY *policy);

/**
 * This function is used to respond to a connection request, indicating that the connection should be
 * established and configured without further client interaction. This function should be called
 * in response to an OI_L2CAP_CONNECT_IND indication callback. The OI_L2CAP_CONNECT_COMPLETE_CALLBACK callback
 * function will be called when establishment and configuration of the accepted connection are complete.
 *
 * @param confirmCB      Callback function to be called with the result of the attempt to establish and configure
 *                       the connection; may be NULL if the response parameter does not indicate success
 * @param disconnectInd  Callback function to indicate that the connection created by this request was disconnected
 * @param recvDataInd    Callback function to indicate that data was received on the connection created by this request
 * @param cid            Specifies the local channel identifier for the connection created by this request
 * @param connectSpec    Pointer to the structure containing the configuration parameters requested by the
 *                       local device for this connection; a NULL value for this pointer indicates that the
 *                       default values are to be used.
 * @param accept         TRUE to accept the connection, FALSE to reject
 * @param policy         Pointer to the connection policy required on this connection
 *                       which will be managed and enforced by the Policy Manager
 *
 * @return               Status of connection acceptance request: OI_OK, if successful; failure code otherwise
 */
OI_STATUS OI_L2CAP_Accept(OI_L2CAP_CONNECT_COMPLETE_CALLBACK confirmCB,
                          OI_L2CAP_DISCONNECT_IND disconnectInd,
                          OI_L2CAP_RECV_DATA_IND recvDataInd,
                          OI_L2CAP_CID cid,
                          const OI_L2CAP_CONNECT_PARAMS *connectSpec,
                          OI_BOOL accept,
                          const OI_CONNECT_POLICY *policy);


/**
 * This function allows the caller to provide a receive buffer for a specific L2CAP channel. Received data will be
 * reassembled directly into this buffer rather than using L2CAP's internal reassembly buffer. For
 * some applications this can avoid memory to memory copies. The buffer must be large enough for the
 * L2CAP MTU for this channel. Note that L2CAP "owns" the buffer passed in until another buffer is
 * set or is set to NULL.
 *
 * @param cid         Identifies the L2CAP channel on which to set a receive buffer
 *
 * @param buffer      Pointer to the receive buffer or NULL to revert to using the internal L2CAP
 *                    buffer.
 *
 * @param bufLen      Specifies the size of the buffer being passed in.
 *
 * @param prevBuffer  Returns the buffer set by the previous call to this function, or NULL if there
 *                    was no previous buffer.
 *
 * @return
 *               - OI_OK on success
 *               - OI_STATUS_INVALID_STATE if not called from within a receive indication callback
 *               - OI_STATUS_BUFFER_TOO_SMALL if the bufLen is less than the L2CAP MTU
 *               - Or other error status codes.
 */
OI_STATUS OI_L2CAP_SetReceiveBuffer(OI_L2CAP_CID cid,
                                    OI_BYTE *buffer,
                                    OI_UINT16 bufLen,
                                    OI_BYTE **prevBuffer);


/**
 * This function gets a pointer to the buffer previously set on an L2CAP channel by a call to
 * OI_L2CAP_SetReceiveBuffer().
 *
 * @param cid   Identifies the L2CAP channel for which to get the receive buffer.
 *
 * @return      Pointer to the receive buffer, or NULL if no buffer has been set on this
 *              channel or if the cid is invalid.
 */
OI_BYTE* OI_L2CAP_GetReceiveBuffer(OI_L2CAP_CID cid);


/**
 * This function returns the connection mode for a given L2CAP connection. Since the mode requested
 * in a connection may not be supported by the remote device, a profile or application can call this
 * API to determine the mode that is actually configured.
 * `
 * @param cid            Local channel identifier for the connection being queried
 * @param mode           Pointer value returned by this function
 *
 * @return               OI_OK or an error status if the connection id is invalid.
 */
OI_STATUS OI_L2CAP_GetMode(OI_L2CAP_CID cid,
                           OI_L2CAP_MODE *mode);


/**
 * This function is called to determine if a specific L2CAP connection mode is supported by the local
 * device.
 *
 * @param mode  Specifies the mode to query for support.
 *
 * @return TRUE if the mode is supported, FALSE if it is not.
 */
OI_BOOL OI_L2CAP_IsModeSupported(OI_L2CAP_MODE mode);


/**
 * This function returns the SDU and MPS sizes for the given L2CAP connection. In Basic mode, the SDU
 * and MPS will be the same size. In Enhanced Retransmission mode and Streaming mode, the application
 * may want to write data in packets in the largest whole number multiple of the MPS size that fits
 * within the SDU size.
 * `
 * @param cid            Local channel identifier for the connection being queried
 * @param mtu            Pointer to MTU value returned by this function
 * @param mps            Pointer to MPS value returned by this function
 *
 * @return               OI_OK or an error status if the connection id is invalid.
 */
OI_STATUS OI_L2CAP_GetTxSizes(OI_L2CAP_CID cid,
                              OI_UINT16 *mtu,
                              OI_UINT16 *mps);


/**
 * This function commands the termination of a connection associated with a particular channel identifier.
 * The disconnect callback function passed to OI_L2CAP_Accept() or OI_L2CAP_Connect() will be called when the
 * disconnection is complete.
 *
 * @param  cid       Specifies the local channel identifier (CID) of the connection to terminate
 */
OI_STATUS OI_L2CAP_Disconnect(OI_L2CAP_CID cid);


/**
 * This function calls for data to be written from the output buffer associated with a particular connection.
 *
 * Once this function has been called to write data to a particular connection (CID), it must not be
 * called again to write to that same connection until a callback of type OI_L2CAP_DATA_WRITE_CFM
 * has been received for that connection, confirming that L2CAP has finished writing from the output
 * buffer and no longer needs access to it. Calling this function twice with the same CID parameter
 * without an intervening OI_L2CAP_DATA_WRITE_CFM confirmation for that CID will result in an error
 * return status and the data will not be written.
 *
 * Concurrent writes to different channels are always permitted, since each channel has its own output buffer.
 *
 * @param callback       Callback to be called for confirmation that the data has been written to the connection
 *                       from the output buffer
 *
 * @param cid            Local channel identifier (CID) of the connection to which to write
 *
 * @param outBuffer      Pointer to the output buffer containing the data to be written
 *
 * @param length         Number of bytes to write from the buffer
 *
 * @return               - OI_OK if the packet has been accepted by L2CAP
 *
 *                       - OI_STATUS_WRITE_IN_PROGRESS indicates that the write failed becauses of a
 *                         write in progress. Caller can retry the write after the write confirm callback.
 *
 *                       - Other status codes indicating that the write failed.
 */
OI_STATUS OI_L2CAP_Write(OI_L2CAP_DATA_WRITE_CFM callback,
                         OI_L2CAP_CID cid,
                         OI_UINT8 *outBuffer,
                         OI_UINT16 length);



/**
 * This function calls for queued data to be written from the output buffer associated with a particular connection.
 *
 * In BASIC and STREAMING mode, only one packet can be queued at a time on a given L2CAP channel, in
 * ENHANCED_RETRANSMISSION mode, multiple packets can be queued up to the limit of internal memory.
 * The status code indicates if the packed was successfully queued. Packets will be sent in the
 * order in which they are queued and the callback functions will be called in the order in which
 * the packets are sent. After the callback function has been called the queue may no longer be
 * full in which case another call to OI_L2CAP_QueueWrite() is permitted.
 *
 * Concurrent writes to different channels are always permitted, since each channel has its own queue.
 *
 * @param callback       Callback to be called for confirmation that the data has been written to
 *                       the connection from the output buffer. The callback function is not called
 *                       if the OI_L2CAP_FLAGS_ALLOW_SYNCRONOUS_COMPLETION flags is set and return
 *                       status is OI_STATUS_SEND_COMPLETE.
 *
 * @param cid            Local channel identifier (CID) of the connection to which to write
 *
 * @param outBuffer      Pointer to the output buffer containing the data to be written
 *
 * @param length         Number of bytes to write from the buffer
 *
 * @param writeFlags     OI_L2CAP_FLAGS_NONE or a logical OR of the following flags:
 *                       - OI_L2CAP_FLAGS_ALLOW_SYNCRONOUS_COMPLETION
 *                       - OI_L2CAP_FLAGS_ACCELERATE_WRITE_CFM
 *
 * @param queueFull      Pointer to a boolean value to return the queue status. Returns TRUE if the
 *                       queue is full or FALSE if another packet can be queued.
 *
 * @return               - OI_OK if the packet has been succesfully queued with L2CAP
 *
 *                       - OI_STATUS_SEND_COMPLETE if the OI_L2CAP_FLAGS_ALLOW_SYNCRONOUS_COMPLETION
 *                         flag was set and the data has been sent or queued in an internal buffer.
 *
 *                       - OI_STATUS_WRITE_IN_PROGRESS indicates that the write failed becauses the
 *                         queue was full. Caller can retry the write after a write confirm callback indicates
 *                         that the queue is no longer full.
 *
 *                       - Other status codes indicating that the write failed.
 */
OI_STATUS OI_L2CAP_QueueWrite(OI_L2CAP_DATA_QUEUE_WRITE_CFM callback,
                              OI_L2CAP_CID cid,
                              OI_UINT8 *outBuffer,
                              OI_UINT16 length,
                              OI_UINT8 writeFlags,
                              OI_BOOL *queueFull);

/**
 * This functions determines if a call to L2CAP_QueueWrite will succeed.
 *
 * @param cid            Local channel identifier (CID) of the connection to check.
 *
 * @return               - OI_OK if the packet can be written.
 *
 *                       - OI_STATUS_WRITE_IN_PROGRESS indicates that the write will fail becauses
 *                         there is a write in progress on this channel.
 *
 *                       - Other status codes indicating that the write will fail.
 */
OI_STATUS OI_L2CAP_CanWrite(OI_L2CAP_CID cid);

/**
 * This function sends an L2CAP echo request to the remote device.
 *
 * @param callback        Callback function to be called for confirmation that the echo has succeeded
 * @param context         Callers context will be returned in the callback function
 * @param bdaddr          Bluetooth device to which to send the echo request
 * @param echoData        Pointer to the buffer containing the (optional) echo data to send
 * @param length          Length of echoData in bytes
 */
OI_STATUS OI_L2CAP_EchoReq(OI_L2CAP_ECHO_RSP   callback,
                           void                *context,
                           const OI_BD_ADDR    *addr,
                           OI_UINT8            *echoData,
                           OI_UINT16           dataLen);

/**
 * This function performs an L2CAP information request.
 *
 * @param callback        Callback function to be called when the request has completed
 * @param context         Callers context will be returned in the callback function
 * @param bdaddr          Bluetooth device to which to send the information request
 * @param infoType        Type of information to get, valid types are defined in the Bluetooth specification.
 */
OI_STATUS OI_L2CAP_InfoReq(OI_L2CAP_INFO_RSP    callback,
                           void                *context,
                           const OI_BD_ADDR    *addr,
                           OI_UINT16            infoType);


/**
 * This function enables or disables packet flow when in any mode other than BASIC mode. If the
 * channel ID is for a channel configured for BASIC mode, this function will return an error status
 * code. In ERTM mode after flow has been turned off (control == OI_L2CAP_FLOW_OFF), L2CAP will stop
 * delivering data to the upper layer on the specified channel but flow-off does not necessarily
 * cause the L2CAP layer to send an RNR to the remote peer. Use the OI_L2CAP_FLOW_RNR value to force
 * L2CAP to send an RNR immediately.
 *
 * @param cid             Local channel identifier to set flow on
 * @param control         Enables or disables flow on the channel
 */
OI_STATUS OI_L2CAP_Flow(OI_L2CAP_CID          cid,
                        OI_L2CAP_FLOW_CONTROL control);


/**
 * This function associates an upper layer context with an L2CAP connection. Call
 * OI_L2CAP_GetContext() to retrieve the context. If the context is not NULL and
 * a context is already set, this function returns an error status.
 *
 * @param cid       Specifies the local channel identifier (CID) on which to set the context
 * @param context   Pointer to an upper layer context to set
 *
 * @return          OI_OK or an error status if the connection ID is invalid.
 */
OI_STATUS OI_L2CAP_SetContext(OI_L2CAP_CID cid,
                              void *context);


/**
 * This function returns an upper layer context associated with an L2CAP connection. Call
 * OI_L2CAP_SetContext() to set a context.
 *
 * @param cid       Specifies the local channel identifier (CID) on which to set the context
 * @param context   Pointer variable to return upper layer context pointer
 *
 * @return          OI_OK or an error status if the connection id is invalid.
 */
OI_STATUS OI_L2CAP_GetContext(OI_L2CAP_CID cid,
                              void **context);


/**
 * Get the remote BDAddr associated with a channel ID.
 *
 * @param cid       Specifies the local channel identifier.
 * @return          Remote BD address associated with connection oriented cid or NULL if no such channel.
 */
const OI_BD_ADDR * OI_L2CAP_GetBDAddr(OI_L2CAP_CID cid);


/**
 * This function returns the maximum value of the MTU currently be supported by L2CAP. This is
 * a compile time configuration parameter.
 */
OI_UINT16 OI_L2CAP_MaxSupportedMTU(void);


/**
 * This function requests that an L2CAP channel be created and associated with a given
 * fixed channel ID.  This function completes synchronously (i.e., does not call the
 * connect confirm callback) if the underlying ACL connection is already up and running.
 * If the ACL link is not established before calling this function, then the confirmCB
 * will be called once the ACL link is established and the fixed channel is created.
 *
 * This "sometimes synchronous" behavior is needed because OI_L2CAP_OpenFixedChannel is
 * called to create a "just-in-time" fixed channel when an incoming fixed channel packet is
 * received from a new remote addr. In this case, L2CAP has already reconstucted the packet
 * and is trying to process it. There is no opportunity to wait for callback in this case.
 * And, in fact, there is no reason to wait for one since all channel creation operations
 * other than ACL establishment can occur synchronously.
 *
 * Unlike dynamic L2CAP channels created with OI_L2CAP_Connect and OI_L2CAP_Accept, there is no
 * negociated configuration, PSM or Policy Management.
 *
 * @param confirmCB       Callback function called with the result of this connection request if
 *                        operation will complete asynchronously (return status equals OI_STATUS_PENDING).
 * @param disconnectInd   Callback function to indicate that the connection created by this request was disconnected
 * @param recvInd         Callback function to indicate that data was received on the connection created by this request
 * @param addr            Pointer to the Bluetooth device address of the remote device to which to connect
 * @param fixedCid        Channel identifier (as viewed from the network) for the new L2CAP channel.
 * @param params          Pointer to the structure containing the configuration parameters. A NULL value for this
 *                        pointer indicates that the default values are to be used.
 * @param cid             [out] Pointer to where the connection's channel handle will be written once the
 *                        connection has been created. This param is valid if the return status is OI_OK or
 *                        OI_STATUS_PENDING.
 *
 * @return                OI_OK if connection attempt was successful and function returned synchronously.
 *                        OI_STATUS_PENDING if operation will be completed asynchronously. In this case,
 *                        confirmCB will be called.
 */
OI_STATUS OI_L2CAP_OpenFixedChannel(OI_L2CAP_CONNECT_COMPLETE_CALLBACK    confirmCB,
                                    OI_L2CAP_DISCONNECT_IND               disconnectInd,
                                    OI_L2CAP_RECV_DATA_IND                recvInd,
                                    const OI_BD_ADDR                     *addr,
                                    OI_UINT16                             fixedCid,
                                    const OI_L2CAP_CONNECT_PARAMS        *params,
                                    OI_L2CAP_CID                         *cid);

/**
 * Close a fixed channel connection.
 * This function should only be used with channels created with OI_L2CAP_OpenFixedChannel
 *
 * @param cid   L2CAP channel handle to be closed
 */
OI_STATUS OI_L2CAP_CloseFixedChannel(OI_L2CAP_CID cid);


/**
 * Move an existing L2CAP channel to another AMP/BR-EDR controller.
 *
 * @param moveCompleteCB    Callback called when move is completed (successfully or unsuccessfully)
 * @param localCID          L2CAP channel ID of channel to be moved
 * @param remoteAmpID       ID of the remote AMP device to where the channel should be moved
 * @return OI_OK if successful
 */
OI_STATUS OI_L2CAP_MoveChannel(OI_L2CAP_MOVE_CFM moveCompleteCB,
                               OI_L2CAP_CID      localCID,
                               OI_AMP_ID         remoteAmpId);


/**
 * Accept or reject an incoming attempt to move an existing logical channel.
 * This function should only be called from within a registered implementation of
 * OI_L2CAP_MOVE_IND.
 *
 * @param moveCompleteCB   Callback called upon successful or unsuccessful completion of move operation.
 * @param cid              Channel ID involved in the move operation.
 * @param allowMove        TRUE if move attempt should be allowed. FALSE otherwise.
 */
OI_STATUS OI_L2CAP_AcceptMoveChannel(OI_L2CAP_MOVE_CFM moveCompleteCB,
                                     OI_L2CAP_CID      cid,
                                     OI_BOOL           allowMove);

/**
 * Given a local L2CAP CID as input, return the remote CID for the same
 * channel.
 *
 * @param local   Local CID designation for a currently existing L2CAP channel
 * @param remote  [OUT] Location to place remote L2CAP CID
 *
 * @return OI_OK  if channel exists
 */
OI_STATUS OI_L2CAP_GetRemoteCid(OI_L2CAP_CID      local,
                                OI_L2CAP_CID     *remote);


/*****************************************************************************/
#ifdef __cplusplus
}
#endif

/**@}*/

#endif /* _OI_L2CAP_H */

