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


#include "cpixhits.h"
#include "idxdb.h"
#include "cpixexc.h"
#include "iqrytype.h"
#include "document.h"

#include "common/cpixlog.h"

namespace Cpix
{

    /**
     * Interface IHits
     */
    IHits::~IHits() 
    {
	;
    }


    void HitsBase::destroyWrappers(int32_t index)
    {
        WrapperMap::iterator
            i = wrappers_.begin();

        if  (index == -1) {
            for (; i != wrappers_.end(); ++i)
            {
                delete i->second;
            }
            wrappers_.erase(wrappers_.begin(),wrappers_.end());
        } else {
            for (; i != wrappers_.find(index); ++i)
            {
                delete i->second;
                
            }
            wrappers_.erase(wrappers_.begin(),wrappers_.find(index));
        }

        
    }


    HitsBase::HitsBase(bool docsOwnedByClucene)
        : docsOwnedByClucene_(docsOwnedByClucene)
    {
        ;
    }


    void HitsBase::wrapDocument(int32_t                      index,
                                lucene::document::Document * doc)
    {
//        if (wrappers_.find(index) != wrappers_.end())
//            {
//                delete wrappers_[index];
//                wrappers_[index] = NULL;
//            }
//
//        Cpix::Document
//            * cpixDoc = new Cpix::Document(doc,
//                                           docsOwnedByClucene_);
//        wrappers_[index] = cpixDoc;
        
        if (wrappers_.find(index) == wrappers_.end())
            {
                Cpix::Document
                    * cpixDoc = new Cpix::Document(doc,
                                                   docsOwnedByClucene_);
                wrappers_[index] = cpixDoc;
            }


    }


    Cpix::Document * HitsBase::getDocument(int32_t index)
    {
        Cpix::Document
            * rv = NULL;

        if (wrappers_.find(index) == wrappers_.end())
            {
                getDocument_(index);
            }

        rv = wrappers_[index];

        return rv;
    }

    int HitsBase::resetDocumentCache(int32_t index, int32_t count)
    {
        int32_t reqCount = index + count;
        int rv = 0;
        int i = (wrappers_.begin()->first)+ (int32_t) wrappers_.size();
        if ((i < reqCount) || (index < wrappers_.begin()->first))
            {
                getDocument_(index);
               
            }
        rv = (int32_t) wrappers_.size();
        return rv;
    }

    /**
     * Class ClhDocumentConsumer
     */
    int32_t ClhDocumentConsumer::beginIndex() const
    {
        return beginIndex_;
    }


    int32_t ClhDocumentConsumer::endIndex() const
    {
        return endIndex_;
    }


    void ClhDocumentConsumer::useDocument(int32_t                      index,
                                          lucene::document::Document * document)
    {
        clHits_.wrapDocument(index,
                             document);
    }


    void ClhDocumentConsumer::failedDocument(int32_t        index,
                                             int            clErrorNumber)
    {
        failedDocErrors_[index - beginIndex_] = clErrorNumber;
    }


    ClhDocumentConsumer::ClhDocumentConsumer(LuceneHits & clHits)
        : pageSize_(IdxDbMgr::instance()->getClHitsPageSize()),
          beginIndex_(UNUSED),
          endIndex_(UNUSED),
          clHits_(clHits)
    {
        failedDocErrors_.reserve(pageSize_);
        for (int i = 0; i < pageSize_; ++i)
            {
                failedDocErrors_.push_back(CL_ERR_UNKNOWN);
            }
    }


    void ClhDocumentConsumer::setPageBoundaryForIndex(int32_t index)
    {
        if (index >= clHits_.length())
            {
                THROW_CPIXEXC("Not a valid hit doc: %d (hits length is %d)",
                              index,
                              clHits_.length());
            }

        //beginIndex_ = index - (index % pageSize_);
        beginIndex_ = index;
        endIndex_ = beginIndex_ + pageSize_;

        endIndex_ = std::min(clHits_.length(),
                             endIndex_);

        for (int i = 0; i < pageSize_; ++i)
            {
                failedDocErrors_[i] = CL_ERR_UNKNOWN;
            }
    }

        
    void ClhDocumentConsumer::throwIfFailedDocIndex(int32_t index)
    {
        if (beginIndex_ == UNUSED)
            {
                // nothing has been fetched with this instance, so
                // nothing has failed yet
                return;
            }

        if (index < beginIndex_ || index >= endIndex_)
            {
                // we have not tried to load this document in this
                // batch
                return;
            }

        int32_t
            idx = index - beginIndex_;

        if (failedDocErrors_[idx] != CL_ERR_Success)
            {
                THROW_CPIXEXC("Failed to fetch hit document %d, CL error code: %d (cf. clucene debug/error.h)",
                              index,
                              failedDocErrors_[idx]);
            }
        else
            {
                THROW_CPIXEXC(PL_ERROR "Reloading a document should not be necessary %d",
                              index);
            }
    }





    /**
     * Class LuceneHits wrapping lucene::document::Hits
     */
    LuceneHits::LuceneHits(lucene::search::Hits  * hits, 
                             lucene::search::Query * query,
                             IdxDbHndl               idxDb,
                             Version                 idxDbVersion)
	: HitsBase(true), // docs in here are owned by clucene
          hits_(hits),
          length_(hits != NULL ? hits->length() : 0),
	  query_(query),  // increase reference count
          idxDb_(idxDb),
          idxDbVersion_(idxDbVersion),
          docConsumer_(NULL)
    {
        docConsumer_ = new ClhDocumentConsumer(*this);

        IdxDbMgr::instance()->incRefHndl(idxDb_);

        // pre-fetching the first page - assumption is that first
        // document to get will be #0 anyway, and no need for IPC +
        // ctxt switch with client by which time an idx operation may
        // have upset everything.
        //
        // TODO the dependent idxdb locks are released by this point,
        // so getDocument_ will have to re-acquire all locks, which is
        // not only OS operation but may bump into already modified
        // idx-es. Idea: IIdxDb::search could be reorganized into
        // fetchRecommitting() instead - the first fetch operations
        // would be performed while locks are still in place...
        if (length_ > 0)
            {
                getDocument_(0);
            }
    }
	
    LuceneHits::~LuceneHits() 
    {
        delete hits_; 
        hits_ = 0; 
        
        destroyWrappers(-1);

        delete docConsumer_;

        IdxDbMgr::instance()->releaseHndl(idxDb_);
    }


    void LuceneHits::getDocument_(int32_t index) 
    {
        // check if we have tried to fetch it before and failed
        //docConsumer_->throwIfFailedDocIndex(index);

        // common usage pattern is to enumerate hits, never to access
        // them at true random manner. That means that we never need
        // wrappers for more than the current page - ie when about to
        // fetch the next page we can discard all other wrapped
        // documents. Merely an memory consumption optimization
        // measure, can be commented out.
        destroyWrappers(index);

        // if we get here, then it means we don't have the page of hit
        // docs we need, but it may still throw if the hits is an
        // empty object or the hits itself was NULL
        docConsumer_->setPageBoundaryForIndex(index);

        IIdxDb* idx = IIdxDb::getPtr( idxDb_ );

        lucene::search::Hits
            * currentHits = hits_;

        hits_ = 0;

        // the version number of idx that produced currentHits is
        // either still the same in which case we get back
        // currentHits, or it has changed and in that case currentHits
        // will be destroyed by this call and a new one will be
        // returned - rv will recieve the pointer to the fetched document
        hits_ = idx->fetchRecommitting(currentHits,
                                       &idxDbVersion_,
                                       query_,
                                       *docConsumer_);
        
        length_ = hits_->length();

        if (wrappers_.find(index) == wrappers_.end())
            {
                THROW_CPIXEXC("Failed to fetch doc %d: TODO reason",
                              index);
            }
    }


    int32_t LuceneHits::length()
    {
        return length_;
    }

    
    /**
     * HitDocumentList (for suggestion feature)
     */
    HitDocumentList::HitDocumentList()
	: HitsBase(false) // docs owned by this
    {
        ;
    }
	
    HitDocumentList::~HitDocumentList()
    {
        destroyWrappers(-1);
        for (std::vector<lucene::document::Document*>::iterator i = documents_.begin(); 
             i != documents_.end(); 
             i++) 
            {
                _CLDELETE(*i);
            }
    }
    
    void HitDocumentList::getDocument_(int32_t index)
    {
        if (index >= 0 && index < documents_.size()) 
            {
                wrapDocument(index,
                             documents_[index]);
            }
        else 
            {
                THROW_CPIXEXC(L"Hit document index '%d' out of bounds",
                              index);
            }
    }

    int32_t HitDocumentList::length()
    {
        return documents_.size(); 
    }


    void HitDocumentList::add(lucene::document::Document* document)
    {
        documents_.push_back(document); 
    }
	

    void HitDocumentList::remove(int index)
    {
        documents_.erase(documents_.begin()+index); 
    }





}
