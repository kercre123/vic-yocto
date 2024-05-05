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
 * This file provides default routines for the initialization and termination of the
 * BLUEmagic 3.0 protocol stack
 *
 *
 *  This default initialization a method for vendor-specific initialization.
 *      Define the token USE_DEFAULT_VENDOR_INIT (any value)
 *      his will cause the external function OI_DefaultVendorInit() to be called.
 *
 */

#define __OI_MODULE__ OI_MODULE_OEM

#include "oi_common.h"
#include "oi_osinterface.h"
#include "oi_argcheck.h"
#include "oi_utils.h"
#include "oi_bt_stack_init.h"
#include "oi_bt_module_init.h"
#include "oi_support_init.h"
#include "oi_wrapper.h"
#include "oi_bt_stack_config.h"
#include "oi_config_table.h"
#include "oi_debug.h"

/*
 * Initialization completion callback
 */
static OI_INIT_COMPLETE_CB stackInitCb;

/*
 * Initializer functions which bring up all required portions of BM3
 */
OI_STATUS OI_BT_StackInit(OI_INIT_COMPLETE_CB                 initComplete,
                          OI_DISPATCH_SERVICE_REQUEST_HANDLER handler)
{
    return(OI_BT_StackInit_Custom(initComplete, handler, NULL));
}

OI_STATUS OI_BT_StackInit_Custom(OI_INIT_COMPLETE_CB                 initComplete,
                                 OI_DISPATCH_SERVICE_REQUEST_HANDLER handler,
                                 void (*customCB)(void))
{
    OI_STATUS status;

    OI_ARGCHECK(NULL != initComplete);
    OI_ARGCHECK(NULL != handler);

    stackInitCb = initComplete;

    /*
     * Initialize infrastructure needed by all modules
     */
    OI_InitFlags_ResetAllFlags();
    OI_ConfigTable_Init();
    /*
     * Give application's a chance to override default configurations (if desired)
     */
    if (NULL != customCB)  {
        OI_Print("Invoking customization callback\n");
        customCB();
    }

    status = OI_Support_Init();
    if (!OI_SUCCESS(status)) {
        OI_Printf("Support code initialization failed\n");
        return status;
    }

    /*
     * Memory Manager
     */
    status = OI_MEMMGR_Init(OI_CONFIG_TABLE_GET(MEMMGR));
    if (!OI_SUCCESS(status)) {
        OI_Printf("Memory Manager initialization failed\n");
        return status;
    }

    /*
     * Dispatcher
     */
    status = OI_Dispatch_Init(handler);
    if (!OI_SUCCESS(status)) {
        OI_Printf("OI_Dispatch_Init failed\n");
        return status;
    }

    /*
     * EventLoop
     */
    status = OI_EVENTLOOP_Init();
    if (!OI_SUCCESS(status)) {
        OI_Printf("OI_EVENTLOOP_Init failed\n");
        return status;
    }

    /* initialization has begun, callback will be called when init has completed */
    return OI_OK;
}

OI_STATUS OI_BT_Stack_Terminate(void)
{
    OI_EVENTLOOP_Shutdown();

    OI_Dispatch_Terminate();

    OI_InitFlags_ResetAllFlags();

    return OI_OK;

}

