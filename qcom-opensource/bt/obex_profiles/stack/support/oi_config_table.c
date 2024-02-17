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
 * This file contains the configuration table and access routines.
 *
 */

#define __OI_MODULE__ OI_MODULE_SUPPORT

#include "oi_core_common.h"
#include "oi_modules.h"
#include "oi_config_table.h"
#include "oi_support_init.h"
#include "oi_init_flags.h"
#include "oi_bt_stack_config.h"
#include "oi_bt_profile_config.h"
#include "oi_bt_module_init.h"

/* The configuration table contains pointers to configuration structures.
    In release mode, the table is indexed directly by module id.
    In debug mode, the table contains the module id as well as the config pointer in order to
        verify consistency of the module id enum with the configuration table.
*/

typedef struct {
    const void    *pConfig ;

    #ifdef OI_DEBUG
    OI_MODULE   moduleId ;
    #endif

} OI_CONFIG_TABLE_ENTRY ;

#ifdef OI_DEBUG
#define DEFAULT_CONFIG_REC(module)  { (const void*)(&oi_default_config_##module), OI_MODULE_##module }
#define NULL_CONFIG_REC(module)     { NULL,                                OI_MODULE_##module }
#else
#define DEFAULT_CONFIG_REC(module)  { (const void*)(&oi_default_config_##module) }
#define NULL_CONFIG_REC(module)     { NULL }
#endif

/* Constant table of default configurations.

    The entries must be in exactly the same order as enumerated in modules.h */

static const OI_CONFIG_TABLE_ENTRY defaultConfigurations[OI_NUM_STACK_MODULES] =  {
    /* profiles and protocols */

    NULL_CONFIG_REC(BIP_CLI),               /**< Basic Imaging Profile protocol client */
    DEFAULT_CONFIG_REC(BIP_SRV),            /**< Basic Imaging Profile protocol server*/
    NULL_CONFIG_REC(BPP_SENDER),            /**< Basic Printing Profile */
    NULL_CONFIG_REC(BPP_PRINTER),           /**< Basic Printing Profile */
    DEFAULT_CONFIG_REC(FTP_CLI),            /**< File Transfer Profile protocol client */
    DEFAULT_CONFIG_REC(FTP_SRV),            /**< File Transfer Profile protocol server */
    DEFAULT_CONFIG_REC(OBEX_CLI),           /**< OBEX protocol, Generic Object Exchange Profile */
    DEFAULT_CONFIG_REC(OBEX_SRV),           /**< OBEX protocol, Generic Object Exchange Profile */
    NULL_CONFIG_REC(OPP_CLI),               /**< Object Push Profile protocol client */
    DEFAULT_CONFIG_REC(OPP_SRV),            /**< Object Push Profile protocol server */
    NULL_CONFIG_REC(PBAP_CLI),              /**< Phonebook Access Profile protocol client */
    DEFAULT_CONFIG_REC(PBAP_SRV),           /**< Phonebook Access Profile protocol server */
    DEFAULT_CONFIG_REC(MAP_CLI),            /**< Message Access Profile Client */
    DEFAULT_CONFIG_REC(MAP_SRV),            /**< Message Access Profile  Server */

    /* corestack components */

    DEFAULT_CONFIG_REC(COMMON_CONFIG),      /**< Config common to all modules */
    DEFAULT_CONFIG_REC(DISPATCH),           /**< Dispatcher */
    NULL_CONFIG_REC(DATAELEM),              /**< Data Elements, marshaller */
#ifdef OI_USE_NATIVE_MALLOC
    NULL_CONFIG_REC(MEMMGR),                /**< modules that do memory management */
#else
    DEFAULT_CONFIG_REC(MEMMGR),             /**< modules that do memory management */
#endif
    NULL_CONFIG_REC(SUPPORT),               /**< support functions, including CThru Dispatcher, Memory Manager, time functions, and stack initialization */
} ;

/* Ram table of current configurations */

static OI_CONFIG_TABLE_ENTRY curConfig[OI_NUM_STACK_MODULES] ;

/*************************************************************

    Initialize the configuration table

    Initialization sets the all configuration pointers to their default values.
    This function should be called before initializing any profiles or core stack
    components

*************************************************************/

void OI_ConfigTable_Init(void)
{
    const OI_CONFIG_TABLE_ENTRY *src ;
    OI_CONFIG_TABLE_ENTRY       *dest ;
    OI_UINT                      i ;

    /* memmgr may not yet be initialized - cannot depend on any OI support at this time. */

    src = &defaultConfigurations[0] ;
    dest = &curConfig[0] ;

    for (i = 0; i < OI_ARRAYSIZE(defaultConfigurations); ++i) {
        #ifdef OI_DEBUG
            OI_ASSERT(i == src->moduleId) ;
        #endif

        *dest = *src ;
        ++src ;
        ++dest ;
    }
}
/*************************************************************

    OI_ConfigTable_GetConfig (module)

    Returns the current configuration pointer for the specified module

*************************************************************/

const void* OI_ConfigTable_GetConfig(OI_MODULE module)
{
    OI_ASSERT(module < OI_NUM_STACK_MODULES) ;
    if (module >= OI_NUM_STACK_MODULES) {
        return NULL;
    }
    OI_ASSERT(NULL != (curConfig[module].pConfig));
    return(curConfig[module].pConfig) ;
}

/*************************************************************

    OI_ConfigTable_SetConfig (configPtr, module)

    The specified pointer becomes the current configuration pointer for the indicated module.

*************************************************************/

void  OI_ConfigTable_SetConfig(const void *configPtr, OI_MODULE module)
{
    OI_INIT_FLAG retVal;
    OI_ASSERT(module < OI_NUM_STACK_MODULES);
    OI_LOG_ERROR(("OI_ConfigTable_SetConfig fail: module %d is already initialized", module)) ;
    // can't set config if module is already initialized
    retVal = OI_InitFlags_GetFlag(module);
    OI_ASSERT((OI_INIT_FLAG_UNINITIALIZED_VALUE == retVal));
    if (OI_INIT_FLAG_UNINITIALIZED_VALUE != OI_InitFlags_GetFlag(module)) {
        OI_LOG_ERROR(("OI_ConfigTable_SetConfig fail: module %d is already initialized", module)) ;
        return ;
    }

    // Common configuration can be changed only if ALL modules are uninitialized
    if ((OI_MODULE_COMMON_CONFIG == module) && !OI_InitFlags_AllUninitialized()) {
        OI_LOG_ERROR(("OI_ConfigTable_SetConfig fail: cannot set Common Configuration")) ;
        return ;
    }

    curConfig[module].pConfig = configPtr ;
}

/*****************************************************************************/



