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
This file implements the functionality defined in oi_varstring.h.
*/

#define __OI_MODULE__ OI_MODULE_SUPPORT

#include "oi_assert.h"
#include "oi_varstring.h"
#include "oi_text.h"
#include "oi_utils.h"
#include "oi_statustext.h"
#include "oi_dataelem_text.h"
#include "oi_memmgr.h"
#include "oi_xml.h"
#include "oi_bytestream.h"
#include "oi_unicodestream.h"
#include "oi_unicode.h"
#include "oi_obextext.h"

#include "../unicode/ConvertUTF.h"

#define VSTRING_INCREMENT_SIZE    32


#define MAX_HEX_LEN            16*10

static const OI_CHAR digits[] = "0123456789ABCDEF";

static OI_STATUS UnsignedCat(OI_VARSTRING *VStr,
                             OI_UINT32 val);


/**
 * Allocate an initialize an empty managed VarString
 */
OI_STATUS OI_VStrAlloc(OI_VARSTRING *VStr,
                       OI_UINT       Size)
{
    VStr->Managed = TRUE;
    VStr->Len = 0;
    VStr->Buffer = OI_Malloc(Size);
    VStr->Overflow = FALSE;
    if (VStr->Buffer == NULL) {
        VStr->MaxLen = 0;
        return OI_STATUS_OUT_OF_MEMORY;
    } else {
        VStr->MaxLen = Size;
        return OI_OK;
    }
}


/**
 * Free the string buffer for a managed varstring.
 */
void OI_VStrFree(OI_VARSTRING *VStr)
{
    OI_ASSERT(VStr->Managed);

    OI_Free(VStr->Buffer);
    VStr->Buffer = NULL;
    VStr->MaxLen = 0;
}


static OI_BOOL StrRealloc(OI_VARSTRING *VStr)
{
    OI_CHAR* buffer;
    OI_UINT16 i;

    OI_ASSERT(VStr->Buffer != NULL);
    OI_ASSERT(VStr->Managed);

    buffer = OI_Malloc(VStr->MaxLen + VSTRING_INCREMENT_SIZE);
    if (buffer == NULL) {
        return FALSE;
    }
    for (i = 0; i < VStr->Len; ++i) {
        buffer[i] = VStr->Buffer[i];
    }
    OI_Free(VStr->Buffer);
    VStr->Buffer = buffer;
    VStr->MaxLen += VSTRING_INCREMENT_SIZE;
    return TRUE;
}


OI_STATUS OI_VStrnCat(OI_VARSTRING  *VStr,
                      const OI_CHAR *Str,
                      OI_UINT        len)
{
    if (Str && len) {
        while (len-- && *Str) {
            if (VStr->Len == (VStr->MaxLen - 1)) {
                if (!VStr->Managed) {
                    VStr->Overflow = TRUE;
                    return OI_OK; /* String is truncated */
                }
                if (VStr->Overflow == FALSE) {
                    if (!StrRealloc(VStr)) {
                        VStr->Overflow = TRUE;
                        return OI_STATUS_OUT_OF_MEMORY;
                    }
                }
            }
            VStr->Buffer[VStr->Len++] = *Str++;
        }
    }
    return OI_OK;
}


OI_STATUS OI_VStrCat(OI_VARSTRING *VStr,
                     const OI_CHAR *Str)
{
    if (Str) {
        while (*Str ) {
            if (VStr->Len == (VStr->MaxLen - 1)) {
                if (!VStr->Managed) {
                    VStr->Overflow = TRUE;
                    return OI_OK; /* String is truncated */
                }
                if (VStr->Overflow == FALSE) {
                    if (!StrRealloc(VStr)) {
                        VStr->Overflow = TRUE;
                        return OI_STATUS_OUT_OF_MEMORY;
                    }
                }
            }
            VStr->Buffer[VStr->Len++] = *Str++;
        }
    }
    return OI_OK;
}


OI_STATUS OI_VStrCatAT(OI_VARSTRING *VStr,
                       const OI_CHAR *Str)
{
    if (Str) {
        while (*Str) {
            if (VStr->Len == (VStr->MaxLen - 1)) {
                if (!VStr->Managed) {
                    VStr->Overflow = TRUE;
                    return OI_OK; /* String is truncated */
                }
                if (VStr->Overflow == FALSE) {
                    if (!StrRealloc(VStr)) {
                        VStr->Overflow = TRUE;
                        return OI_STATUS_OUT_OF_MEMORY;
                    }
                }
            }
            if (*Str == '\r') {
                OI_VStrCat(VStr, "<cr>");
            } else if (*Str == '\n') {
                OI_VStrCat(VStr, "<lf>");
            } else {
                VStr->Buffer[VStr->Len++] = *Str;
            }
            ++Str;
        }
    }
    return OI_OK;
}


OI_STATUS OI_VStrnCatAT(OI_VARSTRING  *VStr,
                        const OI_CHAR *Str,
                        OI_UINT        len)
{
    if (Str && len) {
        while (len--) {
            if (VStr->Len == (VStr->MaxLen - 1)) {
                if (!VStr->Managed) {
                    VStr->Overflow = TRUE;
                    return OI_OK; /* String is truncated */
                }
                if (VStr->Overflow == FALSE) {
                    if (!StrRealloc(VStr)) {
                        VStr->Overflow = TRUE;
                        return OI_STATUS_OUT_OF_MEMORY;
                    }
                }
            }
            if (*Str == '\r') {
                OI_VStrCat(VStr, "<cr>");
            } else if (*Str == '\n') {
                OI_VStrCat(VStr, "<lf>");
            } else if (*Str == '\0') {
                OI_VStrCat(VStr, "<null>");
			} else {
                VStr->Buffer[VStr->Len++] = *Str;
            }
            ++Str;
        }
    }
    return OI_OK;
}


OI_STATUS OI_VSpaceCat(OI_VARSTRING *VStr,
                       OI_UINT       Count)
{
    while (Count--) {
        if (VStr->Len == (VStr->MaxLen - 1)) {
            if (!VStr->Managed) {
                VStr->Overflow = TRUE;
                return OI_OK; /* String is truncated */
            }
            if (VStr->Overflow == FALSE) {
                if (!StrRealloc(VStr)) {
                    VStr->Overflow = TRUE;
                    return OI_STATUS_OUT_OF_MEMORY;
                }
            }
        }
        VStr->Buffer[VStr->Len++] = ' ';
    }
    return OI_OK;
}


/*
 * Hex integer text
 *
 * Size is the number of digits - writes leading zeroes
 */

OI_STATUS OI_VHexCat(OI_VARSTRING *VStr,
                     OI_UINT32 Val,
                     OI_UINT Size)
{
    OI_CHAR hex[9];
    OI_CHAR *t;

    /*
     * Override invalid size specification
     */
    if ((Size < 1) || (Size > (2 * sizeof(OI_UINT32)))) {
        Size = 2 * sizeof(OI_UINT32);
    }

    t = &hex[sizeof(hex) - 1];
    *t =0;

    do {
        --t;
        *t = digits[(Val & 0xF)];
        Val >>= 4;
    } while (--Size);
    return OI_VStrCat(VStr, t);
}


/*
 * Decimal integer text
 */

OI_STATUS OI_VDecCat(OI_VARSTRING *VStr,
                     OI_INT32 Val)
{
    OI_CHAR dec[12];
    OI_CHAR *t;

    if (Val == OI_INT32_MIN) {
        return OI_VStrCat(VStr, "-2147483648");
    }
    if (Val < 0) {
        OI_VStrCat(VStr, "-");
        Val = -Val;
    }
    t = &dec[sizeof(dec) - 1];
    *t = 0;

    do {
        OI_UINT32 div = Val / 10;
        OI_UINT32 rem = Val - div * 10;
        --t;
        *t = digits[rem];
        Val = div;
    } while (Val != 0);
    return OI_VStrCat(VStr, t);
}

/*
 * Time text
 */

static OI_STATUS TimeCat(OI_VARSTRING *VStr,
                         OI_TIME *time)
{
    OI_TIME now;
    OI_CHAR *decimal;
    OI_UINT32 secs;
    OI_UINT32 msecs;

    if (NULL == time) {
        OI_Time_Now(&now);
        time = &now;
    }

    secs = time->seconds & 0x1FFF;
    msecs = time->mseconds;
    if (msecs < 10) {
        decimal = ".00";
    } else if (msecs < 100) {
        decimal = ".0";
    } else {
        decimal = ".";
    }

    UnsignedCat(VStr, secs);
    OI_VStrCat(VStr, decimal);
    return UnsignedCat(VStr, msecs);
}

/*
 * Unsigned decimal integer text
 */

static OI_STATUS UnsignedCat(OI_VARSTRING *VStr,
                             OI_UINT32 val)
{
    OI_CHAR dec[11];
    OI_CHAR *t;

    t = &dec[sizeof(dec)-1];
    *t = 0;

    do {
        OI_UINT32 div = val / 10;
        OI_UINT32 rem = val - div * 10;
        --t;
        *t = digits[rem];
        val = div;
    } while (val != 0);
    return OI_VStrCat(VStr, t);
}


/**
 * conversion of wide characters to ASCII
 */
static OI_STATUS StrnCat16(OI_VARSTRING    *VStr,
                           const OI_CHAR16 *Str,
                           OI_UINT          len)
{
    OI_UTF8 *buf;
    OI_UTF8 *bufEnd;
    const OI_UTF16 *srcEnd;
    ConversionResult res = conversionOK;

    if (Str) {
        OI_UINT stringLength = OI_StrLenUtf16(Str);

        srcEnd = &Str[OI_MIN(stringLength, len)];
        do {
            buf = (OI_UTF8*)&VStr->Buffer[VStr->Len];
            bufEnd = (OI_UTF8*)&VStr->Buffer[VStr->MaxLen - 1];

            res = ConvertUTF16toUTF8(&Str, srcEnd, &buf, bufEnd, strictConversion);
            VStr->Len = (VStr->MaxLen - 1) - (bufEnd - buf);

            if (res == targetExhausted) {
                if (!VStr->Managed) {
                    VStr->Overflow = TRUE;
                    return OI_OK; /* String is truncated */
                }
                if (VStr->Overflow == FALSE) {
                    if (!StrRealloc(VStr)) {
                        VStr->Overflow = TRUE;
                        return OI_STATUS_OUT_OF_MEMORY;
                    }
                }
            }
        } while (res == targetExhausted);

        if (res != conversionOK) {
            return OI_FAIL;
        }
    }

    return OI_OK;
}


#define StrCat16(_vstr, _str) StrnCat16((_vstr), (_str), OI_UINT16_MAX)


static OI_STATUS PStrcat(OI_VARSTRING *VStr,
                         OI_PSTR *str,
                         OI_UNICODE_ENCODING encoding)
{
    OI_UNICODE_STREAM bs;
    OI_UINT32 ch = 0;
    OI_STATUS result;

    if (encoding > OI_UNICODE_UTF16_BE) {
        return OI_STRING_FORMAT_ERROR;
    }
    if (!str->sz) {
        return OI_OK;
    }

    UnicodeStream_Init(bs, str->p, str->sz, encoding);
    ByteStream_Open(bs, BYTESTREAM_READ);
    while (ByteStream_NumReadBytesAvail(bs)) {
        UnicodeStream_GetChar(bs, ch);
        if (ch < 0xff) {
            if (VStr->Len == (VStr->MaxLen - 1)) {
                if (!VStr->Managed) {
                    VStr->Overflow = TRUE;
                    return OI_OK; /* String is truncated */
                }
                if (VStr->Overflow == FALSE) {
                    if (!StrRealloc(VStr)) {
                        VStr->Overflow = TRUE;
                        return OI_STATUS_OUT_OF_MEMORY;
                    }
                }
            }
            VStr->Buffer[VStr->Len++] = (OI_CHAR) (ch & 0xFF);
        } else {
            if (VStr->Len == (VStr->MaxLen - 7)) {
                if (!VStr->Managed) {
                    VStr->Overflow = TRUE;
                    return OI_OK; /* String is truncated */
                }
                if (VStr->Overflow == FALSE) {
                    if (!StrRealloc(VStr)) {
                        VStr->Overflow = TRUE;
                        return OI_STATUS_OUT_OF_MEMORY;
                    }
                }
            }
            VStr->Buffer[VStr->Len++] = '\\';
            result = OI_VHexCat(VStr, ch, 6);
            if (!OI_SUCCESS(result)) {
                return result;
            }
        }
    }
    return OI_OK;
}

static OI_INT UnsignedChars(OI_UINT32 val)
{
    OI_INT digits = 0;

    do {
        val /= 10;
        ++digits;
    } while (val != 0);
    return digits;
}


/**
 * Defines for ANSI color strings. These are used by OI_Printf to implement the %[ format option.
 */
#define ANSI_COLOR_BLACK       "\033[0;30m"
#define ANSI_COLOR_RED         "\033[0;31m"
#define ANSI_COLOR_GREEN       "\033[0;32m"
#define ANSI_COLOR_YELLOW      "\033[0;33m"
#define ANSI_COLOR_BLUE        "\033[0;34m"
#define ANSI_COLOR_MAGENTA     "\033[0;35m"
#define ANSI_COLOR_CYAN        "\033[0;36m"
#define ANSI_COLOR_LT_GRAY     "\033[0;37m"
#define ANSI_COLOR_DK_GRAY     "\033[1;30m"
#define ANSI_COLOR_BR_RED      "\033[1;31m"
#define ANSI_COLOR_BR_GREEN    "\033[1;32m"
#define ANSI_COLOR_BR_YELLOW   "\033[1;33m"
#define ANSI_COLOR_BR_BLUE     "\033[1;34m"
#define ANSI_COLOR_BR_MAGENTA  "\033[1;35m"
#define ANSI_COLOR_BR_CYAN     "\033[1;36m"
#define ANSI_COLOR_WHITE       "\033[1;37m"
#define ANSI_COLOR_DEFAULT     "\033[0;39;49m"



/*
 * Full version of VFormatStr
 */
OI_STATUS OI_VFormatStr(OI_VARSTRING *VStr,
                        const OI_CHAR* format,
                        va_list argp)
{
    const OI_CHAR *p;
    OI_INT16 size, precision;
    OI_BOOL isLong;
    OI_BOOL hasPrecision;
    OI_BOOL moreFmt;
    OI_BOOL hasVarLen;
    OI_BOOL zeroPad;
    OI_BOOL prefixSign;

    for (p = format; *p != 0; p++) {

        if (VStr->Len == (VStr->MaxLen - 1)) {
            if (!VStr->Managed) {
                VStr->Overflow = TRUE;
                return OI_OK; /* String is truncated */
            }
            if (VStr->Overflow == FALSE) {
                if (!StrRealloc(VStr)) {
                    VStr->Overflow = TRUE;
                    return OI_STATUS_OUT_OF_MEMORY;
                }
            }
        }

        if (*p != '%') {
            VStr->Buffer[VStr->Len++] = *p;
            continue;
        }

        moreFmt = TRUE;
        size = 0;
        precision = 0;
        hasPrecision = FALSE;
        hasVarLen = FALSE;
        isLong = FALSE;
        zeroPad = FALSE;
        prefixSign = FALSE;

        while (moreFmt) {
            switch (*++p) {
                case '+':
                    prefixSign = TRUE;
                    break;
                case 'l':
                    if ((p[1] != 'd') && (p[1] != 'x') && (p[1] != 's') && (p[1] != 'c') && (p[1] != 'u') && (p[1] != 'b')) {
                        return OI_STRING_FORMAT_ERROR;
                    }
                    isLong = TRUE;
                    break;
                case '#':
                    OI_VStrCat(VStr, "0x");
                    break;
                case '.':
                    hasPrecision = TRUE;
                    break;
                case '0':
                    if (!hasPrecision && (0 == size)) {
                        zeroPad = TRUE;
                    }
                    /* fall through */
                case '1':
                case '2':
                case '3':
                case '4':
                case '5':
                case '6':
                case '7':
                case '8':
                case '9':
                    if (hasPrecision)
                        precision = precision * 10 + (*p - '0');
                    else
                        size = size * 10 + (*p - '0');
                    break;
                case 'b': /* binary */
                    {
                        OI_UINT32 i;
                        OI_UINT32 b = 0x80000000;
                        if (isLong) {
                            i  = (OI_UINT32) va_arg(argp, OI_UINT32);
                        } else {
                            i  = (OI_UINT32) va_arg(argp, OI_UINT);
                        }
                        if (hasVarLen) {
                            hasVarLen = FALSE;
                            size = (OI_UINT16)va_arg(argp, OI_UINT);
                            if (size > 32) {
                                return OI_STRING_FORMAT_ERROR;
                            }
                        } else if (size == 0) {
                            size = isLong ? 8 * sizeof(OI_UINT32) : 8 * sizeof(OI_UINT);

                        }
                        if (size > 0) {
                            b >>= 32 - size;
                            while (b) {
                                OI_VStrCat(VStr, (b & i) ? "1" : "0");
                                b >>= 1;
                            }
                        }
                        moreFmt = FALSE;
                    }
                    break;
                case 'B': /* boolean */
                    if (va_arg(argp, OI_UINT)) {
                        OI_VStrCat(VStr, "TRUE");
                    } else {
                        OI_VStrCat(VStr, "FALSE");
                    }
                    moreFmt = FALSE;
                    break;
                case 'c':
                    VStr->Buffer[VStr->Len++] = (OI_CHAR) va_arg(argp, OI_UINT);
                    moreFmt = FALSE;
                    break;
                case 'd':
                    {
                        OI_INT32 i;
                        OI_UINT32 ui;
                        OI_CHAR *sign = NULL;

                        if (isLong) {
                            i  = (OI_INT32) va_arg(argp, OI_INT32);
                        } else {
                            i  = (OI_INT32) va_arg(argp, OI_INT);
                        }
                        if (i < 0) {
                            i = -i;
                            sign = "-";
                            --size;
                        } else if (prefixSign) {
                            sign = "+";
                            --size;
                        }
                        ui = (OI_UINT32)i;
                        do {
                            ui /= 10;
                            --size;
                        } while ((size > 0) && (ui != 0));

                        if (size > 0) {
                            if (zeroPad) {
                                OI_VStrCat(VStr, sign);
                                OI_VStrnCat(VStr, "000000000000000", size);
                            } else {
                                OI_VSpaceCat(VStr, size);
                                OI_VStrCat(VStr, sign);
                            }
                        } else {
                            OI_VStrCat(VStr, sign);
                        }
                        UnsignedCat(VStr, (OI_UINT32)i);
                    }
                    moreFmt = FALSE;
                    break;
                case 'u':
                    {
                        OI_UINT32 i;
                        if (isLong) {
                            i  = (OI_UINT32) va_arg(argp, OI_UINT32);
                        } else {
                            i  = (OI_UINT32) va_arg(argp, OI_UINT);
                        }
                        if (size > 0) {
                            OI_INT pad = size - UnsignedChars(i);
                            if (pad > 0) {
                                OI_VSpaceCat(VStr, pad);
                            }
                        }
                        UnsignedCat(VStr, i);
                    }
                    moreFmt = FALSE;
                    break;
                case '?':   /* The next value will be followed by a length */
                    hasVarLen = TRUE;
                    break;
                case 's':
                    if (!isLong) {
                        if (hasVarLen || (0 < size) || (0 < precision)) {
                            OI_CHAR *str = va_arg(argp, OI_CHAR*);
                            OI_UINT16 minLen, maxLen;
                            OI_UINT16 len;

                            if (hasVarLen) {
                                minLen = 0;
                                maxLen = (OI_UINT16)va_arg(argp, OI_UINT);
                            }
                            else {
                                minLen = size;
                                maxLen = precision;
                            }

                            OI_ASSERT((minLen <= maxLen) || (0 == maxLen));

                            if (!str) {
                                str = "(null)";
                            }
                            if (maxLen) {
                                OI_VStrnCat(VStr, str, maxLen);
                            }
                            else if (hasVarLen) {
                                OI_VStrCat(VStr, "(empty)");
                            } else {
                                OI_VStrCat(VStr, str);
                            }

                            len = OI_StrLen(str);
                            if (len < minLen) {
                                OI_VSpaceCat(VStr, minLen-len);
                            }
                        } else {
                            OI_CHAR *str = va_arg(argp, OI_CHAR*);
                            if (!str) {
                                OI_VStrCat(VStr, "(null)");
                            } else {
                                OI_VStrCat(VStr, str);
                            }
                        }
                        moreFmt = FALSE;
                        hasVarLen = FALSE;
                        break;
                    }
                    /* falling through */
                case 'S':
                    if (hasVarLen) {
                        OI_CHAR16* str = va_arg(argp, OI_CHAR16*);
                        OI_UINT16 len = (OI_UINT16)va_arg(argp, OI_UINT);
                        if (!str) {
                            OI_VStrCat(VStr, "(null)");
                        } else if (len == 0) {
                            OI_VStrCat(VStr, "(empty)");
                        } else {
                            StrnCat16(VStr, str, len);
                        }
                    } else {
                        OI_CHAR16* str = va_arg(argp, OI_CHAR16*);
                        if (!str) {
                            OI_VStrCat(VStr, "(null)");
                        } else {
                            StrCat16(VStr, str);
                        }
                    }
                    moreFmt = FALSE;
                    hasVarLen = FALSE;
                    break;

                case 'T': /* Time string */ 
                    {
                        OI_TIME *pTime;

                        pTime = (OI_TIME*) va_arg(argp, OI_TIME*);
                        TimeCat(VStr, pTime);
                    }                        
                    moreFmt = FALSE;
                    break;

                case 'a': /* AT string */
                    if (hasVarLen) {
                        OI_CHAR *str = va_arg(argp, OI_CHAR*);
                        OI_UINT16 len = (OI_UINT16)va_arg(argp, OI_UINT);
                        if (!str) {
                            OI_VStrCat(VStr, "(null)");
                        } else if (len == 0) {
                            OI_VStrCat(VStr, "(empty)");
                        } else {
                            OI_VStrnCatAT(VStr, str, len);
                        }
                    } else {
                        OI_CHAR *str = va_arg(argp, OI_CHAR*);
                        if (!str) {
                            OI_VStrCat(VStr, "(null)");
                        } else {
                            OI_VStrCatAT(VStr, str);
                        }
                    }
                    moreFmt = FALSE;
                    hasVarLen = FALSE;
                    break;
                case '/': /* base file name */
                    {
                        OI_CHAR *s = va_arg(argp, OI_CHAR*);
                        OI_CHAR *f = s;
                        if (s == NULL) {
                            OI_VStrCat(VStr, "(null)");
                        } else {
                            while (*s) {
                                if ((*s == '/') || (*s == '\\')) {
                                    f = s + 1;
                                }
                                ++s;
                            }
                        }
                        OI_VStrCat(VStr, f);
                    }
                    moreFmt = FALSE;
                    break;
                case 'x':
                    if (size == 0) {
                        size = 8; /* default */
                    }
                    if (isLong) {
                        OI_VHexCat(VStr, va_arg(argp, OI_UINT32), size);
                    } else {
                        OI_VHexCat(VStr, (OI_UINT32) va_arg(argp, OI_UINT), size);
                    }
                    moreFmt = FALSE;
                    break;
                case '^':
                    {
                        OI_DATAELEM *elem = va_arg(argp, OI_DATAELEM *);
                        if (!elem) {
                            OI_VStrCat(VStr, "(null)");
                        } else {
                            OI_DataElement_XML(VStr, elem, 0);
                        }
                    }
                    moreFmt = FALSE;
                    break;
                case '=':
                    {
                        OI_OBEX_HEADER *hdr = va_arg(argp, OI_OBEX_HEADER *);
                        if (!hdr) {
                            OI_VStrCat(VStr, "(null)");
                        } else {
                            if (hasVarLen) {
                                hasVarLen = FALSE;
                                size = (OI_UINT16)va_arg(argp, OI_UINT);
                                if (size == 0) {
                                    OI_VStrCat(VStr, "(null)");
                                }
                            } else {
                                size = 1;
                            }
                            while (size--) {
                                OI_OBEX_HeaderTxt(VStr, hdr);
                                if (size > 0) {
                                    OI_VSpaceCat(VStr, 1);
                                }
                                ++hdr;
                            }
                        }
                    }
                    moreFmt = FALSE;
                    break;
                case '%':
                    VStr->Buffer[VStr->Len++] = '%';
                    moreFmt = FALSE;
                    break;
                case '!': /* Format status */
                    OI_VStrCat(VStr, OI_StatusText((OI_STATUS)va_arg(argp, OI_UINT)));
                    moreFmt = FALSE;
                    break;
                case ':': /* Format BD_ADDR */
                    {
                        OI_BD_ADDR *bdAddr;
                        OI_UINT i;

                        bdAddr = va_arg(argp, OI_BD_ADDR *);
                        if (bdAddr == NULL) {
                            OI_VStrCat(VStr, "(null)");
                        } else {
                            for (i = 0; i < OI_BD_ADDR_BYTE_SIZE; ++i) {
                                if (i != 0) {
                                    OI_VStrCat(VStr, ":");
                                }
                                OI_VHexCat(VStr, bdAddr->addr[i], 2 * sizeof(OI_UINT8));
                            }
                        }
                    }
                    moreFmt = FALSE;
                    break;
                case '~': /* UTF encoded Pascal-style string */
                    {
                        OI_PSTR* str = va_arg(argp, OI_PSTR*);
                        OI_UNICODE_ENCODING encoding = (OI_UNICODE_ENCODING)va_arg(argp, OI_UINT);
                        if (!str) {
                            OI_VStrCat(VStr, "(null)");
                        } else {
                            PStrcat(VStr, str, encoding);
                        }
                    }
                    moreFmt = FALSE;
                    break;
   
                case '@': /* Format hex */
                    {
                        OI_BYTE *hex = va_arg(argp, OI_BYTE*);
                        OI_INT len = va_arg(argp, OI_INT);
                        OI_INT i;

                        if (!hex) {
                            OI_VStrCat(VStr, "(null)");
                        } else {
                            /* Sanity check */
                            if ((len < 0) || (len > 32768)) {
                                return OI_STATUS_INVALID_PARAMETERS;
                            }
                            if (size == 0) {
                                size = 16; /* default */
                            }
                            for (i = 0; i < len; ++i) {
                                if (i && (i % size) == 0) {
                                    OI_VStrCat(VStr, "\n");
                                }
                                if (i == MAX_HEX_LEN) {
                                    OI_VStrCat(VStr, " ...");
                                    break;
                                }
                                OI_VHexCat(VStr, hex[i], 2 * sizeof(OI_BYTE));
                                OI_VSpaceCat(VStr, 1);
                            }
                            OI_VStrCat(VStr, "\n");
                        }
                    }
                    moreFmt = FALSE;
                    break;
                case '[': /* ANSI color codes */
                    {
#ifdef OI_SUPPRESS_COLOR_PRINT
                        (void)va_arg(argp, OI_UINT);
#else 
                        OI_CHAR *color;
                        switch (va_arg(argp, OI_UINT)) {
                            case '0':
                                color = ANSI_COLOR_BLACK;
                                break;
                            case 'r':
                                color = ANSI_COLOR_RED;
                                break;
                            case 'g':
                                color = ANSI_COLOR_GREEN;
                                break;
                            case 'b':
                                color = ANSI_COLOR_BLUE;
                                break;
                            case 'c':
                                color = ANSI_COLOR_CYAN;
                                break;
                            case 'y':
                                color = ANSI_COLOR_YELLOW;
                                break;
                            case 'm':
                                color = ANSI_COLOR_MAGENTA;
                                break;
                            case 'W':
                                color = ANSI_COLOR_WHITE;
                                break;
                            case 'R':
                                color = ANSI_COLOR_BR_RED;
                                break;
                            case 'G':
                                color = ANSI_COLOR_BR_GREEN;
                                break;
                            case 'B':
                                color = ANSI_COLOR_BR_BLUE;
                                break;
                            case 'C':
                                color = ANSI_COLOR_BR_CYAN;
                                break;
                            case 'Y':
                                color = ANSI_COLOR_BR_YELLOW;
                                break;
                            case 'M':
                                color = ANSI_COLOR_BR_MAGENTA;
                                break;
                            case 'l':
                            case 'L':
                                color = ANSI_COLOR_LT_GRAY;
                                break;
                            case 'd':
                            case 'D':
                                color = ANSI_COLOR_DK_GRAY;
                                break;
                            default:
                                color = ANSI_COLOR_DEFAULT;
                                break;
                        }
                        OI_VStrCat(VStr, color);
#endif
                        moreFmt = FALSE;
                    }
                    break;
                default:
                    return OI_STRING_FORMAT_ERROR;
            }
            if (size > 32) {
                return OI_STRING_FORMAT_ERROR;
            }
        }
    }
    return OI_OK;
}


OI_STATUS OI_FormatStr(OI_VARSTRING *VStr,
                       const OI_CHAR *format,
                       ...)
{
    OI_STATUS status;
    va_list argp;

    va_start(argp, format);
    status = OI_VFormatStr(VStr, format, argp);
    va_end(argp);

    return status;
}


/*
 * Get null terminated string from a var string
 */

OI_CHAR* OI_VStrGetString(OI_VARSTRING *VStr)
{
    OI_ASSERT(VStr != NULL);
    if (VStr->Buffer == NULL) {
        return NULL;
    } else {
        OI_ASSERT(VStr->Len < VStr->MaxLen);
        VStr->Buffer[VStr->Len] = 0;
        return VStr->Buffer;
    }
}

/**
 * Compare a var string to a C string
 */
OI_INT OI_VStrcmp(OI_VARSTRING *VStr,
                  const OI_CHAR *cStr)
{
    OI_ASSERT(VStr->Len < VStr->MaxLen);
    VStr->Buffer[VStr->Len] = 0;
    return OI_Strcmp(VStr->Buffer, cStr);
}

/**
 * Compare a var string to a C string
 */
OI_INT OI_VStrncmp(OI_VARSTRING  *VStr,
                   const OI_CHAR *cStr,
                   OI_UINT        len)
{
    OI_ASSERT(VStr->Len < VStr->MaxLen);
    VStr->Buffer[VStr->Len] = 0;
    return OI_Strncmp(VStr->Buffer, cStr, (OI_UINT16)len);
}
