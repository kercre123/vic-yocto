#ifndef _OI_TIME_H
#define _OI_TIME_H

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
 *
 * This file provides time type definitions and interfaces to time-related functions.
 *
 * The stack maintains a 64-bit real-time millisecond clock. The choice of
 * milliseconds is for convenience, not accuracy.
 *
 * Timeouts are specified as tenths of seconds in a 32-bit value. Timeout values
 * specified by the Bluetooth specification are usually muliple seconds, so
 * accuracy to a tenth of a second is more than adequate.
 *
 * This file also contains macros to convert between seconds and the Link
 * Manager's 1.28-second units.
 *
 */

#include "oi_stddefs.h"


/** \addtogroup Misc Miscellaneous APIs */
/**@{*/

#ifdef __cplusplus
extern "C" {
#endif



/**
 * Within the core stack, timeouts are specified in intervals of tenths of seconds.
 */

typedef OI_UINT16 OI_INTERVAL;
#define OI_INTERVALS_PER_SECOND     10
#define MSECS_PER_OI_INTERVAL       (1000 / OI_INTERVALS_PER_SECOND)

/** Maximum interval (54 min 36.7 sec) */
#define OI_MAX_INTERVAL   0x7fff


/**
 * The granularity of timeouts is OI_INTERVAL this macro rounds up timout values that are specified
 * as milliseconds to the nearest OI_INTERVAL equivalent. For example this will round up 90ms
 * to 100ms which is the smallest OI_INTERVAL value.
 */
#define OI_MILLISECONDS_ROUND_UP(n)   ((((n) + MSECS_PER_OI_INTERVAL - 1) / MSECS_PER_OI_INTERVAL) * MSECS_PER_OI_INTERVAL)

/**
 * Macro to convert seconds to OI_INTERVAL time units
 */

#define OI_SECONDS(n)    ((OI_INTERVAL) ((n) * OI_INTERVALS_PER_SECOND))

/**
 * Macro to convert milliseconds to OI_INTERVAL time units (Rounded Up)
 */

#define OI_MSECONDS(n)   ((OI_INTERVAL) (((n) + MSECS_PER_OI_INTERVAL - 1) / MSECS_PER_OI_INTERVAL))

/**
 * Macro to convert minutes to OI_INTERVAL time units
 */

#define OI_MINUTES(n)    ((OI_INTERVAL) ((n) * OI_SECONDS(60)))

/** Convert an OI_INTERVAL to milliseconds. */
#define OI_INTERVAL_TO_MILLISECONDS(i) ((i) * MSECS_PER_OI_INTERVAL)

/**
   This structure defines the time. The stack depends on
   relative not absolute time. Any mapping between the stack's
   real-time clock and absolute time and date is
   implementation-dependent.
 */

typedef struct {
    OI_INT32 seconds;
    OI_INT16 mseconds;
} OI_TIME;

/**
 * This function converts an OI_TIME to milliseconds.
 *
 * @param t  Specifies the time to convert
 *
 * @return Time in milliseconds
 */
OI_UINT32 OI_Time_ToMS(OI_TIME *t);


/**
 * This function compares two time values.
 *
 * @param T1 First time to compare.
 *
 * @param T2 Second time to compare.
 *
 * @return
 @verbatim
     -1 if t1 < t2
      0 if t1 = t2
     +1 if t1 > t2
 @endverbatim
 */

OI_INT16 OI_Time_Compare(OI_TIME *T1,
                         OI_TIME *T2);


/**
 * This function returns the interval between two times to a granularity of 0.1 seconds.
 *
 * @param Sooner Specifies a time value more recent that Later.
 *
 * @param Later Specifies a time value later than Sooner.
 *
 * @note The result is an OI_INTERVAL value so this function only works for time intervals
 * that are less than about 71 minutes.
 *
 * @return Time interval between the two times = (Later - Sooner)
 */

OI_INTERVAL OI_Time_Interval(OI_TIME *Sooner,
                             OI_TIME *Later);



/**
 * This function returns the interval between two times to a granularity of milliseconds.
 *
 * @param Sooner Specifies a time value more recent that Later.
 *
 * @param Later Specifies a time value later than Sooner.
 *
 * @note The result is an OI_UINT32 value so this function only works for time intervals
 * that are less than about 50 days.
 *
 * @return Time interval between the two times = (Later - Sooner)
 */

OI_UINT32 OI_Time_IntervalMsecs(OI_TIME *Sooner,
                                OI_TIME *Later);



/**
 * This function answers the question, Have we reached or gone past the target time?
 *
 * @param pTargetTime   Target time
 *
 * @return  TRUE means that the time now is at or past target time;
 *          FALSE means that the target time is still some time in the future
 */

OI_BOOL  OI_Time_NowReachedTime(OI_TIME *pTargetTime);

/**
 *  Convert seconds to the Link Manager 1.28-second units; approximate by using 1.25 conversion factor.
 */

#define OI_SECONDS_TO_LM_TIME_UNITS(lmUnits) ((lmUnits)<4?(lmUnits):(lmUnits)-((lmUnits)>>2))


/**
 *  Convert Link Manager 1.28-second units to seconds;
 *  approximate by using 1.25 conversion factor.
 */

#define OI_LM_TIME_UNITS_TO_SECONDS(lmUnits) ((lmUnits) + ((lmUnits)>>2))

#ifdef __cplusplus
}
#endif

/**@}*/

/* Include for OI_Time_Now() prototype;
 * must be included at end to obtain OI_TIME typedef.
 */
#include "oi_osinterface.h"

/*****************************************************************************/
#endif /* _OI_TIME_H */

