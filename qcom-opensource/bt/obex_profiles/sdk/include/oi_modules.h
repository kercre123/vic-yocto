#ifndef _OI_MODULES_H
#define _OI_MODULES_H
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
 *
   This file provides the enumeration type defining the
   individual stack components.
 *
 */

/** \addtogroup Misc Miscellaneous APIs */
/**@{*/

#ifdef __cplusplus
extern "C" {
#endif


/**
 * This enumeration lists constants for referencing the components of
 * the BLUEmagic 3.0 protocol stack, profiles, and other functionalities.
 *
 * In order to distinguish types of modules, items are grouped with markers to
 * delineate start and end of the groups
 *
 * The module type is used for various purposes:
 *      identification in debug print statements
 *      access to initialization flags
 *      access to the configuration table
 */

typedef enum {
    /* profiles and protocols  --> Updates to oi_debug.c and oi_config_table.c */

    OI_MODULE_BIP_CLI,          /**< 0 Basic Imaging Profile protocol client */
    OI_MODULE_BIP_SRV,          /**< 1 Basic Imaging Profile protocol server */
    OI_MODULE_BPP_SENDER,       /**< 2 Basic Printing Profile */
    OI_MODULE_BPP_PRINTER,      /**< 3 Basic Printing Profile */
    OI_MODULE_FTP_CLI,          /**< 4 File Transfer Profile protocol client */
    OI_MODULE_FTP_SRV,          /**< 5 File Transfer Profile protocol server */
    OI_MODULE_OBEX_CLI,         /**< 6 OBEX protocol client, Generic Object Exchange Profile */
    OI_MODULE_OBEX_SRV,         /**< 7 OBEX protocol server, Generic Object Exchange Profile */
    OI_MODULE_OPP_CLI,          /**< 8 Object Push Profile protocol client */
    OI_MODULE_OPP_SRV,          /**< 9 Object Push Profile protocol server */
    OI_MODULE_PBAP_CLI,         /**< 10 Phonebook Access Profile client */
    OI_MODULE_PBAP_SRV,         /**< 11 Phonebook Access Profile server */
    OI_MODULE_MAP_CLI,          /**< 12 Message Access Profile client*/
    OI_MODULE_MAP_SRV,          /**< 13 Message Access Profile server*/

    OI_MODULE_COMMON_CONFIG,    /**< 14 Common configuration, module has no meaning other than for config struct */
    OI_MODULE_DISPATCH,         /**< 15 Dispatcher */
    OI_MODULE_DATAELEM,         /**< 16 Data Elements, marshaller */
    OI_MODULE_MEMMGR,           /**< 17 modules that do memory management */
    OI_MODULE_SUPPORT,          /**< 18 support functions, including CThru Dispatcher, time functions, and stack initialization */
    // OEM files --> Updates to oi_debug.c
    OI_MODULE_OEM,              /**< 19 Application Memory allocation */

    /* various pieces of code depend on these last 2 elements occuring in a specific order:
       OI_MODULE_ALL must be the 2nd to last element
       OI_MODULE_UNKNOWN must be the last element
       */
    OI_MODULE_ALL,              /**< 20 special value identifying all modules - used for control of debug print statements */
    OI_MODULE_UNKNOWN           /**< 21 special value - used for debug print statements */
} OI_MODULE;

/**
 * This constant is the number of actual modules in the list.  ALL and UNKNOWN are
 * special values that are not actually modules.
 * Used for debug print and memmgr profiling
 */
#define OI_NUM_MODULES  OI_MODULE_ALL


/**
 * This constant is the number of profile and core components.  It is used to size
 * the initialization and configuration tables.
 */
#define OI_NUM_STACK_MODULES    OI_MODULE_OEM


#ifdef __cplusplus
}
#endif

/**@}*/

#endif /* _OI_MODULES_H */

