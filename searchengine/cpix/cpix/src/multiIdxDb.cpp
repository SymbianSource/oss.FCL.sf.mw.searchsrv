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

#include <fstream>
#include <map>
#include <memory>
#include <vector>

#include "CLucene.h"
#include "CLucene/queryParser/MultiFieldQueryParser.h"

#include "cpixtools.h"

#include "cpixsearch.h"
#include "cpixidxdb.h"
#include "multiidxdb.h"
#include "initparams.h"
#include "cpixutil.h"
#include "rotlogger.h"
#include "idxdbmgr.h"
#include "cpixhits.h"
#include "ireaderowner.h"

#include "common/cpixlog.h"


// from idxdb.cpp
Cpix::IHits * GetTerms(lucene::index::IndexReader * reader,
                       const wchar_t              * field,
                       const wchar_t              * wildcard,
                       const wchar_t 		  * appclassPrefix,
                       int                          maxHits);


namespace Cpix
{

    //
    // OriginatorInfo
    //
    OriginatorInfo::OriginatorInfo()
        : version_(0),
          readerOwner_(NULL)
    {
        ;
    }


    OriginatorInfo::OriginatorInfo(IIdxDb * idxDb)
        : version_(0),
          readerOwner_(NULL)
    {
        readerOwner_ = dynamic_cast<IReaderOwner*>(idxDb);

        if (readerOwner_ == NULL)
            {
                THROW_CPIXEXC(PL_ERROR "IIdxDb obj here must be IReaderOwner");
            }
    }


    Version OriginatorInfo::version() const
    {
        return version_;
    }



    void OriginatorInfo::setVersion(Version version)
    {
        version_ = version;
    }


    IReaderOwner & OriginatorInfo::readerOwner() const
    {
        if (readerOwner_ == NULL)
            {
                THROW_CPIXEXC(PL_ERROR "Reader owner pointer is NULL");
            }

        return *readerOwner_;
    }



    //
    // MultiIdxDb
    //

    using namespace lucene::search;
    using namespace lucene::index;


    lucene::search::Hits * MultiIdxDb::search(lucene::search::Query * query,
                                              Version               * version)
    {
        logDbgMsg("MultiIdxDb::search BEGIN");
        Cpt::StopperWatch
            stopperWatch;

        Cpt::SyncRegion
            sr(mutex_);

        const size_t
            size = originators_.size();

        lucene::search::Hits
            * rv = NULL;

        if (size > 0)
            {
                // all the IdxDb instances must be locked, in order to be able
                // to safely search
                //
                // note that the handles are iterated in an ascending order
                // always (benefit of using tree-based set implementation),
                // thus the handles are always locked in the ascending order
                Cpt::MultiSyncRegion
                    msr(originators_.size());

                rv = doSearch(query,
                              version,
                              &msr); 

                // msr dies and unlocks all idxDb here
            }

        logDbgMsg("MultiIdxDb::search END (elapsed: %ld ms)",
                  stopperWatch.elapsedMSecs());

        return rv;

        // sr dies and unlocks this.mutex_
    }



    lucene::search::Hits * 
    MultiIdxDb::fetchRecommitting(lucene::search::Hits     * currentHits,
                                  Version                  * version,
                                  lucene::search::Query    * query,
                                  DocumentConsumer         & documentConsumer)
    {
        logDbgMsg("MultiIdxDb::recommitIfNecessary BEGIN");
        Cpt::StopperWatch
            stopperWatch;

        Cpt::SyncRegion
            sr(mutex_);

        const size_t 
            size = originators_.size();

        std::auto_ptr<lucene::search::Hits>
            rv(currentHits);

        if (size > 0)
            {
                // all the IdxDb instances must be locked, in order to
                // be able to safely search
                //
                // note that the handles are iterated in an ascending
                // order always (benefit of using tree-based set
                // implementation), thus the handles are always locked
                // in the ascending order
                Cpt::MultiSyncRegion
                    msr(size);

                bool
                    recommitNeeded = (currentHits == NULL 
                                      || *version != version_),
                    hasCheckedOriginators = false;

                if (!recommitNeeded)
                    {
                        // so far we don't know of any change - but we
                        // have to check it and actually lock the
                        // originators

                        hasCheckedOriginators = true;

                        OriginatorMap::iterator
                            i = originators_.begin(),
                            end = originators_.end();

                        for (; i != end; ++i)
                            {
                                IReaderOwner
                                    & ro(i->second.readerOwner());
                        
                                // Gets the version number of the
                                // index AND locks it - once we are
                                // told that the version number has
                                // not changed, it MUST NOT change
                                // until we release the originator
                                Version
                                    idxDbVersion = ro.getVersion(msr);
                        
                                if (idxDbVersion != i->second.version())
                                    {
                                        i->second.setVersion(idxDbVersion);
                                        recommitNeeded = true;
                                    }
                            }
                    }

                if (recommitNeeded)
                    {
                        // we already have the originator idx-es
                        // locked if we have checked the versions, so
                        // msr instance is not given anymore (no
                        // reason to lock the very same instances
                        // again)
                        rv.reset(doSearch(query,
                                          version,
                                          hasCheckedOriginators ? NULL : &msr));
                    }

                documentConsumer.fetchDocuments(rv.get());

                // msr dies and unlocks all idxDb here
            }
        else
            {
                // there are no handles this instance refers to (they
                // have been undefined apparently): old hits are dead,
                // there is no new one
                rv.reset(NULL);
            }

        logDbgMsg("MultiIdxDb::recommitIfNecessary END (elapsed: %ld ms)",
                  stopperWatch.elapsedMSecs());

        return rv.release();

        // sr dies and unlocks this.mutex_
    }


    IHits * MultiIdxDb::getTerms(const wchar_t * field, 
                                 const wchar_t * wildcard, 
                                 const wchar_t * appclassPrefix, 
                                 int             maxHits)
    {
        logDbgMsg("MultiIdxDb::getTerms BEGIN");
        Cpt::StopperWatch
            stopperWatch;

        using namespace lucene::index;

        Cpt::SyncRegion
            sr(mutex_);

        cleanup();

        IHits
            * rv = NULL;

        const size_t
            size = originators_.size();

        if (size > 0)
            {
        
                // all the IdxDb instances must be locked, in order to
                // be able to safely search
                //
                // note that the handles are iterated in an ascending
                // order always (benefit of using tree-based set
                // implementation), thus the handles are always locked
                // in the ascending order
                Cpt::MultiSyncRegion
                    msr(size);

                std::vector<IndexReader*>
                    readers;
                readers.reserve(size + 1);

                // this operation (getTerms()) does not need the
                // version number foolery to decide if hits are valid,
                // but we get and maintain that sort of information
                // because
                //
                // (a) getting the version along with the reader and
                // maintaining it is near-zero cost operations
                //
                // (b) it helps a lot during fetchRecommitting()
                // operations if we already know that we need to
                // recommit
                bool
                    hasAnyOriginatorChanged = false;

                OriginatorMap::iterator
                    i = originators_.begin(),
                    end = originators_.end();
                for (; i != end; ++i)
                    {
                        IReaderOwner
                            & ro(i->second.readerOwner());

                        Version
                            version;

                        // idx is locked now (lock information is in
                        // msr)

                        readers.push_back(ro.getReader(&msr,
                                                       &version));

                        if (version != i->second.version())
                            {
                                i->second.setVersion(version);
                                hasAnyOriginatorChanged = true;
                            }
                    }

                if (hasAnyOriginatorChanged)
                    {
                        version_ = IdxDbMgr::instance()->getNextVersion();
                    }

                readers.push_back(NULL);
        
                MultiReader
                    mr(&readers[0],
                       false); // mr does not own the reader array and
                               // ptr-s
        
                /* TEMP
                   logDbgMsg("MultiIdxDb::getTerms MIDDLE, haoc(%d), size(%d)",
                   hasAnyOriginatorChanged,
                   size);
                */

                rv = GetTerms(&mr,
                              field,
                              wildcard,
                              appclassPrefix,
                              maxHits);

                // msr dies and unlocks all idxDb here
            }
        else
            {
                // no handles to depend upon - we return an empty list
                rv = new HitDocumentList();
            }
        
        // readers in reader are not owned by us - nothing to clean
        // reader itself is automaticly cleaned
        // mir is automatically cleaned

        logDbgMsg("MultiIdxDb::getTerms END (elapsed: %ld ms)",
                  stopperWatch.elapsedMSecs());

        return rv;

        // sr dies and unlocks this.mutex_
    }


    SchemaId MultiIdxDb::addSchema(const cpix_FieldDesc * fieldDescs,
                                   size_t                 count) 
    {
        THROW_CPIXEXC(PL_ERROR "should not have been called");
    }
    
    void MultiIdxDb::add(Document * doc,
                         lucene::analysis::Analyzer * analyzer)
    {
        THROW_CPIXEXC(PL_ERROR "should not have been called");
    }

    void MultiIdxDb::add2(SchemaId                      schemaId,
                          const wchar_t               * docUid,
                          const char                  * appClass,
                          const wchar_t               * excerpt,
                          const wchar_t               * mimeType,
                          const wchar_t              ** fieldValues,
                          lucene::analysis::Analyzer  * analyzer)
    {
        THROW_CPIXEXC(PL_ERROR "should not have been called");
    }

    int32_t MultiIdxDb::deleteDocuments(const wchar_t  * docUid) 
    {
        THROW_CPIXEXC(PL_ERROR "should not have been called");
    }

    int32_t MultiIdxDb::deleteDocuments2(lucene::index::Term * term)
    {
        THROW_CPIXEXC(PL_ERROR "should not have been called");
    }

    void MultiIdxDb::update(Document * doc,
                            lucene::analysis::Analyzer * analyzer)
    {
        THROW_CPIXEXC(PL_ERROR "should not have been called");
    }
    
    void MultiIdxDb::update2(SchemaId                     schemaId,
                             const wchar_t              * docUid,
                             const char                 * appClass,
                             const wchar_t              * excerpt,
                             const wchar_t              * mimeType,
                             const wchar_t             ** fieldValues,
                             lucene::analysis::Analyzer * analyzer)
    {
        THROW_CPIXEXC(PL_ERROR "should not have been called");
    }
    

    void MultiIdxDb::setMaxInsertBufSize(size_t /* value */)
    {
        THROW_CPIXEXC(PL_ERROR "should not have been called");
    }


    void MultiIdxDb::flush()
    {
        THROW_CPIXEXC(PL_ERROR "should not have been called");
    }

    
    void MultiIdxDb::close()
    {
        Cpt::SyncRegion
            sr(mutex_);

        releaseHndls();
        cleanup();
    }

    void MultiIdxDb::brutalClose() throw ()
    {
        try
            {
                close();
            }
        catch (...)
            {
                logMsg(CPIX_LL_ERROR,
                       "Could not cleanup multiidxdb instance");
            }
    }


    void MultiIdxDb::doHousekeeping()
    {
        THROW_CPIXEXC(PL_ERROR "should not have been called");
    }


    void MultiIdxDb::dbgDumpState()
    {
        logTestMsg(CPIX_LL_TRACE,
                   "    DUMPING MultiIdxDb instance: BEGIN",
                   reinterpret_cast<long>(this));
        
        logTestMsg(CPIX_LL_TRACE,
                   "    o domainSelector_: %s",
                   domainSelector_.c_str());

        Cpt::SyncRegion
            sr(mutex_);

        OriginatorMap::iterator
            i = originators_.begin(),
            end = originators_.end();

        for (; i != end; ++i)
            {
                logTestMsg(CPIX_LL_TRACE,
                           "    o idxDbHndl %d",
                           i->first);
            }

        logTestMsg(CPIX_LL_TRACE,
                   "    DUMPING MultiIdxDb instance: END.",
                   reinterpret_cast<long>(this));
        
    }

    
    MultiIdxDb::MultiIdxDb(std::set<IdxDbHndl> & idxDbHndls,
                           const char          * domainSelector)
        : refCount_(1),
          domainSelector_(domainSelector),
          searcher_(NULL),
          mutex_(true), // recursive
          version_(IdxDbMgr::instance()->getNextVersion())
    {
        std::set<IdxDbHndl>::const_iterator
            i = idxDbHndls.begin(),
            end = idxDbHndls.end();

        for (; i != end; ++i)
            {
                originators_[*i] = OriginatorInfo(IIdxDb::getPtr(*i));
            }
    }


    MultiIdxDb::~MultiIdxDb()
    {
        brutalClose();
    }


    void MultiIdxDb::recreateIdx()
    {
        THROW_CPIXEXC(PL_ERROR "should not have been called");
    }


    bool MultiIdxDb::removeHndl(IdxDbHndl droppedHndl)
    {
        bool
            rv = false;

        Cpt::SyncRegion
            sr(mutex_);

        version_ = IdxDbMgr::instance()->getNextVersion();
        
        rv = originators_.find(droppedHndl) != originators_.end();

        originators_.erase(droppedHndl);

        return rv;
    }
    
    
    void MultiIdxDb::suggestHndl(IdxDbHndl    newHndl,
                                 const char * baseAppClass)
    {
        Cpt::SyncRegion
            sr(mutex_);

        version_ = IdxDbMgr::instance()->getNextVersion();

        if (IdxDbMgr::match(domainSelector_.c_str(), baseAppClass))
            {
                IdxDbMgr
                    * idxDbMgr = IdxDbMgr::instance();

                // this private method of IdxDbMgr is not thread-safe,
                // but this function is called only from a function
                // (defineVolume) of IdxDbMgr, which already does that
                idxDbMgr->incIdxDbRefCount(newHndl);
                
                originators_[newHndl] = OriginatorInfo(IIdxDb::getPtr(newHndl));
            }
    }
    


    void MultiIdxDb::cleanup()
    {
        std::vector<Searchable*>::iterator
            i = searchers_.begin(),
            end = searchers_.end();

        for (; i != end; ++i)
            {
                delete *i;
            }

        searchers_.clear();
                
        if (searcher_ != NULL)
            {
                delete searcher_;
                searcher_ = NULL;
            }
    }


    void MultiIdxDb::releaseHndls()
    {
        OriginatorMap::iterator
            i = originators_.begin(),
            end = originators_.end();

        for (; i != end; ++i)
            {
                IIdxDb::release(i->first);
            }

        originators_.clear();
    }


    lucene::search::Hits *
    MultiIdxDb::doSearch(lucene::search::Query * query,
                         Version               * version,
                         Cpt::MultiSyncRegion  * msr)
    {
        cleanup();

        // TODO
        //
        // This current implementation builds up a multisearcher from
        // scratch for each search invocation and tears it down once
        // the search is done. Consider optimizing it.
        //
        const size_t size = originators_.size();

        searchers_.reserve(size + 1);

        bool
            hasAnyOriginatorChanged = false;

        OriginatorMap::iterator
            i = originators_.begin(),
            end = originators_.end();

        for (; i != end; ++i)
            {
                IReaderOwner
                    & ro(i->second.readerOwner());

                Version
                    idxDbVersion;

                // idx is locked now (lock information is in msr)
                searchers_.push_back(new IndexSearcher(ro.getReader(msr,
                                                                    &idxDbVersion)));

                if (idxDbVersion != i->second.version())
                    {
                        i->second.setVersion(idxDbVersion);
                        hasAnyOriginatorChanged = true;
                    }
            }

        if (hasAnyOriginatorChanged)
            {
                version_ = IdxDbMgr::instance()->getNextVersion();
            }

        *version = version_;

        searchers_.push_back(NULL);
        
        searcher_ = new MultiSearcher(&searchers_[0]);

        lucene::search::Hits
            * rv = searcher_->search(query); 

        return rv;
    }


}

