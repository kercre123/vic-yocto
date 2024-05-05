#ifndef _OI_VARSTRING_H
#define _OI_VARSTRING_H

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
 *  simple variable-length string package
 *
 *  A variable-length string is represented by an OI_VARSTRING that keeps track of the maximum and current
 *  size of a string buffer.
 *
 *  Functions are provided for appending strings, and integer values in
 *  hex or decimal formats to the string as well as some other convenience functions.
 *
 *  Unlike C strings, the string buffer in a var string can contain embedded null characters. A
 *  helper function is provided for converting a var string to a C string.
 */

#include <stdarg.h>

#include "oi_stddefs.h"
#include "oi_status.h"

/** \addtogroup Misc_Internal */
/**@{*/

#ifdef __cplusplus
extern "C" {
#endif


/**
 * VarStrings are either managed or unamanged. A managed VarString allocates
 * it's own buffer and will reallocate a new buffer (freeing the old one) as
 * required to grow the string. The buffer in an unmanaged VarString is
 * allocated by the caller.  Unmanaged VarStrings are truncated when the string
 * reaches the maximum length, which is one less than MaxLen to allow for null
 * termination.
 */

typedef struct {
    OI_BOOL Managed;   /**< Is string buffer managed by VarString */
    OI_UINT16 MaxLen;  /**< Size of buffer */
    OI_UINT16 Len;     /**< Current length of string */
    OI_CHAR* Buffer;   /**< Character buffer */
    OI_BOOL  Overflow; /**< Indicates that buffer overflow occured */
} OI_VARSTRING;


/**
 * Reset a VARSTRING to an empty string.
 */
#define OI_VStrClear(vstr)        ((vstr)->Len = 0)


/**
 * Accessor function implemented as a macro to obtain the length of a VARSTRING.
 */
#define OI_VStrGetLen(vstr)       ((vstr)->Len)


/**
 * Accessor function implemented as a macro to obtain the underlying buffer from
 * a VARSTRING.
 *
 */
#define OI_VStrGetBuffer(vstr)    ((vstr)->Buffer)


/**
 * Allocate and initialize an empty managed VarString.
 */

OI_STATUS OI_VStrAlloc(OI_VARSTRING *Vstr,
                       OI_UINT       size);


/**
 * Free the string buffer for a managed VarString.
 */

void OI_VStrFree(OI_VARSTRING *Vstr);


/**
 * Append a null terminated C string to a VarString. The null is not appended.
 */

OI_STATUS OI_VStrCat(OI_VARSTRING *VStr,
                     const OI_CHAR *Str);


/**
 * Append N characters to a VarString. Any embedded nulls in the source string
 * will be appended.
 */

OI_STATUS OI_VStrnCat(OI_VARSTRING  *VStr,
                      const OI_CHAR *Str,
                      OI_UINT        N);

/**
 * Append white space to a VarString
 */

OI_STATUS OI_VSpaceCat(OI_VARSTRING *VStr,
                       OI_UINT       Count);

/**
 * Append a hex integer string to a VarString
 *
 * Size is the number of the significant bytes in the integer. The hex value will
 * be written as (2 * Size) bytes. Leading zeroes are always written.
 */

OI_STATUS OI_VHexCat(OI_VARSTRING *VStr,
                     OI_UINT32 Val,
                     OI_UINT Size);


/**
 *
 * Append a signed decimal integer string to a VarString.
 */

OI_STATUS OI_VDecCat(OI_VARSTRING *VStr,
                     OI_INT32 Val);

/**
 * OI_VFormatStr
 *
 */

OI_STATUS OI_VFormatStr(OI_VARSTRING *VStr,
                        const OI_CHAR* format,
                        va_list argp);

/**
 * sprintf-like formatting into a VarString.
 */

OI_STATUS OI_FormatStr(OI_VARSTRING *VStr,
                       const OI_CHAR *format,
                       ...);

/**
 * Get null terminated string from a var string. This appends a final null to the
 * string buffer and returns a pointer to the buffer.
 */

OI_CHAR* OI_VStrGetString(OI_VARSTRING *VStr);


/**
 * Compare a var string to a C string
 */
OI_INT OI_VStrcmp(OI_VARSTRING *VStr,
                  const OI_CHAR *cStr);

/**
 * Compare a var string to a C string
 */
OI_INT OI_VStrncmp(OI_VARSTRING  *VStr,
                   const OI_CHAR *cStr,
                   OI_UINT        len);


/*****************************************************************************/
#ifdef __cplusplus
}
#endif

/**@}*/

#endif /* _OI_VARSTRING_H */

