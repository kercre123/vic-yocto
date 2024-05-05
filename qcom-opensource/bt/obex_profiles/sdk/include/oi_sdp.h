#ifndef _OI_SDP_H
#define _OI_SDP_H

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
 *  This file provides the API for client-side management of connections and
 *  queries to a remote SDP server and its service database.
 *
 *  For more information see the @ref SDP_docpage section of the BLUEmagic 3.0 SDK documentation.
 */

#include "oi_status.h"
#include "oi_bt_spec.h"
#include "oi_dataelem.h"
#include "oi_bt_stack_config.h"

/** \addtogroup SvcDisc Service Discovery APIs */
/**@{*/

#ifdef __cplusplus
extern "C" {
#endif



#define OI_SDP_ClientInit(x)         (OI_OK)
#define OI_SDP_ClientTerminate()     (OI_OK)

/**
 * This is the type definition for a service record handle. A service record
 * handle is returned by an SDP server in response to a service search request
 * (OI_SDP_SearchReq()). The service record handle only has local meaning within
 * the SDP server that returned it. The only exception to this is for the
 * service record handle value 0, which is the service record handle for the
 * service record for the SDP server itself.
 */
typedef OI_UINT32 OI_SDP_SERVICE_RECORD_HANDLE;

/**
 * This is the type definition for a service record handle. A service
 * record handle is returned by OI_SDP_Connect() in order to uniquely
 * identify an SDP connection. The handle is used in all SDP function
 * calls and callbacks.
 */
typedef OI_UINT16 OI_SDP_CONNECTION_HANDLE;

/**
 * A callback function of this type confirms that a connection has been
 * established to an SDP server.
 *
 * A callback function of this type must be provided by the client application
 * in calls to OI_SDP_Connect() and is called when a connect request has
 * completed, either with success or failure. If the connection request failed
 * or was rejected by the SDP server, then the status code indicates the reason
 * for the failure.
 *
 * @param handle       Identifier for the connection associated with this call.
 *
 * @param status       Returned status code; OI_OK if the connection was
 *                     established, error status otherwise.
 */
typedef void (*OI_SDP_CONNECT_CFM_CB)(OI_SDP_CONNECTION_HANDLE handle,
                                      OI_STATUS status);


/**
 * A callback function of this type indicates that a connection to an SDP server
 * has been terminated.
 *
 * A callback function of this type must be provided by the client application
 * in calls to OI_SDP_Connect() and is called when a disconnection has
 * occurred.
 *
 * @param handle       Identifier for the connection associated with this call.
 */
typedef void (*OI_SDP_DISCONNECT_IND_CB)(OI_SDP_CONNECTION_HANDLE handle);


/**
 * A callback function of this type provides the results of a service
 * search request.
 *
 * A callback function of this type must be provided by the client
 * application in calls to OI_SDP_SearchReq() and is called when a
 * service search request has completed.
 *
 * It is the responsibility of the client application to make use of
 * the service record array returned by this callback function. The
 * SDP layer does not itself maintain a data structure to assemble
 * service search responses. If the client application is to make use
 * of the service records returned by a service search response, the
 * client application must copy the service record data into memory
 * local to the application.
 *
 * In short, returned data must be consumed inside the callback and
 * not referenced outside the callback.
 *
 *
 * @param handle               Identifier for the connection associated with
 *                             this call.
 *
 * @param serviceRecord        Pointer to an array of SDP service
 *                             record handles.
 *
 * @param serviceRecordCount   Number of service records in the array.
 *
 * @param status               Status of the service search request; OI_OK if
 *                             successful, error status otherwise. If the
 *                             status is not OI_OK, then parameters
 *                             serviceRecords and serviceRecordCount are
 *                             undefined.
 */
typedef void (*OI_SDP_SEARCH_RSP_CB)(OI_SDP_CONNECTION_HANDLE handle,
                                     OI_SDP_SERVICE_RECORD_HANDLE *serviceRecords,
                                     OI_UINT16 serviceRecordCount,
                                     OI_STATUS status);

/**
 * A callback function of this type provides the results of a service
 * attribute request.
 *
 * A callback function of this type must be provided by the client
 * application in calls to OI_SDP_AttributeReq() and is called when a
 * service attribute request has completed.
 *
 * Attributes are returned as a pointer to a data element sequence
 * (@ref OI_DATAELEM_SEQ) of data elements pairs in which the first
 * data element in each pair is an attribute identifier (@ref
 * OI_DATAELEM_UINT) and the second data element is the attribute
 * value. In other words, the actual attribute values are two levels
 * of indirection away from the value returned by this function.
 *
 * The parameter 'more' indicates that the service attribute request
 * is incomplete and that there are more attributes that meet the
 * search criteria. To request the remaining attributes, call
 * OI_SDP_ContinueCurrentReq().
 *
 * It is the responsibility of the client application to make use of
 * the service attribute record array returned by this callback
 * function. The SDP layer does not itself maintain a data structure
 * to assemble service attribute responses. If the client application
 * is to make use of the service attribute records returned by a
 * service attribute response, the client application must copy the
 * service attribute record data into memory local to the application.
 *
 * In short, returned data must be consumed inside the callback and
 * not referenced outside the callback.
 *
 * @param handle       Identifier for the connection associated with this call
 *
 * @param attributes   This value is a pointer to a data element sequence.
 *                     The data element sequence value is a pointer to an array
 *                     of data element pairs; for each of these pairs, the first
 *                     element is an attribute ID and the second is an attribute
 *                     value.
 *
 * @param more         Indicates that the service attribute request is
 *                     incomplete and that there are more attributes that meet
 *                     the search criteria.
 *
 * @param status       Status of the service attribute request is OI_OK if
 *                     successful, error status otherwise. If the status is
 *                     not OI_OK then other parameters (more and attributes) are
 *                     undefined.
 */
typedef void (*OI_SDP_ATTRIBUTE_RSP_CB)(OI_SDP_CONNECTION_HANDLE handle,
                                        OI_DATAELEM *attributes,
                                        OI_BOOL more,
                                        OI_STATUS status);


/**
 * A callback function of this type provides the results of a service
 * search attribute request.
 *
 * A callback function of this type must be provided by the client
 * application in calls to OI_SDP_SearchAttributeReq() and is called
 * when a service seacrh attribute request has completed.
 *
 * Attributes are returned as a pointer to a data element sequence
 * (@ref OI_DATAELEM_SEQ) of responses. Each response is itself a data
 * element sequence; the value of each of these data element sequences
 * is a list of data element pairs in which the first data element in
 * each pair is an attribute identifier (@ref OI_DATAELEM_UINT) and
 * the second data element is the attribute value. In other words, the
 * actual attribute values are three levels of indirection away from
 * the value returned by this function.
 *
 * The parameter 'more' indicates that the service attribute request
 * is incomplete and that there are more attributes that meet the
 * search criteria. To request the remaining attributes, call
 * OI_SDP_ContinueCurrentReq().
 *
 * It is the responsibility of the client application to make use of
 * the service attribute record array returned by this callback
 * function. The SDP layer does not itself maintain a data structure
 * to assemble service search attribute responses. If the client
 * application is to make use of the service records returned by a
 * service search attribute response, the client application must copy
 * the service attribute record data into memory local to the
 * application.
 *
 * In short, returned data must be consumed inside the callback and
 * not referenced outside the callback.
 *
 * @param handle          Identifier for the connection associated with this
 *                        call.
 *
 * @param attributesList  Pointer to a data element sequence.
 *                        The data element sequence value is a pointer to a list
 *                        (data element sequence) of responses. The value of each of
 *                        these data element sequences is a pointer to a list of data
 *                        element pairs; for each of these pairs, the first element
 *                        is an attribute ID and the second is an attribute value.
 *
 * @param more            Indicates that the service attribute request is
 *                        incomplete and that there are more attributes that
 *                        meet the search criteria.
 *
 * @param status         Status of the service attribute request: OI_OK if
 *                       successful, error status otherwise. If the status is
 *                       not OI_OK then other parameters (more and attributes)
 *                       are undefined.
 */
typedef void (*OI_SDP_SEARCH_ATTRIBUTE_RSP_CB)(OI_SDP_CONNECTION_HANDLE handle,
                                               OI_DATAELEM *attributesList,
                                               OI_BOOL more,
                                               OI_STATUS status);


/**
 * This function requests a connection with an SDP server.
 *
 * The reason for making such a connection is to allow the SDP client on the
 * local device to query the the SDP service database on a remote device.
 *
 * @param connectCfmCB        Confirmation callback function that is
 *                            called when the connection command has completed,
 *                            either successfully or with some error status
 *                            returned.
 *
 * @param disconnectIndCB     Indication callback function that will be
 *                            called if the connection is terminated, either
 *                            because of an error or because the application
 *                            explicitly called OI_SDP_Disconnect().
 *
 * @param deviceAddr          Pointer to the address of the remote Bluetooth device
                              acting as the SDP server.
 *
 * @param handle              Output parameter for the connection handle
 *                            created by this call.
 *
 * @return                    OI_OK if the connection is successfully
 *                            established. If the connection request fails, various
                              error codes indicate the reason for the failure.
 */
OI_STATUS OI_SDP_Connect(OI_SDP_CONNECT_CFM_CB connectCfmCB,
                         OI_SDP_DISCONNECT_IND_CB disconnectIndCB,
                         const OI_BD_ADDR* deviceAddr,
                         OI_SDP_CONNECTION_HANDLE* handle);

/**
 * This function terminates the specified connection to an SDP server. This
 * function does not terminate the underlying L2CAP connection if that
 * connection is in use by other applications or profiles.
 *
 * When a connection to an SDP server has been terminated for any reason, this
 * triggers the invocation of the OI_SDP_DISCONNECT_IND_CB disconnect indication
 * callback function that was provided in the OI_SDP_Connect() call that
 * established the connection in the first place.
 *
 * @param handle      Identifier for the connection associated with this call.
 *
 * @return            OI_OK unless there is no current connection.
 */
OI_STATUS OI_SDP_Disconnect(OI_SDP_CONNECTION_HANDLE handle);


/**
 * This function initiates a search for services in the SDP server referenced by
 * the remote device address, using a list of service UUIDs to select the
 * required services.
 *
 * @param handle             Identifier for the connection associated with this call.
 *
 * @param responseCB         Callback function that returns the results of the
 *                           service search request.
 *
 * @param maxServiceRecords  Specifies the maximum number of service records to query for and
 *                           return in the callback function. This must be > 0.
 *
 * @param UUIDList           @ref OI_DATAELEM_SEQ data element list of
 *                           @ref OI_DATAELEM_UUID UUID data elements representing
 *                           the UUIDs that will be searched for in the remote
 *                           SDP server database.
 *
 * @return                   OI_OK if the request was succesfully queued to be
 *                           sent to the SDP server; error status otherwise.
 */
OI_STATUS OI_SDP_SearchReq(OI_SDP_CONNECTION_HANDLE handle,
                           OI_SDP_SEARCH_RSP_CB responseCB,
                           OI_UINT16 maxServiceRecords,
                           const OI_DATAELEM *UUIDList);

/**
 * This function initiates a search for service attributes in a service record.
 *
 * The service record handle is a 32-bit value that identifies the service
 * record in the remote SDP database that is being searched by this request. The
 * client application obtains service record handles by calling
 * OI_SDP_SearchReq().
 *
 * The @ref OI_DATAELEM_SEQ attribute list that is returned by the response
 * callback function is a list of @ref OI_DATAELEM_UINT 16- and 32-bit unsigned
 * integer data elements. 16-bit data elements represent individual attribute
 * IDs; 32-bit data elements represent attribute ID ranges, where the most
 * significant 16 bits represent the attribute ID at the low bound of the range
 * and the least significant 16 bits represent attribute ID at the high bound of
 * the range.
 *
 * @note The SDP specification requires the attribute list to be sorted from low
 * attribute ID value to high attribute ID value. It is the responsibility of
 * the SDP server application to ensure that this ordering is correct. The
 * attribute request function fails with an error status if the attribute IDs
 * are out of order or there are overlapping ranges. See Volume 3 of the
 * Bluetooth specification for more information.
 *
 * @param handle                Identifier for the connection associated with
 *                              this call.
 *
 * @param responseCB            Callback function that returns the results of
 *                              the service attribute request.
 *
 * @param serviceRecordHandle   Specifies the service record handle.
 *
 * @param attributeRangeList    This is a @ref OI_DATAELEM_SEQ attribute list of
 *                              @ref OI_DATAELEM_UINT 16- and 32-bit unsigned
 *                              integer data elements. The macro
 *                              OI_ATTRID_RANGE in oi_bt_assigned_nos.h can be
 *                              used to assemble a 32-bit value representing a
 *                              range of attributes.
 *
 * @return                      OI_OK if the request was successfully queued to
 *                              be sent to the server; error code otherwise.
 */
OI_STATUS OI_SDP_AttributeReq(OI_SDP_CONNECTION_HANDLE handle,
                              OI_SDP_ATTRIBUTE_RSP_CB responseCB,
                              OI_SDP_SERVICE_RECORD_HANDLE serviceRecordHandle,
                              const OI_DATAELEM *attributeRangeList);


/**
 * This function combines the functionality of SDP_ServerSearchReq() and
 * OI_SDP_AttributeReq() into a single request.
 *
 * @param handle                Identifier for the connection associated with
 *                              this call
 *
 * @param responseCB            Callback function that returns the results of
 *                              the service attribute request.
 *
 * @param UUIDList              An @ref OI_DATAELEM_SEQ data element list of
 *                              @ref OI_DATAELEM_UUID UUID data elements
 *                              representing the UUIDs that will be searched for
 *                              in the remote SDP server database
 *
 * @param attributeRangeList    This is a @ref OI_DATAELEM_SEQ attribute list of
 *                              @ref OI_DATAELEM_UINT 16- and 32-bit unsigned
 *                              integer data elements. The macro
 *                              OI_ATTRID_RANGE in oi_bt_assigned_nos.h can be
 *                              used to assemble a 32-bit value representing a
 *                              range of attributes.
 *
 * @return                      OI_OK if the request was successfully queued to
 *                              be sent to the server; error code otherwise.
 */
OI_STATUS OI_SDP_SearchAttributeReq(OI_SDP_CONNECTION_HANDLE handle,
                                    OI_SDP_SEARCH_ATTRIBUTE_RSP_CB responseCB,
                                    const OI_DATAELEM *UUIDList,
                                    const OI_DATAELEM *attributeRangeList);

/**
 * This function requests a continuation of the service attribute request or
 * service search attribute request for a specific connection.
 *
 * When the complete response to a call to OI_SDP_AttributeReq() or
 * OI_SDP_SearchAttributeReq() can not be returned in a single response packet,
 * the 'more' parameter is set in the request's callback function. To request
 * the rest of the response, the client application calls this continuation
 * function. Depending on the amount of response data and the various packet
 * sizes, the application may need to call the continuation function several
 * times to retrieve the complete response.
 *
 * If the client application does not need to continue the request but there is
 * still more response data outstanding, the client application must call
 * OI_SDP_EndCurrentReq() or OI_SDP_Disconnect() to free up resources allocated
 * to the connection and the ongoing request.
 *
 * @param handle      Identifier for the connection associated with this call.
 *
 * @return    OI_OK if the continuation request was successfully queued to be
 *            sent to the SDP server; error status otherwise, including the case
 *            in which there was no request currently in progress.
 */
OI_STATUS OI_SDP_ContinueCurrentReq(OI_SDP_CONNECTION_HANDLE handle);


/**
 * This function terminates the service attribute request or service
 * attribute search request for a specific connection. See also
 * OI_SDP_ContinueCurrentReq().
 *
 * @param handle      Identifier for the connection associated with this call.
 *
 */
OI_STATUS OI_SDP_EndCurrentReq(OI_SDP_CONNECTION_HANDLE handle);


/*****************************************************************************/
#ifdef __cplusplus
}
#endif

/**@}*/

#endif /* _OI_SDP_H */

