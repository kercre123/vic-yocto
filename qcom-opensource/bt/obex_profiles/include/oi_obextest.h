#ifndef _OI_OBEXTEST_H
#define _OI_OBEXTEST_H

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

#include "oi_stddefs.h"


#ifdef OI_TEST_HARNESS

typedef struct {
    OI_BOOL delaySRM;     /**< Delay start of SRM by sending an SRMP WAIT command */
    OI_BOOL ignoreSRM;    /**< Ignore negotiated SRM - i.e. behave badly */
    OI_BOOL disableSRM;   /**< Disable Single Response Mode */
    OI_BOOL disableL2CAP; /**< Disable OBEX/L2cAP */
    OI_UINT32 frameSize;  /**< Maximum RFCOMM frame size for OBEX/RFCOMM */
    OI_BOOL fastAbort;    /**< Abort a put without waiting for a reply */
    OI_BOOL SendErrorRsp; /**< Send an error in the PUT/GET response */
} OI_OBEX_TEST_GLOBALS;


extern OI_OBEX_TEST_GLOBALS OI_ObexTest;

void OI_OBEX_TestInit(void);

#else

#define OI_OBEX_TestInit()

#endif


#endif /* _OI_OBEXTEST_H */
