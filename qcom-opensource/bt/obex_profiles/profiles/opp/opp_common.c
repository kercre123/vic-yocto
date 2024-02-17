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

Object Push Profile common functions
*/

#define __OI_MODULE__ OI_MODULE_OPP_SRV

#include "oi_opp_common.h"
#include "oi_obexsrv.h"
#include "oi_memmgr.h"
#include "oi_debug.h"
#include "oi_assert.h"
#include "oi_unicode.h"


/**
 * Check that a file or folder name is a base name. That is, check that the name
 * does not include relative or absolute path.
 *
 * Rejects names that contain "/", backslash, "..", or ":"
 */

static OI_BOOL IsBaseName(OI_OBEX_UNICODE *name)
{
    OI_INT i;
    OI_CHAR16 *str;
    OI_UINT16 len;
    const OI_CHAR16 telecom[] = { 't', 'e', 'l', 'e', 'c', 'o', 'm', '/', 0 };

    if ((name == NULL) || (name->str == NULL)) {
        /* An empty name is always a base name. */
        return TRUE;
    }

    str = name->str;
    len = name->len;

    /* The path "telecom/" is OK so only check for path characters after it if
     * it is at the beginning of the NAME header. */
    if (OI_StrncmpUtf16(str, telecom, OI_ARRAYSIZE(telecom) - 1) == 0) {
        str += OI_ARRAYSIZE(telecom) - 1;
        len -= OI_ARRAYSIZE(telecom) - 1;
    }

    /*
     * Reject . ..
     */
    if (str[0] == (OI_CHAR16) '.') {
        if ((len == 1) || (str[1] == 0)) {
            goto NotBase;
        }
        if (str[1] == ((OI_CHAR16) '.')  && ((len == 2) || (str[2] == 0))) {
            goto NotBase;
        }
    }
    /*
     * Reject : / \
     */
    for (i = 0; i < len; ++i) {
        OI_CHAR16 c = str[i];
        if ((c == (OI_CHAR16)(':')) || (c == (OI_CHAR16)('/')) || (c == (OI_CHAR16)('\\'))) {
            goto NotBase;
        }
    }
    return TRUE;

NotBase:

    OI_DBGPRINT2(("Disallowing folder name %S", name->str));
    return FALSE;
}



OI_STATUS OPPCommon_ParseHeaders(OI_OPP_GENERIC_OBJECT *obj,
                                 OI_OBEX_HEADER_LIST *headers)
{
    OI_STATUS status;
    OI_OBEX_UNICODE *uname;
    OI_OBEX_BYTESEQ *type;
    OI_UINT i;

    for (i = 0; i < headers->count; ++i) {
        switch (headers->list[i].id) {
            case OI_OBEX_HDR_NAME:
                if (obj->name.str != NULL) {
                    /*
                     * Ignore the duplicate name header.
                     */
                    break;
                }
                uname = &(headers->list[i].val.name);
                if (!IsBaseName(uname)) {
                    status = OI_OBEX_NOT_FOUND;
                    goto ObjCleanup;
                }
                obj->name.str = OI_Malloc(uname->len * sizeof(OI_CHAR16));
                if (obj->name.str == NULL) {
                    status = OI_STATUS_OUT_OF_MEMORY;
                    goto ObjCleanup;
                }
                obj->name.len = uname->len;
                OI_MemCopy(obj->name.str, uname->str, uname->len * sizeof(OI_CHAR16));
                break;
            case OI_OBEX_HDR_TYPE:
                if (obj->type.data != NULL) {
                    /*
                     * Ignore the duplicate type header.
                     */
                    break;
                }
                type = &(headers->list[i].val.type);
                obj->type.data = OI_Malloc(type->len);
                if (obj->type.data == NULL) {
                    status = OI_STATUS_OUT_OF_MEMORY;
                    goto ObjCleanup;
                }
                obj->type.len = type->len;
                OI_MemCopy(obj->type.data, type->data, type->len);
                break;
            case OI_OBEX_HDR_LENGTH:
                if (obj->objSize != 0) {
                    /*
                     * Ignore the bogus length header.
                     */
                    break;
                }
                type = &(headers->list[i].val.type);
                obj->objSize = headers->list[i].val.length;
                if (obj->objSize == 0) {
                    status = OI_OBEX_VALUE_NOT_ACCEPTABLE;
                    goto ObjCleanup;
                }
                break;
            case OI_OBEX_HDR_BODY:
                obj->objData = headers->list[i].val.body;
                break;
            case OI_OBEX_HDR_END_OF_BODY:
                obj->objData = headers->list[i].val.body;
                /*
                 * The Sony/Ericcsson T68i does not provide the mandatory length
                 * header so we have to figure out the length of the object from
                 * the body header.
                 */
                if (obj->objSize == 0) {
                    obj->objSize = obj->objData.len;
                }
                break;
            default:
                /* Other headers are ignored. */
                break;
        }
    }

    return OI_OK;

ObjCleanup:

    OI_FreeIf(&obj->name.str);
    OI_FreeIf(&obj->type.data);

    return status;
}

