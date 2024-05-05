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


#ifndef BT_PROP_OPS_H
#define BT_PROP_OPS_H

#include <stdbool.h>
/***********************************************************************
**  Type definitions
***********************************************************************/
#define MAX_ALLOWED_LINE_LEN                (80)
#define MAX_PROPERTY_ITER                   (3)
#define MAX_NUM_PROPERTIES                  (10)

#define MAX_NAME_LEN                        (80)
#define MAX_VALUE_LEN                       (80)

extern const char *path;

#define LOG_TAG "bt_property"
#include <syslog.h>

#define PRI_INFO " I"
#define PRI_WARN " W"
#define PRI_ERROR " E"
#define PRI_DEBUG " D"
#define PRI_VERB " V"

#define ALOG(pri, tag, fmt, arg...) //syslog (LOG_WARNING, fmt, ##arg)
#define LOG_DEBUG(fmt, arg...) ALOG(PRI_VERB, LOG_TAG, fmt, ##arg)


#ifdef USE_GLIB
#include <glib.h>
#define strlcpy g_strlcpy
#endif

typedef enum {
        LOAD_FROM_PERSIST = 1,
        CHECK_IF_PROP_EXIST,
        GET_PROP_VALUE,
        SET_PROP_VALUE,
        SAVE_DS_TO_PERSIST,
        DUMP_CURRENT_DS,
        DELETE_PROPERTY,
        DEINIT_PROPERTY_SERVICE
} user_choice;


typedef struct property_unit{
    unsigned char property_name[MAX_NAME_LEN];     // name of property
    unsigned char property_value[MAX_VALUE_LEN];   // string val of property
    bool callback_to_be_invoked;                  // should the cb be invoked
} property_unit;

typedef struct property_db{
    property_unit unit;
    struct property_db *next;
} property_db;

typedef enum {
    CALLBACK_EXECUTE_PASS,
    CALLBACK_EXECUTE_FAILED,
} callback_state;

typedef enum {
    EXT_NAME, //name extraction index
    EXT_VAL   //val extraction index
} ext_value_index;

typedef callback_state (*user_callback)(const char *property_name,
                         const char* property_value);

/***********************************************************************
**  Exposed Functions
***********************************************************************/

/**
 * Create the complete data list from the persist for first time
 * @param Filename as path
 * @return True on success and false otherwise
 */
bool create_node_from_persist(const char *filename);

/**
 * Get the property value for a given property name
 * @param Char * for property name
 * @param Char * for getting the property value from database/persist
 * @return True on success and false otherwise
 */
bool get_property_value_bt(const char*, unsigned char *);

/**
 * Set the property value for a given property name
 * @param Char * for property name
 * @param Char * this is not used for set, used only for get
 * @return True on success and false otherwise
 */
bool set_property_value_bt(const char*, unsigned char *);


/**
 * Save the complete datastructure on RAM into persist
 * @param Void since the file and ds head are known
 * @return True on success and false otherwise
 */
bool save_ds_to_persist(void);

/**
 * Remove the list node with the matching property name
 * @param property name as char*, which has to be removed
 * @return True on success and false otherwise
 */
int remove_ds_node(const char*);

/**
 * Create the complete data list from the persist for first time
 * @param Filename as path
 * @return True on success and false otherwise
 */
void dump_current_ds(void);

/**
 * Internal function to search from the file line by line
 * and add as new nodes in the data structure
 * @param file name as path (persist file with initial values)
 * @return true for success false otherwise
 */
bool __search_and_add_property_val(const char* fpath);

/**
 * Internal function to Pull the values from property in one line
 * and line will be passed as input for parsing
 * @param file name as path (persist file with initial values)
 * @return true for success false otherwise
 */
property_db* __pull_one_line_data(const char*);

/**
 * Internal function to Pull the values from property in one line
 * and line will be passed as input for parsing
 * @param file name as path (persist file with initial values)
 * @return true for success false otherwise
 */
bool __check_for_a_property(const char*);

/**
 * Internal :UNUSED for the moment - used to dump all the persist properties
 * @param Filename as path
 * @return True on success and false otherwise
 */
void __dump_persist(void);

#define UNUSED(x) (void)(x)

#endif /* BT_PROP_OPS_H */
