#ifndef _OI_XML_H_
#define _OI_XML_H_

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
 * Basic xML parser.
 *
 */

#include "oi_stddefs.h"
#include "oi_status.h"
#include "oi_unicode.h"

/** \addtogroup XML */
/**@{*/

#ifdef __cplusplus
extern "C" {
#endif

/**
 * This structure defines an XML attribute.
 */
typedef struct OI_XML_ATTRIBUTE {
    OI_PSTR name;                  /**< Name of this attribute */
    OI_PSTR value;                 /**< Value of this attribute */
    OI_CHAR quote;                 /**< Indicates if values was wrapped in single or double quotes */
    struct OI_XML_ATTRIBUTE *next; /**< Next attribute in the list */
} OI_XML_ATTRIBUTE;



typedef enum {
    OI_XML_ELEMENT,        /**<   <foo> .. </foo>         */
    OI_XML_TEXT,           /**<   Text content            */
    OI_XML_DECLARATION,    /**<   <?xml ... ?>            */
    OI_XML_CDATA,          /**<   <![CDATA[ ... ]]>       */
    OI_XML_PI,             /**<   <? ... ?>      */
    OI_XML_COMMENT,        /**<   <!-- ... >              */
    OI_XML_MISCELLANEOUS   /**<   <! ... >                */
} OI_XML_NODE_TYPE;


/**
  * This structure defines an XML node. The content type indentifies if this is an XML element or one
  * of the non-element entities. The content type of an XML element is OI_XML_TEXT, if the name is
  * null (name.sz == 0) this is a textual content that is intermingled with other elements. For
  * example in the following XML:
  *
  * <outer>
  *      <inner1\>
  *      text
  *      <inner2>more text<\inner2>
  * </outer>
  *
  * The node "outer" has three children, they all have content type OI_XML_TEXT, "inner1" has an
  * empty content string, the second child has no name and a content string "text", the third child
  * has the name "inner2 and content string "more text" this node has no children.
  */
typedef struct OI_XML_NODE {
    OI_PSTR name;                     /**< The node tag - may be an empty string (size 0) */
    OI_PSTR content;                  /**< Text content of the node */
    OI_XML_NODE_TYPE nodeType;        /**< Type of this node */
    OI_XML_ATTRIBUTE *attributes;     /**< Linked list of attributes for this node */
    struct OI_XML_NODE *parent;       /**< Enclosing parent of this node */
    struct OI_XML_NODE *children;     /**< Nodes nested within this node */
    struct OI_XML_NODE *sibling;      /**< Next right-hand sibling of this node */
    void *user;                       /**< This is available for use by the application. */
} OI_XML_NODE;


/**
 * This structure defines an XML document.
 */
typedef struct {
    OI_UNICODE_ENCODING encoding;
    OI_XML_NODE root;
} OI_XML_DOCUMENT;


/*
 * The following XML sequence:
 *
 *   <p>the quick <em>brown</em> dog jumped</p>
 *
 * will generate the following indicator sequence:
 *
 *   OI_XML_START_TAG_IND(p)
 *   OI_XML_CONTENT_IND(the quick )
 *   OI_XML_START_TAG_IND(em)
 *   OI_XML_CONTENT_IND(brown)
 *   OI_XML_END_TAG_IND(em)
 *   OI_XML_CONTENT_IND( dog jumped)
 *   OI_XML_END_TAG_IND(p)
 */
typedef void (*OI_XML_START_TAG_IND)(OI_XML_NODE *node);
typedef void (*OI_XML_END_TAG_IND)(OI_XML_NODE *node);
typedef void (*OI_XML_NON_ELEMENT_IND)(OI_XML_NODE *node);
typedef void (*OI_XML_CONTENT_IND)(OI_PSTR *content, OI_XML_NODE_TYPE nodeType);

/**
  This structure defines the XML start, end, and content node
  indicators.
  */
typedef struct {
    OI_XML_START_TAG_IND startTagInd;
    OI_XML_END_TAG_IND endTagInd;
    OI_XML_NON_ELEMENT_IND nonElementInd;
    OI_XML_CONTENT_IND contentInd;
} OI_XML_PARSER_INDICATORS;

/**
 * Parses an XML stream and build a tree stored in the children of the @a document parameter. nodes
 * are dynamically allocated and inserted into the tree as they are parsed, then the corresponding
 * indicator is issued. When done with the tree OI_XML_FreeTree(document) must be called to release
 * the memory.
 *
 * @param stream      The raw XML stream. This cannot point to
 *                    the inside of a node, it must be at the
 *                    beginning of a node or content section.
 *
 * @param indicators  These are called when nodes are opened, closed, or content is parsed.  The
 *                    parameters to these functions are valid until OI_XML_FreeTree is called.
 *
 * @param maxDepth    The maximum number of XML layers to traverse before declaring failure.
 *                    If maxDepth==0 parser will use the internal maximum.
 *
 * @param encoding    encoding of the XML stream (OI_UNICODE_UNKNOWN, OI_UNICODE_UTF8, OI_UNICODE_UTF16_LE, OI_UNICODE_UTF16_BE)
 *
 * @param document    [OUT] The parsed XML document
 *
 * @return      OI_OK If no errors encountered.
 */
OI_STATUS OI_XML_ParseTree(const OI_PSTR *stream,
                           const OI_XML_PARSER_INDICATORS *indicators,
                           OI_UINT maxDepth,
                           OI_UNICODE_ENCODING encoding,
                           OI_XML_DOCUMENT *document);

/**
 * Release any memory used in building a parse tree.
 *
 * @param document   The children of this node will be freed.
 */
void OI_XML_FreeTree(OI_XML_DOCUMENT *document);


/**
 * Get the type of a node.
 *
 * @param node    The node whose type we are interested in.
 *
 * @return      The type of the node.
 */
OI_XML_NODE_TYPE OI_XML_GetNodeType(const OI_XML_NODE *node);


/**
 * Retrieve a specified attribute from a node.
 *
 * @param document    The document containing the node
 *
 * @param node         The node whose attribute we are interested in.
 *
 * @param name        The name of the attribute.
 *
 * @return  The attribute or NULL if it is not found.
 */
const OI_XML_ATTRIBUTE* OI_XML_GetAttribute(const OI_XML_DOCUMENT *document,
                                            const OI_XML_NODE *node,
                                            const OI_CHAR *name);


/**
 * Gets the unicode name of an XML node
 *
 * @param node       The node whose name we want to get
 *
 * @return      Pointer to unicode string with the name or NULL if the node does not have a name.
 */
const OI_PSTR* OI_XML_GetNodeName(const OI_XML_NODE *node);


/**
 *  Returns a count of the number of children for a specific node. The count includes text and
 *  nested markup nodes.
 *
 * @param node   The node whose children count we want.
 *
 * @return      The number of child nodes for the current node
 */
OI_UINT16 OI_XML_GetNumChildren(const OI_XML_NODE *node);


/**
 * Returns the first child node of the given node or NULL if the node has no children.
 *
 * @param node   The node whose first child we want.
 *
 *
 *@return     The first child node or NULL if the node has no children.
 */
const OI_XML_NODE* OI_XML_GetFirstChildNode(const OI_XML_NODE *node);


/**
 * Returns the next sibling node of the given node or NULL if the node has no siblings.
 *
 * @param node   The node whose next sibling we want.
 *
 *@return     The sibling node or NULL if the node has no siblings.
 */
const OI_XML_NODE* OI_XML_GetNextSiblingNode(const OI_XML_NODE *node);


/**
 * Gets the unicode text content of an XML node. Note that if the text content is intermingled with
 * nested nodes only the first content block, which may be empty, is returned. The remaining
 * content is accsssed via name-less child nodes of this node.
 *
 * @param node       The node whose content we want to get
 *
 * @return     Pointer to unicode content string or NULL if the node has no content.
 */
const OI_PSTR* OI_XML_GetTextContent(const OI_XML_NODE *node);


/**
 *  Returns a count of the number of attributes for a specific node.
 *
 * @param node   The node whose attribute count we want.
 *
 * @return      The number of attributes for the current node
 */
OI_UINT16 OI_XML_GetNumAttributes(const OI_XML_NODE *node);


/**
 * Retrieve the first attribute from a node.
 *
 * @param node     The node whose attributes we are interested in.
 *
 * @return  The attribute or NULL if there are none.
 */
const OI_XML_ATTRIBUTE *OI_XML_GetFirstAttribute(const OI_XML_NODE *node);


/**
 * Retrieve the the next attribute from a node given the previous attribute
 *
 * @param node     The node whose attributes we are interested in.
 *
 * @param prev    The previous attribute
 *
 * @return  The attribute or NULL if there are no more.
 */
const OI_XML_ATTRIBUTE *OI_XML_GetNextAttribute(const OI_XML_NODE *node,
                                                const OI_XML_ATTRIBUTE *prev);



/*
 * A few functions provided for printing the internal structures to standard out.
 * These only print in debug builds.
 */
void OI_XML_DumpTree(OI_XML_DOCUMENT *document);


#ifdef __cplusplus
}
#endif

/**@}*/

#endif /* _OI_XML_H_ */
