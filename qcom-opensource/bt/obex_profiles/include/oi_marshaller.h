#ifndef _MARSHALLER_H
#define _MARSHALLER_H

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
 * This file provides functions for marshalling and unmarshalling data elements into bytestreams.
 */

#include "oi_stddefs.h"
#include "oi_bt_spec.h"
#include "oi_dataelem.h"
#include "oi_bytestream.h"

/** \addtogroup Marshaller_Internal */
/**@{*/

#ifdef __cplusplus
extern "C" {
#endif


/**
 * This function serializes and writes a data element to a byte stream. The
 * caller should ensure that there is enough space in the byte stream to
 * accomodate the complete data element list.  To determine how much space will
 * be required, call OI_DataElement_MarshalledSize. If there is insufficient
 * room to write the data an error status will be reported on the byte stream.
 *
 * @param ByteStream  the byte stream that the data element will be written to.
 *
 * @param Element     a pointer to the data element that will be written.
 *
 * @return            TRUE if the data element was succesfully marshalled, FALSE
 *                    if there was not enough room in the byte stream.
 */

OI_BOOL OI_DataElement_Marshal(OI_BYTE_STREAM *ByteStream,
                               const OI_DATAELEM *Element);


/**
 * This function serializes and writes a segment of a data element to a byte
 * stream.
 *
 * @param ByteStream     the byte stream that the data element will be written to.
 *
 * @param segmentOffset  number of data element bytes to be ignored.
 *
 * @param Element        a pointer to the data element that will be written.
 *
 * @return               TRUE if the data element was completely marshalled, FALSE
 *                       if there was not enough room in the byte stream.
 */

OI_BOOL OI_DataElement_MarshalSegment(OI_BYTE_STREAM *ByteStream,
                                      OI_UINT16 *segmentOffset,
                                      const OI_DATAELEM *Element);

/**
 * Looks at the first data element in the bytestream and returns the element
 * type, size, and the start of the next element. Thes size returned is the size
 * in bytes of the element value. The position in the bytestream remains
 * unchanged.
 */

OI_STATUS OI_DataElement_Peek(OI_BYTE_STREAM *ByteStream,
                              OI_UINT8 *elemType,
                              OI_UINT16 *size);

/**
 * Unmarshalls a data element sequence returning the size of the data element
 * list header and number of bytes in the list itself.
 */
OI_STATUS OI_DataElement_UnmarshallListHeader(OI_BYTE_STREAM *ByteStream,
                                              OI_UINT16 *headerBytes,
                                              OI_UINT16 *listBytes);

/**
 * This function returns the byte count required to store the passed data
 * element, or 0 if insufficient memory to calculate count.
 */

OI_UINT16 OI_DataElement_MarshalledSize(const OI_DATAELEM *Element);


/**
 * This function converts a data element from the marshalled (network)
 * representation to a more convenient in-memory representation.
 *
 * The function allocates dynamic memory for the unmarshalled data element. The
 * caller must call OI_DataElement_Free to deallocate this memory.
 *
 * This function will return an error status if dynamic memory can not be
 * allocated to unmarshal the data element or if the data element is not
 * correclty formed.
 *
 * If the data element contains list elements (DATAELEM_ELEMENT_SEQUENCE or
 * DATAELEM_ELEMENT_ALTERNATIVE) the list elements will be allocated as a
 * contiguous array of SDP_DATA_ELEMENTs.
 */

OI_STATUS OI_DataElement_Unmarshal(OI_BYTE_STREAM *ByteStream,
                                   OI_DATAELEM *Element);

/**
 * This function recursively copies a data element tree.
 */

OI_STATUS OI_DataElement_Clone(OI_DATAELEM *toElem,
                               const OI_DATAELEM *fromElem);


/**
 * Transfer a data value from one data element to another. The fromElem is set
 * to a null data element.
 */
void OI_DataElement_Transfer(OI_DATAELEM *toElem,
                             OI_DATAELEM *fromElem);

/**
 * This function recursively frees all memory allocated to unmarshal a data
 * element.
 */

void OI_DataElement_Free(OI_DATAELEM *Element);



/*****************************************************************************/
#ifdef __cplusplus
}
#endif

/**@}*/

#endif /* _MARSHALLER_H */

