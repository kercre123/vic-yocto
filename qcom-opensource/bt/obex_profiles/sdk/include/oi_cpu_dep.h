#ifndef _OI_CPU_DEP_H
#define _OI_CPU_DEP_H

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
 * This file contains definitions for characteristics of the target CPU and
 * compiler, including primitive data types and endianness.
 *
 * This file defines the byte order and primitive data types for various
 * CPU families. The preprocessor symbol 'CPU' must be defined to be an
 * appropriate value or this header will generate a compile-time error.
 *
 * @note The documentation for this header file uses the x86 family of processors
 * as an illustrative example for CPU/compiler-dependent data type definitions.
 * Go to the source code of this header file to see the details of primitive type
 * definitions for each platform.
 *
 * Additional information is available in the @ref data_types_docpage section.
 */

#ifdef __cplusplus
extern "C" {
#endif

/** \addtogroup Misc Miscellaneous APIs */
/**@{*/

/** @name Definitions indicating family of target OI_CPU_TYPE
 *  @{
 */

#define OI_CPU_X86         1 /**< x86 processor family */
#define OI_CPU_ARM7_LEND  10 /**< ARM7, little-endian */
#define OI_CPU_ARM7_BEND  11 /**< ARM7, big-endian */
#define OI_CPU_ARM9_LEND  19 /**< ARM9, little-endian */
#define OI_CPU_ARM11_LEND 20 /**< ARM11, little-endian */

#ifndef OI_CPU_TYPE
    #error "OI_CPU_TYPE type not defined"
#endif

/**@}*/


/** @name Definitions indicating byte-wise endianness of target CPU
 *  @{
 */

#define OI_BIG_ENDIAN_BYTE_ORDER    0  /**< Multiple-byte values are stored in memory beginning with the most significant byte at the lowest address.  */
#define OI_LITTLE_ENDIAN_BYTE_ORDER 1  /**< Multiple-byte values are stored in memory beginning with the least significant byte at the lowest address. */

/**@}*/


/** @name  CPU/compiler-independent primitive data type definitions
 *  @{
 */

typedef int             OI_BOOL;  /**< Boolean values use native integer data type for target CPU. */
typedef int             OI_INT;   /**< Integer values use native integer data type for target CPU. */
typedef unsigned int    OI_UINT;  /**< Unsigned integer values use native unsigned integer data type for target CPU. */
typedef unsigned char   OI_BYTE;  /**< Raw bytes type uses native character data type for target CPU. */

/**@}*/



/*********************************************************************************/

#if OI_CPU_TYPE==OI_CPU_X86

#define OI_CPU_BYTE_ORDER OI_LITTLE_ENDIAN_BYTE_ORDER  /**< x86 platform byte ordering is little-endian */

/** @name CPU/compiler-dependent primitive data type definitions for x86 processor family
 *  @{
 */
typedef signed char     OI_INT8;   /**< 8-bit signed integer values use native signed character data type for x86 processor. */
typedef signed short    OI_INT16;  /**< 16-bit signed integer values use native signed short integer data type for x86 processor. */
typedef signed long     OI_INT32;  /**< 32-bit signed integer values use native signed long integer data type for x86 processor. */
typedef unsigned char   OI_UINT8;  /**< 8-bit unsigned integer values use native unsigned character data type for x86 processor. */
typedef unsigned short  OI_UINT16; /**< 16-bit unsigned integer values use native unsigned short integer data type for x86 processor. */
typedef unsigned long   OI_UINT32; /**< 32-bit unsigned integer values use native unsigned long integer data type for x86 processor. */

/**@}*/

#endif

/*********************************************************************************/

#if OI_CPU_TYPE==OI_CPU_ARM7_LEND
#define OI_CPU_BYTE_ORDER OI_LITTLE_ENDIAN_BYTE_ORDER

/** @name   little-endian CPU/compiler-dependent primitive data type definitions for the ARM7 processor family
 *  @{
 */

typedef signed char     OI_INT8;   /**< 8-bit signed integer values use native signed character data type for ARM7 processor. */
typedef signed short    OI_INT16;  /**< 16-bit signed integer values use native signed short integer data type for ARM7 processor. */
typedef signed long     OI_INT32;  /**< 32-bit signed integer values use native signed long integer data type for ARM7 processor. */
typedef unsigned char   OI_UINT8;  /**< 8-bit unsigned integer values use native unsigned character data type for ARM7 processor. */
typedef unsigned short  OI_UINT16; /**< 16-bit unsigned integer values use native unsigned short integer data type for ARM7 processor. */
typedef unsigned long   OI_UINT32; /**< 32-bit unsigned integer values use native unsigned long integer data type for ARM7 processor. */

/**@}*/

#endif

/*********************************************************************************/

#if OI_CPU_TYPE==OI_CPU_ARM7_BEND
#define OI_CPU_BYTE_ORDER OI_BIG_ENDIAN_BYTE_ORDER
/** @name   big-endian CPU/compiler-dependent primitive data type definitions for the ARM7 processor family
 *  @{
 */
typedef signed char     OI_INT8;   /**< 8-bit signed integer values use native signed character data type for ARM7 processor. */
typedef signed short    OI_INT16;  /**< 16-bit signed integer values use native signed short integer data type for ARM7 processor. */
typedef signed long     OI_INT32;  /**< 32-bit signed integer values use native signed long integer data type for ARM7 processor. */
typedef unsigned char   OI_UINT8;  /**< 8-bit unsigned integer values use native unsigned character data type for ARM7 processor. */
typedef unsigned short  OI_UINT16; /**< 16-bit unsigned integer values use native unsigned short integer data type for ARM7 processor. */
typedef unsigned long   OI_UINT32; /**< 32-bit unsigned integer values use native unsigned long integer data type for ARM7 processor. */

/**@}*/

#endif


/*********************************************************************************/

#if OI_CPU_TYPE==OI_CPU_ARM9_LEND
#define OI_CPU_BYTE_ORDER OI_LITTLE_ENDIAN_BYTE_ORDER

/** @name   little-endian CPU/compiler-dependent primitive data type definitions for the ARM9 processor family
 *  @{
 */

typedef signed char     OI_INT8;   /**< 8-bit signed integer values use native signed character data type for ARM7 processor. */
typedef signed short    OI_INT16;  /**< 16-bit signed integer values use native signed short integer data type for ARM7 processor. */
typedef signed long     OI_INT32;  /**< 32-bit signed integer values use native signed long integer data type for ARM7 processor. */
typedef unsigned char   OI_UINT8;  /**< 8-bit unsigned integer values use native unsigned character data type for ARM7 processor. */
typedef unsigned short  OI_UINT16; /**< 16-bit unsigned integer values use native unsigned short integer data type for ARM7 processor. */
typedef unsigned long   OI_UINT32; /**< 32-bit unsigned integer values use native unsigned long integer data type for ARM7 processor. */

/**@}*/

#endif

/*********************************************************************************/

#if OI_CPU_TYPE==OI_CPU_ARM11_LEND
#define OI_CPU_BYTE_ORDER OI_LITTLE_ENDIAN_BYTE_ORDER

/** @name   little-endian CPU/compiler-dependent primitive data type definitions for the ARM11 processor family
 *  @{
 */

typedef signed char     OI_INT8;   /**< 8-bit signed integer values use native signed character data type for ARM7 processor. */
typedef signed short    OI_INT16;  /**< 16-bit signed integer values use native signed short integer data type for ARM7 processor. */
typedef signed long     OI_INT32;  /**< 32-bit signed integer values use native signed long integer data type for ARM7 processor. */
typedef unsigned char   OI_UINT8;  /**< 8-bit unsigned integer values use native unsigned character data type for ARM7 processor. */
typedef unsigned short  OI_UINT16; /**< 16-bit unsigned integer values use native unsigned short integer data type for ARM7 processor. */
typedef unsigned long   OI_UINT32; /**< 32-bit unsigned integer values use native unsigned long integer data type for ARM7 processor. */

/**@}*/

#endif

/*********************************************************************************/


#ifndef OI_CPU_BYTE_ORDER
    #error "Byte order (endian-ness) not defined"
#endif


/**@}*/

#ifdef __cplusplus
}
#endif

/*********************************************************************************/
#endif /* _OI_CPU_DEP_H */
