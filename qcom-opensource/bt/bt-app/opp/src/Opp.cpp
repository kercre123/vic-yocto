/*
 * Copyright (c) 2017, The Linux Foundation. All rights reserved.
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
#include "Opp.hpp"
#include "utils.h"
#include "oi_obex.h"
#include "oi_obex_lower.h"
#include "oi_pbap_client.h"
#include "oi_utils.h"
#include "oi_assert.h"
#include <string.h>
#include "oi_memmgr.h"
#include "oi_opp_client.h"
#include "oi_opp_server.h"
#include "oi_opp_sys.h"
#include "oi_unicode.h"
#include "oi_rfcomm_prefs.h"
#include "oi_l2cap_prefs.h"
#include "oi_unicode.h"
#include <unistd.h>

#ifdef __cplusplus
extern "C"
{
#endif

#define LOGTAG "OPP "

using namespace std;


static uint8_t  UUID_OBEX_OBJECT_PUSH[] = {0x00, 0x00, 0x11, 0x05, 0x00, 0x00, 0x10, 0x00,
                                                 0x80, 0x00, 0x00, 0x80, 0x5F, 0x9B, 0x34, 0xFB};
static  uint32_t profileVersion = 0x0102;
static char profile_name[] = "OPP Profile";
static char storageDir[] = "/data/misc/bluetooth/";
static char noNameFileStoreLocation[] = "/data/misc/bluetooth/NoName";
static char configFileName[] = "/data/misc/bluetooth/ext_to_mimetype.conf";
Opp *g_opp = NULL;;

#define UUID_MAX_LENGTH 16
#define IS_UUID(u1,u2)  !memcmp(u1,u2,UUID_MAX_LENGTH)
#define MAX_NAME_LEN                        (256)
#define MAX_STORED_OBJECT_SIZE  1000
#define MAX_MIME_TYE_LIST_LEN 500
#define CONFIG_LINE_BUFFER_SIZE 100

/*
 * TRUE if contrary to the specification the remote peer did not provide a name header
 */
static OI_BOOL rejectPush = FALSE;
OI_OPP_CLIENT_CONNECTION_HANDLE overrideConnection;
static OI_BOOL  serverAllowPull = FALSE;
static OI_BOOL  serverAllowPush = TRUE;
static OI_BOOL  reportProgress = FALSE;

/******************************************************************************
 * Forward struct typedef
 */
typedef struct opp_data_struct OPP_DATA;
typedef struct file_ext_type FILE_TYPE_DATA;

typedef enum {
    STATE_INITIAL,        /**< Initial state. */
    STATE_IDLE,           /**< Idle state. */
    STATE_CONNECTING,     /**< Connecting state. */
    STATE_CONNECTED,      /**< Connected state. */
    STATE_SENDING,        /**< Sending state. */
    STATE_RECEIVING,      /**< Receiving state. */
    STATE_DEINITIALIZING, /**< De-initializing state. */
} OPP_STATE;

struct file_ext_type {
    OI_CHAR ext[MAX_NAME_LEN];                             /**< Extension of data being transferred */
    OI_CHAR mimeType[MAX_NAME_LEN];                             /**< Mime Type of data being transferred */
};

/**
This structure defines the OPP client data structure.
*/
struct opp_data_struct {
    int rec_handle;                                 /**< Record Handle for OPP SDP */
    OI_OPP_CLIENT_CONNECTION_HANDLE clientConnectionHandle;     /**< Connection ID to a server */
    OI_OPP_SERVER_CONNECTION_HANDLE srvConnectionHandle; /**< Connection ID to a client */
    OI_OPP_SERVER_HANDLE serverHandle; /**< Server handle */
    OI_OBEX_LOWER_PROTOCOL lowerProtocol;           /**< Lower protocol info of server */
    OI_BOOL connected;                              /**< Indicates if connected to a server */
    OPP_STATE state;                               /**< Indicates current state of OPP client */
    OI_BD_ADDR addr;                                /**< Remote OPP server address */
    OI_CHAR fileName[MAX_NAME_LEN];                             /**< File Name of data being transferred */
    OI_BOOL defaultName;                                  /**< TRUE if default name to be used for incoming file */
    OI_CHAR *fileExt;                             /**< File Extension of data being transferred */
    OI_CHAR mimeType[MAX_NAME_LEN];                             /**< Mime Type of data being transferred */
    OI_BOOL abort;                                  /**< TRUE if current op is to be aborted */
    int numOfEntries;
    FILE_TYPE_DATA fileTypeData[MAX_MIME_TYE_LIST_LEN];
};

static OPP_DATA opp;

static const OI_BD_ADDR bogusAddr = { {0,0,0,0,0,0} };
#define BASE_ENGLISH(id)   ((id) + 0x100)  /* 0x0100 is the default for the primary language. */
#define BASE_FRANCAIS(id)  ((id) + 0x110)
#define BASE_DEUTSCH(id)   ((id) + 0x120)
#define BASE_JAPANESE(id)  ((id) + 0x130)
#define BASE_KOREAN(id)    ((id) + 0x140)
#define BASE_CHINESE(id)   ((id) + 0x150)
#define BASE_SPANISH(id)   ((id) + 0x160)
#define BASE_ARABIC(id)    ((id) + 0x170)
#define BASE_HEBREW(id)    ((id) + 0x180)
#define OBEX_CONN_ID_HEADER_LEN 5
#define OBEX_SRM_HEADER_LEN 2
#define OBEX_LEN_HEADER_LEN 5
#define OBEX_NAME_HEADER_LEN 3
#define OBEX_TYPE_HEADER_LEN 3
#define OBEX_BODY_HEADER_LEN 3

static const OI_SDPDB_ATTRIBUTE strAttrs[] = {
    { BASE_ENGLISH(OI_ATTRID_ServiceName),        OI_ELEMENT_STRING("OBEX Object Push") },
    { BASE_ENGLISH(OI_ATTRID_ServiceDescription), OI_ELEMENT_STRING("Object Push") }
};

static const OI_SDP_STRINGS strings = {
    strAttrs,
    OI_ARRAYSIZE(strAttrs)
};

/**
 * Prefer OBEX/L2CAP if it is supported - otherwise OBEX/RFCOMM
 */
static OI_OBEX_LOWER_PROTOCOL_ID preferProtocol = OI_OBEX_LOWER_L2CAP;

static uint8_t OPP_FORMAT_VCARD21     = 0x01;
static uint8_t OPP_FORMAT_VCARD30     = 0x02;
static uint8_t OPP_FORMAT_VCAL10      = 0x03;
static uint8_t OPP_FORMAT_ICAL20      = 0x04;
static uint8_t OPP_FORMAT_VNOTE       = 0x05;
static uint8_t OPP_FORMAT_VMESSAGE    = 0x06;
static uint8_t OPP_FORMAT_ANY_TYPE_OF_OBJ = 0xFF;

static uint8_t OPP_FORMAT_ALL [] = {
    OPP_FORMAT_VCARD21,
    OPP_FORMAT_VCARD30,
    OPP_FORMAT_VCAL10,
    OPP_FORMAT_ICAL20,
    OPP_FORMAT_VNOTE,
    OPP_FORMAT_VMESSAGE,
    OPP_FORMAT_ANY_TYPE_OF_OBJ
};

/* ****************************************************************************
 *
 *                      virtual object filing system
 *
 * These functions implement object open, close, read, and write functions
 * for both the OPP client and OPP server.
 *
 * ****************************************************************************/

void read_config_file(char* config_filename) {

    FILE *fp;
    char buf[CONFIG_LINE_BUFFER_SIZE];
    char *token;
    int count = 0;
    char *ctx;

    ALOGV(LOGTAG " read_config_file: Opening %s\n", config_filename);
    if (( fp = fopen(config_filename, "r")) == NULL) {
        ALOGE(LOGTAG  " Failed to open config file %s", config_filename);
        return;
    }

    while (! feof(fp)) {
        if (fgets(buf, CONFIG_LINE_BUFFER_SIZE, fp) == NULL)
            break;
        if (buf[0] == '#' || strlen(buf) == 0) {
            continue;
        }
        /* get the first token */
        token = strtok_r(buf, ",", &ctx);
        if (token != NULL) {
            count ++;
            strlcpy(opp.fileTypeData[opp.numOfEntries].ext,
                token, strlen(token) +  1);
        }
        /* walk through other tokens */
        while( token != NULL ) {
            if (count > 2) {
                ALOGW(LOGTAG "Invalid format in stored line. continuing...");
                continue;
            }
            token = strtok_r(NULL, ",", &ctx);
            if (token != NULL) {
                strlcpy(opp.fileTypeData[opp.numOfEntries].mimeType,
                token, strlen(token) + 1);
                count ++;
            }
        }
        count = 0;
        opp.numOfEntries ++;
    }
    ALOGD(LOGTAG "num Of Entries %d\n", opp.numOfEntries);
    fclose(fp);
}

/**
   This structure defines the object.
 */
typedef struct {
    FILE *handle;
    OI_CHAR16 nameStr[MAX_NAME_LEN];
    OI_UINT16 nameLen;
    OI_BYTE *data;
    OI_UINT32 size;
    OI_UINT32 bytes_read;
    OI_BOOL read;
} OBJECT;


static OBJECT object;

static OI_CHAR * obex_err_code_to_str (OI_STATUS status) {
    switch (status) {
        case OI_OK                              : return "SUCCESS";
        case OI_OBEX_COMMAND_ERROR              : return "COMMAND_ERROR";
        case OI_OBEX_CONNECTION_TIMEOUT         : return "CONNECTION_TIMEOUT";
        case OI_OBEX_CONNECT_FAILED             : return "CONNECT_FAILED";
        case OI_OBEX_DISCONNECT_FAILED          : return "DISCONNECT_FAILED";
        case OI_OBEX_ERROR                      : return "ERROR";
        case OI_OBEX_INCOMPLETE_PACKET          : return "INCOMPLETE_PACKET";
        case OI_OBEX_LENGTH_REQUIRED            : return "LENGTH_REQUIRED";
        case OI_OBEX_NOT_CONNECTED              : return "NOT_CONNECTED";
        case OI_OBEX_NO_MORE_CONNECTIONS        : return "NO_MORE_CONNECTIONS";
        case OI_OBEX_OPERATION_IN_PROGRESS      : return "OPERATION_IN_PROGRESS";
        case OI_OBEX_PUT_RESPONSE_ERROR         : return "PUT_RESPONSE_ERROR";
        case OI_OBEX_GET_RESPONSE_ERROR         : return "GET_RESPONSE_ERROR";
        case OI_OBEX_REQUIRED_HEADER_NOT_FOUND  : return "REQUIRED_HEADER_NOT_FOUND";
        case OI_OBEX_SERVICE_UNAVAILABLE        : return "SERVICE_UNAVAILABLE";
        case OI_OBEX_TOO_MANY_HEADER_BYTES      : return "TOO_MANY_HEADER_BYTES";
        case OI_OBEX_UNKNOWN_COMMAND            : return "UNKNOWN_COMMAND";
        case OI_OBEX_UNSUPPORTED_VERSION        : return "UNSUPPORTED_VERSION";
        case OI_OBEX_CLIENT_ABORTED_COMMAND     : return "CLIENT_ABORTED_COMMAND";
        case OI_OBEX_BAD_PACKET                 : return "BAD_PACKET";
        case OI_OBEX_BAD_REQUEST                : return "BAD_REQUEST";
        case OI_OBEX_OBJECT_OVERFLOW            : return "OBJECT_OVERFLOW";
        case OI_OBEX_NOT_FOUND                  : return "NOT_FOUND";
        case OI_OBEX_ACCESS_DENIED              : return "ACCESS_DENIED";
        case OI_OBEX_VALUE_NOT_ACCEPTABLE       : return "VALUE_NOT_ACCEPTABLE";
        case OI_OBEX_PACKET_OVERFLOW            : return "PACKET_OVERFLOW";
        case OI_OBEX_NO_SUCH_FOLDER             : return "NO_SUCH_FOLDER";
        case OI_OBEX_NAME_REQUIRED              : return "NAME_REQUIRED";
        case OI_OBEX_PASSWORD_TOO_LONG          : return "PASSWORD_TOO_LONG";
        case OI_OBEX_PRECONDITION_FAILED        : return "PRECONDITION_FAILED";
        case OI_OBEX_UNAUTHORIZED               : return "UNAUTHORIZED";
        case OI_OBEX_NOT_IMPLEMENTED            : return "NOT_IMPLEMENTED";
        case OI_OBEX_INVALID_AUTH_DIGEST        : return "INVALID_AUTH_DIGEST";
        case OI_OBEX_INVALID_OPERATION          : return "INVALID_OPERATION";
        case OI_OBEX_DATABASE_FULL              : return "DATABASE_FULL";
        case OI_OBEX_DATABASE_LOCKED            : return "DATABASE_LOCKED";
        case OI_OBEX_INTERNAL_SERVER_ERROR      : return "INTERNAL_SERVER_ERROR";
        case OI_OBEX_UNSUPPORTED_MEDIA_TYPE     : return "UNSUPPORTED_TYPE";
        case OI_OBEX_PARTIAL_CONTENT            : return "PARTIAL_CONTENT";
        case OI_OBEX_METHOD_NOT_ALLOWED         : return "METHOD_NOT_ALLOWED";
        case OI_OBEXSRV_INCOMPLETE_GET          : return"OI_OBEXSRV_INCOMPLETE_GET";
        case OI_OBEX_FOLDER_BROWSING_NOT_ALLOWED : return "FOLDER_BROWSING_NOT_ALLOWED";
        case OI_OBEX_SERVER_FORCED_DISCONNECT   : return "SERVER_FORCED_DISCONNECT";
        case OI_OBEX_OFS_ERROR                  : return "OFS_ERROR";
        case OI_OBEX_FILEOP_ERROR               : return "FILEOP_ERROR";
        case OI_OBEX_USERID_TOO_LONG            : return "USERID_TOO_LONG";
        default :
        ALOGD(LOGTAG "obex_err_code_to_str: status = %d\n", status);
        return "UNKNOWN_ERROR";
    }
}

static OI_CHAR * opp_state_to_str (OPP_STATE state) {
    switch (state) {
        case STATE_INITIAL          : return "STATE_INITIAL";
        case STATE_IDLE             : return "STATE_IDLE";
        case STATE_CONNECTING       : return "STATE_CONNECTING";
        case STATE_CONNECTED        : return "STATE_CONNECTED";
        case STATE_SENDING          : return "STATE_SENDING";
        case STATE_RECEIVING        : return "STATE_RECEIVING";
        case STATE_DEINITIALIZING   : return "STATE_DEINITIALIZING";
        default :
        ALOGD(LOGTAG "opp_state_to_str: state = %d\n", state);
        return "UNKNOWN_STATE";
    }
}

static OI_STATUS OFS_OpenRead(const OI_OBEX_UNICODE *name,
                              const OI_CHAR *type,
                              OI_UINT32 maxRead,
                              OI_OPP_OPEN_READ_CFM openCfm,
                              OI_OPP_CONNECTION oppConnection)
{
    static OI_CHAR fileName[MAX_NAME_LEN + 1];
    int read = 0;
    int remaining;
    OI_STATUS status = OI_OK;

    /*
     * Clear object.
     */
    memset(&object, 0, sizeof(OBJECT));

    object.read = TRUE;
    object.nameLen = name->len;
    if (object.nameLen > (MAX_NAME_LEN - strlen(storageDir) - 1)) {
        ALOGD(LOGTAG "File name len too large, can't open file for reading\n");
        return OI_STATUS_DATA_ERROR;
    }
    /* Copy Unicode name to saved object */
    memcpy(object.nameStr, name->str, name->len);
    /*
     * Get name as an ascii string. This would not be done by an implementation
     * that has native support for unicode object names.
     */
    strlcpy(fileName, storageDir, strlen(storageDir) + 1);
    OI_Utf16ToUtf8((const OI_UTF16*)name->str, name->len,
                            (OI_UTF8*)fileName + strlen(storageDir),
                            sizeof(fileName));
    object.handle = fopen((const char *)fileName, "r+b");
    ALOGD(LOGTAG "Opening object %s to read\n", fileName);
    if (object.handle != NULL) {
        fseek(object.handle, 0L, SEEK_END);
        object.size = ftell(object.handle);
        rewind(object.handle);
        object.bytes_read = 0;
        /* Allocate memory for reading data */
        object.data = (OI_BYTE *)malloc(maxRead);
        if (!object.data) {
            ALOGD(LOGTAG "Unable to allocate %d bytes of memory for reading data", maxRead);
            fclose(object.handle);
            object.handle = NULL;
            return OI_STATUS_OUT_OF_MEMORY;
        }
        if (opp.lowerProtocol.protocol == OI_OBEX_LOWER_L2CAP) {
            maxRead -= (OBEX_CONN_ID_HEADER_LEN + OBEX_LEN_HEADER_LEN +
                OBEX_SRM_HEADER_LEN + OBEX_NAME_HEADER_LEN + OBEX_TYPE_HEADER_LEN +
                OBEX_BODY_HEADER_LEN+ (name->len * 2) + (strlen(type) + 1));
            ALOGD(LOGTAG "maxRead = %d name->len = %d type len = %d\n",
                maxRead, name->len * 2, strlen(type) + 1 );
            remaining = maxRead;
            if (object.bytes_read < object.size) {
                ALOGD(LOGTAG "Reading  %d bytes\n", remaining);
                read = fread(object.data + (maxRead - remaining), 1, remaining, object.handle);
                ALOGD(LOGTAG "Read %d bytes\n", read);
                remaining -= read;
                /*
                 * Pass data from in-memory object to OPP
                 */
                if (remaining == 0)
                    status = OI_OK;
                else if (remaining < maxRead)
                    status = OI_STATUS_END_OF_FILE;

                if (remaining < maxRead) {
                    object.bytes_read += (maxRead - remaining);
                }
            }
        } else {
            maxRead = remaining = 0;
            status = OI_OK;
        }
        /*
         * This implementation uses the address of the object as the handle.
         */
        openCfm((OI_OPP_HANDLE) &object, name, type, object.size, object.data,
            (OI_UINT16) (maxRead - remaining), status, oppConnection);
        return OI_OK;
    } else {
        ALOGD(LOGTAG "Unable to Open file %s for reading\n", fileName);
        OI_Printf("Unable to Open file %s for reading\n", fileName);
        return OI_STATUS_READ_ERROR;
    }
}

static OI_STATUS OFS_OpenWrite(const OI_OBEX_UNICODE *name,
                               const OI_CHAR *type,
                               OI_UINT32 objSize,
                               OI_OPP_OPEN_WRITE_CFM openCfm,
                               OI_OPP_CONNECTION oppConnection)
{
    static OI_CHAR fileName[MAX_NAME_LEN + 1];

    /*
     * Clear object.
     */
    memset(&object, 0, sizeof(OBJECT));

    /*
     * Initialize received object
     */

    /* Copy received data to saved object */
    object.size = objSize;
    object.read = FALSE;
    /* Copy Unicode name to saved object */
    memcpy(object.nameStr, name->str, name->len);
    object.nameLen = name->len;
    if (object.nameLen > (MAX_NAME_LEN - strlen(storageDir) - 1)) {
        ALOGD(LOGTAG "File name len too large, truncating it\n");
        object.nameLen = MAX_NAME_LEN - strlen(storageDir) - 1;
    }
    /*
     * Get name as an ascii string. This would not be done by an implementation
     * that has native support for unicode object names.
     */
    strlcpy(fileName, storageDir, strlen(storageDir) + 1);
    OI_Utf16ToUtf8((const OI_UTF16*)name->str, object.nameLen,
                            (OI_UTF8*)fileName + strlen(storageDir),
                            sizeof(fileName));
    memset(opp.fileName, 0, MAX_NAME_LEN);
    memcpy(opp.fileName, fileName, strlen(fileName));
    ALOGD(LOGTAG "Opening object %s type %s size %ld to write\n", fileName, type, objSize);
    object.handle = fopen((const char *)fileName, "w");
    if (object.handle != NULL) {
        /*
         * This implementation uses the address of the object as the handle.
         */
        openCfm((OI_OPP_HANDLE) &object, OI_OK, oppConnection);
        return OI_OK;
    } else {
        ALOGD(LOGTAG "Unable to Open file %s for storing data\n", fileName);
        OI_Printf("Unable to Open file %s for storing data\n", fileName);
        return OI_STATUS_WRITE_ERROR;
    }
}

static void OFS_Close(OI_OPP_HANDLE handle,
                      OI_STATUS status,
                      OI_OPP_CONNECTION oppConnection)
{
    OBJECT *obj = (OBJECT*) handle;

    if (!OI_SUCCESS(status)) {
        ALOGE(LOGTAG "Close obj on error %d\n", status);
        if (OI_OBEX_CLIENT_ABORTED_COMMAND == status) {
            OI_Printf("Operation aborted\n");
        }
    } else {
        ALOGD(LOGTAG "Close obj size = %ld\n", obj->size);
        /*
         * Get name as an ascii string. This would not be done by an implementation
         * that has native support for unicode object names.
         */
        if (!obj->read) {
            OI_Printf("Received file stored at %s\n", opp.fileName);
            ALOGD(LOGTAG "Received file %d bytes\n",
                (OI_UINT) obj->size);
        } else {
            ALOGD(LOGTAG "Sent file size %d bytes\n",
                (OI_UINT) obj->size);
        }
    }
    if (obj->read && obj->data) {
        /* Free memory for reading data */
        free(obj->data);
        obj->data = NULL;
    }
    if (obj->handle != NULL) {
        fclose(obj->handle);
        obj->handle = NULL;
    }
    if (status && !obj->read && opp.fileName != NULL) {
        ALOGD(LOGTAG "deleting file %s", opp.fileName);
        remove(opp.fileName);
    }
}


static OI_STATUS OFS_Read(OI_OPP_HANDLE handle,
                          OI_UINT32 maxRead,
                          OI_OPP_READ_CFM readCfm,
                          OI_OPP_CONNECTION oppConnection)
{
    OBJECT *obj = (OBJECT*) handle;
    int read = 0;
    int remaining = maxRead;
    OI_STATUS status;

    if (obj->bytes_read < obj->size) {
        ALOGD(LOGTAG "Reading  %d bytes\n", remaining);
        read = fread(obj->data + (maxRead - remaining), 1, remaining, obj->handle);
        ALOGD(LOGTAG "Read %d bytes\n", read);
        remaining -= read;
        /*
         * Pass data from in-memory object to OPP
         */
        if (remaining == 0)
            status = OI_OK;
        else if (remaining < maxRead)
            status = OI_STATUS_END_OF_FILE;

        if (remaining < maxRead) {
            obj->bytes_read += (maxRead - remaining);
            readCfm(handle, obj->data, (OI_UINT16) (maxRead - remaining), status, oppConnection);
        }
    }
    return OI_OK;
}

static OI_STATUS OFS_Write(OI_OPP_HANDLE handle,
                           const OI_BYTE *buffer,
                           OI_UINT32 bufLen,
                           OI_OPP_WRITE_CFM writeCfm,
                           OI_OPP_CONNECTION oppConnection)
{
    OBJECT *obj = (OBJECT*) handle;
    OI_STATUS status;
    int wrote = 0;
    int remaining = bufLen;

    ALOGD(LOGTAG "Write %d bytes\n", bufLen);

    if (bufLen > 0) {
        while (remaining > 0) {
            wrote = fwrite(buffer + (bufLen - remaining), sizeof(OI_CHAR),
                remaining, obj->handle);
            remaining -= wrote;
            if (wrote == 0) {
                ALOGE(LOGTAG "Unable to write to File System\n");
                writeCfm(handle, OI_STATUS_WRITE_ERROR, oppConnection);
                return OI_STATUS_WRITE_ERROR;
            }
        }
    }
    /*
     * Confirm write
     */
    writeCfm(handle, OI_OK, oppConnection);
    return OI_OK;
}

static const OI_OPP_OBJSYS_FUNCTIONS objSys = {
    OFS_OpenRead,
    OFS_OpenWrite,
    OFS_Close,
    OFS_Read,
    OFS_Write,
    NULL
};

/**
 * used for timing push and pull operations
 */
static OI_TIME clientStartTime;
static OI_TIME serverStartTime;

/******************************************************************************/
/**
 * This function performs a simple conversion from ASCII to unicode.
 * The unicode string is declared as static.
 */
static void FilenameToObjname(OI_OBEX_UNICODE *ustr,
                              const OI_CHAR* astr)
{
    static OI_CHAR16 str16[MAX_NAME_LEN];
    OI_INT i;

    /*
     * Allow for null termination.
     */
    ustr->len = strlen(astr) + 1;
    if (ustr->len == 1) {
        ustr->len = 0;
        return;
    }
    ustr->str = str16;
    for (i = 0; i < ustr->len; ++i) {
        /*
         * Don't transcribe cr/lf
         */
        if ((astr[i] == '\r') || (astr[i] == '\n')) {
            ustr->len = i;
            break;
        }
    }

    OI_Utf8ToUtf16((const OI_UTF8*)astr, ustr->len, ustr->str, MAX_NAME_LEN);
}

/**
 *  Computes throughput in bits per second given a start time and an end time
 */
OI_UINT32 OI_Throughput(OI_UINT32 bytes,
                        OI_TIME *startTime,
                        OI_TIME *endTime)
{
    OI_INT32 msecs;

    msecs = 1000 * (endTime->seconds - startTime->seconds);
    msecs += endTime->mseconds - startTime->mseconds;

    if (0 == msecs) {
        msecs = 1;
    }
    if(msecs < 100)
    {
        OI_Printf("Time taken to transfer the file is too low (EndTime = %ds%dmsc, StartTime=%ds%dms).\n"
            "Please calculate the throughput yourself\n", endTime->seconds, endTime->mseconds, startTime->seconds,
            startTime->mseconds);
        return 0;
    }

    return ((OI_UINT32)(bytes / msecs) * 8);
}


/**
 * Returns a string formatted as seconds and milliseconds
 */
const OI_CHAR* OI_ElapsedTimeTxt(OI_TIME *startTime,
                                 OI_TIME *endTime)
{
    static OI_CHAR buffer[16];
    OI_CHAR zeroes[] = "00";
    OI_INT32 secs;
    OI_INT16 msecs;

    secs = endTime->seconds - startTime->seconds;
    msecs = endTime->mseconds - startTime->mseconds;
    if (msecs < 0) {
        msecs += 1000;
        --secs;
    }
    if ((msecs >= 100) || (msecs == 0)) {
        zeroes[0] = 0;
    } else if (msecs >= 10) {
        zeroes[1] = 0;
    }
    OI_SNPrintf(buffer, sizeof(buffer)," %d.%s%d", (int)secs, zeroes, (int)msecs);
    return buffer;
}

static OPP_STATE getOppState() {
    ALOGD(LOGTAG "getOppState: state = %s", opp_state_to_str(opp.state));
    return opp.state;
}

static void setOppState(OPP_STATE newState) {
    ALOGD(LOGTAG "%s => %s", opp_state_to_str(opp.state), opp_state_to_str(newState));
    opp.state = newState;
}

/* ****************************************************************************
 *
 *                      OPP Server Functions
 *
 * ****************************************************************************/


/*
 * Changes these values to allow or reject push or pull operations.
 */

static void ServerConnectInd(OI_BD_ADDR *clientAddr,
                             OI_OPP_SERVER_CONNECTION_HANDLE connectionId)
{
    OI_STATUS status;
    char bd_str[MAX_BD_STR_LEN];

    bdaddr_to_string((const bt_bdaddr_t*)clientAddr, bd_str, MAX_BD_STR_LEN);

    OI_Printf("OPP connection from %s\n", bd_str);
    if (getOppState() == STATE_CONNECTING) {
        bdaddr_to_string((const bt_bdaddr_t*)&opp.addr, bd_str, MAX_BD_STR_LEN);
        ALOGE(LOGTAG "%s: Already connecting to %s", __FUNCTION__, bd_str);
        fprintf(stdout, "Already connecting to %s\n", bd_str);
        OI_Printf("Rejecting connection\n");
        status = OI_OPP_AcceptConnect(connectionId, FALSE, FALSE);
        return;
    }
    if (getOppState() >= STATE_CONNECTED) {
        bdaddr_to_string((const bt_bdaddr_t*)&opp.addr, bd_str, MAX_BD_STR_LEN);
        ALOGE(LOGTAG "%s: Already connected to %s", __FUNCTION__, bd_str);
        fprintf(stdout, "Already connected to %s\n", bd_str);
        OI_Printf("Rejecting connection\n");
        status = OI_OPP_AcceptConnect(connectionId, FALSE, FALSE);
        return;
    }
    /*
     * An OPP server application can selectively allow clients to push or pull
     * objects. If a client is not allowed to either pull or push the connection
     * is rejected and the client will receive an ACCESS DENIED response.
     */
    status = OI_OPP_AcceptConnect(connectionId, serverAllowPush, serverAllowPull);
    if (!OI_SUCCESS(status)) {
        ALOGD(LOGTAG "AcceptConnect returned %d\n", status);
     } else {
        if (getOppState() == STATE_IDLE) {
            opp.srvConnectionHandle = connectionId;
            memcpy(&opp.addr, clientAddr, sizeof(OI_BD_ADDR));
            setOppState(STATE_CONNECTED);
        }
    }
}

static void ServerDisconnectInd(OI_OPP_SERVER_CONNECTION_HANDLE connectionId)
{
    ALOGD(LOGTAG "ServerDisconnectInd");
    if (getOppState() == STATE_DEINITIALIZING) {
        /* BT is being turned off */
        OI_OPPServer_Deregister(opp.serverHandle);
        opp.serverHandle = 0;
        g_opp->RemoveSdpRecord();
        /*
        * Its possible that SDP Client thread terminated before receiving above request,
        * so send stop profile with success.
        */
        BtEvent *stop_event = new BtEvent;
        stop_event->profile_start_event.event_id = PROFILE_EVENT_STOP_DONE;
        stop_event->profile_start_event.profile_id = PROFILE_ID_OPP;
        stop_event->profile_start_event.status = true;
        PostMessage(THREAD_ID_GAP, stop_event);
    }
    if (opp.srvConnectionHandle ==  connectionId) {
        opp.srvConnectionHandle = 0;
        uint8_t zero[sizeof(bt_bdaddr_t)] = { 0 };
        memcpy(&opp.addr, zero, sizeof(bt_bdaddr_t));
        setOppState(STATE_IDLE);
        opp.lowerProtocol.protocol = OI_OBEX_LOWER_NONE;
    }
    if (opp.abort == true) {
        fprintf(stdout, "Aborted last operation \n");
        opp.abort = false;
    }
}

static void ServerEventInd(OI_OPP_SERVER_CONNECTION_HANDLE connectionId,
                           OI_OPP_SERVER_EVENT_DATA       *eventPtr)
{
    OI_TIME endTime;

    switch(eventPtr->event) {
        case OI_OPP_SERVER_EVENT_PUSH:
            if (eventPtr->data.push.localName &&
                eventPtr->data.push.localName->str &&
                eventPtr->data.push.localName->len) {
                OI_Printf("\nIncoming object: %S\n",
                    eventPtr->data.push.localName->str, eventPtr->data.push.localName->len);
            } else {
                OI_Printf("\nIncoming object: Unnamed\n");
                opp.defaultName = TRUE;
            }
            if (eventPtr->data.push.totalSize) {
                OI_Printf("Incoming size: %d bytes\n", eventPtr->data.push.totalSize);
            } else {
                OI_Printf("Incoming size: Unknown\n");
            }
            if (eventPtr->data.push.objType) {
                OI_Printf("Incoming type: %s\n", eventPtr->data.push.objType);
            } else {
                OI_Printf("Incoming type: Unknown\n");
            }

            if (rejectPush || getOppState() != STATE_CONNECTED) {
                OI_Printf("Rejecting incoming file\n");
                OI_OPPServer_AcceptPush(connectionId, NULL, FALSE);
            } else {
                setOppState(STATE_RECEIVING);
                memset(opp.fileName, 0, MAX_NAME_LEN);
                OI_Utf16ToUtf8((const OI_UTF16*)eventPtr->data.push.localName->str,
                                        eventPtr->data.push.localName->len,
                                        (OI_UTF8*)opp.fileName,
                                        sizeof(opp.fileName));
                // pass the same event to Main thread
                BtEvent* bt_event = NULL;
                bt_event = new BtEvent;
                bt_event->event_id = MAIN_EVENT_INCOMING_FILE_REQUEST;
                PostMessage(THREAD_ID_MAIN, bt_event);
            }
            break;

        case OI_OPP_SERVER_EVENT_PUSH_PROGRESS:
            if (reportProgress) {
                OI_Printf("Incoming Progress: %d bytes\n",
                    eventPtr->data.pushProgress.bytesTransferred);
            }
            break;

        case OI_OPP_SERVER_EVENT_PUSH_COMPLETE:
            if (eventPtr->data.pushComplete.status == OI_OK) {
                OI_Time_Now(&endTime);
                ALOGV(LOGTAG "Push completed in %s seconds\n",
                    OI_ElapsedTimeTxt(&serverStartTime, &endTime));
                ALOGV(LOGTAG " Incoming Complete: %d bytes status:%d\n",
                    eventPtr->data.pushComplete.finalSize,
                    eventPtr->data.pushComplete.status);
                OI_Printf("Throughput %d Kbps\n",
                    OI_Throughput(eventPtr->data.pushComplete.finalSize,
                    &serverStartTime, &endTime));
                ALOGD(LOGTAG "Throughput %d Kbps\n",
                    OI_Throughput(eventPtr->data.pushComplete.finalSize,
                    &serverStartTime, &endTime));
            }
            if (getOppState() == STATE_RECEIVING)
                setOppState(STATE_CONNECTED);
            break;

        default:
            break;
    }
}


static const OI_OPP_SERVER_CALLBACKS callbacks = {
    ServerConnectInd,
    ServerDisconnectInd,
    ServerEventInd
};

void BtOppMsgHandler(void *msg)
{
    BtEvent* event = NULL;
    bool status = false;
    OI_STATUS ret;
    if(!msg) {
        ALOGE(LOGTAG "%s: Msg is null, return", __FUNCTION__);
        return;
    }

    event = ( BtEvent *) msg;

    if (event == NULL) {
        ALOGE(LOGTAG "%s: event is null", __FUNCTION__);
        return;
    }

    ALOGD(LOGTAG "BtOppMsgHandler event = %d", event->event_id);
    switch(event->event_id) {
        case PROFILE_API_START:
            {
                BtEvent *start_event = new BtEvent;
                memset(&opp, 0, sizeof(OPP_DATA));
                start_event->profile_start_event.event_id = PROFILE_EVENT_START_DONE;
                start_event->profile_start_event.profile_id = PROFILE_ID_OPP;
                start_event->profile_start_event.status = true;
                PostMessage(THREAD_ID_GAP, start_event);
            }
            break;

        case PROFILE_API_STOP:
            if(g_opp) {
                if (opp.clientConnectionHandle) {
                   /* disconnect opp client if connected */
                   ret = OI_OPPClient_Disconnect(opp.clientConnectionHandle);
                   ALOGD(LOGTAG " OI_OPPClient_Disconnect returned %d", ret);
                }
                if (opp.srvConnectionHandle) {
                    setOppState(STATE_DEINITIALIZING);
                    /* disconnect opp server if connected */
                    ret = OI_OPPServer_ForceDisconnect(opp.srvConnectionHandle);
                    ALOGD(LOGTAG " OI_OPPServer_ForceDisconnect returned %d", ret);
                } else {
                    /* Server is not connected, proceed with de-registration */
                    ret = OI_OPPServer_Deregister(opp.serverHandle);
                    ALOGD(LOGTAG " OI_OPPServer_Deregister returned %d", ret);
                    opp.serverHandle = 0;
                    g_opp->RemoveSdpRecord();
                    /*
                    * Its possible that SDP Client thread terminated before receiving above request,
                    * so send stop profile with success.
                    */
                    BtEvent *stop_event = new BtEvent;
                    stop_event->profile_start_event.event_id = PROFILE_EVENT_STOP_DONE;
                    stop_event->profile_start_event.profile_id = PROFILE_ID_OPP;
                    stop_event->profile_start_event.status = true;
                    PostMessage(THREAD_ID_GAP, stop_event);
                }
            }
            break;

        default:
            if(g_opp) {
               g_opp->ProcessEvent(( BtEvent *) msg);
            }
            break;
    }
    delete event;
}

Opp :: Opp(const bt_interface_t *bt_interface, config_t *config)
{
    this->bluetooth_interface = bt_interface;
    this->config = config;
    opp_connect_timer = NULL;
    if( !(opp_connect_timer = alarm_new())) {
        ALOGE(LOGTAG " unable to create opp_connect_timer");
        return;
    }
}

void opp_connect_timer_expired(void *context) {
    ALOGD(LOGTAG, " opp_connect_timer_expired");

    BtEvent *event = new BtEvent;
    event->event_id = OPP_CONNECT_TIMEOUT;
    memcpy(&event->opp_event.bd_addr, (bt_bdaddr_t *)context, sizeof(bt_bdaddr_t));
    PostMessage(THREAD_ID_OPP, event);
}

Opp :: ~Opp()
{
    alarm_free(g_opp->opp_connect_timer);
    g_opp->opp_connect_timer = NULL;
}

void ConnectionCfmCb(OI_OPP_CLIENT_CONNECTION_HANDLE connectionId,
                                       OI_STATUS status)
{
    char bd_str[MAX_BD_STR_LEN];
    bdaddr_to_string((const bt_bdaddr_t*)&opp.addr, bd_str, MAX_BD_STR_LEN);
    ALOGV(LOGTAG "%s: status: %d connectionID = %p", __FUNCTION__,
        status, connectionId);
    //stoping opp_connect_timer
    if(g_opp) {
        alarm_cancel(g_opp->opp_connect_timer);
    }

    if (getOppState() != STATE_CONNECTING) {
        ALOGE(LOGTAG "%s: received connect cfm in invalid state %s",
            __FUNCTION__, opp_state_to_str(opp.state));
        /* Most likely connection timed out, so disconnect OBEX link now */
        OI_OPPClient_Disconnect(connectionId);
        return;
    }

    if (status == OI_STATUS_SUCCESS) {
        opp.clientConnectionHandle = connectionId;
        setOppState(STATE_CONNECTED);
        opp.abort = false;
        fprintf(stdout, "Connected to %s\n", bd_str);
        /* Send Internal Connect Message to OPP Thread */
        BtEvent *event = new BtEvent;
        event->event_id = OPP_INTERNAL_SEND;
        PostMessage(THREAD_ID_OPP, event);
    } else {
        opp.clientConnectionHandle = NULL;
        setOppState(STATE_IDLE);
        uint8_t zero[sizeof(bt_bdaddr_t)] = { 0 };
        memcpy(&opp.addr, zero, sizeof(bt_bdaddr_t));
        fprintf(stdout, "Failed to Connect to %s\n", bd_str);
    }
}

void DisconnectionCfmCb(OI_OPP_CLIENT_CONNECTION_HANDLE connectionId)
{
    ALOGV(LOGTAG "%s: connectionID = %p", __FUNCTION__, connectionId);
    char bd_str[MAX_BD_STR_LEN];
    if (opp.clientConnectionHandle == connectionId) {
        bdaddr_to_string((const bt_bdaddr_t*)&opp.addr, bd_str, MAX_BD_STR_LEN);
        fprintf(stdout, "Disconnected with %s\n", bd_str);
        opp.clientConnectionHandle = NULL;
        setOppState(STATE_IDLE);
        opp.lowerProtocol.protocol = OI_OBEX_LOWER_NONE;
        opp.abort = false;
        uint8_t zero[sizeof(bt_bdaddr_t)] = { 0 };
        memcpy(&opp.addr, zero, sizeof(bt_bdaddr_t));
    }
}

void AbortCfmCb(OI_OPP_CLIENT_CONNECTION_HANDLE connectionId)
{
    ALOGV(LOGTAG "%s: connectionID = %p", __FUNCTION__, connectionId);
    fprintf(stdout, "Aborted last operation \n");
    opp.abort = false;
}

static void ClientEventInd(OI_OPP_CLIENT_CONNECTION_HANDLE  connectionId,
                       const OI_OPP_CLIENT_EVENT_DATA  *evtPtr,
                       OI_STATUS                        status)
{
    OI_TIME endTime;
    static bool pushIndReceived;
    switch(evtPtr->event) {
        case OI_OPP_CLIENT_CONNECTED:
            ConnectionCfmCb(connectionId, status);
            pushIndReceived = false;
            break;

        case OI_OPP_CLIENT_PUSH_STARTED:
            setOppState(STATE_SENDING);
            break;

        case OI_OPP_CLIENT_PUSH_PROGRESS:
            if (!pushIndReceived) {
                OI_Printf("File accepted by remote device\n");
                OI_Time_Now(&clientStartTime);
                pushIndReceived = true;
            }
            break;

        case OI_OPP_CLIENT_PUSH_COMPLETE:
            if (status == OI_OK) {
                OI_Time_Now(&endTime);
                ALOGV(LOGTAG "Push completed in %s seconds\n",
                    OI_ElapsedTimeTxt(&clientStartTime, &endTime));
                ALOGV(LOGTAG "Outgoing Complete: %d bytes status: %s\n", object.size,
                    obex_err_code_to_str(status));
                OI_Printf("Throughput %d Kbps\n", OI_Throughput(object.size,
                    &clientStartTime, &endTime));
                ALOGD(LOGTAG "Throughput %d Kbps\n", OI_Throughput(object.size,
                    &clientStartTime, &endTime));
            } else {
                OI_Printf("Failed to send file, status = %s\n",
                    obex_err_code_to_str(status));
                ALOGD(LOGTAG "Failed to send file, status = %s\n",
                    obex_err_code_to_str(status));
            }
            if (getOppState() == STATE_SENDING)
                setOppState(STATE_CONNECTED);
            if (opp.clientConnectionHandle) {
                /* disconnect opp if connected */
                OI_OPPClient_Disconnect(opp.clientConnectionHandle);
            }
            break;

        case OI_OPP_CLIENT_DISCONNECT:
            DisconnectionCfmCb(connectionId);
            break;

        default:
            ALOGV(LOGTAG "ClientEventInd, unknown event received %d, "
                "status %d\n", evtPtr->event, status);
            break;
    }
}

static void sdp_add_record_callback(bt_status_t status, int handle)
{
    OI_STATUS ret;
    if (!status) {
        opp.rec_handle = handle;
        setOppState(STATE_IDLE);
        opp.lowerProtocol.protocol = OI_OBEX_LOWER_NONE;
        opp.numOfEntries = 0;
        read_config_file(configFileName);
        if (opp.numOfEntries == 0) {
            ALOGE(LOGTAG "%s: unable to read mime type entries", __FUNCTION__);
        }
        fprintf(stdout, "Successfully Registered OPP Server SDP Record\n");
        ret = OI_OPPServer_Register(&callbacks, &objSys,
            OI_OPP_SERVER_OBJ_FORMAT_ANY, &strings, &opp.serverHandle);
    } else {
        fprintf(stdout, "Sdp add record failed. Incoming transfer will fail\n");
        ALOGE(LOGTAG "%s: sdp add record failed, status %d", __FUNCTION__, status);
    }
}

static void sdp_remove_record_callback(bt_status_t status)
{
    OI_STATUS ret;
    if (status) {
        ALOGE(LOGTAG "%s: sdp remove record failed, status %d", __FUNCTION__, status);
        return;
    }
    ALOGV(LOGTAG "%s", __FUNCTION__);
    opp.rec_handle = -1;
    setOppState(STATE_INITIAL);
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
    if (IS_UUID(UUID_OBEX_OBJECT_PUSH, uuid)) {
        bdaddr_to_string(bd_addr, bd_str, MAX_BD_STR_LEN);
        ALOGE(LOGTAG "%s: status %d, addr %s, L2CAP PSM = %d, "
            "RFCOMM channel = %d, profile version = 0x%04x, "
            "supported supported_formats_list_len = %d"
            "more results = %d", __FUNCTION__, status, bd_str,
            record->ops.hdr.l2cap_psm,
            record->ops.hdr.rfcomm_channel_number,
            record->ops.hdr.profile_version,
            record->ops.supported_formats_list_len,
            more_result);
        if (record->ops.hdr.rfcomm_channel_number > 0 ||
            record->ops.hdr.l2cap_psm > 0) {
            if (record->ops.hdr.l2cap_psm > 0 &&
                record->ops.hdr.profile_version >= profileVersion) {
                opp.lowerProtocol.protocol = OI_OBEX_LOWER_L2CAP;
                opp.lowerProtocol.svcId.l2capPSM = record->ops.hdr.l2cap_psm;
            } else {
                opp.lowerProtocol.protocol = OI_OBEX_LOWER_RFCOMM;
                opp.lowerProtocol.svcId.rfcommChannel =
                    record->ops.hdr.rfcomm_channel_number;
            }
            /* Send Internal Connect Message to OPP Thread */
            BtEvent *event = new BtEvent;
            event->event_id = OPP_INTERNAL_CONNECT;
            PostMessage(THREAD_ID_OPP, event);
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

bool getMimeTypeFromFileExt(OI_CHAR * fileExt)
{
    int i;

    if (fileExt == NULL)
        return false;

    for (i = 0; i < opp.numOfEntries; i ++) {
        if (!strncmp (opp.fileTypeData[i].ext, fileExt, strlen(fileExt))) {
            /* Match Found */
            strlcpy(opp.mimeType, opp.fileTypeData[i].mimeType,
                strlen(opp.fileTypeData[i].mimeType));
            return true;
        }
    }
    return false;
}

void Opp::ProcessEvent(BtEvent* pEvent)
{
    ALOGD(LOGTAG "%s: Processing event %d", __FUNCTION__, pEvent->event_id);

    switch(pEvent->event_id) {

        case OPP_SRV_REGISTER:
            AddSdpRecord();
            break;

        case OPP_SEND_DATA:
            strlcpy((char *)opp.fileName, pEvent->opp_event.value, MAX_NAME_LEN);
            opp.fileExt = strrchr(opp.fileName, '.');
            if (opp.fileExt != NULL) {
                opp.fileExt = opp.fileExt+ 1;
                ALOGW(LOGTAG "%s: File extension %s", __FUNCTION__, opp.fileExt);
            } else {
                ALOGW(LOGTAG "%s: invalid type of file being sent, please send valid file",
                    __FUNCTION__, opp.fileName);
                OI_Printf("invalid type of file being sent, please send valid file");
                break;
            }
            memset(opp.mimeType, 0 , MAX_NAME_LEN);
            if (getMimeTypeFromFileExt(opp.fileExt)) {
                PerformSdp(&pEvent->opp_event.bd_addr);
            } else {
                ALOGE(LOGTAG "%s: Unable to find mime type for extension %s, can't proceed",
                    __FUNCTION__, opp.fileExt);
            }
            break;

        case OPP_INTERNAL_CONNECT:
            Connect();
            break;

        case OPP_INTERNAL_SEND:
            SendData();
            break;

        case OPP_CONNECT_TIMEOUT:
            HandleConnectTimeout(&pEvent->opp_event.bd_addr);
            break;

        case OPP_INTERNAL_DISCONNECTION:
            Disconnect();
            break;

        case OPP_ABORT_TRANSFER:
            Abort();
            break;

        case OPP_INCOMING_FILE_RESPONSE:
            IncomingFileRsp(pEvent->opp_event.accept);
            break;

        default:
            ALOGW(LOGTAG "%s: unhandled event: %d", __FUNCTION__, pEvent->event_id);
            break;
    }
}

void Opp :: AddSdpRecord()
{
    if (getOppState() >= STATE_IDLE) {
        fprintf(stdout, "Already registered \n");
        return;
    }
    /* Add OPP Server SDP Record */
    BtEvent *sdp_search_event = new BtEvent;
    sdp_search_event->sdp_client_event.event_id = SDP_CLIENT_ADD_RECORD;
    memset(&sdp_search_event->sdp_client_event.record, 0 , sizeof(bluetooth_sdp_record));
    sdp_search_event->sdp_client_event.record.ops.hdr.type = SDP_TYPE_OPP_SERVER;
    sdp_search_event->sdp_client_event.record.ops.hdr.profile_version = profileVersion;
    sdp_search_event->sdp_client_event.record.ops.hdr.service_name = profile_name;
    sdp_search_event->sdp_client_event.record.ops.hdr.service_name_length = strlen(profile_name);
    sdp_search_event->sdp_client_event.record.ops.hdr.rfcomm_channel_number = RFCOMM_PREF_OPP_SRV;
    sdp_search_event->sdp_client_event.record.ops.hdr.l2cap_psm = L2CAP_PREF_OPP_SRV;
    memset(sdp_search_event->sdp_client_event.record.ops.supported_formats_list,
        0, sizeof(OPP_FORMAT_ALL));
    sdp_search_event->sdp_client_event.record.ops.supported_formats_list_len =
        sizeof(OPP_FORMAT_ALL);
    sdp_search_event->sdp_client_event.addRecordCb = &sdp_add_record_callback;
    PostMessage(THREAD_ID_SDP_CLIENT, sdp_search_event);
}

void Opp :: RemoveSdpRecord()
{
    if (getOppState() == STATE_INITIAL) {
        ALOGE(LOGTAG "Already un-registered ");
        return;
    }
    if (opp.rec_handle != -1) {
        /* Remove OPP Server SDP Record */
        BtEvent *sdp_search_event = new BtEvent;
        sdp_search_event->sdp_client_event.event_id = SDP_CLIENT_REMOVE_RECORD;
        sdp_search_event->sdp_client_event.removeRecordCb = &sdp_remove_record_callback;
        sdp_search_event->sdp_client_event.rec_handle = opp.rec_handle;
        PostMessage(THREAD_ID_SDP_CLIENT, sdp_search_event);
    }
}

bool Opp :: PerformSdp(bt_bdaddr_t *addr)
{
    bool ret = true;
    char bd_str[MAX_BD_STR_LEN];

    if (!addr) {
        ALOGE(LOGTAG "%s: Bluetooth device address null", __FUNCTION__);
        return false;
    }
    bdaddr_to_string((const bt_bdaddr_t*)&opp.addr, bd_str, MAX_BD_STR_LEN);
    if (getOppState() == STATE_INITIAL) {
        ALOGE(LOGTAG "%s: Not registered, please register before connecting!!", __FUNCTION__);
        fprintf(stdout, "Not registered, please register before connecting!!\n");
        return false;
    } else if (getOppState() == STATE_CONNECTING) {
        ALOGE(LOGTAG "%s: Currently connecting to %s", __FUNCTION__, bd_str);
        fprintf(stdout, "Currently connecting to %s\n", bd_str);
        return false;
    } else if (getOppState() >= STATE_CONNECTED) {
        ALOGE(LOGTAG "%s: Already connected to %s", __FUNCTION__, bd_str);
        fprintf(stdout, "Already connected to %s\n", bd_str);
        return false;
    }
    memcpy(&opp.addr, addr, sizeof(bt_bdaddr_t));
    bdaddr_to_string(addr, bd_str, MAX_BD_STR_LEN);
    ALOGV(LOGTAG "%s: %s", __FUNCTION__, bd_str);
    BtEvent *sdp_search_event = new BtEvent;
    /* Perform SDP to determine OPP Server Record */
    sdp_search_event->sdp_client_event.event_id = SDP_CLIENT_SEARCH;
    memcpy(&sdp_search_event->sdp_client_event.bd_addr, &opp.addr, sizeof(bt_bdaddr_t));
    sdp_search_event->sdp_client_event.uuid = UUID_OBEX_OBJECT_PUSH;
    sdp_search_event->sdp_client_event.searchCb = &sdp_search_callback;
    PostMessage(THREAD_ID_SDP_CLIENT, sdp_search_event);

    return ret;
}

bool Opp :: Connect()
{
    char bd_str[MAX_BD_STR_LEN];
    if (getOppState() == STATE_CONNECTING) {
        bdaddr_to_string((const bt_bdaddr_t*)&opp.addr, bd_str, MAX_BD_STR_LEN);
        ALOGE(LOGTAG "%s: Already connecting to %s", __FUNCTION__, bd_str);
        fprintf(stdout, "Already connecting to %s\n", bd_str);
        return false;
    }
    if (getOppState() >= STATE_CONNECTED) {
        bdaddr_to_string((const bt_bdaddr_t*)&opp.addr, bd_str, MAX_BD_STR_LEN);
        ALOGE(LOGTAG "%s: Already connected to %s", __FUNCTION__, bd_str);
        fprintf(stdout, "Already connecting to %s\n", bd_str);
        return false;
    }

    OI_STATUS ret = OI_OPPClient_Connect((OI_BD_ADDR*)&opp.addr,
        &opp.lowerProtocol, &opp.clientConnectionHandle,
        &ClientEventInd, &objSys);
    ALOGV(LOGTAG "%s: OI_OPPClient_Connect returned %d", __FUNCTION__, ret);
    if (ret) {
        bdaddr_to_string((const bt_bdaddr_t*)&opp.addr, bd_str, MAX_BD_STR_LEN);
        ALOGE(LOGTAG "%s: Failed to connect to %s", __FUNCTION__, bd_str);
        fprintf(stdout, "Failed to connect to %s\n", bd_str);
    } else {
         // start the profile connect timer
        alarm_set(opp_connect_timer, OPP_CONNECT_TIMEOUT_DELAY,
                            opp_connect_timer_expired, &opp.addr);
        setOppState(STATE_CONNECTING);
    }
    return ret;
}

bool Opp :: SendData()
{
    OI_OBEX_UNICODE name;
    name.str = (OI_CHAR16*)malloc(MAX_NAME_LEN);
    name.len = strlen(opp.fileName) + 1;
    OI_Utf8ToUtf16((const OI_UTF8*)opp.fileName,
                    MAX_NAME_LEN, name.str, name.len);
    OI_STATUS ret = OI_OPPClient_Push(opp.clientConnectionHandle, &name, (const OI_CHAR * )opp.mimeType);
    free(name.str);
    ALOGV(LOGTAG "%s: OI_OPPClient_Push returned %d", __FUNCTION__, ret);
    if (ret) {
        OI_OPPClient_Disconnect(opp.clientConnectionHandle);
    }
    return ret;
}

bool Opp :: HandleConnectTimeout(bt_bdaddr_t *addr)
{
    char bd_str[MAX_BD_STR_LEN];
    bdaddr_to_string((const bt_bdaddr_t*)addr, bd_str, MAX_BD_STR_LEN);
    fprintf(stdout, "Failed to Connect to %s due to ConnectionTimeout\n", bd_str);
    opp.clientConnectionHandle = NULL;
    setOppState(STATE_IDLE);
    opp.lowerProtocol.protocol = OI_OBEX_LOWER_NONE;
    uint8_t zero[sizeof(bt_bdaddr_t)] = { 0 };
    memcpy(&opp.addr, zero, sizeof(bt_bdaddr_t));
    return true;
}

bool Opp :: Disconnect()
{
    bool ret = true;
    char bd_str[MAX_BD_STR_LEN];

    if (getOppState() != STATE_CONNECTED ) {
        ALOGE(LOGTAG "%s: not connected", __FUNCTION__);
        fprintf(stdout, "Not connected \n");
        return false;
    }
    ALOGV(LOGTAG "%s", __FUNCTION__);

    OI_STATUS status = OI_OPPClient_Disconnect(opp.clientConnectionHandle);
    if (status != OI_STATUS_SUCCESS) {
        ret = false;
        char bd_str[MAX_BD_STR_LEN];
        bdaddr_to_string((const bt_bdaddr_t*)&opp.addr, bd_str, MAX_BD_STR_LEN);
        fprintf(stdout, "Failed to disconnect to %s\n", bd_str);
        ALOGE(LOGTAG "%s: Failed disconnect %s status: %d", __FUNCTION__, bd_str, status);
    }
    return ret;
}

bool Opp :: Abort()
{
    bool ret = true;
    OI_STATUS status = OI_STATUS_SUCCESS;
    ALOGV(LOGTAG "%s", __FUNCTION__);
    if (getOppState() != STATE_SENDING && getOppState() != STATE_RECEIVING) {
        ALOGE(LOGTAG "%s: Data transfer not ongoing, can't abort", __FUNCTION__);
        fprintf(stdout, "Data transfer not ongoing, can't abort\n");
        return false;
    }
    opp.abort = true;
    if (opp.clientConnectionHandle && getOppState() == STATE_SENDING)
        status = OI_OPPClient_Cancel(opp.clientConnectionHandle, &AbortCfmCb);
    else if (opp.srvConnectionHandle && getOppState() == STATE_RECEIVING)
        status = OI_OPPServer_ForceDisconnect(opp.srvConnectionHandle);
    if (status != OI_STATUS_SUCCESS) {
        ALOGE(LOGTAG "%s: Failed to abort ongoing operation, status: %d", __FUNCTION__, status);
        opp.abort = false;
        ret = false;
    } else {
        fprintf(stdout, "Abort in progress!!\n");
    }
    return ret;
}

bool Opp :: IncomingFileRsp(bool accept)
{
    bool ret = true;
    ALOGV(LOGTAG "%s", __FUNCTION__);
    if (getOppState() != STATE_RECEIVING) {
        ALOGE(LOGTAG "%s: File not being received", __FUNCTION__);
        return false;
    }
    if (accept) {
        OI_Time_Now(&serverStartTime);
        if (opp.defaultName) {
            OI_OBEX_UNICODE objName;
            /*
             * Rename object.
             */
            opp.defaultName = FALSE;
            FilenameToObjname(&objName, noNameFileStoreLocation);
            OI_OPPServer_AcceptPush(opp.srvConnectionHandle, &objName, TRUE);
        } else {
            OI_OBEX_UNICODE name;
            name.str = (OI_CHAR16*)malloc(MAX_NAME_LEN);
            name.len = strlen(opp.fileName) + 1;
            OI_Utf8ToUtf16((const OI_UTF8*)opp.fileName,
                            MAX_NAME_LEN, name.str, name.len);
            ALOGE(LOGTAG "%s: Accepting Incoming File", __FUNCTION__);
            OI_OPPServer_AcceptPush(opp.srvConnectionHandle, &name, TRUE);
            free(name.str);
        }
    } else {
        setOppState(STATE_CONNECTED);
        ALOGE(LOGTAG "%s: Rejecting Incoming File", __FUNCTION__);
        OI_OPPServer_AcceptPush(opp.srvConnectionHandle, NULL, FALSE);
    }
    return ret;
}

#ifdef __cplusplus
}
#endif
