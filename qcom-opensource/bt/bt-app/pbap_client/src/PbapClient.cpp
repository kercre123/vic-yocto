/*
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
 */

#include <iostream>
#include <list>
#include <map>
#include "osi/include/log.h"
#include "osi/include/compat.h"
#include "PbapClient.hpp"
#include "utils.h"
#include "oi_obex.h"
#include "oi_obex_lower.h"
#include "oi_pbap_client.h"
#include "oi_utils.h"
#include "oi_assert.h"
#include <string.h>
#ifdef __cplusplus
extern "C"
{
#endif

#define LOGTAG "PBAPClient "

using namespace std;

PbapClient *g_pbapClient = NULL;;

typedef enum {
    DOWNLOAD                = 0x0001,
    BROWSING                = 0x0002,
    DATABASE_IDENTIFIER     = 0x0004,
    FOLDER_VERSION_COUNTER  = 0x0008,
    VCARD_SELECTING         = 0x0010,
    ENHANCE_MISESD_CALLS    = 0x0020,
    X_BT_UCI_VCARD_PROP     = 0x0040,
    X_BT_UID_VCARD_PROP     = 0x0080,
    CONTACT_REFRENCING      = 0x0100,
    DEFAULT_VCARD_FORMAT    = 0x0200,
} PBAP_SUPPORTED_FEATURES;

static uint8_t  UUID_PBAP_PSE[] = {0x00, 0x00, 0x11, 0x2F, 0x00, 0x00, 0x10, 0x00,
                                   0x80, 0x00, 0x00, 0x80, 0x5F, 0x9B, 0x34, 0xFB};
static  uint32_t profileVersion = 0x0102;
static char profile_name[] = "PBAP Client";
static char storageDir[] = "/data/misc/bluetooth/";
static char vcardFile[] = "vcard.vcf";
static char phoneBookFile[] = "pb.txt";
static char vCardListingFile[] = "vcarListing.txt";

#define UUID_MAX_LENGTH 16
#define IS_UUID(u1,u2)  !memcmp(u1,u2,UUID_MAX_LENGTH)

/******************************************************************************
 * Forward struct typedef
 */
typedef struct pbap_client_data_struct PBAP_CLIENT_DATA;

/******************************************************************************

/******************************************************************************
 * Callback prototypes
 */
static void sdp_add_record_callback(bt_status_t status, int handle);
static void sdp_remove_record_callback(bt_status_t status);
static void sdp_search_callback(bt_status_t status, bt_bdaddr_t *bd_addr, uint8_t* uuid,
            bluetooth_sdp_record *record, bool more_result);

/******************************************************************************

/******************************************************************************
 * Function prototypes
 */
/* run-time variable access function prototypes */
static void set_repository(const OI_CHAR *str);
static void get_repository();
static void set_phonebook(const OI_CHAR *str);
static void get_phonebook();
static void set_format(const OI_CHAR *str);
static void get_format();
static void set_filter(const OI_CHAR *str);
static void get_filter();
static void set_order(const OI_CHAR *str);
static void get_order();

/******************************************************************************
 *
 * Basic utilities.
 *
 */

#ifndef isdelimiter
#define isdelimiter(c) ((c) == ' ' || (c) == ',' || (c) == '\f' || (c) == '\n' || \
        (c) == '\r' || (c) == '\t' || (c) == '\v')
#endif

#ifndef isdigit
#define isdigit(c) ((c) >= '0' && (c) <= '9')
#endif

#ifndef ishexdigit
#define ishexdigit(c) (((c) >= '0' && (c) <= '9') || ((c) >= 'a' && (c) <= 'f') || \
        ((c) >= 'A' && (c) <= 'F'))
#endif

#ifndef toupper
#define toupper(c) (((c) >= 'a') && ((c) <= 'z') ? ((c) - 32) : (c))
#endif

#ifndef tolower
#define tolower(c) (((c) >= 'A') && ((c) <= 'Z') ? ((c) + 32) : (c))
#endif

/******************************************************************************
 * Valid values for each PBAP client variable.
 */
static const OI_CHAR *valid_repositories[] = {
    "local",
    "sim1",
    "unset"
};

static const OI_CHAR *valid_phonebooks[] = {
    "pb",
    "ich",
    "och",
    "mch",
    "cch",
    "unset"
};

static const OI_CHAR *valid_formats[] = {
    "v2.1",
    "v3.0"
};

static const OI_CHAR *valid_filters[] = {
    "version",
    "fn",
    "n",
    "photo",
    "bday",
    "adr",
    "label",
    "tel",
    "email",
    "mailer",
    "tz",
    "geo",
    "title",
    "role",
    "logo",
    "agent",
    "org",
    "note",
    "rev",
    "sound",
    "url",
    "uid",
    "key",
    "nickname",
    "categories",
    "proid",
    "class",
    "sort_string",
    "call_daytime",
    "all",  /* short cut to enable all fields */
};

static const OI_CHAR *valid_paths[] = {
    "/telecom",
    "/telecom/pb",
    "/telecom/ich",
    "/telecom/och",
    "/telecom/mch",
    "/telecom/cch",
    "/SIM1/telecom/",
    "/SIM1/telecom/pb",
    "/SIM1/telecom/ich",
    "/SIM1/telecom/och",
    "/SIM1/telecom/mch",
    "/SIM1/telecom/cch",
};

static const OI_CHAR *valid_orders[] = {
    "indexed",
    "alphabetical",
    "phonetical"
};

static const OI_CHAR *valid_search_attribs[] = {
    "name",
    "number",
    "sound"
};

typedef enum {
    STATE_INITIAL,      /**< Initial state. */
    STATE_IDLE,         /**< Idle state. */
    STATE_CONNECTING,   /**< Connecting state. */
    STATE_CONNECTED,     /**< Connected state. */
    STATE_DEINITIALIZING, /**< De-initializing state. */
} PBAP_STATE;

/**
This structure defines the PBAP client data structure.
*/
struct pbap_client_data_struct {
    int rec_handle;                                 /**< Record Handle for PBAP Client SDP */
    OI_PBAP_CONNECTION connection;                  /**< Connection ID to a server */
    OI_OBEX_LOWER_PROTOCOL lowerProtocol;           /**< Lower protocol info of server */
    OI_BOOL connected;                              /**< Indicates if connected to a server */
    PBAP_STATE state;                               /**< Indicates current state of pbap client */
    OI_UINT32 supportedFeatures;                    /**< Indicates supported features of pbap client */
    OI_UINT16 pb_size;                              /**< Indicates phonebook size */
    OI_BD_ADDR addr;                                /**< Remote PBAP server address */
    OI_CHAR *fileName;                             /**< File Name of currently active pbap data being transferred */
    OI_PBAP_REPOSITORY repository;                  /**< Run-time variable: current repository */
    OI_PBAP_PHONEBOOK phonebook;                    /**< Run-time varialbe: current phonebook */
    OI_PBAP_FORMAT_TAG_VALUES format;               /**< Run-time variable: vCard format */
    OI_UINT64 filter;                               /**< Run-time variable: vCard entry filter */
    OI_PBAP_ORDER_TAG_VALUES order;                 /**< Run-time variable: sorting order */
    OI_PBAP_SEARCH_ATTRIBUTE_TAG_VALUES searchAttribute; /**< Run-time variable: search attribute */
    OI_UINT32 start_offset;                         /**< Run-time variable: vCard list start offset */
    OI_UINT32 max_list_count;                       /**< Run-time variable: vCard max list count */
    OI_BOOL abort;                                  /**< TRUE if current op is to be aborted */
    OI_BOOL getting_phonebook;                      /**< TRUE if pulling phonebook data */
    OI_BOOL getting_listing;                        /**< TRUE if pulling phonebook list */
    OI_BOOL getting_vcard;                          /**< TRUE if pulling vcard */
    OI_BOOL set_path;                               /**< TRUE if performing set path */
    OI_BYTE *searchValue;                           /**< Search Value */
    OI_UINT8 searchValueLen;                        /**< Search Value Length */
};

static PBAP_CLIENT_DATA pbap_client;

/******************************************************************************
 * This structure defines the PBAP client variable.
 */
typedef struct {
    const OI_CHAR *name;            /**< Run-time variable name */
    const OI_CHAR *description;     /**< Run-time variable description */
    const OI_CHAR **valid_options;  /**< List of valid variable values */
    OI_INT valid_option_cnt;        /**< Size of valid value list */
} PBAP_CLIENT_VARIABLE;

/******************************************************************************
 * List of PBAP client variables.
 */
const PBAP_CLIENT_VARIABLE variable_list[] = {
    { "repository", "Phonebook repository to access",
        valid_repositories, OI_ARRAYSIZE(valid_repositories) - 1 },
    { "phonebook", "Phonebook object to access",
        valid_phonebooks, OI_ARRAYSIZE(valid_phonebooks) - 1 },
    { "format", "vCard format of phonebook data", valid_formats, OI_ARRAYSIZE(valid_formats) },
    { "filter", "vCard fields to get (use , separated list for multiple fields)",
      valid_filters, OI_ARRAYSIZE(valid_filters) },
    { "order", "vCard listing sort order", valid_orders, OI_ARRAYSIZE(valid_orders) },
    { "attributes", "vCard listing search attributes", valid_search_attribs, OI_ARRAYSIZE(valid_search_attribs) },
    { "path", "Virtual Paths to be set on Server", valid_paths, OI_ARRAYSIZE(valid_paths) },
};

/* PBAP data reception function prototypes */
static OI_STATUS pb_open(const OI_OBEX_UNICODE *name, OI_PBAP_OPEN_CFM open_cfm,
    OI_PBAP_CONNECTION connection);
static void pb_close(OI_PBAP_HANDLE handle, OI_PBAP_CONNECTION pbapConnection, OI_STATUS status);
static OI_STATUS pb_write(OI_PBAP_HANDLE handle, const OI_BYTE *buffer, OI_UINT16 buf_len,
    OI_PBAP_WRITE_CFM write_cfm, OI_PBAP_CONNECTION connection);
static void print_help(const PBAP_CLIENT_VARIABLE *var);

static const OI_PBAP_CLIENT_FILESYS_FUNCTIONS pbap_file_ops = {
    pb_open,
    pb_close,
    pb_write
};

/**
 * skip_delimiter()
 *
 * This function returns a pointer to the first non-delimiter character.
 */
static const OI_CHAR * skip_delimiter(const OI_CHAR *data)
{
    OI_ASSERT(data);

    while (*data && isdelimiter(*data)) {
        data++;
    }
    return data;
}

/**
 * find_str_in_list()
 *
 * Returns the index of an entry in a list of strings that matches the search
 * string.
 */
static int find_str_in_list(const OI_CHAR *str, const OI_CHAR * const *list,
                                   OI_INT list_size)
{
    OI_INT i;
    OI_INT item = list_size;
    OI_INT match_cnt = 0;
    OI_UINT len;

    OI_ASSERT(str);
    OI_ASSERT(list);
    OI_ASSERT(list_size > 0);

    for (len = 0; (str[len] != '\0' && !isdelimiter(str[len])); len++);

    for (i = 0; i < list_size; i++) {
        if (!OI_StrncmpInsensitive(list[i], str, (OI_UINT16)len)) {
            item = i;
            match_cnt++;
            if (len == strlen(list[i])) {
                break;
            }
        }
    }

    if (match_cnt == 1) {
        return item;
    } else {
        return list_size;
    }
}

/******************************************************************************
 *
 * PBAP client sample variable manipulation functionality.
 *
 */

/**
 * set_repository()
 *
 * This function sets the repository variable used for determining which set
 * of phonebooks to access.  It also tells BM3 of the newly set repository
 * which in turn will tell the server.
 */
static void set_repository(const OI_CHAR *str)
{
    OI_UINT i;

    OI_ASSERT(str);

    if (*str == '\0') {
        fprintf(stdout, "Repository cannot be set to nothing\n");
        print_help(&variable_list[0]);
        return;
    }

    i = find_str_in_list(str, valid_repositories, OI_ARRAYSIZE(valid_repositories) - 1);

    if (i < OI_ARRAYSIZE(valid_repositories) - 1) {
        pbap_client.repository = (OI_PBAP_REPOSITORY)i;
        fprintf(stdout, "Repository set to: %s\n", str);
    } else {
        fprintf(stdout, "Invalid repository name: %s\n", str);
        print_help(&variable_list[0]);
    }
}

/**
 * get_repository()
 *
 * This function puts a string representation of the repository's value into a
 * string buffer.
 */
static void get_repository()
{
    OI_CHAR str[256];
    OI_UINT str_len = 256;

    strlcpy(str, valid_repositories[pbap_client.repository], str_len);
    fprintf(stdout, "Current repository = %s\n", str);
    ALOGV(LOGTAG "%s: Current repository: %s", __FUNCTION__, str);
    print_help(&variable_list[0]);
}

/**
 * set_phonebook()
 *
 * This function sets the phonebook variable used for determining which set of
 * phonebook entries to access.  It also tells BM3 of the newly set phonebook
 * which in turn will tell the server.
 */
static void set_phonebook(const OI_CHAR *str)
{
    OI_UINT i;

    OI_ASSERT(str);

    if (*str == '\0') {
        OI_Printf("phonebook cannot be set to nothing\n");
        fprintf(stdout, "Phonebook cannot be set to nothing \n");
        print_help(&variable_list[1]);
        return;
    }

    i = find_str_in_list(str, valid_phonebooks, OI_ARRAYSIZE(valid_phonebooks) - 1);

    if (i < OI_ARRAYSIZE(valid_phonebooks) - 1) {
        pbap_client.phonebook = (OI_PBAP_PHONEBOOK)i;
        fprintf(stdout, "Phonebook set to: %s\n", str);
    } else {
        fprintf(stdout, "Invalid phonebook name: %s\n", str);
        print_help(&variable_list[1]);
    }
}

/**
 * get_phonebook()
 *
 * This function puts a string representation of the phonebook's value into a
 * string buffer.
 */
static void get_phonebook()
{
    OI_CHAR str[256];
    OI_UINT str_len = 256;

    strlcpy(str, valid_phonebooks[pbap_client.phonebook], str_len);
    fprintf(stdout, "Current phonebook = %s\n", str);
    ALOGV(LOGTAG "%s: Current phonebook: %s", __FUNCTION__, str);
    print_help(&variable_list[1]);
}

/**
 * set_format()
 *
 * This function sets the format variable used for determining which vCard
 * format to use.
 */
static void set_format(const OI_CHAR *str)
{
    OI_UINT i;

    OI_ASSERT(str);

    if (*str == '\0') {
        OI_Printf("format cannot be set to nothing\n");
        fprintf(stdout, "Format cannot be set to nothing \n");
        print_help(&variable_list[2]);
        return;
    }

    i = find_str_in_list(str, valid_formats, OI_ARRAYSIZE(valid_formats));

    if (i < OI_ARRAYSIZE(valid_formats)) {
        pbap_client.format = (OI_PBAP_FORMAT_TAG_VALUES)i;
        fprintf(stdout, "Format set to: %s\n", str);
    } else {
        fprintf(stdout, "Invalid format: %s\n", str);
        print_help(&variable_list[2]);
    }
}

/**
 * get_format()
 *
 * This function puts a string representation of the format's value into a
 * string buffer.
 */
static void get_format()
{
    OI_CHAR str[256];
    OI_UINT str_len = 256;

    strlcpy(str, valid_formats[pbap_client.format], str_len);
    fprintf(stdout, "Current format = %s\n", str);
    ALOGV(LOGTAG "%s: Current format: %s", __FUNCTION__, str);
    print_help(&variable_list[2]);
}

/**
 * set_filter()
 *
 * This function sets the filter variable used for determining which fields in
 * the vCard to use.
 */
static void set_filter(const OI_CHAR *str)
{
    OI_UINT i;
    OI_UINT32 filter = 0;

    OI_ASSERT(str);

    if (*str == '\0') {
        OI_Printf("filter cannot be set to nothing\n");
        fprintf(stdout, "Filter cannot be set to nothing \n");
        print_help(&variable_list[3]);
        return;
    }

    fprintf(stdout, "Setting Filter to: %s\n", str);
    while (*str != '\0') {
        i = find_str_in_list(str, valid_filters, OI_ARRAYSIZE(valid_filters));

        /* Check if filter is set to "all" */
        if (i == OI_ARRAYSIZE(valid_filters) - 1) {
            filter = 0;
            break;
        }

        if (i >= OI_ARRAYSIZE(valid_filters)) {
            fprintf(stdout, "Invalid filter values: %s\n", str);
            print_help(&variable_list[3]);
            return;
        }
        filter |= 1 << i;
        str += strlen(valid_filters[i]);
        str = skip_delimiter(str);
    }
    pbap_client.filter.I2 = filter;
}

/**
 * get_filter()
 *
 * This function puts a string representation of the filter's value into a
 * string buffer.
 */
static void get_filter()
{
    OI_UINT i;
    OI_INT pos = 0;
    OI_CHAR string[256];
    OI_CHAR *str = string;
    OI_UINT str_len = 256;

    memset(str, 0, sizeof(string));
    if (OI_PBAP_VCARD_ATTRIBUTE_CHECK_ALL(&pbap_client.filter)) {
        i = OI_ARRAYSIZE(valid_filters) - 1;
        strlcpy(str + pos, valid_filters[i], str_len);
        pos += strlen(valid_filters[i]) + 1;
    } else {
        for (i = 0; i < OI_ARRAYSIZE(valid_filters); i++) {
            if (pbap_client.filter.I2 & (1 << i)) {
                if (str_len > strlen(valid_filters[i])) {
                    strlcpy(str + pos, valid_filters[i], str_len);
                    str_len -= strlen(valid_filters[i]);
                    pos += strlen(valid_filters[i]);
                    str[pos] = ' ';
                    str_len--;
                    pos++;
                } else {
                    pos = 1;
                    break;
                }
            }
        }
    }
    str[pos - 1] = '\0';
    fprintf(stdout, "Current filter: %s\n", str);
    ALOGV(LOGTAG "%s: Current filter: %s", __FUNCTION__, str);
    print_help(&variable_list[3]);
}

/**
 * set_order()
 *
 * This function sets the order variable used for determining the sorting
 * order of phonebook entry listings.
 */
static void set_order(const OI_CHAR *str)
{
    OI_UINT i;
    OI_INT len;

    OI_ASSERT(str);

    if (*str == '\0') {
        OI_Printf("order cannot be set to nothing\n");
        fprintf(stdout, "Order cannot be set to nothing \n");
        print_help(&variable_list[4]);
        return;
    }

    for (i = 0; i < OI_ARRAYSIZE(valid_orders); i++) {
        len = 0;
        while (!isdelimiter(str[len]) && str[len] != '\0') {
            len++;
        }
        if (!OI_StrncmpInsensitive(valid_orders[i], str, (OI_UINT16)len)) {
            break;
        }
    }

    if (i < OI_ARRAYSIZE(valid_orders)) {
        pbap_client.order = (OI_PBAP_ORDER_TAG_VALUES)i;
        fprintf(stdout, "Order set to: %s\n", str);
    } else {
        fprintf(stdout, "Invalid order: %s\n", str);
        print_help(&variable_list[4]);
    }
}

/**
 * get_order()
 *
 * This function puts a string representation of the order's value into a
 * string buffer.
 */
static void get_order()
{
    OI_CHAR str[256];
    OI_UINT str_len = 256;

    strlcpy(str, valid_orders[pbap_client.order], str_len);
    fprintf(stdout, "Current search order = %s\n", str);
    ALOGV(LOGTAG "%s: Current search order: %s", __FUNCTION__, str);
    print_help(&variable_list[4]);
}

/**
 * set_search_value()
 *
 * This function sets the search variable used for determining the search
 * criterion of next ponebook fetch.
 */
static void set_search_value(const OI_CHAR *str)
{
    OI_UINT i;
    OI_INT len;

    OI_ASSERT(str);

    if (pbap_client.searchValue) {
        delete pbap_client.searchValue;
        pbap_client.searchValue = NULL;
        pbap_client.searchValueLen = 0;
    }
    pbap_client.searchValue = new OI_BYTE[strlen(str) + 1];
    strlcpy((OI_CHAR *)pbap_client.searchValue, str, strlen(str) + 1);
    pbap_client.searchValueLen = strlen(str);
}

/**
 * set_search_attribute()
 *
 * This function sets the search attribute for determining the search
 * criterion of phonebook entry listings.
 */
static void set_search_attribute(const OI_CHAR *str)
{
    OI_UINT i;
    OI_INT len;

    OI_ASSERT(str);

    if (*str == '\0') {
        OI_Printf("search attribute cannot be set to nothing\n");
        fprintf(stdout, "search attribute cannot be set to nothing \n");
        print_help(&variable_list[5]);
        return;
    }

    for (i = 0; i < OI_ARRAYSIZE(valid_search_attribs); i++) {
        len = 0;
        while (!isdelimiter(str[len]) && str[len] != '\0') {
            len++;
        }
        if (!OI_StrncmpInsensitive(valid_search_attribs[i], str, (OI_UINT16)len)) {
            break;
        }
    }

    if (i < OI_ARRAYSIZE(valid_search_attribs)) {
        pbap_client.searchAttribute = (OI_PBAP_SEARCH_ATTRIBUTE_TAG_VALUES)i;
        fprintf(stdout, "search attribute set to: %s\n", str);
    } else {
        fprintf(stdout, "Invalid search attribute: %s\n", str);
        print_help(&variable_list[5]);
    }
}

/**
 * get_search_attribute()
 *
 * This function puts a string representation of the search attribute's value into a
 * string buffer.
 */
static void get_search_attribute()
{
    OI_CHAR str[256];
    OI_UINT str_len = 256;

    strlcpy(str, valid_search_attribs[pbap_client.searchAttribute], str_len);
    fprintf(stdout, "Current search attribute = %s\n", str);
    ALOGV(LOGTAG "%s: Current search attribute: %s", __FUNCTION__, str);
    print_help(&variable_list[5]);
}

/**

/**
 * help_cmd_handler()
 *
 * This command handles the HELP command.  It displays useful help information
 * to the user.
 */
static void print_help(const PBAP_CLIENT_VARIABLE *var)
{
    int i;
    if (var) {
        printf("\n=====HELP=====\n%s:\t%s\n(valid options:", var->name, var->description);
        for (i = 0; i < (OI_UINT)var->valid_option_cnt; i++) {
            printf(" %s", var->valid_options[i]);
        }
        printf(")\n");
    }
}

void BtPbapClientMsgHandler(void *msg)
{
    BtEvent* event = NULL;
    bool status = false;
    if(!msg) {
        ALOGE(LOGTAG "%s: Msg is null, return", __FUNCTION__);
        return;
    }

    event = ( BtEvent *) msg;

    if (event == NULL) {
        ALOGE(LOGTAG "%s: event is null", __FUNCTION__);
        return;
    }

    ALOGD(LOGTAG "BtPbapClientMsgHandler event = %d", event->event_id);
    switch(event->event_id) {
        case PROFILE_API_START:
            {
                BtEvent *start_event = new BtEvent;
                memset(&pbap_client, 0, sizeof(PBAP_CLIENT_DATA));
                start_event->profile_start_event.event_id = PROFILE_EVENT_START_DONE;
                start_event->profile_start_event.profile_id = PROFILE_ID_PBAP_CLIENT;
                start_event->profile_start_event.status = true;
                PostMessage(THREAD_ID_GAP, start_event);
            }
            break;

        case PROFILE_API_STOP:
            if(g_pbapClient) {
                if (pbap_client.connection) {
                    pbap_client.state = STATE_DEINITIALIZING;
                    /* disconnect connected device */
                    OI_PBAPClient_Disconnect(pbap_client.connection);
                } else {
                    /* PBAP Client is not connected, proceed directly with record removal */
                    g_pbapClient->RemoveSdpRecord();
                    /*
                    * Its possible that SDP Client thread terminated before receiving above request,
                    * so send stop profile with success.
                    */
                    BtEvent *stop_event = new BtEvent;
                    stop_event->profile_start_event.event_id = PROFILE_EVENT_STOP_DONE;
                    stop_event->profile_start_event.profile_id = PROFILE_ID_PBAP_CLIENT;
                    stop_event->profile_start_event.status = true;
                    PostMessage(THREAD_ID_GAP, stop_event);
                }
            }
            break;

        default:
            if(g_pbapClient) {
               g_pbapClient->ProcessEvent(( BtEvent *) msg);
            }
            break;
    }
    delete event;
}

/******************************************************************************
*
* PBAP client bluetooth functionality.
*
*/

/**
* pb_open()
*
* This function gets called when the PBAP server is about to send phonebook
* related data, such as a listing, phonebook entry, or complete phonebook.
*/
static OI_STATUS pb_open(const OI_OBEX_UNICODE *name, OI_PBAP_OPEN_CFM open_cfm,
    OI_PBAP_CONNECTION connection)
{
    OI_STATUS status = OI_OK;
    FILE *stream = NULL;
    char filename[256];

    if (pbap_client.getting_listing) {
        snprintf(filename, sizeof(filename), "%s%s", storageDir, vCardListingFile);
    } else if (pbap_client.getting_phonebook) {
        snprintf(filename, sizeof(filename), "%s%s", storageDir, phoneBookFile);
    } else if (pbap_client.getting_vcard) {
        snprintf(filename, sizeof(filename), "%s%s", storageDir, vcardFile);
    } else {
        ALOGE(LOGTAG "unknown operation calling open");
        return OI_STATUS_INVALID_PARAMETERS;
    }
    pbap_client.fileName = new char[strlen(filename) + 1];
    if (pbap_client.fileName != NULL) {
        strlcpy(pbap_client.fileName, filename, strlen(filename) + 1);
        pbap_client.fileName[strlen(filename)] = '\0';
        ALOGD(LOGTAG "Opening file: %s\n", pbap_client.fileName);
        stream = fopen(pbap_client.fileName, "w+b");
        if (stream == NULL) {
            status = OI_STATUS_OUT_OF_MEMORY;
            delete pbap_client.fileName;
            pbap_client.fileName = NULL;
        }
    } else {
        status = OI_STATUS_OUT_OF_MEMORY;
    }
    open_cfm(stream, status, connection);
    return status;
}

/**
* pb_close()
*
* This function gets called when there is no more data coming from the PBAP
* server.
*/
static void pb_close(OI_PBAP_HANDLE handle, OI_PBAP_CONNECTION pbapConnection, OI_STATUS status)
{
    ALOGD(LOGTAG "Closed file with status: %d\n", status);

    if (handle != NULL)
        fclose((FILE *)handle);
    if (status && pbap_client.fileName != NULL)
        remove(pbap_client.fileName);
}

/**
* pb_write()
*
* This function is called when phonebook data has just been receieved from
* the PBAP server.
*/
static OI_STATUS pb_write(OI_PBAP_HANDLE handle, const OI_BYTE *buffer, OI_UINT16 buf_len,
    OI_PBAP_WRITE_CFM write_cfm, OI_PBAP_CONNECTION connection)
{
    OI_STATUS status = OI_OK;
    OI_UINT writeLen;

    if (pbap_client.abort) {
        ALOGD(LOGTAG "%s: operation aborted, not writing to file\n", __FUNCTION__);
        status = OI_STATUS_INTERNAL_ERROR;
        write_cfm(handle, status, connection);
        return status;
    }
    if (handle != NULL)
    {
        writeLen = fwrite(buffer, buf_len, 1, (FILE *)handle);
    }
    write_cfm(handle, status, connection);
    return status;
}

PbapClient :: PbapClient(const bt_interface_t *bt_interface, config_t *config)
{
    this->bluetooth_interface = bt_interface;
    this->config = config;
    pbap_connect_timer = NULL;
    if( !(pbap_connect_timer = alarm_new())) {
        ALOGE(LOGTAG, " unable to create pbap_connect_timer");
        return;
    }
}

void pbap_connect_timer_expired(void *context) {
    ALOGD(LOGTAG, " pbap_connect_timer_expired");

    BtEvent *event = new BtEvent;
    event->event_id = PBAP_CLIENT_CONNECT_TIMEOUT;
    memcpy(&event->pbap_client_event.bd_addr, (bt_bdaddr_t *)context, sizeof(bt_bdaddr_t));
    PostMessage(THREAD_ID_PBAP_CLIENT, event);
}

PbapClient :: ~PbapClient()
{
    alarm_free(g_pbapClient->pbap_connect_timer);
    g_pbapClient->pbap_connect_timer = NULL;
}

void connectionCfm(OI_PBAP_CONNECTION connectionId,
                                       OI_STATUS status)
{
    char bd_str[MAX_BD_STR_LEN];
    bdaddr_to_string((const bt_bdaddr_t*)&pbap_client.addr, bd_str, MAX_BD_STR_LEN);
    ALOGV(LOGTAG "%s: status: %d connectionID = %p", __FUNCTION__,
        status, connectionId);
    //stoping pbap_connect_timer
    if(g_pbapClient) {
        alarm_cancel(g_pbapClient->pbap_connect_timer);
    }

    if (pbap_client.state != STATE_CONNECTING) {
        ALOGE(LOGTAG "%s: received connect cfm in invalid state %d",
            __FUNCTION__, pbap_client.state);
        /* Most likely connection timed out, so disconnect OBEX link now */
        OI_PBAPClient_Disconnect(connectionId);
        return;
    }

    if (status == OI_STATUS_SUCCESS) {
        pbap_client.connection = connectionId;
        pbap_client.state = STATE_CONNECTED;
        /* Set default parameters */
        pbap_client.repository = OI_PBAP_LOCAL_REPOSITORY;
        pbap_client.phonebook = OI_PBAP_MAIN_PHONEBOOK;
        pbap_client.format = OI_PBAP_FORMAT_VCARD_2_1;
        pbap_client.order = OI_PBAP_ORDER_INDEXED;
        pbap_client.searchAttribute = OI_PBAP_SEARCH_ATTRIBUTE_NAME;
        pbap_client.start_offset = 0;
        pbap_client.max_list_count = OI_UINT16_MAX;
        if (pbap_client.searchValue) {
            delete pbap_client.searchValue;
            pbap_client.searchValue = NULL;
            pbap_client.searchValueLen = 0;
        }
        if (pbap_client.fileName != NULL) {
            delete pbap_client.fileName;
            pbap_client.fileName = NULL;
        }
        /* Set Filter to all */
        pbap_client.filter.I1 = 0;
        pbap_client.filter.I2 = 0;
        /* reset all operations to false */
        pbap_client.getting_vcard = false;
        pbap_client.getting_phonebook = false;
        pbap_client.getting_listing = false;
        pbap_client.set_path = false;
        pbap_client.abort = false;
        fprintf(stdout, "Connected to %s\n", bd_str );
    } else {
        pbap_client.connection = NULL;
        pbap_client.state = STATE_IDLE;
        uint8_t zero[sizeof(bt_bdaddr_t)] = { 0 };
        memcpy(&pbap_client.addr, zero, sizeof(bt_bdaddr_t));
        fprintf(stdout, "Failed to Connect to %s\n", bd_str );
    }
}

void disconnectInd(OI_PBAP_CONNECTION connectionId)
{
    ALOGV(LOGTAG "%s: connectionID = %p", __FUNCTION__, connectionId);

    if (pbap_client.state == STATE_DEINITIALIZING) {
        /* PBAP Client is not connected, proceed directly with record removal */
        g_pbapClient->RemoveSdpRecord();
        /*
        * Its possible that SDP Client thread terminated before receiving above request,
        * so send stop profile with success.
        */
        BtEvent *stop_event = new BtEvent;
        stop_event->profile_start_event.event_id = PROFILE_EVENT_STOP_DONE;
        stop_event->profile_start_event.profile_id = PROFILE_ID_PBAP_CLIENT;
        stop_event->profile_start_event.status = true;
        PostMessage(THREAD_ID_GAP, stop_event);
    }

    char bd_str[MAX_BD_STR_LEN];
    if (pbap_client.connection == connectionId) {
        bdaddr_to_string((const bt_bdaddr_t*)&pbap_client.addr, bd_str, MAX_BD_STR_LEN);
        fprintf(stdout, "Disconnected with %s\n", bd_str );
        pbap_client.connection = NULL;
        pbap_client.state = STATE_IDLE;
        if (pbap_client.searchValue) {
            delete pbap_client.searchValue;
            pbap_client.searchValue = NULL;
            pbap_client.searchValueLen = 0;
        }
        if (pbap_client.fileName != NULL) {
            delete pbap_client.fileName;
            pbap_client.fileName = NULL;
        }
        pbap_client.getting_vcard = false;
        pbap_client.getting_phonebook = false;
        pbap_client.getting_listing = false;
        pbap_client.set_path = false;
        pbap_client.abort = false;
        uint8_t zero[sizeof(bt_bdaddr_t)] = { 0 };
        memcpy(&pbap_client.addr, zero, sizeof(bt_bdaddr_t));
    }
}

void authenticationCB(OI_PBAP_CONNECTION connectionId, OI_BOOL userIdRequired)
{
    OI_CHAR userid_buf[OI_OBEX_MAX_USERID_LEN + 1] = "userid";
    OI_CHAR password_buf[OI_OBEX_MAX_PASSWORD_LEN + 1] = "password";
    OI_UINT8 useridLen = strlen("userid");
    OI_STATUS status;
    fprintf(stdout, "Authentication requested by remote device\n");
    ALOGV(LOGTAG "%s: connectionID = %p, userIdRequired = %d",
        __FUNCTION__, connectionId, userIdRequired);
    if (userIdRequired) {
        OI_Printf("Authenticating with userid=%s and password=%s\n",
                  userid_buf, password_buf);
        status = OI_PBAPClient_AuthenticationRsp(connectionId,
                                               (OI_BYTE*)userid_buf,
                                               useridLen,
                                               password_buf);
    } else {
        OI_Printf("Authenticating with password=%s\n",
                  password_buf);
        status = OI_PBAPClient_AuthenticationRsp(connectionId,
                                                 NULL,
                                                 0,
                                                 password_buf);
    }
    if (!OI_SUCCESS(status)) {
        OI_Printf("Failed to send authentication response: %!\n", status);
    }
}

void pullPhonebookSizeCB(OI_PBAP_CONNECTION connectionId,
                                                     OI_UINT16 phonebookSize,
                                                     OI_STATUS status)
{
    ALOGV(LOGTAG "%s: connectionID = %p, phonebookSize = %d status = %d",
        __FUNCTION__, connectionId, phonebookSize, status);
    if (status == OI_STATUS_SUCCESS) {
        fprintf(stdout, "Phonebook Size is %d\n", phonebookSize);
    } else {
        fprintf(stdout, "Get Phonebook Size failed!!\n");
    }
    pbap_client.getting_phonebook = false;
}

void pullPhonebookCB(OI_PBAP_CONNECTION connectionId,
                                                 OI_UINT8 newMissedCalls,
                                                 OI_STATUS status)
{
    ALOGV(LOGTAG "%s: connectionID = %p, newMissedCalls = %d status = %d",
        __FUNCTION__, connectionId, newMissedCalls, status);
    if (status == OI_STATUS_SUCCESS) {
        ALOGD(LOGTAG "Phonebook downloaded to %s", pbap_client.fileName);
        fprintf(stdout, "Phonebook downloaded to %s\n", pbap_client.fileName);
    } else {
        ALOGD(LOGTAG "Download Phonebook failed, status %d ", status);
        fprintf(stdout, "Download Phonebook failed!!\n");
    }
    pbap_client.getting_phonebook = false;
    if (pbap_client.fileName != NULL) {
        delete pbap_client.fileName;
        pbap_client.fileName = NULL;
    }
}

void pullVcardCB(OI_PBAP_CONNECTION connectionId, OI_STATUS status)
{
    ALOGV(LOGTAG "%s: connectionID = %p, status = %d",
        __FUNCTION__, connectionId, status);
    if (status == OI_STATUS_SUCCESS) {
        ALOGD(LOGTAG "Vcard downloaded to %s ", pbap_client.fileName);
        fprintf(stdout, "Vcard downloaded to %s \n", pbap_client.fileName);
    } else {
        ALOGD(LOGTAG "Vcard Download failed, status %d ", status);
        fprintf(stdout, "Vcard Download failed!!\n");
    }
    pbap_client.getting_vcard = false;
    if (pbap_client.fileName != NULL) {
        delete pbap_client.fileName;
        pbap_client.fileName = NULL;
    }
}

void pullPVcardListingCB(OI_PBAP_CONNECTION connectionId,
                                                 OI_UINT8 newMissedCalls,
                                                 OI_STATUS status)
{
    ALOGV(LOGTAG "%s: connectionID = %p, newMissedCalls = %d status = %d",
        __FUNCTION__, connectionId, newMissedCalls, status);
    if (status == OI_STATUS_SUCCESS) {
        ALOGD(LOGTAG "Vcard Listing downloaded to %s ", pbap_client.fileName);
        fprintf(stdout, "Vcard Listing downloaded to %s\n ", pbap_client.fileName);
    } else {
        ALOGD(LOGTAG "Vcard Listing failed, status %d ", status);
        fprintf(stdout, "Vcard Listing failed!!\n");
    }
    pbap_client.getting_listing = false;
    if (pbap_client.fileName != NULL) {
        delete pbap_client.fileName;
        pbap_client.fileName = NULL;
    }
    if (pbap_client.searchValue) {
        delete pbap_client.searchValue;
        pbap_client.searchValue = NULL;
        pbap_client.searchValueLen = 0;
    }
}

void setPathCB (OI_PBAP_CONNECTION connectionId, OI_STATUS status)
{
    if (status == OI_STATUS_SUCCESS) {
        ALOGD(LOGTAG "set path succeeded");
        fprintf(stdout, "set path succeeded\n");
    } else {
        ALOGD(LOGTAG "set path failed, status %d ", status);
        fprintf(stdout, "set path failed!!\n");
    }
    pbap_client.set_path = false;
}

void abortCfm(OI_PBAP_CONNECTION connectionId)
{
    ALOGV(LOGTAG "%s: connectionID = %p", __FUNCTION__, connectionId);
    fprintf(stdout, "Aborted last operation \n");
    pbap_client.getting_vcard = false;
    pbap_client.getting_phonebook = false;
    pbap_client.getting_listing = false;
    pbap_client.set_path = false;
    pbap_client.abort = false;
}

static void sdp_add_record_callback(bt_status_t status, int handle)
{
    if (status) {
        fprintf(stdout, "Sdp add record failed can't proceed with connection\n");
        ALOGE(LOGTAG "%s: sdp add record failed, status %d", __FUNCTION__, status);
    } else {
        pbap_client.rec_handle = handle;
        pbap_client.state = STATE_IDLE;
        fprintf(stdout, "Successfully Registered PBAP Client SDP Record\n");
    }
}

static void sdp_remove_record_callback(bt_status_t status)
{
    if (status) {
        ALOGE(LOGTAG "%s: sdp remove record failed, status %d", __FUNCTION__, status);
        return;
    }
    ALOGV(LOGTAG "%s", __FUNCTION__);
    pbap_client.rec_handle = -1;
    pbap_client.state = STATE_INITIAL;
}

static void sdp_search_callback(bt_status_t status, bt_bdaddr_t *bd_addr, uint8_t* uuid,
            bluetooth_sdp_record *record, bool more_result)
{
    char bd_str[MAX_BD_STR_LEN];

    if (status) {
        ALOGE(LOGTAG "%s: sdp search failed, status %d", __FUNCTION__, status);
        fprintf(stdout, "Sdp search failed can't proceed with connection\n");
        return;
    }
    ALOGE(LOGTAG "%s", __FUNCTION__);
    if (IS_UUID(UUID_PBAP_PSE, uuid)) {
        bdaddr_to_string(bd_addr, bd_str, MAX_BD_STR_LEN);
        ALOGE(LOGTAG "%s: status %d, addr %s, L2CAP PSM = %d, "
            "RFCOMM channel = %d, profile version = 0x%04x, "
            "supported features = %d, repositories = %d, "
            "more results = %d", __FUNCTION__, status, bd_str,
            record->pse.hdr.l2cap_psm,
            record->pse.hdr.rfcomm_channel_number,
            record->pse.hdr.profile_version,
            record->pse.supported_features,
            record->pse.supported_repositories,
            more_result);
        if (record->pse.hdr.rfcomm_channel_number > 0 ||
            record->pse.hdr.l2cap_psm > 0) {
            if (record->pse.hdr.l2cap_psm > 0 &&
                record->pse.hdr.profile_version >= profileVersion) {
                pbap_client.lowerProtocol.protocol = OI_OBEX_LOWER_L2CAP;
                pbap_client.lowerProtocol.svcId.l2capPSM = record->pse.hdr.l2cap_psm;
            } else {
                pbap_client.lowerProtocol.protocol = OI_OBEX_LOWER_RFCOMM;
                pbap_client.lowerProtocol.svcId.rfcommChannel =
                    record->pse.hdr.rfcomm_channel_number;
            }
            pbap_client.supportedFeatures = DOWNLOAD | BROWSING;
            /* Send Internal Connect Message to PBAP Client Thread */
            BtEvent *event = new BtEvent;
            event->event_id = PBAP_CLIENT_INTERNAL_CONNECT;
            PostMessage(THREAD_ID_PBAP_CLIENT, event);
        } else {
            ALOGE(LOGTAG "%s: Could not find remote rfcomm channel or l2cap psm, can't connect",
                 __FUNCTION__);
            fprintf(stdout, "Could not find remote rfcomm channel or l2cap psm, can't connect\n");
        }
    } else {
        ALOGE(LOGTAG "%s: Unknown uuid sdp result received, ignoring!!", __FUNCTION__);
        fprintf(stdout, "Unknown uuid sdp result received, ignoring!!\n");
    }
}

void PbapClient::ProcessEvent(BtEvent* pEvent)
{
    ALOGD(LOGTAG "%s: Processing event %d", __FUNCTION__, pEvent->event_id);

    switch(pEvent->event_id) {

        case PBAP_CLIENT_REGISTER:
            AddSdpRecord();
            break;

        case PBAP_CLIENT_CONNECT:
            PerformSdp(&pEvent->pbap_client_event.bd_addr);
            break;

        case PBAP_CLIENT_INTERNAL_CONNECT:
            Connect();
            break;

        case PBAP_CLIENT_CONNECT_TIMEOUT:
            HandleConnectTimeout(&pEvent->pbap_client_event.bd_addr);
            break;

        case PBAP_CLIENT_DISCONNECT:
            Disconnect(&pEvent->pbap_client_event.bd_addr);
            break;

        case PBAP_CLIENT_ABORT:
            Abort();
            break;

        case PBAP_CLIENT_GET_PHONEBOOK_SIZE:
            GetPhonebookSize();
            break;

        case PBAP_CLIENT_GET_PHONEBOOK:
            GetPhonebook();
            break;

        case PBAP_CLIENT_GET_VCARD:
            GetVcard(pEvent->pbap_client_event.value);
            break;

        case PBAP_CLIENT_GET_VCARD_LISTING:
            GetVcardListing();
            break;

        case PBAP_CLIENT_SET_PATH:
            SetPath(pEvent->pbap_client_event.value);
            break;

        case PBAP_CLIENT_SET_FILTER:
            set_filter(pEvent->pbap_client_event.value);
            break;

        case PBAP_CLIENT_GET_FILTER:
            get_filter();
            break;

        case PBAP_CLIENT_SET_ORDER:
            set_order(pEvent->pbap_client_event.value);
            break;

        case PBAP_CLIENT_GET_ORDER:
            get_order();
            break;

        case PBAP_CLIENT_SET_SEARCH_ATTRIBUTE:
            set_search_attribute(pEvent->pbap_client_event.value);
            break;

        case PBAP_CLIENT_GET_SEARCH_ATTRIBUTE:
            get_search_attribute();
            break;

        case PBAP_CLIENT_SET_SEARCH_VALUE:
            set_search_value(pEvent->pbap_client_event.value);
            break;

        case PBAP_CLIENT_SET_PHONE_BOOK:
            set_phonebook(pEvent->pbap_client_event.value);
            break;

        case PBAP_CLIENT_GET_PHONE_BOOK:
            get_phonebook();
            break;

        case PBAP_CLIENT_SET_REPOSITORY:
            set_repository(pEvent->pbap_client_event.value);
            break;

        case PBAP_CLIENT_GET_REPOSITORY:
            get_repository();
            break;

        case PBAP_CLIENT_SET_VCARD_FORMAT:
            set_format(pEvent->pbap_client_event.value);
            break;

        case PBAP_CLIENT_GET_VCARD_FORMAT:
            get_format();
            break;

        case PBAP_CLIENT_SET_LIST_COUNT:
            pbap_client.max_list_count = pEvent->pbap_client_event.max_list_count;
            fprintf(stdout, "Max List set to =: %d\n", pbap_client.max_list_count);
            break;

        case PBAP_CLIENT_GET_LIST_COUNT:
            ALOGV(LOGTAG "%s: Max List Count =: %d", __FUNCTION__, pbap_client.max_list_count);
            fprintf(stdout, " Max List Count =: %d\n", pbap_client.max_list_count);
            break;

        case PBAP_CLIENT_SET_START_OFFSET:
            pbap_client.start_offset = pEvent->pbap_client_event.list_start_offset;
            fprintf(stdout, "List Start set to %d\n", pbap_client.start_offset);
            break;

        case PBAP_CLIENT_GET_START_OFFSET:
            ALOGV(LOGTAG "%s: List Start Offset =: %d", __FUNCTION__, pbap_client.start_offset);
            fprintf(stdout, "List Start Offset =: %d\n", pbap_client.start_offset);
            break;

        default:
            ALOGW(LOGTAG "%s: unhandled event: %d", __FUNCTION__, pEvent->event_id);
            break;
    }
}

void PbapClient :: AddSdpRecord()
{
    if (pbap_client.state >= STATE_IDLE) {
        fprintf(stdout, "Already registered \n");
        return;
    }
    /* Add PBAP Client SDP Record */
    BtEvent *sdp_search_event = new BtEvent;
    sdp_search_event->sdp_client_event.event_id = SDP_CLIENT_ADD_RECORD;
    memset(&sdp_search_event->sdp_client_event.record, 0 , sizeof(bluetooth_sdp_record));
    sdp_search_event->sdp_client_event.record.pce.hdr.type = SDP_TYPE_PBAP_PCE;
    sdp_search_event->sdp_client_event.record.pce.hdr.profile_version = profileVersion;
    sdp_search_event->sdp_client_event.record.pce.hdr.service_name = profile_name;
    sdp_search_event->sdp_client_event.record.pce.hdr.service_name_length = strlen(profile_name);
    sdp_search_event->sdp_client_event.addRecordCb = &sdp_add_record_callback;
    PostMessage(THREAD_ID_SDP_CLIENT, sdp_search_event);
}

void PbapClient :: RemoveSdpRecord()
{
    if (pbap_client.state == STATE_INITIAL) {
        ALOGE(LOGTAG "Already un-registered ");
        return;
    }
    if (pbap_client.searchValue) {
        delete pbap_client.searchValue;
        pbap_client.searchValue = NULL;
        pbap_client.searchValueLen = 0;
    }
    if (pbap_client.rec_handle != -1) {
        /* Remove PBAP Client SDP Record */
        BtEvent *sdp_search_event = new BtEvent;
        sdp_search_event->sdp_client_event.event_id = SDP_CLIENT_REMOVE_RECORD;
        sdp_search_event->sdp_client_event.removeRecordCb = &sdp_remove_record_callback;
        sdp_search_event->sdp_client_event.rec_handle = pbap_client.rec_handle;
        PostMessage(THREAD_ID_SDP_CLIENT, sdp_search_event);
    }
}

bool PbapClient :: PerformSdp(bt_bdaddr_t *addr)
{
    bool ret = true;
    char bd_str[MAX_BD_STR_LEN];

    if (!addr) {
        ALOGE(LOGTAG "%s: Bluetooth device address null", __FUNCTION__);
        return false;
    }
    bdaddr_to_string((const bt_bdaddr_t*)&pbap_client.addr, bd_str, MAX_BD_STR_LEN);
    if (pbap_client.state == STATE_INITIAL) {
        ALOGE(LOGTAG "%s: Not registered, please register before connecting!!", __FUNCTION__);
        fprintf(stdout, "Not registered, please register before connecting!!\n");
        return false;
    } else if (pbap_client.state == STATE_CONNECTING) {
        ALOGE(LOGTAG "%s: Currently connecting to %s", __FUNCTION__, bd_str);
        fprintf(stdout, "Currently connecting to %s\n", bd_str);
        return false;
    } else if (pbap_client.state == STATE_CONNECTED) {
        ALOGE(LOGTAG "%s: Already connected to %s", __FUNCTION__, bd_str);
        fprintf(stdout, "Already connected to %s\n", bd_str);
        return false;
    }
    memcpy(&pbap_client.addr, addr, sizeof(bt_bdaddr_t));
    bdaddr_to_string(addr, bd_str, MAX_BD_STR_LEN);
    ALOGV(LOGTAG "%s: %s", __FUNCTION__, bd_str);
    BtEvent *sdp_search_event = new BtEvent;
    /* Perform SDP to determine PBAP Server Record */
    sdp_search_event->sdp_client_event.event_id = SDP_CLIENT_SEARCH;
    memcpy(&sdp_search_event->sdp_client_event.bd_addr, &pbap_client.addr, sizeof(bt_bdaddr_t));
    sdp_search_event->sdp_client_event.uuid = UUID_PBAP_PSE;
    sdp_search_event->sdp_client_event.searchCb = &sdp_search_callback;
    PostMessage(THREAD_ID_SDP_CLIENT, sdp_search_event);

    return ret;
}

bool PbapClient :: Connect()
{
    OI_STATUS ret = OI_PBAPClient_Connect((OI_BD_ADDR*)&pbap_client.addr,
        &pbap_client.lowerProtocol, OI_OBEXCLI_AUTH_NONE,
        &pbap_client.connection, pbap_client.supportedFeatures,
        &connectionCfm, &disconnectInd, &authenticationCB, &pbap_file_ops);
    ALOGV(LOGTAG "%s: OI_PBAPClient_Connect returned %d", __FUNCTION__, ret);
    if (ret) {
        char bd_str[MAX_BD_STR_LEN];
        bdaddr_to_string((const bt_bdaddr_t*)&pbap_client.addr, bd_str, MAX_BD_STR_LEN);
        ALOGE(LOGTAG "%s: Failed to connect to %s", __FUNCTION__, bd_str);
        fprintf(stdout, "Failed to connect to %s\n", bd_str);
    } else {
         // start the profile connect timer
        alarm_set(pbap_connect_timer, PBAP_CONNECT_TIMEOUT_DELAY,
                            pbap_connect_timer_expired, &pbap_client.addr);
        pbap_client.state = STATE_CONNECTING;
    }
    return ret;
}

bool PbapClient :: HandleConnectTimeout(bt_bdaddr_t *addr)
{
    char bd_str[MAX_BD_STR_LEN];
    bdaddr_to_string((const bt_bdaddr_t*)addr, bd_str, MAX_BD_STR_LEN);
    fprintf(stdout, "Failed to Connect to %s due to ConnectionTimeout\n", bd_str);
    pbap_client.connection = NULL;
    pbap_client.state = STATE_IDLE;
    uint8_t zero[sizeof(bt_bdaddr_t)] = { 0 };
    memcpy(&pbap_client.addr, zero, sizeof(bt_bdaddr_t));
    return true;
}

bool PbapClient :: Disconnect(bt_bdaddr_t *addr)
{
    bool ret = true;
    char bd_str[MAX_BD_STR_LEN];

    if (!addr) {
        ALOGE(LOGTAG "%s: Bluetooth device address null", __FUNCTION__);
        return false;
    }
    bdaddr_to_string(addr, bd_str, MAX_BD_STR_LEN);
    if (pbap_client.state != STATE_CONNECTED || !bdaddr_equals(addr,
            (const bt_bdaddr_t *)&pbap_client.addr)) {
        ALOGE(LOGTAG "%s: not connected to %s", __FUNCTION__, bd_str);
        fprintf(stdout, "Not connected to %s\n", bd_str);
        return false;
    }
    ALOGV(LOGTAG "%s: %s", __FUNCTION__, bd_str);

    OI_STATUS status = OI_PBAPClient_Disconnect(pbap_client.connection);
    if (status != OI_STATUS_SUCCESS) {
        ret = false;
        char bd_str[MAX_BD_STR_LEN];
        bdaddr_to_string((const bt_bdaddr_t*)&pbap_client.addr, bd_str, MAX_BD_STR_LEN);
        fprintf(stdout, "Failed to disconnect to %s\n", bd_str);
        ALOGE(LOGTAG "%s: Failed disconnect %s status: %d", __FUNCTION__, bd_str, status);
    }
    return ret;
}

bool PbapClient :: GetPhonebookSize()
{
    bool ret = true;

    ALOGV(LOGTAG "%s", __FUNCTION__);
    if (pbap_client.getting_vcard || pbap_client.getting_phonebook
            || pbap_client.getting_listing || pbap_client.set_path
            || pbap_client.abort) {
        fprintf(stdout, "PBAP Operation in progress, please retry again!!\n");
        return false;
    }
    if (pbap_client.state != STATE_CONNECTED) {
        ALOGE(LOGTAG "%s: Not connected", __FUNCTION__);
        fprintf(stdout, "Not connected, can't get phonebook size\n");
        return false;
    }
    OI_STATUS status = OI_PBAPClient_GetPhonebookSize(pbap_client.connection,
                        pbap_client.repository, pbap_client.phonebook,
                        &pullPhonebookSizeCB);
    if (status != OI_STATUS_SUCCESS) {
        ALOGE(LOGTAG "%s: Failed get phonebook size, status: %d", __FUNCTION__, status);
        fprintf(stdout, "Failed get phonebook size, err: %d\n", status);
        ret = false;
    } else {
        pbap_client.getting_phonebook = true;
        fprintf(stdout, "Get Phonebook size in progress!!\n");
    }
    return ret;
}

bool PbapClient :: GetPhonebook()
{
    bool ret = true;

    ALOGV(LOGTAG "%s", __FUNCTION__);
    if (pbap_client.getting_vcard || pbap_client.getting_phonebook
            || pbap_client.getting_listing || pbap_client.set_path
            || pbap_client.abort) {
        fprintf(stdout, "PBAP Operation in progress, please retry again!!\n");
        return false;
    }
    if (pbap_client.state != STATE_CONNECTED) {
        ALOGE(LOGTAG "%s: Not connected", __FUNCTION__);
        fprintf(stdout, "Not connected, can't download\n");
        return false;
    }
    pbap_client.getting_phonebook = true;
    OI_STATUS status = OI_PBAPClient_PullPhonebook(pbap_client.connection,
                        pbap_client.repository, pbap_client.phonebook,
                        &pbap_client.filter, pbap_client.format,
                        pbap_client.max_list_count, pbap_client.start_offset,
                        &pullPhonebookCB);
    if (status != OI_STATUS_SUCCESS) {
        ALOGE(LOGTAG "%s: Failed pull phonebook, status: %d", __FUNCTION__, status);
        fprintf(stdout, "Pull Phonebook failed, err: %d\n", status);
        pbap_client.getting_phonebook = false;
        ret = false;
    } else {
        fprintf(stdout, "Pull Phonebook in progress!!\n");
    }
    return ret;
}

bool PbapClient :: GetVcard(const OI_CHAR *handle)
{
    bool ret = true;
    OI_UINT32 number = 0;
    const OI_CHAR *dp = handle;

    ALOGV(LOGTAG "%s", __FUNCTION__);
    if (pbap_client.getting_vcard || pbap_client.getting_phonebook
            || pbap_client.getting_listing || pbap_client.set_path
            || pbap_client.abort) {
        fprintf(stdout, "PBAP Operation in progress, please retry again!!\n");
        return false;
    }
    if (pbap_client.state != STATE_CONNECTED) {
        ALOGE(LOGTAG "%s: Not connected", __FUNCTION__);
        fprintf(stdout, "Not connected, can't download\n");
        return false;
    }
    while (*dp != '\0' && ishexdigit(*dp) && (number < OI_UINT32_MAX)) {
        number *= 16;
        if((*dp >= 'a') && (*dp <= 'f')) {
            number += (10 + (*dp - 'a'));
        } else if ((*dp >= 'A') && (*dp <= 'F')) {
            number += (10 + (*dp - 'A'));
        } else {
            number += (*dp - '0');
        }
        dp++;
    }
    pbap_client.getting_vcard = true;
    OI_STATUS status = OI_PBAPClient_PullvCardEntry(pbap_client.connection,
                        pbap_client.repository, pbap_client.phonebook, number,
                        &pbap_client.filter, pbap_client.format, &pullVcardCB);
    if (status != OI_STATUS_SUCCESS) {
        ALOGE(LOGTAG "%s: Failed pull vcard, status: %d", __FUNCTION__, status);
        fprintf(stdout, "Pull Vcard failed, err: %d\n", status);
        pbap_client.getting_vcard = false;
        ret = false;
    } else {
        fprintf(stdout, "Pull Vcard in progress!!\n");
    }
    return ret;
}

bool PbapClient :: GetVcardListing()
{
    bool ret = true;

    ALOGV(LOGTAG "%s", __FUNCTION__);
    if (pbap_client.getting_vcard || pbap_client.getting_phonebook
            || pbap_client.getting_listing || pbap_client.set_path
            || pbap_client.abort) {
        fprintf(stdout, "PBAP Operation in progress, please retry again!!\n");
        return false;
    }
    if (pbap_client.state != STATE_CONNECTED) {
        ALOGE(LOGTAG "%s: Not connected", __FUNCTION__);
        fprintf(stdout, "Not connected, can't download\n");
        return false;
    }
    pbap_client.getting_listing = true;
    OI_STATUS status = OI_PBAPClient_PullvCardListing(pbap_client.connection,
                        pbap_client.repository, pbap_client.phonebook,
                        pbap_client.order, pbap_client.searchAttribute,
                        pbap_client.searchValue, pbap_client.searchValueLen,
                        pbap_client.max_list_count, pbap_client.start_offset,
                        &pullPVcardListingCB);
    if (status != OI_STATUS_SUCCESS) {
        ALOGE(LOGTAG "%s: Failed pull vcard, status: %d", __FUNCTION__, status);
        fprintf(stdout, "Pull Vcard listing failed, err: %d\n", status);
        pbap_client.getting_listing = false;
        ret = false;
    } else {
        fprintf(stdout, "Pull Vcard Listing in progress!!\n");
    }
    return ret;
}

bool PbapClient :: SetPath(const OI_CHAR *str)
{
    OI_UINT i;
    OI_PBAP_REPOSITORY rep = OI_PBAP_INVALID_REPOSITORY;
    OI_PBAP_PHONEBOOK pb = OI_PBAP_INVALID_PHONEBOOK;

    if (pbap_client.getting_vcard || pbap_client.getting_phonebook
            || pbap_client.getting_listing || pbap_client.set_path
            || pbap_client.abort) {
        fprintf(stdout, "PBAP Operation in progress, please retry again!!\n");
        return false;
    }
    if (pbap_client.state != STATE_CONNECTED) {
        ALOGE(LOGTAG "%s: Not connected", __FUNCTION__);
        fprintf(stdout, "Not connected, can't perform setpath\n");
        return false;
    }
    OI_ASSERT(str);

    if (*str == '\0') {
        OI_Printf("path cannot be set to nothing\n");
        fprintf(stdout, "path cannot be set to nothing \n");
        print_help(&variable_list[6]);
        return false;
    }

    i = find_str_in_list(str, valid_paths, OI_ARRAYSIZE(valid_paths));

    if (i < OI_ARRAYSIZE(valid_paths)) {
        fprintf(stdout, "path set to: %s\n", str);
    } else {
        fprintf(stdout, "Invalid format: %s\n", str);
        print_help(&variable_list[6]);
    }
    switch (i) {
        case 0:
            rep = OI_PBAP_LOCAL_REPOSITORY;
            break;
        case 1:
            rep = OI_PBAP_LOCAL_REPOSITORY;
            pb = OI_PBAP_MAIN_PHONEBOOK;
            break;
        case 2:
            rep = OI_PBAP_LOCAL_REPOSITORY;
            pb = OI_PBAP_INCOMING_CALLS_HISTORY;
            break;
        case 3:
            rep = OI_PBAP_LOCAL_REPOSITORY;
            pb = OI_PBAP_OUTGOING_CALLS_HISTORY;
            break;
        case 4:
            rep = OI_PBAP_LOCAL_REPOSITORY;
            pb = OI_PBAP_MISSED_CALLS_HISTORY;
            break;
        case 5:
            rep = OI_PBAP_LOCAL_REPOSITORY;
            pb = OI_PBAP_COMBINED_CALLS_HISTORY;
            break;
        case 6:
            rep = OI_PBAP_SIM1_REPOSITORY;
            break;
        case 7:
            rep = OI_PBAP_SIM1_REPOSITORY;
            pb = OI_PBAP_MAIN_PHONEBOOK;
            break;
        case 8:
            rep = OI_PBAP_SIM1_REPOSITORY;
            pb = OI_PBAP_INCOMING_CALLS_HISTORY;
            break;
        case 9:
            rep = OI_PBAP_SIM1_REPOSITORY;
            pb = OI_PBAP_OUTGOING_CALLS_HISTORY;
            break;
        case 10:
            rep = OI_PBAP_SIM1_REPOSITORY;
            pb = OI_PBAP_MISSED_CALLS_HISTORY;
            break;
        case 11:
            rep = OI_PBAP_SIM1_REPOSITORY;
            pb = OI_PBAP_COMBINED_CALLS_HISTORY;
            break;
        default:
            break;
    }
    pbap_client.set_path = true;
    OI_STATUS status = OI_PBAPClient_SetPath(pbap_client.connection, rep, pb, &setPathCB);
    if (status != OI_STATUS_SUCCESS) {
        ALOGE(LOGTAG "%s: Set path, status: %d", __FUNCTION__, status);
        fprintf(stdout, "Set path failed, err:  %d\n", status);
        pbap_client.set_path = false;
        return false;
    } else {
        fprintf(stdout, "Set path in progress!!\n");
        return true;
    }
}

bool PbapClient :: Abort()
{
    bool ret = true;

    ALOGV(LOGTAG "%s", __FUNCTION__);
    if (pbap_client.state != STATE_CONNECTED) {
        ALOGE(LOGTAG "%s: Not connected", __FUNCTION__);
        fprintf(stdout, "Not connected, can't perform abort\n");
        return false;
    }
    if (!(pbap_client.getting_vcard || pbap_client.getting_phonebook ||
        pbap_client.getting_listing)) {
        ALOGE(LOGTAG "%s: no operation ongoing", __FUNCTION__);
        fprintf(stdout, "No operation ongoing, can't perform abort\n");
        return false;
    }
    pbap_client.abort = true;
    OI_STATUS status = OI_PBAPClient_Abort(pbap_client.connection, &abortCfm);
    if (status != OI_STATUS_SUCCESS) {
        ALOGE(LOGTAG "%s: Failed to abort ongoing operation, status: %d", __FUNCTION__, status);
        pbap_client.abort = false;
        ret = false;
    } else {
        fprintf(stdout, "Abort in progress!!\n");
    }
    return ret;
}

#ifdef __cplusplus
}
#endif


