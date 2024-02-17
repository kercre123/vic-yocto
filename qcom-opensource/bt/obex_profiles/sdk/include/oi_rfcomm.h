#ifndef _OI_RFCOMM_H
#define _OI_RFCOMM_H

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
 * This file provides the RFCOMM application programming interface (API).
 *
 * See the @ref RFCOMM_docpage section of the BLUEmagic 3.0 SDK documentation
 * for more information.
 */

#include "oi_common.h"
#include "oi_bt_stack_config.h"
#include "oi_connect_policy.h"
#include "oi_mbuf.h"
#include "oi_l2cap.h"

/** \addtogroup RFCOMM RFCOMM APIs */
/**@{*/

#ifdef __cplusplus
extern "C" {
#endif

/* RFCOMM frame size constants */
#define OI_RFCOMM_FRAMESIZE_DEFAULT        127   /**< Default RFCOMM frame size in bytes */
#define OI_RFCOMM_FRAMESIZE_MIN            23    /**< Minimum RFCOMM frame size in bytes */
#define OI_RFCOMM_FRAMESIZE_MAX            32767 /**< Maximum RFCOMM frame size in bytes */

/**
 * The overhead per RFCOMM frame. These values do not include L2CAP enveloping
 *
 * Address field    1 byte
 * Control field    1 byte
 * Credit field     0 or 1 byte  (depends if credits are piggybacking on packet)
 * Length field     1 or 2 bytes (1 byte if payload < 128, 2 bytes otherwise)
 * FCS              1 byte
 */
#define OI_RFCOMM_MTU_PER_MAXFRAME(x) (((x) >= 128) ? ((x) + 6) : ((x) + 5))
#define OI_RFCOMM_MAXFRAME_PER_MTU(x) (((x) <= (127 + 5)) ? ((x) - 5) : ((x) - 6))

#define OI_RFCOMM_MIN_MTU_FRAMESIZE     OI_RFCOMM_MAXFRAME_PER_MTU(OI_L2CAP_MTU_MIN)

/** Type definition for an RFCOMM link handle by which RFCOMM links are referenced. */
typedef OI_HANDLE OI_RFCOMM_LINK_HANDLE;

/**
   This structure is used to carry
   serial port parameters to be passed to the
   OI_RFCOMM_SetPortParams() function
 */
typedef struct {

    /** The bits set in this mask indicate which parameters are to be set. Bits
     * set to zero indicate that the parameters in this structure associated
     * with those bits are to be ignored. For example, a mask of 0x0001 in a
     * call to OI_RFCOMM_SetPortParams() will result in the bit rate for the
     * port being set to baudRate and every parameter other than baudRate being
     * ignored. Instead of using 0x0001 as a mask, the more human-readable name
     * OI_RFCOMM_MASK_BITRATE can be used. Default is OI_RFCOMM_MASK_ALL. */
    OI_INT16    mask;

    /** Bit rate; default: OI_RFCOMM_BAUDRATE_9600 */
    OI_BYTE    baudRate;

    /** Number of data bits; default: OI_RFCOMM_DATABIT_8 */
    OI_BYTE    dataBits;

    /** Number of stop bits; default: OI_RFCOMM_STOPBIT_1 */
    OI_BYTE    stopBits;

    /** Parity; default: OI_RFCOMM_NO_PARITY */
    OI_BYTE    parity;

    /** Type of parity; default: OI_RFCOMM_PARITY_ODD */
    OI_BYTE    parityType;

    /** Type of flow control; default: OI_RFCOMM_FLOW_NONE */
    OI_BYTE    flowControlType;

    /** Character to use for XON; default: 0x11 (DC1) */
    OI_BYTE    XON_Char;

    /** Character to use for XOFF; default: 0x13 (DC3) */
    OI_BYTE    XOFF_Char;

} OI_RFCOMM_PORTPARAM;

/* OI_RFCOMM_PORTPARAM constant definitions */

/**
 * @name Mask bits
 * These values are used for setting the mask parameter in OI_RFCOMM_PORTPARAM.
 */
/** @{ */
#define OI_RFCOMM_MASK_OI_BITRATE     OI_BIT0        /**< Mask bit definition for setting the bit rate parameter. */
#define OI_RFCOMM_MASK_DATA_OI_BITS   OI_BIT1        /**< Mask bit definition for setting the number of data bits parameter. */
#define OI_RFCOMM_MASK_STOP_OI_BITS   OI_BIT2        /**< Mask bit definition for setting the number of stop bits parameter.  */
#define OI_RFCOMM_MASK_PARITY      OI_BIT3        /**< Mask bit definition for setting the parity parameter.  */
#define OI_RFCOMM_MASK_PARITYTYPE  OI_BIT4        /**< Mask bit definition for setting the parity type parameter.  */
#define OI_RFCOMM_MASK_XON_CHAR    OI_BIT5        /**< Mask bit definition for setting the XON character parameter (RFCOMM v1.1 section 6) */
#define OI_RFCOMM_MASK_XOFF_CHAR   OI_BIT6        /**< Mask bit definition for setting the XOFF character parameter (RFCOMM v1.1 section 6) */
//          bit 7: reserved
#define OI_RFCOMM_MASK_XON_INPUT   OI_BIT8        /**< Mask bit definition for setting the XON/XOFF flow control on input (RFCOMM v1.1 section 6) */
#define OI_RFCOMM_MASK_XON_OUTPUT  OI_BIT9        /**< Mask bit definition for setting the XON/XOFF flow control on output (RFCOMM v1.1 section 6) */
#define OI_RFCOMM_MASK_RTR_INPUT   OI_BIT10       /**< Mask bit definition for setting the RTR/RTS flow control on input (BRFCOMM v1.1 section 6) */
#define OI_RFCOMM_MASK_RTR_OUTPUT  OI_BIT11       /**< Mask bit definition for setting the RTR/RTS flow control on output (RFCOMM v1.1 section 6) */
#define OI_RFCOMM_MASK_RTC_INPUT   OI_BIT12       /**< Mask bit definition for setting the RTC/CTS flow control on input (RFCOMM v1.1 section 6) */
#define OI_RFCOMM_MASK_RTC_OUTPUT  OI_BIT13       /**< Mask bit definition for setting the RTC/CTS flow control on output */
#define OI_RFCOMM_MASK_ALL         ((OI_UINT16) ~(OI_BIT7 | OI_BIT14 | OI_BIT15))   /**< definition for mask that indicates that all parameters are to be set */
/** @} */

/** @name Bit rate */
/** @{ */
#define OI_RFCOMM_BAUDRATE_2400    0           /**< Definition for bit rate parameter setting, indicating 2,400 bits per second. */
#define OI_RFCOMM_BAUDRATE_4800    1           /**< Definition for bit rate parameter setting, indicating 4,800 bits per second. */
#define OI_RFCOMM_BAUDRATE_7200    2           /**< Definition for bit rate parameter setting, indicating 7,200 bits per second. */
#define OI_RFCOMM_BAUDRATE_9600    3           /**< Definition for bit rate parameter setting, indicating 9,600 bits per second. */
#define OI_RFCOMM_BAUDRATE_19200   4           /**< Definition for bit rate parameter setting, indicating 19,200 bits per second. */
#define OI_RFCOMM_BAUDRATE_38400   5           /**< Definition for bit rate parameter setting, indicating 38,400 bits per second. */
#define OI_RFCOMM_BAUDRATE_57600   6           /**< Definition for bit rate parameter setting, indicating 57,600 bits per second. */
#define OI_RFCOMM_BAUDRATE_115200  7           /**< Definition for bit rate parameter setting, indicating 115,200 bits per second. */
#define OI_RFCOMM_BAUDRATE_230400  8           /**< Definition for bit rate parameter setting, indicating 230,400 bits per second. */
/** @} */

/** @name Number of data bits */
/** @{ */
#define OI_RFCOMM_DATABIT_5    0                     /**< Definition for number of data bits, indicating 5 data bits. */
#define OI_RFCOMM_DATAOI_BIT_6    OI_BIT1               /**< Definition for number of data bits, indicating 6 data bits. */
#define OI_RFCOMM_DATAOI_BIT_7    OI_BIT0               /**< Definition for number of data bits, indicating 7 data bits. */
#define OI_RFCOMM_DATAOI_BIT_8    ( OI_BIT0 | OI_BIT1 ) /**< Definition for number of data bits, indicating 8 data bits. */
/** @} */

/** @name Stop bits */
/** @{ */
#define OI_RFCOMM_STOPOI_BIT_1     0           /**< Definition for number of stop bits, indicating that 1 stop bit should be used. */
#define OI_RFCOMM_STOPOI_BIT_1_5   OI_BIT2     /**< Definition for number of stop bits, indicating that 1.5 stop bits should be used. */
/** @} */

/** @name Parity */
/** @{ */
#define OI_RFCOMM_NO_PARITY     0           /**< Definition for parity setting, indicating that parity checking is disabled */
#define OI_RFCOMM_PARITY        OI_BIT3     /**< Definition for parity setting, indicating that parity checking is enabled */
/** @} */

/** @name Parity type */
/** @{ */
#define OI_RFCOMM_PARITY_ODD    0                     /**< Definition for parity type, indicating that odd parity is used. */
#define OI_RFCOMM_PARITY_EVEN   OI_BIT5               /**< Definition for parity type, indicating that even parity is used. */
#define OI_RFCOMM_PARITY_MARK   OI_BIT4               /**< Definition for parity type, indicating that mark parity is used. */
#define OI_RFCOMM_PARITY_SPACE  ( OI_BIT4 | OI_BIT5 ) /**< Definition for parity type, indicating that space parity is used. */
/** @} */

/** @name Flow control */
/** @{ */
#define OI_RFCOMM_FLOW_NONE        0           /**< Definition for flow control type, indicating that no flow control is used. */
#define OI_RFCOMM_FLOW_XON_INPUT   OI_BIT0     /**< Definition for flow control type, indicating that XON/XOFF flow control is used on input. */
#define OI_RFCOMM_FLOW_XON_OUTPUT  OI_BIT1     /**< Definition for flow control type, indicating that XON/XOFF flow control is used on output. */
#define OI_RFCOMM_FLOW_RTR_INPUT   OI_BIT2     /**< Definition for flow control type, indicating that RTR/RTS flow control is used. */
#define OI_RFCOMM_FLOW_RTR_OUTPUT  OI_BIT3     /**< Definition for flow control type, indicating that RTR/RTS flow control is used. */
#define OI_RFCOMM_FLOW_RTC_INPUT   OI_BIT4     /**< Definition for flow control type, indicating that RTC/CTS flow control is used. */
#define OI_RFCOMM_FLOW_RTC_OUTPUT  OI_BIT5     /**< Definition for flow control type, indicating that RTC/CTS flow control is used. */
/** @} */

/**
 * @name Line status codes
 * These line status (error) codes are passed to OI_RFCOMM_SendLineStatus()
 */
/** @{ */
#define OI_RFCOMM_LINESTATUS_OVERRUN_ERR  (OI_BIT0 | OI_BIT1) /**< Line status: overrun error */
#define OI_RFCOMM_LINESTATUS_PARITY_ERR   (OI_BIT0 | OI_BIT2) /**< Line status: parity error */
#define OI_RFCOMM_LINESTATUS_FRAMING_ERR  (OI_BIT0 | OI_BIT3) /**< Line status: framing error  */
/** @} */

/**
 * @name Line control bits
 * These line control bits are passed to OI_RFCOMM_SetModemSignals()
 */
/** @{ */
#define OI_RFCOMM_LINE_RTC OI_BIT2 /**< RTC (Ready To Communicate) bit; maps to DSR/DTR */
#define OI_RFCOMM_LINE_RTR OI_BIT3 /**< RTR (Ready To Receive) bit; maps to RTS/CTS */
#define OI_RFCOMM_LINE_IC  OI_BIT6 /**< IC (Incoming Call) bit; maps to RI */
#define OI_RFCOMM_LINE_DV  OI_BIT7 /**< DV (Data Valid) bit; maps to DCD */
/** @} */

/************************************************************************

  callback function type definitions

 ************************************************************************/

/**
 * A callback function of this type confirms the establishment of a link to a
 * remote device.
 *
 * @param link       Specifies the link handle.
 *
 * @param frameSize  Frame size to be used for the link.
 *
 * @param result     OI_OK if successful; failure code otherwise.
 *
 */
typedef void (*OI_RFCOMM_CONNECT_CFM)(OI_RFCOMM_LINK_HANDLE link,
                                      OI_UINT16 frameSize,
                                      OI_STATUS result);

/**
 * A callback function of this type indicates that a remote application is
 * attempting to establish a link.
 *
 * @param addr        Pointer to the address of the remote Bluetooth device
 *                    attempting to establish a link
 *
 * @param channel     Specifies the server channel to which to connect.
 *
 * @param frameSize   Specifies the frame size for the incoming connection.
 *
 * @param link        Specifies the link handle for the connection.
 */
typedef void (*OI_RFCOMM_CONNECT_IND)(OI_BD_ADDR *addr,
                                      OI_UINT8 channel,
                                      OI_UINT16 frameSize,
                                      OI_RFCOMM_LINK_HANDLE link);

/**
 * A callback function of this type indicates that the specified link has been disconnected.
 *
 * @param linkHandle   Specifies the handle of the link that was disconnected.
 *
 * @param reason       Status code indicating the reason for the disconnection.
 */
typedef void (*OI_RFCOMM_DISCONNECT_IND)(OI_RFCOMM_LINK_HANDLE link,
                                         OI_STATUS reason);


/**
 * A callback function of this type confirms the remote port negotiation or request
 * for the remote port parameters.
 *
 * @param link  Specifies the link handle.
 *
 * @param port  Pointer to the structure containing the port parameters.
 */
typedef void (*OI_RFCOMM_PORT_PARAM_CFM)(OI_RFCOMM_LINK_HANDLE link,
                                         OI_RFCOMM_PORTPARAM *port);

/**
 * A callback function of this type indicates that the remote side of the
 * session is requesting to set the port parameters on the local side of the
 * virtual serial link. The implementation of this function should set the
 * parameters in the structure pointed to by the 'port' parameter to the desired
 * values before returning. Leaving a proposed parameter unchanged indicates
 * acceptance of that proposed parameter.
 *
 * @param link  Specifies the link handle.
 *
 * @param port  Pointer to the structure containing the port parameters.
 */
typedef void (*OI_RFCOMM_PORT_PARAM_IND)(OI_RFCOMM_LINK_HANDLE link,
                                         OI_RFCOMM_PORTPARAM *port);

/**
 * A callback function of this type indicates the control signals set by the
 * remote device by means of the function OI_RFCOMM_SetModemSignals().
 *
 * @param link          Specifies the link handle for the link on which the signals were set.
 *
 * @param lineControl   State of the line control bits.
 *
 * @param breakControl  State of the break control bits.
 *
 */
typedef void (*OI_RFCOMM_LINE_CONTROL_IND)(OI_RFCOMM_LINK_HANDLE link,
                                           OI_BYTE lineControl,
                                           OI_BYTE breakControl);

/**
 * A callback function of this type indicates the line status sent by the remote
 * device.
 *
 * @param link            Specifies the link handle.
 *
 * @param lineStatus      Specifies the line status.
 */
typedef void (*OI_RFCOMM_LINE_STATUS_IND)(OI_RFCOMM_LINK_HANDLE link,
                                          OI_BYTE lineStatus);

/**
 * A callback function of this type confirms that data was written to the
 * specified link.
 *
 * @param link         Specifies the link handle for the link to where the data was written.
 *
 * @param dataBuf      Pointer to the buffer containing the data to be written.
 *
 * @param dataLen      Specifies the number of bytes written, always the same as the number
 *                     of bytes passed to RFCOMM_Write().
 */
typedef void (*OI_RFCOMM_WRITE_CFM)(OI_RFCOMM_LINK_HANDLE link,
                                    OI_UINT8 *dataBuf,
                                    OI_UINT16 dataLen,
                                    OI_STATUS result);


/**
 * A callback function of this type is called when the write to the MBUF has completed.
 */
typedef void (*OI_RFCOMM_WRITE_MBUF_CFM)(OI_RFCOMM_LINK_HANDLE link,
                                         OI_MBUF *mbuf,
                                         OI_STATUS result);


/**
 * A callback function of this type indicates the receipt of data on the
 * specified link.
 *
 * @param link        Specifies the link handle for the link on which the data was received.
 *
 * @param dataBuf     Pointer to the buffer containing the received data.
 *
 * @param dataLen     Specifies the number of bytes received.
 */
typedef void (*OI_RFCOMM_RECV_DATA_IND)(OI_RFCOMM_LINK_HANDLE link,
                                        OI_BYTE *dataBuf,
                                        OI_UINT16 dataLen);

/**
 * A callback function of this type confirms receipt of a test response from a
 * remote device.
 *
 * @param addr         Pointer to the address of the remote Bluetooth
 *                     device.
 *
 * @param testPattern  Pointer to the buffer containing the test pattern data
 *                     received from the remote device.
 *
 * @param length       Specifies the number of bytes in the test pattern.
 */
typedef void (*OI_RFCOMM_TEST_CFM)(OI_BD_ADDR *addr,
                                   OI_UINT8 *testPattern,
                                   OI_UINT16 length);

/**
   This structure groups together the indication callbacks
   associated with a link and is passed to OI_RFCOMM_Connect()
   by a client application or OI_RFCOMM_RegisterServer() by a
   server application.
 */
typedef struct {

    /** The callback function to be called when a link is
        released. */
    OI_RFCOMM_DISCONNECT_IND disconnectInd;

    /** The callback function to be called when data is received. */
    OI_RFCOMM_RECV_DATA_IND recvDataInd;

    /** The callback function to be called to indicate the line
       status sent by the remote device. (It is not necessary to
       register this type of callback function, so this may be
       NULL.) */
    OI_RFCOMM_LINE_STATUS_IND lineStatusInd;

    /** The callback function to be called to indicate the state of
       the control lines. (It is not necessary to register this type
       of callback function, so this may be NULL.) */
    OI_RFCOMM_LINE_CONTROL_IND lineControlInd;

    /** The callback function to be called to indicate that a remote
       device wishes to configure a port. (It is not necessary to
       register this type of callback function, so this may be
       NULL.) */
    OI_RFCOMM_PORT_PARAM_IND portParamInd;

} OI_RFCOMM_LINK_INDICATIONS;


/************************************************************************

  functions

 ************************************************************************/

/**
 * This function registers a server application with the RFCOMM layer and
 * obtains a server channel number.
 *
 * @param callback      The indication callback function used to notify the
 *                      server of a client application attempting to connect
 *                      @param indications pointer to a structure containing
 *                      indication callback functions to be used for any new
 *                      link
 *
 * @param maxFrameSize  This indicates the maximum frame size that can be
 *                      received on links with this server application. The
 *                      negotiated frame size (which is reported by the
 *                      OI_RFCOMM_CONNECT_IND callback function) may be smaller
 *                      than this.
 *
 * @param bufSize       This indicates the size of buffer to use for flow
 *                      control on a link created with this server. This
 *                      buffering will be used for storing received data when
 *                      incoming flow is disabled. To achieve optimal use of
 *                      buffer space, this number should be a multiple of the
 *                      frame size. If the buffer size is 0, then no buffer
 *                      space will be allocated for the link and the application
 *                      should not invoke OI_RFCOMM_FlowEnable() with the
 *                      Boolean parameter FALSE.
 *
 * @param serverChannel Pointer to the location where the server channel number
 *                      assigned to the application will be written. If the input
 *                      value is a valid RFCOMM channel number (1 - 30), this
 *                      channel number will be used in preference to any other.
 *
 * @param policy        Specifies the required connection policy.
 *
 * @return              OI_OK if successful; OI_STATUS_NO_RESOURCES if no more
 *                      application registrations are allowed.
 */
OI_STATUS OI_RFCOMM_RegisterServer(OI_RFCOMM_CONNECT_IND callback,
                                   const OI_RFCOMM_LINK_INDICATIONS *indications,
                                   OI_UINT16 maxFrameSize,
                                   OI_UINT16 bufSize,
                                   OI_UINT8 *serverChannel,
                                   const OI_CONNECT_POLICY *policy);

/**
 * This function deregisters a server application with the RFCOMM layer.
 *
 * @param serverChannel  Specifies the server channel number of the server to deregister.
 *
 * @return               OI_OK if successful; OI_RFCOMM_INVALID_CHANNEL if the
 *                       server channel number is invalid.
 */
OI_STATUS OI_RFCOMM_DeregisterServer(OI_UINT8 serverChannel);

/**
 * This function establishes an RFCOMM link with a remote server.
 *
 * @param callback       Confirmation callback function to be called when
 *                       the link is established.
 *
 * @param indications    Pointer to a structure containing indication callback
 *                       functions to be used for this link.
 *
 * @param addr           Pointer to the Bluetooth device address of the remote
 *                       device to which to connect.
 *
 * @param serverChannel  Specifies the server channel to which to connect on the remote device.
 *
 * @param maxFrameSize   Indicates the maximum frame size that can be
 *                       received on this link. The negotiated frame size (which
 *                       is reported by the OI_RFCOMM_CONNECT_IND callback
 *                       function) may be smaller than this.
 *
 * @param bufSize        Indicates the size of buffer to use for flow
 *                       control on this link. This buffering will be used for
 *                       storing received data when incoming flow is disabled.
 *                       To achieve optimal use of buffer space, this number
 *                       should be a multiple of the frame size. If the buffer
 *                       size is 0, then no buffer space will be allocated for
 *                       the link and the application should not invoke
 *                       OI_RFCOMM_FlowEnable() with the Boolean parameter FALSE.
 *
 * @param handle         Pointer to the buffer where the link handle will be written.
 *                       This link handle can be used to reference the link once
 *                       a success (OI_OK) status is returned and the
 *                       OI_RFCOMM_CONNECT_CFM confirmation callback function
 *                       has been called.
 *
 * @param policy         Specifies the required connection policy for this connection.
 *
 * @return               OI_OK if successful
 */
OI_STATUS OI_RFCOMM_Connect(OI_RFCOMM_CONNECT_CFM callback,
                            const OI_RFCOMM_LINK_INDICATIONS *indications,
                            OI_BD_ADDR *addr,
                            OI_UINT8 serverChannel,
                            OI_UINT16 maxFrameSize,
                            OI_UINT16 bufSize,
                            OI_RFCOMM_LINK_HANDLE *handle,
                            const OI_CONNECT_POLICY *policy);

/**
 * This function is used to respond to an indication that a remote application
 * is attempting to establish a link. No connection parameters are passed in
 * here, since connection parameters (frame size and buffer size) were already
 * configured in OI_RFCOMM_RegisterServer().
 *
 * @param callback   Specifies the confirmation callback to call when the connection has been
 *                   established.
 *
 * @param link       Specifies the link handle for the connection.
 *
 * @param accept     TRUE to accept and establish the link; FALSE to reject.
 */
OI_STATUS OI_RFCOMM_Accept( OI_RFCOMM_CONNECT_CFM callback,
                            OI_RFCOMM_LINK_HANDLE link,
                            OI_BOOL accept);

/**
 * This function terminates the specified RFCOMM link. The disconnect indication
 * callback function passed to OI_RFCOMM_Accept() or OI_RFCOMM_Connect() will be
 * called when the disconnection is complete.
 *
 * @param link       Specifies the link handle of the link to be terminated.
 *
 * @return           OI_OK if successful.
 */
OI_STATUS OI_RFCOMM_Disconnect(OI_RFCOMM_LINK_HANDLE link);

/**
 * This function writes data to the specified RFCOMM link. Once this function
 * has been called with a given link handle, it should not be called again with
 * the same link handle before receiving an OI_RFCOMM_WRITE_CFM
 * confirmation callback function for that link. Concurrent writes to the same
 * link handle without an intervening write confirmation callback will result in
 * an error return status from OI_RFCOMM_Write().
 *
 * The argument dataLen must be less than or equal to the negotiated maximum RFCOMM frame
 * size, maxFrameSize. Use the function OI_RFCOMM_WriteSegmented() to send data
 * with a dataLen larger than maxFrameSize.
 *
 * @param callback    Specifies the callback function for confirming the write.
 *
 * @param link        Specifies the link handle of the link to which to write data.
 *
 * @param data        Pointer to the output buffer containing the data to write.
 *
 * @param dataLen     Specifies the number of bytes to send from the data output buffer.
 *
 * @return            OI_OK if successful.
 */
OI_STATUS OI_RFCOMM_Write(OI_RFCOMM_WRITE_CFM callback,
                          OI_RFCOMM_LINK_HANDLE link,
                          OI_UINT8 *data,
                          OI_UINT16 dataLen);

/**
 * This function sends an entire buffer of data. Unlike OI_RFCOMM_Write(),
 * this function accepts a buffer with a length greater than the frame size
 * for the specified link. If dataLen is greater than the frame size, RFCOMM
 * will perform segmentation internally, sending multiple frames of data no
 * more than the frame size in length. The callback will be called once the entire
 * contents of the buffer have been sent.
 *
 * @param callback    Specifies the callback function for confirming the write.
 *
 * @param link        Specifies the link handle of the link to which to write data.
 *
 * @param data        Pointer to the output buffer containing the data to write.
 *
 * @param dataLen     Number of bytes to send from the data output buffer.
 */
OI_STATUS OI_RFCOMM_WriteSegmented(OI_RFCOMM_WRITE_CFM callback,
                                   OI_RFCOMM_LINK_HANDLE link,
                                   OI_UINT8 *data,
                                   OI_UINT16 dataLen);


/**
 * Write to an MBUF. RFCOMM will segment the write into multiple frames as required.
 *
 * @param callback  Specifies the callback function that will be called when the MBUF has been completely
 *                  transmitted.
 *
 * @param link      Specifies the RFCOMM link on which to transmit the data.
 *
 * @param fastCfm   Requests that RFCOMM confirm the write as quickly as possible. This may require
 *                  data to be copied so this should only be done when there is no response expected
 *                  from the remote device.
 *
 * @param mbuf      The mbuf to send.
 */
OI_STATUS OI_RFCOMM_WriteMBUF(OI_RFCOMM_WRITE_MBUF_CFM callback,
                              OI_RFCOMM_LINK_HANDLE link,
                              OI_BOOL fastCfm,
                              OI_MBUF *mbuf);


/**
 * This function requests that the port parameters for the remote side of the
 * virtual serial link specified by the OI_RFCOMM_LINK_HANDLE parameter be
 * set to the values indicated by the OI_RFCOMM_PORTPARAM structure. A mask in
 * this structure indicates which parameters are to be set and which are to be
 * ignored. The result of the request will be reported in the callback.
 *
 * @param callback    Specifies the confirmation callback function to be called when the
 *                    parameter setting function has completed.
 *
 * @param link        Specifies the link handle associated with the remote serial port for
 *                    which the configuration is being set.
 *
 * @param port        Pointer to the structure containing the port parameters
 *
 */
OI_STATUS OI_RFCOMM_SetPortParams(OI_RFCOMM_PORT_PARAM_CFM callback,
                                  OI_RFCOMM_LINK_HANDLE link,
                                  OI_RFCOMM_PORTPARAM *port);

/**
 * This function requests a read of the port parameters for the remote side of
 * the virtual serial link specified by the OI_RFCOMM_LINK_HANDLE.
 *
 * @param callback    Specifies the confirmation callback function.
 *
 * @param handle      Specifies the link handle associated with the serial port for which
 *                    the remote configuration is being read.
 */
OI_STATUS OI_RFCOMM_GetPortParams(OI_RFCOMM_PORT_PARAM_CFM callback,
                                  OI_RFCOMM_LINK_HANDLE handle);

/**
 * This function sets the modem control signals for the specified
 * link. If connected, the modem control signals will be sent to
 * the remote side. If not, the signal's values will be cached and
 * sent to the remote side after the SAMB/UA exchange establishing the RFCOMM
 * multiplexer.
 *
 * This function can be called any time the application has a
 * valid RFCOMM link handle. The earliest point in time when the application
 * has a valid link handle is either:
 *
 * - upon return from OI_RFCOMM_Connect() (for an outgoing connection), or
 * - upon #OI_RFCOMM_CONNECT_IND callback (for an incoming connection).
 *
 * @param link         Specifies the link handle of the link for which to set the signals.
 *
 * @param lineControl  Carries the line control bits. This should be
 *                     the logical OR of any of:
 *                     - OI_RFCOMM_LINE_RTC
 *                     - OI_RFCOMM_LINE_RTR
 *                     - OI_RFCOMM_LINE_IC
 *                     - OI_RFCOMM_LINE_DV
 *
 * @param breakControl  Break control bits
 */
OI_STATUS OI_RFCOMM_SetModemSignals(OI_RFCOMM_LINK_HANDLE link,
                                    OI_INT8 lineControl,
                                    OI_INT8 breakControl);

/**
 * This function sends the specified line status code to the remote device.
 *
 * @param link        Specifies the link handle of the link on which to send the status
 *                    code.
 *
 * @param lineStatus  Specifies the line status code to send.
 */
OI_STATUS OI_RFCOMM_SendLineStatus(OI_RFCOMM_LINK_HANDLE link,
                                   OI_BYTE lineStatus);


/**
 * This function tests the data link to the specified device by sending a test
 * data pattern. The remote device should write the same test pattern back. The
 * callback function confirms that the response has been received.
 *
 * @param callback     Specifies the confirmation callback function to be called when the
 *                     test response is received.
 *
 * @param addr         Pointer to the address of the remote Bluetooth
 *                     device.
 *
 * @param testPattern  Pointer to the buffer containing the test pattern data to
 *                     be sent to the remote device.
 *
 * @param length       Specifies the number of bytes in the test pattern, which must not
 *                     be greater than 255.
 */
OI_STATUS OI_RFCOMM_Test(OI_RFCOMM_TEST_CFM callback,
                         OI_BD_ADDR *addr,
                         OI_BYTE *testPattern,
                         OI_UINT8 length);

/**
 * Indicate whether credit-based flow control is being used on this link.
 * If credit-based flow control is not being used, OI_RFCOMM_FlowEnable will
 * use the GSM TS 7.10 styles of flow control (specifically using the Modem
 * Status Command) which may mean that packets will arrive at the application after
 * OI_RFCOMM_FlowEnable has been issued, but before the remote device has processed
 * the MSC.
 * This function may be called successfully during the OI_RFCOMM_CONNECT_CFM or
 * anytime afterwards.
 *
 * @param handle  Specifies the link handle to query.
 *
 * @return  TRUE if using credit-based flow control.
 */
OI_BOOL OI_RFCOMM_CreditBasedFlowControl(OI_RFCOMM_LINK_HANDLE handle);


/**
 * Enables or disables the flow of data sent from RFCOMM to the
 * application. An application that cannot receive more data should call
 * OI_RFCOMM_FlowEnable() with the Boolean parameter <code>enable</code>
 * set to FALSE.  This will pause the flow of data from RFCOMM to the
 * application. See OI_RFCOMM_CreditBasedFlowControl for further discussion
 * under links that do not support credit based flow control.
 *
 * @param handle  Specifies the link handle for the link on which to enable or disable flow.
 *
 * @param enable  TRUE to enable flow, FALSE to disable flow.
*/
OI_STATUS OI_RFCOMM_FlowEnable(OI_RFCOMM_LINK_HANDLE handle,
                               OI_BOOL enable );


/**
 * This function returns the SCN (server channel number) aka RFCOMM DLCI for a given
 * rfcomm link handle. This API is provided for BREW compatibility.
 *
 * @param handle    Specifies the RFCOMM link handle.
 *
 * @param[out] pSCN     Pointer to the caller's var where the SCN will be stored.
 *
 * @return          OI_OK if successful. Any other return value indicates failure and
 *                  *pSCN is unchanged.
 */

OI_STATUS OI_RFCOMM_GetSCNByHandle(OI_RFCOMM_LINK_HANDLE handle,
                                   OI_UINT8 *pSCN);


/**
 * Associates a caller defined context with a registered RFCOMM server channel.
 * This context can then be retrieved by calling OI_RFCOMM_GetServerContext.
 *
 * This function should only be used by the application or profile that is
 * in direct control of the RFCOMM server for the requested channel. Any
 * other manipulation of the context value will confuse the profile that
 * expected to have control of the context.
 *
 * @param serverChannel   Specifies a valid RFCOMM channel number for a registered RFCOMM
 *                        server.
 *
 * @param context         Value supplied by the caller.
 *
 * @return                OI_OK if the context was set, OI_STATUS_NOT_FOUND if
 *                        serverChannel is not valid.
 */
OI_STATUS OI_RFCOMM_SetServerContext(OI_UINT8 serverChannel,
                                     void *context);


/**
 * Gets a caller defined context from a registered RFCOMM server channel. This
 * is a value that we previously set by a call to OI_RFCOMM_SetServerContext.
 *
 * @param serverChannel   Valid RFCOMM channel number for a registered RFCOMM
 *                        server.
 *
 * @return                Context pointer or NULL if the handle is invalid or
 *                        there is no context associated with this server
 *                        channel.
 */
void* OI_RFCOMM_GetServerContext(OI_UINT8 serverChannel);


/**
 * Associates a caller defined context with an RFCOMM link. This context can
 * then be retrieved by calling OI_RFCOMM_GetLinkContext.
 *
 * This function should only be used by the application or profile
 * that is in direct control of the RFCOMM link. Any other
 * manipulation of the context value will confuse the profile that
 * expected to have control of the context.
 *
 * @param handle          Specifies the RFCOMM link handle to which to associate the context.
 *
 * @param context         Value supplied by the caller.
 *
 * @return                OI_OK if the context was set, OI_STATUS_NOT_FOUND if
 *                        the link handle is not valid.
 */
OI_STATUS OI_RFCOMM_SetLinkContext(OI_RFCOMM_LINK_HANDLE handle,
                                   void *context);


/**
 * Gets a caller defined context associate with an RFCOMM link. This is a value
 * that we previously set by a call to OI_RFCOMM_SetLinkContext.
 *
 * @param handle          Specifies the RFCOMM link handle from which to get the context.
 *
 * @return                Context pointer or NULL if the handle is invalid or
 *                        there is no context associated with this handle.
 */
void* OI_RFCOMM_GetLinkContext(OI_RFCOMM_LINK_HANDLE handle);



/**
 * Given an RFCOMM link, this function returns the L2CAP channel ID for the RFCOMM session for this
 * link.
 *
 * @param handle      Specifies the RFCOMM link handle to use.
 * @param cid         Pointer to out parameter for the L2CAP CID
 *
 * @return      - OI_OK if the L2CAP channel id was returned succesfully
 *              - OI_RFCOMM_LINK_NOT_FOUND if the handle is invalid
 *              - OI_STATUS_NOT_CONNECTED if the RFCOMM session is not connected or is disconnecting
 *
 *
 */
OI_STATUS OI_RFCOMM_GetL2capCID(OI_RFCOMM_LINK_HANDLE handle,
                                OI_L2CAP_CID *cid);


/**
 * Indicate to RFCOMM whether it should use ERTM mode for L2CAP connection.
 *
 * @param ertm   Specifies whether RFCOMM should specifically request ERTM mode for its L2CAP connection
 *               or allow L2CAP to choose the mode.
 *
 */
void OI_RFCOMM_SetL2capERTM(OI_BOOL ertm);

#ifdef __cplusplus
}
#endif

/**@}*/

#endif /* _OI_RFCOMM_H */
