#ifndef _OI_PBAP_CONSTS_H
#define _OI_PBAP_CONSTS_H

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
 * This file contains definitions for values specified in the PBAP (Phonebook Access) Profile.
 */

#include "oi_stddefs.h"

/** \addtogroup PBAP PBAP APIs */
/**@{*/

#ifdef __cplusplus
extern "C" {
#endif


#define OI_PBAP_PHONEBOOK_TYPE      "x-bt/phonebook"
#define OI_PBAP_VCARD_LISTING_TYPE  "x-bt/vcard-listing"
#define OI_PBAP_VCARD_TYPE          "x-bt/vcard"

#define OI_PBAP_OBEX_TARGET_UUID   { 0x79,0x61,0x35,0xf0,0xf0,0xc5,0x11,0xd8,0x09,0x66,0x08,0x00,0x20,0x0c,0x9a,0x66 }


typedef enum {
    OI_PBAP_TAG_ID_ORDER             = 0x01,
    OI_PBAP_TAG_ID_SEARCH_VALUE      = 0x02,
    OI_PBAP_TAG_ID_SEARCH_ATTRIBUTE  = 0x03,
    OI_PBAP_TAG_ID_MAX_LIST_COUNT    = 0x04,
    OI_PBAP_TAG_ID_LIST_START_OFFSET = 0x05,
    OI_PBAP_TAG_ID_FILTER            = 0x06,
    OI_PBAP_TAG_ID_FORMAT            = 0x07,
    OI_PBAP_TAG_ID_PHONEBOOK_SIZE    = 0x08,
    OI_PBAP_TAG_ID_NEW_MISSED_CALLS  = 0x09,
    OI_PBAP_TAG_ID_SUPP_FEATURES     = 0x10
} OI_PBAP_APPLICATION_PARAM_TAG_IDS;

typedef enum {
    OI_PBAP_ORDER_INDEXED      = 0x00,
    OI_PBAP_ORDER_ALPHANUMERIC = 0x01,
    OI_PBAP_ORDER_PHONETIC     = 0x02
} OI_PBAP_ORDER_TAG_VALUES;

typedef enum {
    OI_PBAP_SEARCH_ATTRIBUTE_NAME   = 0x00,
    OI_PBAP_SEARCH_ATTRIBUTE_NUMBER = 0x01,
    OI_PBAP_SEARCH_ATTRIBUTE_SOUND  = 0x02
} OI_PBAP_SEARCH_ATTRIBUTE_TAG_VALUES;


/** PBAP filter mask values */
/* Lower 32 bits of PBAP Filter mask values */
#define OI_PBAP_VCARD_ATTRIBUTE_VERSION       OI_BIT0   /**<  VERSION vCard Version */
#define OI_PBAP_VCARD_ATTRIBUTE_FN            OI_BIT1   /**<  FN Formatted Name */
#define OI_PBAP_VCARD_ATTRIBUTE_N             OI_BIT2   /**<  N Structured Presentation of Name */
#define OI_PBAP_VCARD_ATTRIBUTE_PHOTO         OI_BIT3   /**<  PHOTO Associated Image or Photo */
#define OI_PBAP_VCARD_ATTRIBUTE_BDAY          OI_BIT4   /**<  BDAY Birthday */
#define OI_PBAP_VCARD_ATTRIBUTE_ADR           OI_BIT5   /**<  ADR Delivery Address */
#define OI_PBAP_VCARD_ATTRIBUTE_LABEL         OI_BIT6   /**<  LABEL Delivery */
#define OI_PBAP_VCARD_ATTRIBUTE_TEL           OI_BIT7   /**<  TEL Telephone Number */
#define OI_PBAP_VCARD_ATTRIBUTE_EMAIL         OI_BIT8   /**<  EMAIL Electronic Mail Address */
#define OI_PBAP_VCARD_ATTRIBUTE_MAILER        OI_BIT9   /**<  MAILER Electronic Mail */
#define OI_PBAP_VCARD_ATTRIBUTE_TX            OI_BIT10  /**<  TZ Time Zone */
#define OI_PBAP_VCARD_ATTRIBUTE_GEO           OI_BIT11  /**<  GEO Geographic Position */
#define OI_PBAP_VCARD_ATTRIBUTE_TITLE         OI_BIT12  /**<  TITLE Job */
#define OI_PBAP_VCARD_ATTRIBUTE_ROLE          OI_BIT13  /**<  ROLE Role within the Organization */
#define OI_PBAP_VCARD_ATTRIBUTE_LOGO          OI_BIT14  /**<  LOGO Organization Logo */
#define OI_PBAP_VCARD_ATTRIBUTE_AGENT         OI_BIT15  /**<  AGENT vCard of Person Representing */
#define OI_PBAP_VCARD_ATTRIBUTE_ORG           OI_BIT16  /**<  ORG Name of Organization */
#define OI_PBAP_VCARD_ATTRIBUTE_NOTE          OI_BIT17  /**<  NOTE Comments */
#define OI_PBAP_VCARD_ATTRIBUTE_REV           OI_BIT18  /**<  REV Revision */
#define OI_PBAP_VCARD_ATTRIBUTE_SOUND         OI_BIT19  /**<  SOUND Pronunciation of Name */
#define OI_PBAP_VCARD_ATTRIBUTE_URL           OI_BIT20  /**<  URL Uniform Resource Locator */
#define OI_PBAP_VCARD_ATTRIBUTE_UID           OI_BIT21  /**<  UID Unique ID */
#define OI_PBAP_VCARD_ATTRIBUTE_KEY           OI_BIT22  /**<  KEY Public Encryption Key */
#define OI_PBAP_VCARD_ATTRIBUTE_NICKNAME      OI_BIT23  /**<  NICKNAME Nickname */
#define OI_PBAP_VCARD_ATTRIBUTE_CATEGORIES    OI_BIT24  /**<  CATEGORIES Categories */
#define OI_PBAP_VCARD_ATTRIBUTE_PROID         OI_BIT25  /**<  PROID Product ID */
#define OI_PBAP_VCARD_ATTRIBUTE_CLASS         OI_BIT26  /**<  CLASS Class Information */
#define OI_PBAP_VCARD_ATTRIBUTE_SORT_STRING   OI_BIT27  /**<  SORT_STRING String Used for Sorting Operation */
#define OI_PBAP_VCARD_ATTRIBUTE_CALL_DATETIME OI_BIT28  /**<  X-IRMC-CALL-DATETIME Time Stamp */

#define OI_PBAP_VCARD_ATTRIBUTE_RSVD0         OI_BIT29  /**<  Reserved for future use */
#define OI_PBAP_VCARD_ATTRIBUTE_RSVD1         OI_BIT30  /**<  Reserved for future use */
#define OI_PBAP_VCARD_ATTRIBUTE_RSVD2         OI_BIT31  /**<  Reserved for future use */

/* Upper 32 bits of PBAP Filter mask values */
#define OI_PBAP_VCARD_ATTRIBUTE_PROPRIETARY OI_BIT7  /**< PROPRIETARY Flag indicating bits 40-63 contain proprietary fields */

#define OI_PBAP_VCARD_ATTRIBUTE_ALL { 0, 0 }
#define OI_PBAP_VCARD_ATTRIBUTE_CHECK_ALL(_filter) (((_filter)->I1 == 0) && ((_filter)->I2 == 0))

typedef enum {
    OI_PBAP_FORMAT_VCARD_2_1  = 0x00,
    OI_PBAP_FORMAT_VCARD_3_0  = 0x01
} OI_PBAP_FORMAT_TAG_VALUES;


/** PBAP Supported Features */
#define OI_PBAP_SUPPORTED_FEATURE_PHONEBOOK_DOWNLOAD OI_BIT0
#define OI_PBAP_SUPPORTED_FEATURE_PHONEBOOK_BROWSE   OI_BIT1

/** PBAP Suported Repositories */
#define OI_PBAP_SUPPORTED_REPOSITORIES_LOCAL OI_BIT0
#define OI_PBAP_SUPPORTED_REPOSITORIES_SIM   OI_BIT1

/**
 * The list of repositories defined by the PBAP specification.
 */
typedef enum {
    OI_PBAP_LOCAL_REPOSITORY,   /**< Collection of phonebooks stored on the phone. */
    OI_PBAP_SIM1_REPOSITORY,    /**< Collection of phonebooks stored on the SIM card. */
    OI_PBAP_INVALID_REPOSITORY  /**< Invalid repository. */
} OI_PBAP_REPOSITORY;


/**
 * List of phonebooks defined by the PBAP specification.
 */
typedef enum {
    OI_PBAP_MAIN_PHONEBOOK,           /**< The main phonebook. */
    OI_PBAP_INCOMING_CALLS_HISTORY,   /**< List of recent incomming calls. */
    OI_PBAP_OUTGOING_CALLS_HISTORY,   /**< List of recent outgoing calls. */
    OI_PBAP_MISSED_CALLS_HISTORY,     /**< List of recently missed incomming calls. */
    OI_PBAP_COMBINED_CALLS_HISTORY,   /**< All recent incomming, outgoing, and missed calls. */
    OI_PBAP_INVALID_PHONEBOOK         /**< Invalid phonebook. */
} OI_PBAP_PHONEBOOK;

#ifdef __cplusplus
}
#endif

/**@}*/

#endif /* _OI_PBAP_CONSTS_H */
