/******************************************************************************
 *
 *  Copyright (C) 2014 Google, Inc.
 *
 *  Licensed under the Apache License, Version 2.0 (the "License");
 *  you may not use this file except in compliance with the License.
 *  You may obtain a copy of the License at:
 *
 *  http://www.apache.org/licenses/LICENSE-2.0
 *
 *  Unless required by applicable law or agreed to in writing, software
 *  distributed under the License is distributed on an "AS IS" BASIS,
 *  WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 *  See the License for the specific language governing permissions and
 *  limitations under the License.
 *
 ******************************************************************************/
#include <assert.h>
#include <ctype.h>
#include <stdio.h>
#include <string.h>
#include <hardware/bluetooth.h>
#include <stdbool.h>

#include "hash_map.h"

#ifdef __cplusplus
extern "C"
{
#endif

bool bdaddr_is_empty(const bt_bdaddr_t *addr);

bool bdaddr_equals(const bt_bdaddr_t *first, const bt_bdaddr_t *second);

bt_bdaddr_t *bdaddr_copy(bt_bdaddr_t *dest, const bt_bdaddr_t *src);

const char *bdaddr_to_string(const bt_bdaddr_t *addr, char *string, size_t size);

bool string_is_bdaddr(const char *string);

bool string_to_bdaddr(const char *string, bt_bdaddr_t *addr);

hash_index_t hash_function_bdaddr(const void *key);
#ifdef __cplusplus
}
#endif
