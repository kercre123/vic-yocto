#ifndef _OI_STATUSTEXT_H
#define _OI_STATUSTEXT_H

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
 * This file provides the API for a single debugging function, which provides the
 * text string for OI_STATUS values.
 * This function is only defined in the debug version of the stack library.
 *
 *
 */

#include "oi_status.h"

/** \addtogroup Debugging Debugging APIs */
/**@{*/

#ifdef __cplusplus
extern "C" {
#endif


/**
 * Map a status code into a text string for printing out error statuses in
 * the SDK samples and in application test programs.
 *
 * Application error messages are returned by a status text filter registered via
 * OI_RegisterAppStatusTextFilter().
 *
 * @param value a status code.
 *
 * @return a pointer to text string.
 */

OI_CHAR* OI_StatusText(OI_STATUS value);

#ifdef OI_DEBUG

/**
 * A function of this type may be registered with the status text module in order
 * for the application to provide a text string for application defined status values.
 *
 * @param value an application defined status code.
 *
 * @return a pointer to a text string.  This may be NULL, in which case the value
 *         is unknown to the filter and may be handled by another filter.
 */
typedef OI_CHAR* (*OI_APP_STATUS_TEXT_FILTER)(OI_STATUS value);

/**
 * This registers an application function that formats application defined status
 * codes into text strings.  The registered function will be called whenever a status
 * code in the application range is to be printed.
 *
 * Several filters can be registered. They will be called in order until one returns
 * a text string.
 *
 * @param filter the function called when an application defined status code is
 *               being displayed.
 */
void OI_RegisterAppStatusTextFilter(OI_APP_STATUS_TEXT_FILTER filter);

#else

/*
 * This function does nothing in a non-debug build
 */
#define OI_RegisterAppStatusTextFilter(filter)

#endif

/*****************************************************************************/
#ifdef __cplusplus
}
#endif

/**@}*/

#endif /* _OI_STATUSTEXT_H */

