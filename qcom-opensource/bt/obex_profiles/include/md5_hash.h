#ifndef _MD5_HASH_H
#define _MD5_HASH_H
/** @file
 * @internal
 * This code implements the MD5 message-digest algorithm. The algorithm is due
 * to Ron Rivest and is described in RFC 1321.
 *
 * This code was written by Colin Plumb in 1993; no copyright is claimed.
 *
 * To compute the message digest of a chunk of bytes, declare an MD5_Context
 * structure, pass it to OI_MD5_Init, call OI_MD5_Update as needed on buffers full of
 * bytes, and then call OI_MD5_Final, which will fill a supplied 16-byte array with
 * the digest.
 */

#include "oi_stddefs.h"

/** \addtogroup Misc_Internal */
/**@{*/

#ifdef __cplusplus
extern "C" {
#endif


typedef struct {
    OI_UINT32 buf[4];
    OI_UINT32 bytes[2];
    OI_UINT32 in[16];
} MD5_CONTEXT;


void OI_MD5_Init(MD5_CONTEXT *ctx);


void OI_MD5_Update(MD5_CONTEXT *ctx,
                OI_BYTE *buf,
                OI_UINT16 len);


void OI_MD5_Final(OI_BYTE *digest,
               MD5_CONTEXT *ctx);


#ifdef __cplusplus
}
#endif

/**@}*/

#endif /* _MD5_HASH_H */

