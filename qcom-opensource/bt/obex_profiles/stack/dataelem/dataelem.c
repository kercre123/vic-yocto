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
This file provides helper functions for the managing of data elements.
*/

#define __OI_MODULE__ OI_MODULE_DATAELEM

#include "oi_debug.h"
#include "oi_bt_assigned_nos.h"
#include "oi_memmgr.h"
#include "oi_assert.h"
#include "oi_dataelem.h"

/*****************************************************************************
  comparison functions and macros for different size UUIDs
 ******************************************************************************/

/** macro for comparing two 128-bit UUIDs */
#define LongUUID_eq_LongUUID(uA, uB)   (OI_MemCmp((uA), (uB), sizeof(OI_UUID128)) == 0)

/** function for comparing a 32-bit UUID to a 128-bit UUID */
OI_BOOL OI_ShortUUID_eq_LongUUID(OI_UUID32 shortUUID,
                                 const OI_UUID128 *longUUID)
{
    OI_UUID128 TestUUID = OI_UUID_BASE_UUID128;

    OI_ASSERT(longUUID != NULL);

    /*
     * TestUUID was initialized to the constant 128-bit base UUID, so all that is needed is to set
     * the 32-bit variable component and compare the resultant 128-bit UUID.
     */
    TestUUID.ms32bits = shortUUID;
    return LongUUID_eq_LongUUID(longUUID, &TestUUID);
}

/** function for comparing two UUIDs */
OI_BOOL OI_DATAELEM_SameUUID(const OI_DATAELEM *A,
                             const OI_DATAELEM *B)
{
    if ((A->ElemType != OI_DATAELEM_UUID) || (B->ElemType != OI_DATAELEM_UUID)) {
        return FALSE;
    }

    switch (A->Size) {
        case sizeof(OI_UUID16):
        case sizeof(OI_UUID32):
            if (B->Size == sizeof(OI_UUID128)) {
                return OI_ShortUUID_eq_LongUUID(A->Value.ShortUUID, B->Value.LongUUID);
            } else {
                return A->Value.ShortUUID == B->Value.ShortUUID;
            }
        case sizeof(OI_UUID128):
            if (B->Size == sizeof(OI_UUID128)) {
                return LongUUID_eq_LongUUID(A->Value.LongUUID, B->Value.LongUUID);
            } else {
                return OI_ShortUUID_eq_LongUUID(B->Value.ShortUUID, A->Value.LongUUID);
            }
    }
    return FALSE;
}


OI_STATUS OI_DATAELEM_ConvertToUUID128(OI_UUID128 *uuid128, const OI_DATAELEM *U)
{

    if (U->ElemType != OI_DATAELEM_UUID) {
        return OI_FAIL;
    }
    if (U->Size == sizeof(OI_UUID128)) {
        *uuid128 = *U->Value.LongUUID;
    } else {
        OI_UUID128 baseUUID = OI_UUID_BASE_UUID128;
        baseUUID.ms32bits = U->Value.ShortUUID;
        *uuid128 = baseUUID;
    }
    return OI_OK;
}


OI_UUID32 OI_DATAELEM_ConvertToUUID32(const OI_DATAELEM *U)
{
    OI_UUID128 TestUUID = OI_UUID_BASE_UUID128;

    if (U->ElemType == OI_DATAELEM_UUID) {
        switch (U->Size) {
            case sizeof(OI_UUID16):
            case sizeof(OI_UUID32):
                return U->Value.ShortUUID;
            case sizeof(OI_UUID128):
                TestUUID.ms32bits = U->Value.LongUUID->ms32bits;
                if (LongUUID_eq_LongUUID(U->Value.LongUUID, &TestUUID)) {
                    return (OI_UUID32) U->Value.LongUUID->ms32bits;
                }
        }
    }
    /*
     * no 32-bit representation of this UUID
     */
    return 0;
}

OI_CHAR* OI_UUIDDataelemText(const OI_DATAELEM *pElem)
{
    OI_ASSERT(OI_DATAELEM_UUID == pElem->ElemType);

    if (sizeof(OI_UUID128) == pElem->Size) {
        return OI_UUID128Text(pElem->Value.LongUUID);
    } else {
        return OI_UUIDText(pElem->Value.ShortUUID);
    }
}

