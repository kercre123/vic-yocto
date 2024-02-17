#ifndef _OI_BT_ASSIGNED_NOS_H
#define _OI_BT_ASSIGNED_NOS_H

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

    This file contains constant definitions for Bluetooth assigned numbers, including
    Bluetooth device types, L2CAP PSMs (protocol/service multiplexors), and
    SDP UUIDs (universally unique identifiers) and Attribute Identifiers.

    For up-to-date information on these various assigned numbers, see the Bluetooth
    wireless technology "Assigned Numbers" specification, which is no longer part of
    the core Bluetooth specification but is maintained on the Bluetooth SIG website at
    http://www.bluetooth.org/assigned-numbers/.

*/

#include "oi_stddefs.h"
#include "oi_bt_spec.h"

#ifdef __cplusplus
extern "C" {
#endif

/** \addtogroup Misc Miscellaneous APIs */
/**@{*/

/**
 * @name Class of Device/Service field
 * The Class of Device/Service bitfield indicates Bluetooth device type and
 * is composed by using the ClassOfDevice macro with service class, major device
 * class, and minor device class values as input.
 * @{
 */

/** Create a class of device field from major service, major device, and minor device numbers. */
#define OI_CLASS_OF_DEVICE(majserv, majdev, mindev) ((majserv) | (majdev) | (mindev))

/**
 * Extract the Major Service Class from a class of device.
 */
#define OI_GET_MAJOR_SERVICE(cod)  ((cod) & OI_BT_DEV_CLASS_MAJOR_SERVICE_MASK)

/**
 * Extract the Major Device Class from a class of device.
 */
#define OI_GET_MAJOR_DEVICE(cod) ((cod) & OI_BT_DEV_CLASS_MAJOR_DEVICE_MASK)

/**
 * Extract the Minor Device Class from a class of device.
 */
#define OI_GET_MINOR_DEVICE(cod) ((cod) & OI_BT_DEV_CLASS_MINOR_DEVICE_MASK)

/** Convert a number to a major device class value. */
#define OI_BT_MAJOR_DEVICE_CLASS(x) ((x) << 8)

/** Convert a number to a minor device class value. */
#define OI_BT_MINOR_DEVICE_CLASS(x) ((x) << 2)

/**@}*/


/**
 * @name Service class
 * This field indicates type of service.
 * @note OI_BIT14 and OI_BIT15 are reserved.
 * @{
 */

#define OI_SERVICE_CLASS_LIMITED_DISCOVERABLE  OI_BIT13
//OI_BIT14 and OI_BIT15 reserved
#define OI_SERVICE_CLASS_POSITIONING           OI_BIT16
#define OI_SERVICE_CLASS_NETWORKING            OI_BIT17
#define OI_SERVICE_CLASS_RENDERING             OI_BIT18
#define OI_SERVICE_CLASS_CAPTURING             OI_BIT19
#define OI_SERVICE_CLASS_OBJECT_TRANSFER       OI_BIT20
#define OI_SERVICE_CLASS_AUDIO                 OI_BIT21
#define OI_SERVICE_CLASS_TELEPHONY             OI_BIT22
#define OI_SERVICE_CLASS_INFORMATION           OI_BIT23

/**@}*/

/**
 * @name Major device class
 * This field indicates major device class.
 * @{
 */

#define OI_BT_MAJOR_DEVICE_CLASS_MISCELLANEOUS      OI_BT_MAJOR_DEVICE_CLASS(0)
#define OI_BT_MAJOR_DEVICE_CLASS_COMPUTER           OI_BT_MAJOR_DEVICE_CLASS(1)
#define OI_BT_MAJOR_DEVICE_CLASS_PHONE              OI_BT_MAJOR_DEVICE_CLASS(2)
#define OI_BT_MAJOR_DEVICE_CLASS_ACCESS_POINT       OI_BT_MAJOR_DEVICE_CLASS(3)
#define OI_BT_MAJOR_DEVICE_CLASS_AUDIO_VIDEO        OI_BT_MAJOR_DEVICE_CLASS(4)
#define OI_BT_MAJOR_DEVICE_CLASS_PERIPHERAL         OI_BT_MAJOR_DEVICE_CLASS(5)
#define OI_BT_MAJOR_DEVICE_CLASS_IMAGING            OI_BT_MAJOR_DEVICE_CLASS(6)
#define OI_BT_MAJOR_DEVICE_CLASS_WEARABLE           OI_BT_MAJOR_DEVICE_CLASS(7)
#define OI_BT_MAJOR_DEVICE_CLASS_UNCATEGORIZED      OI_BT_MAJOR_DEVICE_CLASS(31)


/**@}*/

/**
 * @name Minor device class: uncategorized
 * This value indicates that the device's minor device class is uncategorized.
 * @{
 */

#define OI_BT_MINOR_DEVICE_CLASS_UNCATEGORIZED      0

/**@}*/

/**
 * @name Minor device class within computer major device class
 * This field indicates minor device class within the computer major device class.
 * @{
 */

#define OI_BT_MINOR_DEVICE_CLASS_DESKTOP            OI_BT_MINOR_DEVICE_CLASS(1)
#define OI_BT_MINOR_DEVICE_CLASS_SERVER             OI_BT_MINOR_DEVICE_CLASS(2)
#define OI_BT_MINOR_DEVICE_CLASS_LAPTOP             OI_BT_MINOR_DEVICE_CLASS(3)
#define OI_BT_MINOR_DEVICE_CLASS_HANDHELD           OI_BT_MINOR_DEVICE_CLASS(4)
#define OI_BT_MINOR_DEVICE_CLASS_PALM_SIZED         OI_BT_MINOR_DEVICE_CLASS(5)
#define OI_BT_MINOR_DEVICE_CLASS_WEARABLE           OI_BT_MINOR_DEVICE_CLASS(6)

/**@}*/

/**
 * @name Minor device class within phone major device class
 * This field indicates minor device class within the phone major device class.
 * @{
 */

#define OI_BT_MINOR_DEVICE_CLASS_CELLULAR           OI_BT_MINOR_DEVICE_CLASS(1)
#define OI_BT_MINOR_DEVICE_CLASS_CORDLESS           OI_BT_MINOR_DEVICE_CLASS(2)
#define OI_BT_MINOR_DEVICE_CLASS_SMARTPHONE         OI_BT_MINOR_DEVICE_CLASS(3)
#define OI_BT_MINOR_DEVICE_CLASS_MODEM              OI_BT_MINOR_DEVICE_CLASS(4) /**< Wired modem or voice gateway */
#define OI_BT_MINOR_DEVICE_CLASS_ISDN_ACCESS        OI_BT_MINOR_DEVICE_CLASS(5)

/**@}*/

/**
 * @name Minor device class within audio/video major device class
 * This field indicates minor device class within the audio/video major device class.
 * @note Values 3 and 17 are reserved.
 * @{
 */

#define OI_BT_MINOR_DEVICE_CLASS_HEADSET            OI_BT_MINOR_DEVICE_CLASS(1)
#define OI_BT_MINOR_DEVICE_CLASS_HANDS_FREE         OI_BT_MINOR_DEVICE_CLASS(2)
                                          //3 reserved
#define OI_BT_MINOR_DEVICE_CLASS_MICROPHONE         OI_BT_MINOR_DEVICE_CLASS(4)
#define OI_BT_MINOR_DEVICE_CLASS_LOUDSPEAKER        OI_BT_MINOR_DEVICE_CLASS(5)
#define OI_BT_MINOR_DEVICE_CLASS_HEADPHONES         OI_BT_MINOR_DEVICE_CLASS(6)
#define OI_BT_MINOR_DEVICE_CLASS_PORTABLE_AUDIO     OI_BT_MINOR_DEVICE_CLASS(7)
#define OI_BT_MINOR_DEVICE_CLASS_CAR_AUDIO          OI_BT_MINOR_DEVICE_CLASS(8)
#define OI_BT_MINOR_DEVICE_CLASS_SET_TOP_BOX        OI_BT_MINOR_DEVICE_CLASS(9)
#define OI_BT_MINOR_DEVICE_CLASS_HIFI_AUDIO         OI_BT_MINOR_DEVICE_CLASS(10)
#define OI_BT_MINOR_DEVICE_CLASS_VCR                OI_BT_MINOR_DEVICE_CLASS(11)
#define OI_BT_MINOR_DEVICE_CLASS_VIDEO_CAMERA       OI_BT_MINOR_DEVICE_CLASS(12)
#define OI_BT_MINOR_DEVICE_CLASS_CAMCORDER          OI_BT_MINOR_DEVICE_CLASS(13)
#define OI_BT_MINOR_DEVICE_CLASS_VIDEO_MONITOR      OI_BT_MINOR_DEVICE_CLASS(14)
#define OI_BT_MINOR_DEVICE_CLASS_VIDEO_DISPLAY      OI_BT_MINOR_DEVICE_CLASS(15)
#define OI_BT_MINOR_DEVICE_CLASS_VIDEO_CONFERENCING OI_BT_MINOR_DEVICE_CLASS(16)
/* #define OI_BT_MINOR_DEVICE_CLASS_RESERVED        OI_BT_MINOR_DEVICE_CLASS(17) */
#define OI_BT_MINOR_DEVICE_CLASS_VIDEO_GAMING       OI_BT_MINOR_DEVICE_CLASS(18)

/**@}*/


/**
 * @name Minor device class within peripheral major device class
 * This field indicates minor device class within the peripheral major device class.
 * @{
 */

#define OI_BT_MINOR_DEVICE_CLASS_KEYBOARD           OI_BIT6
#define OI_BT_MINOR_DEVICE_CLASS_POINTING_DEVICE    OI_BIT7
#define OI_BT_MINOR_DEVICE_CLASS_COMBO_KEYBOARD_POINTING  (OI_BIT6 | OI_BIT7)

#define OI_BT_MINOR_DEVICE_CLASS_JOYSTICK           OI_BT_MINOR_DEVICE_CLASS(1)
#define OI_BT_MINOR_DEVICE_CLASS_GAMEPAD            OI_BT_MINOR_DEVICE_CLASS(2)
#define OI_BT_MINOR_DEVICE_CLASS_REMOTE_CONTROL     OI_BT_MINOR_DEVICE_CLASS(3)
#define OI_BT_MINOR_DEVICE_CLASS_SENSING_DEVICE     OI_BT_MINOR_DEVICE_CLASS(4)
#define OI_BT_MINOR_DEVICE_CLASS_DIGITIZER_TABLET   OI_BT_MINOR_DEVICE_CLASS(5)

/**@}*/

/**
 * @name Minor device class within imaging major device class
 * This field indicates minor device class within the imaging major device class.
 * @{
 */

#define OI_BT_MINOR_DEVICE_CLASS_DISPLAY  OI_BIT4
#define OI_BT_MINOR_DEVICE_CLASS_CAMERA   OI_BIT5
#define OI_BT_MINOR_DEVICE_CLASS_SCANNER  OI_BIT6
#define OI_BT_MINOR_DEVICE_CLASS_PRINTER  OI_BIT7

/**
 * @name Minor device class within Wearable major device class
 * This field indicates minor device class within the Wearable major device class.
 * @{
 */

#define OI_BT_MINOR_DEVICE_CLASS_WRISTWATCH         OI_BT_MINOR_DEVICE_CLASS(1)
#define OI_BT_MINOR_DEVICE_CLASS_PAGER              OI_BT_MINOR_DEVICE_CLASS(2)
#define OI_BT_MINOR_DEVICE_CLASS_JACKET             OI_BT_MINOR_DEVICE_CLASS(3)
#define OI_BT_MINOR_DEVICE_CLASS_HELMET             OI_BT_MINOR_DEVICE_CLASS(4)
#define OI_BT_MINOR_DEVICE_CLASS_GLASSES            OI_BT_MINOR_DEVICE_CLASS(5)

/**@}*/


/**
 * @name Protocol/Service Multiplexer Definitions.
 * @{
 * These numbers identify protocols to L2CAP and to L2CAP client protocols.
 */

#define OI_PSM_SDP                 0x0001 /**< Service Discovery Protocol (SDP) */
#define OI_PSM_RFCOMM              0x0003 /**< RFCOMM */
#define OI_PSM_TCS                 0x0005 /**< Telephony Control protocol Specification (TCS) */
#define OI_PSM_TCS_CORDLESS        0x0007 /**< Telephony Control protocol Specification (TCS), cordless */
#define OI_PSM_BNEP                0x000F /**< Bluetooth Network Encapsulation Protocal */
#define OI_PSM_HID_CONTROL         0x0011 /**< Human Interface Device Control */
#define OI_PSM_HID_INTERRUPT       0x0013 /**< Human Interface Device Interrupt */
#define OI_PSM_AVCTP               0x0017 /**< Audio/Video Control Transport Protocol */
#define OI_PSM_AVDTP               0x0019 /**< Audio/Video Distribution Transport Protocol */
#define OI_PSM_AVCTP_BROWSING      0x001B /**< Audio/Video Control Transport Protocol for Browsing */
#define OI_PSM_UDI_C_PLANE         0x001D /**< Unrestricted Digital Information Profile [UDI] */

/**@}*/


/**
 * @name Bluetooth Base UUID definition
 * @{
 * This definition initializes the base 128-bit Bluetooth UUID, OI_UUID_BASE_UUID128.
 */

#define OI_UUID_BASE_UUID128 \
    { 0x00000000, { 0x00, 0x00, 0x10, 0x00, 0x80, 0x00, 0x00, 0x80, 0x5F, 0x9B, 0x34, 0xFB } }

/**@}*/

/**
 * A 128-bit UUID initialized to the base UUID.
 */
extern const OI_UUID128 OI_UUID_BaseUUID128;


/**
 * @name Protocol UUIDs
 * @{
 */

#define OI_UUID_NULL                    0      /**< special value for invalid or unknown UUID */

#define OI_UUID_SDP                    0x0001  /**< Bluetooth Service Discovery Protocol (SDP) */
#define OI_UUID_UDP                    0x0002  /**< UDP (User Datagram Protocol) */
#define OI_UUID_RFCOMM                 0x0003  /**< RFCOMM with TS 07.10 */
#define OI_UUID_TCP                    0x0004  /**< TCP (Transmission Control Protocol) */
#define OI_UUID_TCS_BIN                0x0005  /**< Bluetooth Telephony Control Specification / TCS Binary */
#define OI_UUID_TCS_AT                 0x0006  /**< modem */
#define OI_UUID_OBEX                   0x0008  /**< OBEX */
#define OI_UUID_IP                     0x0009  /**< IP (Internet Protocol) */
#define OI_UUID_FTP                    0x000A  /**< FTP (File Transfer Protocol) */
#define OI_UUID_HTTP                   0x000C  /**< HTTP (Hypertext Transport Protocol) */
#define OI_UUID_WSP                    0x000E  /**< WSP (Wireless Session Protocol (related to WAP) */
#define OI_UUID_BNEP                   0x000F  /**< BNEP (Basic Network Encapsulation Protocol) */
#define OI_UUID_UPNP                   0x0010  /**< ESDP (Extended Service Discovery Protocol for Universal Plug and Play) */
#define OI_UUID_HIDP                   0x0011  /**< HID (Human Interface Device Profile) */
#define OI_UUID_HardcopyControlChannel 0x0012  /**< part of HCRP (Hardcopy Cable Replacement Profile) */
#define OI_UUID_HardcopyDataChannel    0x0014  /**< part of HCRP (Hardcopy Cable Replacement Profile) */
#define OI_UUID_HardcopyNotification   0x0016  /**< part of HCRP (Hardcopy Cable Replacement Profile) */
#define OI_UUID_AVCTP                  0x0017  /**< Audio/Video Control Transport Protocol */
#define OI_UUID_AVDTP                  0x0019  /**< Audio/Video Distribution Transport Protocol */
#define OI_UUID_CMPT                   0x001B  /**< CAPI Message Transport Protocol */
#define OI_UUID_UDI_C_Plane            0x001D  /**< Unrestricted Digital Information Profile [UDI] */
#define OI_UUID_MCAP_Control           0x001E  /**< Multi-channel adaptation protocol (MCAP), control channel */
#define OI_UUID_MCAP_Data              0x001F  /**< Multi-channel adaptation protocol (MCAP), data channel */
#define OI_UUID_L2CAP                  0x0100  /**< Logical Link Control and Adaptation Protocol specification */

/**@}*/

/**
 * @name Service class identifiers and names
 * @{
 * If the specified service class directly and exactly implies a certain profile,
 * then the profile is identified.
 */

#define OI_UUID_ServiceDiscoveryServerServiceClassID  0x1000  /**< service class for SDP itself */
#define OI_UUID_BrowseGroupDescriptorServiceClassID   0x1001  /**< See SDP. */
#define OI_UUID_PublicBrowseGroup                     0x1002  /**< root browse group UUID */
#define OI_UUID_SerialPort                            0x1101  /**< See Generic Access Profile. */
#define OI_UUID_LANAccessUsingPPP                     0x1102  /**< See Local Area Network Access Profile. */
#define OI_UUID_DialupNetworking                      0x1103  /**< See Dial-Up Networking Profile. */
#define OI_UUID_IrMCSync                              0x1104  /**< See Synchronization Profile. */
#define OI_UUID_OBEXObjectPush                        0x1105  /**< See Object Push Profile. */
#define OI_UUID_OBEXFileTransfer                      0x1106  /**< See File Transfer Profile. */
#define OI_UUID_IrMCSyncCommand                       0x1107  /**< See Synchronization Profile. */
#define OI_UUID_Headset                               0x1108  /**< See Generic Access Profile. */
#define OI_UUID_CordlessTelephony                     0x1109  /**< See Cordless Telephony Profile. */
#define OI_UUID_AudioSource                           0x110A  /**< n/a */
#define OI_UUID_AudioSink                             0x110B  /**< n/a */
#define OI_UUID_AV_RemoteControlTarget                0x110C  /**< Audio/Video Control Profile */
#define OI_UUID_AdvancedAudioDistribution             0x110D  /**< Advanced Audio Distribution Profile */
#define OI_UUID_AV_RemoteControl                      0x110E  /**< Audio/Video Control Profile */
#define OI_UUID_VideoConferencing                     0x110F  /**< Video Conferencing Profile */
#define OI_UUID_Intercom                              0x1110  /**< See Intercom Profile. */
#define OI_UUID_Fax                                   0x1111  /**< See Fax Profile. */
#define OI_UUID_HeadsetAudioGateway                   0x1112  /**< See Generic Access Profile. */
#define OI_UUID_WAP                                   0x1113  /**< See Interoperability Requirements for Bluetooth as a WAP. */
#define OI_UUID_WAP_CLIENT                            0x1114  /**< See Interoperability Requirements for Bluetooth as a WAP. */
#define OI_UUID_PANU                                  0x1115  /**< See Personal Area Networking profile. */
#define OI_UUID_NAP                                   0x1116  /**< See Personal Area Networking profile. */
#define OI_UUID_GN                                    0x1117  /**< See Personal Area Networking profile. */
#define OI_UUID_DirectPrinting                        0x1118  /**< See Basic Printing Profile */
#define OI_UUID_ReferencePrinting                     0x1119  /**< See Basic Printing Profile */
#define OI_UUID_Imaging                               0x111A  /**< [IMAGING] */
#define OI_UUID_ImagingResponder                      0x111B  /**< [IMAGING] */
#define OI_UUID_ImagingAutomaticArchive               0x111C  /**< [IMAGING] */
#define OI_UUID_ImagingReferencedObjects              0x111D  /**< [IMAGING] */
#define OI_UUID_Handsfree                             0x111E  /**< Handsfree Profile */
#define OI_UUID_HandsfreeAudioGateway                 0x111F  /**< Handsfree Profile */
#define OI_UUID_DirectPrintingReferenceObjectsService 0x1120  /**< See Basic Printing Profile. */
#define OI_UUID_ReflectedUI                           0x1121  /**< See Basic Printing Profile. */
#define OI_UUID_BasicPrinting                         0x1122  /**< See Basic Printing Profile. */
#define OI_UUID_PrintingStatus                        0x1123  /**< See Basic Printing Profile. */
#define OI_UUID_HumanInterfaceDeviceService           0x1124  /**< See Human Interface Device. */
#define OI_UUID_HardcopyCableReplacement              0x1125  /**< See Hardcopy Cable Replacement Profile. */
#define OI_UUID_HCR_Print                             0x1126  /**< See Hardcopy Cable Replacement Profile. */
#define OI_UUID_HCR_Scan                              0x1127  /**< See Hardcopy Cable Replacement Profile. */
#define OI_UUID_Common_ISDN_Access                    0x1128  /**< See CAPI Message Transport Protocol */
#define OI_UUID_VideoConferencingGW                   0x1129  /**< See Video Conferencing Profile (VCP) */
#define OI_UUID_UID_MT                                0x112A  /**< See UID */
#define OI_UUID_UID_TA                                0x112B  /**< See UID */
#define OI_UUID_Audio_Video                           0x112C  /**< See Video Conferencing Profile (VCP)*/
#define OI_UUID_SIM_Access                            0x112D  /**< See SIM Access Profile (SAP) */
#define OI_UUID_PhonebookAccessClient                 0x112E  /**< Phonebook Access Profile Client Equipment */
#define OI_UUID_PhonebookAccessServer                 0x112F  /**< Phonebook Access Profile Server Equipment */
#define OI_UUID_PhonebookAccess                       0x1130  /**< Phonebook Access Profile ID */
#define OI_UUID_Headset_HS                            0x1131  /**< Headset profile */
#define OI_UUID_MessageAccessServer                   0x1132  /**< MAP profile */
#define OI_UUID_MessageNotificationServer             0x1133  /**< MAP profile */
#define OI_UUID_MessageAccessProfile                  0x1134  /**< MAP profile */

#define OI_UUID_PnPInformation                        0x1200  /**< Bluetooth Device Identification */
#define OI_UUID_GenericNetworking                     0x1201  /**< n/a */
#define OI_UUID_GenericFileTransfer                   0x1202  /**< n/a */
#define OI_UUID_GenericAudio                          0x1203  /**< n/a */
#define OI_UUID_GenericTelephony                      0x1204  /**< n/a */
#define OI_UUID_UPNP_Service                          0x1205  /**< [ESDP} and possible future profiles */
#define OI_UUID_UPNP_IP_Service                       0x1206  /**< [ESDP} and possible future profiles */

#define OI_UUID_ESDP_UPNP_IP_PAN                      0x1300  /**< [ESDP] */
#define OI_UUID_ESDP_UPNP_IP_LAP                      0x1301  /**< [ESDP] */
#define OI_UUID_ESDP_UPNP_IP_L2CAP                    0x1302  /**< [ESDP] */

#define OI_UUID_VideoSource                           0x1303  /**< Video Distribution Profile */
#define OI_UUID_VideoSink                             0x1304  /**< Video Distribution Profile */
#define OI_UUID_VideoDistribution                     0x1305  /**< Video Distribution Profile */

#define OI_UUID_HDP                                   0x1400  /**< Health Device Profile */
#define OI_UUID_HDPSource                             0x1401  /**< Health Device Profile */
#define OI_UUID_HDPSink                               0x1402  /**< Health Device Profile */

#define OI_UUID_MAX_VALUE                             0x1402  /**< The current maximum UUID value */

/**@}*/


/**
 * @name Attribute Identifier Codes
 * @{
 */

#define OI_ATTRID_ServiceRecordHandle               0x0000  /**< See Bluetooth Service Discovery Protocol (SDP). */
#define OI_ATTRID_ServiceClassIDList                0x0001  /**< [SDP] */
#define OI_ATTRID_ServiceRecordState                0x0002  /**< [SDP] */
#define OI_ATTRID_ServiceID                         0x0003  /**< [SDP] */
#define OI_ATTRID_ProtocolDescriptorList            0x0004  /**< [SDP] */
#define OI_ATTRID_BrowseGroupList                   0x0005  /**< [SDP] */
#define OI_ATTRID_LanguageBaseAttributeIDList       0x0006  /**< [SDP] */
#define OI_ATTRID_ServiceInfoTimeToLive             0x0007  /**< [SDP] */
#define OI_ATTRID_ServiceAvailability               0x0008  /**< [SDP] */
#define OI_ATTRID_BluetoothProfileDescriptorList    0x0009  /**< [SDP] */
#define OI_ATTRID_DocumentationURL                  0x000A  /**< [SDP] */
#define OI_ATTRID_ClientExecutableURL               0x000B  /**< [SDP] */
#define OI_ATTRID_IconURL                           0x000C  /**< [SDP] */
#define OI_ATTRID_AdditionalProtocolDescriptorLists 0x000D  /**< [SDP] */
#define OI_ATTRID_GroupID                           0x0200  /**< [SDP] */
#define OI_ATTRID_IpSubnet                          0x0200  /**< See Personal Area Networking profile. */
#define OI_ATTRID_VersionNumberList                 0x0200  /**< [SDP] */
#define OI_ATTRID_SupportFeaturesList               0x0200  /**< [HDP] */
#define OI_ATTRID_ServiceDatabaseState              0x0201  /**< [SDP] */
#define OI_ATTRID_Service_Version                   0x0300  /**< n/a */
#define OI_ATTRID_DataExchangeSpecification         0x0301  /**< See Health Device Profile (HDP) */
#define OI_ATTRID_External_Network                  0x0301  /**< See Cordless Telephony Profile. */
#define OI_ATTRID_Network                           0x0301  /**< See the Handsfree Profile (HFP) */
#define OI_ATTRID_Supported_Data_Stores_List        0x0301  /**< See Synchronization Profile. */
#define OI_ATTRID_FaxClass1Support                  0x0302  /**< standard fax (See Fax Profile.) */
#define OI_ATTRID_RemoteAudioVolumeControl          0x0302  /**< See Generic Access Profile. */
#define OI_ATTRID_MCAP_SupportedFeatures            0x0302  /**< See Health Device Profile (HFP). */
#define OI_ATTRID_FaxClass20Support                 0x0303  /**< standard fax (See Fax Profile.) */
#define OI_ATTRID_SupportedFormatsList              0x0303  /**< See Object Push Profile. */
#define OI_ATTRID_FaxClass2Support                  0x0304  /**< non-standard fax (See Fax Profile.)*/
#define OI_ATTRID_Audio_Feedback_Support            0x0305  /**< N/A */
#define OI_ATTRID_NetworkAddress                    0x0306  /**< See Interoperability Requirements for Bluetooth as a WAP */
#define OI_ATTRID_WAPGateWay                        0x0307  /**< See Interoperability Requirements for Bluetooth as a WAP */
#define OI_ATTRID_HomePageURL                       0x0308  /**< See Interoperability Requirements for Bluetooth as a WAP */
#define OI_ATTRID_WAPStackType                      0x0309  /**< See Interoperability Requirements for Bluetooth as a WAP */
#define OI_ATTRID_SecurityDescription               0x030A  /**< See Personal Area Networking profile. */
#define OI_ATTRID_NetAccessType                     0x030B  /**< See Personal Area Networking profile. */
#define OI_ATTRID_MaxNetAccessRate                  0x030C  /**< See Personal Area Networking profile. */
#define OI_ATTRID_IPV4_Subnet                       0x030D  /**< See Personal Area Networking profile. */
#define OI_ATTRID_IPV6_Subnet                       0x030E  /**< See Personal Area Networking profile. */
#define OI_ATTRID_SupportedCapabilities             0x0310  /**< [IMAGING] */
#define OI_ATTRID_SupportedFeatures                 0x0311  /**< [IMAGING],[HFP],[BPAP] */
#define OI_ATTRID_SupportedFunctions                0x0312  /**< [IMAGING] */
#define OI_ATTRID_TotalImagingDataCapacity          0x0313  /**< [IMAGING] */
#define OI_ATTRID_SupportedRepositories             0x0314  /**< [PBAP] */
#define OI_ATTRID_MAS_InstanceID                    0x0315  /**< Message Access Profile */
#define OI_ATTRID_SupportedMessageTypes             0x0316  /**< Message Access Profile */

#define OI_ATTRID_BPP_DocumentFormats               0x0350  /**<  Basic Printing Profile */
#define OI_ATTRID_BPP_CharacterRepertoires          0x0352  /**<  Basic Printing Profile */
#define OI_ATTRID_BPP_XHTMLPrintImageFormats        0x0354  /**<  Basic Printing Profile */
#define OI_ATTRID_BPP_Color                         0x0356  /**<  Basic Printing Profile */
#define OI_ATTRID_BPP_1284ID                        0x0358  /**<  Basic Printing Profile */
#define OI_ATTRID_BPP_PrinterName                   0x035a  /**<  Basic Printing Profile */
#define OI_ATTRID_BPP_PrinterLocation               0x035c  /**<  Basic Printing Profile */
#define OI_ATTRID_BPP_Duplex                        0x035e  /**<  Basic Printing Profile */
#define OI_ATTRID_BPP_MediaTypes                    0x0360  /**<  Basic Printing Profile */
#define OI_ATTRID_BPP_MaxMediaWidth                 0x0362  /**<  Basic Printing Profile */
#define OI_ATTRID_BPP_MaxMediaLength                0x0364  /**<  Basic Printing Profile */
#define OI_ATTRID_BPP_EnhancedLayout                0x0366  /**<  Basic Printing Profile */
#define OI_ATTRID_BPP_RUIFormats                    0x0368  /**<  Basic Printing Profile */
#define OI_ATTRID_BPP_ReferencePrintingRUI          0x0370  /**<  Basic Printing Profile */
#define OI_ATTRID_BPP_DirectPrintingRUI             0x0372  /**<  Basic Printing Profile */
#define OI_ATTRID_BPP_ReferencePrintingTopURL       0x0374  /**<  Basic Printing Profile */
#define OI_ATTRID_BPP_DirectPrintingTopURL          0x0376  /**<  Basic Printing Profile */
#define OI_ATTRID_MAX_VALUE                         0x0377  /**< highest attribute ID used; not maximum allowable, which is 0xFFFF */

#define OI_ATTRID_OBEX_over_L2CAP                   0x0200  /**< Attribute for publishing the PSM for OBEX/L2CAP (this value has not yet been established) */

/**@}*/

/**
 * @name Extended Inquiry Response Data types (GAP spec)
 * @{
 */

#define OI_EIR_FLAGS                            0x01  /**< Flags */
#define OI_EIR_UUID16_SOME                      0x02  /**< 16-bit UUID, more available */
#define OI_EIR_UUID16_ALL                       0x03  /**< 16-bit UUID, all listed */
#define OI_EIR_UUID32_SOME                      0x04  /**< 32-bit UUID, more available */
#define OI_EIR_UUID32_ALL                       0x05  /**< 32-bit UUID, all listed */
#define OI_EIR_UUID128_SOME                     0x06  /**< 128-bit UUID, more available */
#define OI_EIR_UUID128_ALL                      0x07  /**< 128-bit UUID, all listed */
#define OI_EIR_NAME_SHORT                       0x08  /**< shortened local name */
#define OI_EIR_NAME_COMPLETE                    0x09  /**< complete local name */
#define OI_EIR_TX_POWER                         0x0A  /**< Transmit power level */
#define OI_OOB_CLASS_OF_DEVICE                  0x0D  /**< Class of Device (OOB only) */
#define OI_OOB_SIMPLE_PAIRING_HASH_C            0x0E  /**< Simple Pairing HashC (OOB only) */
#define OI_OOB_SIMPLE_PAIRING_RAND_R            0x0F  /**< Simple Pairing RandR (OOB only) */
#define OI_EIR_DEVICE_ID                        0x10  /**< Device ID */
#define OI_EIR_MANF_DATA                        0xFF  /**< Manufacturer Specific Data */



/**@}*/
/**
 * @name String Attribute Identifiers
 * @{
 * Attribute Identifier codes for string attributes are computed by adding
 * a base offset, as given by the LanguageBaseOffset attribute.
 * For the primary language, this is 0x0100, as described in section 5.1.8 of
 * the Bluetooth specification v3.0+HS vol 3 part B.
 */

#define OI_ATTRID_DEFAULT_LanguageBaseOffset 0x0100

#define OI_ATTRID_ServiceName                0x0000  /**< [SDP] */
#define OI_ATTRID_ServiceDescription         0x0001  /**< [SDP] */
#define OI_ATTRID_ProviderName               0x0002  /**< [SDP] */
#define OI_ATTRID_ServiceName_Primary        (OI_ATTRID_ServiceName | OI_ATTRID_DEFAULT_LanguageBaseOffset)         /**< primary service name computed by adding OI_ATTRID_DEFAULT_LanguageBaseOffset to OI_ATTRID_ServiceName */
#define OI_ATTRID_ServiceDescription_Primary (OI_ATTRID_ServiceDescription | OI_ATTRID_DEFAULT_LanguageBaseOffset)  /**< primary service name computed by adding OI_ATTRID_DEFAULT_LanguageBaseOffset to OI_ATTRID_ServiceDescription */
#define OI_ATTRID_ProviderName_Primary       (OI_ATTRID_ProviderName | OI_ATTRID_DEFAULT_LanguageBaseOffset)        /**< primary service name computed by adding OI_ATTRID_DEFAULT_LanguageBaseOffset to OI_ATTRID_ProviderName */

/**@}*/

/**
 * @name SDP language identifiers
 * @{
 * These definitions come from ISO 639:1988.
 */
#define OI_LANG_ID(c1, c2)    ((OI_UINT16) ((c1 << 8) | (c2)))  /**< Macro for generating unsigned 16 bit value for language id */

#define OI_LANG_ID_ARABIC   OI_LANG_ID('a','r')
#define OI_LANG_ID_GERMAN   OI_LANG_ID('d','e')
#define OI_LANG_ID_ENGLISH  OI_LANG_ID('e','n')
#define OI_LANG_ID_SPANISH  OI_LANG_ID('e','s')
#define OI_LANG_ID_FRENCH   OI_LANG_ID('f','r')
#define OI_LANG_ID_HEBREW   OI_LANG_ID('h','e')
#define OI_LANG_ID_JAPANESE OI_LANG_ID('j','a')
#define OI_LANG_ID_KOREAN   OI_LANG_ID('k','o')
#define OI_LANG_ID_CHINESE  OI_LANG_ID('z','h')
/**@}*/

/**
 * @name SDP Character Sets
 * @{
 * Defines for character set encodings. The Bluetooth specification recommends
 * the use of UTF-8 encoding.
 *
 * These are "MIBenum" values as defined in the IANA character set assignment
 * registry. This document may be found at
 * http://www.iana.org/assignments/character-sets
 */
#define OI_LANG_ENCODING_US_ASCII         3  /**< ISO646-US encoding. See IETF RFC 1345 */
#define OI_LANG_ENCODING_ISO_8859_1       4  /**< ISO-8859-1 encoding, also known as ISO Latin 1. See IETF RFC 1345 */
#define OI_LANG_ENCODING_JIS             16  /**< JIS X 0202-1991 encoding */
#define OI_LANG_ENCODING_SHIFT_JIS       17  /**< JIS X 0208:1997 Appendix 1 */
#define OI_LANG_ENCODING_EUC_JP          18  /**< OSF/UNIX standard */
#define OI_LANG_ENCODING_KSC_5601        36  /**< ECMA standard */
#define OI_LANG_ENCODING_ISO_2022_KR     37  /**< IETF RFC 1557, KS_C_5601-1987 */
#define OI_LANG_ENCODING_EUC_KR          38  /**< IETF RFC 1557, KS_C_5861-1992 */
#define OI_LANG_ENCODING_ISO_2022_JP     39  /**< IETF RFC 1468, IETF RFC 2237 */
#define OI_LANG_ENCODING_ISO_2022_JP_2   40  /**< IETF RFC 1554 */
#define OI_LANG_ENCODING_GB2312_80       57  /**< ECMA */
#define OI_LANG_ENCODING_UTF8           106  /**< IETF RFC 2279 */
#define OI_LANG_ENCODING_GBK            113  /**< http://www.iana.org/assignments/charset-reg/GBK */
#define OI_LANG_ENCODING_UTF16BE       1013  /**< UTF-16, big-endian byte order. See IETF RFC 2781 */
#define OI_LANG_ENCODING_UTF16LE       1014  /**< UTF-16, little-endian byte order. See IETF RFC 2781 */
#define OI_LANG_ENCODING_UTF16         1015  /**< UTF-16, byte order specified by optional BOM. See IETF RFC 2781 */
#define OI_LANG_ENCODING_BIG5_HKSCS    2101  /**< http://www.iana.org/assignments/charset-reg/Big5-HKSCS */
#define OI_LANG_ENCODING_BIG5          2026  /**< Taiwanese encoding */

/**@}*/

/**
 * @def OI_ATTRID_RANGE
 * This macro constructs an attribute range as defined in the SDP section of
 * the Bluetooth specification. The attribute identifier range is a 32-bit value that
 * describes a list of attributes with consecutive identifiers and is sent as a response
 * to an SDP attribute request. The high-order 16 bits are the low bound of the range.
 * The low-order 16 bits are the high bound of the range.
 *
 * @param lowBound is the low bound of an attribute range
 *
 * @param highBound is the high bound of an attribute range
 */

#define OI_ATTRID_RANGE(lowBound, highBound)   ((((OI_UINT32)(lowBound)) << 16) | ((OI_UINT32)(highBound)))

/**
 * @def OI_ATTRID_RANGE_ALL
 * This macro constructs an attribute range which covers all attribute IDs. This
 * value may be used to construct an SDP query to retrieve all information about
 * a service
 */

#define OI_ATTRID_RANGE_ALL OI_ATTRID_RANGE(0x0000, 0xFFFF)

/**
 * @name Protocol Parameter Indices
 * @{
 */

#define OI_PROTOCOL_PARAM_PSM                            1
#define OI_PROTOCOL_PARAM_RFCOMM_Channel                 1
#define OI_PROTOCOL_PARAM_TCP_Port                       1
#define OI_PROTOCOL_PARAM_UDP_Port                       1
#define OI_PROTOCOL_PARAM_BNEP_Version                   1
#define OI_PROTOCOL_PARAM_BNEP_SupportedNetworkTypeList  2

/**@}*/

/**
 * This function returns a text string corresponding to an attribute ID.
 * NULL will be returned if the attribute ID is not in the list of Bluetooth
 * assigned numbers.
 *
 * @param AttrId an attribute ID from the assigned numbers list
 *
 * @return a text string or NULL
 */

OI_CHAR* OI_AttrIdText(OI_UINT16 AttrId);


/**
 * This function returns a text string corresponding to a UUID16 or UUID32.
 * NULL will be returned if the UUID is not in the list of Bluetooth assigned
 * numbers.
 *
 * @param UUID            This is a 16-bit or 32-bit universally unique identifier.
 *                        A 16-bit UUID (of type OI_UINT16) can be cast to OI_UUID32.
 *
 * @return a text string or NULL
 */

OI_CHAR* OI_UUIDText(OI_UUID32 UUID);

/**
 * This function returns a text string corresponding to a UUID128.
 * NULL will be returned if the UUID is not in the list of Bluetooth assigned
 * numbers.
 *
 * @param UUID           A pointer to a 128 bit UUID.
 *
 * @return a text string or NULL
 */

OI_CHAR* OI_UUID128Text(OI_UUID128 *UUID);

/**
 * This function returns a text string corresponding to a fixed PSM.
 * Empty string ("") will be returned if the PSM is not in the list of Bluetooth assigned numbers.
 *
 * @param PSM       psm
 *
 * @return a text string
 */

OI_CHAR* OI_PSMText(OI_UINT16 psm);

/**
 * @name Character Set repertoires
 * @{
 * These constants are used to indicate support for the character reportoire of
 * the referenced standard. They does not refer to the character encoding used by
 * that standard.
 */

#define OI_CHARSET_ISO_8859_1       OI_BIT0        /**< Latin alphabet No. 1 */
#define OI_CHARSET_ISO_8859_2       OI_BIT1        /**< Latin alphabet No. 2 */
#define OI_CHARSET_ISO_8859_3       OI_BIT2        /**< Latin alphabet No. 3 */
#define OI_CHARSET_ISO_8859_4       OI_BIT3        /**< Latin alphabet No. 4 */
#define OI_CHARSET_ISO_8859_5       OI_BIT4        /**< Latin/Cyrillic alphabet */
#define OI_CHARSET_ISO_8859_6       OI_BIT5        /**< Latin/Arabic alphabet */
#define OI_CHARSET_ISO_8859_7       OI_BIT6        /**< Latin/Greek alphabet */
#define OI_CHARSET_ISO_8859_8       OI_BIT7        /**< Latin/Hebrew alphabet */
#define OI_CHARSET_ISO_8859_9       OI_BIT8        /**< Latin alphabet No. 5 */
#define OI_CHARSET_ISO_8859_10      OI_BIT9        /**< Latin alphabet No. 6 */
#define OI_CHARSET_ISO_8859_13      OI_BIT10       /**< Latin alphabet No. 7 */
#define OI_CHARSET_ISO_8859_14      OI_BIT11       /**< Latin alphabet No. 8 */
#define OI_CHARSET_ISO_8859_15      OI_BIT12       /**< Latin alphabet No. 9 */
#define OI_CHARSET_GB_2312_80       OI_BIT13       /**< Chinese (People's Republic of China) */
#define OI_CHARSET_SHIFT_JIS        OI_BIT14       /**< Japanese */
#define OI_CHARSET_KS_C_5601_1987   OI_BIT15       /**< Korean */
#define OI_CHARSET_BIG5             OI_BIT16       /**< Chinese (Taiwan) */
#define OI_CHARSET_TIS_620          OI_BIT17       /**< Thai */

/**@}*/

#ifdef __cplusplus
}
#endif

/*****************************************************************************/
#endif /* _OI_BT_ASSIGNED_NOS_H */
