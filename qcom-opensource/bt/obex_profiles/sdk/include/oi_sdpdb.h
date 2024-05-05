#ifndef _OI_SDPDB_H
#define _OI_SDPDB_H

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
 *
 *  This file provides the API for management of the SDP database.
 *
 *  For more information see the @ref SDP_docpage section of the
 *  BLUEmagic SDK documentation.
 */

#include "oi_status.h"
#include "oi_bt_spec.h"
#include "oi_dataelem.h"

/** \addtogroup SvcDisc Service Discovery APIs */
/**@{*/

#ifdef __cplusplus
extern "C" {
#endif


/**
 * To support multiple languages, text attribute IDs are modified by a language
 * ID. The language ID is defined by an attribute in an SDP service record. The
 * SDP specification also defines a language attribute ID that can always be used to
 * select the default language. See Volume 3 the Bluetooth specification
 * for information on the language attribute ID.
 */
#define OI_SDP_DEFAULT_LANGUAGE_ID(id) (id | OI_ATTRID_DEFAULT_LanguageBaseOffset)


/**
 * Attribute IDs are 16-bit integers.
 */
typedef OI_UINT16 OI_SDP_ATTRIBUTE_ID;

/**
 * A service record attribute is represented as a structure consisting of
 * a 16-bit attribute ID and an attribute value represented by a union of type
 * @ref OI_DATAELEM. A variety of macros defined in oi_dataelem.h can be used to
 * statically initialize the data element in an OI_SDPDB_ATTRIBUTE.
 * (See oi_bt_assigned_nos.h for more information on attribute IDs.)
 */
typedef struct {
    OI_SDP_ATTRIBUTE_ID Id;  /**< 16-bit attribute ID */
    OI_DATAELEM Element;     /**< Data element value for the attribute value */
} OI_SDPDB_ATTRIBUTE;


/**
   This structure defines the internationalized strings to use
   for an SDP service record.
 *
   @param  attrs  Attribute pairs (uuid, string)
   @param  num    Number of attributes
 */
typedef struct {
    const OI_SDPDB_ATTRIBUTE *attrs;
    OI_UINT16 num;
} OI_SDP_STRINGS;


/**
   This structure defines the attributes of a service in a
   service record. An SDP service record is an array of
   attribute ID/value pairs.
 */
typedef struct {
    OI_SDPDB_ATTRIBUTE const *Attributes; /**< A service record is initialized with an attribute list.
                                               This will normally be statically allocated attributes
                                               that do not change during the lifetime of a service. */

    OI_UINT16 NumAttributes;              /**< Number of attributes in the ConstAttributes array */

    OI_SDPDB_ATTRIBUTE const *Strings;    /**< Name and description strings */
    OI_UINT16 NumStrings;                 /**< Number of strings */

} OI_SDPDB_SERVICE_RECORD;


/**
   This structure provides a means to associate an SDP service record
   with the corresponding profile.
 */
typedef struct {
    OI_UINT32   handle;             /* Service record handle */
    OI_UUID16   serviceClassUUID16; /* Bluetooth service class UUID */
    OI_UUID16   profileUUID16;      /* Bluetooth profile UUID */
}  OI_SDP_SUMMARY_REC;

/**
 * This enumeration lists the different XML output formats available
 * when using OI_SDPDB_GetRecordAsXML() and OI_SDPDB_GetAttributeAsXML().
 * Format information in sdp_xml_formats.c depends on the order of
 * this list.
 */

typedef enum {
    OI_SDPDB_XML_FORMAT_OPENBLUE = 0,
    OI_SDPDB_XML_FORMAT_BZ,
    /* Add new formats here */
    OI_SDPDB_XML_NUM_FORMATS  /* Must be last in the list */
} OI_SDPDB_XML_FORMAT;

/**
 * A function of this type is called whenever a record in the SDP
 * database is added or modified. The supplied handle may be used to
 * get information from the SDP database.
 *
 * @param handle  Specifies the service record handle
 *
 */

typedef void (*OI_SDPDB_CHANGED_RECORD)(OI_UINT32 handle);

/**
 * A function of this type is called whenever a record in the SDP
 * database is removed. The handle is still valid when this function
 * is called.
 *
 * @param handle  Specifies the service record handle
 *
 */

typedef void (*OI_SDPDB_REMOVING_RECORD)(OI_UINT32 handle);


/**
 * This function requests that a service record be added to the SDP database.
 * Once added, the service will immediately become visible to the SDP clients.
 *
 * @param serviceRecord       Defines the attributes of a service; an SDP service record is an array of attribute ID/value pairs.
 *
 * @param serviceRecordHandle Pointer to the buffer where the service record handle will be written once
 *                            the service record is added. The service record handle is a locally unique handle
 *                            that can be used to reference a service record when accessing attributes within the
 *                            service record. This parameter can be NULL if the caller does not need to store the
 *                            service record handle.
 *
 * @return                    OI_OK if the service record is added; error status otherwise, including if the
 *                            SDP database is full
 */
OI_STATUS OI_SDPDB_AddServiceRecord(OI_SDPDB_SERVICE_RECORD const *serviceRecord,
                                    OI_UINT32 *serviceRecordHandle);


/**
 * This function removes a service record from the SDP database. Once removed, the service will
 * no longer be visible to the SDP clients.
 *
 * @param serviceRecordHandle      Service record handle of the service to be removed
 *
 * @return                         OI_OK if the service record was removed; error status otherwise, including
 *                                 if the service record handle is invalid
 */
OI_STATUS OI_SDPDB_RemoveServiceRecord(OI_UINT32 serviceRecordHandle);


/**
 * An application can use the Service Availability attribute to advertise in
 * the service record if a service is available. This function sets
 * the value of the Service Availability attribute for a given service. Other
 * than 0 (meaning not available) and 255 (meaning fully available), the
 * interpretation of intermediate values is profile/application specific.
 *
 * @param serviceRecordHandle   Identifies the service record to which the
 *                              attribute is to be added.
 *
 * @param availability          0 means not available, 255 means 100% available,
 *                              128 means 50% available etc.
 *
 * @return                      OI_OK if the attributes was set; error status
 *                              if the service record handle is invalid.
 */
OI_STATUS OI_SDPDB_SetServiceAvailability(OI_UINT32 serviceRecordHandle,
                                          OI_UINT8 availability);

/**
 * This function dynamically sets the attributes on a service record. Memory allocated for the
 * attributes must not be freed by the caller until the attribute list is
 * removed from the SDP database by calling OI_SDPDB_RemoveAttributeList().
 *
 * @param serviceRecordHandle   Identifies the service record to which the
 *                              attribute is to be added
 *
 * @param attributeList         Pointer to the list of attributes to add
 *
 * @param numAttributes         Number of attributes in list
 *
 * @return                      OI_OK if the attributes were added; error status
 *                              otherwise, including if the service record
 *                              handle is invalid
 */
OI_STATUS OI_SDPDB_SetAttributeList(OI_UINT32 serviceRecordHandle,
                                    OI_SDPDB_ATTRIBUTE const *attributeList,
                                    OI_UINT16 numAttributes);

/**
 * This function removes the attribute list previously set by OI_SDPDB_SetAttributeList().
 *
 * @param serviceRecordHandle   Identifies the service record that contains the
 *                              attribute list to be removed.
 *
 * @param[out] attributeList    Returns a pointer to the attribute list that was removed;
 *                              this is the same pointer that was passed in to
 *                              OI_SDPBP_SetAttributeList(). The caller can now
 *                              free any memory allocated for this attribute
 *                              list. The parameter may be NULL if the caller does not need this
 *                              pointer.
 *
 * @param[out] numAttributes    Returns the length of the attribute list that was
 *                              removed. This parameter can be NULL if the caller does
 *                              not need this value.
 */
OI_STATUS OI_SDPDB_RemoveAttributeList(OI_UINT32 serviceRecordHandle,
                                       OI_SDPDB_ATTRIBUTE **attributeList,
                                       OI_UINT16 *numAttributes);

/**
 * This function copies an attribute list. For example, this may be used in combination with
 * OI_SDPDB_RemoveAttributeList to copy the returned list into a new list
 * containing additional attributes. OI_SDPDB_FreeAttributeListElements
 * must be called when the destination list is no longer needed to release
 * the memory used.
 *
 * @param toList         Destination to which the source attributes are copied; this
 *                       must point to enough space to include numAttributes attributes.
 * @param fromList       Specifies the source attribute list.
 * @param numAttributes  Specifies the number of attributes to copy from the source list into
 *                       the destination list.
 */
OI_STATUS OI_SDPDB_CloneAttributeList(OI_SDPDB_ATTRIBUTE *toList,
                                      OI_SDPDB_ATTRIBUTE *fromList,
                                      OI_UINT16 numAttributes);

/**
 * This function frees memory allocated for data elements in an attribute list. Memory for the
 * attribute list itself must be freed by the caller.
 */
void OI_SDPDB_FreeAttributeListElements(OI_SDPDB_ATTRIBUTE *attributeList,
                                        OI_UINT16 numAttributes);

/**
 * This function explicitly sets the browse group list. By default, each time a service record is
 * created, SDPDB automatically creates a browse group list containing the public
 * browse group. Applications can override this default with their own browse group
 * list using this API. This API can also be used to completely remove
 * a browse group list from the service record.
 *
 * @param serviceRecordHandle   Identifies the service record to which the
 *                              attribute is to be added
 *
 * @param groupList             Pointer to the browse group list;
 *                              NULL pointer will remove the browse group list entirely
 */

OI_STATUS OI_SDPDB_SetBrowseGroupList(OI_UINT32 serviceRecordHandle,
                                      const OI_DATAELEM *groupList);


/**
 * This function returns a list of SDP record handles available in the local SDP database.
 *
 * @param handleList          Buffer into which the service record handles are written.
 * @param maxNumHandles       Maximum number of handles that could be returned (@handleList buffer size).
 * @param numHandlesReturned  Number of SDP record handles written into @handleList.
 * @param totalNumHandles     Total number of valid record handles in the local SDP database; may be larger than
 *                            @numHandlesReturned if @handleList is not large enough to hold all the service
 *                            record handles.
 * @return                    OI_OK if record handles were successfully retrieved; error status otherwise.
 *
 */
OI_STATUS OI_SDPDB_GetRecordHandles(OI_UINT32 *handleList,
                                    OI_UINT32 maxNumHandles,
                                    OI_UINT32 *numHandlesReturned,
                                    OI_UINT32 *totalNumHandles);

/**
 * This function returns a list of SDP record handles + the associated Profile UUIDs available in the local SDP database.
 *
 * @param recordList          Buffer into which the service record summaries are written.
 * @param maxNumrecords       Maximum number of record summaries that could be returned (@recordList buffer size).
 * @param numRecordsReturned  Number of SDP record summaries written into @recordList.
 * @param totalNumRecords     Total number of valid records in the local SDP database; may be larger than
 *                            @numRecordsReturned if @RecordList is not large enough to hold all the service
 *                            record handles.
 * @return                    OI_OK if record handles were successfully retrieved; error status otherwise.
 *
 */
OI_STATUS OI_SDPDB_GetLocalDBSummary(OI_SDP_SUMMARY_REC *recordList,
                                     OI_UINT32 maxNumRecords,
                                     OI_UINT32 *numRecordsReturned,
                                     OI_UINT32 *totalNumRecords);

/**
 * This function returns the total number of SDP records available in local SDP database.
 *
 * @param totalNumRecords     Total number of valid records in the local SDP database.
 *
 * @return                    OI_OK if the request was successful; error status otherwise.
 *
 */
OI_STATUS OI_SDPDB_GetNumServiceRecords(OI_UINT32 *totalNumRecords);


/**
 * This function generates an XML stream for a specified SDP reecord.
 *
 * @param recordHandle Indicates the requested record.
 *
 * @param pBufferXML  Pointer to a buffer where the generated XML
 *                    string will be written, with NULL termination.
 *
 * @param pBufferSize Pointer to the size of buffer indicated by
 *                    @pBufferXML. Upon function's return, will
 *                    contain the actual size of the generated XML
 *                    string. If the function returns
 *                    OI_STATUS_NO_RESOURCES, the referenced value
 *                    will be set to the buffer size required to
 *                    contain the complete XML output.
 *
 * @return            OI_OK if the request was successful; error status
 *                    otherwise. Error status OI_STATUS_NO_RESOURCES
 *                    will indicate that the generated XML string did
 *                    not fit into provided buffer. The buffer will
 *                    contain the truncated version of XML output. The
 *                    required total size will be written to
 *                    @pBufferSize.
 *
 * @deprecated        Equivalent to OI_SDPDB_GetRecordAsXML(attr, pBufferXML, pBufferSize, indent, OI_SDPDB_XML_FORMAT_OPENBLUE)
 */
OI_STATUS OI_SDPDB_ReadRecordXML( OI_UINT32  recordHandle,
                                  OI_CHAR   *pBufferXML,
                                  OI_UINT16 *pBufferSize);

/**
 * This function generates an XML stream in the requested format for a
 * specified SDP reecord.
 *
 * @param recordHandle Indicates the requested record.
 *
 * @param pBufferXML  Pointer to a buffer where the generated XML
 *                    string will be written, with NULL termination.
 *
 * @param pBufferSize Pointer to the size of buffer indicated by
 *                    @pBufferXML. Upon function's return, will
 *                    contain the actual size of generated XML
 *                    string. If the function returns
 *                    OI_STATUS_NO_RESOURCES, the referenced value
 *                    will be set to the buffer size required to
 *                    contain the complete XML output.
 *
 * @param format      The XML format to use. Refer to
 *                    OI_SDPDB_XML_FORMAT for the list of available
 *                    formats.
 *
 * @return            OI_OK if the request was successful; error status
 *                    otherwise. Error status OI_STATUS_NO_RESOURCES
 *                    will indicate that the generated XML string did
 *                    not fit into provided buffer. The buffer will
 *                    contain the truncated version of XML output. The
 *                    required total size will be written to
 *                    @pBufferSize.
 */
OI_STATUS OI_SDPDB_GetRecordAsXML( OI_UINT32  recordHandle,
                                   OI_CHAR   *pBufferXML,
                                   OI_UINT16 *pBufferSize,
                                   OI_SDPDB_XML_FORMAT format);

/**
 * This function generates an XML stream for a specified SDP attribute.
 *
 * @param recordHandle Indicates the requested SDP attribute.
 *
 * @param pBufferXML   Pointer to a buffer where the generated XML
 *                     string will be written, with NULL termination.
 *
 * @param pBufferSize  Pointer to the size of buffer indicated by
 *                     @pBufferXML. Upon function's return, will
 *                     contain the actual size of generated XML
 *                     string.  If the function returns
 *                     OI_STATUS_NO_RESOURCES, the referenced value
 *                     will be set to the buffer size required to
 *                     contain the complete XML output.
 *
 * @param indent       Initial indentaion (spatial offset) of SDP attribute
 *                     in XML form.
 *
 * @return             OI_OK if the request was successful; error status
 *                     otherwise. Error status OI_STATUS_NO_RESOURCES
 *                     will indicate that the generated XML string did
 *                     not fit into provided buffer. The buffer will
 *                     contain the truncated version of XML
 *                     output. The required total size will be written
 *                     to @pBufferSize.
 *
 * @deprecated         Equivalent to OI_SDPDB_GetAttributeAsXML(attr, pBufferXML, pBufferSize, indent, OI_SDPDB_XML_FORMAT_OPENBLUE)
 */
OI_STATUS OI_SDPDB_ReadAttrXML( OI_SDPDB_ATTRIBUTE *attr,
                                OI_CHAR            *pBufferXML,
                                OI_UINT16          *pBufferSize,
                                OI_UINT8            indent);

/**
 * This function generates an XML stream in the requested format for a
 * specified SDP attribute.
 *
 * @param recordHandle Indicates the requested SDP attribute.
 *
 * @param pBufferXML   Pointer to a buffer where the generated XML
 *                     string will be written, with NULL termination.
 *
 * @param pBufferSize  Pointer to the size of buffer indicated by
 *                     @pBufferXML. Upon function's return, will
 *                     contain the actual size of generated XML
 *                     string. If the function returns
 *                     OI_STATUS_NO_RESOURCES, the referenced value
 *                     will be set to the buffer size required to
 *                     contain the complete XML output.
 *
 * @param indent       Initial indentation (spatial offset) of SDP attribute
 *                     in XML form.
 *
 * @param format       The XML format to use.  Refer to
 *                     OI_SDPDB_XML_FORMAT for the list of available
 *                     formats.
 *
 * @return             OI_OK if the request was successful; error status
 *                     otherwise. Error status OI_STATUS_NO_RESOURCES
 *                     will indicate that the generated XML string did
 *                     not fit into provided buffer. The buffer will
 *                     contain the truncated version of XML
 *                     output. The required total size will be written
 *                     to @pBufferSize.
 */
OI_STATUS OI_SDPDB_GetAttributeAsXML( OI_SDPDB_ATTRIBUTE *attr,
                                      OI_CHAR            *pBufferXML,
                                      OI_UINT16          *pBufferSize,
                                      OI_UINT8            indent,
                                      OI_SDPDB_XML_FORMAT format);

/**
 * This function sets callback functions that are triggered when the SDP database
 * is modified.  These callbacks default to NULL, and may be set to NULL or non-NULL
 * values by this function.
 *
 * @param change Pointer to a function that will be called when an SDPDB record
 *               is added or modified.
 *
 * @param remove Pointer to a function that will be called when an SDPDB record
 *               is removed.
 */
void OI_SDPDB_RegisterNotification( OI_SDPDB_CHANGED_RECORD change,
                                    OI_SDPDB_REMOVING_RECORD remove);



/*
 * Lookup service records in the local SBD database that contain the requested UUID
 *
 * @param   serviceUUID Service name UUID, 32 bit version
 * @param   enumerator  Pointer to an integer that keeps track of progress through the
 *                      database. The caller should set the initial value of *enumerator to zero.
 *
 * @return  Value of service record handle, zero if not found.
 *
 */
OI_UINT32 OI_SDPDB_LookupServiceRecord(OI_UUID32 serviceUUID,
                                       OI_UINT16 *enumerator);

/*
 * Lookup an attribute in a specific service record in the local SDP database.
 *
 * @param   srec        Handle to a service record
 * @param   attrID      Attribute ID
 *
 * @return  Pointer to the matching attribute or NULL if the attribute is not
 *          found. The caller must not attempt to free the result and must treat the
 *          attribute as read only.
 *
 */
const OI_DATAELEM *OI_SDPDB_LookupAttribute(OI_UINT32 srec,
                                            OI_UINT16 attrId);


#ifdef __cplusplus
}
#endif

/**@}*/

/*****************************************************************************/
#endif /* _OI_SDPDB_H */

