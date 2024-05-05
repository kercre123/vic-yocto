#ifndef _OI_SIMPLEMAP_H
#define _OI_SIMPLEMAP_H

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
 * @file
 *
 *  Generic utility for mapping from an arbitrary key to a value.
 */

#include "oi_stddefs.h"


#ifdef __cplusplus
extern "C" {
#endif


/**
 * Initialize this module
 */
void OI_SimpleMap_Init(void);


/**
 * Add an entry. Key values must be unique for a given namespace.
 *
 * Note that the namespace is matched by pointer value not a string comparison.
 *
 * @param key           The key
 * @param keyNamespace  The namespace for key
 * @param value         The value
 */
void OI_SimpleMap_Add(const void *key, const OI_CHAR *keyNamespace, void *value);


/**
 * @param key           The key
 * @param keyNamespace  The namespace of key
 * @return  The previously mapped value or NULL if there was no such mapping
 *
 * Note that the namespace is matched by pointer value not a string comparison.
 */
void* OI_SimpleMap_Remove(const void *key, const OI_CHAR *keyNamespace);


/**
 * @param key           The key
 * @param keyNamespace  The namespace of key
 * @return  The mapped value or NULL if there is no such mapping
 *
 * Note that the namespace is matched by pointer value not a string comparison.
 */
void* OI_SimpleMap_Get(const void *key, const OI_CHAR *keyNamespace);


/**
 * Enumerates all keys for the specified namespace
 *
 * @param iter          Initialized to pointer to NULL, keeps track of progress
 * @param keyNamespace  The namespace to iterate over
 *
 * @return The next entry or NULL when the map is exhausted
 */
const void* OI_SimpleMap_Enum(void **iter, const OI_CHAR *keyNamespace);


#ifdef __cplusplus
}
#endif

#endif /* OI_SIMPLEMAP_H */
