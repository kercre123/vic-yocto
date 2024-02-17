#ifndef _OI_BT_PROFILE_CONFIG_H
#define _OI_BT_PROFILE_CONFIG_H

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
 *
 * This file provides type definitions for configuration of data structures for each
 * BLUEmagic 3.0 profile, used for configuring memory usage.
 *
 * The comments in this file contain details on recommended and required usage of the
 * parameters defined herein. The values of these parameters may be changed in the file
 * oi_bt_profile_config_default.c.
 *
 * Adhering to naming conventions is critical to proper operation.
 *  The typedef for the configuration structure is
 *      OI_CONFIG_<module>
 *  where module is one of the enumerated module IDs, minus the OI_MODULE_ prefix
 * (e.g., OI_CONFIG_AVDTP).
 *
 */

#include "oi_stddefs.h"
#include "oi_time.h"
#include "oi_bt_spec.h"
#include "oi_rfcomm_prefs.h"

/** \addtogroup InitConfig */
/**@{*/

#ifdef __cplusplus
extern "C" {
#endif

/********************************************************/

/** This structure defines the Basic Imaging Profile (Client).
        There are currently no configuration parameters for
        this profile. The structure below is just a placeholder.
*/
typedef struct {
    OI_UINT32 dummy;
} OI_CONFIG_BIP_CLI;

/** This structure defines the Basic Imaging Profile (Server).
*/
typedef struct  {
    OI_UINT maxServers;      /**< number of servers that can be registered simultaneously     */
} OI_CONFIG_BIP_SRV;

/** Basic Imaging Profile, Server role
*/
extern const OI_CONFIG_BIP_SRV oi_default_config_BIP_SRV;

/********************************************************/

/** This structure defines the Basic Printing Profile (Sender)
        configuration. There are currently no configuration
        parameters for this profile. The structure below is just
        a placeholder.
*/
typedef struct  {
    OI_UINT32 dummy;
} OI_CONFIG_BPP_SENDER;

/** This structure defines the Basic Printing Profile (Printer)
        configuration. There are currently no configuration
        parameters for this profile. The structure below is just
        a placeholder.
*/
typedef struct  {
    OI_UINT32 dummy;
} OI_CONFIG_BPP_PRINTER;

/********************************************************/

/** This structure defines the File Transfer Profile (Server)
    configuration.
*/
typedef struct  {
    OI_UINT8 rfcomm_channel_pref;           /**< Preferred RFCOMM channel ID (from oi_rfcomm_prefs.h) */
} OI_CONFIG_FTP_SRV;

/** This structure defines the File Transfer Profile (Client)
    configuration.
*/
typedef struct  {
    OI_UINT8 max_folder_tree_depth;         /**< Maximum folder recursion for folder put/get */
} OI_CONFIG_FTP_CLI;

/** File Transfer Profile (Server) */
extern const OI_CONFIG_FTP_SRV oi_default_config_FTP_SRV;

/** File Transfer Profile (Client) */
extern const OI_CONFIG_FTP_CLI oi_default_config_FTP_CLI;

/********************************************************/

/**  This structure defines the Generic Object Exchange Profile
     client configuration.
*/
typedef struct {
    OI_INTERVAL responseTimeout;   /**< how long to wait for a response to an OBEX command */
    OI_UINT     unused;
    OI_UINT16   maxPktLen;         /**< maximum length of a request packet */
    OI_CHAR     *privateKey;       /**< key used for OBEX authentication */
} OI_CONFIG_OBEX_CLI;

/**  This structure defines the Generic Object Exchange Profile
     server configuration.
*/
typedef struct {
    OI_INTERVAL connectTimeout;   /**< how long to wait for completion of an unauthenticated connect request to a multiplexed service above OBEX after RFCOMM connection has been established */
    OI_INTERVAL authTimeout;      /**< how long to wait for completion of an authenticated connect request to a multiplexed service above OBEX after RFCOMM connection has been established */
    OI_UINT     unused;
    OI_UINT16   maxPktLen;        /**< maximum length packet (must be >= 255) */
    OI_CHAR     *privateKey;      /**< pointer to key used for OBEX authentication */
} OI_CONFIG_OBEX_SRV;

/** Generic Object Exchange Profile client */
extern const OI_CONFIG_OBEX_CLI     oi_default_config_OBEX_CLI;

/** Generic Object Exchange Profile server */
extern const OI_CONFIG_OBEX_SRV     oi_default_config_OBEX_SRV;

/********************************************************/

/** This structure defines the Object Push Profile server
    configuration.
*/
typedef struct  {
    OI_UINT8 rfcomm_channel_pref;     /**< preferred RFCOMM channel ID (from oi_rfcomm_prefs.h) */
    OI_UINT16 l2cap_psm_pref;         /**< preferred L2CAP PSM (from oi_l2cap_prefs.h) */
} OI_CONFIG_OPP_SRV;

/** Object Push Profile */
extern const OI_CONFIG_OPP_SRV oi_default_config_OPP_SRV;

/********************************************************/

/** This structure defines the Phonebook Access Profile server
    configuration.
*/
typedef struct  {
    OI_UINT8 rfcomm_channel_pref;           /**< Preferred RFCOMM channel ID (from oi_rfcomm_prefs.h) */
    OI_UINT16 l2cap_psm_pref;               /**< preferred L2CAP PSM (from oi_l2cap_prefs.h) */
} OI_CONFIG_PBAP_SRV;

/** This structure defines the Phonebook Access Profile client
    configuration.
*/
typedef struct  {
    OI_UINT32 dummy;
} OI_CONFIG_PBAP_CLI;

/** Phonebook Access Profile */
extern const OI_CONFIG_PBAP_SRV oi_default_config_PBAP_SRV;
/* extern const OI_CONFIG_FTP_CLI oi_default_config_FTP_CLI; */

/********************************************************/

/** This structure defines the Message Access Profile (Server)
    configuration.
*/
typedef struct  {
    OI_UINT8 rfcomm_channel_pref; /**< Preferred RFCOMM channel ID (from oi_rfcomm_prefs.h) */
    OI_UINT8 maxConnections;     /* Max. MAS connections to the server */
    OI_UINT8 maxMCEConnected;   /* Max. no. of MCEs that can be connected to the server */ 
} OI_CONFIG_MAP_SRV;

/** MAP Server */
extern const OI_CONFIG_MAP_SRV oi_default_config_MAP_SRV;
/********************************************************/

/** This structure defines the Message Notification Service (Server)
    configuration.
*/
typedef struct  {
    OI_UINT8 rfcomm_channel_pref;           /**< Preferred RFCOMM channel ID (from oi_rfcomm_prefs.h) */
} OI_CONFIG_MAP_CLI;

/** MAP Server */
extern const OI_CONFIG_MAP_CLI oi_default_config_MAP_CLI;
/********************************************************/

#ifdef __cplusplus
}
#endif

/**@}*/

#endif  /* _OI_BT_PROFILE_CONFIG_H */

