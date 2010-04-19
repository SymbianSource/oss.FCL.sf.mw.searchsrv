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

#include <stdio.h>
#include <stdlib.h>
#include <wchar.h>
#include <glib.h>

#include <exception>
#include <functional>

#include "cpixsynctools.h"
#include "cpixjobqueue.h"

// Own header
#include "cpix_async.h"

// cLucene API
#include "CLucene.h"
#include "CLucene/queryParser/MultiFieldQueryParser.h"


// CPix internal
#include "iidxdb.h"
#include "idxdb.h"
#include "cpixsearch.h"
#include "cpixhits.h"
#include "initparams.h"
#include "cpixutil.h"
#include "iqrytype.h"
#include "analyzer.h"
#include "document.h"
#include "cpixthreadlocaldata.h"


#include "common/cpixlog.h"

// TODO CLucene deep cancel feature will require this header
// #include "CLucene/store/SearchCancel.h"

namespace
{
    // job in progress
    //cpix_JobId currentJobInProgress;

    enum AsyncOpKind
        {
            IDX,
            QRY
        };


    /**
     * Represents an asynchronous CPix job, polymorphic base class.
     */
    class AsyncJob
    {
    public:
        //
        // public typedefs
        //
        typedef Cpt::JobQueue<AsyncJob*> Queue;


    private:
        //
        // private members
        //
        void                 * cookie_;
        cpix_AsyncCallback   * callback_;
        
        cpix_Error           * err_;

        /**
         * Instantiated during async engine initialization, so there
         * is point in protecting the pointer itself, and the queue
         * instance itself is thread-safe.
         */
        static Queue         * idxQueue_;
        static Queue         * qryQueue_;

        /**
         * The job id for the next AsyncJob instance created.
         *
         * NOTE: currently only one thread accesses the AsyncJob
         * constructor (the master thread), so there is no need to
         * protect this variable with mutex.
         */
        static cpix_JobId      nextJobId_;

    protected:
        //
        // protected members
        //
        IdxDbHndl              handle_;

        volatile bool          cancelled_;
        volatile bool          calledBack_;
        // accesses to: cancelled_, calledBack_ and callback_ must be
        // synchronized via this mutex
        Cpt::Mutex             mutex_;

        cpix_JobId                  id_;
        

    public:
        //
        // public operations
        //

        static Queue * CreateIdxQueue(size_t queueSize)
        {
            if (idxQueue_ != NULL)
                {
                    THROW_CPIXEXC(PL_ERROR "IDX job queue already exists");
                }

            idxQueue_ = new Queue(queueSize);

            return idxQueue_;
        }


        static Queue * GetIdxQueue()
        {
            if (idxQueue_ == NULL)
                {
                    THROW_CPIXEXC(PL_ERROR "IDX job queue uninitialized");
                }

            return idxQueue_;
        }

        
        static Queue * CreateQryQueue(size_t queueSize)
        {
            if (qryQueue_ != NULL)
                {
                    THROW_CPIXEXC(PL_ERROR "QRY job queue already exists");
                }

            qryQueue_ = new Queue(queueSize);

            return qryQueue_;
        }


        static Queue * GetQryQueue()
        {
            if (qryQueue_ == NULL)
                {
                    THROW_CPIXEXC(PL_ERROR "QRY job queue uninitialized");
                }

            return qryQueue_;
        }

        

        static Queue * GetQueue(AsyncOpKind asyncOpKind)
        {
            Queue
                * rv = NULL;

            switch (asyncOpKind)
                {
                case IDX:
                    rv = GetIdxQueue();
                    break;
                case QRY:
                    rv = GetQryQueue();
                    break;
                default:
                    THROW_CPIXEXC(PL_ERROR "Unknown AsyncOpKind %d",
                                  asyncOpKind);
                }

            return rv;
        }


        static void Shutdown()
        {
            delete idxQueue_;
            idxQueue_ = NULL; 

            delete qryQueue_;
            qryQueue_ = NULL; 
        }


        /**
         * Cancels this job if necessary - decision is based on
         * cancelArg. This method is called from the master (producer)
         * thread.
         *
         * @return true if the caller should destroy this job
         * instance. If the callback has already happened, then the
         * slave thread will not deal with this instance anymore, so
         * the destruction of this instance must happen now, in the
         * canceller client context.
         */
        bool cancel(cpix_JobId jobId)
        {
            bool
                rv = false;

            if (id_ != jobId)
                {
                    THROW_CPIXEXC("Cancel messup");
                }

            { // SYNC
                Cpt::SyncRegion
                    sr(mutex_);
                    
                if (calledBack_)
                    {
                        rv = true;
                    }
                else
                    {
                        cancelled_ = true;
                    }
            } // SYNC

            return rv;
        }


        static cpix_CancelAction Cancel(cpix_JobId   jobId,
                                        AsyncOpKind  asyncOpKind)
        {
            if (asyncOpKind != QRY)
                {
                    THROW_CPIXEXC("Only qry ops can be cancelled (%d)",
                                  jobId);
                }

            cpix_CancelAction
                rv = cpix_CANCEL_ERROR;

            std::auto_ptr<AsyncJob>
                job(GetAsyncJobFromQueue(jobId,
                                         asyncOpKind));

            if (AsyncJob::GetQueue(asyncOpKind)->cancelNonStartedJob( 
                  compose1(bind2nd(equal_to<cpix_JobId>(),
                                       jobId),
                           mem_fun(&AsyncJob::id))))
                {
                    logMsg(CPIX_LL_TRACE,
                           "Cancelled job %d removed before started.",
                           jobId);

                    return cpix_CANCEL_SHALLOW;
                }
#if 0 // TODO CLucene deep cancel
            else if (jobId == currentJobInProgress)
                {
                    setSearchCancel(true);
                    rv = cpix_CANCEL_DEEP;
                }
#endif // 0
            bool
                destroy = job->cancel(jobId);

            if (destroy)
                {
                    // job is taken out of jobqueue
                    AsyncJob::GetQueue(asyncOpKind)->jobCompleted(job.get());
                    rv = cpix_CANCEL_JOBDONE;
                }
            else
                {
                    // job is not to be destroyed yet
                    job.release();
                }

            // job is taken care of by auto_ptr if needed

            return rv;
        }


        /**
         * Performs the job represented by this instance. Called from
         * slave (consumer) thread.
         *
         * @returns true if this job should be completed and destroyed
         * by the slave thread right after this function returns.
         */
        bool doJob()
        {
            try
                {
                    err_ = doJob_();
                }
            catch (...)
                {
                    if (err_ == NULL)
                        {
                            err_ = CreateError(ET_CPIX_EXC,
                                               L"doJob_ has thrown unknown exception - see logs for exact derived class / job type");
                        }

                    logMsg(CPIX_LL_ERROR,
                           "%s::doJob_ failed with exception",
                           typeid(*this).name());
                }

            bool
                rv = false;

            // On completion of job, we must call the callback unless
            // the job was cancelled on the master thread.
            { // SYNC
                Cpt::SyncRegion
                    sr(mutex_);

                if (!cancelled_)
                    {
                        try
                            {
                                (*callback_)(cookie_,
                                             id_);
                                calledBack_ = true;
                            }
                        catch (...)
                            {
                                logMsg(CPIX_LL_ERROR,
                                       "callback(%p,%d) has thrown exception",
                                       cookie_,
                                       id_);
                            }
                    }
                else
                    {
                        rv = true;
                    }

            } // SYNC

            return rv;
        }


        /**
         * @returns the error result of the completed (non-cancelled)
         * job. May be NULL, if the operation was successful. At the
         * same time, it releases ownership of the error object, if
         * any.
         */
        cpix_Error * releaseErr()
        {
            cpix_Error
                * rv = err_;
            
            err_ = NULL;

            return rv;
        }

        
        /**
         * @returns the id of this job
         */
        cpix_JobId id() const
        {
            return id_;
        }



        //
        // lifetime management
        //
        /**
         * Constructor.
         */
        AsyncJob(IdxDbHndl            handle,
                 void               * cookie,
                 cpix_AsyncCallback * callback)
            : cookie_(cookie),
              callback_(callback),
              err_(NULL),
              handle_(handle),
              cancelled_(false),
              calledBack_(false),
              id_(nextJobId_++)
        {
            ;
        }


        /**
         * Destructor (virtual).
         */
        virtual ~AsyncJob()
        {
            // if it is not NULL, it is still owned by this job,
            // therefore released at this point
            cpix_ClearError(this);
        }


        /**
         * Acquires the lock of this job and immediately releases
         * it. Sounds silly, but it is necessary - this method is only
         * called from GetStartedAsyncJobFromQueue() method, see
         * comments there.
         */
        void lockAndUnlock()
        {
            Cpt::SyncRegion
                sr(mutex_);
        }


    protected:
        //
        // protected members
        //
        /**
         * Performs the job represented by this instance. Called from
         * the slave (consumer) thread. Implementation may take
         * advantage of the fact that cancel_ can be set to true at
         * any point (called from the master thread).
         */
        virtual cpix_Error * doJob_() = 0;


        /**
         * Tries to put the created job to the queue, reporting
         * errors. Templated: cpix_IdxDb or cpix_IdxSearcher.
         */
        template<typename WRAPPER>
        static void PutAsyncJobToQueue(std::auto_ptr<AsyncJob> & job,
                                       WRAPPER                 * wrapper,
                                       AsyncOpKind               asyncOpKind)
        {
            bool
                result = GetQueue(asyncOpKind)->put(job.get());
            if (result)
                {
                    // on success, ownership is transferred to queue
                    job.release();
                }
            else
                {
                    // failure: for clarity, explicitly destroyed
                    job.reset();

                    wrapper->err_ = CreateError(ET_CPIX_EXC,
                                                "Could not put async job to queue");
                }
        }


        static AsyncJob * GetStartedAsyncJobFromQueue(cpix_JobId  jobId,
                                                      AsyncOpKind asyncOpKind)
        {
            using namespace std;

            AsyncJob
                * rv = NULL;

            bool
                result = GetQueue(asyncOpKind)
                ->findStartedJob(compose1(bind2nd(equal_to<cpix_JobId>(),
                                                  jobId),
                                          mem_fun(&AsyncJob::id)),
                                 &rv);
            if (!result)
                {
                    THROW_CPIXEXC(PL_ERROR "AsyncJob not found");
                }

            // If the client thread (master thread) is "fast" enough,
            // it could start collecting the results of a job even
            // before the slave thread has released its lock. In that
            // case we would get an EBUSY which we want to treat as an
            // error (to catch possibly logic errors wrt
            // lifetime/threading) therefore all we need to do here is
            // to sync the master thread with the slave thread:
            // collecting results (for which this method is needed)
            // will have to wait until the slave thead has released
            // the lock on this job.
            rv->lockAndUnlock();

            return rv;
        }


        static AsyncJob * GetAsyncJobFromQueue(cpix_JobId  jobId,
                                               AsyncOpKind asyncOpKind)
        {
            using namespace std;

            AsyncJob
                * rv = NULL;

            bool
                result = GetQueue(asyncOpKind)
                ->findJob(compose1(bind2nd(equal_to<cpix_JobId>(),
                                           jobId),
                                   mem_fun(&AsyncJob::id)),
                          &rv);
            if (!result)
                {
                    THROW_CPIXEXC(PL_ERROR "AsyncJob not found");
                }

            return rv;
        }

    };



    // static members of AsyncJob
    AsyncJob::Queue * AsyncJob::idxQueue_    = NULL;
    AsyncJob::Queue * AsyncJob::qryQueue_    = NULL;
    cpix_JobId        AsyncJob::nextJobId_   = 0;




    /**
     *   Async Job class for cpix_IdxDb_add()/cpix_IdxDb_update()
     *   operations.
     */
    class AsyncAddUpdateJob : public AsyncJob
    {
    private:
        //
        // private members
        //
        cpix_Document      * document_;
        cpix_Analyzer      * analyzer_;
        bool                 update_;


    public:
        //
        // public operators
        //
        static cpix_JobId CreateJob(cpix_IdxDb         * idxDb,
                                    cpix_Document      * document,
                                    cpix_Analyzer      * analyzer,
                                    void               * cookie,
                                    cpix_AsyncCallback * callback,
                                    bool                 update)
        {
            std::auto_ptr<AsyncJob>
                job(new AsyncAddUpdateJob(idxDb,
                                          document,
                                          analyzer,
                                          cookie,
                                          callback,
                                          update));
            cpix_JobId
                rv = job->id();

            PutAsyncJobToQueue(job,
                               idxDb,
                               IDX);

            return rv;
        }


        static void Results(cpix_IdxDb * idxDb,
                            cpix_JobId   jobId)
        {
            std::auto_ptr<AsyncJob>
                job(GetStartedAsyncJobFromQueue(jobId,
                                                IDX));

            idxDb->err_ = job->releaseErr();

            // job is taken out of jobqueue
            AsyncJob::GetQueue(IDX)->jobCompleted(job.get());

            // job is destroyed by auto_ptr
        }
                                 
        
        //
        // lifetime management
        //
        AsyncAddUpdateJob(cpix_IdxDb         * idxDb,
                          cpix_Document      * document,
                          cpix_Analyzer      * analyzer,
                          void               * cookie,
                          cpix_AsyncCallback * callback,
                          bool                 update)
            : AsyncJob(idxDb->handle_,
                       cookie,
                       callback),
              document_(NULL),
              analyzer_(NULL),
              update_(update)
        {
            // we must make sure that these native objects live even
            // if the client calls cancel in the middle and discards
            // those argument right after cancel.

            Cpix::IdxDbMgr::instance()->incRefHndl(handle_);

            document_ = SharedCopy(document,
                                   idxDb); // error put here

            if (document_ != NULL)
                {
                    analyzer_ = SharedCopy(analyzer,
                                           idxDb); // error put here
                }

            if (document_ == NULL || analyzer_ == NULL)
                {
                    THROW_CPIXEXC("Failed to create shared copies (doc, an)");
                }
        }


        virtual ~AsyncAddUpdateJob()
        {
            // in the constructor we created (shared) copies, now we
            // must discard them as usual

            Cpix::IdxDbMgr::instance()->releaseHndl(handle_);
            cpix_Document_destroy(document_);
            cpix_Analyzer_destroy(analyzer_);
        }
        
    protected:
        //
        // protected members
        //
        virtual cpix_Error * doJob_()
        {
            cpix_IdxDb
                idxDb;
            idxDb.handle_ = handle_;

            if (update_)
                {
                    cpix_IdxDb_update(&idxDb,
                                      document_,
                                      analyzer_);
                }
            else
                {
                    cpix_IdxDb_add(&idxDb,
                                   document_,
                                   analyzer_);
                }

            return idxDb.err_;
        }

    };




    /**
     *   Asyn Job class for cpix_IdxDb_deleteDocuments() operation.
     */
    class AsyncDelDocsJob : public AsyncJob
    {
    private:
        //
        // private members
        //
        std::wstring     docUid_;
        int32_t          delCount_;


    public:
        //
        // public operators
        //
        static cpix_JobId CreateJob(cpix_IdxDb         * idxDb,
                                    const wchar_t      * docUid,
                                    void               * cookie,
                                    cpix_AsyncCallback * callback)
        {
            std::auto_ptr<AsyncJob>
                job(new AsyncDelDocsJob(idxDb,
                                        docUid,
                                        cookie,
                                        callback));
            
            cpix_JobId
                rv = job->id();

            PutAsyncJobToQueue(job,
                               idxDb,
                               IDX);

            return rv;
        }


        static int32_t Results(cpix_IdxDb * idxDb,
                               cpix_JobId   jobId)
        {
            std::auto_ptr<AsyncJob>
                job(GetStartedAsyncJobFromQueue(jobId,
                                                IDX));
            
            idxDb->err_ = job->releaseErr();

            AsyncDelDocsJob
                * tmp = dynamic_cast<AsyncDelDocsJob*>(job.get());

            if (tmp == NULL)
                {
                    THROW_CPIXEXC(PL_ERROR "Should be AsyncDelDocsJob");
                }
            
            int32_t
                rv = tmp->delCount_;

            // job is taken out of jobqueue
            AsyncJob::GetQueue(IDX)->jobCompleted(job.get());

            return rv;

            // job is destroyed by auto_ptr
        }
                                 
        
        //
        // lifetime management
        //
        AsyncDelDocsJob(cpix_IdxDb         * idxDb,
                        const wchar_t      * docUid,
                        void               * cookie,
                        cpix_AsyncCallback * callback)
            : AsyncJob(idxDb->handle_,
                       cookie,
                       callback),
              docUid_(docUid),
              delCount_(0)
        {
            Cpix::IdxDbMgr::instance()->incRefHndl(handle_);
        }


        virtual ~AsyncDelDocsJob()
        {
            Cpix::IdxDbMgr::instance()->releaseHndl(handle_);
        }
        
    protected:
        //
        // protected members
        //
        virtual cpix_Error * doJob_()
        {
            cpix_IdxDb
                idxDb;
            idxDb.handle_ = handle_;

            delCount_ = cpix_IdxDb_deleteDocuments(&idxDb,
                                                   docUid_.c_str());
            
            return idxDb.err_;
        }

    };




    /**
     *   Asyn Job class for cpix_IdxDb_flush() operation.
     */
    class AsyncFlushJob : public AsyncJob
    {
    public:
        //
        // public operators
        //
        static cpix_JobId CreateJob(cpix_IdxDb         * idxDb,
                                    void               * cookie,
                                    cpix_AsyncCallback * callback)
        {
            std::auto_ptr<AsyncJob>
                job(new AsyncFlushJob(idxDb,
                                      cookie,
                                      callback));
            
            cpix_JobId
                rv = job->id();

            PutAsyncJobToQueue(job,
                               idxDb,
                               IDX);

            return rv;
        }


        static void Results(cpix_IdxDb * idxDb,
                            cpix_JobId   jobId)
        {
            std::auto_ptr<AsyncJob>
                job(GetStartedAsyncJobFromQueue(jobId,
                                                IDX));
            
            idxDb->err_ = job->releaseErr();

            // job is taken out of jobqueue
            AsyncJob::GetQueue(IDX)->jobCompleted(job.get());

            // job is destroyed by auto_ptr
        }
                                 
        
        //
        // lifetime management
        //
        AsyncFlushJob(cpix_IdxDb         * idxDb,
                      void               * cookie,
                      cpix_AsyncCallback * callback)
            : AsyncJob(idxDb->handle_,
                       cookie,
                       callback)
        {
            Cpix::IdxDbMgr::instance()->incRefHndl(handle_);
        }


        virtual ~AsyncFlushJob()
        {
            Cpix::IdxDbMgr::instance()->releaseHndl(handle_);
        }
        
    protected:
        //
        // protected members
        //
        virtual cpix_Error * doJob_()
        {
            cpix_IdxDb
                idxDb;
            idxDb.handle_ = handle_;

            cpix_IdxDb_flush(&idxDb);
            
            return idxDb.err_;
        }

    };



    /**
     * Async Job class for cpix_doHousekeeping() operation.
     */
    class AsyncDoHousekeepingJob : public AsyncJob
    {
    public:
        //
        // public operators
        //
        static cpix_JobId CreateJob(cpix_Result        * result,
                                    void               * cookie,
                                    cpix_AsyncCallback * callback)
        {
            std::auto_ptr<AsyncJob>
                job(new AsyncDoHousekeepingJob(cookie,
                                               callback));

            cpix_JobId
                rv = job->id();

            PutAsyncJobToQueue(job,
                               result,
                               IDX);

            return rv;
        }

        
        static void Results(cpix_Result * result,
                            cpix_JobId    jobId)
        {
            std::auto_ptr<AsyncJob>
                job(GetStartedAsyncJobFromQueue(jobId,
                                                IDX));
            
            result->err_ = job->releaseErr();
            
            // job is taken out of queue
            AsyncJob::GetQueue(IDX)->jobCompleted(job.get());
            
            //job is destroyed by auto_ptr
        }


        //
        // lifetime management
        //
        AsyncDoHousekeepingJob(void               * cookie,
                               cpix_AsyncCallback * callback)
            : AsyncJob(0,
                       cookie,
                       callback)
        {
            ;
        }


        virtual ~AsyncDoHousekeepingJob()
        {
            ;
        }


    protected:
        //
        // protected members
        //
        virtual cpix_Error * doJob_()
        {
            cpix_doHousekeeping();

            return NULL;
        }
    };



    /**
     *   Asyn Job class for cpix_IdxDb_search() operation.
     */
    template<typename       IDX,
             cpix_Hits * (* SEARCH)(IDX*, cpix_Query*)>
    class AsyncSearchJob_ : public AsyncJob
    {
    private:
        //
        // private members
        //
        cpix_Query     * query_;

        cpix_Hits      * hits_;


    public:
        //
        // public operators
        //
        static cpix_JobId CreateJob(IDX                * idx,
                                    cpix_Query         * query,
                                    void               * cookie,
                                    cpix_AsyncCallback * callback)
        {
            std::auto_ptr<AsyncJob>
                job(new AsyncSearchJob_(idx,
                                        query,
                                        cookie,
                                        callback));

            cpix_JobId
                rv = job->id();

            PutAsyncJobToQueue(job,
                               idx,
                               QRY);

            return rv;
        }


        static cpix_Hits * Results(IDX        * idx,
                                   cpix_JobId   jobId)
        {
            std::auto_ptr<AsyncJob>
                job(GetStartedAsyncJobFromQueue(jobId,
                                                QRY));
            
            idx->err_ = job->releaseErr();

            AsyncSearchJob_
                * tmp = dynamic_cast<AsyncSearchJob_*>(job.get());

            if (tmp == NULL)
                {
                    THROW_CPIXEXC(PL_ERROR "Should be AsyncSearchJob_");
                }
            
            cpix_Hits
                * rv = tmp->hits_;

            tmp->hits_ = NULL;

            // job is taken out of jobqueue
            AsyncJob::GetQueue(QRY)->jobCompleted(job.get());

            return rv;

            // job is destroyed by auto_ptr
        }
                                 
        
        //
        // lifetime management
        //
        AsyncSearchJob_(IDX                * idx,
                        cpix_Query         * query,
                        void               * cookie,
                        cpix_AsyncCallback * callback)
            : AsyncJob(idx->handle_,
                       cookie,
                       callback),
              query_(NULL),
              hits_(NULL)
        {
            // we must make sure that this object lives even if the
            // client calls cancel in the middle and discards this
            // argument right after cancel
            Cpix::IdxDbMgr::instance()->incRefHndl(handle_);

            query_ = SharedCopy(query,
                                idx); // error put here

            if (query_ == NULL)
                {
                    THROW_CPIXEXC("Failed to create shared copy (query)");
                }
        }


        virtual ~AsyncSearchJob_()
        {
            Cpix::IdxDbMgr::instance()->releaseHndl(handle_);

            // in the constructor we created (shared) copy, now we
            // must discard it as usual

            cpix_Query_destroy(query_);

            // destroying the resulting hits_ object - if the result
            // was collected, then hits_ is set to NULL (so nothing
            // will happen here and the client took ownership of the
            // hits_)
            cpix_Hits_destroy(hits_);
        }
        
    protected:
        //
        // protected members
        //
        virtual cpix_Error * doJob_()
        {
            IDX
                idx;
            idx.handle_ = handle_;

            hits_ = (*SEARCH)(&idx,
                              query_);
            
            return idx.err_;
        }

    };



    typedef AsyncSearchJob_<cpix_IdxDb, &cpix_IdxDb_search> 
    AsyncIdxDbSearchJob;

    typedef AsyncSearchJob_<cpix_IdxSearcher, &cpix_IdxSearcher_search> 
    AsyncIdxSearcherSearchJob;




    /**
     *   Async Job class for cpix_Hits_doc() operation.
     */
    class AsyncHitsDocJob : public AsyncJob
    {
    private:
        //
        // private members
        //
        cpix_Hits      * hits_;
        int32_t          index_;
        cpix_Document  * target_;


    public:
        //
        // public operators
        //
        static cpix_JobId CreateJob(cpix_Hits          * hits,
                                    int32_t              index,
                                    cpix_Document      * target,
                                    void               * cookie,
                                    cpix_AsyncCallback * callback)
        {
            std::auto_ptr<AsyncJob>
                job(new AsyncHitsDocJob(hits,
                                        index,
                                        target,
                                        cookie,
                                        callback));

            cpix_JobId
                rv = job->id();

            PutAsyncJobToQueue(job,
                               hits,
                               QRY);

            return rv;
        }


        static void Results(cpix_Hits  * hits,
                            cpix_JobId   jobId)
        {
            std::auto_ptr<AsyncJob>
                job(GetStartedAsyncJobFromQueue(jobId,
                                                QRY));
            
            hits->err_ = job->releaseErr();

            AsyncHitsDocJob
                * tmp = dynamic_cast<AsyncHitsDocJob*>(job.get());

            if (tmp == NULL)
                {
                    THROW_CPIXEXC(PL_ERROR "Should be AsyncHitsDocJob");
                }
            
            // job is taken out of jobqueue
            AsyncJob::GetQueue(QRY)->jobCompleted(job.get());

            // job is destroyed by auto_ptr
        }
                                 
        
        //
        // lifetime management
        //
        AsyncHitsDocJob(cpix_Hits          * hits,
                        int32_t              index,
                        cpix_Document      * target,
                        void               * cookie,
                        cpix_AsyncCallback * callback)
            : AsyncJob(IdxDbHndl(), // default value for handlers
                       cookie,
                       callback),
              hits_(NULL),
              index_(index),
              target_(target)
        {
            // We must make sure that these native objects live even
            // if the client calls cancel in the middle of operation
            // and discards the argument right after cancel.

            // NOTE: reference to the originating idxdb handle is
            // incremented by the hits instance construction (if necessary)

            hits_ = SharedCopy(hits,
                               hits); // error put here

            if (hits_ == NULL)
                {
                    THROW_CPIXEXC("Failed to create shared copy of hits");
                }
        }


        virtual ~AsyncHitsDocJob()
        {
            // in the constructor we created (shared) copies, now we
            // must discard them as usual

            // NOTE: reference to the originating idxdb handle is
            // decremented by the hits instance destruction (if necessary)
            cpix_Hits_destroy(hits_);
        }
        
    protected:
        //
        // protected members
        //
        virtual cpix_Error * doJob_()
        {
            cpix_Hits
                hits;
            hits.ptr_ = hits_->ptr_;
            hits.err_ = NULL;

            cpix_Hits_doc(&hits,
                          index_,
                          target_);

            return hits.err_;
        }

    };



    



    void * QrySlaveThreadFunc(void * param)
    {
        logMsg(CPIX_LL_TRACE,
               "Slave thread (Qry) started");
        
        // set ThreadLocalData so that error handling code can always
        // know what to do
        Cpix::ThreadLocalData
            * tld = new Cpix::ThreadLocalData(true);

        // TODO CLucene deep cancel, commented out code
        // InitSearchCancel();

        int
            result = pthread_setspecific(Cpix::ThreadLocalDataKey,
                                         tld);
        
        if (result != 0)
            {
                logMsg(CPIX_LL_ERROR,
                       "PANIC Could not set thread specific key (%d)",
                       result);
                logMsg(CPIX_LL_ERROR,
                       "PANIC slave thread can't work, async API is useless");
                exit(1);
            }

        AsyncJob::Queue
            * queue = reinterpret_cast<AsyncJob::Queue*>(param);

        try
            {
                bool
                    goOn = true;

                while (goOn)
                    {
                        AsyncJob
                            * job = NULL;

                        goOn = queue->get(&job);

                        if (goOn)
                            {
                                try
                                    {
                                        // TODO CLucene deep cancel, commented out code
                                        // reset searchCancel before executing
                                        // search job
                                        // setSearchCancel(false);

                                        //currentJobInProgress = job->id();

                                        bool
                                            destroy = job->doJob();
                                        if (destroy)
                                            {
                                                queue->jobCompleted(job);
                                                
                                                delete job;
                                            }
                                    }
                                catch (std::exception & exc)
                                    {
                                        logMsg(CPIX_LL_ERROR,
                                               "Doing async job failed - %s",
                                               exc.what());
                                    }
                                catch (...)
                                    {
                                        logMsg(CPIX_LL_ERROR,
                                               "Doing async job failed - unknown reason");
                                    }
                            }
                    }
            }
        catch (...)
            {
                logMsg(CPIX_LL_ERROR,
                       "Slave thread (Qry) failed");
            }

        logMsg(CPIX_LL_TRACE,
               "Slave thread (Qry) completed");

        return NULL;
    }




    void * IdxSlaveThreadFunc(void * param)
    {
        logMsg(CPIX_LL_TRACE,
               "Slave thread (Idx) started");
        
        // set ThreadLocalData so that error handling code can always
        // know what to do
        Cpix::ThreadLocalData
            * tld = new Cpix::ThreadLocalData(true);

        int
            result = pthread_setspecific(Cpix::ThreadLocalDataKey,
                                         tld);
        
        if (result != 0)
            {
                logMsg(CPIX_LL_ERROR,
                       "PANIC Could not set thread specific key (%d)",
                       result);
                logMsg(CPIX_LL_ERROR,
                       "PANIC slave thread can't work, async API is useless");
                exit(1);
            }

        AsyncJob::Queue
            * queue = reinterpret_cast<AsyncJob::Queue*>(param);

        try
            {
                bool
                    goOn = true;

                while (goOn)
                    {
                        AsyncJob
                            * job = NULL;

                        goOn = queue->get(&job);

                        if (goOn)
                            {

                                try
                                    {
                                        bool
                                            destroy = job->doJob();
                                        if (destroy)
                                            {
                                                queue->jobCompleted(job);
                                                
                                                delete job;
                                            }
                                    }
                                catch (std::exception & exc)
                                    {
                                        logMsg(CPIX_LL_ERROR,
                                               "Doing async job failed - %s",
                                               exc.what());
                                    }
                                catch (...)
                                    {
                                        logMsg(CPIX_LL_ERROR,
                                               "Doing async job failed - unknown reason");
                                    }

                                result = sched_yield();

                                if (result != 0)
                                    {
                                        logMsg(CPIX_LL_ERROR,
                                               "sched_yield failed: %d",
                                               errno);
                                    }
                            }
                    }
            }
        catch (...)
            {
                logMsg(CPIX_LL_ERROR,
                       "Slave thread (Idx) failed");
            }

        logMsg(CPIX_LL_TRACE,
               "Slave thread (Idx) completed");

        return NULL;
    }




    class PthreadAttrDestroyer
    {
    private:
        pthread_attr_t    * attr_;


        // No value semantics supported: just declared private to
        // prevent their use - never defined
        PthreadAttrDestroyer(const PthreadAttrDestroyer &);
        PthreadAttrDestroyer & operator=(const PthreadAttrDestroyer&);

    public:
        PthreadAttrDestroyer(pthread_attr_t * attr)
            : attr_(attr)
        {
            ;
        }


        ~PthreadAttrDestroyer()
        {
            int
                result = pthread_attr_destroy(attr_);

            if (result != 0)
                {
                    logMsg(CPIX_LL_ERROR,
                           "Could not destroy pthread_attr_t instance.");
                }
        }
    };

}



namespace {

    pthread_t IdxSlaveThreadHandle;
    pthread_t QrySlaveThreadHandle;
}


namespace Cpix
{

    void InitAsyncEngine(Cpix::InitParams & ip)
    {
        int
            result = 0;

        pthread_attr_t
            attr;

        result = pthread_attr_init(&attr);

        if (result != 0)
            {
                THROW_CPIXEXC("Could not initialize pthread attribute (%d)",
                              result);
            }

        PthreadAttrDestroyer
            attrDestroyer(&attr);

        enum { DESIRED_STACKSIZE = 0x10000 }; // 64k as default 8k is too little

        result = pthread_attr_setstacksize(&attr,
                                           DESIRED_STACKSIZE);

        if (result != 0)
            {
                THROW_CPIXEXC("Could not set thread stacksize to %d bytes (%d)",
                              DESIRED_STACKSIZE,
                              result);
            }
    
         int
             schedPolicy = SCHED_RR;
         
         int
             minPriority = sched_get_priority_min(schedPolicy),
             maxPriority = sched_get_priority_max(schedPolicy),
             defPriority = 0;
    
         result
             = pthread_attr_setschedpolicy(&attr,
                                           schedPolicy);
         
         if (result != 0)
             {
                 THROW_CPIXEXC("Could not set thread sched policy to  %d (%d)",
                               schedPolicy,
                               result);
         }

         struct sched_param
             param;
         result = pthread_attr_getschedparam(&attr,
                                             &param);

         if (result != 0)
             {
                 THROW_CPIXEXC("Could not get sched param (%d)",
                               result);
             }

         defPriority = param.sched_priority;

         param.sched_priority = defPriority + ip.getIdxThreadPriorityDelta();

         result = pthread_attr_setschedparam(&attr,
                                             &param);

         if (result != 0)
             {
                 THROW_CPIXEXC("Could not set thread sched priority to  %d (%d)",
                               param.sched_priority,
                               result);
         }


        result = pthread_key_create(&ThreadLocalDataKey,
                                    &ThreadLocalData::Destroy);

        if (result != 0)
            {
                THROW_CPIXEXC("Could not create pthread_key_t (%d)",
                              result);
            }

        ThreadLocalData
            * tld = new ThreadLocalData(false); // master thread
        
        result = pthread_setspecific(ThreadLocalDataKey,
                                     tld);
        
        if (result != 0)
            {
                logMsg(CPIX_LL_ERROR,
                       "PANIC Could not set thread specific key (%d)",
                       result);
                logMsg(CPIX_LL_ERROR,
                       "PANIC master thread can't work, async API is useless");
                exit(1); // TODO protect it, only in debug builds
            }
        
        AsyncJob::Queue
            * queue = AsyncJob::CreateIdxQueue(ip.getIdxJobQueueSize());
    
        result = pthread_create(&IdxSlaveThreadHandle,
                                &attr,
                                IdxSlaveThreadFunc,
                                queue);

        if (result != 0)
            {
                THROW_CPIXEXC("Could not start slave thread IDX");
            }

        queue = AsyncJob::CreateQryQueue(ip.getQryJobQueueSize());
    
        param.sched_priority = defPriority + ip.getQryThreadPriorityDelta();
        
        result = pthread_attr_setschedparam(&attr,
                                            &param);
        
        if (result != 0)
            {
                THROW_CPIXEXC("Could not set thread sched priority to  %d (%d)",
                              param.sched_priority,
                              result);
            }

        result = pthread_create(&QrySlaveThreadHandle,
                                &attr,
                                QrySlaveThreadFunc,
                                queue);

        if (result != 0)
            {
                AsyncJob::GetQueue(IDX)->stopProcessing();
                THROW_CPIXEXC("Could not start slave thread QRY");
            }
    }



    /**
     * To be invoked on the master thread.
     */
    void ShutdownAsyncEngine()
    {
        // Request slave thread to stop processing and exit
        AsyncJob::GetQueue(IDX)->stopProcessing();
        AsyncJob::GetQueue(QRY)->stopProcessing();
    
        int result = 
            pthread_join(IdxSlaveThreadHandle, 
                         NULL); 
    
        if (result != 0) 
            {
                logMsg(CPIX_LL_ERROR,
                       "Joining slave thread IDX failed.");
            }
    
        result = 
            pthread_join(QrySlaveThreadHandle, 
                         NULL); 
    
        if (result != 0) 
            {
                logMsg(CPIX_LL_ERROR,
                       "Joining slave thread QRY failed.");
            }
    
        // The slave thread should be down now. Go forth to release 
        // the asynchronization related resources. 
    
        ThreadLocalData::Destroy(pthread_getspecific(ThreadLocalDataKey));
    
        // does not destroy the value
        pthread_key_delete(ThreadLocalDataKey);
    
        AsyncJob::Shutdown();
    }

}


cpix_JobId cpix_IdxDb_asyncAdd(cpix_IdxDb         * thisIdxDb,
                               cpix_Document      * document,
                               cpix_Analyzer      * analyzer,
                               void               * cookie,
                               cpix_AsyncCallback * callback)
{
    return XlateExc(thisIdxDb,
                    CallFreeFunc(&AsyncAddUpdateJob::CreateJob,
                                 thisIdxDb,
                                 document,
                                 analyzer,
                                 cookie,
                                 callback,
                                 false)); // add
}


void cpix_IdxDb_asyncAddResults(cpix_IdxDb * thisIdxDb,
                                cpix_JobId   jobId)
{
    XlateExc(thisIdxDb,
             CallFreeFunc(&AsyncAddUpdateJob::Results,
                          thisIdxDb,
                          jobId));
}



cpix_JobId cpix_IdxDb_asyncDeleteDocuments(cpix_IdxDb         * thisIdxDb,
                                           const wchar_t      * docUid,
                                           void               * cookie,
                                           cpix_AsyncCallback * callback)
{
    return XlateExc(thisIdxDb,
                    CallFreeFunc(&AsyncDelDocsJob::CreateJob,
                                 thisIdxDb,
                                 docUid,
                                 cookie,
                                 callback));
}


int32_t cpix_IdxDb_asyncDeleteDocumentsResults(cpix_IdxDb * thisIdxDb,
                                               cpix_JobId   jobId)
{
    return XlateExc(thisIdxDb,
                    CallFreeFunc(&AsyncDelDocsJob::Results,
                                 thisIdxDb,
                                 jobId));
}


cpix_JobId cpix_IdxDb_asyncUpdate(cpix_IdxDb         * thisIdxDb,
                                  cpix_Document      * document,
                                  cpix_Analyzer      * analyzer,
                                  void               * cookie,
                                  cpix_AsyncCallback * callback)
{
    return XlateExc(thisIdxDb,
                    CallFreeFunc(&AsyncAddUpdateJob::CreateJob,
                                 thisIdxDb,
                                 document,
                                 analyzer,
                                 cookie,
                                 callback,
                                 true)); // update
}


void cpix_IdxDb_asyncUpdateResults(cpix_IdxDb * thisIdxDb,
                                   cpix_JobId   jobId)
{
    XlateExc(thisIdxDb,
             CallFreeFunc(&AsyncAddUpdateJob::Results,
                          thisIdxDb,
                          jobId));
}


cpix_JobId cpix_IdxDb_asyncFlush(cpix_IdxDb         * thisIdxDb,
                                 void               * cookie,
                                 cpix_AsyncCallback * callback)
{
    return XlateExc(thisIdxDb,
                    CallFreeFunc(&AsyncFlushJob::CreateJob,
                                 thisIdxDb,
                                 cookie,
                                 callback));
}


void cpix_IdxDb_asyncFlushResults(cpix_IdxDb * thisIdxDb,
                                  cpix_JobId   jobId)
{
    XlateExc(thisIdxDb,
             CallFreeFunc(&AsyncFlushJob::Results,
                          thisIdxDb,
                          jobId));
}


cpix_JobId cpix_asyncDoHousekeeping(cpix_Result        * result,
                                    void               * cookie,
                                    cpix_AsyncCallback * callback)
{
    return XlateExc(result,
                    CallFreeFunc(&AsyncDoHousekeepingJob::CreateJob,
                                 result,
                                 cookie,
                                 callback));
}


void cpix_asyncDoHousekeepingResults(cpix_Result       * result,
                                     cpix_JobId          jobId)
{
    XlateExc(result,
             CallFreeFunc(&AsyncDoHousekeepingJob::Results,
                          result,
                          jobId));
}



cpix_JobId cpix_IdxDb_asyncSearch(cpix_IdxDb         * thisIdxDb,
                                  cpix_Query         * query,
                                  void               * cookie,
                                  cpix_AsyncCallback * callback)
{
    return XlateExc(thisIdxDb,
                    CallFreeFunc(&AsyncIdxDbSearchJob::CreateJob,
                                 thisIdxDb,
                                 query,
                                 cookie,
                                 callback));
}



cpix_Hits * cpix_IdxDb_asyncSearchResults(cpix_IdxDb * thisIdxDb,
                                          cpix_JobId   jobId)
{
    return XlateExc(thisIdxDb,
                    CallFreeFunc(&AsyncIdxDbSearchJob::Results,
                                 thisIdxDb,
                                 jobId));
}


cpix_CancelAction cpix_IdxDb_cancelSearch(cpix_IdxDb * thisIdxDb,
                                          cpix_JobId        jobId)
{
    return XlateExc(thisIdxDb,
                    CallFreeFunc(&AsyncIdxDbSearchJob::Cancel,
                                 jobId,
                                 QRY));
}



cpix_JobId cpix_IdxSearcher_asyncSearch(cpix_IdxSearcher   * thisIdxSearcher,
                                        cpix_Query         * query,
                                        void               * cookie,
                                        cpix_AsyncCallback * callback)
{
    return XlateExc(thisIdxSearcher,
                    CallFreeFunc(&AsyncIdxSearcherSearchJob::CreateJob,
                                 thisIdxSearcher,
                                 query,
                                 cookie,
                                 callback));
}


cpix_Hits * 
cpix_IdxSearcher_asyncSearchResults(cpix_IdxSearcher * thisIdxSearcher,
                                    cpix_JobId         jobId)
{
    return XlateExc(thisIdxSearcher,
                    CallFreeFunc(&AsyncIdxSearcherSearchJob::Results,
                                 thisIdxSearcher,
                                 jobId));
}


cpix_CancelAction 
cpix_IdxSearcher_cancelSearch(cpix_IdxSearcher * thisIdxSearcher,
                              cpix_JobId         jobId)
{
    return XlateExc(thisIdxSearcher,
                    CallFreeFunc(&AsyncIdxSearcherSearchJob::Cancel,
                                 jobId,
                                 QRY));
}


cpix_JobId cpix_Hits_asyncDoc(cpix_Hits          * thisHits,
                              int32_t              index,
                              cpix_Document      * target,
                              void               * cookie,
                              cpix_AsyncCallback * callback)
{
    return XlateExc(thisHits,
                    CallFreeFunc(&AsyncHitsDocJob::CreateJob,
                                 thisHits,
                                 index,
                                 target,
                                 cookie,
                                 callback));
}


void cpix_Hits_asyncDocResults(cpix_Hits * thisHits,
                               cpix_JobId  jobId)
{
    XlateExc(thisHits,
             CallFreeFunc(&AsyncHitsDocJob::Results,
                          thisHits,
                          jobId));
}

    
cpix_CancelAction cpix_Hits_cancelDoc(cpix_Hits * thisHits,
                                      cpix_JobId  jobId)
{
    return XlateExc(thisHits,
                    CallFreeFunc(&AsyncHitsDocJob::Cancel,
                                 jobId,
                                 QRY));
}



