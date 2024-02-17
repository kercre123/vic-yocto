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
 * Phone Book Access Profile client
 */

#define __OI_MODULE__ OI_MODULE_PBAP_CLI

#include "oi_bt_assigned_nos.h"
#include "oi_pbap_client.h"
#include "oi_dispatch.h"
#include "oi_std_utils.h"
#include "oi_memmgr.h"
#include "oi_pbap_sys.h"
#include "oi_obexcli.h"
#include "oi_argcheck.h"
#include "oi_assert.h"
#include "oi_sdpdb.h"
#include "oi_sdp_utils.h"
#include "oi_bytestream.h"
#include "oi_unicode.h"

#include "oi_debug.h"

#include "oi_init_flags.h"
#include "oi_config_table.h"
#include "oi_bt_profile_config.h"

#include "oi_pbap_private.h"


/**
 * These are the various client connection states.
 */
typedef enum {
    CLIENT_STATE_DISCONNECTED,
    CLIENT_STATE_AUTHENTICATING,
    CLIENT_STATE_CONNECTED,
    CLIENT_STATE_PULLING_VCARD,
    CLIENT_STATE_PULLING_PHONEBOOK,
    CLIENT_STATE_PULLING_PHONEBOOK_SIZE,
    CLIENT_STATE_PULLING_VCARD_LISTING,
    CLIENT_STATE_ABORTING
} PBAP_CLIENT_STATE;


/****************************************************************
 *
 * Service record
 *
 ****************************************************************/

static OI_UINT32 srecHandle;

/** service class ID list */
static const OI_DATAELEM ServiceClassIDList[] = {
    OI_ELEMENT_UUID32(OI_UUID_PhonebookAccessClient)
};

/* profile descriptor list */
static const OI_DATAELEM Profile0[] = {
    OI_ELEMENT_UUID32(OI_UUID_PhonebookAccess),
    OI_ELEMENT_UINT16(0x0100) //version 1.0
};

static const OI_DATAELEM ProfileDescriptorList[] = {
    OI_ELEMENT_SEQ(Profile0)
};

/** SDP attribute lists */
static const OI_SDPDB_ATTRIBUTE ServiceDescription[] = {
    { OI_ATTRID_ServiceClassIDList,                   OI_ELEMENT_SEQ(ServiceClassIDList) },
    { OI_ATTRID_BluetoothProfileDescriptorList,       OI_ELEMENT_SEQ(ProfileDescriptorList) }
};


typedef struct _PBAP_CLIENT PBAP_CLIENT;

/* Stucture for tracking the currently selected path and destination path. */
typedef void(*PBAP_SET_PATH_DONE)(PBAP_CLIENT *client, OI_STATUS status);

typedef struct {
    OI_PBAP_REPOSITORY currRepo;
    OI_PBAP_PHONEBOOK currPb;
    OI_BOOL inTelecom;
    OI_PBAP_REPOSITORY destRepo;
    OI_PBAP_PHONEBOOK destPb;
    PBAP_SET_PATH_DONE SetPathDone;
} PBAP_PATH_STATE;

/* Get request parameters. */
typedef struct {
    OI_PBAP_REPOSITORY repo;                            /**< Repository containing desired information */
    OI_PBAP_PHONEBOOK pb;                               /**< Phonebook containing desired information */
    OI_UINT32 entry;                                    /**< Desired Phonebook entry (if applicable) */
    const OI_CHAR *type;                                /**< Type of get request. */
    OI_UINT16 typeLen;                                  /**< Length of Type string. */
    OI_BYTE *appParam;                                  /**< Byte array of the application paramters */
    OI_UINT16 appParamLen;                              /**< Length of the application parameter byte array */
    OI_CHAR16 pathStr[OI_PBAP_MAX_PBAP_PATH_SIZE];      /**< Unicode pathname of desired object */
    OI_UINT16 pathStrLen;                               /**< Length of the pathname */
    union {
        OI_PBAP_CLIENT_PULL_PHONEBOOK_CB pullPb;        /**< Pull phonebook callback */
        OI_PBAP_CLIENT_GET_PHONEBOOK_SIZE_CB getSize;   /**< Pull phonebook size callback */
        OI_PBAP_CLIENT_PULL_VCARD_LISTING_CB pullList;  /**< Pull listing callback */
        OI_PBAP_CLIENT_PULL_VCARD_ENTRY_CB pullVCard;   /**< Pull vCard Entry callback */
        OI_PBAP_CLIENT_SETPATH_CB setPath;              /**< Set Path callback */
    } cb;
    struct {
        OI_UINT16 phonebookSize;                        /**< Total number of entries in the phonebook */
        OI_UINT8  newMissedCalls;                       /**< Number of newly missed calls */
    } results;

} PBAP_GET_REQ;

/**
 * Struct for an PBAP client connection.
 */
struct _PBAP_CLIENT {
    OI_OBEXCLI_CONNECTION_HANDLE id;                   /**< The underlying OBEX client connection */
    PBAP_CLIENT_STATE state;                           /**< state of this pbap conntection */

    PBAP_PATH_STATE pathState;                         /**< Which path is currently set */

    OI_PBAP_CONNECTION_CFM connectionCfm;              /**< Connect complete callback */
    OI_PBAP_DISCONNECTION_IND disconnectInd;           /**< Disconnect indication callback */
    OI_PBAP_CLIENT_AUTHENTICATION_CB authenticationCB; /**< Callback function to get authentication information */
    OI_PBAP_ABORT_CFM abortCfm;                        /**< Abort complete callback */

    const OI_PBAP_CLIENT_FILESYS_FUNCTIONS *fops;      /**< File operations required by the client */
    OI_PBAP_HANDLE fh;                                 /**< File handle for use with file operations */
    OI_BOOL fileOpen;                                  /**< Boolean indicating if file is opened */

    DISPATCH_CB_HANDLE dispatchHandle;                 /**< Set repository/phonebook complete has been registered with the dispatcher */

    PBAP_GET_REQ req;                                  /**< GET request info */
    OI_BOOL disconnected;
    OI_BOOL final;
    OI_BOOL fileOpCfmPending;
};

/**
 * Connection policy.
 */
static const OI_CONNECT_POLICY  connectPolicy =
{
    OI_ELEMENT_UUID32(OI_UUID_PhonebookAccessClient),     /* OI_DATAELEM         serviceUuid           */
    FALSE,                              /* OI_BOOL             mustBeMaster          */
    NULL,                               /* OI_L2CAP_FLOWSPEC   *flowspec;            */
    0                                   /* OI_UINT8            powerSavingDisables ; */
};

#define IS_CLIENT_CONNECTED (OI_INIT_FLAG_VALUE(PBAP_CLI) && (client) && (client->state >= CLIENT_STATE_CONNECTED))

/**
 * Helper macro to get the Bip Client pointer from the OBEX connection handle
 */
#define PbapClient(c) ((PBAP_CLIENT*)OI_OBEXCLI_GetConnectionContext((OI_OBEXCLI_CONNECTION_HANDLE)(c)))


#if defined(OI_DEBUG)
/**
 * ClientStateText()
 *
 * Helper function to convert the state to a human readable string for printing.
 */
static const OI_CHAR *ClientStateText(PBAP_CLIENT_STATE state)
{
    switch (state) {
    case CLIENT_STATE_DISCONNECTED:           return "CLIENT_STATE_DISCONNECTED";
    case CLIENT_STATE_AUTHENTICATING:         return "CLIENT_STATE_AUTHENTICATING";
    case CLIENT_STATE_CONNECTED:              return "CLIENT_STATE_CONNECTED";
    case CLIENT_STATE_PULLING_VCARD:          return "CLIENT_STATE_PULLING_VCARD";
    case CLIENT_STATE_PULLING_PHONEBOOK:      return "CLIENT_STATE_PULLING_PHONEBOOK";
    case CLIENT_STATE_PULLING_PHONEBOOK_SIZE: return "CLIENT_STATE_PULLING_PHONEBOOK_SIZE";
    case CLIENT_STATE_PULLING_VCARD_LISTING:  return "CLIENT_STATE_PULLING_VCARD_LISTING";
    case CLIENT_STATE_ABORTING:               return "CLIENT_STATE_ABORTING";
    }
    return "<unknown state>";
}

/**
 * setState()
 *
 * Helper macro for setting the state.  (Written as a macro so that the
 * OI_DBPRINT() reports the line in the function where the state changed
 * rather than the line in setState().
 */
#define setState(_client, _newState)                    \
    do {                                                \
        OI_DBG_PRINT1(("PBAP Client state %s ==> %s\n",      \
                     ClientStateText((_client)->state), \
                     ClientStateText(_newState)));      \
        (_client)->state = (_newState);                 \
    } while (0)
#else
/**
 * ClientStateText()
 *
 * Empty definition of above incase it is used by non-debug code.
 */
#define ClientStateText(state) ""

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

/**
 * VALIDATE_CFM_STATE()
 *
 * This macro validates that a confirm callback is pending and that we are in
 * the correct state for this confirm.
 */
#define VALIDATE_CFM_STATE(_fn, _client, _state_test)                   \
    do {                                                                \
        if (!OI_INIT_FLAG_VALUE(PBAP_CLI) ||                            \
            !(_client) ||                                               \
            !(_state_test) ||                                           \
            !(_client)->fileOpCfmPending) {                             \
            OI_SLOG_ERROR(OI_STATUS_NONE,                               \
                          ("%s() called at wrong time (state %s valid? %d; confirm pending? %d)", \
                           (_fn), ClientStateText((_client)->state),    \
                           (_state_test),                               \
                           ((_client)->fileOpCfmPending)));             \
            return;                                                     \
        }                                                               \
        (_client)->fileOpCfmPending = FALSE;                            \
        if ((_client)->disconnected) {                                  \
            OI_DBG_PRINT2(("Closing file %x\n", (_client)->fh));     \
            FileClose((_client), OI_OBEX_NOT_CONNECTED);                \
            return;                                                     \
        }                                                               \
    } while (0)


/***********************************************************************
 *
 * Deferred close
 *
 ***********************************************************************/

typedef struct {
    PBAP_CLIENT *client;
    OI_STATUS status;
} CLOSE_INFO;

/**
 * FileCloseCB()
 *
 * This Dispatcher callback closes any open file handles related to the most
 * recent action and performs the final clean up steps for that action.
 */
static void FileCloseCB(DISPATCH_ARG *darg)
{
    CLOSE_INFO *info = Dispatch_GetPointerFromArg(darg, CLOSE_INFO);
    PBAP_CLIENT *client;
    PBAP_CLIENT_STATE snapState;

    OI_TRACE_USER(("FileCloseCB(<*darg = %x>)", darg));
    OI_DBG_PRINT2(("darg => status: %d\n", info->status));

    client = info->client;
    snapState = client->state;

    client->dispatchHandle = 0;

    if (client->disconnected) {
        setState(client, CLIENT_STATE_DISCONNECTED);
    } else  {
        setState(client, CLIENT_STATE_CONNECTED);
    }

    OI_FreeIf(&client->req.appParam);

    if (client->fileOpen) {
        OI_PBAP_HANDLE handle = client->fh;

        client->fh = 0;
        OI_TRACE_USER(("Calling close(<*handle = %x>, id = %d, status = %d)",
                          handle, (OI_PBAP_CONNECTION)client->id, info->status));
        client->fops->close(handle, (OI_PBAP_CONNECTION)client->id, info->status);
        client->fileOpen = FALSE;
    }

    switch (snapState) {
    case CLIENT_STATE_PULLING_VCARD:
        client->req.cb.pullVCard(client->id, info->status);
        break;

    case CLIENT_STATE_PULLING_PHONEBOOK:
        client->req.cb.pullPb(client->id, client->req.results.newMissedCalls, info->status);
        break;

    case CLIENT_STATE_PULLING_PHONEBOOK_SIZE:
        client->req.cb.getSize(client->id, client->req.results.phonebookSize, info->status);
        break;

    case CLIENT_STATE_PULLING_VCARD_LISTING:
        client->req.cb.pullList(client->id, client->req.results.newMissedCalls, info->status);
        break;

    default:
        break;
    }

    /* If disconnected, tell the application and re-initialize ourselves */
    if (client->disconnected) {
        OI_PBAP_CONNECTION id = (OI_PBAP_CONNECTION)client->id;
        OI_PBAP_DISCONNECTION_IND disconnectInd = client->disconnectInd;

        OI_Free(client);

        disconnectInd(id);

        OI_INIT_FLAG_PUT_FLAG(FALSE, PBAP_CLI);
    }
}

/**
 * FileClose()
 *
 * Sets up to close the open file handle and clean up from the operation on
 * the Dispatcher when it is safe to do so.
 */
static void FileClose(PBAP_CLIENT *client, OI_STATUS status)
{
    DISPATCH_ARG darg;
    CLOSE_INFO info;

    OI_TRACE_USER(("FileClose(<*client = %x>, status = %d)", client, status));

    info.client = client;
    info.status = status;

    Dispatch_SetArg(darg, info);
    (void) OI_Dispatch_RegisterFunc(FileCloseCB, &darg, &client->dispatchHandle);
}


/************************************************************************
 *
 * PBAP path utility functions
 *
 ************************************************************************/

/**
 * BuildRepoPathName()
 *
 * This function builds the base part of a full path name to some PBAP entity
 * from the root directory if the current directory is the root directory.
 * Otherwise it returns an empty string.  It depends on *buf having ample room
 * for "SIM1/telecom".
 */
static OI_INT BuildRepoPathName(PBAP_PATH_STATE *pstate,
                                OI_PBAP_REPOSITORY repo,
                                OI_CHAR16 *buf)
{
    OI_INT len = 0;
    OI_INT plen;

    OI_TRACE_USER(("BuildRepoPathName(<*pstate = %x>, repo = %d, <*buf = %x>)",
                          pstate, repo, buf));

    if ((pstate->currRepo == OI_PBAP_LOCAL_REPOSITORY) && (repo == OI_PBAP_SIM1_REPOSITORY)) {
        plen = OI_StrLenUtf16(OI_PBAP_usim1) + 1;  /* Count the nul terminator too. */
        OI_MemCopy(buf, OI_PBAP_usim1, plen * sizeof(OI_CHAR16));
        len += plen;
    }

    if (!pstate->inTelecom) {
        if (len > 0) {
            buf[len - 1] = '/';
        }

        plen = OI_StrLenUtf16(OI_PBAP_utelecom) + 1;  /* Count the nul terminator too. */
        OI_MemCopy(&buf[len], OI_PBAP_utelecom, plen * sizeof(OI_CHAR16));
        len += plen;
    }

    OI_DBG_PRINT2(("Path = \"%s\"", buf, len));

    return len;
}

/**
 * BuildPbDirPathName()
 *
 * This function builds the full path name to a phonebook directory starting
 * from the current directory.  It depends on *buf having ample room for
 * "SIM1/telecom/ich".
 */
static OI_INT BuildPbDirPathName(PBAP_PATH_STATE *pstate,
                                 OI_PBAP_REPOSITORY repo,
                                 OI_PBAP_PHONEBOOK pb,
                                 OI_CHAR16 *buf)
{
    OI_INT len = 0;

    OI_TRACE_USER(("BuildPbDirPathName(<*pstate = %x>, repo = %d, pb = %d, <*buf = %x>)",
                          pstate, repo, pb, buf));

    if (pstate->currPb == OI_PBAP_INVALID_PHONEBOOK) {
        len += BuildRepoPathName(pstate, repo, buf);

        if (len > 0) {
            buf[len - 1] = '/';
        }

        OI_MemCopy(&buf[len], OI_PBAP_upbdirs[pb], OI_PBAP_upbdirsizes[pb] * sizeof(OI_CHAR16));
        len += OI_PBAP_upbdirsizes[pb];
    }

    OI_DBG_PRINT2(("Path = \"%s\"", buf, len));

    return len;
}

/**
 * BuildPbVCardPathName()
 *
 * This function builds the full path name to a phonebook vCard starting from
 * the current directory.  It depends on *buf having ample room for
 * "SIM1/telecom/ich.vcf".
 */
static OI_INT BuildPbVCardPathName(PBAP_PATH_STATE *pstate,
                                   OI_PBAP_REPOSITORY repo,
                                   OI_PBAP_PHONEBOOK pb,
                                   OI_CHAR16 *buf)
{
    OI_INT len = 0;
    OI_INT plen;

    OI_TRACE_USER(("BuildPbVCardPathName(<*pstate = %x>, repo = %d, pb = %d, <*buf = %x>)",
                          pstate, repo, pb, buf));

    len += BuildPbDirPathName(pstate, repo, pb, buf) - 1;

    plen = OI_StrLenUtf16(OI_PBAP_usim1) + 1;  /* Count the nul terminator too. */
    OI_MemCopy(&buf[len], OI_PBAP_vcf, plen * sizeof(OI_CHAR16));
    len += plen;

    OI_DBG_PRINT2(("Path = \"%s\"\n", buf, len));

    return len;
}

/**
 * BuildVCardPathName()
 *
 * This function builds the full path name to a specific vCard starting from
 * the current directory.  It depends on *buf having ample room for
 * "SIM1/telecom/ich/65535.vcf".
 */
static OI_INT BuildVCardPathName(PBAP_PATH_STATE *pstate,
                                 OI_PBAP_REPOSITORY repo,
                                 OI_PBAP_PHONEBOOK pb,
                                 OI_UINT32 entry,
                                 OI_CHAR16 *buf)
{
    OI_INT len;
    OI_INT baseLen;
    OI_UINT8 digit;
    OI_UINT32 i;
    OI_INT plen;

    OI_TRACE_USER(("BuildVCardPathName(<*pstate = %x>, repo = %d, pb = %d, entry = %d, <*buf = %x>)",
                          pstate, repo, pb, entry, buf));

    len = BuildPbDirPathName(pstate, repo, pb, buf);

    if (len > 0) {
        buf[len - 1] = '/';
    }
    baseLen = len;

    for (i = 0x10000000; i > 0; i /= 16) {
        digit = (OI_UINT8)(entry / i);
        entry -= digit * i;

        if ((digit >= 10) && (digit <= 16)) {

            buf[len++] = (digit - 10) + 'A';

        } else if ((1==i) || (digit > 0) || (len > baseLen)) {
            /* (digit > 0) || (len > baseLen) check eliminates leading zeros. */
            /* 1==i check is for 0.vcf. */
            buf[len++] = digit + '0';
        }
    }
    plen = OI_StrLenUtf16(OI_PBAP_usim1) + 1;  /* Count the nul terminator too. */
    OI_MemCopy(&buf[len], OI_PBAP_vcf, plen * sizeof(OI_CHAR16));
    len += plen;

    OI_DBG_PRINT2(("Path = \"%s\"\n", buf, len));

    return len;
}

static void SetPath(PBAP_CLIENT *client,
                    PBAP_PATH_STATE *pstate);

/**
 * ToRootDirCfm()
 *
 * This is just the SETPATCH CFM callback that gets called when the root
 * directory is entered.
 */
static void ToRootDirCfm(OI_OBEXCLI_CONNECTION_HANDLE connectionId, OI_STATUS status)
{
    PBAP_CLIENT *client = PbapClient(connectionId);
    PBAP_PATH_STATE *pstate;

    OI_TRACE_USER(("ToRootDirCfm(connectionId = %d, status = %d)",
                          connectionId, status));

    OI_ASSERT(client);
    if (!client) {
        OI_SLOG_ERROR(OI_STATUS_NONE, ("Client lookup failed"));
        return;
    }

    pstate = &client->pathState;

    if (OI_SUCCESS(status)) {
        pstate->currRepo = OI_PBAP_LOCAL_REPOSITORY;
        pstate->inTelecom = FALSE;
        pstate->currPb = OI_PBAP_INVALID_PHONEBOOK;

        if (pstate->destRepo == OI_PBAP_INVALID_REPOSITORY) {
            OI_TRACE_USER(("Calling SetPathDone(<*client = %x>, status = %d)",
                                  client, status));
            pstate->SetPathDone(client, status);
        } else {
            SetPath(client, pstate);
        }
    } else {
        OI_TRACE_USER(("Calling SetPathDone(<*client = %x>, status = %d)",
                              client, status));
        pstate->SetPathDone(client, status);
    }
}

/**
 * ToSim1DirCfm()
 *
 * This is just the SETPATCH CFM callback that gets called when the SIM1
 * directory is entered.
 */
static void ToSim1DirCfm(OI_OBEXCLI_CONNECTION_HANDLE connectionId, OI_STATUS status)
{
    PBAP_CLIENT *client = PbapClient(connectionId);
    PBAP_PATH_STATE *pstate;

    OI_TRACE_USER(("ToSim1DirCfm(connectionId = %d, status = %d)",
                          connectionId, status));

    OI_ASSERT(client);
    if (!client) {
        OI_SLOG_ERROR(OI_STATUS_NONE, ("Client lookup failed"));
        return;
    }

    pstate = &client->pathState;

    if (OI_SUCCESS(status)) {
        pstate->currRepo = OI_PBAP_SIM1_REPOSITORY;
        pstate->currPb = OI_PBAP_INVALID_PHONEBOOK;

        SetPath(client, pstate);
    } else {
        OI_TRACE_USER(("Calling SetPathDone(<*client = %x>, status = %d)",
                              client, status));
        pstate->SetPathDone(client, status);
    }
}

/**
 * ToTelecomDirCfm()
 *
 * This is just the SETPATCH CFM callback that gets called when the telecom
 * directory is entered.
 */
static void ToTelecomDirCfm(OI_OBEXCLI_CONNECTION_HANDLE connectionId, OI_STATUS status)
{
    PBAP_CLIENT *client = PbapClient(connectionId);
    PBAP_PATH_STATE *pstate;

    OI_TRACE_USER(("ToTelecomDirCfm(connectionId = %d, status = %d)",
                          connectionId, status));

    OI_ASSERT(client);
    if (!client) {
        OI_SLOG_ERROR(OI_STATUS_NONE, ("Client lookup failed"));
        return;
    }

    pstate = &client->pathState;

    if (OI_SUCCESS(status)) {
        pstate->inTelecom = TRUE;
        pstate->currPb = OI_PBAP_INVALID_PHONEBOOK;

        SetPath(client, pstate);
    } else {
        OI_TRACE_USER(("Calling SetPathDone(<*client = %x>, status = %d)",
                              client, status));
        pstate->SetPathDone(client, status);
    }
}

/**
 * ToPbDirCfm()
 *
 * This is just the SETPATCH CFM callback that gets called when a phonebook
 * directory is entered.
 */
static void ToPbDirCfm(OI_OBEXCLI_CONNECTION_HANDLE connectionId, OI_STATUS status)
{
    PBAP_CLIENT *client = PbapClient(connectionId);
    PBAP_PATH_STATE *pstate;

    OI_TRACE_USER(("ToPbDirCfm(connectionId = %d, status = %d)",
                          connectionId, status));

    OI_ASSERT(client);
    if (!client) {
        OI_SLOG_ERROR(OI_STATUS_NONE, ("Client lookup failed"));
        return;
    }

    pstate = &client->pathState;

    if (OI_SUCCESS(status)) {
        pstate->currPb = pstate->destPb;
    }
    OI_TRACE_USER(("Calling SetPathDone(<*client = %x>, status = %d)",
                          client, status));
    pstate->SetPathDone(client, status);
}

/**
 * SetPath()
 *
 * This function is the brains behind deciding which SETPATH command to send
 * next to get to the correct directory.
 */
static void SetPath(PBAP_CLIENT *client, PBAP_PATH_STATE *pstate)
{
    OI_STATUS status = OI_OK;
    OI_OBEX_UNICODE upath;

    OI_TRACE_USER(("SetPath(<*client = %x>, <*pstate = %x>)", client, pstate));

    if (pstate->destRepo != pstate->currRepo) {
        if (pstate->currRepo == OI_PBAP_LOCAL_REPOSITORY &&
            !pstate->inTelecom) {
            if (pstate->destRepo != OI_PBAP_INVALID_REPOSITORY) {
                upath.str = (OI_CHAR16*)OI_PBAP_usim1;
                upath.len = OI_StrLenUtf16(OI_PBAP_usim1) + 1;  /* Count the nul terminator too. */
                OI_DBG_PRINT2(("dest path: \"%s\"\n", upath.str, upath.len));
                status = OI_OBEXCLI_SetPath(client->id, &upath, TRUE, FALSE, ToSim1DirCfm, NULL);
            } else {
                OI_TRACE_USER(("Calling SetPathDone(<*client = %x>, status = %d)",
                                      client, status));
                pstate->SetPathDone(client, status);
            }
        } else {
            OI_DBG_PRINT2(("dest path: <root>\n"));
            status = OI_OBEXCLI_SetPath(client->id, NULL, TRUE, FALSE, ToRootDirCfm, NULL);
        }
    } else {
        if (!pstate->inTelecom) {
            upath.str = (OI_CHAR16*)OI_PBAP_utelecom;
            upath.len = OI_StrLenUtf16(OI_PBAP_utelecom) + 1;  /* Count the nul terminator too. */
            OI_DBG_PRINT2(("dest path: \"%s\"\n", upath.str, upath.len));
            status = OI_OBEXCLI_SetPath(client->id, &upath, TRUE, FALSE, ToTelecomDirCfm, NULL);
        } else {
            if (pstate->destPb != pstate->currPb) {
                if (pstate->currPb == OI_PBAP_INVALID_PHONEBOOK) {
                    upath.str = (OI_CHAR16*)OI_PBAP_upbdirs[pstate->destPb];
                    upath.len = OI_PBAP_upbdirsizes[pstate->destPb];
                    OI_DBG_PRINT2(("dest path: \"%s\"\n", upath.str, upath.len));
                    status = OI_OBEXCLI_SetPath(client->id, &upath, TRUE, FALSE, ToPbDirCfm, NULL);
                } else {
                    OI_DBG_PRINT2(("dest path: <up>\n"));
                    status = OI_OBEXCLI_SetPath(client->id, NULL, TRUE, TRUE, ToTelecomDirCfm, NULL);
                }
            } else {
                OI_TRACE_USER(("Calling SetPathDone(<*client = %x>, status = %d)",
                                      client, status));
                pstate->SetPathDone(client, status);
            }
        }
    }
    if (!OI_SUCCESS(status)) {
        OI_TRACE_USER(("Calling SetPathDone(<*client = %x>, status = %d)",
                              client, status));
        pstate->SetPathDone(client, status);
    }
}

/**
 * SetPhonebookDir()
 *
 * This function serves as an entry point for the rest of the PBAP client code
 * for ensuring that the correct directory is set.
 */
static void SetPhonebookDir(PBAP_CLIENT *client,
                            OI_PBAP_REPOSITORY repo,
                            OI_PBAP_PHONEBOOK pb,
                            PBAP_SET_PATH_DONE SetPathDone)
{
    PBAP_PATH_STATE *pstate = &client->pathState;

    OI_TRACE_USER(("SetPhonebookDir(<*client = %x>, repo = %d, pb = %d, <SetPathDone = %x>)",
                          client, repo, pb, SetPathDone));

    pstate->destRepo = repo;
    pstate->destPb = pb;
    pstate->SetPathDone = SetPathDone;

    SetPath(client, pstate);
}


/************************************************************************
 *
 * Application Parameter handling utility functions
 *
 ************************************************************************/

/**
 * ExtractAppParams()
 *
 * This function just abstracts away the code that parses the the GET Response
 * APPLICATION PARAMETERs.
 */
static void ExtractAppParams(PBAP_CLIENT *client, OI_OBEX_HEADER *hdr)
{
    OI_BYTE_STREAM bs;
    OI_UINT8 tagId = 0;
    OI_UINT8 tagLen = 0;
    OI_UINT16 bufLen = hdr->val.applicationParams.len;

    ByteStream_Init(bs, hdr->val.applicationParams.data, bufLen);
    ByteStream_Open(bs, BYTESTREAM_READ);

    while (bufLen) {
        ByteStream_GetUINT8_Checked(bs, tagId);
        ByteStream_GetUINT8_Checked(bs, tagLen);
        bufLen -= 2 * sizeof(OI_UINT8);

        if (tagId == OI_PBAP_TAG_ID_NEW_MISSED_CALLS) {
            ByteStream_GetUINT8_Checked(bs, client->req.results.newMissedCalls);
        } else if (tagId == OI_PBAP_TAG_ID_PHONEBOOK_SIZE) {
            ByteStream_GetUINT16_Checked(bs, client->req.results.phonebookSize, OI_BIG_ENDIAN_BYTE_ORDER);
        } else {
            OI_DBG_PRINT2(("Ingnoring unexpected Application Parameter: %d\n", tagId));
            ByteStream_Skip_Checked(bs, tagLen);
        }
        bufLen -= tagLen;

        if(ByteStream_Error(bs)) {
            OI_LOG_ERROR(("Failed to parse application parameters"));
            break;
        }
    }

    ByteStream_Close(bs);
}

/**
 * SetAppParamTagInfo()
 *
 * Helper macro to reduce the tedium of setting the tag ID and tag length.
 */
#define SetAppParamTagInfo(bs, tagId, tagLen)   \
    do {                                        \
        ByteStream_PutUINT8((bs), (tagId));     \
        ByteStream_PutUINT8((bs), (tagLen));    \
    } while (0)

/**
 * BuildAppParams()
 *
 * Helper function to build the APPLICATION PARAMETERs.
 */
static OI_STATUS BuildAppParams(PBAP_CLIENT *client,
                                const OI_PBAP_ORDER_TAG_VALUES *order,
                                const OI_PBAP_SEARCH_ATTRIBUTE_TAG_VALUES *searchAttribute,
                                const OI_BYTE *searchValue,
                                OI_UINT8 searchValueLen,
                                const OI_UINT16 *listStartOffset,
                                const OI_UINT16 *maxListCount,
                                const OI_UINT64 *filter,
                                const OI_PBAP_FORMAT_TAG_VALUES *format,
                                const OI_UINT32 *supportedFeatures)
{
    OI_BYTE_STREAM bs;
    OI_UINT16 paramLen = ((order           ? OI_PBAP_APP_PARAM_ORDER_SIZE                       : 0) +
                          (searchAttribute ? OI_PBAP_APP_PARAM_SEARCHATTRIBUTE_SIZE             : 0) +
                          (searchValue     ? OI_PBAP_APP_PARAM_SEARCHVALUE_SIZE(searchValueLen) : 0) +
                          (listStartOffset ? OI_PBAP_APP_PARAM_LISTSTARTOFFSET_SIZE             : 0) +
                          (maxListCount    ? OI_PBAP_APP_PARAM_MAXLISTCOUNT_SIZE                : 0) +
                          (filter          ? OI_PBAP_APP_PARAM_FILTER_SIZE                      : 0) +
                          (format          ? OI_PBAP_APP_PARAM_FORMAT_SIZE                      : 0) +
                          (supportedFeatures ? OI_PBAP_APP_PARAM_SUPP_FEATURE_SIZE              : 0));

    if (paramLen == 0) {
        client->req.appParam = NULL;
        client->req.appParamLen = 0;
        return OI_OK;
    }

    client->req.appParam = OI_Malloc(paramLen);
    if (!client->req.appParam) {
        return OI_STATUS_OUT_OF_MEMORY;
    }

    ByteStream_Init(bs, client->req.appParam, paramLen);
    ByteStream_Open(bs, BYTESTREAM_WRITE);

    if (filter) {
        SetAppParamTagInfo(bs, OI_PBAP_TAG_ID_FILTER, sizeof(OI_UINT64));
        ByteStream_PutUINT32(bs, filter->I1, OI_BIG_ENDIAN_BYTE_ORDER);
        ByteStream_PutUINT32(bs, filter->I2, OI_BIG_ENDIAN_BYTE_ORDER);
    }

    if (format) {
        SetAppParamTagInfo(bs, OI_PBAP_TAG_ID_FORMAT, sizeof(OI_UINT8));
        ByteStream_PutUINT8(bs, (OI_UINT8)*format);
    }

    if (order) {
        SetAppParamTagInfo(bs, OI_PBAP_TAG_ID_ORDER, sizeof(OI_UINT8));
        ByteStream_PutUINT8(bs, (OI_UINT8)*order);
    }

    if (searchValue) {
        SetAppParamTagInfo(bs, OI_PBAP_TAG_ID_SEARCH_VALUE, searchValueLen);
        ByteStream_PutBytes(bs, searchValue, searchValueLen);

        if (searchAttribute) {
            SetAppParamTagInfo(bs, OI_PBAP_TAG_ID_SEARCH_ATTRIBUTE, sizeof(OI_UINT8));
            ByteStream_PutUINT8(bs, (OI_UINT8)*searchAttribute);
        }
    }

    if (listStartOffset) {
        SetAppParamTagInfo(bs, OI_PBAP_TAG_ID_LIST_START_OFFSET, sizeof(OI_UINT16));
        ByteStream_PutUINT16(bs, *listStartOffset, OI_BIG_ENDIAN_BYTE_ORDER);
    }

    if (maxListCount) {
        SetAppParamTagInfo(bs, OI_PBAP_TAG_ID_MAX_LIST_COUNT, sizeof(OI_UINT16));
        ByteStream_PutUINT16(bs, *maxListCount, OI_BIG_ENDIAN_BYTE_ORDER);
    }

    if (supportedFeatures) {
        SetAppParamTagInfo(bs, OI_PBAP_TAG_ID_SUPP_FEATURES, sizeof(OI_UINT32));
        ByteStream_PutUINT32(bs, *supportedFeatures, OI_BIG_ENDIAN_BYTE_ORDER);
    }

    ByteStream_Close(bs);
    client->req.appParamLen = ByteStream_GetSize(bs);

    return OI_OK;
}


/************************************************************************
 *
 * GET Request functions.
 *
 ************************************************************************/

static void PbapDataCB(OI_OBEXCLI_CONNECTION_HANDLE connectionId,
                       OI_OBEX_HEADER_LIST *rspHeaders,
                       OI_STATUS rcvStatus);

/**
 * PullAbortCfm()
 *
 * This function handles the abort confirmation resulting from a get/pull error.
 */
static void PullAbortCfm(OI_OBEXCLI_CONNECTION_HANDLE connection)
{
    PBAP_CLIENT *client = PbapClient(connection);

    if (client == NULL)
        return;

    OI_TRACE_USER(("PullAbortCfm(connection = %d)", connection));
    OI_DBG_PRINT2(("Closing file 0x%x\n", client->fh));
    FileClose(client, OI_OBEX_FILEOP_ERROR);
}

/**
 * PbapWriteCfm()
 *
 * This function is called by the application to inform the stack of the
 * success or failure of the write operation.
 */
static void PbapWriteCfm(OI_PBAP_HANDLE handle,
                         OI_STATUS status,
                         OI_PBAP_CONNECTION pbapConnection)
{
    PBAP_CLIENT *client = PbapClient(pbapConnection);

    if (client == NULL)
        return;

    OI_TRACE_USER(("PbapWriteCfm(<*handle = %x>, status = %d, pbapConnection = %d)",
                       handle, status, pbapConnection));

    VALIDATE_CFM_STATE("PbapWriteCfm", client,
                       ((client->state == CLIENT_STATE_PULLING_VCARD) ||
                        (client->state == CLIENT_STATE_PULLING_PHONEBOOK) ||
                        (client->state == CLIENT_STATE_PULLING_VCARD_LISTING)));

    if (!client->final) {
        if (!OI_SUCCESS(status)) {
            OI_DBG_PRINT1(("File write returned error %d\n", status));

            /*
             * Let OBEX know we are terminating the GET because of an error.
             */
            status = OI_OBEXCLI_Abort(client->id, PullAbortCfm);
        } else {
            status = OI_OBEXCLI_Get(client->id, NULL, PbapDataCB, TRUE);
            if (OI_SUCCESS(status)) {
                return;
            }
            OI_DBG_PRINT1(("OI_OBEXCLI_Get returned error %d\n", status));
        }
    }

    if (client->final || !OI_SUCCESS(status)) {
        OI_DBG_PRINT2(("Closing file 0x%x\n", client->fh));
        FileClose(client, status);
    }
}

/**
 * PbapDataCB()
 *
 * This function handles the GET Response from the PBAP server.  If there is
 * data to be written to the file handle it will do so.
 */
static void PbapDataCB(OI_OBEXCLI_CONNECTION_HANDLE connectionId,
                       OI_OBEX_HEADER_LIST *rspHeaders,
                       OI_STATUS rcvStatus)
{
    PBAP_CLIENT *client = PbapClient(connectionId);
    OI_INT i;
    OI_OBEX_HEADER *hdr;
    OI_OBEX_HEADER *bodyHdr = NULL;
    OI_STATUS status;

    OI_TRACE_USER(("PbapDataCB(connectionId = %d, <*rspHeaders = %x>, rcvStatus = %d)",
                          connectionId, rspHeaders, rcvStatus));

    if (client == NULL)
        return;

    if (!IS_CLIENT_CONNECTED) {
        OI_SLOG_ERROR(OI_STATUS_NONE, ("PbapDataCB while PBAP not connected"));
        return;
    }

    if (OI_SUCCESS(rcvStatus) || rcvStatus == OI_OBEX_CONTINUE) {
        OI_PBAP_DumpObexHeaders(rspHeaders);
        for (i = 0; i < rspHeaders->count; i++) {
            hdr = &rspHeaders->list[i];
            switch (hdr->id) {
            case OI_OBEX_HDR_BODY:
            case OI_OBEX_HDR_END_OF_BODY:
                bodyHdr = hdr;
                break;

            case OI_OBEX_HDR_APPLICATION_PARAMS:
                ExtractAppParams(client, hdr);
                break;

            default:
                OI_DBG_PRINT2(("Received unexpected obex header %2x", hdr->id));
                break;
            }
        }

        client->final = (rcvStatus == OI_OK);

        /* Check to see if we're just supposed to be getting the phonebook
         * size. */
        if (client->state == CLIENT_STATE_PULLING_PHONEBOOK_SIZE) {
            if (!client->final) {
                /* The entire OBEX response should be in one packet
                 * considering that the only thing we should be getting is
                 * just the phonebook size. */
                OI_SLOG_ERROR(OI_STATUS_NONE,
                              ("PBAP server erroneously indicates more data to get while getting the phonebook size"));
                OI_OBEXCLI_Abort((OI_OBEXCLI_CONNECTION_HANDLE)connectionId, NULL);
            }
        } else {
            client->fileOpCfmPending = TRUE;
            if ((bodyHdr == NULL) || (bodyHdr->val.body.len == 0)) {
                /*
                 * Nothing to write to file this time.
                 */
                PbapWriteCfm(client->fh, OI_OK, client->id);
                return;
            }

            OI_TRACE_USER(("Calling write(<*handle = %x>, <*data = %x>, size = %d, <*writeCfm = %x>, id = %d)",
                              client->fh, bodyHdr->val.body.data, bodyHdr->val.body.len,
                              PbapWriteCfm, client->id));
            status = client->fops->write(client->fh, bodyHdr->val.body.data, bodyHdr->val.body.len,
                                         PbapWriteCfm, client->id);
            if (!OI_SUCCESS(status)) {
                OI_SLOG_ERROR(status, ("App Write failed"));
                OI_OBEXCLI_Abort((OI_OBEXCLI_CONNECTION_HANDLE)connectionId, PullAbortCfm);
            }
        }
    } else {
        OI_SLOG_WARNING(rcvStatus, ("PBAP: Failed to get phonebook data"));
    }

    if (rcvStatus != OI_OBEX_CONTINUE) {
        /* FileClose does more than just close the file handle, and we need
         * that other functionality here. */
        FileClose(client, rcvStatus);
    }
}

/**
 * FinishReq()
 *
 * This function actually issues the OBEX GET request.  It is common to all
 * PBAP GET requests and will get called after entering the appropriate
 * directory and the application's file is opened if applicable.
 */
static void FinishReq(PBAP_CLIENT *client, OI_STATUS status)
{
    OI_OBEX_HEADER hdr[3];
    OI_INT hdrCnt = 0;
    OI_OBEX_HEADER_LIST hdrList;

    OI_TRACE_USER(("FinishReq(<*client = %x>, status = %d)", client, status));

    if (OI_SUCCESS(status)) {
        client->req.results.phonebookSize = 0;
        client->req.results.newMissedCalls = 0;

        hdr[0].id = OI_OBEX_HDR_NAME;
        hdr[0].val.name.str = client->req.pathStr;
        hdr[0].val.name.len = client->req.pathStrLen;
        hdrCnt++;

        hdr[1].id = OI_OBEX_HDR_TYPE;
        hdr[1].val.type.data = (OI_BYTE*)client->req.type;
        hdr[1].val.type.len = client->req.typeLen;
        hdrCnt++;

        if (client->req.appParam) {
            hdr[2].id = OI_OBEX_HDR_APPLICATION_PARAMS;
            hdr[2].val.applicationParams.data = client->req.appParam;
            hdr[2].val.applicationParams.len = client->req.appParamLen;
            hdrCnt++;
        }

        hdrList.list = hdr;
        hdrList.count = hdrCnt;

        OI_PBAP_DumpObexHeaders(&hdrList);

        client->final = FALSE;
        status = OI_OBEXCLI_Get(client->id, &hdrList, PbapDataCB, TRUE);
    }

    if (!OI_SUCCESS(status)) {
        PbapDataCB(client->id, NULL, status);
    }
}

/**
 * PbapOpenCfm()
 *
 * This function is called by the application to indicate the success or
 * failure of the open operation and to provide the handle for writing data.
 */
static void PbapOpenCfm(OI_PBAP_HANDLE handle,
                        OI_STATUS openStatus,
                        OI_PBAP_CONNECTION pbapConnection)
{
    PBAP_CLIENT *client = PbapClient(pbapConnection);

    OI_TRACE_USER(("PbapOpenCfm(handle = %x, openStatus = %d, pbapConnection = %d)",
                      handle, openStatus, pbapConnection));

    if (client) {
        VALIDATE_CFM_STATE("PbapOpenCfm", client,
                           ((client->state == CLIENT_STATE_PULLING_VCARD) ||
                            (client->state == CLIENT_STATE_PULLING_PHONEBOOK) ||
                            (client->state == CLIENT_STATE_PULLING_VCARD_LISTING)));

        if (!IS_CLIENT_CONNECTED) {
            OI_SLOG_ERROR(OI_STATUS_NONE, ("PbapOpenCfm while PBAP not connected"));
            return;
        }

        if (OI_SUCCESS(openStatus)) {
            client->fileOpen = TRUE;
            client->fh = handle;
            FinishReq(client, OI_OK);
        } else {
            FileClose(client, openStatus);
        }
    }
}

/**
 * PbapOpen()
 *
 * This function opens a handle to the application for writing data to.
 */
static void PbapOpen(PBAP_CLIENT *client, OI_STATUS status)
{
    OI_OBEX_UNICODE path;

    OI_TRACE_USER(("PbapOpen(<*client = %x>, status = %d)\n", client, status));

    path.str = client->req.pathStr;
    path.len = client->req.pathStrLen;

    if (OI_SUCCESS(status)) {
        client->fileOpCfmPending = TRUE;
        OI_TRACE_USER(("Calling open(path = \"%s\", <*openCfm = %x>, id = %d)",
                          path.str, PbapOpenCfm, client->id));
        status = client->fops->open(&path, PbapOpenCfm, (OI_PBAP_CONNECTION)client->id);
        if (!OI_SUCCESS(status)) {
            OI_SLOG_ERROR(status, ("File Op Open failed"));
            setState(client, CLIENT_STATE_CONNECTED);
        }
    } else {
        FileClose(client, status);
    }
}

/**
 * PbapSetPathDone()
 *
 * This function builds the appropriate path name to the desired object after
 * all the requisite SETPATHs have successfully been performed.  It will also
 * call the application's Open callback to open file handle if necessary. */
static void PbapSetPathDone(PBAP_CLIENT *client, OI_STATUS status)
{
    if (OI_SUCCESS(status)) {
        client->req.pathStr[0] = 0;

        switch (client->state) {
        case CLIENT_STATE_PULLING_PHONEBOOK:
            client->req.pathStrLen = BuildPbVCardPathName(&client->pathState, client->req.repo,
                                                          client->req.pb, client->req.pathStr);
            OI_DBG_PRINT1(("PBAP Pull Phonebook: \"%s\"\n", client->req.pathStr));
            PbapOpen(client, status);
            break;

        case CLIENT_STATE_PULLING_VCARD:
            client->req.pathStrLen = BuildVCardPathName(&client->pathState, client->req.repo,
                                                        client->req.pb, client->req.entry,
                                                        client->req.pathStr);
            OI_DBG_PRINT1(("PBAP Pull vCard: \"%s\"\n", client->req.pathStr));
            PbapOpen(client, status);
            break;

        case CLIENT_STATE_PULLING_PHONEBOOK_SIZE:
            if (client->pathState.currPb == OI_PBAP_INVALID_PHONEBOOK) {
                client->req.pathStrLen = BuildPbVCardPathName(&client->pathState, client->req.repo,
                                                              client->req.pb, client->req.pathStr);
            } else {
                client->req.pathStrLen = BuildPbDirPathName(&client->pathState, client->req.repo,
                                                            client->req.pb, client->req.pathStr);
            }
            OI_DBG_PRINT1(("PBAP Get Phonebook Size: \"%s\"\n", client->req.pathStr));
            FinishReq(client, status);
            break;

        case CLIENT_STATE_PULLING_VCARD_LISTING:
            client->req.pathStrLen = BuildPbDirPathName(&client->pathState, client->req.repo,
                                                        client->req.pb, client->req.pathStr);
            OI_DBG_PRINT1(("PBAP List Phonebook vCards: \"%s\"\n", client->req.pathStr));
            PbapOpen(client, status);
            break;

        default:
            OI_SLOG_ERROR(OI_FAIL, ("Finish building PBAP GET request in invalid state: %d",
                                    client->state));
            FinishReq(client, OI_FAIL);
        }
    } else {
        OI_SLOG_ERROR(status, ("SET PATH failed"));
        FinishReq(client, status);
    }
}

/**
 * OI_PBAPClient_GetPhonebookSize()
 *
 * This function is the API used by an application to request the size of a
 * phonebook.
 */
OI_STATUS OI_PBAPClient_GetPhonebookSize(OI_PBAP_CONNECTION connectionId,
                                         OI_PBAP_REPOSITORY repository,
                                         OI_PBAP_PHONEBOOK phonebook,
                                         OI_PBAP_CLIENT_GET_PHONEBOOK_SIZE_CB getPhonebookSizeCB)
{
    PBAP_CLIENT *client = PbapClient(connectionId);
    OI_STATUS status;
    OI_UINT16 maxListCount = 0;  /* Get the number of entries in the phonebook */

    OI_TRACE_USER(("OI_PBAPClient_GetPhonebookSize(connectionId = %d, repository = %d, phonebook = %d, <getPhonebookSizeCB = %x>)",
                      connectionId, repository, phonebook, getPhonebookSizeCB));

    OI_ARGCHECK(repository < OI_PBAP_INVALID_REPOSITORY);
    OI_ARGCHECK(phonebook < OI_PBAP_INVALID_PHONEBOOK);
    OI_ARGCHECK(getPhonebookSizeCB != NULL);

    if (!IS_CLIENT_CONNECTED) {
        OI_DBG_PRINT1(("Not connected"));
        return OI_OBEX_NOT_CONNECTED;
    }

    client->req.cb.getSize = getPhonebookSizeCB;
    client->req.repo = repository;
    client->req.pb = phonebook;

    status = BuildAppParams(client,
                            NULL,               /* order (N/A) */
                            NULL,               /* search attribute (N/A) */
                            NULL,               /* search value (N/A) */
                            0,                  /* search value len (N/A) */
                            NULL,               /* list start offset (N/A) */
                            &maxListCount,      /* max list count */
                            NULL,               /* filter (N/A) */
                            NULL,               /* format (N/A) */
                            NULL);              /* supported features (N/A) */
    if (!OI_SUCCESS(status)) {
        return status;
    }

    setState(client, CLIENT_STATE_PULLING_PHONEBOOK_SIZE);

    if ((client->pathState.currPb == OI_PBAP_INVALID_PHONEBOOK) ||
        (client->pathState.currRepo != client->req.repo)) {
        client->req.type = OI_PBAP_PHONEBOOK_TYPE;
        client->req.typeLen = sizeof(OI_PBAP_PHONEBOOK_TYPE);

        /* Make sure we are in the <root> directory. */
        SetPhonebookDir(client, OI_PBAP_INVALID_REPOSITORY, OI_PBAP_INVALID_PHONEBOOK, PbapSetPathDone);
    } else {
        client->req.type = OI_PBAP_VCARD_LISTING_TYPE;
        client->req.typeLen = sizeof(OI_PBAP_VCARD_LISTING_TYPE);

        /* Make sure we are in the right directory. */
        SetPhonebookDir(client, repository, phonebook, PbapSetPathDone);
    }

    return OI_OK;
}

/**
 * OI_PBAPClient_PullPhonebook()
 *
 * This function is the API used by an application to request the entire
 * contents of a phonebook.
 */
OI_STATUS OI_PBAPClient_PullPhonebook(OI_PBAP_CONNECTION connectionId,
                                      OI_PBAP_REPOSITORY repository,
                                      OI_PBAP_PHONEBOOK phonebook,
                                      const OI_UINT64 *filter,
                                      OI_PBAP_FORMAT_TAG_VALUES format,
                                      OI_UINT16 maxListCount,
                                      OI_UINT16 listStartOffset,
                                      OI_PBAP_CLIENT_PULL_PHONEBOOK_CB pullPhonebookCB)
{
    PBAP_CLIENT *client = PbapClient(connectionId);
    OI_STATUS status;

    OI_TRACE_USER(("OI_PBAPClient_PullPhonebook(connectionId = %d, repository = %d, phonebook = %d, filter = %x-%x, format = %d, maxListCount = %d, listStartOffset = %d, <pullPhonebookCB = %x>)",
                      connectionId, repository, phonebook,
                      filter ? filter->I1 : 0, filter ? filter->I2 : 0,
                      format, maxListCount, listStartOffset, pullPhonebookCB));

    OI_ARGCHECK(repository < OI_PBAP_INVALID_REPOSITORY);
    OI_ARGCHECK(phonebook < OI_PBAP_INVALID_PHONEBOOK);
    OI_ARGCHECK(pullPhonebookCB != NULL);

    if (!IS_CLIENT_CONNECTED) {
        OI_DBG_PRINT1(("Not connected"));
        return OI_OBEX_NOT_CONNECTED;
    }

    client->req.cb.pullPb = pullPhonebookCB;
    client->req.repo = repository;
    client->req.pb = phonebook;

    client->req.type = OI_PBAP_PHONEBOOK_TYPE;
    client->req.typeLen = sizeof(OI_PBAP_PHONEBOOK_TYPE);

    status = BuildAppParams(client,
                            NULL,               /* order (N/A) */
                            NULL,               /* search attribute (N/A) */
                            NULL,               /* search value (N/A) */
                            0,                  /* search value len (N/A) */
                            &listStartOffset,   /* list start offset */
                            &maxListCount,      /* max list count */
                            filter,             /* filter */
                            &format,            /* format */
                            NULL);              /* supported features (N/A) */
    if (!OI_SUCCESS(status)) {
        return status;
    }

    setState(client, CLIENT_STATE_PULLING_PHONEBOOK);

    /* Make sure we are in the <root> directory. */
    SetPhonebookDir(client, OI_PBAP_INVALID_REPOSITORY, OI_PBAP_INVALID_PHONEBOOK, PbapSetPathDone);

    return OI_OK;
}

/**
 * OI_PBAPClient_PullvCardListing()
 *
 * This function is the API used by the appliction to get a vCard listing from
 * the PBAP server.
 */
OI_STATUS OI_PBAPClient_PullvCardListing(OI_PBAP_CONNECTION connectionId,
                                         OI_PBAP_REPOSITORY repository,
                                         OI_PBAP_PHONEBOOK phonebook,
                                         OI_PBAP_ORDER_TAG_VALUES order,
                                         OI_PBAP_SEARCH_ATTRIBUTE_TAG_VALUES searchAttribute,
                                         OI_BYTE *searchValue,
                                         OI_UINT8 searchValueLen,
                                         OI_UINT16 maxListCount,
                                         OI_UINT16 listStartOffset,
                                         OI_PBAP_CLIENT_PULL_VCARD_LISTING_CB pullvCardListingCB)
{
    PBAP_CLIENT *client = PbapClient(connectionId);
    OI_STATUS status;

    OI_ARGCHECK(repository < OI_PBAP_INVALID_REPOSITORY);
    OI_ARGCHECK(phonebook < OI_PBAP_INVALID_PHONEBOOK);
    OI_ARGCHECK(pullvCardListingCB != NULL);

    OI_TRACE_USER(("OI_PBAPClient_PullvCardListing(connectionId = %d, repository = %d, phonebook = %d, order = %d, searchAttribute = %d, searchValue = \"%s\", searchValueLen = %d maxListCount = %d, listStartOffset = %d, <pullvCardListingCB = %x>)",
                      connectionId, repository, phonebook, order, searchAttribute,
                      searchValue ? (const OI_CHAR*)searchValue : "<null>", searchValueLen,
                      maxListCount, listStartOffset, pullvCardListingCB));

    if (!IS_CLIENT_CONNECTED) {
        OI_DBG_PRINT1(("Not connected"));
        return OI_OBEX_NOT_CONNECTED;
    }

    client->req.cb.pullList = pullvCardListingCB;
    client->req.repo = repository;
    client->req.pb = phonebook;

    client->req.type = OI_PBAP_VCARD_LISTING_TYPE;
    client->req.typeLen = sizeof(OI_PBAP_VCARD_LISTING_TYPE);

    status = BuildAppParams(client,
                            &order,             /* order */
                            &searchAttribute,   /* search attribute */
                            searchValue,        /* search value */
                            searchValueLen,     /* search value len */
                            &listStartOffset,   /* list start offset */
                            &maxListCount,      /* max list count */
                            NULL,               /* filter (N/A) */
                            NULL,               /* format (N/A) */
                            NULL);              /* supported features (N/A) */
    if (!OI_SUCCESS(status)) {
        return status;
    }

    setState(client, CLIENT_STATE_PULLING_VCARD_LISTING);

    SetPhonebookDir(client, repository, phonebook, PbapSetPathDone);

    return OI_OK;
}

/**
 * OI_PBAPClient_Pull_vCard_Entry()
 *
 * This function is the API used by the application to request a specific
 * vCard from the PBAP server.
 */
OI_STATUS OI_PBAPClient_PullvCardEntry(OI_PBAP_CONNECTION connectionId,
                                       OI_PBAP_REPOSITORY repository,
                                       OI_PBAP_PHONEBOOK phonebook,
                                       OI_UINT32 entry,
                                       const OI_UINT64 *filter,
                                       OI_PBAP_FORMAT_TAG_VALUES format,
                                       OI_PBAP_CLIENT_PULL_VCARD_ENTRY_CB pullvCardCB)
{
    PBAP_CLIENT *client = PbapClient(connectionId);
    OI_STATUS status;

    OI_TRACE_USER(("OI_PBAPClient_Pull_vCard_Entry(connectionId = %d, repository = %d, phonebook = %d, entry = %d, filter = %x-%x, format = %d, <pullvCardCB = %x>)",
                      connectionId, repository, phonebook, entry,
                      filter ? filter->I1 : 0, filter ? filter->I2 : 0,
                      format, pullvCardCB));

    OI_ARGCHECK(repository < OI_PBAP_INVALID_REPOSITORY);
    OI_ARGCHECK(phonebook < OI_PBAP_INVALID_PHONEBOOK);
    OI_ARGCHECK(pullvCardCB != NULL);

    if (!IS_CLIENT_CONNECTED) {
        OI_DBG_PRINT1(("Not connected"));
        return OI_OBEX_NOT_CONNECTED;
    }

    client->req.cb.pullVCard = pullvCardCB;
    client->req.repo = repository;
    client->req.pb = phonebook;
    client->req.entry = entry;

    client->req.type = OI_PBAP_VCARD_TYPE;
    client->req.typeLen = sizeof(OI_PBAP_VCARD_TYPE);

    status = BuildAppParams(client,
                            NULL,       /* order (N/A) */
                            NULL,       /* search attribute (N/A) */
                            NULL,       /* search value (N/A) */
                            0,          /* search value len (N/A) */
                            NULL,       /* list start offset (N/A) */
                            NULL,       /* max list count (N/A) */
                            filter,     /* filter */
                            &format,    /* format */
                            NULL);      /* supported features (N/A) */
    if (!OI_SUCCESS(status)) {
        return status;
    }

    setState(client, CLIENT_STATE_PULLING_VCARD);

    SetPhonebookDir(client, repository, phonebook, PbapSetPathDone);

    return OI_OK;
}

/**
 * AbortCfm()
 *
 * This function handles the deferred abort confirmation.
 */
static void AbortCfm(OI_OBEXCLI_CONNECTION_HANDLE connection)
{
    PBAP_CLIENT *client = PbapClient(connection);

    OI_TRACE_USER(("AbortCfm(connection = %d)\n", connection));

    if (!IS_CLIENT_CONNECTED) {
        OI_SLOG_ERROR(OI_STATUS_NONE, ("AbortCfm while PBAP not connected"));
        return;
    }

    if (client->abortCfm) {
        OI_TRACE_USER(("Calling abortCfm(connection = %d)", connection));
        client->abortCfm((OI_PBAP_CONNECTION)connection);
    }

    if (client->state == CLIENT_STATE_ABORTING) {
        setState(client, CLIENT_STATE_CONNECTED);
    }
}

/**
 * OI_PBAPClient_Abort()
 *
 * This function tells the connected PBAP server to abort the data transfer.
 */
OI_STATUS OI_PBAPClient_Abort(OI_PBAP_CONNECTION connectionId, OI_PBAP_ABORT_CFM abortCfm)
{
    PBAP_CLIENT *client = PbapClient(connectionId);
    OI_STATUS status;

    OI_TRACE_USER(("OI_PBAPClient_Abort(connectionId = %d, <abortCfm = %x>)",
                      connectionId, abortCfm));

    if (!IS_CLIENT_CONNECTED) {
        OI_DBG_PRINT1(("Not connected"));
        return OI_OBEX_NOT_CONNECTED;
    }

    client->abortCfm = abortCfm;
    status = OI_OBEXCLI_Abort((OI_OBEXCLI_CONNECTION_HANDLE)connectionId, AbortCfm);
    if (OI_SUCCESS(status)) {
        setState(client, CLIENT_STATE_ABORTING);
    }

    return status;
}


/************************************************************************
 *
 * Connection management functions.
 *
 ************************************************************************/

/**
 * ClientConnectCfm()
 *
 * This function indicates the success or failure of a connection attempt to a
 * PBAP server.
 */
static void ClientConnectCfm(OI_OBEXCLI_CONNECTION_HANDLE connectionId,
                             OI_BOOL readOnly,
                             OI_STATUS status)
{
    PBAP_CLIENT *client = PbapClient(connectionId);
    OI_PBAP_CONNECTION_CFM connectionCfm;

    OI_TRACE_USER(("ClientConnectCfm(connectionId = %d, readOnly = %d, status = %d)",
                          connectionId, readOnly, status));

    if (!client) {
        OI_SLOG_ERROR(OI_STATUS_NONE, ("connectionId lookup failure"));
        return;
    }

    connectionCfm = client->connectionCfm;

    if (OI_SUCCESS(status)) {
        OI_ASSERT(connectionId == client->id);
        setState(client, CLIENT_STATE_CONNECTED);
    } else {
        OI_SLOG_WARNING(status, ("PBAP client connect failed"));
        OI_Free(client);
        client = NULL;
        OI_INIT_FLAG_PUT_FLAG(FALSE, PBAP_CLI);
    }

    OI_TRACE_USER(("Calling connectionCfm(connectionId = %d, status = %d)",
                      connectionId, status));
    connectionCfm(connectionId, status);
}

/**
 * ClientDisconnectInd()
 *
 * This function indicates the disconnection from a PBAP server.
 */
static void ClientDisconnectInd(OI_OBEXCLI_CONNECTION_HANDLE connectionId)
{
    PBAP_CLIENT *client = PbapClient(connectionId);

    OI_TRACE_USER(("ClientDisconnectInd(connectionId = %d)", connectionId));

    if (!client) {
        OI_SLOG_ERROR(OI_STATUS_NONE, ("connectionID lookup failed"));
        return;
    }

    /*
     * record the disconnect event for subsequent processes
     */
    client->disconnected = TRUE;

    /* If there was a close pending we need to make sure it is called now
     * because we are about to free the client.
     */
    if (client->dispatchHandle) {
        OI_DBG_PRINT2(("Disconnect: calling the set repository/phonebook complete now"));
        OI_Dispatch_CallFunc(client->dispatchHandle);
        return;
    }

    if (client->fileOpCfmPending) {
        /*
         * We're waiting for a file op confirm callback, just have to keep
         * waiting
         */
    } else {
        /*
         * Status to be reported to the file op code and app depends on
         * whether we're idle or getting/putting.
         */
        OI_DBG_PRINT2(("Closing file handle %x\n", client->fh));
        FileClose(client, client->state > CLIENT_STATE_CONNECTED ? OI_OBEX_NOT_CONNECTED : OI_OK);
    }
}

/**
 * ClientAuthenticationInd()
 *
 * This function informs the application of the need for authentication.
 */
static void ClientAuthenticationInd(OI_OBEXCLI_CONNECTION_HANDLE connectionId,
                                    OI_BOOL userIdRequired,
                                    OI_OBEX_REALM *realm)
{
    PBAP_CLIENT *client = PbapClient(connectionId);

    OI_TRACE_USER(("ClientAuthenticationInd(connectionId = %d, userIdRequired = %d)",
                          connectionId, userIdRequired));

    if (!client) {
        OI_SLOG_ERROR(OI_STATUS_NONE, ("connectionID lookup failed"));
        return;
    }

    setState(client, CLIENT_STATE_AUTHENTICATING);
    OI_TRACE_USER(("Calling authenticationCB(connectionId = %d, userIdRequired = %d)",
                      connectionId, userIdRequired));
    client->authenticationCB(connectionId, userIdRequired);
}

OI_STATUS OI_PBAPClient_Connect(OI_BD_ADDR *addr,
                                OI_OBEX_LOWER_PROTOCOL *lowerProtocol,
                                OI_OBEXCLI_AUTHENTICATION authentication,
                                OI_PBAP_CONNECTION *connectionId,
                                OI_UINT32 supportedFeatures,
                                OI_PBAP_CONNECTION_CFM connectionCfm,
                                OI_PBAP_DISCONNECTION_IND disconnectInd,
                                OI_PBAP_CLIENT_AUTHENTICATION_CB authenticationCB,
                                const OI_PBAP_CLIENT_FILESYS_FUNCTIONS *fops)
{
    static const OI_OBEXCLI_CB_LIST clientCallbacks = {
        ClientConnectCfm,           /* connectCfmCB         */
        ClientDisconnectInd,        /* disconnectIndCB      */
        ClientAuthenticationInd,    /* authChallengeIndCB   */
        NULL,                       /* progressIndCB        */
        NULL                        /* bulkPutCfm        */
    };
    PBAP_CLIENT *client;
    static OI_BYTE target[OI_OBEX_UUID_SIZE] = OI_PBAP_OBEX_TARGET_UUID;
    static OI_OBEX_HEADER_LIST hdrList;
    OI_INT hdrCnt = 0;
    static OI_OBEX_HEADER hdr[2];
    OI_OBEX_CONNECTION_OPTIONS connOpts;
    OI_STATUS status;

    OI_ARGCHECK(addr);
    OI_TRACE_USER(("OI_PBAPClient_Connect(bdaddr = %:, lowerProtocol = %x, <authentication = %x>, "
                    "<*connectionId = %x>, <connectionCfm = %x>, <disconnectInd = %x>, <authenticationCB = %x>,"
                    "<*fops = %x>)", addr, lowerProtocol, authentication, connectionId,
                    connectionCfm, disconnectInd, authenticationCB, fops));

    OI_ARGCHECK(connectionCfm && disconnectInd && authenticationCB &&
             (connectionId != NULL) && (fops != NULL));
    OI_ARGCHECK(fops->open && fops->close && fops->write);

    /* Current implementation only allows one PBAP connection at a time. */
    if (OI_INIT_FLAG_VALUE(PBAP_CLI)) {
        return OI_STATUS_ALREADY_CONNECTED;
    }

    client = OI_Calloc(sizeof(PBAP_CLIENT));
    if (client == NULL) {
        OI_SLOG_ERROR(OI_STATUS_INVALID_PARAMETERS,
                      ("Failed to allocate memory for PBAP client structure"));
        return OI_STATUS_OUT_OF_MEMORY;
    }

    client->pathState.currRepo = OI_PBAP_LOCAL_REPOSITORY;
    client->pathState.inTelecom = FALSE;
    client->pathState.currPb = OI_PBAP_INVALID_PHONEBOOK;

    client->connectionCfm = connectionCfm;
    client->disconnectInd = disconnectInd;
    client->authenticationCB = authenticationCB;
    client->fops = fops;

    /* Target header specifies the OBEX PBAP service. */
    hdr[0].id = OI_OBEX_HDR_TARGET;
    hdr[0].val.body.data = target;
    hdr[0].val.body.len = OI_OBEX_UUID_SIZE;
    hdrCnt ++;

    if (lowerProtocol->protocol == OI_OBEX_LOWER_L2CAP) {
        status = BuildAppParams(client,
                                NULL,               /* order (N/A) */
                                NULL,               /* search attribute (N/A) */
                                NULL,               /* search value (N/A) */
                                0,                  /* search value len (N/A) */
                                NULL,               /* list start offset (N/A) */
                                NULL,               /* max list count (N/A) */
                                NULL,               /* filter (N/A) */
                                NULL,               /* format (N/A) */
                                &supportedFeatures);/* supported features */
        if (!OI_SUCCESS(status)) {
            return status;
        }
        if (client->req.appParam) {
            hdr[1].id = OI_OBEX_HDR_APPLICATION_PARAMS;
            hdr[1].val.applicationParams.data = client->req.appParam;
            hdr[1].val.applicationParams.len = client->req.appParamLen;
            hdrCnt ++;
        }
    }

    hdrList.list = &hdr;
    hdrList.count = hdrCnt;
    /*
     * We allow SRM for OPP
     */
    connOpts.enableSRM = TRUE;

    status = OI_OBEXCLI_Connect(addr,
                                lowerProtocol,
                                &connOpts,
                                authentication,
                                &hdrList,
                                &clientCallbacks,
                                connectionId,
                                &connectPolicy);

    if (OI_SUCCESS(status)) {
        client->id = *connectionId;
        /* Set our initialization flag TRUE - we're initialized. */
        OI_INIT_FLAG_PUT_FLAG(TRUE, PBAP_CLI);
        /*
         * Associate the "client" pointer with the OBEX client handle
         */
        OI_OBEXCLI_SetConnectionContext(*connectionId, (void*)client);
    } else {
        OI_Free(client);
        OI_SLOG_ERROR(status, ("OI_OBEXCLI_Connect failed"));
    }
    return status;

}


/**
 * OI_PBAPClient_Disconnect()
 *
 * This function disconnects from a currently connected PBAP server.
 */
OI_STATUS OI_PBAPClient_Disconnect(OI_PBAP_CONNECTION connectionId)
{
    PBAP_CLIENT *client = PbapClient(connectionId);
    OI_STATUS status;

    OI_TRACE_USER(("OI_PBAPClient_Disconnect(connectionId = %d)", connectionId));

    if (!IS_CLIENT_CONNECTED) {
        OI_DBG_PRINT1(("Not connected"));
        return OI_OBEX_NOT_CONNECTED;
    }

    status = OI_OBEXCLI_Disconnect(client->id, NULL);
    if (!OI_SUCCESS(status)) {
        OI_SLOG_WARNING(status, ("OI_OBEXCLI_Disconnect failed"));
    }
    return status;
}

/**
 * OI_PBAPClient_AuthenticationRsp()
 *
 * This function provides authentication information to the PBAP server.
 */
OI_STATUS OI_PBAPClient_AuthenticationRsp(OI_PBAP_CONNECTION connectionId,
                                          const OI_BYTE *userId,
                                          OI_UINT8 userIdLen,
                                          const OI_CHAR *password)
{
    PBAP_CLIENT *client = PbapClient(connectionId);

    OI_TRACE_USER(("OI_PBAPClient_AuthenticationRsp(connectionId = %d, userId = \"%s\", userIdLen = %d, password = \"%s\")",
                   connectionId, userId, userIdLen, password));

    if (!client) {
        OI_SLOG_ERROR(OI_STATUS_INVALID_PARAMETERS, ("connectionID lookup failed"));
        return OI_STATUS_INVALID_PARAMETERS;
    }

    if (OI_INIT_FLAG_VALUE(PBAP_CLI) && (client->state == CLIENT_STATE_AUTHENTICATING)) {
        return OI_OBEXCLI_Authentication(client->id, userId, userIdLen, password, NULL);
    } else {
        return OI_OBEX_INVALID_OPERATION;
    }
}

/**
 * PbapSetExplicitPathDone()
 *
 * This function builds the appropriate path name to the desired object after
 * all the requisite SETPATHs have successfully been performed. */
static void PbapSetExplicitPathDone(PBAP_CLIENT *client, OI_STATUS status)
{
    if (client->req.cb.setPath)
        client->req.cb.setPath(client->id, status);
}

/**
 * OI_PBAPClient_SetPath()
 *
 * This function sets path on the currently connected PBAP server.
 */
OI_STATUS OI_PBAPClient_SetPath(OI_PBAP_CONNECTION connectionId,
                                       OI_PBAP_REPOSITORY repository,
                                       OI_PBAP_PHONEBOOK phonebook,
                                       OI_PBAP_CLIENT_SETPATH_CB setPathCB)
{
    PBAP_CLIENT *client = PbapClient(connectionId);

    OI_TRACE_USER(("OI_PBAPClient_SetPath(connectionId = %d, repository = %d, phonebook = %d, <setPathCB = %x>)",
                      connectionId, repository, phonebook, setPathCB));

    OI_ARGCHECK(setPathCB != NULL);


    if (!IS_CLIENT_CONNECTED) {
        OI_DBG_PRINT1(("Not connected"));
        return OI_OBEX_NOT_CONNECTED;
    }

    client->req.repo = repository;
    client->req.pb = phonebook;
    client->req.cb.setPath = setPathCB;

    SetPhonebookDir(client, repository, phonebook, PbapSetExplicitPathDone);
    return OI_OK;
}

