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

#define __OI_MODULE__ OI_MODULE_DATAELEM

#include "oi_status.h"
#include "oi_dataelem.h"
#include "oi_debug.h"
#include "oi_assert.h"
#include "oi_varstring.h"
#include "oi_bt_assigned_nos.h"



#define STR_0x            "0x"
#define STR_LT            "<"
#define STR_LT_SLASH      "</"
#define STR_GT            ">"
#define STR_SLASH_GT_NL   "/>\n"


#define STR_NEWLINE       "\n"


#define INDENT_INCREMENT 4
#define MAX_BYTES_ON_LINE 32


#define NewlineCat(t)    OI_VStrCat((t), STR_NEWLINE)


#ifdef OI_DEBUG

/**
 * This function puts a symbolic name as an attribute in the tag for element values that map
 * to symbolic names.
 */
static void ShowName(OI_VARSTRING *VStr,
                     OI_DATAELEM const *Element)
{
    OI_CHAR *name = NULL;

    switch (Element->ElemType) {
        case OI_DATAELEM_UUID:
            if (Element->Size <= sizeof(OI_UUID32)) {
                name = OI_UUIDText(Element->Value.ShortUUID);
            }
            break;
        default:
            return;
    }

    if (name != NULL) {
        OI_VStrCat(VStr, " id=\"");
        OI_VStrCat(VStr, name);
        OI_VStrCat(VStr, "\"");
    }
}

void OI_DataElement_XML(OI_VARSTRING *VStr,
                        OI_DATAELEM const *Element,
                        OI_UINT8 Indent)
{
    OI_UINT8 i;
    OI_UINT16 tagStart;
    OI_UINT16 tagEnd;

    if (Element->ElemType == OI_DATAELEM_REF) {
        Element = Element->Value.ElemRef;
    }

    OI_VSpaceCat(VStr, Indent);
    OI_VStrCat(VStr, STR_LT);
    tagStart = VStr->Len;

    switch (Element->ElemType) {
        case OI_DATAELEM_BOOL:
            OI_VStrCat(VStr, (Element->Value.Boolean) ? "true" : "false");
            OI_VStrCat(VStr, STR_SLASH_GT_NL);
            return;
        case OI_DATAELEM_NULL:
            OI_VStrCat(VStr, "null");
            OI_VStrCat(VStr, STR_SLASH_GT_NL);
            return;
        case OI_DATAELEM_UINT:
            OI_VStrCat(VStr, "uint");
            OI_VDecCat(VStr, Element->Size * 8);
            break;
        case OI_DATAELEM_SINT:
            OI_VStrCat(VStr, "sint");
            OI_VDecCat(VStr, Element->Size * 8);
            break;
        case OI_DATAELEM_UUID:
            OI_VStrCat(VStr, "uuid");
            OI_VDecCat(VStr, Element->Size * 8);
            break;
        case OI_DATAELEM_TEXT:
            OI_VStrCat(VStr, "text");
            break;
        case OI_DATAELEM_URL:
            OI_VStrCat(VStr, "url");
            break;
        case OI_DATAELEM_SEQ:
            OI_VStrCat(VStr, "seq");
            break;
        case OI_DATAELEM_ALT:
            OI_VStrCat(VStr, "alt");
            break;
    }

    tagEnd = VStr->Len;
    ShowName(VStr, Element);
    OI_VStrCat(VStr, STR_GT);

    switch (Element->ElemType) {
        case OI_DATAELEM_UINT:
        case OI_DATAELEM_SINT:
            if (Element->Size > sizeof(OI_UINT32)) {
                OI_VStrCat(VStr, STR_0x);
                if (Element->Size == sizeof(OI_UINT64)) {
                    OI_VHexCat(VStr, Element->Value.UInt64->I1, 2 * sizeof(OI_UINT32));
                    OI_VHexCat(VStr, Element->Value.UInt64->I2, 2 * sizeof(OI_UINT32));
                } else {
                    OI_VHexCat(VStr, Element->Value.UInt128->I1, 2 * sizeof(OI_UINT32));
                    OI_VHexCat(VStr, Element->Value.UInt128->I2, 2 * sizeof(OI_UINT32));
                    OI_VHexCat(VStr, Element->Value.UInt128->I3, 2 * sizeof(OI_UINT32));
                    OI_VHexCat(VStr, Element->Value.UInt128->I4, 2 * sizeof(OI_UINT32));
                }
            } else {
                if (Element->ElemType == OI_DATAELEM_UINT) {
                    OI_VStrCat(VStr, STR_0x);
                    OI_VHexCat(VStr, Element->Value.UInt, 2 * Element->Size);
                } else {
                    OI_VDecCat(VStr, Element->Value.SInt);
                }
            }
            break;
        case OI_DATAELEM_UUID:
            OI_VStrCat(VStr, STR_0x);
            if (Element->Size <= sizeof(OI_UUID32)) {
                OI_VHexCat(VStr, Element->Value.ShortUUID, 2 * Element->Size);
            } else {
                OI_VHexCat(VStr, Element->Value.LongUUID->ms32bits, 2 * sizeof(OI_UINT32));
                OI_VStrCat(VStr, ",");
                for (i = 0; i < sizeof(Element->Value.LongUUID->base); ++i) {
                    OI_VHexCat(VStr, Element->Value.LongUUID->base[i], 2 * sizeof(OI_UINT8));
                    if ((i == 1) || (i == 3) || (i == 5)) {
                        OI_VStrCat(VStr, ",");
                    }
                }
            }
            break;
        case OI_DATAELEM_SEQ:
        case OI_DATAELEM_ALT:
            NewlineCat(VStr);
            Indent += INDENT_INCREMENT;
            for (i = 0; i < Element->Size; ++i) {
                OI_DataElement_XML(VStr, &Element->Value.ElemSeq[i], Indent);
            }
            Indent -= INDENT_INCREMENT;
            OI_VSpaceCat(VStr, Indent);
            break;
        case OI_DATAELEM_URL:
        case OI_DATAELEM_TEXT:
            OI_VStrnCat(VStr, Element->Value.Text, Element->Size);
            break;
        default:
            OI_VStrCat(VStr, "!!!Corrupt Data Element!!!");
    }

    OI_VStrCat(VStr, STR_LT_SLASH);
    OI_VStrnCat(VStr, &VStr->Buffer[tagStart], (OI_UINT16) (tagEnd - tagStart));
    OI_VStrCat(VStr, STR_GT);
    NewlineCat(VStr);
}


void OI_DataElement_Print(OI_DATAELEM *Element)
{
    OI_VARSTRING vStr;
    OI_CHAR      *str;

    OI_VStrAlloc(&vStr, 100);
    OI_DataElement_XML(&vStr, Element, 0);
    str = OI_VStrGetString(&vStr);
    if (str) {
        OI_DBGPRINTSTR((str));
    }
    OI_VStrFree(&vStr);
}

#else /* OI_DEBUG */

void OI_DataElement_Print(OI_DATAELEM *Element)
{
}

void OI_DataElement_XML(OI_VARSTRING *VStr,
                        OI_DATAELEM const *Element,
                        OI_UINT8 Indent)
{
    OI_VStrCat(VStr, "<data element/>");
}

#endif /* OI_DEBUG */
