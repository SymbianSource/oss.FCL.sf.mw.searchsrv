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

#ifndef CPIX_IDXDB_H
#define CPIX_IDXDB_H

#include <wchar.h>
#include <stdint.h>

#include <string>
#include <vector>

#include "cpixsynctools.h"

#include "fwdtypes.h"
#include "iidxdb.h"
#include "ireaderowner.h"
#include "cpixidxdb.h"
#include "idxdbdelta.h"


namespace Cpix
{


    /**
     * Class for insert buffers. An insert buffer is a temporary index
     * kept in RAM until it gets committed to disk.
     *
     * Basically a ram directory and a writer that can pour stuff into
     * it. It also has a set for docuids in the insert buffer, so that
     * deletions can happen on insert buffer items too.
     */
    class InsertBuf : public Impl::IInsertBuf
    {
    private:
        //
        // private members
        //

        /**
         * User to give maximize the number of docs at which the
         * writer_ starts optimizing. It is true, that the writer_ is
         * writing to a ramDir_, but still, too small maxDocs_ may
         * mean too much extra processing (merging), while too big one
         * may increase peak RAM consumption.
         */
        size_t                                     maxDocs_;


        /**
         * The IndexWriter instance used to put documents to the index
         * stored in the ramDir_. May be NULL some times.
         */
        lucene::index::IndexWriter               * writer_;

        /**
         * The ram directory used to store the index.
         */
        lucene::store::TransactionalRAMDirectory * ramDir_;

    public:
        //
        // public operators
        //

        //
        // from interface Cpix::Impl::IInsertBuf
        //

        /**
         * Constructs an insert buffer.
         *
         * @param maxDocs is an internal setting for the writer
         * (setBufferMaxDocs()) used to specify how frequently the
         * writer should start fussing around with merging the index
         * stored in the ram directory.
         */
        InsertBuf(size_t maxDocs);


        /**
         * Closes all members and destroys them (if pointer). Cleans
         * completely insert buffer state (doc uids are also
         * emptied). Not thread safe.
         */
        virtual void close();


        /**
         * @return true if this insert buffer is empty
         *
         * Not thread safe.
         */
        virtual bool isEmpty() const;


        /**
         * @return the current ram directory, if any. If there is no
         * ram dir instance, it throws. The internal writer_ member is
         * closed at the same time.
         *
         * Not thread-safe.
         */
        virtual lucene::store::TransactionalRAMDirectory * getRAMDir();


        /**
         * Practically closes() end destructs.
         */
        virtual ~InsertBuf();


        //
        // Own, cpix logic-related functionality
        //
        
        /**
         * Adds a new document to this insert buffer.
         */
        void add(Cpix::Document             * doc,
                 lucene::analysis::Analyzer * analyzer);

        /**
         * Deletes document(s) from the insert buffer (if it is not
         * empty).
         */
        int32_t deleteDocuments(lucene::index::Term * term);

        
        /**
         * @return the byte size of the current ram directory.
         */
        size_t size();


    private:
        //
        // private implementation details
        //
        
        /**
         * For lazy initialization.
         */
        void lazyInit();


        /**
         * Deletes the writer
         */
        void getRidOfWriter();

    };



    /**
     * A class that most importantly holds and manages an IndexReader
     * instance. It also manages the version of the index (version
     * changes whenever the reader is closed, because then previous
     * hits are invalid). Also, it manages the IndexSearcher instance
     * that depends on the IndexReader instance.
     *
     * This class is a very integral part of IdxDb, so there is a
     * high level of coupling between the two, namely, its operations
     * take some mutex or a locker facility (Cpt::MultiSyncRegion)
     * arguments many times.
     *
     * This instance is the only object that decides in what order a
     * directory-mutex and its own mutex protecting the
     * readers/searcher/version number should be locked.
     */
    class VersionedReader : public Cpix::Impl::IIdxReader
    {
    private:
        //
        // private members
        //
        /**
         * The IndexReader instance: may be NULL (closed state) or
         * non-NULL (open state).
         */
        lucene::index::IndexReader          * reader_;

        /**
         * The version number of the reader_ instance, whenever the
         * reader_ is closed, this version number must be updated to
         * force all dependent clients to re-fetch.
         */
        Version                               version_;

        /**
         * The IndexSearcher instance. May be NULL or may be
         * non-NULL. It can only be non-NULL is reader_ is not NULL.
         */
        lucene::search::IndexSearcher       * searcher_;

        /**
         * The mutex protecting all the other members: reader_,
         * version_ and searcher_.
         */
        Cpt::Mutex                            mutex_;


        /**
         * Number of deletions on the current reader_ instance that is
         * still uncommitted to disk.
         */
        int32_t                               deletionsCount_;


    public:
        //
        // public operators
        //

        //
        // from interface Cpix::Impl::IIdxReader
        //

        /**
         * See comments for Cpix::Impl::IIdxReader.
         *
         * Lock order: this - dirMutex.
         *
         * NOTE: low level interface method used by the interface
         * client (delta commit logic), so IdxDb should not be
         * calling it directly. Use load() instead.
         */
        virtual void open(const char * clIdxPath,
                          Cpt::Mutex & dirMutex);

        /**
         * See comments for Cpix::Impl::IIdxReader.
         *
         * Lock order: this - dirMutex.
         *
         * NOTE: low level interface method used by the interface
         * client (delta commit logic), so IdxDb should not be
         * calling it directly. Use load() instead.
         */
        virtual void reopen(Cpt::Mutex & dirMutex,
                            const char * clIdxPath,
                            bool         reRead);        

        /**
         * See comments for Cpix::Impl::IIdxReader.
         *
         * Lock order: this - dirMutex.
         */
        virtual bool commitIfNecessary(Cpt::Mutex & dirMutex);

        
        //
        // lifetime mgmt
        //

        /**
         * Constructs
         */
        VersionedReader();

        /**
         * Destructs (close()-es).
         */
        virtual ~VersionedReader();

        
        //
        // Own stuff
        //


        /**
         * Closes this instance. Lock order: this - dirMutex. dirMutex
         * may be NULL.
         *
         * @param dirMutex mutex - the mutex to lock in order to gain
         * access to the cpix directory in which our clucene directory is
         *
         * @param cpixDir if it is given, it will delete all content
         * in that cpix directory, and will recreate clucene dir with
         * an empty index. If it is NULL, it does not recreate the
         * index, it just closes it.
         */
        void close(Cpt::Mutex * dirMutex,
                   const char * cpixDir = NULL);


        /**
         * If msr is given, it locks the mutex of this instance (and
         * have it locked).
         *
         * @return true if it is open
         */
        bool isOpen(Cpt::MultiSyncRegion * msr);


        /**
         * @return if it has currently pending deletions that were not
         * yet committed to disk. Does not need locking.
         */
        bool hasDeletions() const;
        

        /**
         * Gets the current version, uses msr to lock this and keep it
         * locked. (Client must get a guarantee that the version is
         * not changing until the lock is released).
         *
         * @param msr the multi sync region collecting the lock on the
         * mutex of this instance.
         */
        Version getVersion(Cpt::MultiSyncRegion & msr);


        /**
         * Gets the reader if open, otherwise throws. Not thread-safe,
         * the mutex of this must be locked.
         */
        lucene::index::IndexReader * getReader(Version * version);


        /**
         * Perform search, updates version number. Not thread-safe,
         * this instance must be locked at this point. Must be in
         * open()-ed state.
         *
         * @param query the query to perform
         *
         * @param version the current version of this versioned reader
         * is put here so that the client can store it.
         */
        lucene::search::Hits * search(lucene::search::Query * query,
                                      Version               * version);


        /**
         * Performs the delete. Not thread-safe, this instance must be
         * locked at this point. Must be in open()-ed state.
         */
        int32_t deleteDocuments(lucene::index::Term * term);


        /**
         * Re-loads the reader from somewhere under the CPix index
         * path.
         *
         * NOTE: Not thread safe, the this.mutex_ MUST BE locked
         * somehow by the time this function is called. Therefore,
         * locking order: this.mutex_, dirMutex_.
         *
         * @param cpixIdxPath the path to the cpix index directory
         *
         * @param dirMutex the mutex to protect concurrent accesses to
         * the cpix index directory
         */
        void load(const char           * cpixIdxPath,
                  Cpt::Mutex           & dirMutex);


    private:
        //
        // private implementation details
        //
        /**
         * Closes and destroys the member searcher_, if not NULL.
         */
        void destroySearcher();

    };



    /**
     * This class represents an index database thru which all operations
     * (search, add, delete, update, flush) will be managed. Sibling
     * (improved version of IdxDb).
     */
    class IdxDb : public IIdxDb, public IReaderOwner
    {
    private:
        //
        // private members
        //
        struct FieldDesc
        {
            std::wstring                       name_;
            int                                cfg_;
            
            FieldDesc(const cpix_FieldDesc * fieldDesc);
            FieldDesc();
        };
        
  
        typedef std::vector<FieldDesc>  SchemaDesc;

        /**
         * Path to the "Cpix Index Directory". A Cpix index directory
         * is not the same as a "Clucene index directory", although it
         * contains one (or in certain cases two). It can also contain
         * other files.
         */
        std::string                     indexDbPath_;
        
        VersionedReader                 reader_;

        InsertBuf                       insertBuf_;


        std::vector<SchemaDesc>         schemas_;

        // see cpixinit.h for explanation
        size_t                          maxInsertBufSize_;
        // see cpixinit.h for explanation
        size_t                          insertBufMaxDocs_;


	// protects insertBuf_ and schema_ members, and since
	// IdxDb(2) instances correspond to physical directories
	// (through IdxDbMgr's), it also happens to protect the
	// physical directories from concurrent mess
	Cpt::Mutex                      idxMutex_;

        // only to prevent value semantics
        IdxDb(const IdxDb &);
        // only to prevent value semantics
        IdxDb & operator=(const IdxDb &);


    public:
	//
	// Functions from interface IIdxDb
	//
	

        /**
         * Performs a search on this index. Thread-safe.
         */
        lucene::search::Hits * search(lucene::search::Query * query,
                                      Version               * version);

        
        /**
         * Performs a fetch/recommit operation. Thread-safe.
         */
        virtual lucene::search::Hits * 
        fetchRecommitting(lucene::search::Hits        * currentHits,
                          Version                     * version,
                          lucene::search::Query       * query,
                          DocumentConsumer            & documentConsumer);

        /**
         * Returns a hits object containing a ranked list of documents
         * representing matching terms. The terms are ranked based on
         * the document frequence and the term amount is bounded from
         * the top by maxHits. Only the terms of given field and
         * matching the wilcard are returned. For example requesting
         * terms for DOCUID field will likely return cryptic
         * identifier codes, while requesting terms for SMS
         * content field will return words that are most often used in
         * SMS messages, and requesting contact name field will return
         * common names.
         * 
         * The returned documents contain the term text in the field
         * named "term" and the document frequence in the field
         * "docfreq".
         *
         * Thread-safe.
         * 
         * @param field The field of the term.
         *
         * @param wilcard Returned terms are matched against this
         * wildcard . MUST contain '*' or '?' symbols.
         *
         * @returns Ranked list of terms
         */
        IHits * getTerms(const wchar_t * field, 
                         const wchar_t * wildcard, 
                         const wchar_t * appclassPrefix,
                         int             maxHits);

        /**
         * Adds a schema to this index. After this, schema based
         * addition (add2) and update (update2) can be
         * used. Thread-safe.
         */
        SchemaId addSchema(const cpix_FieldDesc * fieldDescs,
                           size_t                 count);


        /**
         * Adds a document to this index (may not be reflected on disk
         * right away).
         *
         * Thread-safe.
         */
        void add(Cpix::Document             * doc,
                 lucene::analysis::Analyzer * analyzer);

        /**
         * Adds a document to this index (may not be reflected on disk
         * right away). Thread-safe.
         *
         * @param analyzer - the analyzer MUST NOT BE NULL, as there is
         *  no default one used (currently)
         */
        void add2(SchemaId                      schemaId,
                  const wchar_t               * docUid,
                  const char                  * appClass,
                  const wchar_t               * excerpt,
                  const wchar_t               * mimeType,
                  const wchar_t              ** fieldValues,
                  lucene::analysis::Analyzer  * analyzer);

        /**
         * Deletes (all) documents with the given docUid. Uniqueness
         * of documents by their docUid is up to the client, so
         * technically it is possibly that 0, 1 or more documents are
         * deleted by this operation.
         *
         * Thread-safe.
         *
         * @return number of documents deleted
         */
        int32_t deleteDocuments(const wchar_t  * docUid);


        /**
         * Deletes a number of documents based on a term (field name /
         * field value pair). Any document matching the given "term
         * query" will be deleted.
         *
         * Thread-safe.
         *
         * @return number of documents deleted
         */
        int32_t deleteDocuments2(lucene::index::Term * term);


        /**
         * Updates a document (deletes and adds it).
         *
         * Thread-safe.
         */
        void update(Cpix::Document             * doc,
                    lucene::analysis::Analyzer * analyzer);

        /**
         * TODO FIX current implementation is abysmally pessimal
         *
         * Thread-safe.
         *
         * @parami analyzer - the analyzer MUST NOT BE NULL, as there is
         *  no default one used (currently)
         */
        void update2(SchemaId                     schemaId,
                     const wchar_t              * docUid,
                     const char                 * appClass,
                     const wchar_t              * excerpt,
                     const wchar_t              * mimeType,
                     const wchar_t             ** fieldValues,
                     lucene::analysis::Analyzer * analyzer);


        /**
         * Sets the flushing treshold for buffer. Buffer is flushed
         * when the internal buffer's size increase over given threshold. 
         */
        void setMaxInsertBufSize(size_t value);


        /**
         * Thread-safe.
         */
        void flush();


        /**
         * Properly closes this index (commits changes to disk). Thread-safe.
         */
        void close();

        
        /**
         * Deletes every index manipulator, with no considerations
         * given to flushing etc ... Thread-safe.
         */
        void brutalClose() throw ();


	/**
	 * If there is a delta (deletions and/or insert buffer), it is
	 * committed to the disk. Supposed to be called regularly.
	 */
	void doHousekeeping();

        
        /**
         *
         * Sets the refcount to 1.
         */
        IdxDb(const char  * indexDbPath,
	       InitParams  & ip);


        /**
         * Dumps the state of this instance (debug
         * feature). Thread-safe.
         */
        virtual void dbgDumpState();


        //
        // from interface IReaderOwner
        //
        Version getVersion(Cpt::MultiSyncRegion & msr);
        lucene::index::IndexReader* getReader(Cpt::MultiSyncRegion * msr,
                                              Version              * version);


        /**
         * Recreates the index (makes it empty). Thread-safe.
         */
        virtual void recreateIdx();


    private:
        //
        // private methods
        //
        /**
         * Destructor. It will not commit changes to the disk (ie
         * equivalent to a brutalClose()), for a properly committing
         * changes to the disk on must call close() on this before
         * destroying.
         */
        virtual ~IdxDb();

        
        /**
         * Makes sure reader_ is open. The QRY / reader_ mutex must be
         * locked and also the idxMutex_ is going to be locked - the
         * order is: QRY - IDX.
         */
        void loadReader(Cpt::MultiSyncRegion * msr);
        

        /**
         * Should create a searcher instance if there is none (recover
         * reader, etc). Should set the new version number. Thread
         * safe (locks reader_'s mutex) using msr.
         */
        lucene::search::Hits * doSearch(lucene::search::Query * query,
                                        Version               * version,
                                        Cpt::MultiSyncRegion  * msr);



        /**
         * Closes everything without flushing. Can optionally recreate
         * the index, empty-ing it.
         *
         * @param cpixDir if it is given, it will destroy all contents
         * in that cpix directory, and recreate (a clucene dir with)
         * an empty index. May be NULL.
         */
        void forceClose(const char * cpixDir);



    };


}

#endif

