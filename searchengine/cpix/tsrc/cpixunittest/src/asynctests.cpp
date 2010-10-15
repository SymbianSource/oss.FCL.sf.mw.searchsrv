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

#include <wchar.h>
#include <stddef.h>
#include <unistd.h>

#include <pthread.h>

#include <algorithm>
#include <iostream>
#include <string>

#include "cpixtools.h"

#include "itk.h"

#include "cpix_async.h"

#include "config.h"
#include "suggestion.h"
#include "testutils.h"
#include "testcorpus.h"
#include "setupsentry.h"
#include "testutils.h"
#include "std_log_result.h"

// TODO PROPER, EXAMPLARY error clearing (cpix_ClearError())


bool AddField(cpix_Document * doc,
              const wchar_t * name,
              const wchar_t * value,
              int             flags)
{
    bool
        rv = false;

    cpix_Field
        field;

    cpix_Field_initialize(&field,
                          name,
                          value,
                          flags);

    if (cpix_Succeeded(&field))
        {
            cpix_Document_add(doc,
                              &field);

            if (cpix_Succeeded(doc))
                {
                    rv = true;
                }
            else
                {
                    cpix_Field_release(&field);
                }
        }

    return rv;
}


cpix_Document * CreateSmsDoc(Itk::TestMgr  * testMgr,
                             size_t          docUid,
                             const wchar_t * body,
                             Cpt::Mutex    & cpixMutex)
{
    using namespace std;

    wstring 
        docUidStr = GetItemId(docUid); 
    cpix_Result
        result;

    Cpt::SyncRegion
        sr(cpixMutex);

    cpix_Document
        * doc = cpix_Document_create(&result,
                                     docUidStr.c_str(),
                                     SMSAPPCLASS,
                                     body,
                                     NULL);
    ITK_ASSERT(testMgr,
               cpix_Succeeded(&result),
               "Failed to create document for SMS");

    bool
        success = AddField(doc,
                           LTO_FIELD,
                          L"+3585554444",
                          cpix_STORE_YES | cpix_INDEX_UNTOKENIZED);

    if (success)
        {
            success = AddField(doc,
                               LBODY_FIELD,
                               body,
                               cpix_STORE_NO | cpix_INDEX_TOKENIZED);
        }

    if (!success)
        {
            cpix_Document_destroy(doc);

            ITK_ASSERT(testMgr,
                       false,
                       "Could not add field(s) to SMS doc");
        }

    return doc;
}
                              

// TODO these SyncedAdd operations must evolve to lock on Itk::TestMgr
// * instance (or have a synced Itk::TestMgr derivant class)



/**
 * Reports the CPIX error if the last operation on cpix_Obj
 * failed. Also clears the error status on cpix_Obj (necessary for
 * error info lifetime mgmt).
 *
 * @return if the last operation cpix_Obj was successful, that is,
 * it's return valus is equivalent to calling
 * cpix_Succeeded(cpix_Obj). (But note, that this call clears the
 * error status, thus a cpix_Succeeded(cpix_Obj) call after a call to
 * this function will alsways return true).
 */
template <typename CPIX_OBJ>
bool ReportIfFailed(Itk::TestMgr * testMgr,
                    CPIX_OBJ     * cpix_Obj,
                    Cpt::Mutex   & cpixMutex)
{
    bool
        succeeded = true;

    if (cpix_Failed(cpix_Obj))
        {
            succeeded = false;

            wchar_t
                buffer[256];

            Cpt::SyncRegion
                sr(cpixMutex);

            cpix_Error_report(cpix_Obj->err_,
                              buffer,
                              sizeof(buffer) / sizeof(wchar_t));
            ITK_MSG(testMgr,
                    "CPIX failure: %S",
                    buffer);
            cpix_ClearError(cpix_Obj);
        }

    return succeeded;
}



/**
 * Adds an SMS item to an index using the async API, but it is used as
 * a sync API here.
 */
class SyncedAdd
{
    Cpt::SyncQueue<cpix_JobId>   q_;

    Cpt::Mutex                 & cpixMutex_;

public:
    
    SyncedAdd(Cpt::Mutex & cpixMutex)
        : q_(1),
          cpixMutex_(cpixMutex)
    {
        ;
    }


    void call(cpix_IdxDb    * idxDb,
              cpix_Document * doc,
              cpix_Analyzer * analyzer,
              Itk::TestMgr  * testMgr)
    {
        cpix_JobId
            jobId;

        {
            Cpt::SyncRegion
                sr(cpixMutex_);

            jobId = cpix_IdxDb_asyncAdd(idxDb,
                                        doc,
                                        analyzer,
                                        this,
                                        &callback);
        }

        ITK_ASSERT(testMgr,
                   ReportIfFailed(testMgr,
                                  idxDb,
                                  cpixMutex_),
                   "Making async call failed");

        cpix_JobId
            jobId2;

        bool 
            couldGet = q_.get(&jobId2);
        
        ITK_ASSERT(testMgr,
                   jobId == jobId2,
                   "Messed up callback!");

        if (couldGet)
            {
                {
                    Cpt::SyncRegion
                        sr(cpixMutex_);
                    
                    cpix_IdxDb_asyncAddResults(idxDb,
                                               jobId);
                }

                ITK_EXPECT(testMgr,
                           ReportIfFailed(testMgr,
                                          idxDb,
                                          cpixMutex_),
                           "Synced addition failed");

                if (cpix_Succeeded(idxDb))
                    {
                        ITK_MSG(testMgr,
                                "Indexed");
                    }
            }
        else
            {
                ITK_ASSERT(testMgr,
                           false,
                           "Could not get result from async call");
            }
    }
     

private:
    static void callback(void       * cookie,
                         cpix_JobId   jobId)
    {
        SyncedAdd
            * this_ = reinterpret_cast<SyncedAdd*>(cookie);

        this_->q_.put(jobId);
    }
};



/**
 * Deletes an SMS item from an index using the async API, but it is used as
 * a sync API here.
 */
class SyncedDel
{
    Cpt::SyncQueue<cpix_JobId>   q_;
    Cpt::Mutex                 & cpixMutex_;


public:
    
    SyncedDel(Cpt::Mutex & cpixMutex)
        : q_(1),
          cpixMutex_(cpixMutex)
    {
        ;
    }


    void call(cpix_IdxDb    * idxDb,
              const wchar_t * docUid,
              Itk::TestMgr  * testMgr)
    {
        cpix_JobId
            jobId;

        {
            Cpt::SyncRegion
                sr(cpixMutex_);

            jobId = cpix_IdxDb_asyncDeleteDocuments(idxDb,
                                                    docUid,
                                                    this,
                                                    &callback);
        }

        ITK_ASSERT(testMgr,
                   ReportIfFailed(testMgr,
                                  idxDb,
                                  cpixMutex_),
                   "could not make async call");

        cpix_JobId
            jobId2;

        bool
            couldGet = q_.get(&jobId2);

        ITK_ASSERT(testMgr,
                   jobId == jobId2,
                   "Messed up callback");

        if (couldGet)
            {
                int32_t
                    rv;

                {
                    Cpt::SyncRegion
                        sr(cpixMutex_);

                    rv = cpix_IdxDb_asyncDeleteDocumentsResults(idxDb,
                                                                jobId);
                }

                ITK_EXPECT(testMgr,
                           ReportIfFailed(testMgr,
                                          idxDb,
                                          cpixMutex_),
                           "Synced delete failed");

                if (cpix_Succeeded(idxDb))
                    {
                        ITK_MSG(testMgr,
                                "Deleted %d",
                                rv);
                    }
            }
        else
            {
                ITK_ASSERT(testMgr,
                           false,
                           "Could not get result from async call");
            }
    }
     

private:
    static void callback(void       * cookie,
                         cpix_JobId   jobId)
    {
        SyncedDel
            * this_ = reinterpret_cast<SyncedDel*>(cookie);

        this_->q_.put(jobId);
    }
};



/**
 * Updates an SMS item to an index using the async API, but it is used as
 * a sync API here.
 */
class SyncedUpdate
{
    Cpt::SyncQueue<cpix_JobId>   q_;

    Cpt::Mutex                 & cpixMutex_;

public:
    
    SyncedUpdate(Cpt::Mutex & cpixMutex)
        : q_(1),
          cpixMutex_(cpixMutex)
    {
        ;
    }


    void call(cpix_IdxDb    * idxDb,
              cpix_Document * doc,
              cpix_Analyzer * analyzer,
              Itk::TestMgr  * testMgr)
    {
        cpix_JobId
            jobId;

        {
            Cpt::SyncRegion
                sr(cpixMutex_);

            jobId = cpix_IdxDb_asyncUpdate(idxDb,
                                           doc,
                                           analyzer,
                                           this,
                                           &callback);
        }

        ITK_ASSERT(testMgr,
                   ReportIfFailed(testMgr,
                                  idxDb,
                                  cpixMutex_),
                   "Could not make async call");

        cpix_JobId
            jobId2;

        bool
            couldGet = q_.get(&jobId2);

        ITK_ASSERT(testMgr,
                   jobId == jobId2,
                   "Messed up callback");

        if (couldGet)
            {
                {
                    Cpt::SyncRegion
                        sr(cpixMutex_);
                    cpix_IdxDb_asyncUpdateResults(idxDb,
                                                  jobId);
                }
                    
                ITK_EXPECT(testMgr,
                           ReportIfFailed(testMgr,
                                          idxDb,
                                          cpixMutex_),
                           "Synced update failed");

                if (cpix_Succeeded(idxDb))
                    {
                        ITK_MSG(testMgr,
                                "Updated");
                    }
            }
        else
            {
                ITK_ASSERT(testMgr,
                           false,
                           "Could not get result from async call");
            }
    }
     

private:
    static void callback(void       * cookie,
                         cpix_JobId   jobId)
    {
        SyncedUpdate
            * this_ = reinterpret_cast<SyncedUpdate*>(cookie);

        this_->q_.put(jobId);
    }
};



class SyncedHousekeep
{
    Cpt::SyncQueue<cpix_JobId>   q_;
    Cpt::Mutex                 & cpixMutex_;


public:
    SyncedHousekeep(Cpt::Mutex & cpixMutex)
        : q_(1),
          cpixMutex_(cpixMutex)
    {
        ;
    }


    void call(Itk::TestMgr   * testMgr)
    {
        cpix_JobId
            jobId;

        cpix_Result
            result;

        {
            Cpt::SyncRegion
                sr(cpixMutex_);

            jobId = cpix_asyncDoHousekeeping(&result,
                                             this,
                                             &callback);
        }

        ITK_ASSERT(testMgr,
                   ReportIfFailed(testMgr,
                                  &result,
                                  cpixMutex_),
                   "Performing async housekeeping failed");

        cpix_JobId
            jobId2;

        bool
            couldGet = q_.get(&jobId2);

        ITK_ASSERT(testMgr,
                   jobId == jobId2,
                   "Messed up callback!");

        if (couldGet)
            {
                {
                    Cpt::SyncRegion
                        sr(cpixMutex_);

                    cpix_asyncDoHousekeepingResults(&result,
                                                    jobId);

                    ITK_EXPECT(testMgr,
                               ReportIfFailed(testMgr,
                                              &result,
                                              cpixMutex_),
                               "Synced doHousekeeping failed");

                    if (cpix_Succeeded(&result))
                        {
                            ITK_MSG(testMgr,
                                    "Housekeeping done");
                        }
                }
            }
        else
            {
                ITK_ASSERT(testMgr,
                           false,
                           "Could not get result from async call");
            }
    }



private:
    static void callback(void               * cookie,
                         cpix_JobId           jobId)
    {
        SyncedHousekeep
            * this_ = reinterpret_cast<SyncedHousekeep*>(cookie);

        this_->q_.put(jobId);
    }


};



class SyncedDoc
{
    Cpt::SyncQueue<cpix_JobId>   q_;

    Cpt::Mutex                 & cpixMutex_;

public:
    SyncedDoc(Cpt::Mutex & cpixMutex)
        : q_(1),
          cpixMutex_(cpixMutex)
    {
        ;
    }


    bool call(cpix_Hits     * hits,
              int32_t         index,
              cpix_Document ** target,
              Itk::TestMgr  * testMgr,
              bool            cancel)
    {
        bool
            rv = false;

        cpix_JobId
            jobId;

        {
            Cpt::SyncRegion
                sr(cpixMutex_);

            jobId = cpix_Hits_asyncDoc(hits,
                                       index,
                                       target,
                                       this,
                                       &callback,
                                       1);
        }
        
        ITK_ASSERT(testMgr,
                   ReportIfFailed(testMgr,
                                  hits,
                                  cpixMutex_),
                   "Could not make async doc retrieval");

        if (cancel)
            {
                cpix_CancelAction
                    cancelAction = cpix_CANCEL_ERROR;

                {
                    Cpt::SyncRegion
                        sr(cpixMutex_);
                    
                    cancelAction = cpix_Hits_cancelDoc(hits,
                                                       jobId);
                }

                ReportIfFailed(testMgr,
                               hits,
                               cpixMutex_);

                ITK_MSG(testMgr,
                        "Doc retrieval CANCELLED (%d)",
                        cancelAction);

                // !!! Even if we did call cancel(), the callback
                //     might have hitted us before the cancel()
                //     returned.  we have to clear the status, but we
                //     must not collect any results.
                if (q_.full())
                    {
                        ITK_MSG(testMgr,
                                "Clearing cancelled callback status...\n");
                        cpix_JobId
                            jobId2;
                        
                        bool
                            couldGet = q_.get(&jobId2);
                        
                        ITK_ASSERT(testMgr,
                                   jobId = jobId2,
                                   "Messed up callback");
                        ITK_MSG(testMgr,
                                "... clearing cancelled callback status done\n");
                    }
            }
        else
            {
                cpix_JobId
                    jobId2;

                bool
                    couldGet = q_.get(&jobId2);

                ITK_ASSERT(testMgr,
                           jobId == jobId2,
                           "Messed up callback");

                if (couldGet)
                    {
                        {
                            Cpt::SyncRegion
                                sr(cpixMutex_);
                            
                            cpix_Hits_asyncDocResults(hits,
                                                      jobId);
                        }

                        ITK_EXPECT(testMgr,
                                   ReportIfFailed(testMgr,
                                                  hits,
                                                  cpixMutex_),
                                   "Synced doc retrieval failed");

                        if (cpix_Succeeded(hits))
                            {
                                ITK_MSG(testMgr,
                                        "Doc is retrieved");
                                rv = true;
                            }
                    }
                else
                    {
                        // not assert: this can happen
                        ITK_MSG(testMgr,
                                false,
                                "Could not get result from async hit doc");
                    }
            }

        return rv;
    }

private:
    static void callback(void       * cookie,
                         cpix_JobId   jobId)
    {
        SyncedDoc
            * this_ = reinterpret_cast<SyncedDoc*>(cookie);

        this_->q_.put(jobId);
    }

};





class SyncedSearch
{
    Cpt::SyncQueue<cpix_JobId>    q_;

    SyncedDoc                     syncedDoc_;

    cpix_Document                 **targetDoc_;

    Cpt::Mutex                  & cpixMutex_;

public:
    
    SyncedSearch(Cpt::Mutex & cpixMutex)
        : q_(1),
          syncedDoc_(cpixMutex),
          cpixMutex_(cpixMutex)
    {
        ALLOC_DOC(targetDoc_, 1);
    }
    
    ~SyncedSearch() {
        FREE_DOC(targetDoc_, 1);
    }
    
    void call(cpix_IdxSearcher  * searcher,
              cpix_Query        * query,
              Itk::TestMgr      * testMgr,
              bool                cancel)
    {
        // cancel = false;

        cpix_JobId
            jobId;

        {
            Cpt::SyncRegion
                sr(cpixMutex_);

            jobId = cpix_IdxSearcher_asyncSearch(searcher,
                                                 query,
                                                 this,
                                                 &callback);
        }

        ITK_ASSERT(testMgr,
                   ReportIfFailed(testMgr,
                                  searcher,
                                  cpixMutex_),
                   "Could not make async call");

        if (cancel)
            {
                cpix_CancelAction
                    cancelAction = cpix_CANCEL_ERROR;

                {
                    Cpt::SyncRegion
                        sr(cpixMutex_);

                    cancelAction = cpix_IdxSearcher_cancelSearch(searcher,
                                                                 jobId);
                }

                ReportIfFailed(testMgr,
                               searcher,
                               cpixMutex_);
                ITK_MSG(testMgr,
                        "Search/terms query CANCELLED (%d)",
                        cancelAction);

                // !!! Even if we did call cancel(), the callback
                //     might have hitted us before the cancel()
                //     returned.  we have to clear the status, but we
                //     must not collect any results.
                if (q_.full())
                    {
                        ITK_MSG(testMgr,
                                "Clearing cancelled callback status...\n");
                        cpix_JobId
                            jobId2;
                        
                        bool
                            couldGet = q_.get(&jobId2);
                        
                        ITK_ASSERT(testMgr,
                                   jobId = jobId2,
                                   "Messed up callback");
                        ITK_MSG(testMgr,
                                "... clearing cancelled callback status done\n");
                    }
            }
        else
            {

                cpix_JobId
                    jobId2;

                bool
                    couldGet = q_.get(&jobId2);

                ITK_ASSERT(testMgr,
                           jobId = jobId2,
                           "Messed up callback");

                if (couldGet)
                    {
                        cpix_Hits
                            * hits = NULL;

                        {
                            Cpt::SyncRegion
                                sr(cpixMutex_);
                            
                            hits = cpix_IdxSearcher_asyncSearchResults(searcher,
                                                                       jobId);
                        }

                        ITK_EXPECT(testMgr,
                                   ReportIfFailed(testMgr,
                                                  searcher,
                                                  cpixMutex_),
                                   "Synced search failed");

                        if (cpix_Succeeded(searcher))
                            {
                                ITK_MSG(testMgr,
                                        "Search/terms query SUCCEEDED");

                                printSome(hits,
                                          testMgr);
                            }

                        {
                            Cpt::SyncRegion
                                sr(cpixMutex_);
                            cpix_Hits_destroy(hits);
                        }
                    }
                else
                    {
                        ITK_ASSERT(testMgr,
                                   false,
                                   "Could not get result from async call");
                    }
            }
    }
     

private:

    
    void printSome(cpix_Hits    * hits,
                   Itk::TestMgr * testMgr)
    {
        int32_t
            length;

        {
            Cpt::SyncRegion
                sr(cpixMutex_);

            length = cpix_Hits_length(hits);
        }

        ITK_MSG(testMgr,
                "Num of hits: %d",
                length);

        enum 
        { 
            // The setup of havig SOME=10 and cancelling doc retrieval
            // operations for doc indexes on the condition "(i%4) ==
            // 1" is special because this way
            //
            // (a) sooner or later there is going to be an indexing
            //     operation during doc enumeration, invalidating the
            //     hits
            //
            // (b) the last doc retrieval (index 9) is cancelled,
            //     after which this hits is destroyed
            //
            // (c) this situation previously has crashed CPix
            SOME = 10
        };

        for (int32_t i = 0; i < SOME && i < length; ++i)
            {
                bool
                    succeeded = syncedDoc_.call(hits,
                                                i,
                                                targetDoc_,
                                                testMgr,
                                                (i%4) == 1); // cancel or not

                if (succeeded)
                    {
                        Cpt::SyncRegion
                            sr(cpixMutex_);

                        PrintHit(targetDoc_[0],
                                 testMgr);
                    }
            }
    }



    static void callback(void       * cookie,
                         cpix_JobId   jobId)
    {
        SyncedSearch
            * this_ = reinterpret_cast<SyncedSearch*>(cookie);

        this_->q_.put(jobId);
    }
    

};






class ASyncContext : public Itk::ITestContext
{
private:
    cpix_IdxDb       * idxDb1_;
    cpix_IdxDb       * idxDb2_;
    cpix_IdxSearcher * searcher1_;
    cpix_IdxSearcher * searcher2_;
    cpix_Analyzer    * analyzerI_; // for indexing thread
    cpix_Analyzer    * analyzerQ_; // for query thread
    cpix_QueryParser * queryParser_;
    cpix_Query       * searchQuery_;
    cpix_Query       * termsQuery_;
    
    LineTestCorpusRef  corpus_;

    /* OBS
    // TODO currently protecting both 
    //   (a) CPix (non-thread safe) and
    //   (b) Itk::TestMgr
    //
    // turn Itk::TestMgr into protected later, this tempMutex_ is to
    // be renamed, and it should protect individual calls (async and
    // sync alike) to cpix within these tests
    Cpt::Mutex         tempMutex_;
    */

    /**
     * CPix itself is to be invoked from a single thread (API is not
     * multithreaded). However, for sake of clarity, we test it from
     * multiple threads (indexing and searching client threads). So
     * they need to be synchronized - no two threads should be
     * executing an sync / async call at the same time.
     */
    Cpt::Mutex         cpixMutex_;

    bool               keepIndexing_;

    Itk::TestMgr     * testMgr_;

    /**
     * Statistics POV
     */
    struct Stats
    {
        std::string    name_;
        size_t         opCount_;
        long           opSumMs_;

        Stats(const char * name)
            : name_(name),
              opCount_(0),
              opSumMs_(0)
        {
            ;
        }


        void update(Cpt::StopperWatch & stopperWatch)
        {
            ++opCount_;
            opSumMs_ += stopperWatch.elapsedMSecs();
        }

        
        void output(Itk::TestMgr * testMgr)
        {
            if (opCount_ > 0)
                {
                    ITK_REPORT(testMgr,
                               name_.c_str(),
                               "%ld ops in %ld ms (%.3f ms/op)",
                               opCount_,
                               opSumMs_,
                               static_cast<double>(opSumMs_) 
                               / static_cast<double>(opCount_));
                }
            else
                {
                    ITK_REPORT(testMgr,
                               name_.c_str(),
                               "%ld ops in %ld ms (### ms/op)",
                               opCount_,
                               opSumMs_);
                }
        }


        void reset()
        {
            opCount_ = 0;
            opSumMs_ = 0;
        }
    };


    // statistics for indexing
    Stats              addStats_;
    Stats              delStats_;
    Stats              updateStats_;

    // statistics for search
    Stats              searchStats_;
    Stats              termsStats_;

public:
    virtual void setup() throw (Itk::PanicExc)
    {
        SetupSentry
            ss(*this);

        cpix_Result
            result;

        cpix_IdxDb_dbgScrapAll(&result);
        if (cpix_Failed(&result))
            {
                ITK_PANIC("Could not dbg scrapp all indexes");
            }

        const Volume
            *p = Volumes;

        for (; p->qbac_ != NULL; ++p)
            {
                cpix_IdxDb_defineVolume(&result,
                                        p->qbac_,
                                        p->path_);

                if (cpix_Failed(&result))
                    {
                        ITK_PANIC("Could not define volume '%s'",
                                  p->qbac_);
                    }
            }

        cpix_IdxDb_defineVolume(&result,
                                SMS_QBASEAPPCLASS,
                                NULL);

        if (cpix_Failed(&result))
            {
                ITK_PANIC("Could not define index for SMS");
            }

        idxDb1_ = cpix_IdxDb_openDb(&result,
                                    SMS_QBASEAPPCLASS,
                                    cpix_IDX_CREATE);
        if (cpix_Failed(&result))
            {
                ITK_PANIC("Could not open(create) index for SMS");
            }

        static const char
            sms2Qbac[] = SMS_QBASEAPPCLASS "2";

        cpix_IdxDb_defineVolume(&result,
                                sms2Qbac,
                                NULL);

        if (cpix_Failed(&result))
            {
                ITK_PANIC("Could not define index for SMS(2)");
            }

        idxDb2_ = cpix_IdxDb_openDb(&result,
                                    sms2Qbac,
                                    cpix_IDX_CREATE);
        if (cpix_Failed(&result))
            {
                ITK_PANIC("Could not open(create) index for SMS(2)");
            }

        searcher1_ = cpix_IdxSearcher_openDb(&result,
                                             "root msg");
        if (searcher1_ == NULL)
            {
                ITK_PANIC("Could not create searcher");
            }

        searcher2_ = cpix_IdxSearcher_openDb(&result,
                                             "root");
        if (searcher2_ == NULL)
            {
                ITK_PANIC("Could not create searcher");
            }
        
        analyzerI_ = cpix_CreateSimpleAnalyzer(&result);
        
        if (analyzerI_ == NULL)
            {
                ITK_PANIC("Could not create analyzer");
            }

        analyzerQ_ = cpix_CreateSimpleAnalyzer(&result);
        
        if (analyzerQ_ == NULL)
            {
                ITK_PANIC("Could not create analyzer");
            }

        queryParser_ = cpix_QueryParser_create(&result,
                                               LCPIX_DEFAULT_FIELD,
                                               analyzerQ_);
        if (queryParser_ == NULL)
            {
                ITK_PANIC("Could not create query parser");
            }
        
        searchQuery_ = cpix_QueryParser_parse(queryParser_,
                                              L"happy");
        if (cpix_Failed(queryParser_)
            || searchQuery_ == NULL)
            {
                ITK_PANIC("Could not parse search query");
            }

        cpix_QueryParser_destroy(queryParser_);
        queryParser_ = NULL;

        queryParser_ = cpix_QueryParser_create(&result,
                                               L"SecondaryTokens",
                                               analyzerQ_);
        if (queryParser_ == NULL)
            {
                ITK_PANIC("Could not create query parser");
            }

        termsQuery_ = cpix_QueryParser_parse(queryParser_,
                                             L"ha* OR _aggregate:happy");
        if (cpix_Failed(queryParser_)
            || termsQuery_ == NULL)
            {
                ITK_PANIC("Could not parse terms query");
            }

        ss.setupComplete();
    }


    virtual void tearDown() throw()
    {
        cleanup();
    }


    virtual ~ASyncContext()
    {
        cleanup();
    }


    //
    // public operations
    //
    ASyncContext()
        : idxDb1_(NULL),
          idxDb2_(NULL),
          searcher1_(NULL),
          searcher2_(NULL),
          analyzerI_(NULL),
          analyzerQ_(NULL),
          queryParser_(NULL),
          searchQuery_(NULL),
          termsQuery_(NULL),
          corpus_(DEFAULT_TEST_CORPUS_PATH),
          keepIndexing_(true),
          addStats_("adding"),
          delStats_("deleting"),
          updateStats_("updating"),
          searchStats_("searching"),
          termsStats_("term-qrying")
    {
        ;
    }


    
    void testSingleThreadIdx(Itk::TestMgr * testMgr)
    {
        testMgr_ = testMgr;

        // cpix_setLogLevel(CPIX_LL_DEBUG);

        pthread_t
            indexerThreadHndl;

        void
            * rv;

        int
            result = pthread_create(&indexerThreadHndl,
                                    NULL,
                                    &IndexerThreadFunc,
                                    this);
        ITK_ASSERT(testMgr,
                   result == 0,
                   "Could not create POSIX thread for indexing");

        printf("MAIN: Created indexer thread\n");

        printf("MAIN: waiting for worker thread (indexer)\n");

        result = pthread_join(indexerThreadHndl,
                              &rv);
        
        ITK_EXPECT(testMgr,
                   result == 0,
                   "Could not join indexer thread");


        printStatistics();
    }
    


    void testSingleThreadQry(Itk::TestMgr * testMgr)
    {
        testMgr_ = testMgr;

        // cpix_setLogLevel(CPIX_LL_DEBUG);

        pthread_t
            searcherThreadHndl;

        void
            * rv;

        int
            result = pthread_create(&searcherThreadHndl,
                                    NULL,
                                    &SearcherThreadFunc,
                                    this);
        ITK_ASSERT(testMgr,
                   result == 0,
                   "Could not create POSIX thread for searching");

        printf("MAIN: Created searcher thread\n");

        printf("MAIN: waiting for worker thread (searcher)\n");

        result = pthread_join(searcherThreadHndl,
                              &rv);

        ITK_EXPECT(testMgr,
                   result == 0,
                   "Could not join searcher thread");

       
        // cpix_setLogL       printStatistics();
    }



    void testMultiThreads(Itk::TestMgr * testMgr)
    {
        cleanup();
        setup();

        testMgr_ = testMgr;

        // cpix_setLogLevel(CPIX_LL_DEBUG);

        pthread_t
            indexerThreadHndl,
            searcherThreadHndl;

        void
            * rv;

        int
            result = pthread_create(&indexerThreadHndl,
                                    NULL,
                                    &IndexerThreadFunc,
                                    this);
        ITK_ASSERT(testMgr,
                   result == 0,
                   "Could not create POSIX thread for indexing");

        printf("MAIN: Created indexer thread\n");

        result = pthread_create(&searcherThreadHndl,
                                NULL,
                                &SearcherThreadFunc,
                                this);

        if (result != 0)
            {
                keepIndexing_ = false;

                result = pthread_join(indexerThreadHndl,
                                      &rv);
                ITK_EXPECT(testMgr,
                           result == 0,
                           "Could not force closure of indexer thread");
                ITK_ASSERT(testMgr,
                           false,
                           "Could not create POSIX thread for searching");
            }


        printf("MAIN: Created searcher thread\n");
        
        printf("MAIN: waiting for both worker threads\n");

        result = pthread_join(indexerThreadHndl,
                              &rv);
        
        ITK_EXPECT(testMgr,
                   result == 0,
                   "Could not join indexer thread");

        
        result = pthread_join(searcherThreadHndl,
                              &rv);

        ITK_EXPECT(testMgr,
                   result == 0,
                   "Could not join searcher thread");

       
        // cpix_setLogLevel(CPIX_LL_TRACE);

 
    }


private:
    void cleanup()
    {
        cpix_IdxDb_releaseDb(idxDb1_);
        idxDb1_ = NULL;

        cpix_IdxDb_releaseDb(idxDb2_);
        idxDb2_ = NULL;
        
        cpix_IdxSearcher_releaseDb(searcher1_);
        searcher1_ = NULL;

        cpix_IdxSearcher_releaseDb(searcher2_);
        searcher2_ = NULL;

        cpix_Analyzer_destroy(analyzerI_);
        analyzerI_ = NULL;

        cpix_Analyzer_destroy(analyzerQ_);
        analyzerQ_ = NULL;

        cpix_QueryParser_destroy(queryParser_);
        queryParser_ = NULL;

        cpix_Query_destroy(searchQuery_);
        searchQuery_ = NULL;

        cpix_Query_destroy(termsQuery_);
        termsQuery_ = NULL;
    }

    
    void printStatistics()
    {
        addStats_.output(testMgr_);
        delStats_.output(testMgr_);
        updateStats_.output(testMgr_);
        searchStats_.output(testMgr_);
        termsStats_.output(testMgr_);
    }


    void * indexStuff()
    {
        using namespace std;
        using namespace Cpt;

        enum 
        { ROUNDS        = 200, 
          SAMPLE_START  = 180,
          ACTION_PERIOD = 5,
        };

        Cpt::StopperWatch
            stopperWatch;

        SyncedAdd
            syncedAdd(cpixMutex_);
        SyncedDel
            syncedDel(cpixMutex_);
        SyncedUpdate
            syncedUpdate(cpixMutex_);
        SyncedHousekeep
            syncedHousekeep(cpixMutex_);

        try
            {
                int
                    curId = 0;
                
                for (size_t i = 0; i < ROUNDS && keepIndexing_; ++i)
                    {
                        if (i == SAMPLE_START)
                            {
                                addStats_.reset();
                                delStats_.reset();
                                updateStats_.reset();
                            }

                        ++curId;
                        std::wstring
                            body(corpus_.item(curId));
                        
                        stopperWatch.reset();

                        cpix_Document
                            * doc = CreateSmsDoc(testMgr_,
                                                 curId,
                                                 body.c_str(),
                                                 cpixMutex_);
                        
                        syncedAdd.call(idxDb1_,
                                       doc,
                                       analyzerI_,
                                       testMgr_);

                        {
                            Cpt::SyncRegion
                                sr(cpixMutex_);
                            cpix_Document_destroy(doc);
                            doc = NULL;
                        }

                        addStats_.update(stopperWatch);

                        stopperWatch.reset();

                        doc = CreateSmsDoc(testMgr_,
                                           curId,
                                           body.c_str(),
                                           cpixMutex_);
                        
                        syncedAdd.call(idxDb2_,
                                       doc,
                                       analyzerI_,
                                       testMgr_);

                        {
                            Cpt::SyncRegion
                                sr(cpixMutex_);
                            cpix_Document_destroy(doc);
                            doc = NULL;
                        }

                        addStats_.update(stopperWatch);

                        if (curId % ACTION_PERIOD == 0)
                            {
                                stopperWatch.reset();

                                wstring
                                    docUid = GetItemId(curId/ACTION_PERIOD);

                                syncedDel.call(idxDb1_,
                                               docUid.c_str(),
                                               testMgr_);

                                delStats_.update(stopperWatch);

                                stopperWatch.reset();

                                doc = CreateSmsDoc(testMgr_,
                                                   curId/ACTION_PERIOD,
                                                   L"happy bla",
                                                   cpixMutex_);

                                syncedUpdate.call(idxDb2_,
                                                  doc,
                                                  analyzerI_,
                                                  testMgr_);
                                
                                {
                                    Cpt::SyncRegion
                                        sr(cpixMutex_);
                                    cpix_Document_destroy(doc);
                                    doc = NULL;
                                }

                                updateStats_.update(stopperWatch);

                                ITK_DBGMSG(testMgr_, "+");

                                syncedHousekeep.call(testMgr_);
                            }
                        else
                            {
                                ITK_DBGMSG(testMgr_, ".");
                            }
                    }
            }
        catch (...)
            {
                
                ITK_EXPECT(testMgr_,
                           false,
                           "INDEXER: Failed indexing\n");
                throw;
            }

        syncedHousekeep.call(testMgr_);

        return NULL;
    }


    static void * IndexerThreadFunc(void * p)
    {
        ASyncContext
            * thisPtr = reinterpret_cast<ASyncContext*>(p);
        return thisPtr->indexStuff();
    }


    void * searchStuff()
    {
        using namespace Cpt;

        sleep(43);

        // on emulator, this amount of search happens together with
        // indexing/deleting/updating, the rest runs alone, therefore
        // distoring statistics (we are interested in how slow
        // searching when (a) runs alone ("sq" test case) or when (b) runs mixed
        // with indexing ("mt" test case).
        enum 
        { 
            ROUNDS       = 21,
            SAMPLE_START = 1
        };

        Cpt::StopperWatch
            stopperWatch;

        SyncedSearch
            syncedSearch(cpixMutex_);

        try
            {
                for (size_t i = 0; i < ROUNDS; ++i)
                    {
                        if (i == SAMPLE_START)
                            {
                                searchStats_.reset();
                                termsStats_.reset();
                            }

                        stopperWatch.reset();
                            syncedSearch.call(searcher1_,
                                              searchQuery_,
                                              testMgr_,
                                              i%2 == 1);

                        searchStats_.update(stopperWatch);

                        stopperWatch.reset();
                            syncedSearch.call(searcher2_,
                                              termsQuery_,
                                              testMgr_,
                                              i%2 == 1);

                        termsStats_.update(stopperWatch);
                        
                        ITK_DBGMSG(testMgr_, ":");
                    }
            }
        catch (...)
            {
                
                ITK_EXPECT(testMgr_,
                           false,
                           "SEARCHER: Failed searching\n");
                throw;
            }

        return NULL;
    }


    static void * SearcherThreadFunc(void * p)
    {
        ASyncContext
            * thisPtr = reinterpret_cast<ASyncContext*>(p);
        return thisPtr->searchStuff();
    }


};



Itk::TesterBase * CreateASyncTests()
{
    using namespace Itk;

    ASyncContext
        * asyncContext = new ASyncContext();
    ContextTester
        * contextTester = new ContextTester("async",
                                           asyncContext);

#define TEST "si"
    contextTester->add(TEST,
                       asyncContext,
                       &ASyncContext::testSingleThreadIdx,
                       TEST);
#undef TEST

#define TEST "sq"
    contextTester->add(TEST,
                       asyncContext,
                       &ASyncContext::testSingleThreadQry,
                       TEST,
                       "CANCELLED"); // cancellation may happen in
                                     // different ways
#undef TEST

#define TEST "mt"
    contextTester->add(TEST,
                       asyncContext,
                       &ASyncContext::testMultiThreads,
                       TEST,
                       SuiteTester::REDIRECT_ONLY);
#undef TEST

    return contextTester;
}
