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

#ifndef CPIX_CPIXINIT_H
#define CPIX_CPIXINIT_H

#include "cpixerror.h"
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif



    /**
     * A POV instance that
     *
     * (a) initializes with the default Cpix runtime parameters
     *
     * (b) allows getting those values
     *
     * (c) allows setting those values with sanity checks performed.
     *
     * The only type of operations allowed before cpix_init() are
     * cpix_InitParams creation and property set / get calls.
     */
    struct cpix_InitParams_
    {
        // pointer to native (CLucene/Cpix) impl
        void       * ptr_;
        
        // Last error, if any, that resulted from executing the
        // last operation on this cpix_XXX object
        // Use macros cpix_Failed, cpix_Succeeded and cpix_ClearError.
        // Do not attempt releasing it.
        cpix_Error * err_;
    };
    typedef cpix_InitParams_ cpix_InitParams;

    

    /**
     * Creates a cpix_InitParams instance (that can be used to call
     * cpix_init()) that is initialized with the default runtime
     * parameters.
     *
     * @param result error in creating the instance will be
     * communicated via this
     *
     * @return new cpix_InitParams instance that can be used to
     * initialize Cpix core.
     *
     * NOTE: once cpix_init is called, setting any values on the
     * cpix_InitParams instance has no effect. The instance can
     * (should) be safely destroyed after cpix_init().
     *
     * NOTE: Obviously, one does not need to set all the parameters,
     * only the ones where one wishes to use values different to the
     * default ones.
     */
    cpix_InitParams * cpix_InitParams_create(cpix_Result * result);

    
    /**
     * Destroys this cpix_InitParams instance.
     */
    void cpix_InitParams_destroy(cpix_InitParams * thisIp);



    /**
     * Gets / sets property "cpixDir".
     *
     * CpixDir is the path to the directory where cpix should store:
     *
     *   (a) cpixreg.txt, the persisted qualified base app class ->
     *       volume (physical index database) path associations
     *
     *   (b) all the volumes (physical index databases) with automatic
     *       paths (client does not care where it is)
     *
     * Must not be NULL or empty string.
     *
     * Default value is cf DEFAULT_CPIX_DIR in cfg/indevicecfg.h.
     */
    const char * cpix_InitParams_getCpixDir(cpix_InitParams * thisIp);
    void cpix_InitParams_setCpixDir(cpix_InitParams * thisIp,
                                    const char      * value);

    /**
     * Gets / sets property "resourceDir".
     *
     * ResourceDir is the path to the directory where cpix should look for:
     *
     *   (a) Localization data
     *   
     * Must not be NULL or empty string.
     *
     * Default value is cf DEFAULT_CPIX_DIR in cfg/indevicecfg.h.
     */
    const char * cpix_InitParams_getResourceDir(cpix_InitParams * thisIp);
    void cpix_InitParams_setResourceDir(cpix_InitParams * thisIp,
										const char      * value);
    
    /**
     * Gets / sets property "logFileBase".
     *
     * LogFileBase the base part of the path to the log file to be
     * have the std output and error redirected to. Base: it does not
     * have extension. For instance, a value "c:\data\libcpix" will
     * produce log files like "c:\data\libcpix_N_log.txt" where N is
     * the rotational number (0-4, that is, 5 log files are used
     * ever).
     *
     * May be NULL, in which case no out/err redirection happens and
     * nothing is logged to files. It also means that not only logs
     * will not happen to log files but not even to STDOUT. If you
     * want to have logs to appear on STDOUT but not on logfiles (in
     * some console app, or unit test code) use cpix_dbgForceLogs()
     * below.
     *
     * Default is NULL.
     */
    const char * cpix_InitParams_getLogFileBase(cpix_InitParams * thisIp);
    void cpix_InitParams_setLogFileBase(cpix_InitParams * thisIp,
                                        const char      * value);

    
    
    /**
     * Gets / sets property "logSizeLimit".
     *
     * How big the log files are allowed to grow, if logging occurs at
     * all (cf logFileBase property). Actual log file size may be
     * bigger, as it is not constantly checked for its size (cf
     * property logSizeCheckRecurrency).
     *
     * Tradeoffs: bigger log files are likely to preserve more
     * information (they are being rotated), but consume more disk
     * space.
     */
    size_t cpix_InitParams_getLogSizeLimit(cpix_InitParams * thisIp);
    void cpix_InitParams_setLogSizeLimit(cpix_InitParams * thisIp,
                                        size_t            value);


    /**
     * Gets / sets property "logSizeCheckRecurrency".
     *
     * How many cpix operations are done between two checkups of log
     * file sizes (cf property logSizeLimit), if logging happens at
     * all (cf logFileBase property).
     * 
     * Tradeoffs: more frequent checks will make sure that the log
     * files don't grow significantly bigger than the limit set by
     * property "logSizeLimit", but it slows down Cpix core.
     */
    size_t cpix_InitParams_getLogSizeCheckRecurrency(cpix_InitParams * thisIp);
    void cpix_InitParams_setLogSizeCheckRecurrency(cpix_InitParams * thisIp,
                                                   size_t            value);



    /**
     * Gets / sets property "maxIdleSec".
     *
     * The time (in seconds) an IdxDb instance can be in an
     * uncommitted state unused (idle) until it gets flushed to the
     * disk and released (internally). Such flush & release does not
     * concern clients (they can still hold active handles to indexes
     * and searchers), except maybe the next operation will be slower
     * as it requires re-load.
     *
     * For this setting to have any effect, the cpix_doHousekeeping()
     * has to be called regularly.
     *
     * Tradeoffs: shorter idle time results in possibly more frequent
     * disk operations (increased CPU load and battery use), but make
     * information loss less likely.
     */
    size_t cpix_InitParams_getMaxIdleSec(cpix_InitParams * thisIp);
    void cpix_InitParams_setMaxIdleSec(cpix_InitParams * thisIp,
                                       size_t            value);



    /**
     * Gets / sets property "maxInsertBufSize".
     *
     * The size of insert buffer at which it gets committed to disk.
     *
     * Tradeoffs: smaller values result in more frequent commits
     * (increasing CPU and battery use) but are less likely to lose
     * information.
     *
     * NOTE: cf cpixidxdb.h, cpix_IdxDb_setMaxInsertBufSize() which
     * sets the very same value for one particular index (cpix_IdxDb
     * instance) only. This here manages the value generally used by
     * default.
     */
    size_t cpix_InitParams_getMaxInsertBufSize(cpix_InitParams * thisIp);
    void cpix_InitParams_setMaxInsertBufSize(cpix_InitParams * thisIp,
                                                size_t            value);


    /**
     * Gets / sets property "insertBufMaxDocs".
     *
     * The max number of documents in the insert buf writer before it
     * commits to its RAM index.
     *
     * The insert buf is technically an index stored in RAM and a
     * writer that writes to it. How frequently the writer should
     * commit to that RAM index is controlled by the CLucene
     * IndexWriter::maxDocs property.
     *
     * Tradeoffs: it's kind of difficult to see in what way this
     * parameter affects performance, but probably it does affect
     * it. Guess: smaller value results in more RAM operations but is
     * likely to reduce the peak-usage of RAM memory. To be confirmed
     * by tuning tests.
     */
    size_t cpix_InitParams_getInsertBufMaxDocs(cpix_InitParams * thisIp);
    void cpix_InitParams_setInsertBufMaxDocs(cpix_InitParams * thisIp,
                                             size_t            value);




    /**
     * Gets / sets property "idxJobQueueSize".
     *
     * Controls how many jobs can be
     * placed into the indexing job queue (async API, see
     * cpix_async.h), before it gets full. At that point the
     * producer thread (single master thread) is blocked until
     * there is a job taken out and started processing.
     *
     * This value should be just big enough to allow ongoing
     * harvesting with occasional odd indexing operations without
     * blocking the master thread.
     */
    size_t cpix_InitParams_getIdxJobQueueSize(cpix_InitParams * thisIp);
    void cpix_InitParams_setIdxJobQueueSize(cpix_InitParams * thisIp,
                                            size_t            value);


    /**
     * Gets / sets property "qryJobQueueSize".
     *
     * Controls how many jobs can be
     * placed into the query job queue (async API, see
     * cpix_async.h), before it gets full. At that point the
     * producer thread (single master thread) is blocked until
     * there is a job taken out and started processing.
     *
     * This value should be just big enough to allow 2-3 clients to
     * search simultaneously without blocking the master thread.
     */
    size_t cpix_InitParams_getQryJobQueueSize(cpix_InitParams * thisIp);
    void cpix_InitParams_setQryJobQueueSize(cpix_InitParams * thisIp,
                                            size_t            value);



    /**
     * Get / sets property "idxThreadPriorityDelta".
     *
     * Controls the thread priority of the indexer slave (worker)
     * thread. Positive value mean increasing thread priority,
     * negative value means decreasing it.
     *
     * NOTE: You have to know how much the default thread priority is
     * in your platform, as well as what is the min and max value for
     * POSIX thread priority. Cf sched_get_priority_min and
     * sched_get_priority_max functions. On Symbian OS / OpenC, the
     * min is 0, max is 255 and the default is 100.
     *
     * NOTE: if two competing threads, that sometimes lock on the same
     * resources, have different priority, it is not always clear how
     * much actual benefit such thread-prioritization will bring given
     * a certain usage pattern. One should try and measure the gains
     * of thread-priority settings.
     *
     * Default thread priority delta is 0, meaning that the thread
     * priority of a slave thread is left unchanged.
     */
    int cpix_InitParams_getIdxThreadPriorityDelta(cpix_InitParams * thisIp);
    void cpix_InitParams_setIdxThreadPriorityDelta(cpix_InitParams * thisIp,
                                                   int            value);


    /**
     * Get / sets property "qryThreadPriorityDelta".
     *
     * See comments above for property "idxThreadPriorityDelta". This
     * property does the very same for qry slave thread.
     */
    int cpix_InitParams_getQryThreadPriorityDelta(cpix_InitParams * thisIp);
    void cpix_InitParams_setQryThreadPriorityDelta(cpix_InitParams * thisIp,
                                                   int            value);


    /**
     * Gets / sets property "clHitsPageSize".
     *
     * Controls how many clucene hit documents are prefetched and
     * wrapped in clucene queries (see LuceneHits class in
     * cpixhits.h). In order to fetch subsequent documents in an
     * environment where indexes can be modified simultaneously
     * (overlapped AND concurrent access) re-committing a query can be
     * very costly. To reduce the number of roundtrips, hit documents
     * are pre-fetched in pages.
     *
     * Tradeoffs: having a greater page size increases RAM consumption
     * but reduces overall time required for full
     * search-and-enumerate-all-docs rounds.
     */
    size_t cpix_InitParams_getClHitsPageSize(cpix_InitParams * thisIp);
    void cpix_InitParams_setClHitsPageSize(cpix_InitParams * thisIp,
                                           size_t            value);


    /**
     * Gets / Sets property "CluceneLockDir".
     * 
     * Clucene lock dir is the path to the directory to have 
     * the clucene lock dir. 
     *
     * Default is c:\system\temp
     */
    const char * cpix_InitParams_getCluceneLockDir(cpix_InitParams * thisIp);
    void cpix_InitParams_setCluceneLockDir(cpix_InitParams * thisIp,
                                    const char      * value);

    /************************************************************************
     * General CPix init and shutdown functions.
     *
     * Initializes the CPix engine. No CPix calls should be made prior
     * to initializing it (except the ones that create cpix_InitParams
     * instance and set properties of it). With this call, it is
     * possible to redirect std out and std err to set of rotating
     * logfiles. If opening a logfile at initialization time did not
     * succeed, it is as if no logging was specified, and
     * initialization will still succeed. If the opening of the first
     * logfile succeeds, then std out and std err (console) will be
     * shut down, and their output will be redirected to the
     * logfile. There is no recovering the console after this point -
     * DON'T USE LOGs from console applications, only from daemons /
     * servers.
     *
     * @param result the success or error (with the details) will be
     *        communicated via this
     *
     * @param initParams the instance that configures CPix core. May
     *        be NULL, in which case default initialization parameters
     *        will be used. The initParams instance can (should) be
     *        destroyed after a call to cpix_init, any changes made to
     *        the instance after the cpix_init() call are not taken
     *        into account.
     *
     * NOTE: Rotating and/or actual std redirection of logs may fail,
     * in which case no logs will be produced.
     */
    void cpix_init(cpix_Result     * result,
                   cpix_InitParams * initParams);


    /**
     * Shuts down the CPix engine. No CPix calls can be made one
     * it has been shut down.
     */
    void cpix_shutdown();



    /**
     * @returns the version (and/or) revision of this CPix library.
     */
    const char * cpix_version();



    /**
     * Performs regular housekeeping activities, like flushing and
     * releasing resources associated to idle idx handles.
     */
    void cpix_doHousekeeping();



    /**
     * For logging levels.
     */
    enum cpix_LogLevel
        {
            CPIX_LL_ERROR   = 0,
            CPIX_LL_WARNING,
            CPIX_LL_TRACE,
            CPIX_LL_DEBUG
        };

    /**
     * Sets the loglevel. Default is CPIX_LL_TRACE.
     *
     * @param logLevel the new log level.
     *
     * @return the current log level
     */
    cpix_LogLevel cpix_setLogLevel(cpix_LogLevel logLevel);


    
    /**
     * If 'logFileBase' property given to cpix_InitParams at
     * cpix_init() time was the default NULL, then no logging occurs
     * at all. Now, some unit test code may want to re-enable logging
     * (that happens to STDOUT) even if there is no
     * log-redirection. By calling this function you can force CPix to
     * send logs to STDOUT (even if they are not redirected to log
     * files).
     *
     * Cf cpix_InitParams_getLogFileBase().
     */
    void cpix_dbgForceLogs();


    
    /**
     * Dumps the internal state to the logs, with CPIX_LL_TRACE
     * loglevel.
     *
     * NOTE: This function is NOT to be used in production code, with
     * ASYNC API. It can deadlock the process. So use it only in
     * console / regression tests in contexts where it cannot do harm.
     */
    void cpix_dbgDumpState();



    // See clucene/src/CLucene/debug/lucenebase.h and memtracking.cpp.
    // This function relies on the modifications enabled over there.
#define DBG_MEM_USAGE 1
#ifdef DBG_MEM_USAGE
    /**
     * For debug / development / performance measurement
     * purposes. Assumes CLucene being compiled with the DBG_MEM_USAGE
     * flag (modification by us), which, when enabled, uses the
     * already overloaded "operator new" operators to update the
     * largest amount of dynamic memory in use.
     *
     * @returns the largest amount of dynamic memory in use since this
     * process was started or since the last call to
     * cpix_dbgResetMemUsage - based on the overloaded "operator new"
     * stuff done in CLucene.
     */
    uint32_t cpix_dbgPeakDynMemUsage();

    /**
     * For debug / development / performance measurement
     * purposes. Resets the static global variable that stores the
     * greatest amount of heap used snice the beginning of the process
     * or the previous call to cpix_dbgResetMemUsage();
     */
    void cpix_dbgResetDynMemUsage();
#endif


#ifdef __cplusplus
} /* extern "C" */
#endif /* __cplusplus */



#endif /* CPIX_CPIXINIT_H */
