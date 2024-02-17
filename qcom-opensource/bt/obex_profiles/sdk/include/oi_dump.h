#ifndef _OI_DUMP_H
#define _OI_DUMP_H

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

/** @file

      This header file exposes interfaces to various dump utilties for debugging.

    */

#include "oi_utils.h"

/** \addtogroup Debugging Debugging APIs */
/**@{*/

#ifdef __cplusplus
extern "C" {
#endif



#ifdef OI_DEBUG

/** Dump utility for debugging L2CAP. */
void OI_L2CAP_Dump(void);

/** Dump utility for debugging RFCOMM. */
void OI_RFCOMM_Dump(void);

/** Dump utility for debugging HCI. */
void OI_HCI_Dump(void);

/** Dump utility for debugging the Device Manager. */
void OI_DEVMGR_Dump(void);

/** Dump utility for debugging the Policy Manager. */
void OI_POLICYMGR_Dump(void);

/** Dump utility for debugging the Security Manager. */
void OI_SECMGR_Dump(void);

/** Dump utility for debugging the AMP. */
void OI_AMP_Dump(void);

/** Dump utility for debugging SDPDB. */
void OI_SDPDB_Print(void);

/** Dump utility for debugging handle manager. */
void OI_HANDLE_Dump(void);

/** Dump utility for debugging Dispatcher. */
void OI_DISPATCH_Dump(void);

/** Dump utility for debugging AMP policies. */
void OI_AMP_PolicyDump(void);

#else /* OI_DEBUG */

#define OI_L2CAP_Dump()                  OI_Printf("\nNot compiled with OI_DEBUG\n");

#define OI_RFCOMM_Dump()                 OI_Printf("\nNot compiled with OI_DEBUG\n");

#define OI_HCI_Dump()                    OI_Printf("\nNot compiled with OI_DEBUG\n");

#define OI_DEVMGR_Dump()                 OI_Printf("\nNot compiled with OI_DEBUG\n");

#define OI_POLICYMGR_Dump()              OI_Printf("\nNot compiled with OI_DEBUG\n");

#define OI_SECMGR_Dump()                 OI_Printf("\nNot compiled with OI_DEBUG\n");

#define OI_AMP_Dump()                    OI_Printf("\nNot compiled with OI_DEBUG\n");

#define OI_SDPDB_Print()                 OI_Printf("\nNot compiled with OI_DEBUG\n");

#define OI_HANDLE_Dump()                 OI_Printf("\nNot compiled with OI_DEBUG\n");

#define OI_DISPATCH_Dump()               OI_Printf("\nNot compiled with OI_DEBUG\n");

#define OI_AMP_PolicyDump()              OI_Printf("\nNot compiled with OI_DEBUG\n");

#endif /* OI_DEBUG */


#ifdef MEMMGR_PROFILE

/** Dump utility for debugging the Memory Manager. */
void OI_MEMMGR_Dump(void);

#else

#define OI_MEMMGR_Dump()           OI_Printf("\nNot compiled with MEMMGR_PROFILE\n")

#endif /* MEMMGR_PROFILE */


#ifdef MEMMGR_DEBUG

/** Dump utility for debugging the used blocks in the Memory Manager. */
void OI_MEMMGR_DumpUsedBlocks(void);

/** Dump utility for debugging the memory pools in the Memory Manager. */
void OI_MEMMGR_DumpPools(void);

#else /* MEMMGR_DEBUG */

#define OI_MEMMGR_DumpUsedBlocks() OI_Printf("\nNot compiled with MEMMGR_DEBUG\n")

#define OI_MEMMGR_DumpPools()      OI_Printf("\nNot compiled with MEMMGR_DEBUG\n")

#endif /* MEMMGR_DEBUG */



#ifdef __cplusplus
}
#endif

/**@}*/

#endif /* _OI_DUMP_H */

