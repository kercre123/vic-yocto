#ifndef _OI_RFCOMM_PREFS_H
#define _OI_RFCOMM_PREFS_H

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
 * This file defines RFCOMM preferences.
 *
 The following are the RFCOMM channel number preferences for the various
    profiles which register RFCOMM servers. If the numbers are selected
    appropriately, a given service using RFCOMM will always be assigned the
    same RFCOMM channel number.

    @note: Numbers are restricted to range of 1-30.
 */

#ifdef __cplusplus
extern "C" {
#endif


#define RFCOMM_PREF_FAX              5
#define RFCOMM_PREF_HANDSFREE        7
#define RFCOMM_PREF_HANDSFREE_AG     8
#define RFCOMM_PREF_HEADSET          9
#define RFCOMM_PREF_HEADSET_AG      10
#define RFCOMM_PREF_SAP_SRV         11
#define RFCOMM_PREF_OPP_SRV         12
#define RFCOMM_PREF_SYNC_SRV        14
#define RFCOMM_PREF_SYNC_CMD_SVC    15
#define RFCOMM_PREF_MAP_SRV         16
#define RFCOMM_PREF_MNS_SRV         17
#define RFCOMM_PREF_PBAP_SRV        19
#define RFCOMM_PREF_FTP_SRV         20
#define RFCOMM_PREF_SPP             21
#define RFCOMM_PREF_DUN             25

/*****************************************************************************/
#ifdef __cplusplus
}
#endif

#endif /* _OI_RFCOMM_PREFS_H */

