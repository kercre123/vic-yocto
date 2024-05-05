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

#define LOG //LOG_DEBUG

extern FILE *fp;
char line[MAX_ALLOWED_LINE_LEN];

int main()
{
    int user_choice = 0;

again:

    LOG_DEBUG(" \n -------------MENU---------------------------------------------------------\n");
    LOG_DEBUG(" User choice 1 : load data from file into database\n");
    LOG_DEBUG(" User choice 2 : search for given property if exists\n");
    LOG_DEBUG(" User choice 3 : get the values of given property from ds\n");
    LOG_DEBUG(" User choice 4 : set the value into the dt\n");
    LOG_DEBUG(" User choice 5 : overwrite changed dt into persist \n");
    LOG_DEBUG(" User choice 6 : dump the current data structures\n");
    LOG_DEBUG(" User choice 7 : Delete property \n");
    LOG_DEBUG(" User choice 8 : Deinit the prop serivce- will write last set to persist\n");
    LOG_DEBUG(" \n ---------------------------------------------------------------------------\n");

    scanf("%d", &user_choice);
    switch(user_choice)
    {
        int retval = -1;
        bool result = false;
        char prop_name[MAX_ALLOWED_LINE_LEN];
        char prop_val[MAX_ALLOWED_LINE_LEN];

        case LOAD_FROM_PERSIST:
        result = create_node_from_persist(path);
        if (result ==true)
        {
            LOG_DEBUG("List Should be created by now head = %0x\n",
                    __get_list_head());
        } else {
            LOG_DEBUG("List Pull Failure\n");
        }
        break;

        case CHECK_IF_PROP_EXIST:
            //this should pass
            memset(prop_name, 0, MAX_ALLOWED_LINE_LEN);
            strncpy(prop_name , "wc_transport.soc_initialized", strlen("wc_transport.soc_initialized"));//test
            result = check_for_a_property(&prop_name[0]);
            LOG_DEBUG("Property searched =%s , exists =%d \n", prop_name, result);

            //this should pass
            memset(prop_name, 0, MAX_ALLOWED_LINE_LEN);
            strncpy(prop_name , "bluetooth.status", strlen("bluetooth.status"));//test
            result = check_for_a_property(prop_name);
            LOG_DEBUG("Property searched =%s , exists =%d \n", prop_name, result);

            //this should pass
            memset(prop_name, 0, MAX_ALLOWED_LINE_LEN);
            strncpy(prop_name , "wc_transport.start_root", strlen("wc_transport.start_root"));//test
            result = check_for_a_property(prop_name);
            LOG_DEBUG("Property searched =%s , exists =%d \n", prop_name, result);

            //this should fail
            memset(prop_name, 0, MAX_ALLOWED_LINE_LEN);
            strncpy(prop_name , "ksadjlkajdsj.start_root", strlen("ksadjlkajdsj.start_root"));//test
            result = check_for_a_property(prop_name);
            LOG_DEBUG("Property searched =%s , exists =%d \n", prop_name, result);
        break;

        case GET_PROP_VALUE:

            //this should pass
            memset(prop_name, 0, MAX_ALLOWED_LINE_LEN);
            memset(prop_val, 0, MAX_ALLOWED_LINE_LEN);
            strncpy(prop_name , "wc_transport.soc_initialized", strlen("wc_transport.soc_initialized"));//test
            result = get_property_value_bt(prop_name,prop_val);
            LOG_DEBUG("Property searched =%s exists =%d prop_val=%s \n", prop_name, result, prop_val);

            //this should pass
            memset(prop_name, 0, MAX_ALLOWED_LINE_LEN);
            memset(prop_val, 0, MAX_ALLOWED_LINE_LEN);
            strncpy(prop_name , "bluetooth.status", strlen("bluetooth.status"));//test
            result = get_property_value_bt(prop_name,prop_val);
            LOG_DEBUG("Property searched =%s exists =%d prop_val=%s \n", prop_name, result, prop_val);

            //this should pass
            memset(prop_name, 0, MAX_ALLOWED_LINE_LEN);
            memset(prop_val, 0, MAX_ALLOWED_LINE_LEN);
            strncpy(prop_name , "wc_transport.start_root", strlen("wc_transport.start_root"));//test
            result = get_property_value_bt(prop_name,prop_val);
            LOG_DEBUG("Property searched =%s exists =%d prop_val=%s \n", prop_name, result, prop_val);

            //this should fail
            memset(prop_name, 0, MAX_ALLOWED_LINE_LEN);
            memset(prop_val, 0, MAX_ALLOWED_LINE_LEN);
            strncpy(prop_name , "ksadjlkajdsj.start_root", strlen("ksadjlkajdsj.start_root"));//test
            result = get_property_value_bt(prop_name,prop_val);
            LOG_DEBUG("Property searched =%s exists =%d prop_val=%s \n", prop_name, result, prop_val);
        break;

        case SET_PROP_VALUE:

            //this should pass
            memset(prop_name, 0, MAX_ALLOWED_LINE_LEN);
            memset(prop_val, 0, MAX_ALLOWED_LINE_LEN);
            strncpy(prop_name , "wc_transport.soc_initialized", strlen("wc_transport.soc_initialized"));//test
            strncpy(prop_val , "true", strlen("true"));
            result = set_property_value_bt(prop_name,prop_val);
            LOG_DEBUG("Prop update status =%d ,Value @property = %s\n", result, prop_val);


            //this should pass
            memset(prop_name, 0, MAX_ALLOWED_LINE_LEN);
            memset(prop_val, 0, MAX_ALLOWED_LINE_LEN);
            strncpy(prop_name , "bluetooth.status", strlen("bluetooth.status"));//test
            strncpy(prop_val , "false", strlen("false"));
            result = set_property_value_bt(prop_name,prop_val);
            LOG_DEBUG("Prop update status =%d ,Value @property = %s\n", result, prop_val);

            //this should pass
            memset(prop_name, 0, MAX_ALLOWED_LINE_LEN);
            memset(prop_val, 0, MAX_ALLOWED_LINE_LEN);
            strncpy(prop_name , "bluetooth.start_root", strlen("bluetooth.start_root"));//test
            strncpy(prop_val , "true", strlen("true"));
            result = set_property_value_bt(prop_name,prop_val);
            LOG_DEBUG("Prop update status =%d ,Value @property = %s\n", result, prop_val);


            //this should pass
            memset(prop_name, 0, MAX_ALLOWED_LINE_LEN);
            memset(prop_val, 0, MAX_ALLOWED_LINE_LEN);
            strncpy(prop_name , "bluetooth.hciattach", strlen("bluetooth.hciattach"));//test
            strncpy(prop_val , "false", strlen("false"));
            result = set_property_value_bt(prop_name,prop_val);
            LOG_DEBUG("Prop update status =%d ,Value @property = %s\n", result, prop_val);


            //this should add a new property
            memset(prop_name, 0, MAX_ALLOWED_LINE_LEN);
            memset(prop_val, 0, MAX_ALLOWED_LINE_LEN);
            strncpy(prop_name , "kajdsajdkjhciattach", strlen("adhsakjdhksdiattach"));//test
            strncpy(prop_val , "false", strlen("false"));
            result = set_property_value_bt(prop_name,prop_val);
            LOG_DEBUG("Prop update status =%d ,Value @property = %s\n", result, prop_val);

            //this should add a new property
            memset(prop_name, 0, MAX_ALLOWED_LINE_LEN);
            memset(prop_val, 0, MAX_ALLOWED_LINE_LEN);
            strncpy(prop_name , "kajdsajdkjhciattach", strlen("adhsakjdhksdiattach"));//test
            strncpy(prop_val , "true", strlen("true"));
            result = set_property_value_bt(prop_name,prop_val);
            LOG_DEBUG("Prop update status =%d ,Value @property = %s\n", result, prop_val);

            //this should add a new property
            memset(prop_name, 0, MAX_ALLOWED_LINE_LEN);
            memset(prop_val, 0, MAX_ALLOWED_LINE_LEN);
            strncpy(prop_name , "kjhciattach", strlen("kjhciattach"));//test
            strncpy(prop_val , "true", strlen("true"));
            result = set_property_value_bt(prop_name,prop_val);
            LOG_DEBUG("Prop update status =%d ,Value @property = %s\n", result, prop_val);


            //this should add a new property
            memset(prop_name, 0, MAX_ALLOWED_LINE_LEN);
            memset(prop_val, 0, MAX_ALLOWED_LINE_LEN);
            strncpy(prop_name , "askjdlsajdlsajdsaljdajhdaksjdhsakjhd" , strlen("askjdlsajdlsajdsaljdajhdaksjdhsakjhd"));
            strncpy(prop_val , "true", strlen("true"));
            result = set_property_value_bt(prop_name,prop_val);
            LOG_DEBUG("Prop update status =%d ,Value @property = %s\n", result, prop_val);
        break;

        case SAVE_DS_TO_PERSIST:
            result = save_ds_to_persist();
            LOG_DEBUG("Save to persist returned =%d", result);
        break;

        case DUMP_CURRENT_DS:
            dump_current_ds();
            dump_persist();
        break;

        case DELETE_PROPERTY:

            memset(prop_name, 0, MAX_ALLOWED_LINE_LEN);
            strncpy(prop_name , "kajdsajdkjhciattach", strlen("kajdsajdkjhciattach"));//test
            result = __remove_node_from_list(prop_name);
            LOG_DEBUG("Prop update status =%d , @property = %s\n", result, prop_name);

            memset(prop_name, 0, MAX_ALLOWED_LINE_LEN);
            strncpy(prop_name , "bluetooth.hciattach", strlen("bluetooth.hciattach"));//test
            result = __remove_node_from_list(prop_name);
            LOG_DEBUG("Prop update status =%d , @property = %s\n", result, prop_name);
        break;

        case DEINIT_PROPERTY_SERVICE:
            __free_list();
            fp = fopen(path, "w");
            int filewrite_status = -1;
            if (NULL != fp)
            fclose(fp);
        break;

        default:
            break;

    }
        goto again;
}

