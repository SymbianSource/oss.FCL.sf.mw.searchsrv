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

#ifndef CPIX_CPIXIDXDB_H
#define CPIX_CPIXIDXDB_H

#include <inttypes.h>

#include "cpixinit.h"
#include "cpixerror.h"
#include "cpixdoc.h"
#include "cpixanalyzer.h"
#include "cpixsearch.h"


/**
 * CPix API for searching.
 *
 * The CPix engine must be initialized before use and shut down
 * afterwards (see. cpixinit.h).
 *
 * Currently, this engine supports only single-threaded access.
 *
 * Short description of lifetime management
 *
 * All the cpix_XXX structs in here are wrappers over native (clucene)
 * classes. Some functions result in creation of new native instances,
 * some other functions merely return reference pointers to existing
 * ones that are owned by some other instance. While the wrappers must
 * be deallocated in both cases, the native instances wrapped by them
 * must be deallocated in one case and must not be deallocated in the
 * other. Obviously, there will have to be two different ways of
 * deallocating wrappers. To make things simple, and also to be able
 * to express intent of lifetime management in function signatures,
 * the following rules are adopted.
 *
 *   (a) Returning C-strings (char*, wchar_t*) will always be
 *       documented if the ownership of the returned string passes to
 *       the caller.
 *
 *   (b) Returning a pointer to a cpix_XXX will always indicate that
 *       the native type and its cpix_XXX wrapper are newly created
 *       instances. In this case, the caller must make sure to clean
 *       them up (both the native and the wrapper) with a call to
 *       cpix_XXX_destroy(...). DO NOT USE free() or delete on these
 *       pointers!
 *
 *   (c) Some functions will take a pointer to an cpix_XXX instance
 *       and fill it up with a pointer to a native instance. This
 *       always indicates that the native instance is not a newly
 *       created one. Therefore, deallocation of such wrappers are not
 *       needed, and cpix_XXX_destroy most not be called. The most
 *       comfortable way to use these functions is to have the
 *       cpix_XXX wrapper instance as a local variable of a function
 *       (i.e. on the stack), and when the variable's lifetime is over
 *       everything will be fine. Of course, a client may create
 *       cpix_XXX instance on the heap too, in which case, it is the
 *       callers responsibility to deallocate the dynamically
 *       allocated cpix_XXX instance with free() or delete (depending
 *       on how it was allocated).
 *
 *
 * CPIX VOCABULARY
 *
 * Application class or "appclass", for short, is simply a string
 * representiation of a type of a document in an index. It has an
 * internal structure expressing hierarchy of those types. The most
 * generic type is "root", and all documents representing files are of
 * the type "root file" or a descendant type of "root file", like
 * "root file usrdoc text". See appclass-hierarchy.txt.
 *
 * Base application class or "baseappclass" is an appclass, that an
 * application / application domain deems significant enough to assign
 * its own application index to it. Like files are stored in an index
 * identified by base app class "root file". It means that ALL
 * documents that go to that index MUST be of the type "root file" or
 * some of its descendant, like "root file usrdoc text".
 *
 * Qualified base app class: an application index, that is a whole
 * conceptually, can be broken down to several physical index
 * databases, called volumes. The usual reason is performance: if you
 * have 10 thousand documents that you don't ever need to update, but
 * there are tens of documents that change frequently, you can
 * organize your application index into two volumes, and only one of
 * them is frequently updated. Such physical index needs sometimes
 * explicit addressing, and this is achieved by qualifying the base
 * app class with a volume id. For instancem, files harvested from the
 * e drive may be stored in a volume identified by qualified base app
 * class "@d:root file", where the volume identifier, 'd', is in the
 * beginning of the string.
 *
 * Unqualified base app class: the same as a (plain) base app class.
 *
 * Domain selector: a non empty, comma-separated list of
 * appclasses. Like "@d:root file,root msg phone sms" domain selector,
 * when gicen to index searcher constructor, will search those two
 * indexes and nothing else.
 *
 */


#ifdef __cplusplus
extern "C" {
#endif

    
    /**
     * A type storing internal state for the excerpt processing
     * utility functions like cpix_getExcerptOfWText and
     * cpix_getExcerptOfText.
     *
     * Must be initialized by a call to cpix_init_EPIState().
     */
    typedef int  cpix_EPIState;

    
    /**
     * Initializes an excerpt processor internal state instance.
     */
    void cpix_init_EPIState(cpix_EPIState * state);



    /**
     * A simple utility function getting the first couple of words of
     * a text by compressing adjacent blank characters. Has a
     * signature that is geared toward processing text streams.
     *
     * @param dst the destination buffer to copy to, must not be NULL
     *
     * @param src the original text to get the first couple of words
     * of, must not be NULL
     *
     * @param maxWords pointer to the the maximum number of first
     * words to get. Must not be NULL, and it is updated: it is
     * decreased with the number of words read.
     *
     * @param bufSize pointer to the size of dst buffer. Must not be
     * NULL, and it is updated: it is decreased with the number of
     * characters written to dst (excluding the terminating zero).
     *
     * @param state as this function is meant for stream processing,
     * some state may have to be preserved accross function
     * calls. Must not be NULL. Before invoking this function for a
     * text stream for a first time, it must be initialized by a call
     * to cpix_init_EPIState.
     *
     * @returns the pointer to the terminating zero at the end of the
     * string in dst.
     */
    wchar_t * cpix_getExcerptOfWText(wchar_t       * dst,
                                     const wchar_t * src,
                                     size_t        * maxWords,
                                     size_t        * bufSize,
                                     cpix_EPIState * state);


    /**
     * The very same semantics as that of cpix_getExcerptOfWText,
     * except the source string here is not a wide string.
     */
    wchar_t * cpix_getExcerptOfText(wchar_t       * dst,
                                    const char    * src,
                                    size_t        * maxWords,
                                    size_t        * bufSize,
                                    cpix_EPIState * state);



    /**
     * A field descriptor struct for schema registration purposes. To
     * be able to index some content with cpix_IdxDb, one must define
     * document schemas for it first. Fields of documents to be
     * indexed are described by this cpix_FieldDesc.
     */
    struct cpix_FieldDesc
    {
        /**
         * Name of the field.
         */
        const wchar_t           * name_;

        /**
         * Use the cpix_Store and cpix_Index flags in here (from
         * cpixdoc.h).
         */
        int                       cfg_;
    };


    /************************************************************************
     * Class cpix_Term
     *
     * NOTE TO INTERNAL DEVELOPMENT: Native instances of this class
     * MUST be specially freed using _CLDECDELETE.
     */
    struct cpix_Term_
    {
        // pointer to native (CLucene) impl
        void       * ptr_;
        
        // Last error, if any, that resulted from executing the
        // last operation on this cpix_XXX object
        // Use macros cpix_Failed, cpix_Succeeded and cpix_ClearError.
        // Do not attempt releasing it.
        cpix_Error * err_;
    };
    typedef cpix_Term_ cpix_Term;

    
    /**
     * Constructs a term with a field name and field value.
     *
     * @param fieldName the name of the field this term is about
     *
     * @param fieldValue the textual value of the field this term
     *        is about
     *
     * @return newly created Term object the ownership of which
     * is transferred to the caller. May fail, in which case NULL
     * is returned.
     */
    cpix_Term * cpix_Term_create(cpix_Result     * result,
                                 const wchar_t   * fieldName,
                                 const wchar_t   * fieldValue);



    /**
     * Destroys this term instance, never fails.
     */
    void cpix_Term_destroy(cpix_Term * thisTerm);


    /**
     * cpix_IdxDb
     */
    struct cpix_IdxDb_
    {
        // instead of a pointer to native (CLucene) impl, there is a
        // handle here only. The type here must be equivalent to 
        IdxDbHndl    handle_;
        
        // Last error, if any, that resulted from executing the
        // last operation on this cpix_XXX object
        // Use macros cpix_Failed, cpix_Succeeded and cpix_ClearError.
        // Do not attempt releasing it.
        cpix_Error * err_;
    };
    typedef cpix_IdxDb_ cpix_IdxDb;
    
    
    enum cpix_IDX_OpenMode_
        {
            cpix_IDX_OPEN   = 0,
            cpix_IDX_CREATE = 1
        };
    typedef cpix_IDX_OpenMode_ cpix_IDX_OpenMode;


    /**
     * Returns a cpix_IdxDb instance through which a client can search
     * of modify the index database responsible for a given qualified
     * base app class. The qualified base app class must have an index
     * path associated to it (via defineVolume call).
     *
     * @param result pointer through which error details are
     * communicated back
     *
     * @param qualBaseAppClass the qualified base application class,
     * that is the latest common appliciaton super class for all
     * documents that will be put into this index. See
     * appclass-hierarchy.txt for details. Index databases MUST have
     * unique qualified baseappclass values.
     *
     * @param openMode whether to open the an existing index database
     * or (re-)create it even if it exists.
     *
     * @retur pointer to the cpix_IdxDb instance that can be used for
     * searching and modifying the index.
     */
    cpix_IdxDb * cpix_IdxDb_openDb(cpix_Result       * result,
                                   const char        * qualBaseAppClass,
                                   cpix_IDX_OpenMode   openMode);


    /**
     * Releases the native index database manipulator (from the caller
     * client). (Actually, it only decreases its reference count, as
     * index database manipulator is kept around for performance as
     * long as it is needed.) It also frees up the wrapper.
     */
    void cpix_IdxDb_releaseDb(cpix_IdxDb * thisIdxDb);
    

    /**
     * Searches the index.
     *
     * NOTE: Keep the original query parser and query instances alive
     * as long as you use the result of the search - in general, they
     * may be needed.
     *
     * @param thisIdxDb the index db instance that should
     *        perform the search, must not be NULL.
     *
     * @param query the query itself in a structured form, must not be
     *        NULL, its ownership does not change.
     *
     * @return the hits instance with the search results, that will be
     * owned by the calling client.
     */
    cpix_Hits * 
    cpix_IdxDb_search(cpix_IdxDb * thisIdxDb,
                      cpix_Query * query);
    
    
    typedef size_t SchemaId; // TODO rename -> cpix_SchemaId

    /**
     * Adds a schema for efficient (bulk) addition or update. Instead
     * of communicating the schema implicitly every time a document is
     * created and populated with fields, we can define the field
     * names and their configurations only once, and use that later.
     *
     * @param thisIdxDb the index db instance to register the schema
     * to.
     *
     * @param fieldDescs array of field descriptors
     *
     * @param count the number of field descriptors in the array
     * fieldDescs.
     *
     * @return an opaque ID for the schema. Do not persist this
     * value. Currently it is guaranteed to be valid only between a
     * cpix_IdxDb_openDb call and a cpix_IdxDb_releaseDb call. The
     * schema ID is not valid for other cpix_IdxDb instances.
     */
    SchemaId cpix_IdxDb_addSchema(cpix_IdxDb           * thisIdxDb,
                                  const cpix_FieldDesc * fieldDescs,
                                  size_t                 count);


    /**
     * Adds a document to this index.
     *
     * @param thidIdxDb the index to add the document to
     *
     * @param document the document with all the mandatory, optional
     * and custom fields populated
     *
     * @param analyzer - MUST NOT BE NULL.
     */
    void cpix_IdxDb_add(cpix_IdxDb      * thisIdxDb,
                        cpix_Document   * document,
                        cpix_Analyzer   * analyzer);

    /**
     * Adds a document (=set of fields) to the index.
     *
     * @param thisIdxDb the index to add to
     *
     * @param schemaId identifies the schema to use
     *
     * @param docUid document unique id - whatever string the document
     * can be identified with. Needed for deletion. It needs to be
     * unique only in the context of this index database. Mandatory
     * field. See global variable cpix_DOCUID_FIELD
     * (cpixdox.h). Stored and indexed untokenized.
     *
     * @param appClass the application class of the document. See
     * appclass-hierarchy.txt for more details. Mandatory field. See
     * global variable cpix_APPCLASS_FIELD (cpixdox.h). Stored and
     * indexed untokenized. NOTE: this value is supposed to be ASCII,
     * so it is accepted as char*, but the actual field is stored as
     * wchar_t*, and that is how it is returned on documents.
     *
     * @param excerpt a short textual extract of the document. It's up
     * to the client to figure out what short text can represent the
     * documents best when shown in search results. Mandatory
     * field. See global variable cpix_EXCERPT_FIELD
     * (cpixdoc.h). Stored, not indexed.
     *
     * @param mimeType contains document's mime-type to be identified
     * with the application, which would be able to view or edit this
     * document.  For example, Messaging Application is able to view
     * SMS messages. Application handling the data is chosen by the
     * system based on the given mime-type.  Optional field, can be
     * NULL. See global variable cpix_MIMETYPE_FIELD
     * (cpixdoc.h). Stored and indexed not tokenized.
     *
     * @param fieldValues the array with the field values. The length
     * of the array must be exactly the same as the length of the
     * corresponding schema, identified by schemaId. However,
     * individual pointers in the array may be NULL, indicating that
     * the corresponding field will not have any value. Field values
     * are delt with according to the schema given: field names, and
     * configurations (indexed, stored, ...) are taken from the
     * schema.
     *
     * A special use case is when the field value is not an actual
     * field value but a file system path. In this case the
     * corresponding cpix_FieldDesc defines the chain of filters that
     * needs to be applied to obtain the actual textual content for
     * the field. See cpix_FieldDesc.
     *
     * @param analyzer the analyzer to use for this document. Must NOT
     * BE NULL (currently there is no default analyzer defined for an
     * index database).
     *
     * NOTE: after a lot of additions, it is worth calling
     * cpix_IdxDb_optimize(), otherwise the search may be (much)
     * slower.
     *
     */
    void cpix_IdxDb_add2(cpix_IdxDb      * thisIdxDb,
                         SchemaId          schemaId,
                         const wchar_t   * docUid,
                         const char      * appClass,
                         const wchar_t   * excerpt,
                         const wchar_t   * mimeType,
                         const wchar_t  ** fieldValues,
                         cpix_Analyzer   * analyzer);
    

    /**
     * Deletes the document from the index using the document ID value
     * the system field cpix_DOCUID_FIELD. To delete based on an
     * arbitrary field, use deleteDocuments2().
     *
     * @param thidIdxDb the index database to delete from
     *
     * @param docUid the unique id of the document to delete
     *
     * @return the number of documents deleted
     */
    int32_t cpix_IdxDb_deleteDocuments(cpix_IdxDb    * thisIdxDb,
                                       const wchar_t * docUid);

    /**
     * Deletes documents based on an arbitrary field.
     *
     * @param thidIdxDb the index database to delete from
     *
     * @param term the field name/value pair to match documents for
     * deletion
     *
     * @return the number of documents deleted
     */
    int32_t cpix_IdxDb_deleteDocuments2(cpix_IdxDb    * thisIdxDb,
                                        cpix_Term     * term);


    /**
     * Updates a document. Conceptually, it is equivalent to deleting
     * the document if existed and re-adding it.
     *
     * @param thidIdxDb the index to add the document to
     *
     * @param document the document with all the mandatory, optional
     * and custom fields populated
     *
     * @param analyzer - MUST NOT BE NULL.
     */
    void cpix_IdxDb_update(cpix_IdxDb      * thisIdxDb,
                           cpix_Document   * document,
                           cpix_Analyzer   * analyzer);

    /**
     * Updates a document (=set of fields) in the index. Conceptually,
     * it is equivalent to deleting the document if existed and
     * re-adding it.
     *
     * For the parameters, see the comments of cpix_IdxDb_add().
     */
    void cpix_IdxDb_update2(cpix_IdxDb      * thisIdxDb,
                            SchemaId          schemaId,
                            const wchar_t   * docUid,
                            const char      * appClass,
                            const wchar_t   * excerpt,
                            const wchar_t   * mimeType,
                            const wchar_t  ** fieldValues,
                            cpix_Analyzer   * analyzer);


    /**
     * Sets the physical maximum amount of the RAM buffer, where the
     * indexed documents are saved before flushing them into the
     * actual index databse. Having higher value will increase memory
     * consumption, but will speed up indexing operations.
     *
     * @param thisIdxDb the index database to set the maximum index
     * size for
     *
     * @param value The maximum number of bytes RAM buffer can hold
     * before flushing
     *
     * NOTE: see cpixinit.h, cpix_InitParams_setMaxInsertBufSize()
     * - that call sets the general, default buffer size, while this
     * one sets the same value only on thisIdxDb.
     */
    void cpix_IdxDb_setMaxInsertBufSize(cpix_IdxDb * thisIdxDb,
                                        size_t       value);



    /**
     * Forces this index database to write its state (including the
     * insert buffer's state) to the disk.
     *
     * Do NOT use this operation lightly, as it is a costly
     * operation. In fact, there is only one (more-or-less) justified
     * use case for this call: when a harvester at the end of its
     * harvesting session, if it wants to make sure of quick update
     * before the regular cpix_doHousekeep() is ran.
     *
     * @param thisIdxDb the index database to flush to the hard drive.
     */
    void cpix_IdxDb_flush(cpix_IdxDb * thisIdxDb);

    

    /**
     * Scraps all information wrt what base classes are and what index
     * databases there are, but only if there are no index db handlers
     * in use. Meant to be used only for unit testing. May fail.
     *
     * @param result pointer through which error details are
     * communicated back
     *
     */
    void cpix_IdxDb_dbgScrapAll(cpix_Result * result);
    
    
    
    /**
     * Defines a base app class - path association. If the path
     * already holds the index, then it can be readily used by an
     * cpix_IdxDb_openDb call (so it can have mounting semantics).
     *
     * TODO this signature should evolve to take a human readable nice
     * display name for the volume. Actually, a human readable name
     * for the application domain as well as a human readable name for
     * the volume itself.
     *
     * @param result since it may fail, errors are reported through
     * this argument
     *
     * @param qualBaseAppClass the qualified base app class that will
     * be used to access the volume being defined now. Must NOT be
     * NULL. The same qualified base app class must not already be
     * associated to another path (redefinition to the same path is
     * allowed (= NOP)).
     *
     * @param path the path to the index volume to use. CPix will
     * access the index from under here. If the directory and/or
     * usable index does not exist, they will be created (empty
     * state). If the index exist but is corrupt, it will be
     * re-created. If the index exists and is usable, it will be kept
     * for use. NOTE: later calls to cpix_IdxDb_openDb with creation
     * mode are possible, discarding whatever state the existing index
     * db was in.
     */
    void cpix_IdxDb_defineVolume(cpix_Result * result,
                                 const char  * qualBaseAppClass,
                                 const char  * path);

    
    /**
     * Undefines a volume (does not delete it, only forgets about it).
     * If there was no such volume, nothing is done.
     */
    void cpix_IdxDb_undefineVolume(const char * qualBaseAppClass);


#ifdef __cplusplus
} /* extern "C" */
#endif /* __cplusplus */


#endif
