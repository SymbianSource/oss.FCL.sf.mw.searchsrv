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

#ifndef CPIX_CPIX_ASYNC_H
#define CPIX_CPIX_ASYNC_H

#include "cpixidxdb.h"
#include "cpixsearch.h"

/****************************************************************
 *
 *    O V E R V I E W
 *
 * CPix API for some asynchronous API operations (not all sync
 * operations have async conterparts). All async operations are
 * semantically equivalent to their synchronous brethren, they just
 * happen to be asynchronous. For sample function
 *
 * "int cpix_Foo_bar(cpix_Foo * thisFoo,
 *                   int        arg);"
 *
 * The following async function will be created:
 *
 * "cpix_JobId cpix_Foo_asyncBar(cpix_Foo * thisFoo,
 *                               int        arg,
 *                               void     * cookie,
 *                               void    (* callback)(void *, cpix_JobId));"
 *
 * Argument thisFoo has the same meaning in both the sync and the
 * async version: the operation "bar(int)" is performed on that
 * object. If there is an error during creating the async job itself,
 * the error is reported on thisFoo.
 *
 * Hypothetical argument "arg" has the exact same meaning: some argument
 * for "bar(int)".
 *
 * Argument "cookie" is some client specific data that the client
 * wants to see as an argument given to the callback in the async
 * version - to be able to identify the request.
 *
 * Argument "callback" is a callback function that can conceptually
 * give kick the caller with the given cookie and the job id that was
 * allocated for the job. The arguments given to the callback will be
 * the cookie and the job id that has been completed. The callback
 * should be doing no serious processing of the results, only some
 * administrative work to signal some party to collect the results.
 *
 * Normally, once the asyncBar has kicked the callback, the client can
 * come and collect the results of the async operation 'bar', like
 *
 * int cpix_Foo_asyncBarResults(cpix_Foo   * thisFoo,
 *                              cpix_JobId   jobId);
 *
 * Where the return value is given there as well as error status is
 * communicated through thisFoo. For all (non-cancellable) async
 * functions calls, there must be a corresponding result-collection
 * call. Such result collection call must be done only after the
 * callback has been called by the worker. DO NOT collect the results
 * as part of the callback.
 *
 * NOTE, that the job-creation itself may fail, that is, the call to
 * cpix_Foo_asuyncBar may report error on thisFoo. In that case the
 * job was not created, there is no need to cancel or collect the
 * results.
 *
 * Some async operations are cancellable.
 *
 * Calling cancel on an object (cpix_IdxDb*, cpix_IdxSearcher* or
 * cpix_Hits*) will guarantee that the callback is not called for the
 * client object once the cancel function returns. For all cancellable
 * async function calls, there must be either a cancel or a
 * result-collection call. Cancel operations can be called any time,
 * even before the callback has been called by the worker.
 *
 * Instances of cpix_IdxDb and cpix_IdxSearcher should not be deleted
 * before either cancel() or cpix_XXX_asyncXXXResults() is
 * called. Argument instances like cpix_Document, cpix_Analyzer,
 * cpix_Query can be immediately released by the client call.
 *
 * Any argument that is an object (like cpix_Document*, const
 * wchar_t*, ...) can be immediately discarded by the client after a
 * call to cpix_XXX_cancelMMM(): those instances are either cloned or
 * have reference counted lifetime management.
 *
 * NOTE:
 *
 * 1 Don't ever use the same cpix_IdxDb or cpix_IdxSearch instance
 *   from multiple threads. These objects themselves are not
 *   thread-safe.
 *
 * 2 You should be able to use different cpix_IdxDb instances that
 *   actually manage the same underlyng physical index volume - that
 *   should work.
 *
 * 3 Make absolutely certain that you clear the error status on
 *   cpix_Foo on all async operations (like cpix_Foo_asyncBar,
 *   cpix_foo_asyncBarResults, cpix_Foo_cancel()). Failure to do so
 *   may result in leaking error-status objects. (Obviously, you
 *   should check the error status on cpix_Foo anyway after all calls
 *   anyway.) To clear the error status, use macro cpix_ClearError
 *   from cpixerror.h.
 *
 * 4 If you use an async version of an operation, then you MUST use
 *   the async version of each operation where available.
 *
 *   That means that once you use (any of) this ASYNC api, you must
 *   never call any of these functions:
 *
 *     cpix_IdxDb_add,
 *     cpix_IdxDb_deleteDocuments,
 *     cpix_IdxDb_update,
 *     cpix_IdxDb_flush,
 *     cpix_IdxDb_doHousekeeping,
 *     cpix_IdxDb_search,
 *     cpix_IdxSearcher_search,
 *     cpix_Hits_doc.
 *
 *
 * RULES concerning cancel(), cpix_XXX_asyncXXXResults()
 * (results(), for short) and callback():
 *
 * 1 Once cancel() returns, callback() will not occur.
 *
 * 2 There must be exactly one of these calls: cancel() and
 *   results(). There can be no duplicate cancels() or calling
 *   results() after cancel() has been called.
 *
 * 3 results() give back results, possibly transferring ownership of
 *   resulting instances to the client, but cancel() makes sure that
 *   all resources related to the job are released.
 *
 * 4 Therefore once a call to cancel() has been made, you must not
 *   attempt to try to collect the results by calling result().
 *
 * 5 callback() may occur any time before cancel() returns. It can
 *   happen that the client is already in the cancel() call, and
 *   during that time callback() happens. In that case, the client
 *   must clear the called back status, and make sure that the
 *   results() are not called by mistake.
 */


#ifdef __cplusplus
extern "C" {
#endif


    /**
     * Every asynchronous job will have an ID allocated, one must
     * collect the result of the job / cancel it referring to that ID.
     */
    typedef size_t cpix_JobId;


    /**
     * Callback type to signal the client that the operation has
     * completed. Normally, after the callback, the client would
     * proceed to collect the error status and possible return values.
     */
    typedef void cpix_AsyncCallback(void       * cookie,
                                    cpix_JobId   jobId);




    /**
     * Each cancel operation returns how the cancel request was
     * handled. Useful for development purposes (logging and observing
     * clien behaviour) but has no real benefit for end user.
     */
    enum cpix_CancelAction_
        {
            /**
             * If the cancel request while the request was still
             * pending in the job queue - the job was killed even
             * before it could have started.
             */
            cpix_CANCEL_SHALLOW,

            /**
             * The cancel request cam exactly while the job was being
             * executed - the job was interrupted and killed that way.
             */
            cpix_CANCEL_DEEP,

            /**
             * The cancel request came after the job was already done.
             */
            cpix_CANCEL_JOBDONE,

            /**
             * Unknown error during cancel.
             */
            cpix_CANCEL_ERROR
        };
    typedef cpix_CancelAction_ cpix_CancelAction;


    /***************************************************************
     *
     *    A S Y N C   F U N C T I O N S
     *
     */


    /**
     * Corresponding sync function: cpix_IdxDb_add (cpixidxdb.h), see
     * overview and description for params 'thisIdxDb', 'document' and
     * 'analyzer' there. Error status is signaled on thisIdxDb right
     * away if the very creation of the asynchronous job has
     * failed. In that case, you can't collect the results of the
     * operation.
     *
     * For a general description of parameters 'cookie' and
     * 'callback', see the OVERVIEW comment above.
     *
     * @return the job id of the created job, if successful
     */
    cpix_JobId cpix_IdxDb_asyncAdd(cpix_IdxDb         * thisIdxDb,
                                   cpix_Document      * document,
                                   cpix_Analyzer      * analyzer,
                                   void               * cookie,
                                   cpix_AsyncCallback * callback);


    /**
     * Collects the results on the completed async add operation on
     * thisIdxDb. Error status of the job execution is signalled on
     * thisIdxDb.
     */
    void cpix_IdxDb_asyncAddResults(cpix_IdxDb * thisIdxDb,
                                    cpix_JobId   jobId);


    /**
     * Corresponding to sync function: cpix_IdxDb_deleteDocuments
     * (cpixidxdb.h), see overview and description for params 'thisIdxDb'
     * and 'docUid' there. Error status is signaled on thisIdxDb right
     * away if the very creation of the asynchronous job has
     * failed. In that case, you can't collect the results of the
     * operation.
     *
     * For a general description of parameters 'cookie' and
     * 'callback', see the OVERVIEW comment above.
     *
     * @return the job id of the created job, if successful
     */
    cpix_JobId cpix_IdxDb_asyncDeleteDocuments(cpix_IdxDb         * thisIdxDb,
                                               const wchar_t      * docUid,
                                               void               * cookie,
                                               cpix_AsyncCallback * callback);
    
    
    /**
     * Collects the results on the completed async delete documents
     * operation on thisIdxDb. Error status of the job execution is
     * signalled on thisIdxDb.
     */
    int32_t cpix_IdxDb_asyncDeleteDocumentsResults(cpix_IdxDb * thisIdxDb,
                                                   cpix_JobId   jobId);



    /**
     * Corresponding sync function: cpix_IdxDb_update (cpixidxdb.h), see
     * overview and description for params 'thisIdxDb', 'document' and
     * 'analyzer' there. Error status is signaled on thisIdxDb right
     * away if the very creation of the asynchronous job has
     * failed. In that case, you can't collect the results of the
     * operation.
     *
     * For a general description of parameters 'cookie' and
     * 'callback', see the OVERVIEW comment above.
     *
     * @return the job id of the created job, if successful
     */
    cpix_JobId cpix_IdxDb_asyncUpdate(cpix_IdxDb         * thisIdxDb,
                                      cpix_Document      * document,
                                      cpix_Analyzer      * analyzer,
                                      void               * cookie,
                                      cpix_AsyncCallback * callback);


    /**
     * Collects the results of the completed async update operation on
     * thisIdxDb. Error status of the job execution is signalled on
     * thisIdxDb.
     */
    void cpix_IdxDb_asyncUpdateResults(cpix_IdxDb * thisIdxDb,
                                       cpix_JobId   jobId);



    /**
     * Corresponding sync function: cpix_IdxDb_flush (cpixidxdb.h),
     * see overview and description for params 'thisIdxDb'. Error
     * status is signaled on thisIdxDb right away if the very creation
     * of the asynchronous job has failed. In that case, you can't
     * collect the results of the operation.
     *
     * For a general description of parameters 'cookie' and
     * 'callback', see the OVERVIEW comment above.
     *
     * @return the job id of the created job, if successful
     */
    cpix_JobId cpix_IdxDb_asyncFlush(cpix_IdxDb         * thisIdxDb,
                                     void               * cookie,
                                     cpix_AsyncCallback * callback);
    

    /**
     * Collects the results of the completed flush operation on the
     * given thisIdxDb. Error status of the job execution is signalled
     * on thisIdxDb.
     */
    void cpix_IdxDb_asyncFlushResults(cpix_IdxDb * thisIdxDb,
                                      cpix_JobId   jobId);



    /**
     * Corresponding sync function: cpix_doHousekeeping (cpixinit.h),
     * see overview. This function is a "static"/"library" function,
     * that is, it does not have a "this" parameter, only cpix_Result
     * that can tell if the creation of async job has failed. On
     * failure of creating the job, you can't collect the results of
     * the operation.
     *
     * For a general description of parameters 'cookie' and
     * 'callback', see the OVERVIEW comment above.
     *
     * @return the job id of the created job, if successful
     */
    cpix_JobId cpix_asyncDoHousekeeping(cpix_Result        * result,
                                        void               * cookie,
                                        cpix_AsyncCallback * callback);


    /**
     * Collects the results of the completed doHousekeeping
     * operation. Error status of job execution is signalled on
     * result.
     */
    void cpix_asyncDoHousekeepingResults(cpix_Result       * result,
                                         cpix_JobId          jobId);


    /**
     * TODO: review this comment "Error status is NOT signaled on thisIdxDb"
     * Corresponding sync function: cpix_IdxDb_search (cpixidxdb.h),
     * see overview and description for params 'thisIdxDb' and
     * 'query'. Error status is NOT signaled on thisIdxDb. Error
     * status is signaled on thisIdxDb right away if the very creation
     * of the asynchronous job has failed. In that case, you can't
     * cancel or collect the results of the operation.
     *
     * For a general description of parameters 'cookie' and
     * 'callback', see the OVERVIEW comment above.
     *
     * @return the job id of the created job, if successful
     */
    cpix_JobId cpix_IdxDb_asyncSearch(cpix_IdxDb         * thisIdxDb,
                                      cpix_Query         * query,
                                      void               * cookie,
                                      cpix_AsyncCallback * callback);


    /**
     * Collects the results on the completed async search operation on
     * thisIdxDb. Error status of the job execution is signalled on
     * thisIdxDb.
     */
    cpix_Hits * cpix_IdxDb_asyncSearchResults(cpix_IdxDb * thisIdxDb,
                                              cpix_JobId   jobId);


    /**
     * Cancels the outstanding async operation on the given index
     * database. Once this method has returned, no callbacks will be
     * called for that object (there may be callbacks before this
     * returns).
     *
     * @returns the way the cancel was handled
     */
    cpix_CancelAction cpix_IdxDb_cancelSearch(cpix_IdxDb * thisIdxDb,
                                              cpix_JobId   jobId);


    /**
     * Corresponding sync function: cpix_IdxSearcher_search
     * (cpixsearch.h), see overview and description for params
     * 'thisIdxSearcher' and 'query'. Error status is signaled on
     * thisIdxDb right away if the very creation of the asynchronous
     * job has failed. In that case, you can't cancel or collect the
     * results of the operation.
     *
     * For a general description of parameters 'cookie' and
     * 'callback', see the OVERVIEW comment above.
     *
     * @return the job id of the created job, if successful
     */
    cpix_JobId cpix_IdxSearcher_asyncSearch(cpix_IdxSearcher   * thisIdxSearcher,
                                            cpix_Query         * query,
                                            void               * cookie,
                                            cpix_AsyncCallback * callback);



    /**
     * Collects the results on the completed async search operation on
     * thisIdxSearcher. Error status of the job execution is signalled
     * on thisIdxDb.
     *
     * Ownership of the hits is transferred to the client.
     */
    cpix_Hits * 
    cpix_IdxSearcher_asyncSearchResults(cpix_IdxSearcher * thisIdxsearcher,
                                        cpix_JobId         jobId);

    /**
     * Cancels the outstanding async search operation on the given
     * searcher. Once this method has returned, no callbacks will be
     * called for that object (there may be callbacks before this
     * returns).
     *
     * @returns the way the cancel was handled
     */
    cpix_CancelAction 
    cpix_IdxSearcher_cancelSearch(cpix_IdxSearcher * thisIdxSearcher,
                                  cpix_JobId         jobId);


    /**
     * Corresponding to the function: cpix_Hits_doc
     * (cpixsearch.h). See overview and description for params
     * 'thisHits', 'index' and 'target'. Error status is signaled on
     * thisHits right away if the very creation of the asynchronous
     * job has failed. In that case, you can't cancel or collect the
     * results of the operation.
     *
     * For generation description of parameters 'cookie' and
     * 'callback' see the OVERVIEW comment above.
     *
     * Document retrieval may trigger re-committing the original
     * search, if the underlying indexes were modified, hence the need
     * for async version of this operation.
     *
     * NOTE: The semantics of param 'target' are almost exactly the
     * same as in the sync version: the wrapper is owned by the
     * client, the wrapped is owned by engine, hence this type of
     * signature.
     *
     * NOTE: However, there is one 
     *                          C R U C I A L
     *                                     difference between how
     * target document can be allocated. In the sync version it
     * can/should be a local variable, on the stack. But in the async
     * version, you MUST absolutely make certain that cpix_Document
     * instance target points to is not on the stack, but part of some
     * C++ class instance / C struct instance layout. This is because
     * while cpix_Hits_asycnDoc is cancellable, it only means that the
     * client has the right to not collect the results, but the
     * operation may (will) be completed, and therefore the place
     * target points to will be initialized as a cpix_Document. But,
     * by the time that happens, the client will have moved on, after
     * cancelling the operation, to do other stuff, which means that
     * callstack of the client will be MESSED UP. Hence, keep the
     * target document off the stack, somewhere in the layout of a
     * C/C++ object instance.
     *
     * @return the job id of the created job, if successful
     */
    cpix_JobId cpix_Hits_asyncDoc(cpix_Hits          * thisHits,
                                  int32_t              index,
                                  cpix_Document      ** target,
                                  void               * cookie,
                                  cpix_AsyncCallback * callback,
                                  int32_t              count);


    /**
     * Collects the results of the completed async doc-retrieval
     * operation on thisHits. Error status of the job execution is
     * signalled on thisHits.
     */
    void cpix_Hits_asyncDocResults(cpix_Hits * thisHits,
                                   cpix_JobId  jobId);

    
    /**
     * Cancels the outstanding async search operation on the given
     * hits list. Once this method as has returned, no callbacks will
     * be called for that object (there may be callbacks before this
     * returns).
     *
     *
     * @returns the way the cancel was handled
     */
    cpix_CancelAction cpix_Hits_cancelDoc(cpix_Hits * thisHits,
                                          cpix_JobId  jobId);






    



#ifdef __cplusplus
} /* extern "C" */
#endif /* __cplusplus */

#endif CPIX_CPIX_ASYNC_H
