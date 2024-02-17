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
This file contains utilities that are commonly found in stdlib libraries,
including functions for manipulating and comparing strings.
*/

#define __OI_MODULE__ OI_MODULE_SUPPORT

#include "oi_std_utils.h"
#include "oi_assert.h"
#include "oi_debug.h"
#include "oi_bt_assigned_nos.h"

/** The zero address.  Used to test for an uninitialized address. */
const OI_BD_ADDR OI_ZeroAddr = { {0,0,0,0,0,0} };

/**
 * A 128-bit UUID initialized to the base UUID.
 */
const OI_UUID128 OI_UUID_BaseUUID128 = OI_UUID_BASE_UUID128;

#define IsDecDigit(c)    (((c) >= '0') && ((c) <= '9'))


const OI_CHAR* OI_ScanUInt(const OI_CHAR *str,
                           OI_UINT32 *val)
{
    const OI_CHAR *p = str;
    OI_INT32 n;

    if (!str || !val) {
        return str;
    }

    while ((*p == ' ') || (*p == '\t')) {
        ++p;
    }
    if (IsDecDigit(*p)) {
        const OI_CHAR *s = OI_ScanInt(p, &n);
        if (s != p) {
            *val = (OI_UINT32)n;
            return s;
        }
    }
    return str;
}


const OI_CHAR* OI_ScanInt(const OI_CHAR *str,
                          OI_INT32 *val)
{
    const OI_CHAR *p = str;
    OI_UINT32 n = 0;
    OI_BOOL neg;

    if (!str || !val) {
        return str;
    }

    while ((*p == ' ') || (*p == '\t')) {
        ++p;
    }
    if (*p == '-') {
        ++p;
        neg = TRUE;
    } else {
        neg = FALSE;
    }
    if (!IsDecDigit(*p)) {
        return str;
    }
    if ((p[0] == '0') && (p[1] == 'x')) {
        p += 2;
        while (*p) {
            OI_CHAR d = *p;
            if (IsDecDigit(d)) {
                n = (n * 16) + (d - '0');
            } else if ((d >= 'a') && (d <= 'f')) {
                n = (n * 16) + (10 + d - 'a');
            } else if ((d >= 'A') && (d <= 'F')) {
                n = (n * 16) + (10 + d - 'A');
            } else {
                break;
            }
            ++p;
        }
    } else {
        while (*p) {
            const OI_CHAR d = *p;
            if (!IsDecDigit(d)) {
                break;
            }
            n = (n * 10) + (d - '0');
            ++p;
        }
    }
    if (neg) {
        *val = -(OI_INT32)(n);
    } else {
        *val = (OI_INT32)n;
    }
    return p;
}


const OI_CHAR* OI_ScanStr(const OI_CHAR *str,
                          OI_CHAR *outStr,
                          OI_UINT16 len)
{
    const OI_CHAR *p = str;
    OI_UINT16 pos = 0;

    if (!str || !len) {
        return str;
    }

    while ((*p == ' ') || (*p == '\t')) {
        ++p;
    }
    while ((++pos < len) && *p) {
        *outStr++ = *p++;
        if ((*p == ' ') || (*p == '\t')) {
            break;
        }
    }
    *outStr = 0;
    if (1 == pos) {
        return str;
    } else {
        return p;
    }
}


const OI_CHAR* OI_ScanAlt(const OI_CHAR *str,
                          const OI_CHAR *alts,
                          OI_INT *index)
{
    const OI_CHAR *p = str;

    if (!str || !alts || !index) {
        return str;
    }

    *index = 0;

    while ((*p == ' ') || (*p == '\t')) {
        ++p;
    }
    while (TRUE) {
        OI_UINT len = 0;
        while (alts[len] && (alts[len] != '|')) {
            ++len;
        }
        if (OI_StrncmpInsensitive(p, alts, len) == 0) {
            return p + len;
        }
        if (!alts[len]) {
            break;
        }
        alts += len + 1;
        *index += 1;
    }
    *index = -1;
    return str;
}

const OI_CHAR* OI_ScanAltExact(const OI_CHAR *str,
                               const OI_CHAR *alts,
                               OI_INT *index)
{
    const OI_CHAR *p = str;
    OI_UINT        strLen;

    if (!str || !alts || !index) {
        return str;
    }

    *index = 0;

    strLen = OI_StrLen(str);
    while ((*p == ' ') || (*p == '\t')) {
        ++p;
    }
    while (TRUE) {
        OI_UINT len = 0;
        while (alts[len] && (alts[len] != '|')) {
            ++len;
        }
        if ((strLen == len) && (OI_Strncmp(p, alts, len) == 0)) {
            return p + len;
        }
        if (!alts[len]) {
            break;
        }
        alts += len + 1;
        *index += 1;
    }
    *index = -1;
    return str;
}



/** Convert a string to an integer.
 *
 * @param str  The string to parse.
 *
 * @return The integer value of the string or 0 if the string could not be parsed.
 */
OI_INT OI_atoi(const OI_CHAR *str)
{
    OI_INT32 i;

    if (str == OI_ScanInt(str, &i)) {
        return 0;
    }
    else {
        return (OI_INT)i;
    }
}


OI_UINT OI_Strlcpy(OI_CHAR *pDest, OI_CHAR const *pStr, OI_UINT len)
{
    OI_CHAR *p = pDest;

    if (len != 0) {
        while (*pStr && --len) {
            *p++ = *pStr++;
        }
        *p = 0;
    }
    while (*pStr++) {
        ++p;
    }
    return (OI_UINT)(p - pDest);
}

OI_UINT OI_Strlcat(OI_CHAR *pDest, OI_CHAR const *pStr, OI_UINT len)
{
    OI_CHAR *p = pDest;

    if (len != 0) {
        while (*p && --len) {
            ++p;
        }
        if (len != 0) {
            while (*pStr && --len) {
                *p++ = *pStr++;
            }
            *p = 0;
        }
    }
    while (*pStr++) {
        ++p;
    }
    return (OI_UINT)(p - pDest);
}

#ifndef OI_USE_NATIVE_MEMCPY

OI_UINT OI_StrLen(OI_CHAR const *pStr)
{
    OI_UINT len = 0;

    while (pStr[len] != 0) {
        ++len;
    }
    return(len);
}


OI_INT OI_Strcmp(OI_CHAR const *p1,
                 OI_CHAR const *p2)
{
    OI_ASSERT(p1 != NULL);
    OI_ASSERT(p2 != NULL);

    for (;;) {
        if (*p1 < *p2) return -1;
        if (*p1 > *p2) return 1;
        if (*p1 == '\0') return 0;
        p1++;
        p2++;
    }
}

OI_INT OI_Strncmp(OI_CHAR const *p1,
                  OI_CHAR const *p2,
                  OI_UINT32      len)
{
    OI_ASSERT(p1 != NULL);
    OI_ASSERT(p2 != NULL);
    while (len--) {
        if (*p1 < *p2) return -1;
        if (*p1 > *p2) return 1;
        if (*p1 == '\0') return 0;
        p1++;
        p2++;
    }
    return 0;
}
#endif /*  ifndef OI_USE_NATIVE_MEMCPY */


OI_INT OI_StrcmpInsensitive(OI_CHAR const *p1,
                            OI_CHAR const *p2)
{
    OI_ASSERT(p1 != NULL);
    OI_ASSERT(p2 != NULL);

    for (;;) {
        OI_CHAR uc1 = OI_toupper(*p1);
        OI_CHAR uc2 = OI_toupper(*p2);
        if (uc1 < uc2) return -1;
        if (uc1 > uc2) return 1;
        if (uc1 == '\0') return 0;
        p1++;
        p2++;
    }
}


OI_INT OI_StrncmpInsensitive(OI_CHAR const *p1,
                             OI_CHAR const *p2,
                             OI_UINT        len)
{
    OI_ASSERT(p1 != NULL);
    OI_ASSERT(p2 != NULL);

    while (len--) {
        OI_CHAR uc1 = OI_toupper(*p1);
        OI_CHAR uc2 = OI_toupper(*p2);
        if (uc1 < uc2) return -1;
        if (uc1 > uc2) return 1;
        if (uc1 == '\0') return 0;
        p1++;
        p2++;
    }
    return 0;
}


void OI_StrToUpper(OI_CHAR *str) {
    OI_INT i;

    for (i = 0; i < (OI_INT)OI_StrLen(str); i++) {
        str[i] = OI_toupper(str[i]);
    }
}


/*****************************************************************************/
/* Parses BD_ADDR from a string of 12 hex digits, optionally separated by colons:
   "xxxxxxxxxxxx"
   OR
   "xx:xx:xx:xx:xx:xx"
   */

#define OI_isdigit(c)   ( ((c) >= '0') && ((c) <= '9') ? TRUE : FALSE)


static OI_BOOL xCharToBin(OI_BYTE  c,
                          OI_UINT *pBin)
{

    if ((c >= '0') && (c <= '9')) {
        *pBin = c - '0';
        return TRUE;
    }
    else if ((c >= 'a') && (c <= 'f')) {
        *pBin = c - 'a' + 10;
        return TRUE;
    }
    else if ((c >= 'A') && (c <= 'F')) {
        *pBin = c - 'A' + 10;
        return TRUE;
    }
    return FALSE;
}


static OI_BOOL hex2_2_bin(const OI_BYTE *hexStr,
                          OI_UINT8 *pBin)
{
    OI_UINT loNibble;
    OI_UINT hiNibble;

    if (xCharToBin(hexStr[0], &hiNibble) && xCharToBin(hexStr[1], &loNibble)) {
        *pBin = (OI_UINT8)((hiNibble << 4) + loNibble);
        return TRUE;
    }
    return FALSE;
}


const OI_CHAR* OI_ScanBdAddr(const OI_CHAR *str,
                             OI_BD_ADDR *addr)
{
    const OI_CHAR *p = str;
    OI_INT i;

    if (!addr || !str) {
        return str;
    }
    /* strip leading white space */
    while ((*p == ' ') || (*p == '\t')) {
        ++p;
    }

    for (i = 0; i < OI_BD_ADDR_BYTE_SIZE; ++i) {
        if (!hex2_2_bin((const OI_BYTE *)p, &addr->addr[i])) {
            return str;
        }
        p += 2;
        if (':' == *p) {
            ++p;
        }
    }
    return p;
}


OI_BOOL OI_ParseBdAddr(const OI_CHAR *str,
                       OI_BD_ADDR *addr)

{
    return OI_ScanBdAddr(str, addr) != str;
}

void OI_RemoveNewlines(OI_CHAR *str)
{
    OI_UINT i = 0;

    OI_ASSERT(str != NULL);
    if (OI_StrLen(str) < 1)
        return;

    /* Replace newline on the end with null */
    if ( (str[OI_StrLen(str) - 1] == '\n') ||
         (str[OI_StrLen(str) - 1] == '\r') )
    {
        str[OI_StrLen(str) - 1] = '\0';
    }

    /* Replace newlines in the middle with space */
    for (i = 0; i < (OI_StrLen(str)); i++) {
        if ( (str[i] == '\n') ||
             (str[i] == '\r') )
        {
            str[i] = ' ';
        }
    }
}


/**
 * Terminate a UTF8 string.
 *
 *  This function terminates or truncates a UTF8 string at a maximum length with a
 *  null-terminator.  The function ensures termination/truncation does not occur
 *  in the middle of a multi-byte sequence.  As such, the final length may be
 *  shorter than than the requested maxLen.
 *
 *  @note - null-termination occurs 'in place', termination alters callers string.
 *
 * @param pStr      pointer to UTF8 string
 *
 * @param maxLen    requested maximum length of string after truncation
 *
 * @return          returns actual length of string after truncation, i.e. strlen(pStr)
 *
 */
OI_UINT OI_UTF8_Terminate(OI_UTF8 *pStr, OI_UINT maxLen)
{
    OI_UINT curLen;
    OI_UINT i;

    if (NULL == pStr) {
        OI_SLOG_ERROR(OI_STATUS_INVALID_PARAMETERS, ("pStr is NULL"));
        return 0;
    }
    /*
     *  First check if string is already appropriately null-terminated.  Don't use
     *  strlen() because the string may not be null terminated.
     */
    curLen = 0;
    while ((pStr[curLen] != 0) && (curLen <= maxLen)) {
        ++curLen;
    }
    if (curLen <= maxLen) {
        return curLen;
    }

    /* string is not null-terminated within maxLen */

    i = maxLen;             /* index of byte we're going to stomp on */

    if (pStr[i] > 0x7F) {
        /*
         *  Last byte is non 7bit ASCII, so it part of a multi-byte sequence.
         *  Work backwards until we find start of sequence, indicated by byte
         *  with the 2 high-order bits set.
         */

        while ((i > 0) && (pStr[i] < 0xC0)) {
            i--;
        }
    }
    pStr[i] = 0;

    OI_UINT len = OI_StrLen((OI_CHAR*)pStr);
    OI_ASSERT(len == (i));
    return (i);
}
