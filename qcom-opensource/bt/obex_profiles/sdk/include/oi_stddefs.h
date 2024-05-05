#ifndef OI_STDDEFS_H
#define OI_STDDEFS_H

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
 * This file contains BM3 standard type definitions.
 *
 */

#include "oi_cpu_dep.h"

/** \addtogroup Misc Miscellaneous APIs */
/**@{*/

#ifdef __cplusplus
extern "C" {
#endif

#ifndef FALSE
#define FALSE 0         /**< This define statement sets FALSE as a preprocessor alias for 0. */
#endif

#ifndef TRUE
#define TRUE (!FALSE)  /**< This define statement sets TRUE as a preprocessor alias for !FALSE. */
#endif

#ifndef NULL
    #define NULL ((void*)0) /**< This define statement sets NULL as a preprocessor alias for (void*)0 */
#endif

/**
 *  OI_OFFSETOF has the same usage and semantics as stddef.h offsetof, but is implemented
 *  with a non-zero address to avoid warnings from QTI's SuperLint program which complained 
 *  about the previous version of this macro
 *      Error (Warning) 413: Likely use of null pointer 'unknown-name'
 *  The use of 0x100 as base address 'fixes' Lint
 */

#define OI_OFFSETOF(type, field)   (((int)&(((type *)0x100)->field)) - 0x100)

/**
 * @name  Maximum and minimum values for basic types
 * @{
 */
#define OI_INT8_MIN   ((OI_INT8)0x80)          /**< Decimal value: -128 */
#define OI_INT8_MAX   ((OI_INT8)0x7F)          /**< Decimal value: 127 */
#define OI_INT16_MIN  ((OI_INT16)0x8000)       /**< Decimal value: -32768 */
#define OI_INT16_MAX  ((OI_INT16)0x7FFF)       /**< Decimal value: 32767 */
#define OI_INT32_MIN  ((OI_INT32)0x80000000)   /**< Decimal value: -2,147,483,648 */
#define OI_INT32_MAX  ((OI_INT32)0x7FFFFFFF)   /**< Decimal value: 2,147,483,647 */
#define OI_UINT8_MIN  ((OI_UINT8)0)            /**< Decimal value: 0 */
#define OI_UINT8_MAX  ((OI_UINT8)0xFF)         /**< Decimal value: 255 */
#define OI_UINT16_MIN ((OI_UINT16)0)           /**< Decimal value: 0 */
#define OI_UINT16_MAX ((OI_UINT16)0xFFFF)      /**< Decimal value: 65535 */
#define OI_UINT32_MIN ((OI_UINT32)0)           /**< Decimal value: 0 */
#define OI_UINT32_MAX ((OI_UINT32)0xFFFFFFFF)  /**< Decimal value: 4,294,967,295 */

/**
 * @}
 */

/**
 * @name  Integer types required by the Service Discovery Protocol
 * @{
 */

/** This structure is an unsigned 64-bit integer as a structure
    of two unsigned 32-bit integers. */
typedef struct {
    OI_UINT32 I1; /**< Most significant 32 bits */
    OI_UINT32 I2; /**< Least significant 32 bits */
} OI_UINT64;

#define OI_UINT64_MIN { (OI_UINT32)0x00000000, (OI_UINT32)0x00000000 }
#define OI_UINT64_MAX { (OI_UINT32)0XFFFFFFFF, (OI_UINT32)0XFFFFFFFF }

/** This structure is a signed 64-bit integer as a structure of
    one unsigned 32-bit integer and one signed 32-bit
    integer. */
typedef struct {
    OI_INT32  I1; /**< Most significant 32 bits as a signed integer */
    OI_UINT32 I2; /**< Least significant 32 bits as an unsigned integer */
} OI_INT64;

#define OI_INT64_MIN { (OI_INT32)0x80000000, (OI_UINT32)0x00000000 }
#define OI_INT64_MAX { (OI_INT32)0X7FFFFFFF, (OI_UINT32)0XFFFFFFFF }

/** This structure is an unsigned 128-bit integer as a structure
    of four unsigned 32-bit integers. */
typedef struct {
    OI_UINT32 I1; /**< Most significant 32 bits */
    OI_UINT32 I2; /**< Second-most significant 32 bits */
    OI_UINT32 I3; /**< Third-most significant 32 bits */
    OI_UINT32 I4; /**< Least significant 32 bits */
} OI_UINT128;

#define OI_UINT128_MIN { (OI_UINT32)0x00000000, (OI_UINT32)0x00000000,  (OI_UINT32)0x00000000, (OI_UINT32)0x00000000 }
#define OI_UINT128_MAX { (OI_UINT32)0XFFFFFFFF, (OI_UINT32)0XFFFFFFFF,  (OI_UINT32)0XFFFFFFFF, (OI_UINT32)0XFFFFFFFF }

/** This structure is a signed 128-bit integer as a structure of
    three unsigned 32-bit integers and one signed 32-bit
    integer. */
typedef struct {
    OI_INT32  I1;  /**< Most significant 32 bits as a signed integer */
    OI_UINT32 I2;  /**< Second-most significant 32 bits as an unsigned integer */
    OI_UINT32 I3;  /**< Third-most significant 32 bits as an unsigned integer */
    OI_UINT32 I4;  /**< Least significant 32 bits as an unsigned integer */
} OI_INT128;

#define OI_INT128_MIN { (OI_UINT32)0x80000000, (OI_UINT32)0x00000000,  (OI_UINT32)0x00000000, (OI_UINT32)0x00000000 }
#define OI_INT128_MAX { (OI_UINT32)0X7FFFFFFF, (OI_UINT32)0XFFFFFFFF,  (OI_UINT32)0XFFFFFFFF, (OI_UINT32)0XFFFFFFFF }

/**
 * @}
 */


/**
 * Type for ASCII character data items
 */
typedef char OI_CHAR;

/**
 * Type for double-byte character data items
 */
typedef OI_UINT16 OI_CHAR16;

/**
 * Types for UTF encoded strings.
 */
typedef OI_UINT8  OI_UTF8;
typedef OI_UINT16 OI_UTF16;
typedef OI_UINT32 OI_UTF32;

/**
 * A generic opaque handle type. Some profiles derive their handles from this type.
 */
typedef void* OI_HANDLE;


/**
 * @name Single-bit operation macros
 * @{
 * In these macros, x is the data item for which a bit is to be tested or set and y specifies which bit
 * is to be tested or set.
 */

/** This macro's value is TRUE if the bit specified by y is set in data item x. */
#define OI_BIT_TEST(x,y)   ((x) & (y))

/** This macro's value is TRUE if the bit specified by y is not set in data item x. */
#define OI_BIT_CLEAR_TEST(x,y)  (((x) & (y)) == 0)

/** This macro sets the bit specified by y in data item x. */
#define OI_BIT_SET(x,y)    ((x) |= (y))

/** This macro clears the bit specified by y in data item x. */
#define OI_BIT_CLEAR(x,y)  ((x) &= ~(y))

/** @} */

/**
 * The OI_ARRAYSIZE macro is set to the number of elements in an array
 * (instead of the number of bytes, which is returned by sizeof()).
 */

#ifndef OI_ARRAYSIZE
#define OI_ARRAYSIZE(a) (sizeof(a)/sizeof(a[0]))
#endif

/**
 * @name Preprocessor aliases for individual bit positions
 *      Bits are defined here only if they are not already defined.
 * @{
 */

#ifndef OI_BIT0

#define OI_BIT0   0x00000001  /**< Preprocessor alias for 32-bit value with bit 0 set, used to specify this single bit */
#define OI_BIT1   0x00000002  /**< Preprocessor alias for 32-bit value with bit 1 set, used to specify this single bit */
#define OI_BIT2   0x00000004  /**< Preprocessor alias for 32-bit value with bit 2 set, used to specify this single bit */
#define OI_BIT3   0x00000008  /**< Preprocessor alias for 32-bit value with bit 3 set, used to specify this single bit */
#define OI_BIT4   0x00000010  /**< Preprocessor alias for 32-bit value with bit 4 set, used to specify this single bit */
#define OI_BIT5   0x00000020  /**< Preprocessor alias for 32-bit value with bit 5 set, used to specify this single bit */
#define OI_BIT6   0x00000040  /**< Preprocessor alias for 32-bit value with bit 6 set, used to specify this single bit */
#define OI_BIT7   0x00000080  /**< Preprocessor alias for 32-bit value with bit 7 set, used to specify this single bit */
#define OI_BIT8   0x00000100  /**< Preprocessor alias for 32-bit value with bit 8 set, used to specify this single bit */
#define OI_BIT9   0x00000200  /**< Preprocessor alias for 32-bit value with bit 9 set, used to specify this single bit */
#define OI_BIT10  0x00000400  /**< Preprocessor alias for 32-bit value with bit 10 set, used to specify this single bit */
#define OI_BIT11  0x00000800  /**< Preprocessor alias for 32-bit value with bit 11 set, used to specify this single bit */
#define OI_BIT12  0x00001000  /**< Preprocessor alias for 32-bit value with bit 12 set, used to specify this single bit */
#define OI_BIT13  0x00002000  /**< Preprocessor alias for 32-bit value with bit 13 set, used to specify this single bit */
#define OI_BIT14  0x00004000  /**< Preprocessor alias for 32-bit value with bit 14 set, used to specify this single bit */
#define OI_BIT15  0x00008000  /**< Preprocessor alias for 32-bit value with bit 15 set, used to specify this single bit */
#define OI_BIT16  0x00010000  /**< Preprocessor alias for 32-bit value with bit 16 set, used to specify this single bit */
#define OI_BIT17  0x00020000  /**< Preprocessor alias for 32-bit value with bit 17 set, used to specify this single bit */
#define OI_BIT18  0x00040000  /**< Preprocessor alias for 32-bit value with bit 18 set, used to specify this single bit */
#define OI_BIT19  0x00080000  /**< Preprocessor alias for 32-bit value with bit 19 set, used to specify this single bit */
#define OI_BIT20  0x00100000  /**< Preprocessor alias for 32-bit value with bit 20 set, used to specify this single bit */
#define OI_BIT21  0x00200000  /**< Preprocessor alias for 32-bit value with bit 21 set, used to specify this single bit */
#define OI_BIT22  0x00400000  /**< Preprocessor alias for 32-bit value with bit 22 set, used to specify this single bit */
#define OI_BIT23  0x00800000  /**< Preprocessor alias for 32-bit value with bit 23 set, used to specify this single bit */
#define OI_BIT24  0x01000000  /**< Preprocessor alias for 32-bit value with bit 24 set, used to specify this single bit */
#define OI_BIT25  0x02000000  /**< Preprocessor alias for 32-bit value with bit 25 set, used to specify this single bit */
#define OI_BIT26  0x04000000  /**< Preprocessor alias for 32-bit value with bit 26 set, used to specify this single bit */
#define OI_BIT27  0x08000000  /**< Preprocessor alias for 32-bit value with bit 27 set, used to specify this single bit */
#define OI_BIT28  0x10000000  /**< Preprocessor alias for 32-bit value with bit 28 set, used to specify this single bit */
#define OI_BIT29  0x20000000  /**< Preprocessor alias for 32-bit value with bit 29 set, used to specify this single bit */
#define OI_BIT30  0x40000000  /**< Preprocessor alias for 32-bit value with bit 30 set, used to specify this single bit */
#define OI_BIT31  0x80000000  /**< Preprocessor alias for 32-bit value with bit 31 set, used to specify this single bit */

#endif  /* OI_BIT0 et al */


/** @} */


#ifdef __cplusplus
}
#endif

/**@}*/

/*****************************************************************************/
#endif /* OI_STDDEFS_H */
