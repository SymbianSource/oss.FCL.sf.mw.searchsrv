/*
* Copyright (c) 2010 Nokia Corporation and/or its subsidiary(-ies).
* All rights reserved.
* This component and the accompanying materials are made available
* under the terms of "Eclipse Public License v1.0"
* which accompanies this distribution, and is available
* at the URL "http://www.eclipse.org/legal/epl-v10.html".
*
* Initial Contributors:
* Nokia Corporation - initial contribution.
*
* Contributors:
*
* Description: 
*
*/

#ifndef CPIX_CPIXDOC_H
#define CPIX_CPIXDOC_H


#include "cpixinit.h"
#include "cpixerror.h"

#include <math.h> // float_t

//#include "indevicecfg.h"

/**
 * Document related CPix API.
 *
 * See cpixsearch.h for further info and notes on CPix API.
 */


#ifdef __cplusplus
extern "C" {
#endif


    /************************************************************************
     * Class cpix_Field
     */
    /**
     * Storage related flags
     */
    enum cpix_Store_
        {
            // stores the field in the index
            cpix_STORE_YES = 1,
                
            // does not stores the field in the index
            cpix_STORE_NO  = 2
                
            // Not supported as yet
            // STORE_COMPRESS = 4
        };
    typedef enum cpix_Store_ cpix_Store;

    /**
     * Indexing action related flags
     */
    enum cpix_Index_
        {
            // do not index the field value - but it can still be
            // retrieved
            cpix_INDEX_NO = 16,
                
            // index the value of the field after an analyzer has
            // tokenized it
            cpix_INDEX_TOKENIZED = 32,

            // index the value of the field without an analyzer and
            // disable storing of norms
            cpix_INDEX_UNTOKENIZED = 64,
        };

    typedef enum cpix_Index_ cpix_Index;

    enum cpix_Aggregate_ 
    	{
            // NOTE: Aggregate flags are picked intentionally so that
            // 	     they won't conflict with CLucene.
    	
            // Do not expose the value to be searchable through the
            // aggregate field
            cpix_AGGREGATE_NO = 1<<30,
            
            // Expose the value to be searchable throught the
            // aggregate field
            cpix_AGGREGATE_YES = 1<<31
    	};
    
    typedef enum cpix_Aggregate_ cpix_Aggregate;   
    
    struct cpix_Field_
    {
        // pointer to native (CLucene) impl
        void       * ptr_;
        
     //   bool		aggregate_;
        
        // Last error, if any, that resulted from executing the
        // last operation on this cpix_XXX object
        // Use macros cpix_Failed, cpix_Succeeded and cpix_ClearError.
        // Do not attempt releasing it.
        cpix_Error * err_;
    };
    typedef cpix_Field_ cpix_Field;

    
    /** 
     * Creates (initializes) a field - creates the native instance and
     * puts it into the wrapper provided. May fail - in which case
     * error is signalled on the field wrapper (use
     * cpix_Succeeded(..)).
     *
     * @param name the name of the field to create. Must not be NULL,
     * ownership not transferred.
     *
     * @param value the value of the field to create. Must not be NULL,
     * ownership not transferred.
     *
     * @param configs flags from cpix_Store, cpix_Index values.
     *
     * NOTE: An actual wrapper instance is not created by this
     * function, only the native instance it will wrap. This is
     * because the usecase of creating fields is that to add them to
     * Document, which transfers the ownership of the native instance
     * while not transferring the owernship of the wrapper. So, to
     * manage that situation comfortably, we do not force the creation
     * of the wrapper on the heap, only initialize it on the stack.
     */
    void cpix_Field_initialize(cpix_Field    * field,
                               const wchar_t * name,
                               const wchar_t * value,
                               int             configs);

    
    /**
     * Gets the name of this field. Never fails.
     *
     * @param thisField this field instance
     * @return the name of this field - ownership not transferred
     */
    const wchar_t * cpix_Field_name(cpix_Field * thisField);

    /**
     * Tells if this field has binary value. Never fails.
     *
     * @param thisFiels this field instance.
     * @return true if this field has binary value.
     */
    int cpix_Field_isBinary(cpix_Field * thisField);

    /**
     * Returns boost value for thisField
     * 
     * @param thisField this field instance.
     * @return boost value as a float_t.
     */
    float_t cpix_Field_boost(cpix_Field * thisField);

    /**
     * Set boost value for thisField
     * 
     * @param thisField this field instance to set boost for.
     * @param boost boost value as a float_t.
     */
    void cpix_Field_setBoost(cpix_Field * thisField,
                             float_t      boost);

    // TODO implement store, index, termvector related property
    // getters


    /**
     * Gets the string value of this field. Never fails.
     *
     * @param thisField this field instance
     * @return the string value of this field, if this field is not
     * binary, otherwise NULL. Ownership not transferred.
     */
    const wchar_t * cpix_Field_stringValue(cpix_Field * thisField);

    // TODO cpix_Field_streamValue() for getting content with a streamer


    /**
     * Releases the native instance wrapped by this wrapper but does
     * not free the wrapper.
     *
     * @param thisField this field (wrapper) to release
     */
    void cpix_Field_release(cpix_Field * thisField);



    /************************************************************************
     * Class cpix_DocFieldEnum to enumerate fields of a cpix_Document
     */
    struct cpix_DocFieldEnum_
    {
        // pointer to native (CLucene) impl
        void       * ptr_;
        
        // Last error, if any, that resulted from executing the
        // last operation on this cpix_XXX object
        // Use macros cpix_Failed, cpix_Succeeded and cpix_ClearError.
        // Do not attempt releasing it.
        cpix_Error * err_;
    };
    typedef cpix_DocFieldEnum_ cpix_DocFieldEnum;

    
    /**
     * Tells if there are more fields in this enumeration that were not
     * yet enumerated. Never fails.
     * @param thisDocFieldEnum this document-field enumeration instance
     *
     * @return true if there are yet un-enumerated fields, false (0)
     * otherwise
     */
    int cpix_DocFieldEnum_hasMore(cpix_DocFieldEnum * thisDocFieldEnum);

    /**
     * Gets the next field if this doc-field enum has more. Never fails.
     *
     * @param thisDocFieldEnum this doc-field enumeration
     *
     * @param target to put the field to. NOTE no new native instance
     * is created for the target, and therefore cpix_Field_destroy
     * must NOT be called on it. The instance 'target' is best to
     * have as a local variable (on the stack). See description of
     * lifetime management in the beginning of this header.
     */
    void cpix_DocFieldEnum_next(cpix_DocFieldEnum * thisDocFieldEnum,
                                cpix_Field        * target);



    /**
     * Destroys this document field enum instance. Never fails.
     *
     * @param thisDocFieldEnum this instance to destroy
     */
    void cpix_DocFieldEnum_destroy(cpix_DocFieldEnum * thisDocFieldEnum);



    struct cpix_Document_
    {
        // pointer to native (CPix) impl
        void       * ptr_;
        
        // Last error, if any, that resulted from executing the
        // last operation on this cpix_XXX object
        // Use macros cpix_Failed, cpix_Succeeded and cpix_ClearError.
        // Do not attempt releasing it.
        cpix_Error * err_;
    };
    typedef struct cpix_Document_ cpix_Document;

    
    /**
     * @param result any details on failure are communicated via this
     * struct - must not be NULL
     *
     * @param docUid document unique id - whatever string the document
     * can be identified with. Needed for deletion. It needs to be
     * unique only in the context of this index database. Mandatory
     * field. See enum LCPIX_DOCUID_FIELD. Stored and indexed
     * untokenized. If NULL is passed, exception is thrown.
     *
     * @param appClass the application class of the document. See
     * appclass-hierarchy.txt for more details. Optional field, can be
     * NULL, but recommended to define it. See enum
     * LCPIX_APPCLASS_FIELD. Stored and indexed tokenized. NOTE: this
     * value is supposed to be ASCII, so it is accepted as char*, but
     * the actual field is stored as wchar_t*, and that is how it is
     * returned on documents. If you want to be able to search
     * specific types in an index (like "root file media jpg" type
     * docs in the index "root file"), then you have to define this
     * field.
     *
     * @param excerpt a short textual extract of the document. It's up
     * to the client to figure out what short text can represent the
     * documents best when shown in search results. Optional
     * field. See enum LCPIX_EXCERPT_FIELD, as well as file parser
     * filter (LCPIX_FILEPARSER_FID). If defined: stored, not indexed.
     *
     * @param mimeType the mime type. Search clients can use it for
     * better displaying (like fetching corresponding icons, starting
     * viewing applications / components, etc...). Optional field, can
     * be NULL. See global variable LCPIX_MIMETYPE_FIELD. Stored if
     * specified, indexed untokenized.
     *
     * @return a newly created document instance. May fail, in which
     * case NULL is returned. Ownership transferred to caller.
     */
    cpix_Document * cpix_Document_create(cpix_Result   * result,
                                         const wchar_t * docUid,
                                         const char    * appClass,
                                         const wchar_t * excerpt,
                                         const wchar_t * mimeType);

    
    /**
     * Adds a field to this document.
     *
     * @param thisDocument this document to add a field to.
     *
     * @param field the field to add.
     *
     * NOTE!!! The ownership of the native field instance IS
     * transferred to this document. However, the ownership of the
     * wrapper (cpix_Field) is still at the caller. To work
     * comfortably with this situation, it has a cpix_Field_initialize()
     * function that initializes a wrapper instead of creating it - cf
     * above.
     */
    void cpix_Document_add(cpix_Document * thisDocument,
                           cpix_Field    * field);


    /**
     * Gets the value of a field in as a newly created string,
     * if the field exists in the document.
     *
     * @param thisDocument the document from which to get
     *        a field
     *
     * @param fieldName the name of the field, must not be NULL.
     *
     * @return the newly created string with the value of the
     *         field. If multiple fields exists with this name on the
     *         document, the first value is returned.  If only binary
     *         values exist or no value at all, NULL is returned. May
     *         fail. The value returned is owned by thisDocument,
     *         ownership is not transferred (do not delete it).
     */
    const wchar_t * 
    cpix_Document_getFieldValue(cpix_Document * thisDocument,
                                const wchar_t * fieldName);


    /**
     * Creates and returns with an enumeration of the fields.
     *
     * @param thisDocument this document of which the fields or are being
     * queried
     *
     * @return the newly created field enumeration. Ownership transferred
     * to the caller.
     */
    cpix_DocFieldEnum * cpix_Document_fields(cpix_Document * thisDocument);



    /**
     * Returns boost value for this Document
     * 
     * @param thisDocument this document instance.
     * @return boost value as a float_t.
     */
    float_t cpix_Document_boost(cpix_Document * thisDocument);



    /**
     * Set boost value for this Document.
     * 
     * @param thisDocument this document instance.
     * @param boost Boost value as a float_t.
     */
    void cpix_Document_setBoost(cpix_Document * thisDocument,
                                float_t         boost);
    
    /**
     * Empties out the document so that it can be reused. Always
     * succeeds. I.e. the document instance is not destroyed, only its
     * contents are emptied.
     *
     * @param thisDocument the document instance to clear
     */
    void cpix_Document_clear(cpix_Document * thisDocument);



    /**
     * Destroys this document instance. Never fails.
     *
     * @param thisDocument the document instance to destroy.
     */
    void cpix_Document_destroy(cpix_Document * thisDocument);



#ifdef __cplusplus
} /* extern "C" */
#endif /* __cplusplus */

#endif /* CPIX_CPIXDOC_H */
