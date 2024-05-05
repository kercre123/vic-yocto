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
 */

/**
@file
@internal

functions for converting OBEX headers to text
*/

#define __OI_MODULE__ OI_MODULE_SUPPORT

#include "oi_stddefs.h"
#include "oi_obextext.h"
#include "oi_obexspec.h"
#include "oi_status.h"
#include "oi_assert.h"
#include "oi_varstring.h"


const OI_CHAR* OI_OBEX_PktText(OI_UINT8 code)
{

    switch (OI_OBEX_FINAL(code)) {
        case OI_OBEX_FINAL(OI_OBEX_CMD_CONNECT):                       return "CONNECT";
        case OI_OBEX_FINAL(OI_OBEX_CMD_DISCONNECT):                    return "DISCONNECT";
        case OI_OBEX_FINAL(OI_OBEX_CMD_PUT):                           return "PUT";
        case OI_OBEX_FINAL(OI_OBEX_CMD_GET):                           return "GET";
        case OI_OBEX_FINAL(OI_OBEX_CMD_SET_PATH):                      return "SET_PATH";
        case OI_OBEX_FINAL(OI_OBEX_CMD_ACTION):                        return "ACTION";
        case OI_OBEX_FINAL(OI_OBEX_CMD_SESSION):                       return "SESSION";
        case OI_OBEX_FINAL(OI_OBEX_CMD_ABORT):                         return "ABORT";
        case OI_OBEX_FINAL(OI_OBEX_RSP_CONTINUE):                      return "CONTINUE";
        case OI_OBEX_FINAL(OI_OBEX_RSP_OK):                            return "OK";
        case OI_OBEX_FINAL(OI_OBEX_RSP_CREATED):                       return "CREATED";
        case OI_OBEX_FINAL(OI_OBEX_RSP_ACCEPTED):                      return "ACCEPTED";
        case OI_OBEX_FINAL(OI_OBEX_RSP_NON_AUTHORITATIVE_INFORMATION): return "NON_AUTHORITATIVE_INFORMATION";
        case OI_OBEX_FINAL(OI_OBEX_RSP_NO_CONTENT):                    return "NO_CONTENT";
        case OI_OBEX_FINAL(OI_OBEX_RSP_RESET_CONTENT):                 return "RESET_CONTENT";
        case OI_OBEX_FINAL(OI_OBEX_RSP_PARTIAL_CONTENT):               return "PARTIAL_CONTENT";
        case OI_OBEX_FINAL(OI_OBEX_RSP_MULTIPLE_CHOICES):              return "MULTIPLE_CHOICES";
        case OI_OBEX_FINAL(OI_OBEX_RSP_MOVED_PERMANENTLY):             return "MOVED_PERMANENTLY";
        case OI_OBEX_FINAL(OI_OBEX_RSP_MOVED_TEMPORARILY):             return "MOVED_TEMPORARILY";
        case OI_OBEX_FINAL(OI_OBEX_RSP_SEE_OTHER):                     return "SEE_OTHER";
        case OI_OBEX_FINAL(OI_OBEX_RSP_NOT_MODIFIED):                  return "NOT_MODIFIED";
        case OI_OBEX_FINAL(OI_OBEX_RSP_USE_PROXY):                     return "USE_PROXY";
        case OI_OBEX_FINAL(OI_OBEX_RSP_BAD_REQUEST):                   return "BAD_REQUEST";
        case OI_OBEX_FINAL(OI_OBEX_RSP_UNAUTHORIZED):                  return "UNAUTHORIZED";
        case OI_OBEX_FINAL(OI_OBEX_RSP_PAYMENT_REQUIRED):              return "PAYMENT_REQUIRED";
        case OI_OBEX_FINAL(OI_OBEX_RSP_FORBIDDEN):                     return "FORBIDDEN";
        case OI_OBEX_FINAL(OI_OBEX_RSP_NOT_FOUND):                     return "NOT_FOUND";
        case OI_OBEX_FINAL(OI_OBEX_RSP_METHOD_NOT_ALLOWED):            return "METHOD_NOT_ALLOWED";
        case OI_OBEX_FINAL(OI_OBEX_RSP_NOT_ACCEPTABLE):                return "NOT_ACCEPTABLE";
        case OI_OBEX_FINAL(OI_OBEX_RSP_PROXY_AUTHENTICATION_REQUIRED): return "PROXY_AUTHENTICATION_REQUIRED";
        case OI_OBEX_FINAL(OI_OBEX_RSP_REQUEST_TIME_OUT):              return "REQUEST_TIME_OUT";
        case OI_OBEX_FINAL(OI_OBEX_RSP_CONFLICT):                      return "CONFLICT";
        case OI_OBEX_FINAL(OI_OBEX_RSP_GONE):                          return "GONE";
        case OI_OBEX_FINAL(OI_OBEX_RSP_LENGTH_REQUIRED):               return "LENGTH_REQUIRED";
        case OI_OBEX_FINAL(OI_OBEX_RSP_PRECONDITION_FAILED):           return "PRECONDITION_FAILED";
        case OI_OBEX_FINAL(OI_OBEX_RSP_REQUESTED_ENTITY_TOO_LARGE):    return "REQUESTED_ENTITY_TOO_LARGE";
        case OI_OBEX_FINAL(OI_OBEX_RSP_REQUEST_URL_TOO_LARGE):         return "REQUEST_URL_TOO_LARGE";
        case OI_OBEX_FINAL(OI_OBEX_RSP_UNSUPPORTED_MEDIA_TYPE):        return "UNSUPPORTED_MEDIA_TYPE";
        case OI_OBEX_FINAL(OI_OBEX_RSP_INTERNAL_SERVER_ERROR):         return "INTERNAL_SERVER_ERROR";
        case OI_OBEX_FINAL(OI_OBEX_RSP_NOT_IMPLEMENTED):               return "NOT_IMPLEMENTED";
        case OI_OBEX_FINAL(OI_OBEX_RSP_BAD_GATEWAY):                   return "BAD_GATEWAY";
        case OI_OBEX_FINAL(OI_OBEX_RSP_SERVICE_UNAVAILABLE):           return "SERVICE_UNAVAILABLE";
        case OI_OBEX_FINAL(OI_OBEX_RSP_GATEWAY_TIMEOUT):               return "GATEWAY_TIMEOUT";
        case OI_OBEX_FINAL(OI_OBEX_RSP_HTTP_VERSION_NOT_SUPPORTED):    return "HTTP_VERSION_NOT_SUPPORTED";
        case OI_OBEX_FINAL(OI_OBEX_RSP_DATABASE_FULL):                 return "DATABASE_FULL";
        case OI_OBEX_FINAL(OI_OBEX_RSP_DATABASE_LOCKED):               return "DATABASE_LOCKED";
        default:                                            return "**** bad packet ****";
    }

}


static const OI_CHAR* HdrId(OI_UINT8 code)
{
    switch (code) {
       case OI_OBEX_HDR_COUNT:                      return "COUNT";
        case OI_OBEX_HDR_NAME:                       return "NAME";
        case OI_OBEX_HDR_TYPE:                       return "TYPE";
        case OI_OBEX_HDR_LENGTH:                     return "LENGTH";
        case OI_OBEX_HDR_TIME:                       return "TIME";
        case OI_OBEX_HDR_DESCRIPTION:                return "DESCRIPTION";
        case OI_OBEX_HDR_TARGET:                     return "TARGET";
        case OI_OBEX_HDR_HTTP:                       return "HTTP";
        case OI_OBEX_HDR_BODY:                       return "BODY";
        case OI_OBEX_HDR_END_OF_BODY:                return "END_OF_BODY";
        case OI_OBEX_HDR_WHO:                        return "WHO";
        case OI_OBEX_HDR_CONNECTION_ID:              return "CONNECTION_ID";
        case OI_OBEX_HDR_APPLICATION_PARAMS:         return "APPLICATION_PARAMS";
        case OI_OBEX_HDR_AUTHENTICATION_CHALLENGE:   return "AUTHENTICATION_CHALLENGE";
        case OI_OBEX_HDR_AUTHENTICATION_RESPONSE:    return "AUTHENTICATION_RESPONSE";
        case OI_OBEX_HDR_OBJECT_CLASS:               return "OBJECT_CLASS";
        case OI_OBEX_HDR_SESSION_PARAMS:             return "SESSION_PARAMS";
        case OI_OBEX_HDR_ACTION_ID:                  return "ACTION_ID";
        case OI_OBEX_HDR_DEST_NAME:                  return "DEST_NAME";
        case OI_OBEX_HDR_SINGLE_RESPONSE_MODE:       return "SINGLE_RESPONSE_MODE";
        case OI_OBEX_HDR_SINGLE_RESPONSE_PARAMETERS: return "SINGLE_RESPONSE_MODE_PARAMETERS";
        case OI_OBEX_HDR_PERMISSIONS:                return "PERMISSIONS";
        default:                                     return "**** unknown header ****";

    }
}


static void FormatSessionParams(OI_VARSTRING *VStr,
                                OI_UINT8 *data,
                                OI_UINT16 datalen)
{
    while (datalen >= 2) {
        OI_UINT8 tag = data[0];
        OI_UINT8 len = data[1];

        datalen -= 2;
        if (datalen < len) {
            break;
        }
        data += 2;
        switch (tag) {
            case OI_OBEX_SESSION_PARAM_OPCODE:
                OI_FormatStr(VStr, "OPCODE %d ", data[0]);
                break;
            case OI_OBEX_SESSION_PARAM_DEVICE_ADDR:
                OI_FormatStr(VStr, "DEVICE ADDR %@ ", data, len);
                break;
            case OI_OBEX_SESSION_PARAM_NONCE:
                OI_FormatStr(VStr, "NONCE %@ ", data, len);
                break;
            case OI_OBEX_SESSION_PARAM_ID:
                OI_FormatStr(VStr, "ID %@ ", data, len);
                break;
            case OI_OBEX_SESSION_PARAM_NEXT_SEQ_NUM:
                OI_FormatStr(VStr, "NEXT SEQ NUM %d ", data[0]);
                break;
            case OI_OBEX_SESSION_PARAM_TIMEOUT:
                OI_FormatStr(VStr, "TIMEOUT %u", data[0] | (data[1] << 8) | (data[2] << 16) || (data[3] << 24));
                break;
            default:
                break;
        }
        datalen -= len;
        data += len;
    }
}

static void FormatApplicationParams(OI_VARSTRING *VStr,
                                OI_UINT8 *data,
                                OI_UINT16 datalen)
{
    while (datalen >= 2) {
        OI_UINT8    tag     = data[0];
        OI_UINT8    tagLen  = data[1];

        data += 2;
        datalen -= 2;
        if (datalen < tagLen) {
            break;
        }

        OI_FormatStr(VStr, " tag %d, len %d;", tag, tagLen);
        data += tagLen;
        datalen -= tagLen;
    }
}

void OI_OBEX_HeaderTxt(OI_VARSTRING *VStr,
                       OI_OBEX_HEADER *hdr)
{
    const OI_CHAR *id = HdrId(hdr->id);

    switch (OI_OBEX_HDR_KIND(hdr->id)) {
        case OI_OBEX_HDR_ID_UNICODE:
            if (hdr->val.unicode.len == 0) {
                OI_FormatStr(VStr, "%s=<empty>", id);
            } else {
                OI_FormatStr(VStr, "%s=\"%S\"", id, hdr->val.unicode.str);
            }
            break;
        case OI_OBEX_HDR_ID_BYTESEQ:
            switch (hdr->id) {
                case OI_OBEX_HDR_TYPE:
                    OI_FormatStr(VStr, "%s=%?s", id, hdr->val.byteseq.data, hdr->val.byteseq.len);
                    break;
                case OI_OBEX_HDR_TARGET:
                    OI_FormatStr(VStr, "%s %@", id, hdr->val.byteseq.data, hdr->val.byteseq.len);
                    break;
                case OI_OBEX_HDR_SESSION_PARAMS:
                    OI_FormatStr(VStr, "%s ", id);
                    FormatSessionParams(VStr, hdr->val.byteseq.data, hdr->val.byteseq.len);
                    break;
                case OI_OBEX_HDR_APPLICATION_PARAMS:
                    OI_FormatStr(VStr, "%s  size=%d;", id, hdr->val.byteseq.len);
                    FormatApplicationParams(VStr, hdr->val.byteseq.data, hdr->val.byteseq.len);
                    break;
                default:
                    OI_FormatStr(VStr, "%s size=%d", id, hdr->val.byteseq.len);
                    break;
            }
            break;
        case OI_OBEX_HDR_ID_UINT8:
            OI_FormatStr(VStr, "%s=%d", id, hdr->val.uInt8);
            break;
        case OI_OBEX_HDR_ID_UINT32:
            if (hdr->id == OI_OBEX_HDR_PERMISSIONS) {
                OI_FormatStr(VStr, "%s=%8x (%32b)", id, hdr->val.uInt32, hdr->val.uInt32);
            } else {
                OI_FormatStr(VStr, "%s=%ld", id, hdr->val.uInt32);
            }
            break;
        default:
            OI_FormatStr(VStr, "BAD HEADER!");
    }
}
