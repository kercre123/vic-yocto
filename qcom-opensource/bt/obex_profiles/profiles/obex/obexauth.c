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

#define __OI_MODULE__ OI_MODULE_OBEX_SRV

#include "oi_debug.h"
#include "oi_argcheck.h"
#include "oi_assert.h"
#include "oi_time.h"
#include "oi_memmgr.h"
#include "oi_obexspec.h"
#include "oi_utils.h"
#include "oi_obexauth.h"
#include "md5_hash.h"



void OI_OBEXAUTH_Reset(OBEX_COMMON *common)
{
    OI_DBGTRACE(("OI_OBEXAUTH_Reset\n"));

    OI_MemZero(common->authentication, sizeof(OBEX_AUTHENTICATION));
}



OI_STATUS OI_OBEXAUTH_SaveAuthInfo(OBEX_COMMON *common,
                                   const OI_BYTE *userId,
                                   OI_UINT8 userIdLen,
                                   const OI_CHAR *password)
{
    OI_UINT len = OI_StrLen(password);

    OI_DBGTRACE(("OI_OBEXAUTH_SaveAuthInfo(userId=%s,password=%s)\n", userId, userIdLen, password));

    /*
     * Truncate to maximum password length. This means that authentication
     * with really long passwords will not work.
     */
    if (len > OI_OBEX_MAX_PASSWORD_LEN) {
       return OI_OBEX_PASSWORD_TOO_LONG;
    }
    OI_MemCopy(common->authentication->password, password, len);
    common->authentication->passwordLen = len;

    if(userIdLen > OI_OBEX_MAX_USERID_LEN) {
        OI_DBGPRINT2(("Incorrect user Id length %d", userIdLen));
        return OI_OBEX_USERID_TOO_LONG;
    }

    if ((userId != NULL) && (userIdLen > 0)) {
        OI_MemCopy(common->authentication->userId, userId, userIdLen);
        common->authentication->userIdLen = userIdLen;
    } else {
        common->authentication->userIdLen = 0;
    }

    return OI_OK;
}



OI_STATUS OI_OBEXAUTH_SaveChallenge(OBEX_COMMON *common,
                                    OI_OBEX_HEADER *challenge)
{
    OBEX_AUTHENTICATION *auth = common->authentication;
    OI_STATUS status = OI_OK;
    OI_UINT8 options = 0;
    OI_UINT i = 0;

    OI_DBGTRACE(("OI_OBEXAUTH_SaveChallenge\n"));

    OI_ASSERT(auth);
    OI_ASSERT(challenge->id == OI_OBEX_HDR_AUTHENTICATION_CHALLENGE);

    /*
     * Locate the digest challenge nonce and save it.
     */
    while (i < challenge->val.byteseq.len) {
        OI_UINT8 tag = challenge->val.byteseq.data[i++];
        OI_UINT8 len = challenge->val.byteseq.data[i++];
        switch(tag) {
            case OI_OBEX_CHALLENGE_DIGEST_REALM_TAG:
                auth->realm.len = len - 1;
                auth->realm.charSet = challenge->val.byteseq.data[i];
                OI_MemCopy(auth->realm.realm, &challenge->val.byteseq.data[i + 1], auth->realm.len);
                OI_DBGPRINT2(("OBEX receive realm charSet= %d %s", auth->realm.charSet, auth->realm.realm, auth->realm.len));
                break;
            case OI_OBEX_CHALLENGE_DIGEST_OPTIONS_TAG:
                if (len != 1) {
                    status = OI_OBEX_INVALID_AUTH_DIGEST;
                } else {
                    options = challenge->val.byteseq.data[i];
                    OI_DBGPRINT2(("OBEX receive challenge options = %1x", options));
                }
                break;
            case OI_OBEX_CHALLENGE_DIGEST_NONCE_TAG:
                if (len != OI_OBEX_AUTH_DIGEST_LEN) {
                    status = OI_OBEX_INVALID_AUTH_DIGEST;
                } else {
                    OI_MemCopy(auth->challengeDigest, &challenge->val.byteseq.data[i], OI_OBEX_AUTH_DIGEST_LEN);
                }
                break;
            default:
                OI_DBGPRINT2(("Unexpected tag in challenge %d", tag));
                break;
        }
        i += len;
    }
    /*
     * Set options.
     */
    auth->userIdRequested = (options & OI_OBEX_AUTH_USERID_OPTION) != 0;
    common->readOnly = (options & OI_OBEX_AUTH_READONLY_OPTION) != 0;

    return status;
}


OI_STATUS OI_OBEXAUTH_SaveResponse(OBEX_COMMON *common,
                                   OI_OBEX_HEADER *response)
{
    OBEX_AUTHENTICATION *auth = common->authentication;
    OI_STATUS status = OI_OK;
    OI_UINT i = 0;

    OI_DBGTRACE(("OI_OBEXAUTH_SaveResponse\n"));

    OI_ASSERT(auth);
    OI_ASSERT(response->id == OI_OBEX_HDR_AUTHENTICATION_RESPONSE);

    OI_MemZero(auth->userIdRemote, OI_OBEX_MAX_USERID_LEN);
    auth->userIdRemoteLen = 0;


    while (i < response->val.byteseq.len) {
        OI_UINT8 tag = response->val.byteseq.data[i++];
        OI_UINT8 len = response->val.byteseq.data[i++];
        switch (tag) {
            case OI_OBEX_RESPONSE_DIGEST_REQUEST_TAG:
                if (len != OI_OBEX_AUTH_DIGEST_LEN) {
                    status = OI_OBEX_INVALID_AUTH_DIGEST;
                } else {
                    OI_MemCopy(auth->responseDigest, &response->val.byteseq.data[i], OI_OBEX_AUTH_DIGEST_LEN);
                }
                break;
            case OI_OBEX_RESPONSE_DIGEST_USERID_TAG:
                if (len > 0) {
                    if (len > OI_OBEX_MAX_USERID_LEN) {
                        status = OI_OBEX_INVALID_AUTH_DIGEST;
                    } else {
                        OI_MemCopy(auth->userIdRemote, &response->val.byteseq.data[i], len);
                        OI_DBGPRINT2(("Obex authentication response user id = %s", auth->userIdRemote));
                        auth->userIdRemoteLen = len;
                    }
                }
                break;
        }
        i += len;
    }

    return status;
}



/**
 * Compose the response to an OBEX authentication challenge.
 */

void OI_OBEXAUTH_ComposeResponse(OBEX_COMMON *common,
                                 OI_OBEX_HEADER *response)
{
    OI_UINT16 len = 0;
    MD5_CONTEXT md5Context;
    OBEX_AUTHENTICATION *auth = common->authentication;

    OI_DBGTRACE(("OI_OBEXAUTH_ComposeResponse\n"));

    OI_ASSERT(auth);

    /*
     * Compose a digest response to the authentication challenge by appending
     * the password to the nonce received in the challenge and computing the MD5
     * hash of the resultant string.
     */
    OI_MD5_Init(&md5Context);
    /*
     * Hash in the challenge digest (nonce), a separator (colon per OBEX spec),
     * and the password
     */
    OI_MD5_Update(&md5Context, common->authentication->challengeDigest, OI_OBEX_AUTH_DIGEST_LEN);
    OI_MD5_Update(&md5Context, (OI_BYTE*)":", 1);
    OI_MD5_Update(&md5Context, common->authentication->password, common->authentication->passwordLen);
    /*
     * Generate the MD5 result and compose the response digest.
     */
    auth->responseHeader[len++] = OI_OBEX_RESPONSE_DIGEST_REQUEST_TAG;
    auth->responseHeader[len++] = OI_OBEX_AUTH_DIGEST_LEN;
    OI_MD5_Final(&auth->responseHeader[len], &md5Context);
    len += OI_OBEX_AUTH_DIGEST_LEN;
    /*
     * Append the user id if we have one.
     */
    if (auth->userIdLen > 0) {
        auth->responseHeader[len++] = OI_OBEX_RESPONSE_DIGEST_USERID_TAG;
        auth->responseHeader[len++] = auth->userIdLen;
        OI_MemCopy(&auth->responseHeader[len], auth->userId, auth->userIdLen);
        len += auth->userIdLen;
    }
    /*
     * Initialize the authentication response header.
     */
    response->id = OI_OBEX_HDR_AUTHENTICATION_RESPONSE;
    response->val.authenticationResponse.data = auth->responseHeader;
    response->val.authenticationResponse.len = len;
}



/**
 * Generate an OBEX authentication challenge.
 */

void OI_OBEXAUTH_ComposeChallenge(const OI_CHAR* privateKey,
                                  const OI_OBEX_REALM* realm,
                                  OBEX_COMMON *common,
                                  OI_OBEX_HEADER *challenge)
{
    OBEX_AUTHENTICATION *auth = common->authentication;
    OI_UINT8 options = 0;
    OI_UINT16 len = 0;
    OI_UINT16 keyLen;
    MD5_CONTEXT md5Context;
    OI_TIME timestamp;

    OI_DBGTRACE(("OI_OBEXAUTH_ComposeChallenge(privateKey=%s)\n", privateKey));

    OI_ASSERT(auth);
    OI_ASSERT(privateKey != NULL);

    if((auth == NULL) || (privateKey == NULL)) {
        return;
    }

    keyLen = OI_StrLen(privateKey);

    /*
     * Compose a digest challenge by appending a private key t(provided as a
     * configuration parameter)o a time stamp and computing the MD5 hash of the
     * resultant string.
     */
    OI_MD5_Init(&md5Context);
    OI_Time_Now(&timestamp);
    /*
     * Hash in the timestamp and the private key. Private key is a NULL
     * terminated string.
     */
    OI_MD5_Update(&md5Context, (OI_BYTE*) &timestamp, sizeof(timestamp));
    OI_MD5_Update(&md5Context, (OI_BYTE*) privateKey, keyLen);
    /*
     * Generate the MD5 result and compose the challenge digest.
     */
    auth->challengeHeader[len++] = OI_OBEX_CHALLENGE_DIGEST_NONCE_TAG;
    auth->challengeHeader[len++] = OI_OBEX_AUTH_DIGEST_LEN;
    OI_MD5_Final(&auth->challengeHeader[len], &md5Context);
    len += OI_OBEX_AUTH_DIGEST_LEN;
    /*
     * Send options if needed.
     */
    if (common->readOnly) {
        options |= OI_OBEX_AUTH_READONLY_OPTION;
    }
    if (auth->userIdRequired) {
        options |= OI_OBEX_AUTH_USERID_OPTION;
    }
    if (options) {
        OI_DBGPRINT2(("OBEX compose challenge options = %1x", options));
        auth->challengeHeader[len++] = OI_OBEX_CHALLENGE_DIGEST_OPTIONS_TAG;
        auth->challengeHeader[len++] = 1;
        auth->challengeHeader[len++] = options;
    }
    if (realm) {
        OI_DBGPRINT2(("OBEX compose challenge realm charSet:%d %s", realm->charSet, realm->realm, realm->len));
        auth->challengeHeader[len++] = OI_OBEX_CHALLENGE_DIGEST_REALM_TAG;
        auth->challengeHeader[len++] = 1 + realm->len;
        auth->challengeHeader[len++] = realm->charSet;
        OI_MemCopy(&auth->challengeHeader[len], realm->realm, realm->len);
        len += realm->len;
    }
    /*
     * Initialize the authentication challenge header.
     */
    challenge->id = OI_OBEX_HDR_AUTHENTICATION_CHALLENGE;
    challenge->val.authenticationChallenge.data = auth->challengeHeader;
    challenge->val.authenticationChallenge.len = len;
}


OI_STATUS OI_OBEXAUTH_Authenticate(OBEX_COMMON *common)
{
    OBEX_AUTHENTICATION *auth = common->authentication;
    MD5_CONTEXT md5Context;
    OI_BYTE digest[OI_OBEX_AUTH_DIGEST_LEN];

    OI_DBGTRACE(("OI_OBEXAUTH_Authenticate\n"));

    OI_ASSERT(auth);

    OI_DBGPRINT(("Authentication user id: %s password: %s\n", auth->userId,
                 auth->userIdLen, auth->password, auth->passwordLen));

    /*
     * Compose the expected response to the challenge.
     */
    OI_MD5_Init(&md5Context);
    /*
     * Hash in the digest from the challenge we sent earlier followed by the
     * separator (colon) and the passowrd.
     */
    OI_MD5_Update(&md5Context, &auth->challengeHeader[2], OI_OBEX_AUTH_DIGEST_LEN);
    OI_MD5_Update(&md5Context, (OI_BYTE*)":", 1);
    OI_MD5_Update(&md5Context, auth->password, auth->passwordLen);
    /*
     * Generate the MD5 result.
     */
    OI_MD5_Final(digest, &md5Context);
    /*
     * Compare the computed digest with the response digest
     */
    if (OI_MemCmp(digest, auth->responseDigest, OI_OBEX_AUTH_DIGEST_LEN) == 0) {
        OI_DBGPRINT(("Authentication successful\n"));
        return OI_OK;
    } else {
        OI_DBGPRINT(("Authentication failed\n"));
        return OI_OBEX_UNAUTHORIZED;
    }
}

