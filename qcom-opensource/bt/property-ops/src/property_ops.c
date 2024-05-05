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

#include <stdio.h>
#include <stdbool.h>
#include <string.h>
#include <malloc.h>
#include "../include/property_ops.h"
#include "../include/ll.h"

#define LOG //LOG_DEBUG

FILE *fp = NULL;
const char *path = "../src/property_config.prop";
char line[MAX_ALLOWED_LINE_LEN];
char pulled[MAX_ALLOWED_LINE_LEN];

bool set_property_value_bt(const char* prop_name, unsigned char *prop_val)
{
   return __update_prop_value(prop_name, prop_val);
}

bool get_property_value_bt(const char* prop_name, unsigned char *prop_val)
{
    property_db *retval = __list_matches_prop_name(prop_name);
    if(NULL != retval)
    {
        LOG_DEBUG("Property exist =%s value = %s\n",
            retval->unit.property_name, retval->unit.property_value);
        snprintf(prop_val,MAX_ALLOWED_LINE_LEN,"%s",
                    retval->unit.property_value);
        prop_val[strlen(retval->unit.property_value)] = '\0';
        return true;
    } else {
        LOG_DEBUG("Property doesnt exist\n");
        return false;
    }
}

bool __check_for_a_property(const char* prop_name)
{
    property_db *retval = __list_matches_prop_name(prop_name);
    LOG_DEBUG("Property exist =%s \n", (retval==NULL)? "false": "true");
    return (retval==NULL)? false: true;
}



bool save_ds_to_persist(void)
{
    property_db *ln = (property_db*)__get_list_head();
    unsigned char stringtowrite[MAX_ALLOWED_LINE_LEN];
    bool retval = false;
    int filewrite_status = -1;

    fp = fopen(path, "w");

    if (NULL != fp)
    {
        LOG_DEBUG("File truncate mode\n");
        for (; ln != NULL; ln = ln->next)
        {
            memset(stringtowrite, 0 , MAX_ALLOWED_LINE_LEN);

            snprintf(stringtowrite, MAX_ALLOWED_LINE_LEN, "%s,%s,%s\n",
                    ln->unit.property_name,ln->unit.property_value,
                    ln->unit.callback_to_be_invoked );

            LOG_DEBUG("Writing to persist %s \n", stringtowrite);

            filewrite_status = fputs(stringtowrite, fp);
            if (filewrite_status < 0 || filewrite_status == EOF)
            {
                fclose(fp);
                retval = false;
                return retval;
            } else {
                retval = true;
            }
        }
    } else {
        LOG_DEBUG("File Doesnt Exist\n");
        retval = false;
    }

    if (fp)
        fclose(fp);

    return retval;
}

int remove_ds_node(const char* prop_name)
{
    return __remove_node_from_list(prop_name);
}

bool add_ds_node(property_db *node)
{
    LOG_DEBUG("[%s] => Adding Node to DS %x\n", __func__, node);
    return __list_add(node);
}

void dump_current_ds(void)
{
    __dump_nodes();
}

void __dump_persist(void)
{
    //TO BE IMPLEMENTED
}

property_db* __pull_one_line_data(const char* line)
{
    int curr_length = 0;
    const char *curr_line_ptr = line;
    char *delimiter = NULL;
    int iterator =0;

    //TODO : We can do this using strtok - will be short code
    property_db *extracted_val = (property_db*)calloc(1, sizeof(property_db)
            *sizeof(unsigned char));
    if (extracted_val == NULL) {
        LOG_DEBUG("No Memory");
        return extracted_val;
    }
    extracted_val->next = NULL; //null added here no need to add in ll.c

    for(iterator=0 ; iterator< MAX_PROPERTY_ITER; iterator++)
    {
        LOG_DEBUG("+++++++++++++++++++++++++++++\n");
        delimiter = strchr(curr_line_ptr, ',');
        LOG("[%s] => line pulled =%s", __func__,curr_line_ptr);
        if(delimiter == NULL)
        {
            LOG_DEBUG("[%s] => Delimiter = NULL, search for newline\n");
            if(strchr(curr_line_ptr, '\n') == NULL)
            {
                LOG("%s, Unknown Delimiter on line\n ", __func__);
            } else {
                LOG_DEBUG("%s, 3rd string =%s\n",__func__, curr_line_ptr);
            }
            return extracted_val;
        }
        //position of the delimiter "," in our case would be length to copy
        curr_length = delimiter - curr_line_ptr;
        if (curr_length > MAX_ALLOWED_LINE_LEN)
        {
            curr_length = MAX_ALLOWED_LINE_LEN;
        }
        if(extracted_val != NULL)
        {
            switch (iterator)
            {
                case EXT_NAME:
                    strlcpy(extracted_val->unit.property_name,
                            curr_line_ptr, (curr_length + 1) );
                    LOG_DEBUG("[%s] => Extracted Name =%s\n", __func__,
                            extracted_val->unit.property_name);
                    break;

                case EXT_VAL:
                    strlcpy(extracted_val->unit.property_value,
                            curr_line_ptr, (curr_length + 1));
                    LOG_DEBUG("[%s] => Extracted Value =%s\n", __func__
                            ,extracted_val->unit.property_value);
                    break;

                default:
                    break;
            }
        }

        LOG("[%s] => iterator=%d, curr_line_ptr=%s, delimiter =%s\n", __func__,
                iterator,curr_line_ptr, delimiter);
        LOG_DEBUG("*****************************\n");
        curr_line_ptr = delimiter+1; //+1 for the delimiter itself
    }
    return extracted_val;
}

bool __search_and_add_property_val(const char* fpath)
{
    fp = fopen(fpath, "r+");
    int line_num =0;
    int retval = -1;
    bool list_add_status = false;
    property_db *extracted_node = NULL;

    LOG("%s ++",  __func__);
    if (NULL != fp)
    {
        while(fgets(line, MAX_ALLOWED_LINE_LEN, fp))
        {
            line_num++;
            LOG_DEBUG("%s, lineread = %s line_num=%d ", __func__,
                            line, line_num);
            extracted_node = (property_db *)__pull_one_line_data(line);
            if (NULL != extracted_node)
            {
                LOG_DEBUG("Node Extracted, adding to list %x\n",
                    extracted_node);
                list_add_status = add_ds_node(extracted_node);
                LOG("%s, extracted_node=%0x added status=%d",
                        __func__, extracted_node,list_add_status);
            }
            memset(line, 0, MAX_ALLOWED_LINE_LEN);
            continue;
        }
        LOG("%s, reached EOF", __func__);
    } else {
        LOG ("%s, no %s", __func__, line);
        list_add_status = false;
        return list_add_status;
    }

    if (fp)
        fclose(fp);

    return list_add_status;
}

bool create_node_from_persist(const char *filename)
{
    return __search_and_add_property_val(filename);
}

