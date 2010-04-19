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

#include <unistd.h>

#include "cpixexc.h"

#include "common/cpixlog.h"

#include "idxdbdelta.h"
#include "fwdtypes.h"
#include "iidxdb.h"
#include "idxdbmgr.h"


namespace
{

    /**
     * TODO comment
     */
    bool isSuffixOf(const char * little,
                    const char * big)
    {
        bool
            rv = false;

        size_t
            littleSize = strlen(little),
            bigSize = strlen(big);

        if (littleSize <= bigSize)
            {
                const char
                    * littleCur = little + littleSize - 1,
                    * bigCur = big + bigSize - 1;

                for (; 
                     littleCur >= little 
                         && bigCur >= big
                         && *littleCur == *bigCur; 
                     --littleCur, --bigCur)
                    {
                        ;
                    }

                rv = (littleCur == (little - 1));
            }

        return rv;
    }



    /**
     * Removes everything under a certain path except an
     * exception-path.
     */
    class RemoverWithException : public Cpt::IFileVisitor
    {
    private:
        int          success_;
        const char * startPath_;
        const char * exception_;

        std::string  newerMarkerFile_;

    public:
        /**
         * Arguments are not copied, only stored, so make sure
         * those paths exists while this instance is alive.
         */
        RemoverWithException(const char * path,
                             const char * exception)
            : success_(0),
              startPath_(path),
              exception_(exception)
        {
            ;
        }

            
        //
        // from interface Cpt::IFileVisitor
        //
        int success() const
        {
            return success_;
        }


        virtual bool visitFile(const char * path)
        {
            if (isSuffixOf(Cpix::Impl::VERSION_MARKER_SUFFIX, path))
                {
                    // if path is a version marker file then defer its
                    // deletion
                    newerMarkerFile_ = path;
                }
            else
                {
                    removePath(path);
                }

            return success_ == 0;
        }


        virtual DirVisitResult visitDirPre(const char * path)
        {
            DirVisitResult
                rv = IFV_CONTINUE;

            if (exception_ != NULL
                && strcmp(path, exception_) == 0)
                {
                    rv = IFV_SKIPTHISDIR;
                }

            return rv;
        }


        virtual bool visitDirPost(const char * path)
        {
            bool
                rv = true;

            bool
                doesNotMatchStartPath = 
                startPath_ == NULL || strcmp(path, startPath_) != 0;

            bool
                doesNotMatchException =
                exception_ == NULL || strcmp(path, exception_) != 0;
                
            if (doesNotMatchStartPath && doesNotMatchException)
                {
                    rv = visitFile(path);
                }

            if (rv
                && strcmp(path, startPath_) == 0
                && newerMarkerFile_.length() > 0)
                {
                    // the version marker file, if any, as the last
                    // item
                    removePath(newerMarkerFile_.c_str());
                    rv = success_ == 0;
                }

            return rv;
        }
            

    private:
        //
        // private members
        //
        void removePath(const char * path)
        {
            if (success_ != 0)
                {
                    logMsg(CPIX_LL_ERROR,
                           PL_ERROR "removing %s, when already broken.",
                           path);
                }

            // 'remove' works for files and directories
            Cpt_EINTR_RETRY(success_,remove(path));

            if (success_ != 0)
                {
                    logMsg(CPIX_LL_ERROR,
                           "RemoverWithException::removePath Failed, errno: %d, %s",
                           errno,
                           path);
                }
        }
    };




}



namespace Cpix
{

    namespace Impl
    {
        /////////////////////////////////////////////////////
        // IInsertBuf
        //
        IInsertBuf::~IInsertBuf()
        {
            ;
        }


        /////////////////////////////////////////////////////
        // IIdxReader
        //
        IIdxReader::~IIdxReader()
        {
            ;
        }


        /////////////////////////////////////////////////////
        // CpixPaths
        //
        CpixPaths::CpixPaths(const char * basePath)
            : oneDir_(basePath),
              oneDirNewerMarkerFile_(basePath),
              otherDir_(basePath),
              otherDirNewerMarkerFile_(basePath),
              delsMarkerFile_(basePath),
              ibufCommittingDir_(basePath),
              ibufCommittedDir_(basePath),
              mergingDir_(basePath),
              idxDirsExistence_(IDXE_NEITHER),
              curDir_(NULL),
              newDir_(NULL)
        {
            using namespace Cpt;

            pathappend(oneDir_,
                       ONE_COMPLETE_SUBDIR);
            pathappend(oneDirNewerMarkerFile_,
                       ONE_SUBDIR_NEWER_MARKER);
            pathappend(otherDir_,
                       OTHER_COMPLETE_SUBDIR);
            pathappend(otherDirNewerMarkerFile_,
                       OTHER_SUBDIR_NEWER_MARKER);
            pathappend(delsMarkerFile_,
                       DELS_COMMITTED_MARKER);
            pathappend(ibufCommittingDir_,
                       IBUF_COMMITTING_SUBDIR);
            pathappend(ibufCommittedDir_,
                       IBUF_COMMITTED_SUBDIR);
            pathappend(mergingDir_,
                       MERGING_SUBDIR);

            bool
                oneDirExists = directoryexists(oneDir_.c_str()),
                otherDirExists = directoryexists(otherDir_.c_str());

            if (oneDirExists && otherDirExists)
                {
                    idxDirsExistence_ = IDXE_BOTH;

                    bool
                        oneDirNewer = true;

                    // first, we try to decide using the marker
                    if (isfile(oneDirNewerMarkerFile_.c_str()))
                        {
                            oneDirNewer = true;
                        }
                    else if (isfile(otherDirNewerMarkerFile_.c_str()))
                        {
                            oneDirNewer = false;
                        }
                    else
                        {
                            // if there is no marker (failed to create
                            // it?), we try to look at modification dates
                            time_t
                                oneModTime = filemodified(oneDir_.c_str()),
                                otherModTime = filemodified(otherDir_.c_str());
                            
                            // the later one is the newer, if they
                            // happen to have the exact same date
                            // (second granurality allows it), then we
                            // just pick the other, but that's
                            // arbitrary (the best we can do)
                            oneDirNewer = oneModTime > otherModTime;

                            if (oneModTime == otherModTime)
                                {
                                    logMsg(CPIX_LL_WARNING,
                                           "No marker file and identical mod dates in cpix dir %s: picking arbitrary version - information may be lost",
                                           basePath);
                                }
                        }

                    if (oneDirNewer)
                        {
                            curDir_ = &otherDir_;
                            newDir_ = &oneDir_;
                        }
                    else
                        {
                            curDir_ = &oneDir_;
                            newDir_ = &otherDir_;
                        }
                }
            else if (oneDirExists)
                {
                    idxDirsExistence_ = IDXE_ONE;

                    curDir_ = &oneDir_;
                    newDir_ = &otherDir_;
                }
            else if (otherDirExists)
                {
                    idxDirsExistence_ = IDXE_ONE;

                    curDir_ = &otherDir_;
                    newDir_ = &oneDir_;
                }
            else
                {
                    curDir_ = &oneDir_;
                    newDir_ = &otherDir_;
                }
        }


        bool CpixPaths::HasValidIndex(const std::string & path)
        {
            bool
                rv = false;

            try
                {
                    using namespace std;
                    using namespace lucene::index;

                    cl_auto_ptr<IndexReader>
                        reader(IndexReader::open(path.c_str()));

                    // close()-ing reader is implied by cl_auto_ptr dtor
                    reader.reset();
                    rv = true;
                }
            catch (...)
                {
                    ;
                }

            return rv;
        }



        void CpixPaths::RemoveExcept(const char * path,
                                     const char * exception)
        {
            RemoverWithException
                rwe(path,
                    exception);

            Cpt::traverse(path,
                          &rwe);

            if (rwe.success() != 0)
                {
                    logMsg(CPIX_LL_ERROR,
                           "Could not cleanup '%s' (leaving '%s' out)",
                           path,
                           exception);
                    THROW_CPIXEXC("Could not cleanup '%s' (leaving '%s' out)",
                                  path,
                                  exception);
                }
        }



        bool CpixPaths::IsCleanCpixDir(const char * basePath)
        {
            using namespace Cpt;
            using namespace std;

            string
                oneDir(basePath),
                oneDirNewerMarkerFile(basePath),
                otherDir(basePath),
                otherDirNewerMarkerFile(basePath),
                delsMarkerFile(basePath),
                ibufCommittingDir(basePath),
                ibufCommittedDir(basePath),
                mergingDir(basePath);

            pathappend(oneDir,
                       ONE_COMPLETE_SUBDIR);
            pathappend(oneDirNewerMarkerFile,
                       ONE_SUBDIR_NEWER_MARKER);
            pathappend(otherDir,
                       OTHER_COMPLETE_SUBDIR);
            pathappend(otherDirNewerMarkerFile,
                       OTHER_SUBDIR_NEWER_MARKER);
            pathappend(delsMarkerFile,
                       DELS_COMMITTED_MARKER);
            pathappend(ibufCommittingDir,
                       IBUF_COMMITTING_SUBDIR);
            pathappend(ibufCommittedDir,
                       IBUF_COMMITTED_SUBDIR);
            pathappend(mergingDir,
                       MERGING_SUBDIR);

            bool
                oneDirExists = directoryexists(oneDir.c_str()),
                otherDirExists = directoryexists(otherDir.c_str());

            bool
                rv = false;

            if (!oneDirExists || !otherDirExists)
                {
                    rv =   !directoryexists(ibufCommittingDir.c_str())
                        && !directoryexists(ibufCommittedDir.c_str())
                        && !directoryexists(mergingDir.c_str())
                        && !isfile(oneDirNewerMarkerFile.c_str())
                        && !isfile(otherDirNewerMarkerFile.c_str())
                        && !isfile(delsMarkerFile.c_str());
                }

            return rv;
        }
        



        /////////////////////////////////////////////////////
        // Path suffixes
        //
        const char ONE_COMPLETE_SUBDIR[]       = "_0";
        const char ONE_SUBDIR_NEWER_MARKER[]   = "_0newer.mrk";
        const char OTHER_COMPLETE_SUBDIR[]     = "_1";
        const char OTHER_SUBDIR_NEWER_MARKER[] = "_1newer.mrk";
        const char DELS_COMMITTED_MARKER[]     = "_dels.mrk";
        const char IBUF_COMMITTING_SUBDIR[]    = "_ibuftmp";
        const char IBUF_COMMITTED_SUBDIR[]     = "_ibufdone";
        const char MERGING_SUBDIR[]            = "_mergetmp";
        const char VERSION_MARKER_SUFFIX[]     = "newer.mrk";



        /////////////////////////////////////////////////////
        // Delta function implementations
        //
        void IdxDbDelta::RecoverReader(const char * basePath,
                                       Cpt::Mutex & idxMutex,
                                       IIdxReader * reader)
        {
            using namespace std;

            string
                idxDbPath;

            { // SYNC
                Cpt::SyncRegion
                    sr(idxMutex);

                CommitStage
                    commitStage = GuessCommitStage_(basePath);

                MergeOnDisk_(basePath,
                             NULL, // no insert buffer
                             commitStage,
                             DefaultCommitStageAction(),
                             idxDbPath);

                CleanupCPixDir_(basePath,
                                idxDbPath.c_str());
            } // SYNC

            if (reader != NULL)
                {
                    reader->open(idxDbPath.c_str(),
                                 idxMutex);
                }
        }



        void IdxDbDelta::CommitToDisk(const char                  * basePath,
                                      IInsertBuf                  * insertBuf,
                                      IIdxReader                  & reader,
                                      bool                          reRead,
                                      Cpt::Mutex                  & idxMutex)
        {
            CommitToDisk_(basePath,
                          insertBuf,
                          reader,
                          reRead,
                          idxMutex,
                          DefaultCommitStageAction());
        }


        void DefaultCommitStageAction::operator()(CommitStage commitStage)
        {
            ;
            /* TMP
            logDbgMsg("DefaultCommitStageAction:: - stage at %d",
                      commitStage);
            */
        }


        CommitStage GuessCommitStage_(const char * basePath)
        {
            using namespace lucene::index;
            using namespace std;
            using namespace Cpt;

            CpixPaths
                paths(basePath);

            CommitStage
                rv;

            if (paths.idxDirsExistence_ == CpixPaths::IDXE_BOTH)
                {
                    rv = CS_MERGED;
                }
            else if (paths.idxDirsExistence_ == CpixPaths::IDXE_NEITHER)
                {
                    int
                        result = removeunder(basePath);

                    if (result != 0)
                        {
                            THROW_CPIXEXC("Could not clean dir '%s'",
                                          basePath);
                        }

                    result = mkdirs(paths.curDir_->c_str(),
                                    0766);

                    if (result != 0)
                        {
                            THROW_CPIXEXC("Could not create idx dir '%s'",
                                          paths.curDir_->c_str());
                        }

                    IdxDbMgr::RecreateFsClIdx(paths.curDir_->c_str());

                    rv = CS_COMPLETE;
                }
            else if (CpixPaths::HasValidIndex(paths.mergingDir_))
                {
                    rv = CS_MERGING;
                }
            else if (directoryexists(paths.ibufCommittedDir_.c_str()))
                {
                    rv = CS_IBUF_COMMITTED;
                }
            else if (CpixPaths::HasValidIndex(paths.ibufCommittingDir_.c_str()))
                {
                    rv = CS_IBUF_COMMITTING;
                }
            else if (isfile(paths.delsMarkerFile_.c_str()))
                {
                    rv = CS_DELS_COMMITTED;
                }
            else
                {
                    rv = CS_COMPLETE;
                }

            return rv;
        }



        void CommitInsertBuffer_(CpixPaths                   & paths,
                                 IInsertBuf                  * insertBuf)
        {
            using namespace Cpt;
            using namespace lucene::index;

            if (insertBuf != NULL && !insertBuf->isEmpty())
                {
                    // create target directory (a temp directory)
                    int
                        result = mkdirs(paths.ibufCommittingDir_.c_str(),
                                        0766);

                    if (result != 0)
                        {
                            THROW_CPIXEXC("Could not create dir '%s'",
                                          paths.ibufCommittingDir_.c_str());
                        }

                    // create index writer to write to target dir
                    lucene::analysis::SimpleAnalyzer
                        dummyAnalyzer;

                    cl_auto_ptr<IndexWriter>
                        target(new IndexWriter(paths.ibufCommittingDir_.c_str(),
                                               &dummyAnalyzer,
                                               true,   // create
                                               true));
                    
                    Directory
                        * dirsToAdd[] = {
                        insertBuf->getRAMDir(),
                        NULL
                    };

                    // merge (add) ramDir's contents to target
                    target->addIndexes(dirsToAdd);

                    // close()-ing target - implied by cl_auto_ptr dtor
                    target.reset();

                    // clean up all insert state in insert buffer
                    //
                    // NOTE: this close() is to be called only if
                    // previous operations were successful
                    insertBuf->close();
                }
        }



        void RenameInsertBufferCommittingDir_(CpixPaths & paths)
        {
            using namespace Cpt;

            if (directoryexists(paths.ibufCommittingDir_.c_str()))
                {
                    int
                        result = rename(paths.ibufCommittingDir_.c_str(),
                                        paths.ibufCommittedDir_.c_str());

                    if (result != 0)
                        {
                            THROW_CPIXEXC("Could not rename '%s' to '%s': %d",
                                          paths.ibufCommittingDir_.c_str(),
                                          paths.ibufCommittedDir_.c_str(),
                                          result);
                        }
                }
        }


        void MergeParts_(CpixPaths & paths)
        {
            using namespace Cpt;
            using namespace lucene::store;

            bool
                iBufDirExists = directoryexists(paths.ibufCommittedDir_.c_str());

            if (isfile(paths.delsMarkerFile_.c_str())
                || iBufDirExists)
                {
                    int
                        result = mkdirs(paths.mergingDir_.c_str(),
                                        0766);
                    if (result != 0)
                        {
                            THROW_CPIXEXC("Could not create '%s'",
                                          paths.mergingDir_.c_str());
                        }

                    // readers to merge, at maximum 2 (current index
                    // and the insertbuffer, if any), plus the third
                    // item is just a placefolder for NULL
                    Directory
                        * dirsToMerge[] = {
                        NULL,
                        NULL,
                        NULL
                    };

                    const char
                        * path = paths.curDir_->c_str();
                    const bool
                        dontCreate = false;

                    Directory
                        * curIdxDir = FSDirectory::getDirectory(path,
                                                                dontCreate),
                        * iBufDir = NULL;
                    
                    dirsToMerge[0] = curIdxDir;
                    
                    try
                        {
                            
                            if (iBufDirExists)
                                {
                                    path = paths.ibufCommittedDir_.c_str();
                                    
                                    iBufDir = FSDirectory::getDirectory(path,
                                                                        dontCreate);
                                    dirsToMerge[1] = iBufDir;
                                }
                            
                            lucene::analysis::SimpleAnalyzer
                                dummyAnalyzer;
                            
                            cl_auto_ptr<IndexWriter>
                                writer(new IndexWriter(paths.mergingDir_.c_str(),
                                                       & dummyAnalyzer,
                                                       true,    // create
                                                       true));
                            
                            writer->addIndexes(dirsToMerge);

                            // close()-ing writer is implied by cl_auto_ptr
                            writer.reset();

                            // see comments below (in catch clause)
                            CallCloseLogging(*curIdxDir);
                            _CLDECDELETE(curIdxDir);
                            if (iBufDir != NULL)
                                {
                                    CallCloseLogging(*iBufDir);
                                    _CLDECDELETE(iBufDir);
                                }
                        }
                    catch (...)
                        {
                            // we release manually these instances,
                            // but why don't we use std::auto_ptr?
                            //
                            // These FSDirectory instances are aquired
                            // by the FSDirectory::getDirectory call,
                            // which creates them with clucene's own
                            // _CLNEW macro. Now, Directory instances
                            // are also reference counted, so if they
                            // were created by clucene, they should
                            // also be released the way clucene
                            // releases them, and that's _CLDECDELETE
                            // macro. To make it worse, FSDirectory
                            // instances are collected in global cache
                            // (clucene internal) and to de-register
                            // from that cache, the close() member
                            // function has to be called, otherwise
                            // leak occurs.
                            //
                            // In short, an FSDirectory instance
                            // (acquired through getDirectory() call)
                            // must be close()-d and _CLDECDELETE-ed
                            // to get rid of properly.
                            CallCloseLogging(*curIdxDir);
                            _CLDECDELETE(curIdxDir);
                            if (iBufDir != NULL)
                                {
                                    CallCloseLogging(*iBufDir);
                                    _CLDECDELETE(iBufDir);
                                }

                            throw;
                        }
                }
        }



        void RenameMergingDir_(CpixPaths & paths)
        {
            using namespace std;
            using namespace Cpt;

            if (directoryexists(paths.mergingDir_.c_str()))
                {
                    string
                        newDirMarker(paths.newDir_->c_str());
                    newDirMarker += VERSION_MARKER_SUFFIX;

                    int
                        result = touch(newDirMarker.c_str(),
                                       0666);

                    if (result != 0)
                        {
                            // failure to create the marker file is
                            // not considered an error, as CpixPaths
                            // constructor uses a fall-back method
                            // (checking modification dates) to decide
                            // which index version is newer - see
                            // above
                            logMsg(CPIX_LL_WARNING,
                                   "Could not create '%s': %d",
                                   newDirMarker.c_str(),
                                   result);
                        }

                    
                    result = rename(paths.mergingDir_.c_str(),
                                    paths.newDir_->c_str());
                    
                    if (result != 0)
                        {
                            THROW_CPIXEXC("Could not rename '%s' to '%s': %d",
                                          paths.mergingDir_.c_str(),
                                          paths.newDir_->c_str(),
                                          result);
                        }
                }
        }

        

        void CleanupCPixDir_(const char * basePath,
                             const char * exception)
        {
            CpixPaths::RemoveExcept(basePath,
                                    exception);
        }


    } // ns

} // ns
