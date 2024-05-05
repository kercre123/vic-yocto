/******************************************************************************
 *
 * Copyright (c) 2016, The Linux Foundation. All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are
 * met:
 *   * Redistributions of source code must retain the above copyright
 *     notice, this list of conditions and the following disclaimer.
 *   * Redistributions in binary form must reproduce the above
 *     copyright notice, this list of conditions and the following
 *     disclaimer in the documentation and/or other materials provided
 *     with the distribution.
 *   * Neither the name of The Linux Foundation nor the names of its
 *     contributors may be used to endorse or promote products derived
 *     from this software without specific prior written permission.
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
 * IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE
 *
 *****************************************************************************/

#include "../include/ll.h"
#include "../include/property_ops.h"


property_db* glisthead = NULL;

bool __list_is_empty()
{
  if (!glisthead)
    return true;
  else
    return false;
}

property_db* __list_matches_prop_name(const char* search_name)
{
    if(__list_is_empty())
    {
        LOG_DEBUG("List is empty, return \n");
        return NULL;
    }
    property_db *ln = glisthead;
    for (; ln !=NULL; ln  = ln ->next)
    {
        if (!strncmp(ln->unit.property_name, search_name, strlen(search_name)))
        {
            LOG_DEBUG("[%s] => search val=%s, curr val =%s\n", __func__,
                    search_name,ln->unit.property_name);
            return ln;
        }
    }
    return NULL;
}

//supposed to create one node here once not from the property_ops
bool __create_list_and_add( const char* search_name, const char* property_value)
{
    property_db *ln = (property_db*) calloc(1, sizeof(property_db)*
        sizeof(unsigned char));

    if (ln == NULL )
        return false;

    strlcpy(ln->unit.property_name, search_name, (strlen(search_name) + 1));
    strlcpy(ln->unit.property_value, property_value, (strlen(property_value) +1));

    ln->next = NULL;

    return __list_add(ln);
}

bool __update_prop_value(const char* search_name, const char* property_value)
{
    property_db *ln = NULL;
    int retval = -1;

    ln = __list_matches_prop_name(search_name);
    if(ln != NULL)
    {
        LOG_DEBUG("List Matches property Updating Value\n");
        memset(ln->unit.property_value, 0, sizeof(ln->unit.property_value));
        strlcpy(ln->unit.property_value,  property_value,
            (strlen(property_value) + 1));
        LOG_DEBUG("Value copied to the db prop name %s", search_name);
        retval =0;
    } else {
        LOG_DEBUG("New value not in the list, create a new node\n");
        retval = __create_list_and_add(search_name, property_value);
        LOG_DEBUG("Node Created Status =%d for prop name %s\n", retval,
            search_name);
    }

    return ((retval==0)? true:false);
}

bool __list_add(property_db* list)
{
    property_db *ln;
    if (__list_is_empty())
    {
        LOG_DEBUG("Adding first Node\n");
        glisthead = list;//assumed ln comes with NULL terminated next
    } else {
        LOG_DEBUG("First Node Present, add subsequent one\n");
        //check if already the property exists
        if (NULL == __list_matches_prop_name(list->unit.property_name))
        {
            //Add node to the end of ln.
            for (ln = glisthead; ln->next != NULL; ln = ln->next);
            ln->next = list;//assumed ln comes with NULL terminated next
        }
    }
    return 0;
}

bool __remove_node_from_list(unsigned char* property_name)
{
    bool retval;

    if (__list_is_empty())
    {
        LOG_DEBUG("List is Empty\n");
        retval = false;
    } else {
        property_db *ln_prev, *ln = glisthead;

        while(ln != NULL) {
            if (!strncmp(ln->unit.property_name, property_name,
                strlen(property_name))) {
                if (glisthead == ln) {
                    //first node matches.
                    glisthead = ln->next;
                    free(ln);
                    LOG_DEBUG("First Node Matched and head moved\n");
                    retval = true;
                    break;
                } else {
                    //remove the element here
                    LOG_DEBUG("Removing Property Entry for @prop name =%s\n",
                        ln->unit.property_name);
                    ln_prev->next = ln->next;
                    free(ln);
                    retval = true;
                    break;
                }
            }
            ln_prev = ln;
            ln = ln->next;
        }
    }
    return retval;
}

//to be called on deinit
bool __free_list()
{
    bool retval;
    property_db *ln = glisthead;
    if (__list_is_empty())
    {
        LOG_DEBUG("List is Empty\n");
        retval = false;
    } else {
        while(ln != NULL)
        {
            property_db *temp = ln;
            ln = ln->next;
            free(temp);
        }
    }
    return retval;
}

property_db* __get_list_head()
{
    return glisthead;
}

void __dump_nodes()
{
    property_db *ln = glisthead;
    if (__list_is_empty())
    {
        LOG_DEBUG("List is Empty\n");
        return;
    }
    for (; ln != NULL; ln = ln->next)
    {
        LOG_DEBUG("%s,%s,%s\n", ln->unit.property_name,
                ln->unit.property_value, ln->unit.callback_to_be_invoked);
    }
}

