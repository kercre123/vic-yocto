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
 * @file
 * @internal
 *
 * Phonebook Access Profile server.
 */

#define __OI_MODULE__ OI_MODULE_PBAP_SRV

#include "oi_pbap_server.h"
#include "oi_pbap_sys.h"
#include "oi_obexsrv.h"
#include "oi_std_utils.h"
#include "oi_memmgr.h"
#include "oi_debug.h"
#include "oi_assert.h"
#include "oi_bt_assigned_nos.h"
#include "oi_sdpdb.h"
#include "oi_sdp_utils.h"
#include "oi_argcheck.h"
#include "oi_dispatch.h"

#include "oi_init_flags.h"
#include "oi_config_table.h"
#include "oi_bt_profile_config.h"

#include "oi_bytestream.h"
#include "oi_unicode.h"

#include "oi_pbap_private.h"



/*****************************************************************
 *
 * Service record
 *
 *****************************************************************/

/** service class ID list */
static const OI_DATAELEM ServiceClassIDList[] = {
    OI_ELEMENT_UUID32(OI_UUID_PhonebookAccessServer)
};

/* protocol descriptor list */
static const OI_DATAELEM L2CAP_Descriptor[] = {
    OI_ELEMENT_UUID32(OI_UUID_L2CAP)
};

/** the RFCOMM server channel
 *      @note: This variable is static in order that its address can be known at
 *      compile time. This lets us put a constant pointer into the SDP descriptor.
 */
static OI_DATAELEM OurServerChannel;

static const OI_DATAELEM RFCOMM_Descriptor[] = {
    OI_ELEMENT_UUID32(OI_UUID_RFCOMM),
    OI_ELEMENT_REF(OurServerChannel)
};

static const OI_DATAELEM OBEX_Descriptor[] = {
    OI_ELEMENT_UUID32(OI_UUID_OBEX)
};

static const OI_DATAELEM ProtocolDescriptorList[] = {
    OI_ELEMENT_SEQ(L2CAP_Descriptor),
    OI_ELEMENT_SEQ(RFCOMM_Descriptor),
    OI_ELEMENT_SEQ(OBEX_Descriptor)
};

/* profile descriptor list */
static const OI_DATAELEM Profile0[] = {
    OI_ELEMENT_UUID32(OI_UUID_PhonebookAccess),
    OI_ELEMENT_UINT16(0x0100) //version 1.0
};

static const OI_DATAELEM ProfileDescriptorList[] = {
    OI_ELEMENT_SEQ(Profile0)
};

static OI_DATAELEM SupportedRepositories;

/** SDP attribute lists */
static const OI_SDPDB_ATTRIBUTE ServiceDescription[] = {
    { OI_ATTRID_ServiceClassIDList,                   OI_ELEMENT_SEQ(ServiceClassIDList) },
    { OI_ATTRID_ProtocolDescriptorList,               OI_ELEMENT_SEQ(ProtocolDescriptorList) },
    { OI_ATTRID_BluetoothProfileDescriptorList,       OI_ELEMENT_SEQ(ProfileDescriptorList) },
    { OI_ATTRID_SupportedRepositories,                OI_ELEMENT_REF(SupportedRepositories) }
};

/*
 * Note that the order of these values is important.
 */
typedef enum {
    PBAP_SERVER_IDLE               = 0,
    PBAP_SERVER_CONNECTED          = 1,
    PBAP_GETTING_PHONEBOOK         = 2,
    PBAP_GETTING_PHONEBOOK_SIZE    = 3,
    PBAP_GETTING_PHONEBOOK_LISTING = 4,
    PBAP_GETTING_VCARD             = 5
} PBAP_SERVER_STATE;



typedef enum {
    PBAP_SERVER_LISTING,
    PBAP_SERVER_VCARD,
    PBAP_SERVER_PHONEBOOK
} PBAP_SERVER_GET_OBJECT;


/*
 * Struct for an OBEX PBAP server connection.
 */

typedef struct {
    PBAP_SERVER_STATE state;                        /**< current state of the PBAP server */
    OI_PBAP_HANDLE file;                            /**< handle for file operations */
    const OI_PBAP_SERVER_FILESYS_FUNCTIONS *fops;   /**< file operations */

    OI_UINT16 maxRead;                              /**< maximum read for the current connection */

    OI_OBEXSRV_CONNECTION_HANDLE id;                /**< connection ID number */

    OI_PBAP_CONNECTION_IND connectInd;              /**< connection event callback */
    OI_PBAP_DISCONNECTION_IND disconnectInd;        /**< disconnect event callback */
    OI_PBAP_SERVER_GET_PHONEBOOK_SIZE_IND getPhonebookSize; /**< get phonebook size request event callback */

    OI_UINT32 srecHandle;                           /**< Service record handle for the PBAP service */
    OI_OBEX_SERVER_HANDLE serverHandle;             /**< OBEX server handle for the registered PBAP server */


    OI_PBAP_REPOSITORY repository;                  /**< Currently set repository */
    OI_PBAP_PHONEBOOK phonebook;                    /**< Currently set phonebook */
    OI_BOOL repositorySet;                          /**< TRUE if a valid repository is set */
    OI_BOOL phonebookSet;                           /**< TRUE if a valid phonebook is set */

    OI_UINT8 folderLevel;                           /**< 0 means at root folder */

    OI_CHAR16 *pathElements[4];
    OI_UINT16 pathElemCnt;

    OI_UINT32 entry;                                /**< vCard entry to retrieve */

    OI_BOOL reportMissedCalls;                      /**< flag indicating when to send newly missed calls to client */
    OI_UINT8 newMissedCalls;                        /**< number of newly missed calls */

    PBAP_SERVER_GET_OBJECT getObject;               /**< type of object to get */

    struct {
        OI_PBAP_ORDER_TAG_VALUES order;             /**< application parameter, Order: listing sort order */
        OI_BOOL orderSet;                           /**< flag indicating if the client set the Order parameter */

        OI_PBAP_SEARCH_ATTRIBUTE_TAG_VALUES searchAttribute;    /**< application parameter, SearchAttribute: specifies vCard field to search */
        OI_BOOL searchAttributeSet;                 /**< flag indicating if the client set the SearchAttribute parameter */

        OI_BYTE *searchValue;                       /**< application parameter, SearchValue: the search criteria */
        OI_UINT8 searchValueLen;                    /**< length of the search criteria */
        OI_BOOL  searchValueSet;                    /**< flag indicating if the client set the SearchValue parameter */

        OI_UINT16 maxListCount;                     /**< applicatoin parameter, MaxListCount: maximum number of vCards the client will accept */
        OI_BOOL   maxListCountSet;                  /**< flag indicating if the client set the MaxListCount application paramter */

        OI_UINT16 listStartOffset;                  /**< application parameter, ListStartOffset: first vCard entry the client wants to receive */
        OI_BOOL   listStartOffsetSet;               /**< flag indicating if the client set the ListStartOffset application parameter */

        OI_UINT64 filter;                           /**< application parameter, Filter: specifies which vCard fields to include in the data sent to the client */
        OI_BOOL   filterSet;                        /**< flag indicating if the client set the Filter application parameter */

        OI_PBAP_FORMAT_TAG_VALUES format;           /**< application parameter, Format: specifies the vCard format */
        OI_BOOL                   formatSet;        /**< flag indicating if the client set the Format application parameter */
    } appParams;

    OI_BOOL unauthorized;                           /**< flag indicating if connecting client needs to provid authentication */
    OI_BOOL incompleteGet;                          /**< flag indication if the server has more data to send */
} PBAP_SERVER;


static PBAP_SERVER *server;

/**
 * Connection policy.
 */
static const OI_CONNECT_POLICY  connectPolicy =
{
    OI_ELEMENT_UUID32(OI_UUID_PhonebookAccessServer),     /* OI_DATAELEM         serviceUuid           */
    FALSE,                              /* OI_BOOL             mustBeMaster          */
    NULL,                               /* OI_L2CAP_FLOWSPEC   *flowspec;            */
    0                                   /* OI_UINT8            powerSavingDisables ; */
};

/*
 * Struct for deferred fops->close() call
 */
typedef struct  {
    OI_PBAP_HANDLE fileHandle;
    OI_STATUS status;
    OI_PBAP_CONNECTION serverId;
} DEFERRED_FILE_CLOSE_ARGS;





#if defined(OI_DEBUG)
/**
 * ServerStateText()
 *
 * Helper function to convert the state to a human readable string for printing.
 */
static const OI_CHAR *ServerStateText(PBAP_SERVER_STATE state)
{
    switch (state) {
    case PBAP_SERVER_IDLE:                  return "PBAP_SERVER_IDLE";
    case PBAP_SERVER_CONNECTED:             return "PBAP_SERVER_CONNECTED";
    case PBAP_GETTING_PHONEBOOK:            return "PBAP_GETTING_PHONEBOOK";
    case PBAP_GETTING_PHONEBOOK_SIZE:       return "PBAP_GETTING_PHONEBOOK_SIZE";
    case PBAP_GETTING_PHONEBOOK_LISTING:    return "PBAP_GETTING_PHONEBOOK_LISTING";
    case PBAP_GETTING_VCARD:                return "PBAP_GETTING_VCARD";
    }
    return "<unknown state>";
}

/**
 * setState()
 *
 * Helper macro for setting the state.  (Written as a macro so that the
 * OI_DBG_PRINT1() reports the line in the function where the state changed
 * rather than the line in setState().
 */
#define setState(_server, _newState)                    \
    do {                                                \
        OI_DBG_PRINT1(("PBAP Server state %s ==> %s\n",      \
                     ServerStateText((_server)->state), \
                     ServerStateText(_newState)));      \
        (_server)->state = (_newState);                 \
    } while (0)
#else
/**
 * ServerStateText()
 *
 * Empty definition of above incase it is used by non-debug code.
 */
#define ServerStateText(state) ""

/**
 * setState()
 *
 * Non-debug version of the setState() macro.
 */
#define setState(_client, _newState)            \
    do {                                        \
        (_client)->state = (_newState);         \
    } while (0);
#endif


/*
 * dispatcher calls us so we can call close()
 */
static void deferredFileClose(DISPATCH_ARG *args)
{
    DEFERRED_FILE_CLOSE_ARGS *pCbArgs;

    pCbArgs = (DEFERRED_FILE_CLOSE_ARGS*)(args->data);
    server->fops->close(pCbArgs->fileHandle, pCbArgs->serverId, pCbArgs->status);
    if (server->appParams.searchValueSet) {
        OI_FreeIf(&server->appParams.searchValue);
    }
}

static void setupDeferredFileClose(OI_PBAP_HANDLE handle,
                                   OI_PBAP_CONNECTION serverId,
                                   OI_STATUS status)
{
    DEFERRED_FILE_CLOSE_ARGS closeArgs;
    DISPATCH_ARG dispatchArg;
    OI_STATUS retVal;

    closeArgs.fileHandle = handle;
    closeArgs.status = status;
    closeArgs.serverId = serverId;
    Dispatch_SetArg(dispatchArg, closeArgs);
    retVal = OI_Dispatch_RegisterFunc(deferredFileClose, &dispatchArg, NULL);
    if (!OI_SUCCESS(retVal)) {
        OI_SLOG_ERROR(retVal, ("Failed to register deferred close"));
    }
}


static OI_STATUS TokenizePath(const OI_OBEX_UNICODE *ustr, OI_CHAR16 **elements, OI_UINT16 *elemCnt)
{
    OI_UINT16 maxCnt = *elemCnt;
    OI_CHAR16 *path;

    if ((ustr->len == 0) || (ustr->len == 1 && ustr->str[0] == 0x0000)) {
        *elemCnt = 0;
        return OI_OK;
    }

    path = OI_Malloc((ustr->len + 1) * sizeof(OI_CHAR16));
    if (!path) {
        return OI_STATUS_OUT_OF_MEMORY;
    }


    OI_MemCopy(path, ustr->str, ustr->len * sizeof(OI_CHAR16));
    path[ustr->len] = 0;  /* Make sure the path is nul terminated. */

    if (path[0] == '/') {
        /* Invalid path */
        return OI_OBEX_NOT_FOUND;
    }

    *elemCnt = 0;
    elements[*elemCnt] = path;
    (*elemCnt)++;

    while (*path != '\0') {
        if (*path == '/') {
            if (*elemCnt == maxCnt) {
                 /* path too long; obviously invalid */
                return OI_OBEX_NOT_FOUND;
            }
            *path = '\0';
            path++;
            OI_DBG_PRINT2(("Tokenized path element: \"%S\"\n", elements[*elemCnt - 1]));
            elements[*elemCnt] = path;
            (*elemCnt)++;
        } else {
            path++;
        }
    }

    OI_DBG_PRINT2(("Tokenized final path element: \"%S\"\n", elements[*elemCnt - 1]));
    return OI_OK;
}


/******************************************************************************
 * Parse fileName into Phonebook files/directories
 */

static OI_STATUS GetRepository(OI_CHAR16 **elements, OI_UINT16 elemCnt, OI_PBAP_REPOSITORY *repo)
{
    if (server->repositorySet) {
        *repo = server->repository;
        return OI_OK;
    } else if (elemCnt >= 2) {
        if ((OI_StrcmpUtf16(elements[0], OI_PBAP_usim1) == 0) &&
            (OI_StrcmpUtf16(elements[1], OI_PBAP_utelecom) == 0)) {
            *repo = OI_PBAP_SIM1_REPOSITORY;
            return OI_OK;
        } else if (OI_StrcmpUtf16(elements[0], OI_PBAP_utelecom) == 0) {
            /* Even if server->repositorySet is FALSE, server->repository will
             * still refer to OI_PBAP_SIM1_REPOSITORY if the client has sent a
             * SETPATH NAME="SIM1". */
            *repo = server->repository;
            return OI_OK;
        }
    }
    return OI_OBEX_NOT_FOUND;
}


static OI_STATUS LookupPhonebook(const OI_CHAR16 *pbStr, OI_PBAP_PHONEBOOK *pb)
{
    OI_INT i;

    OI_DBG_PRINT1(("LookupPhonebook(pbStr = \"%S\", <*pb = %x>)", pbStr, pb));

    for (i = 0; i < OI_PBAP_INVALID_PHONEBOOK; i++) {
        if (OI_StrncmpUtf16(pbStr, OI_PBAP_upbdirs[i], (OI_UINT16)(OI_PBAP_upbdirsizes[i] - 1)) == 0) {
            *pb = (OI_PBAP_PHONEBOOK)i;
            return OI_OK;
        }
    }
    return OI_OBEX_NOT_FOUND;
}


#define PRINT_BADPATH(_msg, _server)                                    \
    OI_DBG_PRINT1(("\t%s: \"%S%s%S%s%S%s%S\" from \"%s%s%S\"\n",     \
                      (_msg),                                           \
                      (_server)->pathElements[0],                       \
                      ((_server)->pathElemCnt) > 1 ? "/" : "",          \
                      ((_server)->pathElemCnt) > 1 ? (_server)->pathElements[1] : (const OI_CHAR16*)"\0", \
                      ((_server)->pathElemCnt) > 2 ? "/" : "",          \
                      ((_server)->pathElemCnt) > 2 ? (_server)->pathElements[2] : (const OI_CHAR16*)"\0", \
                      ((_server)->pathElemCnt) > 3 ? "/" : "",          \
                      ((_server)->pathElemCnt) > 3 ? (_server)->pathElements[3] : (const OI_CHAR16*)"\0", \
                      (_server)->repositorySet ? (((_server)->repository == OI_PBAP_SIM1_REPOSITORY) ? "SIM1/telecom" : "telecom") : "<root>", \
                      (_server)->phonebookSet ? "/" : "",               \
                      (_server)->phonebookSet ? OI_PBAP_upbdirs[(_server)->phonebook] : (const OI_CHAR16*)"\0")); \


static OI_STATUS ParseEntryName(OI_PBAP_REPOSITORY *repo, OI_PBAP_PHONEBOOK *pb, OI_UINT32 *entry)
{
    OI_UINT pos;
    OI_UINT32 e;  /* Use 32-bit int to check for overflow */
    OI_UINT32 digit;
    const OI_CHAR16 *fn;
    OI_UINT fnLen;
    OI_STATUS status = OI_OK;
    OI_UINT vcfLen = OI_StrLenUtf16(OI_PBAP_vcf);

    status = GetRepository(server->pathElements, server->pathElemCnt, repo);
    if ((server->pathElemCnt == 4) && (*repo == OI_PBAP_LOCAL_REPOSITORY)) {
        status = OI_OBEX_NOT_FOUND;
    }
    if (!OI_SUCCESS(status)) {
        OI_SLOG_WARNING(status, ("Repository check failed"));
        goto out;
    }

    if (server->phonebookSet && (server->pathElemCnt == 1)) {
        *pb = server->phonebook;
    } else if (!server->phonebookSet && (server->pathElemCnt >= 2)) {
        status = LookupPhonebook(server->pathElements[server->pathElemCnt - 2], pb);
        if (!OI_SUCCESS(status)) {
            OI_SLOG_WARNING(status, ("Phonebook lookup failed"));
            goto out;
        }
    } else {
        OI_SLOG_WARNING(OI_OBEX_NOT_FOUND, ("Not enough path components"));
        status = OI_OBEX_NOT_FOUND;
        goto out;
    }


    /* vCard entries can only be "<number>.vcf", where number is between 0 and
     * 0xffffffff */

    fn = server->pathElements[server->pathElemCnt - 1];
    fnLen = OI_StrLenUtf16(fn);

    /* Verify the last 4 characters match ".vcf" */
    if (OI_StrcmpUtf16(&fn[fnLen - vcfLen], OI_PBAP_vcf) != 0) {
        OI_SLOG_WARNING(OI_OBEX_NOT_FOUND,
                        ("Requested vCard name is not long enough for \".vcf\""));
        status = OI_OBEX_NOT_FOUND;
        goto out;
    }

    /* Make sure the name isn't obviously too large. */
    if (fnLen > sizeof("1234567890.vcf")) {
        OI_SLOG_WARNING(OI_OBEX_NOT_FOUND,
                        ("Requested vCard name is obviously too long: %d should be less than 14",
                         fnLen));
        status = OI_OBEX_NOT_FOUND;
        goto out;
    }

    e = 0;

    for (pos = 0; pos < (fnLen - vcfLen); pos++) {
        digit = fn[pos] - '0';

        /* Validate the digit */
        if (digit > 9) {
            /* Handle upper case */
            digit = 10 + fn[pos] - 'A';
            if(digit > 15 || digit < 10) {
                /* Handle lower case */
                digit = 10 + fn[pos] - 'a';
                if(digit > 15 || digit < 10) {
                    OI_SLOG_WARNING(OI_OBEX_NOT_FOUND,
                                    ("Invalid character found in vCard file name: \"%c\" (0x%2x)",
                                     digit + '0', digit + '0'));
                    status = OI_OBEX_NOT_FOUND;
                    goto out;
                }
            }
        }
        e *= 16;
        e += digit;
    }

    *entry = (OI_UINT32)e;

  out:
    if (!OI_SUCCESS(status)) {
        PRINT_BADPATH("Remote PBAP client requested invalid vCard file", server);
    }

    return status;
}

static OI_STATUS ParsePhonebookvCard(OI_PBAP_REPOSITORY *repo, OI_PBAP_PHONEBOOK *pb)
{
    OI_STATUS status = OI_OK;


    if (server->phonebookSet && (server->pathElemCnt < 2)) {
        OI_SLOG_WARNING(OI_OBEX_NOT_FOUND,
                        ("Can't get the phonebook file while in a phonebook directory"));
        status = OI_OBEX_NOT_FOUND;
        goto out;
    }

    status = GetRepository(server->pathElements, server->pathElemCnt, repo);
    if ((server->pathElemCnt == 3) && (*repo == OI_PBAP_LOCAL_REPOSITORY)) {
        status = OI_OBEX_NOT_FOUND;
    }
    if (!OI_SUCCESS(status)) {
        OI_SLOG_WARNING(status, ("Repository check failed"));
        goto out;
    }

    status = LookupPhonebook(server->pathElements[server->pathElemCnt - 1], pb);
    if (!OI_SUCCESS(status)) {
        OI_SLOG_WARNING(status, ("Phonebook lookup failed"));
        goto out;
    }

    if (OI_StrLenUtf16(server->pathElements[server->pathElemCnt - 1]) != (OI_PBAP_upbdirsizes[*pb] + OI_StrLenUtf16(OI_PBAP_vcf) - 1)) {
        OI_SLOG_WARNING(OI_OBEX_NOT_FOUND, ("Phonebook filename length is wrong"));
        status = OI_OBEX_NOT_FOUND;
    }

  out:
    if (!OI_SUCCESS(status)) {
        PRINT_BADPATH("Remote PBAP client requested invalid phonebook file", server);
    }
    return status;
}


static OI_STATUS ParsePhonebookDir(OI_PBAP_REPOSITORY *repo, OI_PBAP_PHONEBOOK *pb)
{
    OI_STATUS status = OI_OK;

    if (server->phonebookSet) {
        if (server->pathElemCnt > 0) {
            OI_SLOG_WARNING(OI_OBEX_NOT_FOUND, ("Already in a phonebook directory"));
            status = OI_OBEX_NOT_FOUND;
        } else {
            *repo = server->repository;
            *pb = server->phonebook;
        }
    } else {
        status = GetRepository(server->pathElements, server->pathElemCnt, repo);
        if ((server->pathElemCnt == 3) && (*repo == OI_PBAP_LOCAL_REPOSITORY)) {
            status = OI_OBEX_NOT_FOUND;
        }
        if (!OI_SUCCESS(status)) {
            OI_SLOG_WARNING(status, ("Repository check failed"));
            goto out;
        }

        status = LookupPhonebook(server->pathElements[server->pathElemCnt - 1], pb);
        if (!OI_SUCCESS(status)) {
            OI_SLOG_WARNING(status, ("Phonebook lookup failed"));
            goto out;
        }

        if (OI_StrLenUtf16(server->pathElements[server->pathElemCnt - 1]) != (OI_UINT16)(OI_PBAP_upbdirsizes[*pb] - 1)) {
            OI_SLOG_WARNING(OI_OBEX_NOT_FOUND, ("Phonebook dirname length is wrong"));
            status = OI_OBEX_NOT_FOUND;
        }
    }

  out:
    if (!OI_SUCCESS(status)) {
        PRINT_BADPATH("Remote PBAP client requested invalid phonebook dir", server);
    }
    return status;
}


/******************************************************************************
 * Parameter parsing routines.
 */

static OI_STATUS ParseAppParams(const OI_OBEX_BYTESEQ *appParams)
{
    OI_BYTE_STREAM bs;
    OI_PBAP_APPLICATION_PARAM_TAG_IDS tag;
    OI_UINT8 len = 0;
    OI_STATUS status = OI_OK;
    OI_UINT8 rawByte = 0;

    ByteStream_Init(bs, appParams->data, appParams->len);
    ByteStream_Open(bs, BYTESTREAM_READ);

    while (ByteStream_NumReadBytesAvail(bs) > 0) {
        ByteStream_GetUINT8_Checked(bs, rawByte);
        ByteStream_GetUINT8_Checked(bs, len);
        tag = (OI_PBAP_APPLICATION_PARAM_TAG_IDS)rawByte;  /* Some compilers' type checking is too strong. */

        switch (tag) {
        case OI_PBAP_TAG_ID_ORDER:
            if (len != sizeof(OI_UINT8)) {
                OI_SLOG_ERROR(OI_OBEX_PRECONDITION_FAILED,
                              ("PBAP: Order value has wrong length"));
                status = OI_OBEX_PRECONDITION_FAILED;
                    goto out;
            }
            if (server->appParams.orderSet) {
                OI_SLOG_WARNING(OI_STATUS_NONE,
                                ("PBAP: Order already set, ignoring duplicate tag"));
                ByteStream_Skip_Checked(bs, len);
            } else {
                ByteStream_GetUINT8_Checked(bs, rawByte);
                server->appParams.order = (OI_PBAP_ORDER_TAG_VALUES)rawByte;
                if ((server->appParams.order != OI_PBAP_ORDER_INDEXED) &&
                    (server->appParams.order != OI_PBAP_ORDER_ALPHANUMERIC) &&
                    (server->appParams.order != OI_PBAP_ORDER_PHONETIC)) {
                    OI_SLOG_ERROR(OI_OBEX_PRECONDITION_FAILED,
                                  ("PBAP: client sent invalid order value: %d\n",
                                   server->appParams.order));
                    status = OI_OBEX_PRECONDITION_FAILED;
                    goto out;
                }
                server->appParams.orderSet = TRUE;
            }
            break;

        case OI_PBAP_TAG_ID_SEARCH_VALUE:
            if (server->appParams.searchValueSet) {
                OI_SLOG_WARNING(OI_STATUS_NONE,
                                ("SearchValue already set, ignoring duplicate tag"));
                ByteStream_Skip_Checked(bs, len);
            } else {
                server->appParams.searchValueLen = len;
                server->appParams.searchValue = OI_Malloc(len);
                if (!server->appParams.searchValue) {
                    OI_SLOG_ERROR(OI_STATUS_OUT_OF_MEMORY,
                                  ("Failed to allocate memory for SearchValue"));
                    status = OI_STATUS_OUT_OF_MEMORY;
                    goto out;
                }
                ByteStream_GetBytes_Checked(bs, server->appParams.searchValue, len);
                server->appParams.searchValueSet = TRUE;
            }
            break;

        case OI_PBAP_TAG_ID_SEARCH_ATTRIBUTE:
            if (server->appParams.searchAttributeSet) {
                OI_SLOG_WARNING(OI_STATUS_NONE,
                                ("SearchAttribute already set, ignoring duplicate tag"));
                ByteStream_Skip_Checked(bs, len);
            } else {
                OI_UINT8 sAttr = 0;
                /* Convert the text representation of the search attribute to
                 * a much easier enum value.
                 */
                ByteStream_GetUINT8_Checked(bs, sAttr);
                server->appParams.searchAttribute = (OI_PBAP_SEARCH_ATTRIBUTE_TAG_VALUES)sAttr;
                server->appParams.searchAttributeSet = TRUE;
            }
            break;

        case OI_PBAP_TAG_ID_MAX_LIST_COUNT:
            if (len != sizeof(OI_UINT16)) {
                OI_SLOG_ERROR(OI_OBEX_PRECONDITION_FAILED,
                              ("PBAP: MaxListCount value has wrong length"));
                status = OI_OBEX_PRECONDITION_FAILED;
                    goto out;
            }
            if (server->appParams.maxListCountSet) {
                OI_SLOG_WARNING(OI_STATUS_NONE,
                                ("maxListCount already set, ignoring duplicate tag"));
                ByteStream_Skip_Checked(bs, len);
            } else {
                ByteStream_GetUINT16_Checked(bs, server->appParams.maxListCount,
                                             OI_BIG_ENDIAN_BYTE_ORDER);
                server->appParams.maxListCountSet = TRUE;
            }
            break;

        case OI_PBAP_TAG_ID_LIST_START_OFFSET:
            if (len != sizeof(OI_UINT16)) {
                OI_SLOG_ERROR(OI_OBEX_PRECONDITION_FAILED,
                              ("PBAP: ListStartOffset value has wrong length"));
                status = OI_OBEX_PRECONDITION_FAILED;
                    goto out;
            }
            if (server->appParams.listStartOffsetSet) {
                OI_SLOG_WARNING(OI_STATUS_NONE,
                                ("ListStartOffset already set, ignoring duplicate tag"));
                ByteStream_Skip_Checked(bs, len);
            } else {
                ByteStream_GetUINT16_Checked(bs, server->appParams.listStartOffset,
                                             OI_BIG_ENDIAN_BYTE_ORDER);
                server->appParams.listStartOffsetSet = TRUE;
            }
            break;

        case OI_PBAP_TAG_ID_FILTER:
            if (len != sizeof(OI_UINT64)) {
                OI_SLOG_ERROR(OI_OBEX_PRECONDITION_FAILED,
                              ("PBAP: Filter value has wrong length"));
                status = OI_OBEX_PRECONDITION_FAILED;
                    goto out;
            }
            if (server->appParams.filterSet) {
                OI_SLOG_WARNING(OI_STATUS_NONE,
                                ("Filter already set, ignoring duplicate tag"));
                ByteStream_Skip_Checked(bs, len);
            } else {
                ByteStream_GetUINT32_Checked(bs, server->appParams.filter.I1,
                                             OI_BIG_ENDIAN_BYTE_ORDER);
                ByteStream_GetUINT32_Checked(bs, server->appParams.filter.I2,
                                             OI_BIG_ENDIAN_BYTE_ORDER);
                server->appParams.filterSet = TRUE;
            }
            break;

        case OI_PBAP_TAG_ID_FORMAT:
            if (len != sizeof(OI_UINT8)) {
                OI_SLOG_ERROR(OI_OBEX_PRECONDITION_FAILED, ("PBAP: Format has wrong length"));
                status = OI_OBEX_PRECONDITION_FAILED;
                    goto out;
            }
            if (server->appParams.formatSet) {
                OI_SLOG_WARNING(OI_STATUS_NONE,
                                ("PBAP: Format already set, ignoring duplicate tag"));
                ByteStream_Skip_Checked(bs, len);
            } else {
                ByteStream_GetUINT8_Checked(bs, rawByte);
                server->appParams.format = (OI_PBAP_FORMAT_TAG_VALUES)rawByte;
                if ((server->appParams.format != OI_PBAP_FORMAT_VCARD_2_1) &&
                    (server->appParams.format != OI_PBAP_FORMAT_VCARD_3_0)) {
                    OI_SLOG_ERROR(OI_OBEX_PRECONDITION_FAILED,
                                  ("PBAP: client sent invalid Format: %d\n",
                                   server->appParams.format));
                    status = OI_OBEX_PRECONDITION_FAILED;
                    goto out;
                }
                server->appParams.formatSet = TRUE;
            }
            break;

        default:
            break;
        }
    }

    if (ByteStream_Error(bs)) {
        OI_SLOG_ERROR(OI_OBEX_PRECONDITION_FAILED,
                     ("PBAP: Application parameters are corrupted"));
        status = OI_OBEX_PRECONDITION_FAILED;
    }

  out:
    ByteStream_Close(bs);

    return status;
}


static OI_STATUS ParseObexHeader(const OI_OBEX_HEADER_LIST *cmdHeaders)
{
    OI_INT i;
    OI_OBEX_HEADER *hdr;
    OI_OBEX_BYTESEQ *type = NULL;
    OI_STATUS status = OI_OK;

    OI_PBAP_DumpObexHeaders(cmdHeaders);

    for (i = 0; i < cmdHeaders->count; ++i) {
        hdr = &cmdHeaders->list[i];

        switch (hdr->id) {
        case OI_OBEX_HDR_NAME:
            if (server->pathElemCnt == 0) {
                if (hdr->val.name.str) {
                    status = TokenizePath(&hdr->val.name, server->pathElements, &server->pathElemCnt);
                    if (!OI_SUCCESS(status)) {
                        goto out;
                    }
                } else {
                    server->pathElemCnt = 0;
                }
            } else {
                OI_SLOG_WARNING(OI_STATUS_NONE,
                                ("File name received twice.  Ignoring duplicate"));
            }
            break;

        case OI_OBEX_HDR_TYPE:
            type = &hdr->val.type;
            /* Only 3 OBEX types are supported by PBAP:
             *     - x-bt/vcard
             *     - x-bt/vcard-listing
             *     - x-bt/phonebook
             */
            if (type->len > 0 && type->data) {
                if (OI_StrncmpInsensitive((OI_CHAR*)type->data,
                                          OI_PBAP_VCARD_LISTING_TYPE,
                                          sizeof(OI_PBAP_VCARD_LISTING_TYPE) - 1) == 0) {
                    OI_DBG_PRINT2(("Getting a Listing..."));
                    server->getObject = PBAP_SERVER_LISTING;
                } else if (OI_StrncmpInsensitive((OI_CHAR*)type->data,
                                                 OI_PBAP_VCARD_TYPE,
                                                 sizeof(OI_PBAP_VCARD_TYPE) - 1) == 0) {
                    OI_DBG_PRINT2(("Getting a vCard..."));
                    server->getObject = PBAP_SERVER_VCARD;
                } else if (OI_StrncmpInsensitive((OI_CHAR*)type->data,
                                                 OI_PBAP_PHONEBOOK_TYPE,
                                                 sizeof(OI_PBAP_PHONEBOOK_TYPE) - 1) == 0) {
                    OI_DBG_PRINT2(("Getting a Phonebook..."));
                    server->getObject = PBAP_SERVER_PHONEBOOK;
                } else {
                    /* Invalid type requested. */
                    OI_DBG_PRINT2(("Getting nothing... Invalid type: \"%s\"", type->data));
                    status = OI_OBEX_ACCESS_DENIED;
                    goto out;
                }
            } else {
                /* Invalid type, cannot be empty. */
                OI_SLOG_WARNING(OI_OBEX_ACCESS_DENIED,
                                ("Getting nothing... Invalid type: \"%s\"", type->data));
                status = OI_OBEX_ACCESS_DENIED;
                goto out;
            }
            break;

        case OI_OBEX_HDR_APPLICATION_PARAMS:
            status = ParseAppParams(&hdr->val.applicationParams);
            if (!OI_SUCCESS(status)) {
                goto out;
            }
            break;

        default:
            break;
        }
    }

  out:
    return status;
}


void OI_PBAPServer_SetNewMissedCalls(OI_UINT8 newMissedCalls)
{
    OI_TRACE_USER(("OI_PBAPServer_SetNewMissedCalls(newMissedCalls = %d)", newMissedCalls));
    server->newMissedCalls = newMissedCalls;
}


/******************************************************************************
 * Get phonebook size routines
 */
OI_STATUS OI_PBAPServer_GetPhonebookSizeRsp(OI_PBAP_CONNECTION connectionId,
                                            OI_UINT16 size,
                                            OI_STATUS getStatus)
{
    static OI_OBEX_HEADER hdr;
    static OI_OBEX_HEADER_LIST hdrList;
    static OI_BYTE_STREAM bs;
    static OI_BYTE buffer[(2 * OI_OBEX_APPLICATION_PARAMETER_PREFIX_LEN) +
                          sizeof(OI_UINT16) + sizeof(OI_UINT8)];
    OI_STATUS status = OI_OK;
    int param_cnt = 0;

    OI_TRACE_USER(("OI_PBAPServer_GetPhonebookSizeRsp(connectionId = %d, size = %d, getStatus = %d)",
                 connectionId, size, getStatus));

    hdrList.list = &hdr;
    hdrList.count = 1;

    if (!OI_SUCCESS(getStatus)) {
        /* For whatever reason the application was unable to provide a size so
         * tell the client that the service is unavailable.
         */
        setState(server, PBAP_SERVER_CONNECTED);
        status = OI_OBEXSRV_GetResponse(server->id, NULL, OI_OBEX_SERVICE_UNAVAILABLE);
    } else {
        ByteStream_Init(bs, buffer, sizeof(buffer));
        ByteStream_Open(bs, BYTESTREAM_WRITE);

        ByteStream_PutUINT8(bs, OI_PBAP_TAG_ID_PHONEBOOK_SIZE);
        ByteStream_PutUINT8(bs, 2);
        ByteStream_PutUINT16(bs, size, OI_BIG_ENDIAN_BYTE_ORDER);
        param_cnt += OI_OBEX_APPLICATION_PARAMETER_PREFIX_LEN + sizeof(OI_UINT16);

        if (server->reportMissedCalls) {
            ByteStream_PutUINT8(bs, OI_PBAP_TAG_ID_NEW_MISSED_CALLS);
            ByteStream_PutUINT8(bs, 1);
            ByteStream_PutUINT8(bs, server->newMissedCalls);
            param_cnt += OI_OBEX_APPLICATION_PARAMETER_PREFIX_LEN + sizeof(OI_UINT8);
        }

        hdr.id = OI_OBEX_HDR_APPLICATION_PARAMS;
        hdr.val.applicationParams.len = param_cnt;
        hdr.val.applicationParams.data = ByteStream_GetDataPointer(bs);

        ByteStream_Close(bs);

        OI_PBAP_DumpObexHeaders(&hdrList);

        setState(server, PBAP_SERVER_CONNECTED);
        status = OI_OBEXSRV_GetResponse(server->id, &hdrList, OI_OK);
    }

    return status;
}



static OI_STATUS GetPhonebookSize(void)
{
    OI_PBAP_REPOSITORY repo;
    OI_PBAP_PHONEBOOK pb;
    OI_STATUS status;

    OI_TRACE_USER(("GetPhonebookSize(<void>)"));

    setState(server, PBAP_GETTING_PHONEBOOK_SIZE);

    if (server->getObject == PBAP_SERVER_LISTING) {
        status = ParsePhonebookDir(&repo, &pb);
        if (!OI_SUCCESS(status)) {
            return status;
        }
    } else {
        status = ParsePhonebookvCard(&repo, &pb);
        if (!OI_SUCCESS(status)) {
            return status;
        }
    }

    if (pb == OI_PBAP_MISSED_CALLS_HISTORY) {
        server->reportMissedCalls = TRUE;
    }

    server->getPhonebookSize(server->id, repo, pb);

    return OI_OK;
}


/******************************************************************************
 * Phonebook file related routines.
 */

static void FileReadCfm(OI_PBAP_HANDLE handle,
                        OI_BYTE *data,
                        OI_UINT16 len,
                        OI_STATUS cfmStatus,
                        OI_PBAP_CONNECTION pbapConnection)
{
    static OI_OBEX_HEADER hdr;
    static OI_OBEX_HEADER_LIST hdrList;
    OI_STATUS status;

    OI_TRACE_USER(("FileReadCfm(<*handle = %x>, <*data = %x>, len = %d, cfmStatus = %d, pbapConnection = %d)",
                 handle, data, len, cfmStatus, pbapConnection));

    if (!OI_SUCCESS(cfmStatus) && (cfmStatus != OI_STATUS_END_OF_FILE)) {
        /*
         * File read error: close the file and report error to obex.
         */
        OI_PBAP_HANDLE fileSv = server->file;

        OI_SLOG_ERROR(cfmStatus, ("File read error"));
        server->file = NULL;
        setupDeferredFileClose(fileSv, server->id, cfmStatus);
        status = OI_OBEXSRV_GetResponse(server->id, NULL, cfmStatus);
        if (!OI_SUCCESS(status)) {
            OI_SLOG_ERROR(status, ("Failed to inform client of Read Failure (%d)", cfmStatus));
        }
        setState(server, PBAP_SERVER_CONNECTED);
        return;
    }

    if (cfmStatus == OI_STATUS_END_OF_FILE) {
        hdr.id = OI_OBEX_HDR_END_OF_BODY;
        status = OI_OK;
    } else {
        hdr.id = OI_OBEX_HDR_BODY;
        status = OI_OBEX_CONTINUE;
    }

    hdr.val.body.data = data;
    hdr.val.body.len = len;
    hdrList.list = &hdr;
    hdrList.count = 1;

    OI_PBAP_DumpObexHeaders(&hdrList);

    status = OI_OBEXSRV_GetResponse(server->id, &hdrList, status);
    if (!OI_SUCCESS(status)) {
        OI_PBAP_HANDLE fileSv = server->file;

        server->file = NULL;
        setupDeferredFileClose(fileSv, server->id, status);

        setState(server, PBAP_SERVER_CONNECTED);
    }
}


static void OpenCfm(OI_PBAP_HANDLE handle,
                    OI_STATUS cfmStatus,
                    OI_PBAP_CONNECTION pbapConnection)
{
    static OI_OBEX_HEADER hdr;
    static OI_OBEX_HEADER_LIST hdrList;
    OI_STATUS status = cfmStatus;

    OI_TRACE_USER(("OpenCfm(<*handle = %x>, cfmStatus = %d, pbapConnection = %d)",
                 handle, status, pbapConnection));

    OI_ASSERT(NULL != handle);
    if (NULL == handle) {
        OI_SLOG_ERROR(OI_STATUS_INVALID_PARAMETERS, ("Invalid handle"));
        status = OI_STATUS_INVALID_PARAMETERS;
    }

    if (!OI_SUCCESS(status)) {
        /*
         * File was not opened so it doesn't need to be closed but we need to
         * report the error to obex.
         */
        setState(server, PBAP_SERVER_CONNECTED);
        status = OI_OBEXSRV_GetResponse(server->id, NULL, status);
        if (!OI_SUCCESS(status)) {
            OI_SLOG_ERROR(status, ("Failed to inform the client of the open failure (%d)",
                          cfmStatus));
        }

        if (server->appParams.searchValueSet) {
            OI_DBG_PRINT2(("freeing server->appParams.searchValue"));
            server->appParams.searchValueSet = FALSE;
            OI_FreeIf(&server->appParams.searchValue);
        }
        return;
    }

    server->file = handle;

    hdrList.list = &hdr;
    hdrList.count = 0;

    if ((server->state == PBAP_GETTING_PHONEBOOK ||
         server->state == PBAP_GETTING_PHONEBOOK_LISTING) &&
        server->reportMissedCalls) {
        static OI_BYTE_STREAM bs;
        static OI_BYTE buffer[OI_OBEX_APPLICATION_PARAMETER_PREFIX_LEN + sizeof(OI_UINT8)];

        ByteStream_Init(bs, buffer, sizeof(buffer));
        ByteStream_Open(bs, BYTESTREAM_WRITE);

        ByteStream_PutUINT8(bs, OI_PBAP_TAG_ID_NEW_MISSED_CALLS);
        ByteStream_PutUINT8(bs, 1);
        ByteStream_PutUINT8(bs, server->newMissedCalls);

        hdr.id = OI_OBEX_HDR_APPLICATION_PARAMS;
        hdr.val.applicationParams.len = OI_OBEX_APPLICATION_PARAMETER_PREFIX_LEN + sizeof(OI_UINT8);
        hdr.val.applicationParams.data = ByteStream_GetDataPointer(bs);

        ByteStream_Close(bs);

        hdrList.count++;
    }

    OI_PBAP_DumpObexHeaders(&hdrList);

    status = OI_OBEXSRV_GetResponse(server->id, &hdrList, OI_OBEX_CONTINUE);
    if (!OI_SUCCESS(status)) {
        /*
         * Close the file and cleanup.  Cannot call application on application
         * thread, so put the fops close() on dispatcher.
         */
        setupDeferredFileClose(server->file, server->id, status);
        server->file = NULL;
        setState(server, PBAP_SERVER_CONNECTED);
    }
}


static OI_STATUS OpenDataHandle(void)
{
    OI_PBAP_REPOSITORY repo;
    OI_PBAP_PHONEBOOK pb;
    OI_UINT32 entry = 0; /* initialize to avoid compiler warnings */
    OI_STATUS status = OI_OK;

    if (server->getObject == PBAP_SERVER_LISTING) {
        status = ParsePhonebookDir(&repo, &pb);
        if (!OI_SUCCESS(status)) {
            goto out;
        }

        if (pb == OI_PBAP_MISSED_CALLS_HISTORY) {
            server->reportMissedCalls = TRUE;
        }

        setState(server, PBAP_GETTING_PHONEBOOK_LISTING);
        status = server->fops->listPB(repo,
                                      pb,
                                      server->appParams.order,
                                      server->appParams.searchAttribute,
                                      server->appParams.searchValue,
                                      server->appParams.searchValueLen,
                                      server->appParams.maxListCount,
                                      server->appParams.listStartOffset,
                                      OpenCfm,
                                      (OI_PBAP_CONNECTION)server->id);
    } else if (server->getObject == PBAP_SERVER_VCARD) {
        status = ParseEntryName(&repo, &pb, &entry);

        if (OI_SUCCESS(status)) {
            setState(server, PBAP_GETTING_VCARD);
            status = server->fops->openvCard(repo,
                                             pb,
                                             entry,
                                             &server->appParams.filter,
                                             server->appParams.format,
                                             OpenCfm,
                                             (OI_PBAP_CONNECTION)server->id);
        }
    } else {
        status = ParsePhonebookvCard(&repo, &pb);
        if (!OI_SUCCESS(status)) {
            goto out;
        }

        if (pb == OI_PBAP_MISSED_CALLS_HISTORY) {
            server->reportMissedCalls = TRUE;
        }

        setState(server, PBAP_GETTING_PHONEBOOK);
        status = server->fops->openPB(repo,
                                      pb,
                                      &server->appParams.filter,
                                      server->appParams.format,
                                      server->appParams.maxListCount,
                                      server->appParams.listStartOffset,
                                      OpenCfm,
                                      (OI_PBAP_CONNECTION)server->id);
    }

  out:
    if (!OI_SUCCESS(status)) {
        OI_SLOG_WARNING(status, ("Opening a handle for requested %s in %s-%s",
                                 ((server->getObject == PBAP_SERVER_LISTING) ? "Phonebook Listing"     :
                                  (server->getObject == PBAP_SERVER_VCARD)   ? "Phonebook Entry vCard" : "Phonebook vCard"),
                                 ((repo == OI_PBAP_LOCAL_REPOSITORY) ? "local" :
                                  (repo == OI_PBAP_SIM1_REPOSITORY)  ? "SIM1"  : "Invalid"),
                                 ((pb == OI_PBAP_MAIN_PHONEBOOK)         ? "pb"  :
                                  (pb == OI_PBAP_INCOMING_CALLS_HISTORY) ? "ich" :
                                  (pb == OI_PBAP_OUTGOING_CALLS_HISTORY) ? "och" :
                                  (pb == OI_PBAP_MISSED_CALLS_HISTORY)   ? "mch" :
                                  (pb == OI_PBAP_COMBINED_CALLS_HISTORY) ? "cch" : "Invalid")));
        setState(server, PBAP_SERVER_CONNECTED);
    }

    return status;
}

/******************************************************************************
 * Routine for rejecting PUT request from the client.
 */
static OI_STATUS ServerPutInd(OI_OBEXSRV_CONNECTION_HANDLE connectionId,
                              OI_OBEX_HEADER_LIST *cmdHeaders,
                              OI_STATUS obexStatus)
{

    OI_TRACE_USER(("ServerPutInd(connectionId = %d, <*cmdHeaders = %x>, obexStatus = %d)",
                      connectionId, cmdHeaders, obexStatus));

    if (OI_SUCCESS(obexStatus) || (obexStatus == OI_OBEX_CONTINUE)) {
        OI_SLOG_WARNING(OI_OBEX_BAD_REQUEST,
                        ("Invalid PUT request"));
        OI_DBG_PRINT1(("ServerPutInd terminated with status: OI_OBEX_BAD_REQUEST;!"));
        return OI_OBEX_BAD_REQUEST;
    }

    return obexStatus;
}

/******************************************************************************
 * Routines for basic GET indication from the client.
 */

static void ClearAppParams(void)
{
    /* The PBAP spec doesn't clearly indicate that the "Type" OBEX header is
     * required or what to assume if it is missing, so we'll default the
     * operation of the PBAP server to be as if it received the "x-bt/vcard"
     * type. */
    server->getObject = PBAP_SERVER_VCARD;

    server->reportMissedCalls = FALSE;

    server->appParams.orderSet           = FALSE;
    server->appParams.searchAttributeSet = FALSE;
    server->appParams.searchValueSet     = FALSE;
    server->appParams.maxListCountSet    = FALSE;
    server->appParams.listStartOffsetSet = FALSE;
    server->appParams.filterSet          = FALSE;
    server->appParams.formatSet          = FALSE;

    server->appParams.order           = OI_PBAP_ORDER_INDEXED;
    OI_FreeIf(&server->appParams.searchValue);
    server->appParams.searchValueLen  = 0;
    server->appParams.searchAttribute = OI_PBAP_SEARCH_ATTRIBUTE_NAME;
    server->appParams.maxListCount    = 65535;
    server->appParams.listStartOffset = 0;
    server->appParams.filter.I1       = 0;
    server->appParams.filter.I2       = 0;
    server->appParams.format          = OI_PBAP_FORMAT_VCARD_2_1;
}

static OI_STATUS ServerGetInd(OI_OBEXSRV_CONNECTION_HANDLE connectionId,
                              OI_OBEX_HEADER_LIST *cmdHeaders,
                              OI_STATUS obexStatus)
{
    OI_STATUS status;

    OI_TRACE_USER(("ServerGetInd(connectionId = %d, <*cmdHeaders = %x>, obexStatus = %d)",
                      connectionId, cmdHeaders, obexStatus));

    /* If there was an error or the GET operation is now complete, close the
     * file and free memory allocated.
     */
    if (OI_SUCCESS(obexStatus) ||
        ((obexStatus != OI_OBEX_CONTINUE) && (obexStatus != OI_OBEXSRV_INCOMPLETE_GET))) {
        OI_DBG_PRINT1(("ServerGetInd terminated with status: %d", obexStatus));
        status = OI_OK;
        goto ServerGetCleanup;
    }

    /* Continuing a file or folder get operation. */
    if (!server->incompleteGet && (server->state > PBAP_SERVER_CONNECTED)) {
        OI_DBG_PRINT1(("Get more data..."));
        status = server->fops->read(server->file,
                                    server->maxRead,
                                    FileReadCfm,
                                    (OI_PBAP_CONNECTION)server->id);
        if (!OI_SUCCESS(status)) {
            OI_SLOG_WARNING(status, ("File read failed"));
            status = OI_OBEXSRV_GetResponse(connectionId, NULL, OI_OBEX_CONTINUE);
            if (!OI_SUCCESS(status)) {
                OI_SLOG_ERROR(status, ("Failed to inform client of read failure"));
            }
            goto ServerGetCleanup;
        }
        return OI_OK;
    }

    /* If this is a new request, set all the application parameters to their
     * default values.
     */
    if(!server->incompleteGet) {
        ClearAppParams();
    }

    /* Extract all the parameters from the OBEX headers. */
    status = ParseObexHeader(cmdHeaders);
    if (!OI_SUCCESS(status)) {
        OI_SLOG_ERROR(status, ("Failed to parse OBEX header"));
        goto ServerGetCleanup;
    }


    /* If the get is incomplete we need to send a continue response. */
    server->incompleteGet = (obexStatus == OI_OBEXSRV_INCOMPLETE_GET);
    if (server->incompleteGet) {
        status = OI_OBEXSRV_GetResponse(connectionId, NULL, OI_OBEX_CONTINUE);
        if (!OI_SUCCESS(status)) {
            OI_SLOG_ERROR(status, ("Failed to send OBEX GET repsonse"));
            goto ServerGetCleanup;
        }
        return OI_OK;
    }

    /* Get request is complete */
    if (server->appParams.maxListCountSet && server->appParams.maxListCount == 0) {
        OI_DBG_PRINT1(("Get Phonebook Size"));
        status = GetPhonebookSize();
    } else {
        OI_DBG_PRINT1(("Get Phonebook, Listing, or vCard"));
        status = OpenDataHandle();
    }

    if (OI_SUCCESS(status)) {
        OI_FreeIf(&server->pathElements[0]);
        server->pathElemCnt = 0;
        return OI_OK;
    }

  ServerGetCleanup:

    /* Close file if we are already reading from it. */
    if (!server->incompleteGet && (server->state > PBAP_SERVER_CONNECTED)) {
        /* If we've called file open() but have not yet gotten the openCfm()
         * our state is GETTING, but there is no file open yet.  Only call
         * close if the file is open.
         */
        if (server->file) {
            OI_PBAP_HANDLE fileSv = server->file;

            server->file = NULL;
            setupDeferredFileClose(fileSv, server->id, obexStatus);
        }
    }
    OI_FreeIf(&server->pathElements[0]);
    server->pathElemCnt = 0;
    OI_FreeIf(&server->appParams.searchValue);
    server->incompleteGet = FALSE;
    setState(server, PBAP_SERVER_CONNECTED);

    return status;
}



/******************************************************************************
 * Handle set repository and phonebook operations from the client.
 */

static OI_STATUS ServerSetPathInd(OI_OBEXSRV_CONNECTION_HANDLE connectionId,
                                  OI_OBEX_UNICODE *folder,
                                  OI_BOOL upLevel,
                                  OI_BOOL create)
{
    OI_STATUS status = OI_OK;
    OI_STATUS status2;
#if defined(OI_DEBUG)
    const OI_CHAR16 root[] = {'<', 'r', 'o', 'o', 't', '>', 0};
    (void) root;  /* Suppresses GCC warning for specialized debug builds. */
#endif

    OI_TRACE_USER(("ServerSetPathInd(connectionId = %d, <*folder = %x>, upLevel = %d, create = %d)",
                 connectionId, folder, upLevel, create));

#if defined(OI_DEBUG)
    OI_DBG_PRINT2(("folder = \"%s\"\n",
                      (folder && folder->str && (folder->len > 0)) ? folder->str : root,
                      (folder && folder->str && (folder->len > 0)) ? folder->len : 6));
#endif

    if (!server)
        return OI_OBEX_INTERNAL_SERVER_ERROR;

    /*
     * Create is illegal for phonebook access.
     */
    if (create) {
        status = OI_OBEX_ACCESS_DENIED;
        goto out;
    }

    /*
     * NULL or empty folder name means set to root or parent folder
     */
    if ((folder == NULL) || (folder && folder->str == NULL) ||
        (folder && folder->str[0] == 0)) {
        if (upLevel) {
            /*
             * Check we are not aleady at the root folder
             */
            if (server->folderLevel == 0) {
                OI_SLOG_WARNING(OI_OBEX_NOT_FOUND,
                                ("Remote PBAP client requested invalid directory in setPath, \"%S\"",
                             folder ? folder->str : "NULL"));
                status = OI_OBEX_NOT_FOUND;
                goto out;
            }
            server->folderLevel--;

            if (server->phonebookSet) {
                OI_DBG_PRINT2(("Setting path to parent \"telecom\" or <root> directory"));
                server->phonebookSet = FALSE;
            } else {
                if (server->folderLevel == 0) {
                    OI_DBG_PRINT2(("Setting path to <root> directory"));
                    server->repositorySet = FALSE;
                    server->repository = OI_PBAP_LOCAL_REPOSITORY;
                } else if ((server->repository == OI_PBAP_SIM1_REPOSITORY) &&
                           (server->folderLevel == 1)) {
                    OI_DBG_PRINT2(("Setting path to \"SIM1\" directory"));
                    server->repositorySet = FALSE;
                }
            }
        } else {
            OI_DBG_PRINT2(("Setting path to <root> directory"));
            server->repositorySet = FALSE;
            server->phonebookSet = FALSE;
            server->repository = OI_PBAP_LOCAL_REPOSITORY;
            server->folderLevel = 0;
        }
    } else {
        /* Handle decending into the specified folder */
        OI_CHAR16 *elements[3];  /* 3 elements are all that's needed: SIM1, telecom, pb */
        OI_UINT16 elemCnt = OI_ARRAYSIZE(elements);
        OI_UINT16 elem;


        /*
         * Check we have a valid folder name
         */
        if (folder->len == 0) {
            OI_SLOG_WARNING(OI_OBEX_NOT_FOUND,
                            ("Remote PBAP client requested invalid phonebook directory"));
            status = OI_OBEX_NOT_FOUND;
            goto out;
        }

        /* There are no directories under the phonebook directories. */
        if (server->phonebookSet) {
            OI_SLOG_WARNING(OI_OBEX_NOT_FOUND,
                            ("Remote PBAP client requested SetPath while in a leaf directory, \"%S\"",
                             folder->str));
            status = OI_OBEX_NOT_FOUND;
            goto out;
        }

        status = TokenizePath(folder, elements, &elemCnt);
        if (!OI_SUCCESS(status)) {
            OI_SLOG_WARNING(status, ("PBAP: Error tokenizing folder, \"%S\"", folder->str));
            OI_Free(elements[0]);
            goto out;
        }

        if (server->folderLevel + elemCnt > 3) {
            OI_SLOG_WARNING(OI_OBEX_NOT_FOUND,
                            ("Remote PBAP client requested folder path that exceeds the maximum depth"));
            status = OI_OBEX_NOT_FOUND;
            OI_Free(elements[0]);
            goto out;
        }

        for (elem = 0; (OI_SUCCESS(status) && (elem < elemCnt)); elem++) {
            switch(server->folderLevel) {
            case 0:     /* In the <root> directory. */
                if (OI_StrcmpUtf16(elements[elem], OI_PBAP_usim1) == 0) {
                    server->repository = OI_PBAP_SIM1_REPOSITORY;
                    server->folderLevel++;
                    OI_DBG_PRINT2(("Set path to SIM1"));
                } else if (OI_StrcmpUtf16(elements[elem], OI_PBAP_utelecom) == 0) {
                    server->repository = OI_PBAP_LOCAL_REPOSITORY;
                    server->repositorySet = TRUE;
                    server->folderLevel++;
                    OI_DBG_PRINT2(("Set path to telecom"));
                } else {
                    OI_SLOG_WARNING(OI_OBEX_NOT_FOUND,
                                    ("Remote PBAP client requested invalid directory in SetPath, \"%s\"",
                                     elements[elem]));
                    status = OI_OBEX_NOT_FOUND;
                }
                break;

            case 1:     /* In the "SIM1" or "telecom" directory. */
                if ((server->repository == OI_PBAP_SIM1_REPOSITORY) &&
                    (OI_StrcmpUtf16(elements[elem], OI_PBAP_utelecom) == 0)) {
                    server->repositorySet = TRUE;
                    server->folderLevel++;
                    OI_DBG_PRINT2(("Set path to SIM1/telecom"));
                } else {
                    status = LookupPhonebook(elements[elem], &server->phonebook);
                    if (OI_SUCCESS(status)) {
                        server->phonebookSet = TRUE;
                        server->folderLevel++;
                        OI_DBG_PRINT2(("Set path to telecom/%S",
                                          OI_PBAP_upbdirs[server->phonebook]));
                    } else {
                        OI_SLOG_WARNING(status,
                                        ("Remote PBAP client requested invalid directory in SetPath, \"%s\"",
                                         elements[elem]));
                    }
                }
                break;

            case 2:     /* In the "SIM1/telecom" or "telecom/<phonebook>" directory. */
                if (server->repository == OI_PBAP_LOCAL_REPOSITORY) {
                    OI_SLOG_WARNING(OI_OBEX_NOT_FOUND,
                                    ("Remote PBAP client requested invalid directory in SetPath: \"%s\"",
                                     elements[elem]));
                    status = OI_OBEX_NOT_FOUND;
                } else {
                    status = LookupPhonebook(elements[elem], &server->phonebook);
                    if (OI_SUCCESS(status)) {
                        server->phonebookSet = TRUE;
                        server->folderLevel++;
                        OI_DBG_PRINT2(("Set path to SIM1/telecom/%S",
                                          OI_PBAP_upbdirs[server->phonebook]));
                    } else {
                        OI_SLOG_WARNING(status,
                                        ("Remote PBAP client requested invalid directory in SetPath, \"%s\"",
                                         elements[elem]));
                    }
                }
                break;

            default:    /* No more subdirectories below SIM1/telecom/<phonebook>. */
                OI_SLOG_WARNING(OI_OBEX_NOT_FOUND,
                                ("Remote PBAP client requested invalid directory in SetPath, \"%s\"",
                                 elements[elem]));
                status = OI_OBEX_NOT_FOUND;
                break;
            }
        }

        OI_Free(elements[0]);
    }

  out:
    status2 = OI_OBEXSRV_ConfirmSetpath(server->id, status, NULL);
    if (!OI_SUCCESS(status2)) {
        OI_SLOG_ERROR(status2, ("Failed to confirm (%d) the set path operation with the client",
                                status));
    }

    return status;
}



/******************************************************************************
 * Basic connection management routines.
 */

OI_STATUS OI_PBAPServer_AuthenticationResponse(OI_PBAP_CONNECTION connectionId,
                                               const OI_BYTE *userId,
                                               OI_UINT8 userIdLen,
                                               const OI_CHAR *password)
{
    OI_STATUS status;

    OI_TRACE_USER(("OI_PBAPServer_AuthenticationResponse(connectionId = %d, server's userId = \"%s\", server's userIdLen = %d, server's password = \"%s\")",
                 connectionId, userId, userIdLen, userIdLen, password));

    if (!OI_INIT_FLAG_VALUE(PBAP_SRV)) {
        status = OI_STATUS_NOT_INITIALIZED;
    } else if ((server->state != PBAP_SERVER_IDLE) || !server->unauthorized) {
        status = OI_OBEX_INVALID_OPERATION;
    } else {
        status = OI_OBEXSRV_AuthenticationResponse(server->id, userId, userIdLen, password, TRUE);

        if (OI_SUCCESS(status)) {
            server->maxRead = OI_OBEXSRV_OptimalBodyHeaderSize(server->id);
    }
    }

    return status;
}


static void ServerConnectInd(OI_OBEXSRV_CONNECTION_HANDLE connectionId,
                             OI_BOOL                      unauthorized,
                             OI_BYTE                      *userId,
                             OI_UINT8                     userIdLen,
                             OI_OBEX_REALM                *realm)
{
    OI_STATUS status;
    OI_BD_ADDR clientAddr;

    OI_TRACE_USER(("ServerConnectInd(connectionId = %d, unauthorized = %d, client's userId = \"%s\", client's userIdLen = %d)",
                      connectionId, unauthorized,
                      userId, userIdLen, userIdLen));

    /*
     * Check that the server is not already in use.
     */
    if (server->state > PBAP_SERVER_IDLE) {
        OI_SLOG_ERROR(OI_STATUS_NONE, ("PBAP server already in use"));
        status = OI_OBEXSRV_AcceptConnect(connectionId, FALSE, OI_STATUS_NO_RESOURCES, NULL);
        if (!OI_SUCCESS(status)) {
            OI_SLOG_ERROR(status, ("Failed to inform client of connection failure"));
        }
        return;
    }

    server->unauthorized = unauthorized;
    server->id = connectionId;

    /*
     * Pass address of connecting client to application.
     */
    status = OI_OBEXSRV_GetClientAddr(connectionId, &clientAddr);
    if (!OI_SUCCESS(status)) {
        OI_SLOG_ERROR(status, ("Failed to get client address"));
        status = OI_OBEXSRV_AcceptConnect(connectionId, FALSE, OI_FAIL, NULL);
        if (!OI_SUCCESS(status)) {
            OI_SLOG_ERROR(status, ("Failed to inform client of connection failure"));
        }
    } else {
        server->connectInd(&clientAddr, unauthorized, userId, userIdLen, connectionId);
    }
}


OI_STATUS OI_PBAPServer_AcceptConnection(OI_PBAP_CONNECTION connectionId,
                                         OI_UINT8 newMissedCalls,
                                         OI_BOOL accept)
{
    OI_STATUS status = OI_OK;

    OI_TRACE_USER(("OI_PBAPServer_AcceptConnection(connectionId = %d, newMissedCalls = %d, accept = %d)",
                 connectionId, newMissedCalls, accept));

    if (!OI_INIT_FLAG_VALUE(PBAP_SRV)) {
        status = OI_STATUS_NOT_INITIALIZED;
        goto out;
    }
    if ((server->state != PBAP_SERVER_IDLE) || (server->id != connectionId)) {
        status = OI_OBEX_INVALID_OPERATION;
        goto out;
    }
    /*
     * If the connection is being rejected or was unauthorized reject the
     * connection.
     */
    if (server->unauthorized || !accept) {

        status = OI_OBEX_ACCESS_DENIED;

        OI_DBG_PRINT2(("Denying OBEX access: status = %d", status));
        status = OI_OBEXSRV_AcceptConnect(server->id, FALSE, status, NULL);
        if (!OI_SUCCESS(status)) {
            OI_SLOG_ERROR(status, ("Failed to inform client of connection rejection"));
        }
        goto out;
    }

    server->folderLevel = 0;
    server->repositorySet = FALSE;
    server->phonebookSet = FALSE;
    server->repository = OI_PBAP_LOCAL_REPOSITORY;
    server->newMissedCalls = newMissedCalls;

    status = OI_OBEXSRV_AcceptConnect(server->id, TRUE, OI_OK, NULL);
    if (OI_SUCCESS(status)) {
        setState(server, PBAP_SERVER_CONNECTED);
        server->maxRead = OI_OBEXSRV_OptimalBodyHeaderSize(server->id);
    } else {
        OI_SLOG_ERROR(status, ("PBAP: Failed to accept connection"));
    }
  out:
    return status;
}


static void ServerDisconnectInd(OI_OBEXSRV_CONNECTION_HANDLE connectionId)
{
    OI_TRACE_USER(("ServerDisconnectInd(connectionId = %d)", connectionId));

    OI_ASSERT(server->id == connectionId);
    server->id = 0;
    setState(server, PBAP_SERVER_IDLE);
    server->disconnectInd(connectionId);
}


OI_STATUS OI_PBAPServer_ForceDisconnect(OI_PBAP_CONNECTION connectionId)
{
    OI_TRACE_USER(("OI_PBAPServer_ForceDisconnect(connectionId = %d)", connectionId));

    if (!OI_INIT_FLAG_VALUE(PBAP_SRV)) {
        return OI_STATUS_NOT_INITIALIZED;
    }
    if (connectionId != server->id) {
        return OI_STATUS_INVALID_PARAMETERS;
    }
    if (server->state < PBAP_SERVER_CONNECTED) {
        return OI_OBEX_NOT_CONNECTED;
    }
    return OI_OBEXSRV_ForceDisconnect(server->id);
}


/******************************************************************************
 * Basic service registration management routines.
 */

OI_STATUS OI_PBAPServer_Register(OI_OBEXSRV_AUTHENTICATION authentication,
                                 OI_PBAP_CONNECTION_IND connectInd,
                                 OI_PBAP_DISCONNECTION_IND disconnectInd,
                                 OI_PBAP_SERVER_GET_PHONEBOOK_SIZE_IND getPhonebookSize,
                                 const OI_PBAP_SERVER_FILESYS_FUNCTIONS *fileOperations,
                                 const OI_SDP_STRINGS *strings,
                                 OI_UINT32 *srecHandle,
                                 OI_UINT8 suppRepositories,
                                 OI_OBEX_SERVER_HANDLE *serverHandle)
{
    static OI_OBEXSRV_CB_LIST CBList = {
        ServerConnectInd,       /* connectInd       */
        ServerDisconnectInd,    /* disconnectInd    */
        ServerGetInd,           /* getInd           */
        ServerPutInd,           /* putInd           */
        ServerSetPathInd,       /* setPathInd       */
        NULL,                   /* bulkGetInd        */
        NULL,                   /* actionInd        */
        NULL,                   /* progressInd      */
        NULL,                   /* authInd          */
        NULL                   /* abortInd         */
    };
    static OI_BYTE uuid[OI_OBEX_UUID_SIZE] = OI_PBAP_OBEX_TARGET_UUID;
    OI_OBEX_LOWER_PROTOCOL lowerProtocol;
    OI_OBEX_BYTESEQ target = { OI_ARRAYSIZE(uuid), uuid };
    OI_SDPDB_SERVICE_RECORD srec;
    OI_STATUS status;

    OI_TRACE_USER(("OI_PBAPServer_Register(<*authentication = %x>, <*connectInd = %x>, <*disconnectInd = %x>, <*getPhonebookSize = %x>, <*fileOperations = %x>, <*strings = %x>, <*srecHandle = %x>, suppRepositories = %s%s%s)",
                 authentication, connectInd, disconnectInd, getPhonebookSize, fileOperations, strings,
                 srecHandle,
                 suppRepositories & OI_PBAP_SUPPORTED_REPOSITORIES_LOCAL ? "LOCAL" : "",
                 ~suppRepositories & (OI_PBAP_SUPPORTED_REPOSITORIES_LOCAL | OI_PBAP_SUPPORTED_REPOSITORIES_SIM) ? "" : " and ",
                 suppRepositories & OI_PBAP_SUPPORTED_REPOSITORIES_SIM ? "SIM1" : ""));

    OI_ARGCHECK(fileOperations && connectInd && disconnectInd && strings);
    OI_ARGCHECK((suppRepositories & ~(OI_PBAP_SUPPORTED_REPOSITORIES_LOCAL |
                                   OI_PBAP_SUPPORTED_REPOSITORIES_SIM)) == 0);

    if (OI_INIT_FLAG_VALUE(PBAP_SRV)) {
        return OI_STATUS_ALREADY_REGISTERED;
    }

    server = OI_Calloc(sizeof(PBAP_SERVER));
    if (server == NULL) {
        OI_SLOG_ERROR(OI_STATUS_OUT_OF_MEMORY,
                      ("Failed to allocate memory for PBAP server structure"));
        return OI_STATUS_OUT_OF_MEMORY;
    }

    /* Store the root folder. The current folder is reset to the root folder at
     * the start of each new connection.
     */
    server->connectInd = connectInd;
    server->disconnectInd = disconnectInd;
    server->getPhonebookSize = getPhonebookSize;
    server->fops = fileOperations;

    /* There is no file open. */
    server->file = NULL;

    lowerProtocol.protocol = OI_OBEX_LOWER_RFCOMM;
    /* Try and get the preferred PBAP channel number. */
    lowerProtocol.svcId.rfcommChannel = OI_CONFIG_TABLE_GET(PBAP_SRV)->rfcomm_channel_pref;

    status = OI_OBEXSRV_RegisterServer(&target,
                                       &CBList,
                                       authentication,
                                       NULL,
                                       &lowerProtocol,
                                       1,
                                       &connectPolicy,
                                       &server->serverHandle);
    if (!OI_SUCCESS(status) ) {
        goto ErrorExit;
    }

    /* Register service record. */
    OI_SET_UINT_ELEMENT(OurServerChannel, lowerProtocol.svcId.rfcommChannel);
    OI_SET_UINT_ELEMENT(SupportedRepositories, suppRepositories);

    srec.Attributes = ServiceDescription;
    srec.NumAttributes = OI_ARRAYSIZE(ServiceDescription);
    srec.Strings = strings->attrs;
    srec.NumStrings = strings->num;

    OI_DBG_PRINT1(("Registered Phonebook Access Profile"));

    OI_INIT_FLAG_INCREMENT(PBAP_SRV);

    /* Return the server handle to the caller. */
    *serverHandle = server->serverHandle;

    /* Return the SDP record handle to the caller. */
    if (srecHandle) {
        *srecHandle = server->srecHandle;
    }

    return OI_OK;
  ErrorExit:

    OI_SLOG_ERROR(status, ("OI_PBAPServer_Register failed"));

    if (server) {
        if (server->serverHandle) {
            OI_OBEXSRV_DeregisterServer(server->serverHandle);
        }
        OI_Free(server);
    }

    return status;
}


OI_STATUS OI_PBAPServer_Deregister(OI_OBEX_SERVER_HANDLE serverHandle)
{
    OI_STATUS status;

    OI_TRACE_USER(("OI_PBAPServer_Deregister"));

    if (!OI_INIT_FLAG_VALUE(PBAP_SRV)) {
        return OI_STATUS_NOT_REGISTERED;
    }
    if (!server) {
            return OI_STATUS_INVALID_HANDLE;
    }
    if (server->serverHandle != serverHandle) {
        return OI_STATUS_INVALID_HANDLE;
    }

    status = OI_OBEXSRV_DeregisterServer(server->serverHandle);

    if (!OI_SUCCESS(status)) {
        OI_SLOG_ERROR(status, ("Error deregistering server"));
        goto fail;
    }

    if(server)
        OI_Free(server);
    server = NULL;

    OI_INIT_FLAG_DECREMENT(PBAP_SRV);

    return OI_OK;

 fail:
    return status;
}
