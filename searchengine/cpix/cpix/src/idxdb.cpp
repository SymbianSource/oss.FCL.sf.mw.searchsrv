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

#include <map>
#include <memory>
#include <vector>
#include <queue>
#include <iostream>
#include <sstream>

#include "CLucene.h"
#include "CLucene/queryParser/MultiFieldQueryParser.h"

#include "cpixtools.h"

#include "cpixexc.h"
#include "cpixsearch.h"
#include "cpixidxdb.h"
#include "idxdb.h"
#include "cpixhits.h"
#include "initparams.h"
#include "cpixutil.h"
#include "cpixdoc.h"
#include "ifieldfilter.h"
#include "cluceneext.h"
#include "rotlogger.h"
#include "idxdbmgr.h"
#include "document.h"

#include "cpixmaindefs.h"

#include "analyzer.h"

#include "common/cpixlog.h"

namespace
{

    /**
     * Process a filter by its id. The filter id field is not
     * (anymore) on the document.
     *
     * NOTE: Recent changes allow filter's to install new filters, so
     * they need to be exhaustively and iteratively processed as long
     * as there is anything to process.
     */
    bool processFilter(Cpix::Document * doc,
                       const wchar_t              * filterId)
    {
        using namespace Cpix;
        
        bool
            goOn = true;
        std::wstring
            dummy;

        while (goOn && filterId != NULL)
            {
                FieldFilterSentry
                    ffs(FieldFilterPool::getFieldFilter(filterId));
                goOn = ffs.get()->process(doc);

                if (goOn)
                    {
                        filterId = NULL;
                        filterId = doc->get(LCPIX_FILTERID_FIELD);
                        if (filterId != NULL)
                            {
                                // re-point filterId: same content but
                                // not destroyed when the field is
                                // removed
                                dummy = filterId;
                                filterId = dummy.c_str();

                                // remove the field
                                doc->removeField(LCPIX_FILTERID_FIELD);
                            }
                    }
            }

        return goOn;
    }



    /**
     * Process a filter, by its id, if it is defined, removing the
     * given filter id field.
     */
    bool processFilterIfNecessary(Cpix::Document * doc)
    {
        bool
            rv = true;

        const wchar_t
            * filterId = doc->get(LCPIX_FILTERID_FIELD);

        if (filterId != NULL)
            {
                std::wstring
                    wfid(filterId);

                doc->removeField(LCPIX_FILTERID_FIELD);

                rv = processFilter(doc,
                                   wfid.c_str());
            }

        return rv;
    }



}





// GetTerms implementation details
namespace {

    // Following definitions are needed for memory management and sorting of 
    // getTerms operations: 

    // Document entry definition. First integer contains the document frequence
    typedef std::pair<int, lucene::document::Document*> DocumentEntry;

    // Comparation method for sorting document entries
    /*bool operator<(DocumentEntry first, DocumentEntry second) {
        return first.first < second.first; 
    }
*/
    
    typedef std::vector<DocumentEntry> DocumentEntryVector;

    // Cleans out documents in case of exceptions
    class DocumentEntryQueue : public std::priority_queue<DocumentEntry>
    {
    public:
        DocumentEntryQueue() : std::priority_queue<DocumentEntry>() {}
        ~DocumentEntryQueue() {
            while (!empty()) 
                {
                    delete top().second; 
                    pop(); 
                }
        }
    };

}


Cpix::IHits * GetTerms(lucene::index::IndexReader * reader,
                       const wchar_t              * field,
                       const wchar_t              * wildcard,
                       const wchar_t 		  * appclassPrefix, 
                       int                          maxHits)
{
    using namespace std; 
    using namespace lucene::document; 
    using namespace lucene::index;
    using namespace lucene::search;

    if (field == NULL || wildcard == NULL || reader == NULL)
        {
            THROW_CPIXEXC("Null args to ::GetTerms");
        }

    /* TEMP
    logDbgMsg("::GetTerms BEGIN %S:%S",
              field,
              wildcard);
    */
			
    // Check that wilcard string is valid, WildcardTermEnum will panic otherwise
    const wchar_t* sindex = wcschr( wildcard, 
                                    LUCENE_WILDCARDTERMENUM_WILDCARD_STRING );
    const wchar_t* cindex = wcschr( wildcard, 
                                    LUCENE_WILDCARDTERMENUM_WILDCARD_CHAR );
    if (!sindex && !cindex)
        {
            THROW_CPIXEXC("missing wildcard in term '%S'",
                          wildcard); 
        }
		
    typedef std::vector<int> IntVector;
    typedef std::greater<int> GreaterInt;
		
    DocumentEntryQueue documents; 
    std::priority_queue<int, IntVector, GreaterInt> docFreqList; 
		
    // Setup wildcard term enumeration
    Term
        * term = _CLNEW Term( field, wildcard ); // increases reference
    
    // Appclass filtering infra
    std::auto_ptr<TermDocs> docs;
    FieldCacheAuto * cache=NULL; 

    // Prepare for filtering only if appclass filter is set
    if ( appclassPrefix ) 
        {
            docs.reset( reader->termDocs() );
            cache = FieldCache::DEFAULT->getStrings( reader, 
                                                     LCPIX_UNTOKENIZED_APPCLASS_FIELD ); // not owned.
        }

    /* TEMP
    logDbgMsg("::GetTerms 0 (%p)",
              cache);
    */

    try 
        {
            WildcardTermEnum terms( reader, term ); // increases reference
            
            do {
                Term
                    * t = terms.term(false);
            
                if (t == NULL)
                    {
                        break;
                    }

                // First step: Measure document frequence
                int 
                    docFreq = 0; 
                
                if ( appclassPrefix && cache ) 
                    {
                        // If appclass filter is defined, document
                        // frequence has to be counted from documents
                        // matching the filter
                        docs->seek(t); 
                        while (docs->next()) {
                            int doc = docs->doc();
                            const wchar_t* docField = cache->stringArray[doc];
                            if (docField) {
                                int i = 0; 
                                for (;appclassPrefix[i]; i++) {
                                    if (docField[i] != appclassPrefix[i]) break;
                                }
                                if (!appclassPrefix[i]) {
                                    docFreq++; 
                                }
                            }
                        }
                    } else {
                    // All documents go
                    docFreq = terms.docFreq();
                }

                if (docFreq > 0) 
                    {
                        // Extract information out of current term. Document
                        // frequence needs to be converted into as string
                        const wchar_t
                            * termText = t->text();
				
                        if (documents.size() < maxHits || docFreq > docFreqList.top()) 
                            {   // filter out all documents with low document frequence 
						
                                wostringstream toDocFreqText;
                                toDocFreqText<<docFreq;
						
                                // setup and populate
                                auto_ptr<Document>
                                    doc(new Document());
                                auto_ptr<Field>
                                    newField(new Field(LTERM_TEXT_FIELD, 
                                                       termText, 
                                                       Field::STORE_YES | Field::INDEX_NO));
                                doc->add(*newField.get()); 
                                newField.release();
	
                                newField.reset(new Field(LTERM_DOCFREQ_FIELD, 
                                                         toDocFreqText.str().c_str(), 
                                                         Field::STORE_YES | Field::INDEX_NO));
                                doc->add(*newField.get());
                                newField.release();
						
                                documents.push( DocumentEntry( docFreq, doc.get() ) ); 
                                doc.release();
						
                                // maintain knowledge of the last document frequence
                                if (documents.size() >= maxHits) docFreqList.pop();
                                docFreqList.push(docFreq); 
                            } 
                    }
            
            } while (terms.next());
        }
    catch (...)
        {
            // who knows if auto_ptr is compatible with _CL??? macros
            // - doing it manually
            _CLDECDELETE( term );
            throw;
        }


    // TEMP logDbgMsg("::GetTerms 1");

    _CLDECDELETE( term );

    // extract the sorted documents into hit document list safely
    auto_ptr<Cpix::HitDocumentList> 
        hits(new Cpix::HitDocumentList); 

    while (!documents.empty() && maxHits--) 
        {
            // Pick documents one by one. This will reverse the order
            Document
                * doc = documents.top().second; 

            documents.pop(); // releases ownership
            hits->add( doc ); // grants ownership
        }

    // TEMP logDbgMsg("::GetTerms END");

    return hits.release(); 
}



namespace Cpix
{

    using namespace lucene::analysis;
    using namespace lucene::document;
    using namespace lucene::index;
    using namespace lucene::queryParser;
    using namespace lucene::search; 
    using namespace lucene::store; 
    using namespace lucene::util;


    //////////////////////////////////////////////////////
    //
    //  InsertBuf
    //
    InsertBuf::InsertBuf(size_t maxDocs)
        : maxDocs_(maxDocs),
          writer_(NULL),
          ramDir_(NULL)
    {
        ;
    }
          
    
    void InsertBuf::close()
    {
        getRidOfWriter();

        if (ramDir_ != NULL)
            {
                Impl::CallCloseLogging(*ramDir_);
                _CLDECDELETE(ramDir_);
                ramDir_ = NULL;
            }
    }


    bool InsertBuf::isEmpty() const
    {
        return ramDir_ == NULL;
    }


    lucene::store::TransactionalRAMDirectory * InsertBuf::getRAMDir()
    {
        if (ramDir_ == NULL)
            {
                THROW_CPIXEXC(PL_ERROR "accessing NULL ramDir");
            }
        
        getRidOfWriter();

        return ramDir_;
    }


    InsertBuf::~InsertBuf()
    {
        close();
    }


    void InsertBuf::add(Cpix::Document             * doc,
                        lucene::analysis::Analyzer * analyzer)
    {
        lazyInit();

        std::wstring
            docUid(doc->get(LCPIX_DOCUID_FIELD));

        writer_->addDocument(&doc->native(),
                             analyzer);
    }


    int32_t InsertBuf::deleteDocuments(lucene::index::Term * term)
    {
        int32_t
            rv = 0;

        if (!isEmpty()) // ramDir_ is not NULL but writer_ may be
            {
                using namespace lucene::index;
                
                getRidOfWriter();
                
                // no we do the deletion on the ramDir_ using
                // a temporary reader
                Impl::cl_auto_ptr<IndexReader>
                    tmpReader(IndexReader::open(ramDir_,
                                                false)); // don't close
                rv = tmpReader->deleteDocuments(term);

                // close()-ing tmpReader implied by cl_auto_ptr
                tmpReader.reset();
            }

        return rv;
    }
        
    size_t InsertBuf::size()
    {
        size_t
            rv = 0;

        if (!isEmpty()) // ramDir_ not NULL but writer_ may be
            {
                lazyInit();
                
                std::vector<std::string>
                    files;
                ramDir_->list(&files);
		
                rv = writer_->ramSizeInBytes(); 
		
                std::vector<std::string>::iterator 
                    i = files.begin(),
                    end = files.end();
                
                for (; i != end; ++i) 
                    {
                        rv += ramDir_->fileLength(i->c_str());
                    }
            }
        
        return rv; 
    }


    void InsertBuf::lazyInit()
    {
        bool
            create = false;

        if (ramDir_ == NULL)
            {
                ramDir_ = _CLNEW TransactionalRAMDirectory();
                create = true;
            }

        if (writer_ == NULL)
            {
                writer_ = new IndexWriter(ramDir_,
                                          NULL,
                                          create,
                                          false);
                writer_->setMaxBufferedDocs(maxDocs_);
            }
    }


    void InsertBuf::getRidOfWriter()
    {
        if (writer_ != NULL)
            {
                Impl::CallCloseLogging(*writer_);
                // writer_->close();
                delete writer_;
                writer_ = NULL;
            }
    }




    //////////////////////////////////////////////////////
    //
    //  VersionedReader
    //

    void VersionedReader::open(const char * clIdxPath,
                               Cpt::Mutex & dirMutex)
    {
        using namespace lucene::index;

        logDbgMsg("VersionedReader::open (%s) BEGIN",
                  clIdxPath);

        Cpt::SyncRegion
            sr(mutex_);

        Cpt::SyncRegion
            sr2(dirMutex);

        if (reader_ != NULL)
            {
                logDbgMsg("VersionedReader::open MIDDLE 1");

                destroySearcher();

                // we have to allow this, as open() and reopen() may
                // run concurrently
                Impl::cl_auto_ptr<IndexReader>
                    dummy(reader_);

                // reader_ gets close()-d and destroyed
            }

        reader_ = IndexReader::open(clIdxPath);

        version_ = IdxDbMgr::instance()->getNextVersion();
        deletionsCount_ = 0;

        logDbgMsg("VersionedReader::open END");
    }


    void VersionedReader::reopen(Cpt::Mutex & dirMutex,
                                 const char * clIdxPath,
                                 bool         reRead)
    {
        logDbgMsg("VersionedReader::reopen (%s) BEGIN",
                  clIdxPath);

        using namespace lucene::index;

        // NOTE / TODO ?:
        // This implementation loads the new version first, and only
        // then it swaps the old one with the new one (if there is a
        // re-read, that is). This makes the search clients wait less,
        // but consumes more RAM.
        //
        // If this is a problem, this body could be re-implemented so
        // that it first closes the current reader_ and then loads the
        // new one - consumes less memory but search clients will have
        // to wait more.
     
        // NOTE:
        // Checking for reader_ == NULL could be protected by mutex_,
        // but there is no point. First of all, it would be promptly
        // released, and there would be the same race condition
        // between checking the value and acting on it. (We don't want
        // to keep it locked, that would stall searcher threads!)
        //
        // The best we can do is make sure that nothing leaks or gets
        // double destroyed. From that point on, only performance can
        // suffer, but there is hardly anything one can do when search
        // the first time while harvesting - searches will be blocked
        // a bit.
        bool
            isOpen = reader_ != NULL;

        Impl::cl_auto_ptr<IndexReader>
            newReader(NULL);

        // first, load the new version, if necessary
        if (isOpen && reRead)
            {
                Cpt::SyncRegion
                    sr(dirMutex);

                try
                    {
                        logDbgMsg("VersionedReader::reopen MIDDLE 1");

                        newReader.reset(IndexReader::open(clIdxPath));
                    }
                catch (...)
                    {
                        logMsg(CPIX_LL_ERROR,
                               "Failed (re-)reading new version of idx at %s",
                               clIdxPath);
                        throw;
                    }
            }

        // store the current reader, if any, on the side
        IndexReader
            * oldReader = NULL;

        // actual swap - very quick operation
        {
            Cpt::SyncRegion
                sr(mutex_);
            
            if (reader_ != NULL)
                {
                    logDbgMsg("VersionedReader::reopen MIDDLE 2");

                    oldReader = reader_;

                    destroySearcher();

                    reader_ = newReader.release();
                }

            version_ = IdxDbMgr::instance()->getNextVersion();
            deletionsCount_ = 0;
        }

        // the old reader, if any, has to be destroyed on the side -
        // possibly file i/o may happen
        {
            Cpt::SyncRegion
                sr2(dirMutex);
            if (oldReader != NULL)
                {
                    Impl::cl_auto_ptr<IndexReader>
                        dummy(oldReader);

                    // oldReader gets close()-d and destroyed
                }
        }

        logDbgMsg("VersionedReader::reopen END");
    }



    bool VersionedReader::commitIfNecessary(Cpt::Mutex & dirMutex)
    {
        Cpt::SyncRegion
            sr(mutex_);

        bool
            rv = false;

        if (reader_ != NULL && reader_->hasDeletions())
            {
                Cpt::SyncRegion
                    sr(dirMutex);

                reader_->commit();

                rv = true;
            }

        return rv;
    }

        
    VersionedReader::VersionedReader()
        : reader_(NULL),
          version_(IdxDbMgr::instance()->getNextVersion()),
          searcher_(NULL),
          mutex_(true), // recursive mutex
          deletionsCount_(0)
    {
        ;
    }


    VersionedReader::~VersionedReader()
    {
        close(NULL);
    }


    void VersionedReader::close(Cpt::Mutex * dirMutex,
                                const char * cpixDir)
    {
        logDbgMsg("VersionedReader::close BEGIN");

        Cpt::SyncRegion
            sr(mutex_);

        destroySearcher();

        // lock if we have something to lock on
        std::auto_ptr<Cpt::SyncRegion>
            sr2;
        
        if (dirMutex != NULL)
            {
                sr2.reset(new Cpt::SyncRegion(*dirMutex));
            }
        
        if (reader_ != NULL)
            {
                Impl::cl_auto_ptr<IndexReader>
                    dummy(reader_);
                
                // reader_ gets close()-d and destroyed
                dummy.reset();
                
                reader_ = NULL;
            }
        
        if (cpixDir != NULL)
            {
                IdxDbMgr::RecreateFsCpixIdx(cpixDir);
            }

        version_ = IdxDbMgr::instance()->getNextVersion();
        deletionsCount_ = 0;

        logDbgMsg("VersionedReader::close END");
    }


    bool VersionedReader::isOpen(Cpt::MultiSyncRegion  * msr)
    {
        if (msr != NULL)
            {
                msr->lock(mutex_);
            }

        return reader_ != NULL;
    }


    bool VersionedReader::hasDeletions() const
    {
        return deletionsCount_ > 0;
    }
        

    Version VersionedReader::getVersion(Cpt::MultiSyncRegion & msr)
    {
        logDbgMsg("VersionedReader::getVersion BEGIN");

        msr.lock(mutex_);

        logDbgMsg("VersionedReader::getVersion END");

        return version_;
    }


    lucene::index::IndexReader * VersionedReader::getReader(Version * version)
    {
        if (!isOpen(NULL))
            {
                THROW_CPIXEXC(PL_ERROR "accessing a closed reader");
            }

        *version = version_;
        
        return reader_;
    }


    lucene::search::Hits * 
    VersionedReader::search(lucene::search::Query * query,
                            Version               * version)
    {
        if (!isOpen(NULL))
            {
                THROW_CPIXEXC(PL_ERROR "searching a closed reader");
            }

        if (searcher_ == NULL)
            {
                searcher_ = new lucene::search::IndexSearcher(reader_);
            }

        lucene::search::Hits
            * rv = searcher_->search(query);

        if (version != NULL)
            {
                *version = version_;
            }
        
        return rv;
    }


    int32_t VersionedReader::deleteDocuments(lucene::index::Term * term)
    {
        if (!isOpen(NULL))
            {
                THROW_CPIXEXC(PL_ERROR "deleting on a closed reader");
            }

        int32_t
            rv = reader_->deleteDocuments(term);

        deletionsCount_ += rv;

        return rv;
    }


    void VersionedReader::load(const char           * cpixIdxPath,
                               Cpt::Mutex           & dirMutex)
    {
        logDbgMsg("VersionedReader::load BEGIN");

        if (reader_ != NULL)
            {
                THROW_CPIXEXC(PL_ERROR "loading an open reader");
            }

        Impl::IdxDbDelta::RecoverReader(cpixIdxPath,
                                        dirMutex,
                                        this);

        logDbgMsg("VersionedReader::load END");
    }


    void VersionedReader::destroySearcher()
    {
        if (searcher_ != NULL)
            {
                Impl::CallCloseLogging(*searcher_);
                delete searcher_;
                searcher_ = NULL;
            }
    }


    //////////////////////////////////////////////////////
    //
    //  IdxDb
    //
    IdxDb::FieldDesc::FieldDesc(const cpix_FieldDesc * fieldDesc)
        : name_(fieldDesc->name_),
          cfg_(fieldDesc->cfg_)
    {
        ;
    }


    IdxDb::FieldDesc::FieldDesc()
        : cfg_(0)
    {
        ;
    }



    IdxDb::IdxDb(const char  * indexDbPath,
                   InitParams  & ip)
        : indexDbPath_(indexDbPath),
          insertBuf_(ip.getInsertBufMaxDocs()),
          maxInsertBufSize_(ip.getMaxInsertBufSize())
    {
        ;
    }


    IdxDb::~IdxDb()
    {
        brutalClose();
    }


    void IdxDb::recreateIdx()
    {
        // force closing and recreating the index (empty-ing it)
        forceClose(indexDbPath_.c_str());
    }


    lucene::search::Hits * IdxDb::search(lucene::search::Query    * query,
                                          Version                  * version)
    {
        logDbgMsg("IdxDb::search BEGIN");
        Cpt::StopperWatch
            stopperWatch;
        
        Cpt::MultiSyncRegion
            msr(1);

        lucene::search::Hits
            * rv = doSearch(query,
                            version,
                            &msr);

        logDbgMsg("IdxDb::search END (elapsed: %ld ms)",
                  stopperWatch.elapsedMSecs());

        return rv;
    }


    lucene::search::Hits * 
    IdxDb::fetchRecommitting(lucene::search::Hits        * currentHits,
                              Version                     * version,
                              lucene::search::Query       * query,
                              DocumentConsumer            & documentConsumer)
    
    {
        logDbgMsg("IdxDb::fetchRecommitting BEGIN");
        Cpt::StopperWatch
            stopperWatch;

        std::auto_ptr<lucene::search::Hits>
            rv(currentHits);

        Cpt::MultiSyncRegion
            msr(1);

        Version
            curVersion = getVersion(msr);

        if (currentHits == NULL || curVersion != *version)
            {
                rv.reset(doSearch(query,
                                  version,
                                  NULL));
            }

        documentConsumer.fetchDocuments(rv.get());

        logDbgMsg("IdxDb::fetchRecommitting END (elapsed: %ld ms)",
                  stopperWatch.elapsedMSecs());

        return rv.release();
    }


    IHits * IdxDb::getTerms(const wchar_t * field,
                             const wchar_t * wildcard, 
                             const wchar_t * appclassPrefix,
                             int             maxHits)
    {
        logDbgMsg("IdxDb::getTerms BEGIN");
        Cpt::StopperWatch
            stopperWatch;
     
        Cpt::MultiSyncRegion
            msr(1);
   
        Version
            dummy;

        IndexReader
            * reader = getReader(&msr,
                                 &dummy);

        IHits
            * rv = GetTerms(reader,
                            field,
                            wildcard,
                            appclassPrefix, 
                            maxHits);

        logDbgMsg("IdxDb::getTerms END (elapsed: %ld ms)",
                  stopperWatch.elapsedMSecs());

        return rv;
    }


    SchemaId IdxDb::addSchema(const cpix_FieldDesc * fieldDescs,
                               size_t                 count)
    {
        // TODO this current implementation will not re-use schemas
        // That is, the same schema may be added and currently will
        // receive different IDs, and that's not good.
        //
        // FIX: if the schema to be added already exists, just return
        // with its existing ID.

        SchemaDesc
            schemaDesc;
        schemaDesc.reserve(count);

        for (size_t i = 0; i < count; ++i)
            {
                schemaDesc.push_back(FieldDesc(fieldDescs + i));
            }

        Cpt::SyncRegion
            sr(idxMutex_);

        schemas_.push_back(schemaDesc);

        return schemas_.size() - 1;
    }


    void IdxDb::add(Cpix::Document             * doc,
                     lucene::analysis::Analyzer * analyzer)
    {
        const wchar_t
            * docUid = doc->get(LCPIX_DOCUID_FIELD);
        if (docUid == NULL)
            {
                logDbgMsg("IdxDb::add(<NULL>) !!! about to throw");
                THROW_CPIXEXC("Document has no docuid");
            }

        logDbgMsg("IdxDb::add(%S) BEGIN",
                  docUid);
        Cpt::StopperWatch
            stopperWatch;

        if (!doc)
            {
                THROW_CPIXEXC(PL_ERROR "argument 'doc' is null");
            }

        bool
            goOn = processFilterIfNecessary(doc);

        if (goOn)
            {
                using namespace lucene::util;
                namespace ld = lucene::document;

                std::auto_ptr<ld::Field>
                    newField(new ld::Field(LCPIX_DEFAULT_FIELD,
                                           L"",
                                           ld::Field::STORE_NO | 
                                           ld::Field::INDEX_TOKENIZED));
                
                doc->native().add(* newField.get());
                newField.release();
                
                AggregateFieldAnalyzer 
                    aggrAnalyzer(*doc, *analyzer); 

                insertBuf_.add(doc,
                               &aggrAnalyzer);
                
                if (insertBuf_.size() > maxInsertBufSize_)
                    {
                        flush();
                    }
            }

        logDbgMsg("IdxDb::add END (elapsed: %ld ms)",
                  stopperWatch.elapsedMSecs());
    }



    void IdxDb::add2(SchemaId                            schemaId,
                      const wchar_t                     * docUid,
                      const char                        * appClass,
                      const wchar_t                     * excerpt,
                      const wchar_t                     * mimeType,
                      const wchar_t                    ** fieldValues,
                      lucene::analysis::Analyzer        * analyzer)
    {

        if (schemaId >= schemas_.size())
            {
                THROW_CPIXEXC(L"Unregistered schema identifier '%d'",
                              schemaId);
            }

        std::auto_ptr<Document>
            document(new Document(docUid, appClass, excerpt, mimeType));
        
        const wchar_t
            * filterId = NULL;

        { // LOCK
            // accessing schemas_
            Cpt::SyncRegion
                sr(idxMutex_);

            const SchemaDesc
                & schemaDesc = schemas_[schemaId];
            
            for (SchemaDesc::size_type i = 0;
                 i < schemaDesc.size();
                 ++i)
                {
                    if (fieldValues[i] == NULL)
                        {
                            continue;
                        }

                    const FieldDesc
                        & fieldDesc = schemaDesc[i];

                    if (wcscmp(fieldDesc.name_.c_str(), LCPIX_FILTERID_FIELD) == 0)
                        {
                            filterId = fieldValues[i];
                            continue;
                        }

                    auto_ptr<Field>
                        newField(new Field(fieldDesc.name_.c_str(),
                                           fieldValues[i],
                                           fieldDesc.cfg_));
                    document->add(newField.get());
                    newField.release();
                }
        } // UNLOCK
        
        bool
            goOn = true;

        if (filterId != NULL)
            {
                goOn = processFilter(document.get(),
                                     filterId);
            }

        if (goOn)
            {
                add(document.get(),
                    analyzer);
            }
    }


    int32_t IdxDb::deleteDocuments(const wchar_t  * docUid)
    {
        using namespace lucene::index;

        Term
            term(LCPIX_DOCUID_FIELD,
                 docUid);

        return deleteDocuments2(&term);
    }


    int32_t IdxDb::deleteDocuments2(lucene::index::Term * term)
    {
        logDbgMsg("IdxDb::deleteDocuments2 BEGIN");
        Cpt::StopperWatch
            stopperWatch;

        int
            rv = 0;
        
        { // SYNC

            Cpt::MultiSyncRegion
                msr(1);

            loadReader(&msr);
            
            rv = reader_.deleteDocuments(term);
	  
            // TODO aggregate number of deleted documets, and commit to
            // disk if exceeding preset limit (new init param). (???? -
            // but, most deletions happen as part of an update, and
            // flushing based deletions may interfere with
            // maxInsertBufSize settings)

        } // SYNC

        rv += insertBuf_.deleteDocuments(term);

        logDbgMsg("IdxDb::deleteDocuments2 END (elapsed: %ld ms",
                  stopperWatch.elapsedMSecs());

        return rv;
    }


    void IdxDb::update(Cpix::Document * doc,
                        lucene::analysis::Analyzer * analyzer)
    {
        const wchar_t
            * docUid = doc->get(LCPIX_DOCUID_FIELD);

        int32_t
            numOfDeleted = deleteDocuments(docUid);

        if (numOfDeleted > 1)
            {
                logMsg(CPIX_LL_WARNING,
                       "Updating multiple docs with uid %S - (not unique)",
                       docUid);
            }

        add(doc,
            analyzer);
    }


    void IdxDb::update2(SchemaId                         schemaId,
                         const wchar_t                  * docUid,
                         const char                     * appClass,
                         const wchar_t                  * excerpt,
                         const wchar_t                  * mimeType,
                         const wchar_t                 ** fieldValues,
                         lucene::analysis::Analyzer     * analyzer)
    {
        deleteDocuments(docUid);
        add2(schemaId,
             docUid,
             appClass,
             excerpt,
             mimeType,
             fieldValues,
             analyzer);
    }
  

    void IdxDb::setMaxInsertBufSize(size_t value)
    {
        Property<size_t, MAXINSERTBUFSIZE>
            sanityCheckerDummy;
    
        // this would throw CpixExc if out-of-range
        sanityCheckerDummy.set(value);
    
        // if we get to this point we can use the value
        maxInsertBufSize_ = value;
    }
  

    void IdxDb::flush()
    {
        using namespace Cpix::Impl;

        logDbgMsg("IdxDb::flush BEGIN (%s)",
                  indexDbPath_.c_str());
        Cpt::StopperWatch
            stopperWatch;

        try
            {
                if (!CpixPaths::IsCleanCpixDir(indexDbPath_.c_str()))
                    {
                        logDbgMsg("IdxDb::flush MIDDLE 1");

                        IdxDbDelta::RecoverReader(indexDbPath_.c_str(),
                                                  idxMutex_,
                                                  NULL); // don't load
                                                         // reader,
                                                         // only clean
                                                         // up cpix dir
                    }

                logDbgMsg("IdxDb::flush MIDDLE 2");

                IdxDbDelta::CommitToDisk(indexDbPath_.c_str(),
                                         &insertBuf_,
                                         reader_,
                                         true, // re-Read if file i/o happened
                                         idxMutex_);
            }
        catch (...)
            {
                logMsg(CPIX_LL_ERROR,
                       "IdxDb::flush got exception - brutally closing");
                brutalClose();
                throw;
            }

        logDbgMsg("IdxDb::flush END (elapsed: %ld ms)",
                  stopperWatch.elapsedMSecs());
    }


    void IdxDb::close()
    {
        using namespace Cpix::Impl;

        logDbgMsg("IdxDb::close BEGIN (%s)",
                  indexDbPath_.c_str());
        Cpt::StopperWatch
            stopperWatch;

        try
            {
                if (!CpixPaths::IsCleanCpixDir(indexDbPath_.c_str()))
                    {
                        logDbgMsg("IdxDb::close MIDDLE 1");

                        IdxDbDelta::RecoverReader(indexDbPath_.c_str(),
                                                  idxMutex_,
                                                  NULL); // don't load
                                                         // reader,
                                                         // only clean
                                                         // up cpix dir
                    }

                logDbgMsg("IdxDb::close MIDDLE 2");

                Impl::IdxDbDelta::CommitToDisk(indexDbPath_.c_str(),
                                               &insertBuf_,
                                               reader_,
                                               false, // don't do re-Read
                                               idxMutex_);
            }
        catch (...)
            {
                logMsg(CPIX_LL_ERROR,
                       "IdxDb::close got exception - brutally closing");
                brutalClose();
                throw;
            }

        logDbgMsg("IdxDb::close END (elapsed: %ld ms)",
                  stopperWatch.elapsedMSecs());
    }

  
    void IdxDb::brutalClose() throw ()
    {
        // force closing (no recreating the index)
        forceClose(NULL);
    }


    void IdxDb::doHousekeeping()
    {
        logDbgMsg("IdxDb::doHousekeeping BEGIN");
        Cpt::StopperWatch
            stopperWatch;

        flush();
      
        logDbgMsg("IdxDb::doHousekeeping END (elapsed: %ld ms)",
                  stopperWatch.elapsedMSecs());
    }
  

  
    void IdxDb::dbgDumpState()
    {
        logTestMsg(CPIX_LL_TRACE,
                   "    DUMPING IdxDb instance: BEGIN",
                   reinterpret_cast<long>(this));

        logTestMsg(CPIX_LL_TRACE,
                   "    o  idxDbPath_ : %s",
                   indexDbPath_.c_str());

        char stateStr[] = "....";
        enum
        {
            SEARCHER_POS  = 0,
            READER_POS    = 1,
            DELETIONS_POS = 2,
            WRITER_POS    = 3
        };

        // TODO searcher position is never flipped to 's' state
        
        if (reader_.isOpen(NULL))
            {
                stateStr[READER_POS] = 'r';
                
                if (reader_.hasDeletions())
                    {
                        stateStr[DELETIONS_POS] = 'd';
                    }
            }

        { // SYNC
            Cpt::SyncRegion
            sr(idxMutex_);
            
            if (!insertBuf_.isEmpty())
                {
                    stateStr[WRITER_POS] = 'w';
                }

        } // SYNC

        logTestMsg(CPIX_LL_TRACE,
                   "    o  state (SRDW) %s",
                   stateStr);

        logTestMsg(CPIX_LL_TRACE,
                   "    DUMPING IdxDb instance: END.",
                   reinterpret_cast<long>(this));
    }


    void IdxDb::loadReader(Cpt::MultiSyncRegion  * msr)
    {
        if (!reader_.isOpen(msr))
            {
                reader_.load(indexDbPath_.c_str(),
                             idxMutex_);
            }
    }

        
    lucene::search::Hits * IdxDb::doSearch(lucene::search::Query * query,
                                            Version               * version,
                                            Cpt::MultiSyncRegion  * msr)
    {
        loadReader(msr);

        return reader_.search(query,
                              version);
    }


    Version IdxDb::getVersion(Cpt::MultiSyncRegion & msr)
    {
        return reader_.getVersion(msr);
    }


    lucene::index::IndexReader* IdxDb::getReader(Cpt::MultiSyncRegion * msr,
                                                  Version              * version)
    {
        loadReader(msr);

        return reader_.getReader(version);
    }


    void IdxDb::forceClose(const char * cpixDir)
    {
        try
            {
                // Do not be tempted to lock idxMutex_ manually here
                // and pass NULL to reader_.close(). The lock order of
                // nested locks in VersionedReader must be always the
                // same.
                reader_.close(&idxMutex_,
                              cpixDir);
                
                Cpt::SyncRegion
                    sr(idxMutex_);
                
                Impl::CallCloseLogging(insertBuf_);
            }
        catch (...)
            {
                logMsg(CPIX_LL_ERROR,
                       "IdxDb::forceClose got exception");

                // no re-throw at this point
            }
        
    }


}
