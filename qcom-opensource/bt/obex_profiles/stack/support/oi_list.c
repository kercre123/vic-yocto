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
 @file
@internal

 linked list functions
*/

#define __OI_MODULE__ OI_MODULE_SUPPORT

#include "oi_core_common.h"
#include "oi_list.h"

/** Internal function for adding to a list between two elements. */
static void Add(OI_LIST_ELEM *newElem, OI_LIST_ELEM *prev, OI_LIST_ELEM *next)
{
    if (newElem == NULL || prev == NULL || next == NULL)
        return;

    prev->next = newElem;
    newElem->next = next;
    newElem->prev = prev;
    next->prev = newElem;
}

void OI_List_DynamicInit(OI_LIST_ELEM *elem)
{
    if (elem == NULL)
        return;

    elem->prev = elem;
    elem->next = elem;
}


void OI_List_Add(OI_LIST_ELEM *newElem, OI_LIST_ELEM *list)
{
    if (list == NULL)
        return;

    Add(newElem, list, list->next);
}


void OI_List_AddTail(OI_LIST_ELEM *newElem, OI_LIST_ELEM *list)
{
    if (list == NULL)
        return;

    Add(newElem, list->prev, list);
}


OI_LIST_ELEM* OI_List_Del(OI_LIST_ELEM *elem)
{
    if (elem == NULL)
        return NULL;

    OI_LIST_ELEM *prev = elem->prev;
    OI_LIST_ELEM *next = elem->next;

    /*
     * Ensure that element to be deleted is in list.
     */
    if ((prev->next != elem) || (next->prev != elem)) {
        OI_LOG_ERROR(("Attempt to delete element not in list"));
        return elem;
    }
    prev->next = next;
    next->prev = prev;
#ifdef OI_DEBUG
    elem->next = NULL;
    elem->prev = NULL;
#endif
    return prev;
}


OI_LIST_ELEM *OI_List_RemoveHead(OI_LIST_ELEM *list)
{
    OI_LIST_ELEM *elem;

    if (list == NULL)
        return NULL;

    if (OI_List_IsEmpty(list)) {
        return NULL;
    }

    elem = list->next;
    OI_List_Del(elem);
    return elem;
}


OI_UINT OI_List_CountElements(OI_LIST_ELEM *list)
{
    OI_UINT count = 0;
    OI_LIST_ELEM *pos;

    OI_LIST_FOREACH(pos, list) {
        ++count;
    }
    return count;
}
