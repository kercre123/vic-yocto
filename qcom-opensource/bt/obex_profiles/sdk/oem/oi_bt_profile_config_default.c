#ifndef _OI_BT_PROFILE_CONFIG_C
#define _OI_BT_PROFILE_CONFIG_C

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
 *  @file
 *  This file defines configuration parameters for various profiles and protocols.
 *
 *  Values in this file may be changed; these values will be used to populate instances
 *  of the data structures defined in oi_bt_profile_config.h that will be used during
 *  profile initialization and configuration functions.
 *
 *  Do not change the file oi_bt_profile_config.h.
 *
 */

#include "oi_stddefs.h"
#include "oi_rfcomm_prefs.h"
#include "oi_l2cap_prefs.h"
#include "oi_l2cap.h"
#include "oi_bt_profile_config.h"


/* Basic Imaging Profile */

const OI_CONFIG_BIP_SRV oi_default_config_BIP_SRV =
{
    3     // OI_UINT     maxServers - numer of servers that can be registered simaltaneously
};

/****************************************************/

/* File Transfer Profile */

const OI_CONFIG_FTP_SRV oi_default_config_FTP_SRV = {
    RFCOMM_PREF_FTP_SRV    // OI_UINT8    rfcomm_channel_pref - preferred rfcomm channel id (from oi_rfcomm_prefs.h)
};

const OI_CONFIG_FTP_CLI oi_default_config_FTP_CLI = {
    5        // OI_UINT8 max_folder_tree_depth - maximum folder recursion for folder put/get */
};

/****************************************************/

/* OBEX protocol, Generic Object Exchange Profile */

const OI_CONFIG_OBEX_CLI oi_default_config_OBEX_CLI = {
    OI_SECONDS(25),  /*
                      * OI_INTERVAL responseTimeout; This timeout should be a fairly large value. It
                      * is intended to detect the case where the Bluetooth link is still up but the
                      * OBEX server has gone unresponsive, probably because it has crashed.
                      */

      0,             /* Unused */

    #ifdef  SMALL_RAM_MEMORY
        400,    /* OI_UINT16 maxPktLen; Maximum size of the OBEX packet */
    #else
        65500,  /* OI_UINT16 maxPktLen; Maximum size of the OBEX packet */
    #endif
    "YYYY BLUEmagic 3.0 YYYY"    // OI_CHAR *privateKey;
};


const OI_CONFIG_OBEX_SRV oi_default_config_OBEX_SRV = {

    OI_SECONDS(30),  /*
                      * OI_INTERVAL connectTimeout; Timeout waiting for a respsonse to an OBEX
                      * connection request.
                      */

    OI_SECONDS(120), /*
                      * OI_INTERVAL authTimeout; Timeout waiting for a response to an OBEX
                      * authentication request. Ths timeout must be long enough to allow the user to
                      * enter a PIN code.
                      */

    0,               /* Unused */

    #ifdef  SMALL_RAM_MEMORY
        400,    /* OI_UINT16 maxPktLen; Maximum size of the OBEX packet */
    #else
        65500,  /* OI_UINT16 maxPktLen; Maximum size of the OBEX packet */
    #endif
    "XXXX BLUEmagic 3.0 XXXX",   // OI_CHAR     *privateKey;
};


/****************************************************/

/* Object Push Profile */

const OI_CONFIG_OPP_SRV oi_default_config_OPP_SRV = {
    RFCOMM_PREF_OPP_SRV,    // OI_UINT8    rfcomm_channel_pref - preferred rfcomm channel id (from oi_rfcomm_prefs.h)
    L2CAP_PREF_OPP_SRV      // OI_UINT16    l2cap_channel_pref - preferred rfcomm channel id (from oi_l2cap_prefs.h)
};

/****************************************************/

/* Phonebook Access Profile */

const OI_CONFIG_PBAP_SRV oi_default_config_PBAP_SRV = {
    RFCOMM_PREF_PBAP_SRV,    // OI_UINT8    rfcomm_channel_pref - preferred rfcomm channel id (from oi_rfcomm_prefs.h)
    L2CAP_PREF_PBAP_SRV      // OI_UINT16    l2cap_prefs - preferred rfcomm channel id (from oi_l2cap_prefs.h)
};

/* const OI_CONFIG_PBAP_CLI oi_default_config_PBAP_CLI = { }; */

/****************************************************/

/* MAP  server */

const OI_CONFIG_MAP_SRV oi_default_config_MAP_SRV =  {
    RFCOMM_PREF_MAP_SRV,        // OI_UINT8    rfcomm_channel_pref
    9,                        // Max. MAS connections to the server
	3                         // Max. no. of MCEs that can be connected to the server
};

/****************************************************/

/* MNS  server */

const OI_CONFIG_MAP_CLI oi_default_config_MAP_CLI =  {
    RFCOMM_PREF_MNS_SRV        // OI_UINT8    rfcomm_channel_pref
};

/****************************************************/

#endif  // _OI_BT_PROFILE_CONFIG_C
