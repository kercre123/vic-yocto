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

#define __OI_MODULE__ OI_MODULE_SUPPORT

#include "oi_stddefs.h"
#include "oi_handle.h"
#include "oi_debug.h"
#include "oi_dump.h"
#include "oi_utils.h"


typedef struct {
    void *refData;
    const OI_CHAR *handleType;
    OI_UINT32 salt;
} HANDLE_INTERNAL;

static HANDLE_INTERNAL handles[OI_MAX_HANDLES];

/*
 * Initialize salt to some arbitrary value.
 */
static OI_UINT32 salt = 0x4E61436C; /* "NaCl" */

#define INDEX_BITS 9
#define SALT_BITS  (32 - INDEX_BITS)
#define SALT_MASK  ((1 << SALT_BITS) - 1)


/*
 * Handles don't get allocated that often so we can simply search for a free entry.
 */
OI_HANDLE OI_HANDLE_Alloc(const OI_CHAR *handleType,
                          void *refData)
{
    OI_UINT i;
    OI_HANDLE handle;

    if (refData == NULL) {
        OI_SLOG_ERROR(OI_STATUS_INVALID_PARAMETERS, ("OI_HANDLE_Alloc: data cannot be NULL"));
        return NULL;
    }
    for (i = 0; i < OI_ARRAYSIZE(handles); ++i) {
        if (handles[i].salt == 0) {
            handles[i].refData = refData;
            handles[i].handleType = handleType;
            if (++salt == 0) {
                salt = 1;
            }
            handles[i].salt = salt;
            handle = (OI_HANDLE)((i << SALT_BITS) | (salt & SALT_MASK));
            OI_DBGPRINT(("OI_HANDLE_Alloc index:%d salt:%08x handle:%08x", i, salt, handle));
            return handle;
        }
    }
    return NULL;
}


OI_BOOL OI_HANDLE_IsValid(OI_HANDLE handle,
                          const OI_CHAR *handleType)
{
    OI_UINT32 index = (OI_UINT32)(handle) >> SALT_BITS;
    OI_UINT32 salt = (OI_UINT32)(handle) & SALT_MASK;

    if (index < OI_ARRAYSIZE(handles) && ((handles[index].salt & SALT_MASK) == salt)) {
        return handles[index].handleType == handleType;
    } else {
        return FALSE;
    }
}


/**
 * Returns the value referenced by a handle
 */
void* OI_HANDLE_Deref(OI_HANDLE handle)
{
    OI_UINT32 index = (OI_UINT32)(handle) >> SALT_BITS;
    OI_UINT32 salt = (OI_UINT32)(handle) & SALT_MASK;

    OI_DBGPRINT(("OI_HANDLE_Deref index:%d salt:%08x handle:%08x", index, salt, handle));
    if ((index < OI_ARRAYSIZE(handles)) && ((handles[index].salt & SALT_MASK) == salt)) {
        return handles[index].refData;
    } else {
        return NULL;
    }
}


/**
 * Frees a handle
 */
OI_STATUS OI_HANDLE_Free(OI_HANDLE handle)
{
    OI_UINT32 index = (OI_UINT32)(handle) >> SALT_BITS;
    OI_UINT32 salt = (OI_UINT32)(handle) & SALT_MASK;

    OI_DBGPRINT(("OI_HANDLE_Free index:%d salt:%08x handle:%08x", index, salt, handle));
    if ((index < OI_ARRAYSIZE(handles)) && ((handles[index].salt & SALT_MASK) == salt)) {
        handles[index].salt = 0;
        return OI_OK;
    } else {
        return OI_STATUS_INVALID_HANDLE;
    }
}



#ifdef OI_DEBUG
void OI_HANDLE_Dump(void)
{
    OI_UINT i;

    OI_Printf("Handles\n");
    for (i = 0; i < OI_ARRAYSIZE(handles); ++i) {
        if (handles[i].salt) {
            OI_HANDLE h = (OI_HANDLE)((i << SALT_BITS) | (handles[i].salt & SALT_MASK));
            OI_Printf("[%d] type:%s handle:%#x refData:%#x\n", i, handles[i].handleType, h, handles[i].refData);
        }
    }
}
#endif
