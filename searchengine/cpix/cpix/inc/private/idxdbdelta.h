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

#ifndef CPIXUNITTEST_IDXDBDELTA_H_
#define CPIXUNITTEST_IDXDBDELTA_H_

#include "CLucene.h"

#include "cpixtools.h"
#include "common/cpixlog.h"

namespace Cpix
{

    namespace Impl
    {

        /**
         * These subdirectories are the ones that Cpix uses, sometimes
         * temporarily, during committing index deltas (deletsions,
         * insertions) to disk.
         */

        /**
         * This subdirectory is a potential directory for an actual
         * current index.
         */
        extern const char ONE_COMPLETE_SUBDIR[];

        /**
         * File marker used to tell that the subdir with the
         * ONE_COMPLETE_SUBDIR suffix is the newer.
         */
        extern const char ONE_SUBDIR_NEWER_MARKER[];

        /**
         * This subdirectory is a potential directory for an actual
         * current index.
         */
        extern const char OTHER_COMPLETE_SUBDIR[];

        /**
         * File marker used to tell that the subdir with the
         * OTHER_COMPLETE_SUBDIR suffix is the newer.
         */
        extern const char OTHER_SUBDIR_NEWER_MARKER[];

        /**
         * If the current indexreader has deletions, and if they were
         * committed to disk, then this marker file is used to tell
         * about the fact (namely, that there is a need to have a new
         * version of the index even if there were insertions
         * (insertbuffer was empty)).
         */
        extern const char DELS_COMMITTED_MARKER[];

        /**
         * Insertbuffer is committed to this directory first. Having
         * only this directory under the base path means that
         * committing the insert buffer to disk might have failed -
         * therefore we might have lost insert buffer information.
         */
        extern const char IBUF_COMMITTING_SUBDIR[];

        /**
         * The insertbuffer when committed to disk to the working
         * directory IBUF_COMITTING_SUBDIR, will be renamed to this
         * IBUF_COMMITTED_SUBDIR, which tells that we have the hole
         * insertbuffer information on disk.
         */
        extern const char IBUF_COMMITTED_SUBDIR[];

        /**
         * Merging of the current index (as on disk) with its
         * potential deletions and the insert buffer happens to this
         * temp/work directory first. Having only this directory means
         * that merge might have failed.
         */
        extern const char MERGING_SUBDIR[];


        /**
         * The newer version of an index is indicated by a version
         * marker file that has suffix defined by this constant. It
         * can happen that both the old version and the new version
         * exist on the disk, in which case recovery is made far
         * simpler if the process makes sure that there is a marker
         * indicating the newer version to use.
         */
        extern const char VERSION_MARKER_SUFFIX[];


        using namespace lucene::index;
        using namespace lucene::search;
        using namespace lucene::store;


        /**
         * This interface is an abstraction of what an insert buffer
         * can do for committing purposes: get the ram dir from it and
         * close it entirely.
         */
        class IInsertBuf
        {
        public:
            /**
             * Closes this instance, that is, the ram directory and
             * any other state related to currently inserted items are
             * cleaned up. Not thread safe.
             */
            virtual void close() = 0;


            /**
             * @return true if this insert buffer is empty, false
             * otherwise. Not thread safe.
             */
            virtual bool isEmpty() const = 0;


            /**
             * Gets the ram dir if this insert buffer is NOT
             * empty. Before getting the ram dir, it closes the
             * writer. Not thread safe.
             *
             * @return the ram directory (containing the docs indexed
             * and stored temporarily in RAM). Throws, if the insert
             * buffer is empty.
             */
            virtual lucene::store::TransactionalRAMDirectory * getRAMDir() = 0;


            /**
             * Destructor
             */
            virtual ~IInsertBuf() = 0;
        };


        
        /**
         * An interface expressing the fact that an IndexReader
         * instance is being destructed and re-created. Rationale:
         * there may be some extra actions that need to be done
         * whenever an IndexReader is created or destroyed.
         *
         * NOTE: member functions of this interface that can have a
         * potential access to physical disk (read or commit/close)
         * are given an extra parameter, a mutex (dirMutex). This
         * interface does NOT prescribe, what the lock order between
         * dirMutex and whatever internal mutex an implementation of
         * this interface may have. But it is the responsibility of
         * the implementation to
         *
         *   (a) always use the same lock order
         * 
         *   (b) other classes using the implementation of this
         *   interface stick to the same mutex lock order too
         */
        class IIdxReader
        {
        public:
            //
            // public operations
            //
            
            /**
             * Opens an IndexReader to the given path. The path should
             * contain the clucene index (files). Must be thread-safe
             * from this instance point of view, and physical
             * directory protection is done through dirMutex.
             *
             * NOTE: make sure lock order of dirMutex and whatever
             * other mutex this instance has is always the same.
             *
             * @param clIdxPath the path to the clucene index directory
             * 
             * @param dirMutex the mutex that is used to protect
             * against concurrent accessing the same physical
             * directory
             */
            virtual void open(const char * clIdxPath,
                              Cpt::Mutex & dirMutex) = 0;


            /**
             * Closes this instance if it was open. If it was open AND
             * argument reRead is true, then it also re-opens this
             * instance (re-reads the reader) from the given path. In
             * particular, if it was closed, and reRead is false, it
             * will NOT open this.
             *
             * NOTE: make sure lock order of dirMutex and whatever
             * other mutex this instance has is always the same.
             * 
             * @param dirMutex the mutex to lock on, protecting the
             * physical cpix index directory from multiple entities
             * messing it up.
             *
             * @param clIdxPath the path to the clucene index
             * directory (subdirectory of a cpix index
             * directory). Must not be NULL.
             *
             * @param reRead controls re-read. If it is false, than
             * this behaves like a close() method would. If it is true
             * AND if this instance had a valid reader (open state),
             * then that reader is destroyed and then re-read from
             * disk (because there is updated index content on disk).
             */
            virtual void reopen(Cpt::Mutex & dirMutex,
                                const char * clIdxPath,
                                bool         reRead) = 0;


            /**
             * Commits the reader (and its deletions) if
             * necessary. This instance may be in closed state - then
             * no commit is necessary. It might be on opened state,
             * but may have no deletions at all - then no commit is
             * necessary either. Must be thread-safe from this
             * instance point of view, and physical directory
             * protection is done through dirMutex.
             * 
             * NOTE: make sure lock order of dirMutex and whatever
             * other mutex this instance has is always the same.
             *
             * @param dirMutex the mutex that is used to protect
             * against concurrent accessing the same physical
             * directory
             *
             * @return true if it did have deletions to commit (and
             * did so).
             */
            virtual bool commitIfNecessary(Cpt::Mutex & dirMutex) = 0;


            //
            // lifetime mgmt
            //

            /**
             * Destructs this instance (close()-s it).
             */
            virtual ~IIdxReader() = 0;
        };



        /**
         * A static class that implements the algorithm for committing
         * IdxDb changes to disk in recoverable stages.
         *
         * This class could be part of IdxDb, but for clarity and forced
         * separation, it is made a different class.
         */
        class IdxDbDelta
        {
        public:
            //
            // public operations
            //
        
            /**
             * First, it brings the cpix index directory into a
             * consistent state and cleans it up. Then, it tries to
             * create an IndexReader instance from the most up to date
             * index information under the basePath.
             *
             * Under basePath, CPix maintains a number of
             * subdirectories and files, most of them temporarily, as
             * it commits deltas (deletions and insertions) to
             * disk. That long process can be abrubtly killed by a
             * system shutdown or loss of power at any
             * stage. Therefore this function will try to recover any
             * information that is recoverable.
             *
             * @param basePath is the path to the "CPix index database
             * directory" under which Cpix manages a number of
             * subdirectories / files during the commit process. On
             * abrupt system shutdown, a number of these may got
             * stuck, and RecoverReader tries to continue where the
             * CommitToDisk function was killed in order to recover as
             * much of the state of the index as possible.
             *
             * @param idxMutex the mutex that is used to protect
             * physical accesses to the Cpix index directory
             * basePath. It's the caller's responsibility that each
             * such basePath has exactly one corresponding mutex, so
             * no concurrent tampering with files and directories will
             * happen.
             *
             * @param reader the instance that is currently in closed
             * state and needs its open() function called with the
             * proper clucene index path, where everything is possibly
             * cleaned up and updated. It may be NULL, in which case
             * no reader load operation will happen. In this case,
             * this call, is essentially used to make cpix directory
             * consistent and clean.
             *
             */
            static void RecoverReader(const char * basePath,
                                      Cpt::Mutex & idxMutex,
                                      IIdxReader * reader);


            /**
             * This function will try to commit the delta (deletions
             * and insert buffer (=writer + ramdir)) to disk. It tries
             * to commit the individual pieces like deletions and
             * insertbuffer first, so that an abrupt system shutdown
             * will have less chance of losing some delta information.
             *
             * Searcher needs to be re-created depending on whether it
             * existed or not.
             *
             * Reader needs to be re-created depending on whether it
             * existed or not.
             *
             * There may have been deleted items or not. Deleted items are
             * only if there is a reader. Searcher is only if there is a
             * reader.
             *
             * There may have been an insert buffer or not. Writer and
             * insertbuffer exist exactly the same time.
             *
             * @param basePath is the path to the "CPix index database
             * directory" under which CPix manages a number of
             * subdirectories / files during the commit process. Must
             * not be NULL.
             *
             * @param insertBuf the insert buffer containing the fresh
             * docs indexed held in RAM temporarily. If it is not NULL
             * and was not empty (IInsertBuf::isEmpty()), it will be
             * close()-d (IInsertBUf::close()). May be NULL.
             *
             * @param reader the reader that may hold some deletion
             * information and may need to be close()-d and
             * re-open()-ed, if necessary.
             *
             * @param reRead the client may want to commit only to
             * shut down everything, in which case it does not want
             * the reader to be re-open()-ed. Or, it may want to
             * commit and have an updated (re-open()-ed) reader and
             * continue with the searches.
             *
             * NOTE: If the reader.commitIfNecessary() returned false,
             * then the reader will not be closed. Giving a false
             * value for reRead does not mean, therefore, that the
             * reader will be closed. Giving false for reRead only
             * means that if it was closed, then won't be re-read.
             *
             * @param idxMutex the mutex that is used to protect
             * physical accesses to the Cpix index directory
             * basePath. It also protects insertBuf instance (if
             * any). It's the caller's responsibility that each such
             * basePath has exactly one corresponding mutex, so no
             * concurrent tampering with files and directories will
             * happen.
             *
             * NOTE: LOCK ORDER OF idxMutex_ and whatever there is
             * within reader is up to the reader implementation. The
             * param 'idxMutex' is never held when reader operations
             * are invoked, rather, the idxMutex is given to those
             * reader methods and they can decide what lock order they
             * use.
             */
            static void CommitToDisk(const char                  * basePath,
                                     IInsertBuf                  * insertBuf,
                                     IIdxReader                  & reader,
                                     bool                          reRead,
                                     Cpt::Mutex                  & idxMutex);


        };



        /**
         * Calls the close() method on x, catching any exceptions and
         * logging the fact that close() has failed. It does not
         * re-throw the exception.
         */
        template<typename X>
        void CallCloseLogging(X & x);



        /**
         * This class is a very simple replacement for
         * std::auto_ptr. It does not support assigment and stuff,
         * therefore it's only intended to be used as a local (or
         * member) variable, not as a function argument or return
         * value.
         *
         * It knows only one thing that std::auto_ptr does not: on
         * destruction, it will call "close()" on the stored pointer,
         * if it is not NULL.
         *
         * This way can avoid leaking some clucene resources (like
         * file system locks etc) that are released by a close() call
         * but by the destructor.
         */
        template<typename X>
        class cl_auto_ptr
        {
        private:
            //
            // private members
            //
            X        * ptr_;

            
            // No value semantics
            cl_auto_ptr(const cl_auto_ptr &);
            cl_auto_ptr & operator=(const cl_auto_ptr &);

        public:
            //
            // public operators - these are all very much like the
            // ones found in std::auto_ptr, with the same
            // semantics. Two exceptions: the dereferencing operators
            // (*, ->) are not declared no-throw, as the assertion for
            // NULL-ptr check is implemented with THROW_CPIXEXC.
            //

            explicit cl_auto_ptr(X * ptr) throw();
            ~cl_auto_ptr() throw ();
            
            X & operator*() const;
            X * operator->() const;
            X * get() const throw();
            X * release() throw();
            void reset(X * ptr = NULL) throw();
        };




    } // ns


} // ns





/*****************************************************************
 *
 * Implementation details
 *
 */

namespace Cpix
{
    namespace Impl
    {


        /**
         * Class for computing all the possible cpix-managed
         * subdirectories and files under a base path. Plus some
         * utilities.
         */
        struct CpixPaths
        {
            std::string     oneDir_;
            std::string     oneDirNewerMarkerFile_;
            std::string     otherDir_;
            std::string     otherDirNewerMarkerFile_;
            std::string     delsMarkerFile_;
            std::string     ibufCommittingDir_;
            std::string     ibufCommittedDir_;
            std::string     mergingDir_;

            
            /**
             * Tells about the existence of oneDir_ and otherDir_
             */
            enum IdxDirsExistence
                {
                    /**
                     *  Neither of oneDir_ and otherDir_ exist
                     */
                    IDXE_NEITHER,

                    /**
                     * Only of of oneDir_ and otherDir_ exist
                     */
                    IDXE_ONE,

                    /**
                     * Both oneDir_ and otherDir_ exist
                     */
                    IDXE_BOTH
                };

            
            IdxDirsExistence    idxDirsExistence_;
            std::string       * curDir_;
            std::string       * newDir_;
            

            /**
             * Sets the subdirectory / file paths used by CPix under
             * basePath.
             *
             * If both oneDir_ and otherDir_ exist, bothIdxDirsExist_
             * will be set to true and the older one will be referred
             * by curDir_ and the newer one will be referred by
             * newDir_.
             *
             * If only one of oneDir_ and otherDir_ exists, that one
             * will be referred by curDir_ and the non-existing one
             * will be referred by newDir_.
             *
             * If neither of them exists, curDir_ will refer to
             * oneDir_ and newDir_ will refer to otherDir_.
             */
            CpixPaths(const char * basePath);


            /**
             * @param path to a putative clucene index directory
             *
             * @return true if the clucene index exists and can be
             * loaded from under the path, false otherwise.
             */
            static bool HasValidIndex(const std::string & path);


            /**
             * Removes all contents from under a directory with the
             * exception of one content.
             * 
             * @param path the directory under which almost everything
             * should be deleted.
             *
             * @param exception a path which is a sub-path of path,
             * that should not be deleted.
             */
            static void RemoveExcept(const char * path,
                                     const char * exception);


            /**
             * A "clean" cpix directory is one where there is at most
             * one subdirectory, either called '_0' or '_1', and no
             * other subdirectories or files. This static function
             * checks if the given cpix dir is "clean".
             */
            static bool IsCleanCpixDir(const char * basePath);
        };



        /**
         * While committing to disk, power may be lost / phone can be
         * switched off at any stage.
         *
         * These enums signify different stages (states of disk) of
         * committing process.
         */
        enum CommitStage
            {
                /**
                 * Deletions have been committed and the marker file
                 * has been created.
                 */
                CS_DELS_COMMITTED,

                /**
                 * Insertbuffer committing has started.
                 */
                CS_IBUF_COMMITTING,

                /**
                 * Insertbuffer committing has completed.
                 */
                CS_IBUF_COMMITTED,

                /** Merging of the current index and the delta has
                 * started.
                 *
                 */
                CS_MERGING,

                /**
                 * Merging of the current index and the delta has
                 * completed.
                 */
                CS_MERGED,


                /**
                 * All temp directories / files under a basePath
                 * ("CPix index database directory") have been cleaned
                 * up.
                 */
                CS_COMPLETE
            };



        /**
         * The CommitToStage internal implementation can be
         * interrupted (for testing reasons) at different stages of it
         * progress. The default functor that it calls before
         * commencing a new stage is an instance of this class: it
         * does nothing at all - therefore letting the process to run
         * its completion. A tester code may use another functor class
         * that does throw at certain stages.
         */
        struct DefaultCommitStageAction
        {
            /**
             * Does nothing (but it could).
             */
            void operator()(CommitStage commitStage);
        };


        
        /**
         * By looking at a basepath ("Cpix index database directory")
         * it guesses at what stage the last commit as in when the
         * system shut down. The commit operation may have also
         * succeeded.
         *
         * @return the commit stage from which recovery must continue
         * the commit process first in order to recover most (if not
         * all) of the delta information.
         */
        CommitStage GuessCommitStage_(const char * basePath);
        


        /**
         * Commits the insertbuffer to disk (into the worker/temp
         * insertbuffer directory). It closes the writer, if not NULL,
         * and resets it to NULL. It uses the ramDir to commit its
         * contents to disk, if not NULL, and then destroys it and
         * resets it to NULL.
         *
         * At the end of a successful operation all the insertbuffer
         * state should reside in the insertbuffer worker subdirectory
         * under paths.
         *
         * @param paths the CpixPaths POV instance that holds all
         * subdirectory paths for a given basePath.
         *
         * @param insertBuf the insert buffer, may be NULL.
         */
        void CommitInsertBuffer_(CpixPaths                   & paths,
                                 IInsertBuf                  * insertBuf);
        

        /**
         * The worker/temp subdir holds the full insertbuffer info,
         * and it must be renamed to express the fact that it has the
         * complete insertbuffer info.
         *
         * @param paths the CpixPaths POV instance that holds all
         * subdirectory paths for a given basePath.
         */
        void RenameInsertBufferCommittingDir_(CpixPaths & paths);


        /**
         * Merges the current index (with possible deletion states)
         * and the insertbuffer into a worker/temp directory. (A
         * recovery process thus need not try and load the index to
         * test for its sanity, because it can be confident that the
         * index is sane.)
         *
         * @param paths the CpixPaths POV instance that holds all
         * subdirectory paths for a given basePath.
         */
        void MergeParts_(CpixPaths & paths);


        /**
         * Merging the current index and the delta (deletions +
         * insertions) has successfully completed, now the worker/temp
         * directory to which the merge happens needs to be renamed to
         * express the fact.  (A recovery process thus need not try
         * and load the index to test for its sanity, because it can
         * be confident that the index is sane.)
         */
        void RenameMergingDir_(CpixPaths & paths);



        /**
         * Cleans up a "CPix repository" directory, basically it
         * removes everything under basePath except one subdirectory
         * (the freshest updated state of the index).
         */
        void CleanupCPixDir_(const char * basePath,
                             const char * exception);


        /**
         * Actually performs the steps of merging stuff on disk from
         * the point that deletions have been committed.
         *
         * @param basePath the path to the "Cpix index database
         * directory" under which temp/worker dirs/files will be
         * created and deleted as the merge progresses.
         *
         * @param insertBuf the insert buffer, may be NULL.
         *
         * @param commitStage the stage from which point to continue
         * the merge process. A previous merge process might have
         * gotten to a point and then might have been killed by abrupt
         * system shutdown. So we need to try to continue where it
         * stopped.
         *
         * @param csa the instance that can force abrupt "failures"
         * for testing reasons. A csa instance may do nothing, but if
         * it forces failures, it should do when the commitStage just
         * completed is the same as the one at which csa wants to kill
         * the progress at.
         *
         * @param idxDbPath the path to the updated clucene index
         * directory (subdirectory of basePath (=Cpix index
         * directory)) will be set in this instance.
         * 
         */
        template<typename COMMITSTAGEACTION>
        void MergeOnDisk_(const char                  * basePath,
                          IInsertBuf                  * insertBuf,
                          CommitStage                   commitStage,
                          COMMITSTAGEACTION             csa,
                          std::string                 & idxDbPath)
        {
            using namespace std;
            using namespace Cpt;

            CpixPaths
                paths(basePath);

            if (paths.idxDirsExistence_ == CpixPaths::IDXE_NEITHER)
                {
                    THROW_CPIXEXC(PL_ERROR "Neither of the idx dirs exist under '%s'",
                                  basePath);
                }

            idxDbPath = *paths.newDir_;

            switch (commitStage)
                {
                case CS_DELS_COMMITTED:

                    CommitInsertBuffer_(paths,
                                        insertBuf);

                    csa(CS_IBUF_COMMITTING);

                    // intentional fall-through
                case CS_IBUF_COMMITTING:

                    RenameInsertBufferCommittingDir_(paths);

                    csa(CS_IBUF_COMMITTED);

                    // intentional fall-through
                case CS_IBUF_COMMITTED:

                    MergeParts_(paths);

                    csa(CS_MERGING);
                    
                    // intentional fall-through
                case CS_MERGING:

                    RenameMergingDir_(paths);

                    csa(CS_MERGED);
                    
                    // intentional fall-through
                case CS_MERGED:

                    csa(CS_COMPLETE);

                    break;
                case CS_COMPLETE:
                    // if it was already in the complete state (notice
                    // the "break" before this case, there is no
                    // fall-through), then it is the curDir_ that
                    // holds the path to existing database
                    idxDbPath = *paths.curDir_;
                    break;
                default:
                    THROW_CPIXEXC(PL_ERROR "Unknown commitStage %d",
                                  commitStage);
                }
        }
                                


        /**
         * Actual implementation for IdxDbDelta::CommitToDisk. It
         * almost has the same signature, and conceptually has the
         * same semantics, therefore see comments there.
         *
         * @param csa the extra parameter that
         * IdxDbDelta::CommitToDisk does not have. This functor can be
         * used to emulate abrupt system shutdown during different
         * stages of the commit process. Cf more comments at
         * MergeOnDisk_.
         *
         * IdxDbDelta::CommitToDisk implementation itself uses a
         * functor instance that does nothing, therefore it will never
         * force "test-failures". A tester code might use another
         * functor that does so.
         */
        template<typename COMMITSTAGEACTION>
        void CommitToDisk_(const char                  * basePath,
                           IInsertBuf                  * insertBuf,
                           IIdxReader                  & reader,
                           bool                          reRead,
                           Cpt::Mutex                  & idxMutex,
                           COMMITSTAGEACTION             csa)
        {
            using namespace std;
            using namespace Cpt;

            bool
                hadInsertBuffer = false;

            { // SYNC
                SyncRegion
                    sr(idxMutex);

                hadInsertBuffer = insertBuf != NULL && !insertBuf->isEmpty();
            } // SYNC

            bool
                deleted = reader.commitIfNecessary(idxMutex);

            string
                idxDbPath;

            { // SYNC
                SyncRegion
                    sr(idxMutex);

                if (deleted)
                    {
                        string
                            delsMarker(basePath);
                        pathappend(delsMarker,
                                   DELS_COMMITTED_MARKER);

                        int
                            result = touch(delsMarker.c_str(),
                                           0666);

                        if (result != 0)
                            {
                                THROW_CPIXEXC("Could not create '%s'",
                                              delsMarker.c_str());
                            }
                    }

                csa(CS_DELS_COMMITTED);

                logDbgMsg("IdxDbDelta::CommitToDisk_ MIDDLE 1");
                          
                MergeOnDisk_(basePath,
                             insertBuf,
                             CS_DELS_COMMITTED,
                             csa,
                             idxDbPath);

                logDbgMsg("IdxDbDelta::CommitToDisk_ MIDDLE 2");
            } // SYNC

            // deleted true: reader is open, and had to commit deletions
            // hadInsertBuffer: there was content in insert buffer
            if (deleted || hadInsertBuffer)
                {
                    logDbgMsg("IdxDbDelta::CommitToDisk_ MIDDLE 3.1");

                    reader.reopen(idxMutex,
                                  idxDbPath.c_str(),
                                  reRead);

                    // if performed any disk operations, we clean up now

                    logDbgMsg("IdxDbDelta::CommitToDisk_ MIDDLE 3.2");

                    SyncRegion
                        sr(idxMutex);

                    CleanupCPixDir_(basePath,
                                    idxDbPath.c_str());

                    logDbgMsg("IdxDbDelta::CommitToDisk_ MIDDLE 3.3");
                }
        }




        //////////////////////////////////////////////////////////
        //
        //  Implementation for CallCloseLogging
        //
        //
        template<typename X>
        void CallCloseLogging(X & x)
        {
            try
                {
                    x.close();
                }
            catch (...)
                {
                    logMsg(CPIX_LL_ERROR,
                           "Unknown exception: at %s::close().",
                           typeid(X).name());
                }
        }



        //////////////////////////////////////////////////////////
        //
        //  Implementation for cl_auto_ptr
        //
        //
        template<typename X>
        cl_auto_ptr<X>::cl_auto_ptr(X * ptr) throw()
            : ptr_(ptr)
        {
            ;
        }


        template<typename X>
        cl_auto_ptr<X>::~cl_auto_ptr() throw ()
        {
            reset();
        }
            

        template<typename X>
        X & cl_auto_ptr<X>::operator*() const
        {
            if (get() == NULL)
                {
                    THROW_CPIXEXC(PL_ERROR "Referencing (*) NULL cl_auto_ptr");
                }

            return *get();
        }


        template<typename X>
        X * cl_auto_ptr<X>::operator->() const
        {
            if (get() == NULL)
                {
                    THROW_CPIXEXC(PL_ERROR "Referencing (->) NULL cl_auto_ptr");
                }

            return get();
        }


        template<typename X>
        X * cl_auto_ptr<X>::get() const throw()
        {
            return ptr_;
        }


        template<typename X>
        X * cl_auto_ptr<X>::release() throw()
        {
            X
                * rv = get();

            ptr_ = NULL;

            return rv;
        }


        template<typename X>
        void cl_auto_ptr<X>::reset(X * ptr) throw()
        {
            if (ptr_ != NULL)
                {
                    CallCloseLogging(*ptr_);
                    delete ptr_;
                }

            ptr_ = ptr;
        }





    } // ns
} // ns



#endif // CPIXUNITTEST_IDXDBDELTA_H_

