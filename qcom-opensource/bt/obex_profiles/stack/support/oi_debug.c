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

#define __OI_MODULE__ OI_MODULE_SUPPORT

#include "oi_argcheck.h"
#include "oi_debug.h"
#include "oi_assert.h"
#include "oi_osinterface.h"
#include "oi_memmgr.h"
#include "oi_debugcontrol.h"
#include "oi_bt_assigned_nos.h"
#include "oi_std_utils.h"
#include "oi_dispatch.h"
#include "oi_debugcontrol.h"
#include "oi_dump.h"
#include "oi_support_init.h"
#include "oi_varstring.h"
#include "oi_bt_stack_config.h"
#include "oi_config_table.h"
#include "oi_simplemap.h"

typedef struct {
    OI_MODULE module;
    OI_UINT8  levelEnableFlags;
    OI_UINT8  enableLineNum;
    OI_UINT8  overrideAll;
    OI_UINT8  useDefault;
} OI_DBG_OUTPUT_CTRL;


#if defined(OI_DEBUG) || defined(OI_DEBUG_PER_MOD)
/** The debug info for the modules. IMPORTANT: these must be in the same order
 *  as in the OI_MODULE definition found in sdk/include/oi_modules.h.
 */
static const OI_CHAR* moduleString[] = {
    /* profiles and protocols --> updates to oi_modules.h */

    "AT",           /* OI_MODULE_AT */
    "A2DP",         /* OI_MODULE_A2DP */
    "AVCTP",        /* OI_MODULE_AVCTP */
    "AVDTP",        /* OI_MODULE_AVDTP */
    "AVRCP",        /* OI_MODULE_AVRCP */
    "BIP",          /* OI_MODULE_BIP_CLI */
    "BIP",          /* OI_MODULE_BIP_SRV */
    "BNEP",         /* OI_MODULE_BNEP */
    "BPP",          /* OI_MODULE_BPP_SENDER */
    "BPP",          /* OI_MODULE_BPP_PRINTER */
    "CTP",          /* OI_MODULE_CTP */
    "DUN",          /* OI_MODULE_DUN */
    "FAX",          /* OI_MODULE_FAX */
    "FTP",          /* OI_MODULE_FTP_CLI */
    "FTP",          /* OI_MODULE_FTP_SRV */
    "HANDSFREE",    /* OI_MODULE_HANDSFREE */
    "HANDSFREE_AG", /* OI_MODULE_HANDSFREE_AG */
    "HCRP_CLI",     /* OI_MODULE_HCRP */
    "HCRP_SRV",     /* OI_MODULE_HCRP */
    "HEADSET",      /* OI_MODULE_HEADSET */
    "HEADSET_AG",   /* OI_MODULE_HEADSET_AG */
    "HID",          /* OI_MODULE_HID */
    "INTERCOM",     /* OI_MODULE_INTERCOM */
    "OBEX",         /* OI_MODULE_OBEX_CLI */
    "OBEX",         /* OI_MODULE_OBEX_SRV */
    "OPP",          /* OI_MODULE_OPP_CLI */
    "OPP",          /* OI_MODULE_OPP_SRV */
    "PAN",          /* OI_MODULE_PAN */
    "PBAP",         /* OI_MODULE_PBAP_CLI */
    "PBAP",         /* OI_MODULE_PBAP_SRV */
    "SAP",          /* OI_MODULE_SAP_CLI */
    "SAP",          /* OI_MODULE_SAP_SRV */
    "SPP",          /* OI_MODULE_SPP */
    "SYNC",         /* OI_MODULE_SYNC_CLI */
    "SYNC",         /* OI_MODULE_SYNC_SRV */
    "SYNC",         /* OI_MODULE_SYNC_CMD_CLI */
    "SYNC",         /* OI_MODULE_SYNC_CMD_SRV */
    "SYNCML",       /* OI_MODULE_SYNCML */
    "TCS",          /* OI_MODULE_TCS */
    "VDP",          /* OI_MODULE_VDP */
    "MAP",          /* OI_MODULE_MAP_CLI */
    "MAP",          /* OI_MODULE_MAP_SRV */

    /* BLUEmagic 3.0 software kernel components --> updates to oi_modules.h */

    "COMMON_CONFIG",/* OI_MODULE_COMMON_CONFIG */
    "CMDCHAIN",     /* OI_MODULE_CMDCHAIN */
    "DISPATCH",     /* OI_MODULE_DISPATCH */
    "DATAELEM",     /* OI_MODULE_DATAELEM */
    "DEVMGR",       /* OI_MODULE_DEVMGR */
    "DEVMGR",       /* OI_MODULE_DEVMGR_MODES */
    "HCI",          /* OI_MODULE_HCI */
    "L2CAP",        /* OI_MODULE_L2CAP */
    "MEMMGR",       /* OI_MODULE_MEMMGR */
    "POLICYMGR",    /* OI_MODULE_POLICYMGR */
    "RFCOMM",       /* OI_MODULE_RFCOMM */
    "RFCOMM_SD",    /* OI_MODULE_RFCOMM_SD */
    "SDP",          /* OI_MODULE_SDP_CLI */
    "SDP",          /* OI_MODULE_SDP_SRV */
    "SDP",          /* OI_MODULE_SDPDB */
    "SECMGR",       /* OI_MODULE_SECMGR */
    "SUPPORT",      /* OI_MODULE_SUPPORT */
    "TRANSPORT",    /* OI_MODULE_TRANSPORT */
    "TEST",         /* OI_MODULE_TEST */
    "XML",          /* OI_MODULE_XML */
    "DI",           /* OI_MODULE_DI */
    "AMP",          /* OI_MODULE_AMP */
    "OBEX_SD",      /* OI_MODULE_OBEX_SD */
    "BTLE",         /* OI_MODULE_BTLE */

    /* OEM files --> Updates to oi_modules.h */
    "OEM",          /* OI_MODULE_OEM */

    /* Application glue --> updates to oi_modules.h */
    "APP",          /* OI_MODULE_APP */

    /* Platform modules -> updates to oi_modules.h */
    "DBUS",            /* OI_MODULE_DBUS */
    "SOCKETS",         /* OI_MODULE_SOCKETS */
    "PAL",             /* OI_MODULE_PAL */

    /* Various pieces of code depend on these last 2 elements occuring in a specific order:
       OI_MODULE_ALL must be the 2nd-to-last element
       OI_MODULE_UNKNOWN must be the last element
       */

    "ALL",          /* OI_MODULE_ALL */
    "UNKNOWN"       /* OI_MODULE_UNKNOWN */
};

/* Global debug table. Must have the same number of entries as moduleString.
 * The table is designed to allow easier debugging with JTAG: the layout is
 * 8 bytes, where the first four byte represent the module ID
 * (can be looked up in oi_module.h) and the following settings represent
 * debug options. The most common setting to enable out put of debug info
 * for a certain module would be to set "levelEnableFlags" to 0x3e and
 *  "enableLineNum" to 1.
  */
OI_DBG_OUTPUT_CTRL OI_ModuleDbgControl[OI_NUM_MODULES + 2];

#endif


/**
 * If OI_TEST_HARNESS is not defined we use smaller debug print buffers. This means that debug
 * messages may be truncated.
 */

#ifdef OI_TEST_HARNESS
#define MAX_HEX_TEXT_LEN   2048  /* Must be at least 18 bytes for BD Addr */
#define MAX_DBG_MSG_LEN    4096
#else
#define MAX_HEX_TEXT_LEN   128  /* Must be at least 18 bytes for BD Addr */
#define MAX_DBG_MSG_LEN    256
#endif

#define MAX_DBG_HDR_LEN     64


/*
 * This global variable allows any BM3 function to verify that the
 * current process does in fact own the stack token.
 *
 * Initialize to TRUE for those platforms which do not choose to
 * manipulate this variable from within stackwrapper token handling.
 */

OI_BOOL OI_StackTokenHeld = TRUE;

/**
 * Formatting buffer (possibly large) shared by OI_DbgPrint and OI_Printf
 */
static OI_CHAR msgBuf[MAX_DBG_MSG_LEN];

/**
 * Smaller scratch buffer
 */
static OI_CHAR smallFmtBuf[MAX_HEX_TEXT_LEN];

#if defined(OI_DEBUG) || defined(OI_DEBUG_PER_MOD)
/**
 * Header buffer only used by DbgPrint
 */
static OI_CHAR hdrBuf[MAX_DBG_HDR_LEN];

typedef struct {
    OI_BOOL checked;         /* make sure _OI_Dbg_Check() is called before _OI_DbgPrint() */
    OI_UINT8 debugLevel;     /* requested debug level */
    OI_UINT8 module;         /* current module */
    OI_UINT8 hdrIndent;      /* number of characters in the debug header */
    OI_CHAR *hdr;            /* debug header string */
    OI_BOOL linePrinted;     /* TRUE if the line was printed. */
} DBG_INFO;

static DBG_INFO dbgInfo;
int default_log_level;


static OI_DBG_OUTPUT_CTRL defaultDbgControl = {
    OI_MODULE_UNKNOWN,
    OI_DBG_MSG_ALL_ENABLE,
    TRUE,
    FALSE,
    FALSE
};

#endif

/**
 * Sets the logging level
 *
 * @param level    The level for which logs would be printed
 */
void OI_SetLogLevel(OI_UINT16 level)
{
    default_log_level = level;
}
OI_STATUS OI_Support_Init(void)
{
#if defined(OI_DEBUG) || defined(OI_DEBUG_PER_MOD)

    OI_UINT i;

    for (i = 0; i< OI_ARRAYSIZE(OI_ModuleDbgControl); i++) {
        OI_ModuleDbgControl[i].module = (OI_MODULE) i;
    }

    /* All modules show errors and warnings by default. */
    if (OI_ModuleDbgControl[OI_MODULE_ALL].levelEnableFlags == 0) {
        OI_ModuleDbgControl[OI_MODULE_ALL].levelEnableFlags = OI_DBG_MSG_ERROR_ENABLE;
    }

    OI_ModuleDbgControl[OI_MODULE_UNKNOWN] = defaultDbgControl;
    OI_ModuleDbgControl[OI_MODULE_UNKNOWN].overrideAll = TRUE;

    dbgInfo.checked = FALSE;
#endif

    OI_SimpleMap_Init();
    default_log_level = OI_MSG_CODE_TRACE;

    return OI_OK;
}


OI_CHAR* OI_BDAddrText(const OI_BD_ADDR *Addr)
{
    OI_VARSTRING BDAddrVStr = {FALSE, OI_ARRAYSIZE(smallFmtBuf), 0, smallFmtBuf, FALSE};

    OI_FormatStr(&BDAddrVStr, "%:", Addr);
    return OI_VStrGetString(&BDAddrVStr);
}


OI_CHAR* OI_HexText(const OI_BYTE* Bytes, OI_UINT16 Len)
{
    OI_VARSTRING hexVStr = {FALSE, OI_ARRAYSIZE(smallFmtBuf), 0, smallFmtBuf, FALSE};

    OI_FormatStr(&hexVStr, "%@", Bytes, Len);
    return OI_VStrGetString(&hexVStr);
}


void OI_VPrintf(const OI_CHAR *format, va_list argp)
{
    OI_VARSTRING dbgVStr = {FALSE, OI_ARRAYSIZE(msgBuf), 0, msgBuf, FALSE};
    OI_STATUS status;

    status = OI_VFormatStr(&dbgVStr, format, argp);
    if (!OI_SUCCESS(status)) {
        dbgVStr.Len = 0;
        OI_FormatStr(&dbgVStr, "Invalid parameter in debug output %! \"%s\"", status, format);
    }
    OI_Print(OI_VStrGetString(&dbgVStr));
}


OI_INT32 OI_VSNPrintf(OI_CHAR *buffer,
                      OI_UINT16 bufLen,
                      const OI_CHAR* format,
                      va_list argp)
{
    OI_STATUS status;
    OI_VARSTRING bufVStr;

    if (!buffer || !bufLen) {
        return -1;
    }

    bufVStr.Managed = FALSE;
    bufVStr.MaxLen = bufLen - 1;
    bufVStr.Len = 0;
    bufVStr.Buffer= buffer;

    status = OI_VFormatStr(&bufVStr, format, argp);
    if (OI_SUCCESS(status)) {
        buffer[bufVStr.Len] = 0;
        return bufVStr.Len;
    } else {
        return -1;
    }
}


OI_INT32 OI_SNPrintf(OI_CHAR *buffer,
                     OI_UINT16 bufLen,
                     const OI_CHAR* format, ...)
{
    OI_INT32 len;
    va_list argp;

    va_start(argp, format);
    len = OI_VSNPrintf(buffer, bufLen, format, argp);
    va_end(argp);
    return len;
}


void OI_Printf(const OI_CHAR* format, ...)
{
    va_list argp;

    va_start(argp, format);
    OI_VPrintf(format, argp);
    va_end(argp);
}


