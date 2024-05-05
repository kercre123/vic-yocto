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
 *
 *  Generic utility for mapping from an arbitrary key to a value.
 */

#define __OI_MODULE__ OI_MODULE_SUPPORT

#include "oi_stddefs.h"
#include "oi_memmgr.h"
#include "oi_simplemap.h"


typedef struct _Entry {
    const void *key;
    const OI_CHAR *keyNamespace;
    void *value;
    struct _Entry *next;
} Entry;


static Entry *map;


void OI_SimpleMap_Init(void)
{
    map = NULL;
}


static Entry* Get(const void *key, const OI_CHAR *keyNamespace)
{
    Entry *e = map;
    while (e) {
        if ((e->key == key) && (e->keyNamespace == keyNamespace)) {
            return e;
        }
        e = e->next;
    }
    return NULL;
}


void OI_SimpleMap_Add(const void *key, const OI_CHAR *keyNamespace, void *value)
{
    Entry *e = Get(key, keyNamespace);
    if (!e) {
        e = OI_Malloc(sizeof(Entry));
        if (!e) {
            return;
        }
        e->key = key;
        e->keyNamespace = keyNamespace;
        e->next = map;
        map = e;
    }
    e->value = value;
}


void* OI_SimpleMap_Remove(const void *key, const OI_CHAR *keyNamespace)
{
    void *value;
    Entry *prev = NULL;
    Entry *e = map;
    while (e) {
        if ((e->key == key) && (e->keyNamespace == keyNamespace)) {
            if (prev) {
                prev->next = e->next;
            } else {
                map = e->next;
            }
            value = e->value;
            OI_Free(e);
            return value;
        }
        prev = e;
        e = e->next;
    }
    return NULL;
}


void* OI_SimpleMap_Get(const void *key, const OI_CHAR *keyNamespace)
{
    Entry *e = Get(key, keyNamespace);
    return e ? e->value : NULL;
}


const void* OI_SimpleMap_Enum(void **iter, const OI_CHAR *keyNamespace)
{
    Entry *e;
    if (!iter) {
        return NULL;
    }
    if (*iter == iter) {
        *iter = NULL;
        return NULL;
    }
    /* Check for restart */
    if (*iter == NULL) {
        *iter = map;
    }
    e = (Entry*)(*iter);
    while (e) {
        if (e->keyNamespace == keyNamespace) {
            if (e->next) {
                *iter = e->next;
            } else {
                /* End marker */
                *iter = iter;
            }
            return e->key;
        }
        e = e->next;
    }
    *iter = NULL;
    return NULL;
}
