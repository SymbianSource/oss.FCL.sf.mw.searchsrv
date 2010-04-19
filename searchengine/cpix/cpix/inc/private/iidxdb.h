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


#ifndef IIDXDB_H_
#define IIDXDB_H_

#include <wchar.h>
#include <stdint.h>

#include <string>
#include <vector>

#include "cpixsearch.h"
#include "cpixidxdb.h"

#include "CLucene.h"

#include "cpixtools.h"

#include "cpixexc.h"
#include "cpixsearch.h"
#include "cpixidxdb.h"
#include "cpixdoc.h"
#include "ifieldfilter.h"
#include "cluceneext.h"
#include "rotlogger.h"
#include "initparams.h"
#include "idxdbmgr.h"

#include "fwdtypes.h"



namespace Cpix
{

    /**
     * Interface class 
     */
    class IIdxDb
    {
    public:

        /**
         * Plain search. In addition to returning the hits, it sets
         * the version to the version of this idxdb at the time of the
         * search - so that it will be possible to know later if
         * recommit is possible or not.
         *
         * @param query the clucene query to use (must not be NULL)
         * 
         * @param version the place to put the current version number
         * of this idxdb (must not be NULL)
         *
         * @return the resulting hits, which may be NULL (= empty hits
         * = no hits were found)
         */
        virtual lucene::search::Hits * search(lucene::search::Query * query,
                                              Version               * version) = 0;

        /**
         * Fetch document, possibly recommitting. If the version
         * number of this IIdxDb instance is the same as given as
         * second argument, it returns the currentHits as that hits
         * object is still valid. Must be thread-safe.
         *
         * If it is not the same version, it destroys currentHits,
         * recommits the search with the given query and returns the
         * results.
         *
         * @param currentHits the currently known hits that resulted
         * from this idx when it had version 'version', must not be NULL
         *
         * @param version the version number of this idx when it
         * produced currentHits, it gets updated if recommit did
         * happen, must not be NULL
         *
         * @param query the query that was used to produce
         * currentHits, and will be used for possibly recommit, must
         * not nbe NULL
         *
         * @param documentConsumer is the entity that wants to load a
         * number of documents in a batch, while the necessary sources
         * are locked and kept in the appropriate searchable state
         *
         * @return currentHits (if version number is okay) or a new
         * hits instance (in which case currentHits are deleted). May
         * be NULL (= no hits are found = empty hits).
         */
        virtual lucene::search::Hits * 
        fetchRecommitting(lucene::search::Hits        * currentHits,
                          Version                     * version,
                          lucene::search::Query       * query,
                          DocumentConsumer            & documentConsumer) = 0;
               

        /**
         * Gets terms for field / wildcard / appclassPrefix combination.
         *
         * @return the resulting IHits object (must not be NULL).
         */
        virtual IHits * getTerms(const wchar_t * field, 
                                 const wchar_t * wildcard, 
                                 const wchar_t * appclassPrefix, 
                                 int             maxHits) = 0;

        /**
         * Destructor
         */
        virtual ~IIdxDb();


        /**
         * Recreates the index (makes it empty). Thread-safe.
         */
        virtual void recreateIdx() = 0;


        /**
         * Initializes the whole index db mgmt part of CPix. Must be
         * called before anything else happens.
         *
         * @param initParams may be NULL, in which case default
         * settings are used.
         */
        static void init(InitParams * initParams);


        /**
         * Cleans up clucene directory used for storing file
         * locks. Rationale: on a system / cpix crash, ongoing clucene
         * operations may leave a file lock making all (or half) of
         * the accesses to that index fail unless such leaked file
         * locks are cleaned up.
         *
         * NOTE: This method currently uses the value defined by
         * clucene macro LUCENE_LOCK_DIR_ENV_FALLBACK (CLConfig.h) to
         * clean up the directory.
         *
         * TODO: If that clucene setting above will be turned into a
         * global variable that can be set by client (cpix) for
         * data-caging / security reasons, we need to change the
         * implementation of this method accordingly.
         */
        static void cleanupClLockDir();


        /**
         * Rotates to the next log file, if possible. The current one
         * will be closed, and its size will not increase. If
         * createLog was called with NULL or if it failed, this
         * operation will do nothing.
         */
        static void rotateLog() throw();


        /**
         *
         * Gets a handle to an index database (or possible a
         * multivolume access) based on the domain selector. The
         * corresponding index volumes must exist (created).
         *
         * @param domainSelector is a non-empty list of appclasses.
         *
         * @param allowMultiSearch whether allow searching multiple
         * physical indexes at the same time in case base app class
         * was unqualified.
         *
         * @return the instance ID for use. You must call release()
         * when not using the returned instance anymore. This instance
         * ID must not be persisted.
         *
         */
        static IdxDbHndl getIdxDbHndl(const char  * domainSelector,
                                      bool          allowMultiSearch);
        
        
        /**
         * Creates an IdxDb instance (if necessary), and returns its
         * handler. The corresponding index database path must be
         * defined already.
         *
         * @param qualBaseAppClass a qualified base app class for
         * which the index must be created or recreated now. The
         * associated index database path must be defined
         * (cf. defineVolume).
         *
         * @return the instance ID for use. You must call release()
         * when not using the returned instance any more. This
         * instance ID must not be persisted.
         */
        static IdxDbHndl 
        getIdxDbHndlCreating(const char  * qualBaseAppClass);


        /**
         * @param the index database handle
         *
         * @return the IdxDb instance corresponding to the ID.
         */
        static IIdxDb * getPtr(IdxDbHndl handle);


        /**
         * A client does not need this instance anymore (other clients
         * may still do).
         *
         * (Decreases the refcount and if it drops to zero, destroys the
         * instance.)
         */
        static void release(IdxDbHndl handle);

            

        /**
         * Defines an index database volume.
         *
         * @param qualBaseAppClass either a qualified base app class.
         *
         * @param path where to find the volume for the given app
         * class
         */
        static void defineVolume(const char * qualBaseAppClass,
                                 const char * path);


        /**
         * Undefines an index database volume (all existing instances
         * using it will be left orphaned).
         *
         * @param qualBaseAppClass either a qualified base app class.
         */
        static void undefineVolume(const char * qualBaseAppClass);


        /**
         * TODO
         */
        static void shutdownAll() throw();

        /**
         * If no IdxDb handlers are open, empties all information
         * about path and association handlers as well as deletes the
         * configuration file. ALL SETTINGS are lost - this function
         * is mostly meant for unit testing purposes.
         */
        static void scrapAll();


        
        /**
         * Perform regular housekeeping tasks.
         */
        static void doHousekeepingOnAll();


        /**
         * Dumping the state of the IdxDbMgr singleton instance and
         * everything it holds on: IdxDb and MultiIdxDb instances.
         */
        static void dbgDumpAllState();


        /**
         * TODO
         */
        virtual SchemaId addSchema(const cpix_FieldDesc * fieldDescs,
                                   size_t                 count) = 0;


        /**
         * TODO no need for schema in this case
         */
        virtual void add(Document * doc,
                         lucene::analysis::Analyzer * analyzer) = 0 ;

        /**
         * TODO
         *
         * @param analyzer - the analyzer MUST NOT BE NULL, as there is
         *  no default one used (currently)
         */
        virtual void add2(SchemaId                      schemaId,
                          const wchar_t               * docUid,
                          const char                  * appClass,
                          const wchar_t               * excerpt,
                          const wchar_t               * mimeType,
                          const wchar_t              ** fieldValues,
                          lucene::analysis::Analyzer  * analyzer) = 0 ;

        /**
         * TODO
         */
        virtual int32_t deleteDocuments(const wchar_t  * docUid) = 0 ;


        /**
         * TODO
         */
        virtual int32_t deleteDocuments2(lucene::index::Term * term) = 0 ;


        /**
         * TODO
         */
        virtual void update(Document * doc,
                            lucene::analysis::Analyzer * analyzer) = 0 ;

        /**
         * TODO FIX current implementation is abysmally pessimal
         *
         * @parami analyzer - the analyzer MUST NOT BE NULL, as there is
         *  no default one used (currently)
         */
        virtual void update2(SchemaId                     schemaId,
                             const wchar_t              * docUid,
                             const char                 * appClass,
                             const wchar_t              * excerpt,
                             const wchar_t              * mimeType,
                             const wchar_t             ** fieldValues,
                             lucene::analysis::Analyzer * analyzer) = 0 ;


        /**
         * TODO
         */
        virtual void setMaxInsertBufSize(size_t value) = 0;

        
        /**
         * TODO
         */
        virtual void flush() = 0;
        

        /**
         * Discards all internal state, committing everything to disk
         * that should be.
         */
        virtual void close() = 0;


        /**
         * Discards all internal state, nothing committed to disk.
         */
        virtual void brutalClose() throw () = 0;


        /**
         * If there is a delta (deletions and / or insert buffer), it
         * is committed to the disk. Essentially it performs a flush,
         * but kept in a separate method in case there may be some
         * extra logic involved. Supposed to be called regularly.
         */
        virtual void doHousekeeping() = 0;


        /**
         * In case of some very serious problem, we dump state of this
         * particular instance.
         *
         * NOTE: This is a debug - test tool meant for console /
         * regression tests in safe contexts. Do not use it in
         * production code with ASYNC API, as it can deadlock the
         * threads.
         */
        virtual void dbgDumpState() = 0;

    };


} // namespace Cpix

#endif /* IIDXDB_H_ */
