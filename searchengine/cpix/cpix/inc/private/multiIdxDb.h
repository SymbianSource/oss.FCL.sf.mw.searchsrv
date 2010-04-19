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

#ifndef CPIX_MULTIIDXDB_H
#define CPIX_MULTIIDXDB_H

#include <wchar.h>
#include <stdint.h>

#include <string>
#include <vector>

#include "cpixsynctools.h"

#include "iidxdb.h"
#include "cpixidxdb.h"

namespace Cpix
{
    
    class IReaderOwner;

    /**
     * Stores information about an originator (IIdxDb / IReaderOwner)
     * instance.
     *
     * NOTE: ASSUMPTION THAT MUST NOT BE BROKEN: IdxDbMgr must NOT
     * delete an IIdxDb/IReaderOwner instance until it is in use by
     * MultiIdxDb instances.
     */
    struct OriginatorInfo
    {
    private:
        //
        // private members
        //
        Version              version_;
        IReaderOwner       * readerOwner_;


    public:
        //
        // public operators
        //

        /**
         * Default constructor for std::map value-type purposes
         */
        OriginatorInfo();
              
        
        /**
         * Constructs an originator info with the pointer to the
         * IIdxDb/IReaderOwner instance.
         */
        OriginatorInfo(IIdxDb * idxDb);


        /**
         * Setter / getter for version property
         */
        Version version() const;
        void setVersion(Version version);


        /**
         * Provides the originator IIdxDb/IReaderOwner instance.
         */
        IReaderOwner & readerOwner() const;
    };



    /**
     * This class represents an index database thru which all
     * operations (search, add, delete, update, flush) will be
     * managed.
     */
    class MultiIdxDb : public IIdxDb 
    {

    private:

        int                                        refCount_;

        typedef std::map<IdxDbHndl, OriginatorInfo> OriginatorMap;
        OriginatorMap                              originators_;

        std::string                                domainSelector_;

        std::vector<lucene::search::Searchable*>   searchers_;
        lucene::search::MultiSearcher            * searcher_;

        Cpt::Mutex                                 mutex_;

        Version                                    version_;

        // only to prevent value semantics
        MultiIdxDb(const MultiIdxDb &);
        // only to prevent value semantics
        MultiIdxDb & operator=(const MultiIdxDb &);

    public:


        //
        // Functions from IIdxDb
        //

        /**
         * Search using the MultiSearcher. Thread-safe
         */
        virtual lucene::search::Hits * search(lucene::search::Query * query,
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
         * Get terms using the MultiSearcher. Thread-safe.
         */
        virtual IHits * getTerms(const wchar_t * field, 
                                 const wchar_t * wildcard, 
                                 const wchar_t * appclassPrefix, 
                                 int             maxHits);


        // all these other methods just follow technically from
        // IIdxDb, but they will never be called, therefore will have
        // a dummy (exception throwing) implementation
        SchemaId addSchema(const cpix_FieldDesc * fieldDescs,
                           size_t                 count);
        void add(Document * doc,
                 lucene::analysis::Analyzer * analyzer);
        void add2(SchemaId                      schemaId,
                  const wchar_t               * docUid,
                  const char                  * appClass,
                  const wchar_t               * excerpt,
                  const wchar_t               * mimeType,
                  const wchar_t              ** fieldValues,
                  lucene::analysis::Analyzer  * analyzer);
        int32_t deleteDocuments(const wchar_t  * docUid);
        int32_t deleteDocuments2(lucene::index::Term * term);
        void update(Document * doc,
                    lucene::analysis::Analyzer * analyzer);
        void update2(SchemaId                     schemaId,
                     const wchar_t              * docUid,
                     const char                 * appClass,
                     const wchar_t              * excerpt,
                     const wchar_t              * mimeType,
                     const wchar_t             ** fieldValues,
                     lucene::analysis::Analyzer * analyzer);

        void setMaxInsertBufSize(size_t value);
        void flush();

        // These operations also come from interface IIdxDb, but and
        // they do have meaningful implementation
        /**
         * Thread-safe.
         */
        void close();

        /**
         * Thread-safe.
         */
        void brutalClose() throw ();


        /**
         * Thread-safe.
         */
        virtual void doHousekeeping();


        /**
         * Thread-safe.
         */
        virtual void dbgDumpState();

    private:
        //
        // private methods - exposed to friends only
        //
        friend class IdxDbMgr;

        /**
         * Private constructor.
         *
         * Sets the refcount to 1.
         */
        MultiIdxDb(std::set<IdxDbHndl> & idxDbHndls,
                   const char          * domainSelector);

        
        /**
         * Destructor.
         */
        ~MultiIdxDb();


        /**
         * Recreates the index (makes it empty). Dummy.
         */
        virtual void recreateIdx();


        /**
         * Called when an index is dropped and the corresponding
         * handle has been undefined. Thread-safe.
         *
         * @return true if this multi idx was indeed using the given
         * handle, false otherwise.
         */
        bool removeHndl(IdxDbHndl droppedHndl);


        /**
         * A newcomer physical index with the given base app class has
         * yielded a new instance (and a handle too). An existing
         * multi searcher may want to take it into use (in which case
         * the reference on the handle must be incremented).
         *
         * Thread-safe, but NOTE: see comments for
         * IdxDbMgr::incIdxDbRefCount().
         */
        void suggestHndl(IdxDbHndl    newHndl,
                         const char * qualifiedbaseAppClass);


        //
        // private-private methods (used only by MultiIdxDb itself)
        //

        /**
         * Cleans up searcher_ and searchers_ instances
         */
        void cleanup();
  

        /**
         * Releases the handles of referred IdxDb-s.
         */
        void releaseHndls();


        /**
         * Performs a search - not thread-safe.
         */
        lucene::search::Hits * doSearch(lucene::search::Query    * query,
                                        Version                  * version,
                                        Cpt::MultiSyncRegion     * msr);
        
    };

} // namespace Cpix


#endif // CPIX_MULTIIDXDB_H
