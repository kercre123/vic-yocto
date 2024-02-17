#ifndef _OI_CONFIG_TABLE_H
#define _OI_CONFIG_TABLE_H

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
 * This file provides the interface to the configuration table.
 *
 * Associated with each module defined in oi_modules.h is a
 * configuration structure type definition.
 *
 *  Naming conventions:
 *  - Configuration type definition is OI_CONFIG_<module>
 *    (e.g., OI_CONFIG_MEMMGR, OI_CONFIG_L2CAP).
 *  - Default configuration structure name is oi_default_config_<module>
 *    (e.g., oi_default_config_MEMMGR, oi_default_config_L2CAP)
 *
 *
 *
 * Do not call the function OI_ConfigTable_Init() unluess you are sure that you
 * know what you are doing. It should only be called as part of a global, reset/restart
 * process. Initialization sets the all configuration pointers to their default values.
 * This function should be called before initializing any profiles or core protocol stack
 * and support module components. This function is defined in oi_bt_stack_init.h with the
 * other stack initialization prototypes.
 *
 */

#include "oi_stddefs.h"
#include "oi_modules.h"

/** \addtogroup InitConfig */
/**@{*/

#ifdef __cplusplus
extern "C" {
#endif


extern const void* OI_ConfigTable_GetConfig(OI_MODULE module) ;
extern void  OI_ConfigTable_SetConfig(const void *configPtr, OI_MODULE module) ;

/**
 *    This macro is used for accessing the configuration table.
      It returns a pointer to the configuration structure for
      the module.

      Specify the module name without the OI_MODULE_ prefix. For
      example, call OI_CONFIG_TABLE_GET (DEVMGR), not
      OI_CONFIG_TABLE_GET (OI_MODULE_DEVMGR). The result is a
      pointer to the module's configuration structure.
 */
#define OI_CONFIG_TABLE_GET(module)              \
        ((const OI_CONFIG_##module*)(OI_ConfigTable_GetConfig(OI_MODULE_##module)))

/**
 * This macro is provided primarily to enable backward compatibility with applications
   written with previous versions of the BLUEmagic 3.0 APIs.

   Normally, configuration structures are compiled into the
   application by defining a the variable
   oi_default_config_<module>. This macro updates the
   configuration table entry for the specified module with the
   specified pointer. The caller must guarantee the integrity of
   both the pointer and the data being pointed to. The
   configuration structure may not change in any way while the
   module is 'in use'. If you cannot be sure when a module is
   'in use' or not, do not change the configuration.
 *
 *    Specify module name without the OI_MODULE_ prefix. For example, call
 *    OI_CONFIG_TABLE_GET (DEVMGR), not OI_CONFIG_TABLE_GET (OI_MODULE_DEVMGR).
 *
 *
 *
   Usage notes: \n To set the configuration for a module, you
   should define a static const structure of the appropriate
   type and then use OI_CONFIG_TABLE_PUT() to update the configuration table
   with your structure. The configuration table retains a
   pointer to your structure, so your structure must persist and
   the contents of your structure may not be changed after it
   has been put into the table.
 *
 * If should you wish to change one or more individual elements in a structure while retaining the
   default values for other elements, you should do the
   following:
 *
       -# Use OI_CONFIG_TABLE_GET() to get a reference to the
         current configuration.
       -# Copy the current configuration into a local, static
         structure.
       -# Modify the desired elements in the local structure.
       -# Use OI_CONFIG_TABLE_PUT() to put a reference to your
        local structure into the configuration table.
 *
 */

#define OI_CONFIG_TABLE_PUT(configPtr, module)   \
        (OI_ConfigTable_SetConfig(configPtr, OI_MODULE_##module))

/** Validate the parameters in the configuration table. */
void OI_ConfigTable_Validate(void);

/*****************************************************************************/
#ifdef __cplusplus
}
#endif

/**@}*/

#endif /* _OI_CONFIG_TABLE_H */

