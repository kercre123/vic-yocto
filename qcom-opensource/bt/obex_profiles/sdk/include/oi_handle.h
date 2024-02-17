#ifndef _OI_HANDLE_H
#define _OI_HANDLE_H

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
 * @file
 * @internal
 *
 * This module provide a simple light-weight generic mechansim for managing handles to internal
 * data structures. In normal usage handles are not reused.
 */

#include "oi_stddefs.h"
#include "oi_status.h"

#ifdef __cplusplus
extern "C" {
#endif

/* Set the limit on the number of simultaneously maintained handles */
#define OI_MAX_HANDLES    128


/**
 * Allocates and initializes a handle of a specific type.
 *
 * @param handleType    A unique handle type
 *
 * @param refData       Pointer to data referenced by the handle. This cannot be NULL.
 *
 * @return A new handle or NULL if handle could not be allocated.
 */
OI_HANDLE OI_HANDLE_Alloc(const OI_CHAR *handleType,
                          void *refData);


/**
 * Checks if a handle is valid for a specific handle type.
 *
 * @param handle      The handle being checked
 *
 * @param handleType  The handle type. Note this function does not do string comparison this must be
 *                    exactly the same const OI_CHAR pointer passed to OI_HANDLE_Alloc. .
 */
OI_BOOL OI_HANDLE_IsValid(OI_HANDLE handle,
                          const OI_CHAR *handleType);


/**
 * Checks if a handle is stale (not longer valid)
 *
 * @param handle      The handle being checked
 */
#define OI_HANDLE_IsStale(handle) (OI_HANDLE_Deref(handle) == NULL)


/**
 * Returns the value referenced by a handle or NULL if the handle is invalid.
 *
 * @param handle   The handle to dereference
 */
void* OI_HANDLE_Deref(OI_HANDLE handle);


/**
 * Frees a handle
 *
 * @param handle   The handle to free.
 *
 * @return OI_STATUS_INVALID_HANDLE if the handle was not valid.
 */
OI_STATUS OI_HANDLE_Free(OI_HANDLE handle);


#ifdef __cplusplus
}
#endif

#endif   /* _OI_HANDLE_H */
