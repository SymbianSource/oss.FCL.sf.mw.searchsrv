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

#ifndef CPIX_IDXDBMGR_H
#define CPIX_IDXDBMGR_H


#include "fwdtypes.h"
#include "initparams.h"
#include "cpixsynctools.h"


namespace Cpix
{
    /**
     * An extra indirection introduced in order to be able to
     * implement transparent release of resources while they are
     * inactive.
     */
    class IIdxDbInfo
    {
    private:
        int              refCount_;
        IIdxDb         * ptr_;
        mutable long     lastAccessedSecs_;

    public:
        void incRefCount();

        /**
         * @return true if refcount has dropped to zero.
         */
        void decRefCount();

        /**
         * @return the reference count
         */
        int refCount() const;

        /**
         * Plain getting the ptr_ member.
         */
        IIdxDb * ptr() const;

        
        /**
         * Getting the ptr_ member for use: lastAccessedSecs_ is updated.
         */
        IIdxDb * getPtrForUse() const;
        

        /**
         * Forcefully sets the ptr_ to the given value. If the current
         * ptr_ is not NULL, it destroys it, giving no consideration
         * to flush information, as they will be discarded anyway.
         */
        void setPtr(IIdxDb * ptr);


        /**
         * Releases the currently held pointer, that is returns it
         * while setting the ptr_ member to NULL. Does not destroy
         * anything. Ownership of previous ptr_ value passes to
         * caller.
         */
        IIdxDb * release();
    

        /** OBS
         * destroys the ptr, possibly resetting the refCount_
         * /
        void destroy(bool resetRefCount);
        */
        
        void restIfIdle(long   nowSec,
                        size_t maxIdleSec);

        IIdxDbInfo();

        long lastAccessedSecs() const;

        void dbgDumpState(size_t idx) const;

    private:
        void accessed() const;
    };


    /**
     * This singleton class provides mapping between base application
     * types and their containing index database paths, as well as
     * mapping between index database paths and existing IdxDb
     * instances.
     *
     * The mapping between base application types and their index
     * database paths are persisted to a CPix specific configuration
     * file.
     */
    class IdxDbMgr
    {
    private:
        //
        // private members
        //

        typedef std::string QualBaseAppClass;
        typedef std::string IdxDbPath;

        typedef std::pair<std::string, IdxDbHndl> PathAndHndl;

        std::map<QualBaseAppClass, PathAndHndl>   qbacs_;
        std::vector<IIdxDbInfo>                   idxDbs_;

        // MultiIdxDb instances are stored here, see comments for
        // createMultiIdxDbHndl() for further details
        std::vector<IIdxDbInfo>                   multiIdxDbs_;

        std::string                               cpixDir_;
        std::string                               regFilePath_;

        size_t                                    maxIdleSec_;

        static IdxDbMgr *                         instance_;
        static const char                         delimiter_;

        Cpt::Mutex                                mutex_;

        Cpt::Mutex                                versionMutex_;

        Version                                   version_;

        InitParams                                initParams_;


        // each housekeeping increases this counter
        typedef size_t                            HousekeepCounter;
        HousekeepCounter                          housekeepCounter_;
        
        // IIdxDb instances scheduled for deferred deletions are
        // tracked with this structure
        struct IIdxDbToDestroy
        {
            IIdxDb                              * ptr_;
            HousekeepCounter                      housekeepCounter_;

            IIdxDbToDestroy(IIdxDb           * ptr,
                            HousekeepCounter   housekeepCounter);
        };

        std::list<IIdxDbToDestroy>                idxDbsToDestroy_;


    public:
        //
        // public operations
        //

        /**
         * Must be called once, before anything else doing with
         * indexes is ever attempted. IdxDbMgr::instance does not
         * function propertly unless this method has been called at
         * least once. Calling it multiple times does not hurt.
         */
        static void init(InitParams & initParams);



        static IdxDbMgr * instance();


        /**
         * Scraps all existing databases if they are not in
         * use. Thread-safe (locks the singleton instance).
         */
        static void scrapAll();
        

        static void shutdownAll();

        const InitParams& getInitParams() const;

        /**
         * (Re-)defines an index on a qualified base app class. The
         * qualified base app class - associated index database path
         * must be defined already (cf. defineVolume).
         *
         * Thread safe.
         */
        IdxDbHndl create(const char   * qualBaseAppClass);

        
        /**
         * Qualified base app class is a base app class specification
         * that is qualified with a volume id. General pattern for
         * qualified base app class string: "@VOLUMEID:BASEAPPCLASS".
         *
         * Thread neutral - has nothing to do with the singleton
         * instance.
         *
         * @param domainSelector a generic domain selection string
         * (basically, a list of app classes)
         *
         * @return true if the given string is a single qualified base
         * app class
         */
        static bool isQualBaseAppClass(const char * domainSelector);

        /**
         * Thread neutral - has nothing to do with the singleton
         * instance.
         *
         * @param appClass a single app class
         * 
         * @param baseAppClass - may be qualified or unqualified
         *
         * @return true if the request for the app class should
         * involve using the physical volume associated to
         * baseAppClass
         */
        static bool matchAppClass(const char * appClass,
                                  const char * baseAppClass);


        /**
         * Thread neutral - has nothing to do with the singleton
         * instance.
         *
         * @param domainSelector is a string that can specify somehow
         * what sort of indexes / volumes / documents we want to
         * access. It is a non-empty list of app classes
         * 
         * @param baseAppClass - may be qualified or unqualified
         *
         * @return true if the domain selector string mandates
         * involving the physical volume associated to baseAppClass to
         * be used
         */
        static bool match(const char * domainSelector,
                          const char * baseAppClass);



        /**
         * Gets a next version number. IdxDb and MultiIdxDb instances
         * are upgrading their version numbers as they go through
         * changes.
         */
        Version getNextVersion();


        /**
         * Gets a valid handle for a base app class. Thread-safe.
         *
         * @param baseAppClass may be qualified or unqualified
         * (qualification means volume id prefix).
         *
         * @param allowMultiSearch if true then searching multiple
         * indexes in vase of unqualified base app class is allowed
         * (coming through cpix_IdxSearcher API object).
         */
        IdxDbHndl getHndl(const char * domainSelector,
                          bool         allowMultiSearch);

        
        /**
         * Gets the pointer for the handle, the pointer must be
         * non-NULL. Thread-safe.
         */
        IIdxDb * get(IdxDbHndl handle);

        
        /**
         * Increments the refcount for the handle (does not create
         * anything). Thread-safe.
         */
        void incRefHndl(IdxDbHndl handle);


        /**
         * Releases the resources associated to handle (decrements
         * refcount and releases if 0). Thread-safe.
         */
        void releaseHndl(IdxDbHndl handle);

        
        /**
         * Defines a volume (physical index). Thread-safe.
         *
         * @param qualBaseAppClass new qualified base app class. This
         * qualified base app class must not be already associated
         * with a different path (redefinition to the same path is
         * allowed (= NOP)).
         *
         * @param indexDbPath the path to associated to the q base app
         * class
         */
        void defineVolume(const char * qualBaseAppClass,
                          const char * indexDbPath);

        /**
         * Undefines the volume associated to
         * defineVolume. Thread-safe.
         */
        void undefineVolume(const char * qualBaseAppClass);

        
        /**
         * Perform regular housekeeping tasks. Thread-safe.
         */
        void doHousekeepingOnAll();


        /**
         * In case of some very serious problem, we dump state.
         */
        void dbgDumpState();

        
        /**
         * @return the "clHitsPageSize" initial parameter CPix core
         * was initialized with (stored by the singleton).
         */
        size_t getClHitsPageSize() const;
        

        /**
         * Creates (or recreates) an empty clucene index in the given
         * clucene directory.
         *
         * NOTE: Differs from RecreateFsCpixIdx that the path argument
         * here is a clucene dir, not CPix dir.
         *
         * @param cluceneDir the directory to create (or recreate) the
         * empty clucene index in. It must NOT be NULL. The path must
         * exist.
         */
        static void IdxDbMgr::RecreateFsClIdx(const char * cluceneDir);


        /**
         * Tries to open the physical index residing at the given
         * path.
         *
         * @param cpixDir the directory containing the index database
         *
         * NOTE cpix index is not the same as clucene index
         * directory. cpix index directory may contain zero, one or
         * more clucene index directories and some other marker
         * files. See idxdbdelta.h
         *
         * @throws whatever (LuceneError, ...)
         */
        static void OpenFsIdx(const char * cpixDir);


        /**
         * Physically destroys the index database under the path, and
         * tries to re-create an empty one in its place.
         *
         * NOTE: differs from RecreateFsClIdx that the path argument
         * here is CPix dir, not clucene dir.
         *
         * @param cpixDir the path to the directory in which the cpix
         * index resides. This path is assumed to be there, but it is
         * not necessary for any files underneath to be there.
         *
         * Cpix index is not the same as clucene index directory. cpix
         * index directory may contain zero, one or more clucene index
         * directories and some other marker files. See idxdbdelta.h
         *
         * @throws some exception in case of failure
         */
        static void RecreateFsCpixIdx(const char * cpixDir);

    private:
        //
        // private methods
        //

        /**
         * Private constructor.
         *
         * @param ip InitParams instance if given by the client at
         * cpix_init() time (or a default one created by IIdxDb::init()).
         */
        IdxDbMgr(InitParams & ip);


        ~IdxDbMgr();
        
        
        /**
         * Performs couple of sanity checks wrt. to the base app class
         * and the intented idxDbPath, whether they collide with some
         * existing entries.
         *
         * Not thread-safe.
         *
         * @param succeeded pointer to the boolean telling the result
         * of the check. May be NULL, in which case this method throws
         * an exception in case of error (if not NULL, problem is
         * reported on std out and boolean value is set).
         */
        void sanityCheck(const std::string & baseAppClass,
                         const std::string & idxDbPath,
                         bool              * succeeded);


        /**
         * Thread-neutral, does not use singleton instance.
         *
         * @param the path to make sure that it exists
         *
         * @param recreate if true, then any existing index db will be
         * re-created. Otherwise, it will be (re-)created only if it
         * does not exist or corrupt (cannot be opened).
         */
        static void InitFileSystem(const char * path,
                                   bool         recreate);


        /**
         * Creates an actual IdxDb instance first initializeing the
         * file system (InitFileSystem()) and then adding the created
         * instance to its allocated slot (handle). If InitFileSystem()
         * fails, it will remove the erroneous / stale
         * qualBaseAppClass - path association from the registry and
         * persists it.
         *
         * Not thread-safe
         *
         * @param handle the handle to get the instance for
         *
         * @param path the designated path for the index database
         *
         * @param recreate whether to recreate the index in the case
         * it is there
         *
         * @param qualBaseAppClass the qualified base app class
         * identifying the volume (physical index) we are about to
         * create an IdxDb for
         */
        void createIdxDbInstance(IdxDbHndl    handle,
                                 const char * path,
                                 bool         recreate,
                                 const char * qualBaseAppClass);
        
        /**
         * Not thread-safe.
         *
         * Veneer function to the overloaded version (figures out
         * path, qbac all by itself).
         */
        void createIdxDbInstance(IdxDbHndl    handle,
                                 bool         recreate);


        /**
         * Loads the registry. Not thread-safe.
         */
        void loadReg();
        

        /**
         * Persists the registry. Not thread-safe.
         */
        void storeReg();


        /**
         * Generates the automatic index database path for a qualified
         * base app class. Not thread-safe.
         */
        std::string getAutomaticIdxDbPath(const char * qualBaseAppClass);


        /**
         * Thread-neutral, nothing to do with instance.
         * 
         * @param qualBaseAppClass a qualified base app class
         *
         * @return the pointer within the original C string where the
         * base app class part starts, right after the volume
         * identifier and delimiter.
         */
        static const char * getBaseAppClassPart(const char * qualBaseAppClass);

        
        friend class MultiIdxDb;

        /**
         * Increments the reference count of the IdxDb instance at a
         * given handle. Does not create / instantiate anything.
         *
         * Not thread-safe, the caller must lock on "this". This
         * private method is called from MultiIdxDb (friend), but that
         * function is called by this instance (from defineVolume())
         * which already locks this instance. Sorry for the coupling
         * (but one unnecessary recursive locks less this way).
         */
        void incIdxDbRefCount(IdxDbHndl    handle);


        /**
         * Creates a new MultiIdxDb instance and a handler for
         * it. MultiIdxDb instances are NOT reused accross different
         * clients, even if they ask for the same, they get different
         * instances. (The inferred IdxDb instance will be shared
         * though.)
         *
         * The reason we handle multi idx db instance through handles
         * is that cpix_IdxSearch API object this way only refers to
         * handle-s that may resolve to an IdxDb* or a MultiIdxDb*.
         *
         * Since instances are not reused, once a MultiIdxDb instance
         * is destroyed, it's handle can be reused. These handles are
         * stored in a different vector.
         *
         * The difference between handles that resolve to IdxDb* and
         * the ones that resolve to MultiIdxDb* is that the latter
         * ones have their highest bits set as a marker.
         *
         * Not thread-safe, the caller must lock on this.
         */
        IdxDbHndl createMultiIdxDbHndl(const char * domainSelector);


        /**
         * @returns if the given hande is a handle for multi idx db
         * (highest bit is set).
         */
        static bool isMultiIdxDbHndl(IdxDbHndl handle);


        /**
         * Checks handle sanity. Out of bounds indexing (handle is an
         * index to one of two vectors) is checked. If the handle is
         * referring to multi idx db pointers, then the pointer must
         * be non-NULL. If the handle refers to IdxDb* instances then
         * it's up to the allowUndefinedIdxDb argument to allow it or
         * not.
         *
         * throws CpixExc on insanity
         */
        void checkHndlSanity(IdxDbHndl handle,
                             bool      allowUndefinedIdxDb);



        /**
         * During undefineVolume we unregister an index, but we can't
         * immediately delete the corresponding pointer otherwise we
         * risk a crash if there is a job still using it.
         *
         * Thus we do deferred deletion. This method allows scheduling
         * an IIdxDb* instance for deletion, it will be a housekeeping
         * that will destroy it.
         */
        void scheduleForDeletion(IIdxDb * toDestroy);


        /**
         * ALL IIdxDb instances scheduled for deletion (by a call to
         * scheduleForDeletion()) are deleted here.
         */
        void deleteAllScheduledIdxDbs();


        /**
         * Some IIdxDb instance scheduled for deletion (by a call to
         * scheduleForDeletion()) are deleted here. There is a
         * heuristical decision about when a scheduled instance can be
         * actually deleted.
         */
        void deleteSomeScheduledIdxDbs();

    }; // class



}

#endif
