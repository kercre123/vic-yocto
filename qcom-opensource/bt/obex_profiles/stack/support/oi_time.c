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
@file
@internal
This file implements timer functions.
*/

#define __OI_MODULE__ OI_MODULE_SUPPORT

#include "oi_time.h"
#include "oi_assert.h"
#include "oi_osinterface.h"
#include "oi_debug.h"

OI_UINT32 OI_Time_ToMS(OI_TIME *t)
{
    return (1000 * t->seconds + t->mseconds);
}

/*
 * OI_Time_Compare
 *
 */

OI_INT16 OI_Time_Compare(OI_TIME *T1,
                         OI_TIME *T2)

{
    OI_ASSERT((T1 != NULL) & (T2 != NULL));

    if (T1->seconds < T2->seconds) {
        return -1;
    }
    if (T1->seconds > T2->seconds) {
        return 1;
    }
    if (T1->mseconds < T2->mseconds) {
        return -1;
    }
    if (T1->mseconds > T2->mseconds) {
        return 1;
    }
    return 0;
}

/*
 * OI_Time_Interval
 *
 */

OI_INTERVAL OI_Time_Interval(OI_TIME *Sooner,
                             OI_TIME *Later)
{
    OI_INT32 seconds = Later->seconds - Sooner->seconds;
    OI_INT16 mseconds = Later->mseconds - Sooner->mseconds;

    OI_ASSERT(Later->seconds >= Sooner->seconds);
    OI_ASSERT((Later->seconds != Sooner->seconds) || (Later->mseconds >= Sooner->mseconds));

    if (mseconds < 0) {
        mseconds += 1000;
        --seconds;
    }
    /*
     * OI_INTERVALS are only accurate to within 1/10 of a second so we can avoid doing a potentially
     * expensive divide operation by approximating a divide by 100 to a multiply by 10 and divide by
     * 1024 which any reasonable compiler will implement as a shift.
     */
    return (OI_INTERVAL) (seconds * 10 + (mseconds * 10 + 512) / 1024);
}



/*
 * OI_Time_IntervalMsecs
 *
 */

OI_UINT32 OI_Time_IntervalMsecs(OI_TIME *Sooner,
                                OI_TIME *Later)
{
    OI_INT32 seconds = Later->seconds - Sooner->seconds;
    OI_INT16 mseconds = Later->mseconds - Sooner->mseconds;

    OI_ASSERT(Later->seconds >= Sooner->seconds);
    OI_ASSERT((Later->seconds != Sooner->seconds) || (Later->mseconds >= Sooner->mseconds));

    if (mseconds < 0) {
        mseconds += 1000;
        --seconds;
    }

    return seconds * 1000 + mseconds;
}



/*
 * OI_Time_NowReachedTime
 *
 */

OI_BOOL  OI_Time_NowReachedTime(OI_TIME *pTargetTime)
{
    OI_TIME now;

    OI_Time_Now(&now);
    return(OI_Time_Compare(&now, pTargetTime) >= 0);
}
/*****************************************************************************/
