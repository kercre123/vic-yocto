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
This file provides assigned number text display routines.
*/

#define __OI_MODULE__ OI_MODULE_SUPPORT

#include "oi_bt_assigned_nos.h"
#include "oi_memmgr.h"

static const OI_CHAR digits[] = "0123456789ABCDEF";
static OI_CHAR buffer[7];


#define ATTRID_CASE(x) case OI_ATTRID_##x : return #x

OI_CHAR* OI_AttrIdText(OI_UINT16 AttrId)
{
#ifdef OI_DEBUG
    switch (AttrId) {
        ATTRID_CASE(ServiceRecordHandle);
        ATTRID_CASE(ServiceClassIDList);
        ATTRID_CASE(ServiceRecordState);
        ATTRID_CASE(ServiceID);
        ATTRID_CASE(ProtocolDescriptorList);
        ATTRID_CASE(BrowseGroupList);
        ATTRID_CASE(LanguageBaseAttributeIDList);
        ATTRID_CASE(ServiceInfoTimeToLive);
        ATTRID_CASE(ServiceAvailability);
        ATTRID_CASE(BluetoothProfileDescriptorList);
        ATTRID_CASE(DocumentationURL);
        ATTRID_CASE(ClientExecutableURL);
        ATTRID_CASE(IconURL);
        ATTRID_CASE(AdditionalProtocolDescriptorLists);
        case OI_ATTRID_GroupID:
            /* DUPLICATE ASSIGNED #
            ATTRID_CASE(GroupID);
            ATTRID_CASE(IpSubnet);
            ATTRID_CASE(VersionNumberList);
               */
            return "GroupID or IpSubnet or VersionNumberList";
        ATTRID_CASE(ServiceDatabaseState);
        ATTRID_CASE(Service_Version);
        case OI_ATTRID_External_Network:
            /* DUPLICATE ASSIGNED #
            ATTRID_CASE(External_Network);
            ATTRID_CASE(Network);
            ATTRID_CASE(Supported_Data_Stores_List);
               */
            return "External_Network or Network or Supported_Data_Stores_List";
        case OI_ATTRID_RemoteAudioVolumeControl:
            /* DUPLICATE ASSIGNED #
            ATTRID_CASE(RemoteAudioVolumeControl);
            ATTRID_CASE(FaxClass1Support);
               */
            return "FaxClass1Support or RemoteAudioVolumeControl";
        case OI_ATTRID_FaxClass20Support:
            /* DUPLICATE ASSIGNED #
            ATTRID_CASE(FaxClass20Support);
            ATTRID_CASE(SupportedFormatsList);
               */
            return "FaxClass20Support or SupportedFormatsList";
        ATTRID_CASE(FaxClass2Support);
        ATTRID_CASE(Audio_Feedback_Support);
        ATTRID_CASE(NetworkAddress);
        ATTRID_CASE(WAPGateWay);
        ATTRID_CASE(HomePageURL);
        ATTRID_CASE(WAPStackType);
        ATTRID_CASE(SecurityDescription);
        ATTRID_CASE(NetAccessType);
        ATTRID_CASE(MaxNetAccessRate);
        ATTRID_CASE(SupportedCapabilities);
        ATTRID_CASE(SupportedFeatures);
        ATTRID_CASE(SupportedFunctions);
        ATTRID_CASE(TotalImagingDataCapacity);

        ATTRID_CASE(BPP_DocumentFormats);
        ATTRID_CASE(BPP_CharacterRepertoires);
        ATTRID_CASE(BPP_XHTMLPrintImageFormats);
        ATTRID_CASE(BPP_Color);
        ATTRID_CASE(BPP_1284ID);
        ATTRID_CASE(BPP_PrinterName);
        ATTRID_CASE(BPP_PrinterLocation);
        ATTRID_CASE(BPP_Duplex);
        ATTRID_CASE(BPP_MediaTypes);
        ATTRID_CASE(BPP_MaxMediaWidth);
        ATTRID_CASE(BPP_MaxMediaLength);
        ATTRID_CASE(BPP_EnhancedLayout);
        ATTRID_CASE(BPP_RUIFormats);
        ATTRID_CASE(BPP_ReferencePrintingRUI);
        ATTRID_CASE(BPP_DirectPrintingRUI);
        ATTRID_CASE(BPP_ReferencePrintingTopURL);
        ATTRID_CASE(BPP_DirectPrintingTopURL);

        case OI_ATTRID_DEFAULT_LanguageBaseOffset + OI_ATTRID_ServiceName        : return "ServiceName";
        case OI_ATTRID_DEFAULT_LanguageBaseOffset + OI_ATTRID_ServiceDescription : return "ServiceDescription";
        case OI_ATTRID_DEFAULT_LanguageBaseOffset + OI_ATTRID_ProviderName       : return "ProviderName";

    }
#endif /* OI_DEBUG */
    buffer[0] = '0';
    buffer[1] = 'x';
    buffer[2] = digits[(AttrId >> 12) & 0xF];
    buffer[3] = digits[(AttrId >> 8) & 0xF];
    buffer[4] = digits[(AttrId >> 4) & 0xF];
    buffer[5] = digits[(AttrId >> 0) & 0xF];
    buffer[6] = 0;
    return buffer;
}


OI_CHAR* OI_UUID128Text(OI_UUID128 *UUID)
{

    OI_UUID128 TestUUID = OI_UUID_BASE_UUID128;

    if (!UUID) {
        return "invalid";
    }
    /* other 128-bit UUID's are known only if they are Bluetooth UUID's */
    if (OI_MemCmp(UUID->base, TestUUID.base, sizeof(TestUUID.base)) != 0) {
        return "unknown";
    } else {
        return OI_UUIDText(UUID->ms32bits);
    }
}


OI_CHAR* OI_UUIDText(OI_UUID32 UUID)
{
#ifdef OI_DEBUG
    switch (UUID) {
        /*
         * Protocol UUIDs
         */
        case OI_UUID_NULL                                 : return "NULL";
        case OI_UUID_SDP                                  : return "SDP";
        case OI_UUID_UDP                                  : return "UDP";
        case OI_UUID_RFCOMM                               : return "RFCOMM";
        case OI_UUID_TCP                                  : return "TCP";
        case OI_UUID_TCS_BIN                              : return "TCS_BIN";
        case OI_UUID_TCS_AT                               : return "TCS_AT";
        case OI_UUID_OBEX                                 : return "OBEX";
        case OI_UUID_IP                                   : return "IP";
        case OI_UUID_FTP                                  : return "FTP";
        case OI_UUID_HTTP                                 : return "HTTP";
        case OI_UUID_WSP                                  : return "WSP";
        case OI_UUID_BNEP                                 : return "BNEP";
        case OI_UUID_UPNP                                 : return "UPNP";
        case OI_UUID_HIDP                                 : return "HIDP";
        case OI_UUID_HardcopyControlChannel               : return "HardcopyControlChannel";
        case OI_UUID_HardcopyDataChannel                  : return "HardcopyDataChannel";
        case OI_UUID_HardcopyNotification                 : return "HardcopyNotification";
        case OI_UUID_AVCTP                                : return "AVCTP";
        case OI_UUID_AVDTP                                : return "AVDTP";
        case OI_UUID_L2CAP                                : return "L2CAP";
        /*
         * Service UUIDs
         */
        case OI_UUID_ServiceDiscoveryServerServiceClassID  : return "ServiceDiscoveryServerServiceClassID";
        case OI_UUID_BrowseGroupDescriptorServiceClassID   : return "BrowseGroupDescriptorServiceClassID";
        case OI_UUID_PublicBrowseGroup                     : return "PublicBrowseGroup";
        case OI_UUID_SerialPort                            : return "SerialPort";
        case OI_UUID_LANAccessUsingPPP                     : return "LANAccessUsingPPP";
        case OI_UUID_DialupNetworking                      : return "DialupNetworking";
        case OI_UUID_IrMCSync                              : return "IrMCSync";
        case OI_UUID_OBEXObjectPush                        : return "OBEXObjectPush";
        case OI_UUID_OBEXFileTransfer                      : return "OBEXFileTransfer";
        case OI_UUID_IrMCSyncCommand                       : return "IrMCSyncCommand";
        case OI_UUID_Headset                               : return "Headset";
        case OI_UUID_CordlessTelephony                     : return "CordlessTelephony";
        case OI_UUID_AudioSource                           : return "AudioSource";
        case OI_UUID_AudioSink                             : return "AudioSink";
        case OI_UUID_AV_RemoteControlTarget                : return "AV_RemoteControlTarget";
        case OI_UUID_AdvancedAudioDistribution             : return "AdvancedAudioDistribution";
        case OI_UUID_AV_RemoteControl                      : return "AV_RemoteControl";
        case OI_UUID_VideoConferencing                     : return "VideoConferencing";
        case OI_UUID_Intercom                              : return "Intercom";
        case OI_UUID_Fax                                   : return "Fax";
        case OI_UUID_HeadsetAudioGateway                   : return "HeadsetAudioGateway";
        case OI_UUID_WAP                                   : return "WAP";
        case OI_UUID_WAP_CLIENT                            : return "WAP_CLIENT";
        case OI_UUID_PANU                                  : return "PANU";
        case OI_UUID_NAP                                   : return "NAP";
        case OI_UUID_GN                                    : return "GN";
        case OI_UUID_DirectPrinting                        : return "DirectPrinting";
        case OI_UUID_ReferencePrinting                     : return "ReferencePrinting";
        case OI_UUID_Imaging                               : return "Imaging";
        case OI_UUID_ImagingResponder                      : return "ImagingResponder";
        case OI_UUID_ImagingAutomaticArchive               : return "ImagingAutomaticArchive";
        case OI_UUID_ImagingReferencedObjects              : return "ImagingReferencedObjects";
        case OI_UUID_Handsfree                             : return "Handsfree";
        case OI_UUID_HandsfreeAudioGateway                 : return "HandsfreeAudioGateway";
        case OI_UUID_DirectPrintingReferenceObjectsService : return "DirectPrintingReferenceObjectsService";
        case OI_UUID_ReflectedUI                           : return "ReflectedUI";
        case OI_UUID_BasicPrinting                         : return "BasicPrinting";
        case OI_UUID_PrintingStatus                        : return "PrintingStatus";
        case OI_UUID_HumanInterfaceDeviceService           : return "HumanInterfaceDeviceService";
        case OI_UUID_HardcopyCableReplacement              : return "HardcopyCableReplacement";
        case OI_UUID_HCR_Print                             : return "HCR_Print";
        case OI_UUID_HCR_Scan                              : return "HCR_Scan";
        case OI_UUID_Common_ISDN_Access                    : return "Common_ISDN_Access";
        case OI_UUID_VideoConferencingGW                   : return "VideoConferencingGW";
        case OI_UUID_UID_MT                                : return "UID_MT";
        case OI_UUID_UID_TA                                : return "UID_TA";
        case OI_UUID_Audio_Video                           : return "Audio_Video";
        case OI_UUID_SIM_Access                            : return "SIM_Access";
        case OI_UUID_PhonebookAccessClient                 : return "PhonebookAccessClient";
        case OI_UUID_PhonebookAccessServer                 : return "PhonebookAccessServer";
        case OI_UUID_PhonebookAccess                       : return "PhonebookAccess";
        case OI_UUID_Headset_HS                            : return "Headset_HS";
        case OI_UUID_MessageAccessServer                   : return "MessageAccessServer";
        case OI_UUID_MessageNotificationServer             : return "MessageNotificationServer";
        case OI_UUID_MessageAccessProfile                  : return "MessageAccessProfile";
        case OI_UUID_PnPInformation                        : return "PnPInformation";
        case OI_UUID_GenericNetworking                     : return "GenericNetworking";
        case OI_UUID_GenericFileTransfer                   : return "GenericFileTransfer";
        case OI_UUID_GenericAudio                          : return "GenericAudio";
        case OI_UUID_GenericTelephony                      : return "GenericTelephony";
        case OI_UUID_UPNP_Service                          : return "UPNP_Service";
        case OI_UUID_UPNP_IP_Service                       : return "UPNP_IP_Service";
        case OI_UUID_ESDP_UPNP_IP_PAN                      : return "ESDP_UPNP_IP_PAN";
        case OI_UUID_ESDP_UPNP_IP_LAP                      : return "ESDP_UPNP_IP_LAP";
        case OI_UUID_ESDP_UPNP_IP_L2CAP                    : return "ESDP_UPNP_IP_L2CAP";
        case OI_UUID_VideoSource                           : return "VideoSource";
        case OI_UUID_VideoSink                             : return "VideoSink";
        case OI_UUID_VideoDistribution                     : return "VideoDistribution";
    }
#endif /* OI_DEBUG */
    buffer[0] = '0';
    buffer[1] = 'x';
    buffer[2] = digits[(UUID >> 12) & 0xF];
    buffer[3] = digits[(UUID >> 8) & 0xF];
    buffer[4] = digits[(UUID >> 4) & 0xF];
    buffer[5] = digits[(UUID >> 0) & 0xF];
    buffer[6] = 0;
    return buffer;
}

OI_CHAR* OI_PSMText(OI_UINT16 psm)
{
#ifdef OI_DEBUG
    switch (psm) {
        case OI_PSM_SDP:            return "OI_PSM_SDP";
        case OI_PSM_RFCOMM:         return "OI_PSM_RFCOMM";
        case OI_PSM_TCS:            return "OI_PSM_TCS";
        case OI_PSM_TCS_CORDLESS:   return "OI_PSM_TCS_CORDLESS";
        case OI_PSM_BNEP:           return "OI_PSM_BNEP";
        case OI_PSM_HID_CONTROL:    return "OI_PSM_HID_CONTROL";
        case OI_PSM_HID_INTERRUPT:  return "OI_PSM_HID_INTERRUPT";
        case OI_PSM_AVCTP:          return "OI_PSM_AVCTP";
        case OI_PSM_AVDTP:          return "OI_PSM_AVDTP";
        case OI_PSM_AVCTP_BROWSING: return "OI_PSM_AVCTP_BROWSING";
        case OI_PSM_UDI_C_PLANE:    return "OI_PSM_UDI_C_PLANE";
    }
    return "";
#else
    return "";
#endif
}

/*****************************************************************************/

