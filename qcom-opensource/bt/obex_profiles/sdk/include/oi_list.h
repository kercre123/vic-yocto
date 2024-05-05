#ifndef _OI_LIST_H
#define _OI_LIST_H

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

 Doubly-linked list interface. A linked list consists of a dummy
 header OI_LIST_ELEM and an OI_LIST_ELEM for each actual element in
 the list.
*/

#include "oi_stddefs.h"


#ifdef __cplusplus
extern "C" {
#endif


typedef struct OI_LIST_ELEM {
    struct OI_LIST_ELEM *next;
    struct OI_LIST_ELEM *prev;
} OI_LIST_ELEM;

/** Initialize a list */
#define OI_LIST_INIT(x) { &(x), &(x) }

/** Compile-time init */
#define OI_LIST(x) OI_LIST_ELEM x = OI_LIST_INIT(x)

/**
 * Obtain a pointer to the structure containing a given list element.
 *
 * @param elem  The list element.
 * @param type  The type of the containing structure.
 * @param field  The name of the list field in the containing structure.
 */
#define OI_LIST_ENTRY(elem, type, field) \
  ((type *) (((OI_CHAR *) elem) - OI_OFFSETOF(type, field)))

/** Iterate through a list.
 *
 * @param pos   The loop variable (OI_LIST_ELEM *).
 * @param list  The list (OI_LIST_ELEM *).
 */
#define OI_LIST_FOREACH(pos, list) for( (pos) = (list)->next; (pos) != (list); (pos) = (pos)->next )

/** Initialize a list at runtime. */
void OI_List_DynamicInit(OI_LIST_ELEM *list);

/** Add the new element to the front of the list specified by list. */
void OI_List_Add(OI_LIST_ELEM *newElem, OI_LIST_ELEM *list);

/** Add the new element to the tail of the list specified by list. */
void OI_List_AddTail(OI_LIST_ELEM *newElem, OI_LIST_ELEM *list);

/**
 * Delete the specified elem from its list.
 *
 * @return  The element before the elem that was deleted.
 */
OI_LIST_ELEM* OI_List_Del(OI_LIST_ELEM *elem);


/** Remove and return the first element in the list. */
OI_LIST_ELEM *OI_List_RemoveHead(OI_LIST_ELEM *list);


/**
 * Returns a count of the number of elements in the list
 */
OI_UINT OI_List_CountElements(OI_LIST_ELEM *list);


/** Test whether a list is empty.
OI_BOOL OI_List_IsEmpty(OI_LIST_ELEM *head)
*/
#define OI_List_IsEmpty(head) ((head)->next == (head))


#ifdef __cplusplus
}
#endif

#endif
