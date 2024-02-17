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
@file
@internal

Test harness support for OBEX
*/

#define __OI_MODULE__ OI_MODULE_OBEX_SRV

#include "oi_obextest.h"
#include "oi_debug.h"
#include "oi_test.h"
#include "oi_utils.h"


#ifdef OI_TEST_HARNESS

OI_OBEX_TEST_GLOBALS OI_ObexTest;


static OI_STATUS ObexTest(const OI_CHAR *cmd)
{
    static const OI_CHAR cmdOpts[] = "SRM|L2CAP|FRAME_SZ|ABORT|ERROR_RSP";
    static const OI_CHAR tf[] = "T|F";
    static const OI_CHAR ignore[] = "ignore";
    const OI_CHAR *args;
    OI_INT index;
    OI_BOOL inputOk = FALSE;

    args = OI_ScanAlt(cmd, cmdOpts, &index);
    switch (index) {
        case 0:
            if (OI_ScanAlt(args, tf, &index) != args) {
                inputOk = TRUE;
                OI_ObexTest.disableSRM = (index == 1);
                OI_ObexTest.ignoreSRM = FALSE;
                OI_Printf("SRM %s\n", OI_ObexTest.disableSRM ? "disabled" : "enabled");
                break;
            }
            if (OI_ScanAlt(args, ignore, &index) != args) {
                inputOk = TRUE;
                OI_ObexTest.ignoreSRM = TRUE;
                OI_Printf("SRM will be ignored (incorrect behavior)\n");
            }
            break;
        case 1:
            if (OI_ScanAlt(args, tf, &index) != args) {
                inputOk = TRUE;
                OI_ObexTest.disableL2CAP = (index == 1);
                OI_Printf("OBEX/L2CAP %s\n", OI_ObexTest.disableL2CAP ? "disabled" : "enabled");
            }
            break;
        case 2:
            if (OI_ScanUInt(args, &OI_ObexTest.frameSize) != args) {
                inputOk = TRUE;
                OI_Printf("Max RFCOMM frame size is %d\n", OI_ObexTest.frameSize);
            }
            break;
        case 3:
            if (OI_ScanAlt(args, tf, &index) != args) {
                inputOk = TRUE;
                OI_ObexTest.fastAbort = (index == 0);
                OI_Printf("OBEX PUT fast abort %s\n", OI_ObexTest.fastAbort ? "enabled" : "disabled");
            }
            break;
        case 4:
            if (OI_ScanAlt(args, tf, &index) != args) {
                inputOk = TRUE;
                OI_ObexTest.SendErrorRsp = (index == 0);
            }
            break;
    }

    if (!inputOk) {
        OI_Printf("\nUsage: %s\n", cmdOpts);
        OI_Printf("  SRM T|F|ignore    (enable, disable or ignore Single Response Mode)\n");
        OI_Printf("  L2CAP T|F  (enable or disable OBEX over L2CAP)\n");
        OI_Printf("  FRAME_SZ N (set maximum RFCOMM frames size)\n");
        OI_Printf("  ABORT T|F (send an abort before receiving a put response)\n");
        OI_Printf("  ERROR_RSP T|F (send an error in a put/get response)\n");
    }
    return OI_OK;
}


void OI_OBEX_TestInit(void)
{
    static OI_BOOL registered = FALSE;

    if (!registered) {
        OI_STATUS status;

        registered = TRUE;
        OI_ObexTest.disableSRM = FALSE;
        OI_ObexTest.disableL2CAP = FALSE;
        OI_ObexTest.fastAbort = FALSE;
        OI_ObexTest.SendErrorRsp = FALSE;

        status = OI_TEST_RegisterHandler(ObexTest, "obex");
        if (!OI_SUCCESS(status)) {
            OI_SLOG_ERROR(status, ("OI_TEST_RegisterHandler (obex) failed"));
        }
    }
}

#endif
