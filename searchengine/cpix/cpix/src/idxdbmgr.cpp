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

#include <fstream>
#include <map>
#include <vector>
#include <list>
#include <string>

#include "CLucene.h"
#include "CLucene/queryParser/MultiFieldQueryParser.h"

#include "cpixtools.h"

#include "cpixexc.h"
#include "cpixidxdb.h"

#include "idxdb.h"

#include "iidxdb.h"
#include "multiidxdb.h"

#include "indevicecfg.h"

#include "idxdbmgr.h"
#include "idxdbdelta.h"

#include "common/cpixlog.h"

namespace
{

    const char VOLID_PREFIX     = '@';
    const char VOLID_DELIMITER  = ':';
    const char DOMSEL_DELIMITER = ',';

    const char APPHIERARCHY_DELIMITER = ' ';

    // TODO this is a very suspicious candidate to move to cpixtool lib,
    // like "cpixbittools.h" or something

    /**
     * Produces a bitmask for an integer type that has the highest bit
     * set. Compile time thingy, does not involve runtime cost.
     */
    template<typename INTEGER>
    struct HighestBit
    {
        enum { Value = (1 << (8*sizeof(INTEGER) - 1)) };
    };
    





}


namespace Cpix
{


    /*********************************************
     *
     *          IIdxDbInfo implementation
     *
     */
    void IIdxDbInfo::incRefCount()
    {
        ++refCount_;
    }


    void IIdxDbInfo::decRefCount()
    {
        if (refCount_ == 0)
            {
                THROW_CPIXEXC(PL_ERROR "Decreasing zero refcount");
            }

        --refCount_;

        if (refCount_ == 0)
            {
                if (ptr_ != NULL)
                    {
                        Impl::cl_auto_ptr<IIdxDb>
                            tmp(ptr_);
                        ptr_ = NULL;

                        // close()-ing and destroying former ptr_ is
                        // implied by cl_auto_ptr dtor
                    }
            }
    }


    int IIdxDbInfo::refCount() const
    {
        return refCount_;
    }


    IIdxDb * IIdxDbInfo::ptr() const
    {
        return ptr_;
    }


    IIdxDb * IIdxDbInfo::getPtrForUse() const
    {
        accessed();

        return ptr_;
    }


    void IIdxDbInfo::setPtr(IIdxDb * ptr)
    {
        if (ptr_ != NULL)
            {
                // at this point the current index is thrown away, any
                // rambuffers and stuff need not be properly put to
                // disk - hence brutal close
                //
                // NOTE: void brutalClose() throw(); - no-throw
                ptr_->brutalClose(); 
                delete ptr_;
                ptr_ = NULL;
            }

        ptr_ = ptr;

        accessed();
    }


    
    IIdxDb * IIdxDbInfo::release()
    {
        IIdxDb
            * rv = ptr_;

        ptr_ = NULL;

        return rv;
    }


    
    /* OBS
    void IIdxDbInfo::destroy(bool resetRefCount)
    {
        setPtr(NULL);

        if (resetRefCount)
            {
                refCount_ = 0;
            }
    }
    */



    void IIdxDbInfo::restIfIdle(long   nowSec,
                                size_t maxIdleSec)
    {
        if (ptr_ != NULL)
            {
                if ((nowSec - lastAccessedSecs_) > maxIdleSec)
                    {
                        Impl::CallCloseLogging(*ptr_);
                    }
                else
                    {
                        ptr_->doHousekeeping();
                    }
            }
    }


    IIdxDbInfo::IIdxDbInfo()
        : refCount_(0),
          ptr_(NULL),
          lastAccessedSecs_(0)
    {
        ;
    }

    
    long IIdxDbInfo::lastAccessedSecs() const
    {
        return lastAccessedSecs_;
    }


    void IIdxDbInfo::dbgDumpState(size_t idx) const
    {
        logTestMsg(CPIX_LL_TRACE,
                   "    o entry [%d] -> (%s, %d)",
                   idx,
                   ptr() == NULL ? "NULL" : "@?",
                   refCount());
        logTestMsg(CPIX_LL_TRACE,
                   "            (last accessed: %ld)",
                   lastAccessedSecs());
               
        if (ptr() != NULL)
            {
                ptr()->dbgDumpState();
            }
    }


    void IIdxDbInfo::accessed() const
    {
        lastAccessedSecs_ = Cpt::GetTimeOfDay();
    }



    /*********************************************
     *
     *          IdxDbMgr implementation
     *
     */

    // IdxDbMgr::IIdxDbToDestroy constructor
    IdxDbMgr::IIdxDbToDestroy::IIdxDbToDestroy(IIdxDb           * ptr,
                                               HousekeepCounter   housekeepCounter)
        : ptr_(ptr),
          housekeepCounter_(housekeepCounter)
    {
        ;
    }


    // IdxDbMgr proper
    void IdxDbMgr::init(InitParams & ip)    {
        if (instance_ == NULL)
            {
                instance_ = new IdxDbMgr(ip);
            }
        else
            {
                THROW_CPIXEXC(PL_ERROR "Re-initing (IdxDbMgr singleton)");
            }
    }



    IdxDbMgr * IdxDbMgr::instance()
    {
        return instance_;
    }

    
    bool isEmpty(std::vector<IIdxDbInfo> & idxDbs)
    {
        using namespace std;

        vector<IIdxDbInfo>::iterator
            found = find_if(idxDbs.begin(),
                            idxDbs.end(),
                            compose1(bind2nd(not_equal_to<int>(),
                                             0),
                                     mem_fun_ref(&IIdxDbInfo::refCount)));

        return found == idxDbs.end();
    }


    void IdxDbMgr::scrapAll()
    {
        using namespace Cpix;

        IdxDbMgr
            * mgr = instance();
        
        mgr->deleteAllScheduledIdxDbs();

        Cpt::SyncRegion
            sr(mgr->mutex_);

        // checking if IIdxDb instances are still in use (they should not be)
        if (!isEmpty(mgr->idxDbs_))
            {
                THROW_CPIXEXC("Indexes are still in use");
            }
        else if (!isEmpty(mgr->multiIdxDbs_))
            {
                THROW_CPIXEXC("(Multi) indexes are still in use");
            }

        int
            result = unlink(mgr->regFilePath_.c_str());

        if (result != 0)
            {
                THROW_CPIXEXC("Could not delete CPix reg file %s",
                              mgr->regFilePath_.c_str());
            }

        mgr->idxDbs_.clear();
        mgr->multiIdxDbs_.clear();
        mgr->qbacs_.clear();
    }


    void IdxDbMgr::shutdownAll()
    {
        delete instance_;
        instance_ = NULL;
    }


    IdxDbHndl IdxDbMgr::create(const char   * qualBaseAppClass)
    {
        using namespace std;
        using namespace Cpix;

        IdxDbHndl
            rv = 0;

        string
            qbac(qualBaseAppClass);
        
        Cpt::SyncRegion
            sr(mutex_);

        map<QualBaseAppClass, PathAndHndl>::iterator
            qbacIt = qbacs_.find(qbac);

        if (qbacIt == qbacs_.end())
            {
                // qbacIt should point to a meaningful entry, as a
                // previously called defineVolume should either throw
                // or make sure that the association exists
                THROW_CPIXEXC(PL_ERROR "unknown qbac '%s'",
                              qualBaseAppClass);
            }

        PathAndHndl
            pathAndHndl = qbacIt->second;
        
        // Any previous instance is to be thrown away
        rv = pathAndHndl.second;

        // we are re-creating the index without its refCount_ reset ...

        createIdxDbInstance(rv,
                            pathAndHndl.first.c_str(),
                            true, // re-create
                            qbac.c_str());

        idxDbs_[rv].incRefCount(); // ... we just increment it

        return rv;
    }



    bool IdxDbMgr::isQualBaseAppClass(const char * domainSelector)
    {
        if (domainSelector == NULL
            || strlen(domainSelector) == 0)
            {
                THROW_CPIXEXC("NULL or empty domain selector '%s'",
                              domainSelector);
            }

        bool
            rv = false;

        if (domainSelector[0] == VOLID_PREFIX)
            {
                const char
                    * delimiter = strchr(domainSelector,
                                         VOLID_DELIMITER);

                if (delimiter == NULL
                    || (delimiter - domainSelector) == strlen(domainSelector))
                    {
                        THROW_CPIXEXC("Garbled domain selector '%s'",
                                      domainSelector);
                    }

                rv = strchr(domainSelector, DOMSEL_DELIMITER) == NULL;
            }

        return rv;
    }



    // see comments for match().
    bool IdxDbMgr::matchAppClass(const char * appClass,
                                 const char * baseAppClass)
    {
        bool
            rv = false;

        if (strlen(appClass) == 0)
            {
                THROW_CPIXEXC("Emptry string for app class");
            }

        if (appClass[0] != VOLID_PREFIX 
            && baseAppClass[0] == VOLID_PREFIX)
            {
                // base app class is qualified, we will find the unq part
                baseAppClass = getBaseAppClassPart(baseAppClass);
            }

        rv = Cpt::issubstroforequal(appClass,
                                    baseAppClass);

        return rv;
    }


    //
    //  Things that should match
    //
    //  DOMAIN SELECTOR                   | BASE APP CLASS
    //  (what a searcher asks for)        | (what you have as physical index)
    //  ----------------------------------+----------------------------------
    //
    //
    //    (multiple volumes use case)
    //
    //  "root file"                         "@c:root file"
    //  "root file"                         "@d:root file"
    //  "@c:root file"                      "@c:root file"
    //  "@d:root file"                      "@d:root file"
    //
    //
    //    (generic/widening search use case)
    //
    //  "root"                              "@c:root file"
    //  "root"                              "@d:root file"
    //  "root"                              "@0:root msg phone sms"
    //  "root msg"                          "@0:root msg phone sms"
    //  "@c:root"                           "@c:root file"
    //  "@d:root"                           "@d:root file"
    //
    //
    //    (selecting arbitrary indexes use case)
    //
    //  "@FIN:root maps,@GBR:root maps"     "@FIN:root maps"
    //  "@FIN:root maps,@GBR:root maps"     "@GBR:root maps"
    //  "root file,root msg phone sms"      "@c:root file"
    //  "root file,root msg phone sms"      "@d:root file"
    //  "root file,root msg phone sms"      "@0:root msg phone sms"
    //  "@c:root file,root msg phone sms"   "@c:root file"
    //  "@c:root file,root msg phone sms"   "@0:root msg phone sms"
    //
    //
    //  Things that should NEVER match
    //
    //  "root file"                         "@0:root msg phone sms"
    //  "@c:root file"                      "@0:root msg phone sms"
    //  "root file"                         "@FIN:root maps"
    //  "@c:root file"                      "@FIN:root maps"
    //
    //
    //  TODO ??? UNIMPLEMENTED USE CASE: narrowing searches
    //
    //  "root file media"                   "@c:root file"  (TODO)
    //  "root file media"                   "@d:root file"  (TODO)
    //  "@c:root file media"                "@c:root file"  (TODO)
    //  "@d:root file media"                "@d:root file"  (TODO)
    //
    //    ... and in these cases search/suggest needs to implement the
    //  extra clause of " AND _appclass:"root file media".
    //
    bool IdxDbMgr::match(const char * domainSelector,
                         const char * baseAppClass)
    {
        bool
            rv = false;

        if (domainSelector == NULL)
            {
                THROW_CPIXEXC("NULL string for domain selector");
            }
        else if (strlen(domainSelector) == 0)
            {
                THROW_CPIXEXC(L"Emptry string for domain selector");
            }

        using namespace std;

        list<string>
            appClasses;
        static const char domainSelectorDelimiter[] = { DOMSEL_DELIMITER, 0};

        Cpt::splitstring(domainSelector,
                         domainSelectorDelimiter,
                         appClasses);

        list<string>::const_iterator
            i = appClasses.begin(),
            end = appClasses.end();

        for (; i != end && !rv; ++i)
            {
                rv = matchAppClass(i->c_str(),
                                   baseAppClass);
            }

        return rv;
    }



    Version IdxDbMgr::getNextVersion()
    {
        Cpt::SyncRegion
            sr(mutex_);

        Version
            rv = version_;

        ++version_;

        return rv;
    }



    IdxDbHndl IdxDbMgr::getHndl(const char * domainSelector,
                                bool         allowMultiSearch)
    {
        using namespace std;
        using namespace Cpix;

        Cpt::SyncRegion
            sr(mutex_);

        if (qbacs_.empty())
            {
                THROW_CPIXEXC("Unregistered qual base app class '%s' (empty registry).",
                              domainSelector);
            }

        IdxDbHndl
            rv = 0;

        if (!isQualBaseAppClass(domainSelector) && allowMultiSearch)
            {
                // multi search case: when it is allowed (coming
                // through cpix_IdxSearch API object) and the base app
                // class is unqalified.
                rv = createMultiIdxDbHndl(domainSelector);
            }
        else
            {
                // normal case: when not allowing multi search (coming
                // through cpix_IdxDb API object) or when qualified
                // with volume id even if multi search is allowed
                // (coming through cpix_IdxSearcher API object)

                string
                    qbac(domainSelector);

                map<QualBaseAppClass, PathAndHndl>::iterator
                    qbacIt = qbacs_.find(qbac);

                if (qbacIt == qbacs_.end())
                    {
                        THROW_CPIXEXC("Unregistered qual base app class '%s'",
                                      domainSelector);
                    }
                else
                    {
                        PathAndHndl
                            pathAndHndl = qbacIt->second;
                        rv = pathAndHndl.second;

                        incIdxDbRefCount(rv);
                    }
            }

        return rv;
    }

    
    Cpix::IIdxDb * IdxDbMgr::get(IdxDbHndl handle)
    {
        Cpt::SyncRegion
            sr(mutex_);

        checkHndlSanity(handle,
                        false); // TODO is the 2nd parameter to checkHndlSanity needed?

        std::vector<IIdxDbInfo>
            & idxDbs(isMultiIdxDbHndl(handle) ? multiIdxDbs_ : idxDbs_);
        handle &= ~HighestBit<IdxDbHndl>::Value;

        if (idxDbs[handle].ptr() == NULL)
            {
                if (idxDbs[handle].refCount() == 0)
                    {
                        THROW_CPIXEXC("Accessing closed index.");
                    }
                else
                    {
                        // TODO CHECK : what guarantees that idxDbs is
                        // idxDbs_ and not multiIdxDbs_ ? That
                        // MultiIdxDb instances are destroyed
                        // (null-ed) only when they are completely
                        // released?

                        if (&idxDbs != &idxDbs_)
                            {
                                THROW_CPIXEXC(PL_ERROR "NULL but non-zero refcount multiidxdb handle");
                            }
                        
                        createIdxDbInstance(handle,
                                            false); // don't recreate
                    }
            }
                
        return idxDbs[handle].getPtrForUse();
    }

        
    void IdxDbMgr::incRefHndl(IdxDbHndl handle)
    {
        Cpt::SyncRegion
            sr(mutex_);

        checkHndlSanity(handle,
                        false);

        std::vector<IIdxDbInfo>
            & idxDbs(isMultiIdxDbHndl(handle) ? multiIdxDbs_ : idxDbs_);
        handle &= ~HighestBit<IdxDbHndl>::Value;

        idxDbs[handle].incRefCount();
    }


    void IdxDbMgr::releaseHndl(IdxDbHndl handle)
    {
        Cpt::SyncRegion
            sr(mutex_);

        checkHndlSanity(handle,
                        false);

        std::vector<IIdxDbInfo>
            & idxDbs(isMultiIdxDbHndl(handle) ? multiIdxDbs_ : idxDbs_);
        handle &= ~HighestBit<IdxDbHndl>::Value;

        idxDbs[handle].decRefCount();
    }


    void IdxDbMgr::defineVolume(const char * qualBaseAppClass,
                                const char * indexDbPath)
    {
        if (!isQualBaseAppClass(qualBaseAppClass))
            {
                THROW_CPIXEXC("Not a qualified base app class '%s'",
                              qualBaseAppClass);
            }

        string
            qbac(qualBaseAppClass),
            idp;

        Cpt::SyncRegion
            sr(mutex_);

        if (indexDbPath == NULL || *indexDbPath == 0)
            {
                idp = getAutomaticIdxDbPath(qualBaseAppClass);
            }
        else
            {
                idp = indexDbPath;
            }

        map<QualBaseAppClass, PathAndHndl>::iterator
            qbacIt = qbacs_.find(qbac);
        if (qbacIt != qbacs_.end())
            {
                if (qbacIt->second.first == idp)
                    {
                        // already defined with same path - nothing to do
                        return;
                    }
                else
                    {
                        THROW_CPIXEXC("Volume (qbac %s) already defined with different path",
                                      qbac.c_str());
                    }
            }

        sanityCheck(qbac,
                    idp,
                    NULL); // throw if problem found

        InitFileSystem(idp.c_str(),
                       false);
        qbacIt = qbacs_.find(qbac);

        if (qbacIt == qbacs_.end())
            {
                idxDbs_.push_back(IIdxDbInfo());
                size_t
                    newHandle = idxDbs_.size() - 1;
                qbacs_[qbac] = PathAndHndl(idp.c_str(),
                                           newHandle);
                storeReg();

                std::vector<IIdxDbInfo>::iterator i, end;
                    if (!(multiIdxDbs_.empty())) {
	                    i = multiIdxDbs_.begin();
	                    end = multiIdxDbs_.end();
		                for (; i!= end; ++i)
		                    {
		                        if (i->ptr() != NULL)
		                            {
		                                MultiIdxDb
		                                    * m = dynamic_cast<MultiIdxDb*>(i->ptr());
		                                if ( m )
		                                m->suggestHndl(newHandle,
		                                               qualBaseAppClass);
		                            }
		                    }
					}
            }
    }


    void IdxDbMgr::undefineVolume(const char * qualBaseAppClass)
    {
        using namespace std;
        using namespace Cpix;

        IdxDbHndl
            hndl = 0;
        string
            qbac(qualBaseAppClass);
        IIdxDb
            * toDestroy = NULL;
        
        { // SYNC
            Cpt::SyncRegion
                sr(mutex_);

            map<QualBaseAppClass, PathAndHndl>::iterator
                qbacIt = qbacs_.find(qbac);

            // it's okay if the base app class does not exist
            if (qbacIt != qbacs_.end())
                {
                    PathAndHndl
                        pathAndHndl = qbacIt->second;

                    // Any previous instance is to be thrown away
                    hndl = pathAndHndl.second;

                    qbacs_.erase(qbacIt);

                    // At this point, the handle cannot be looked up
                    // by its qbac. Also, if the ptr associated with
                    // the handle is NULL, it cannot be
                    // re-instantiated, because qbac/path information
                    // has been deleted

                    storeReg();

                    vector<IIdxDbInfo>::iterator i, end;
                    if (!(multiIdxDbs_.empty())) {
                        i = multiIdxDbs_.begin(); //coverty 121612 121611
                        end = multiIdxDbs_.end();
                    for (; i != end; ++i)
                        {
                            if (i->ptr() != NULL)
                                {
                                    MultiIdxDb
                                        * m = dynamic_cast<MultiIdxDb*>(i->ptr());
                                    if ( m ){
                                    bool
                                        used = m->removeHndl(hndl);

                                    if (used)
                                        {
                                            idxDbs_[hndl].decRefCount();
                                        }
                                    }
                                }
                        }
                    }

                    // At this point, all multiidx-es have lost their
                    // reference to the index in question
                    //
                    // At worst, there may be some search / index
                    // operations still ongoing using the current
                    // IIdxDb instance
                    //
                    // We have to store the potential pointer value
                    // because now IdxDbMgr / IIdxDbInfo structures do
                    // not own it anymore
                    toDestroy = idxDbs_[hndl].release();
                }
        } // SYNC


        if (toDestroy != NULL)
            {
                if (idxDbs_[hndl].refCount() == 0)
                    {
                        delete toDestroy;
                    }
                else
                    {
                        // we can't delete it now, there may be some active
                        // jobs using it
                        scheduleForDeletion(toDestroy);
                    }
            }
    }



    void IdxDbMgr::doHousekeepingOnAll()
    {
        using namespace std;

        long
            now = Cpt::GetTimeOfDay();

        vector<IIdxDbInfo>::size_type
            count;

        { // SYNC
            Cpt::SyncRegion
                sr(mutex_);
            count = idxDbs_.size();
        } // SYNC

        // We don't want the mutex held while we are calling
        // restIfIdle, as they may result in file i/o. We just check
        // the size of idxDbs_, and that gives us the possible handle
        // range: [0, size-of-idxDbs).
        //
        // Some of these handles are invalid (undefined), that's ok.
        //
        // Note, that this here should not overindex the vector as
        // long as there is no operation where the idxDbs_ vector is
        // decreased. Currently, there is no such operation, except
        // the dbg-scrapAll(), which is invoked only from debug code
        // and only when all handles have been released. So once
        // idxDbs_ reaches a size, it can't get smaller.

        for (vector<IIdxDbInfo>::size_type handle = 0; 
             handle < count; 
             ++handle)
            {
                idxDbs_[handle].restIfIdle(now,
                                           maxIdleSec_);
            }

        deleteSomeScheduledIdxDbs();

        ++housekeepCounter_;
    }



    IdxDbMgr::IdxDbMgr(InitParams & ip)
        : cpixDir_(ip.getCpixDir()),
          regFilePath_(ip.getCpixDir()),
          maxIdleSec_(ip.getMaxIdleSec()),
          mutex_(true), // recursive
          version_(0),
          initParams_(ip),
          housekeepCounter_(0)
    {
        regFilePath_ += "cpixreg.txt";

        // TODO this should be removed at some point - now it is only
        // here to have the indexes and the cpixreg.txt file in the
        // legacy locations even with this new logic
        cpixDir_ += "indexing\\indexdb";

        loadReg();
    }


    
    void destroyIdxDbs(std::vector<IIdxDbInfo> & idxDbs)
    {
        using namespace std;

        vector<Cpix::IIdxDbInfo>::iterator
            i = idxDbs.begin(),
            end = idxDbs.end();

        for (; i != end; ++i)
            {
                if (i->ptr() != NULL)
                    {
                        Impl::cl_auto_ptr<IIdxDb>
                            tmp(i->ptr());

                        // close()-ing and destroying the IIdxDb
                        // instance is implied by cl_auto_ptr dtor
                    }
            }

        idxDbs.clear();
    }
    

    IdxDbMgr::~IdxDbMgr()
    {
        destroyIdxDbs(multiIdxDbs_);
        destroyIdxDbs(idxDbs_);
        deleteAllScheduledIdxDbs();
    }
        
        
    void IdxDbMgr::sanityCheck(const std::string & baseAppClass,
                               const std::string & idxDbPath,
                               bool              * succeeded)
    {
        using namespace std;

        string
            msg;

        map<QualBaseAppClass, PathAndHndl>::const_iterator
            i = qbacs_.begin(),
            end = qbacs_.end();

        for (; i != end; ++i)
            {
                if (i->second.first == idxDbPath
                    && i->first != baseAppClass)
                    {
                        // (1) We don't allow multiple
                        // baseAppClass-es pointing to the same
                        // physical path
                        msg = "Both ";
                        msg += i->first;
                        msg += " and ";
                        msg += baseAppClass;
                        msg += " point to the same phyisical idx db path ";
                        msg += idxDbPath;
                        break;
                    }
                else if (i->first == baseAppClass
                         && i->second.first != idxDbPath)
                    {
                        // (2) We don't allow the same
                        // baseAppClass to have multiple physical
                        // paths associated.
                        msg = "A base app class ";
                        msg += baseAppClass;
                        msg += " cannot have multiple index db paths.";
                        break;
                    }
            }

        if (succeeded == NULL)
            {
                if (msg.length() > 0)
                    {
                        THROW_CPIXEXC(msg.c_str());
                    }
            }
        else
            {
                if (msg.length() > 0)
                    {
                        *succeeded = false;
                        logMsg(CPIX_LL_ERROR,
                               msg.c_str());
                    }
                else
                    {
                        *succeeded = true;
                    }
            }
    }


    void IdxDbMgr::InitFileSystem(const char * path,
                                  bool         recreate)
    {
        // making sure that the directory exists
        if (!Cpt::directoryexists(path))
            {
                int
                    result = Cpt::mkdirs(path, 0777);
                if (result == -1)
                    {
                        THROW_CPIXEXC("Could not create folder(s): '%s'",
                                      path);
                    }

            }

        // if only opening, ...
        if (!recreate)
            {
                try
                    {
                        // ... then we just try to open the physical
                        // index ...
                        OpenFsIdx(path);
                    }
                catch (LuceneError & clErr)
                    {
                        // ... and if it failed, we must re-create it
                        recreate = true;
                    }
            }

        // if creating or re-creating
        if (recreate)
            {
                RecreateFsCpixIdx(path);
            }
    }


    void IdxDbMgr::createIdxDbInstance(IdxDbHndl    handle,
                                       const char * path,
                                       bool         recreate,
                                       const char * qualBaseAppClass)
    {
        try
            {
                if (idxDbs_[handle].ptr() == NULL)
                    {
                        // there is no instance in use, create one

                        InitFileSystem(path,
                                       recreate);

                        IIdxDb
                            * idx = new IdxDb(path,
                                              initParams_);
                        idxDbs_[handle].setPtr(idx);

                    }
                else
                    {
                        // there is an instance already, we can't
                        // destroy/recreate it without the danger of
                        // crashing another thread still working on
                        // that object
                        if (recreate)
                            {
                                idxDbs_[handle].ptr()->recreateIdx();
                            }
                        else
                            {
                                ; // nothing to do - we already have
                                  // the instance and it's index does
                                  // not need to be reset
                            }
                    }
            }
        catch (...)
            {
                logMsg(CPIX_LL_WARNING,
                       "Unable to load index for %s from %s, deleting from CPix registry.",
                       qualBaseAppClass,
                       path);

                // getting rid of stale stuff
                qbacs_.erase(qualBaseAppClass);
                storeReg();

                throw;
            }
    }


    void IdxDbMgr::createIdxDbInstance(IdxDbHndl    handle,
                                       bool         recreate)
    {
        using namespace std;
        
        map<QualBaseAppClass, PathAndHndl>::iterator
            i = qbacs_.begin(),
            end = qbacs_.end();
        for (; i != end && i->second.second != handle; ++i)
            ;
        
        if (i == end)
            {
                THROW_CPIXEXC(PL_ERROR "Handle %d does not correspond to existing qbac",
                              handle);
            }
        
        const char
            * qbac = i->first.c_str(),
            * path = i->second.first.c_str();
        
        createIdxDbInstance(handle,
                            path,
                            recreate,
                            qbac);
    }



    void IdxDbMgr::loadReg()
    {
        using namespace std;

        bool
            succeeded = false;
            
        // format: <base application type>=<index database path>
        // e.g. "root file usrdoc text=c:/this/and/that/path"
        ifstream
            reg(regFilePath_.c_str());
        
        if (reg.is_open())
            {
                string line; 

                succeeded = true;
                while (getline(reg, line))
                    {
                        size_t
                            delimiterPos = line.find_first_of(delimiter_);
                        if (delimiterPos == line.npos)
                            {
                                logMsg(CPIX_LL_ERROR,
                                       "Garbled idx db config");
                                succeeded = false;
                                break;
                            }

                        string
                            baseAppClass(line.substr(0,
                                                     delimiterPos)),
                            idxDbPath(line.substr(delimiterPos + 1));

                        sanityCheck(baseAppClass,
                                    idxDbPath,
                                    &succeeded); // warn, and report back only
                        if (!succeeded)
                            {
                                break;
                            }

                        idxDbs_.push_back(IIdxDbInfo());

                        PathAndHndl
                            pathAndHndl(idxDbPath,
                                        idxDbs_.size() - 1);
                            
                        qbacs_[baseAppClass] = pathAndHndl;
                    }
                reg.close(); 
            }

        if (!succeeded)
            {
                logMsg(CPIX_LL_WARNING,
                       "Could not read CPix registry %s, (re-)creating it.\n",
                       regFilePath_.c_str());

                remove(regFilePath_.c_str());

                // this call is made here only to check that we can
                // create the file, otherwise we should fail at this
                // point
                storeReg();
            }
    }
        

    void IdxDbMgr::storeReg()
    {
        using namespace std;

        ofstream
            reg(regFilePath_.c_str());

        if (reg.is_open())
            {
                map<string,PathAndHndl>::iterator
                    i = qbacs_.begin(),
                    end = qbacs_.end();

                for (; i != end; ++i)
                    {
                        const string
                            & baseAppClass = i->first;
                        const string
                            & idxDbPath = i->second.first;

                        reg 
                            << baseAppClass
                            << delimiter_
                            << idxDbPath
                            << endl;
                    }
            }
        else
            {
                THROW_CPIXEXC("Cannot open idx db config file '%s'",
                              regFilePath_.c_str());
            }
    }


    std::string IdxDbMgr::getAutomaticIdxDbPath(const char * qualBaseAppClass)
    {
        std::string
            rv(cpixDir_);

        const char
            * ptr = getBaseAppClassPart(qualBaseAppClass);

        while (ptr != NULL && *ptr != 0)
            {
                const char
                    * end = strchr(ptr,
                                   APPHIERARCHY_DELIMITER);
                size_t
                    wordSize = (end == NULL) ? strlen(ptr) : end - ptr;

                rv += '\\';
                rv += std::string(ptr,
                                  wordSize);

                ptr = end;
                if (ptr != NULL)
                    {
                        ++ptr;
                    }
            }

        return rv;
    }



    const char * IdxDbMgr::getBaseAppClassPart(const char * qualBaseAppClass)
    {
        const char
            * delimiter = strchr(qualBaseAppClass,
                                 VOLID_DELIMITER);
        if (delimiter == NULL
            || (delimiter - qualBaseAppClass) == strlen(qualBaseAppClass))
            {
                THROW_CPIXEXC("Garbled qualified base app class '%s'",
                              qualBaseAppClass);
            }
        
        return delimiter + 1;
    }



    void IdxDbMgr::incIdxDbRefCount(IdxDbHndl    handle)
    {
        idxDbs_[handle].incRefCount();
    }


    IdxDbHndl IdxDbMgr::createMultiIdxDbHndl(const char * domainSelector)
    {
        using namespace std;

        IdxDbHndl
            rv = 0;

        vector<IIdxDbInfo>::iterator
            firstEmpty = find_if(multiIdxDbs_.begin(),
                                 multiIdxDbs_.end(),
                                 compose1(bind2nd(equal_to<IIdxDb*>(),
                                                  NULL),
                                          mem_fun_ref(&IIdxDbInfo::ptr)));

        if (firstEmpty == multiIdxDbs_.end())
            {
                firstEmpty = multiIdxDbs_.insert(multiIdxDbs_.end(),
                                                 IIdxDbInfo());
            }

        rv = firstEmpty - multiIdxDbs_.begin();

        set<IdxDbHndl>
            matchedHandles;

        map<QualBaseAppClass, PathAndHndl>::iterator
            i = qbacs_.begin(),
            end = qbacs_.end();

        for (; i != end; ++i)
            {
                if (match(domainSelector, i->first.c_str()))
                    {
                        matchedHandles.insert(i->second.second);
                        incIdxDbRefCount(i->second.second);
                    }
            }

        MultiIdxDb 
            * multi = new MultiIdxDb(matchedHandles,
                                     domainSelector);

        *firstEmpty = IIdxDbInfo();
        firstEmpty->setPtr(multi);
        firstEmpty->incRefCount();

        // this is how we mark that this handle is a multi idx db handle
        rv |= (IdxDbHndl)HighestBit<IdxDbHndl>::Value;

        return rv;
    }


    bool IdxDbMgr::isMultiIdxDbHndl(IdxDbHndl handle)
    {
        const IdxDbHndl
            mask = (IdxDbHndl)HighestBit<IdxDbHndl>::Value;

        return (handle & mask) != 0;
    }


    void IdxDbMgr::checkHndlSanity(IdxDbHndl handle,
                                   bool      allowUndefinedIdxDb)
    {
        bool
            isMulti = isMultiIdxDbHndl(handle); 

        std::vector<IIdxDbInfo>
            & idxDbs(isMulti ? multiIdxDbs_ : idxDbs_);
        handle &= ~HighestBit<IdxDbHndl>::Value;

        if (handle > idxDbs.size())
            {
                THROW_CPIXEXC(PL_ERROR "Invalid idxdb handle %d",
                              handle);
            }
            
        if (idxDbs[handle].ptr() == NULL)
            {
                if (isMulti)
                    {
                        THROW_CPIXEXC(PL_ERROR "MultiIdxDb instance is NULL for handle '%d'",
                                      handle);
                    }
                else if (!allowUndefinedIdxDb)
                    {
                        /* TODO check if this should be allowed to throw
                           THROW_CPIXEXC(PL_ERROR "IdxDb instance is NULL for handle '%d'",
                           handle);
                        */
                    }
            }
    }


    void IdxDbMgr::dbgDumpState()
    {
        logTestMsg(CPIX_LL_TRACE,
                   "DUMPING IdxDbMgr singleton instance: BEGIN");

        logTestMsg(CPIX_LL_TRACE,
                   "  o cpixDir_: %s",
                   cpixDir_.c_str());
        logTestMsg(CPIX_LL_TRACE,
                   "  o regFilePath_: %s",
                   regFilePath_.c_str());

        Cpt::SyncRegion
            sr(mutex_);
        
        {
            logTestMsg(CPIX_LL_TRACE,
                       "  o qbacs_:");
            std::map<QualBaseAppClass,PathAndHndl>::iterator
                i = qbacs_.begin(),
                end = qbacs_.end();
            for (; i != end; ++i)
                {
                    logTestMsg(CPIX_LL_TRACE,
                               "    o  entry: [%s] -> (%s, %d)",
                               i->first.c_str(),
                               i->second.first.c_str(),
                               i->second.second);
                }
        }

        {
            std::vector<IIdxDbInfo>::iterator
                i = idxDbs_.begin(),
                end = idxDbs_.end();
            size_t
                idx = 0;
            logTestMsg(CPIX_LL_TRACE,
                       "  o idxDb_:");
            for (; i != end; ++i)
                {
                    i->dbgDumpState(idx);
                    ++idx;
                }
            
            i = multiIdxDbs_.begin(),
                end = multiIdxDbs_.end();
            idx = 0;
            logTestMsg(CPIX_LL_TRACE,
                       "  o multiIdxDb_:");
            for (; i != end; ++i)
                {
                    i->dbgDumpState(idx);
                    ++idx;
                }
        }

        logTestMsg(CPIX_LL_TRACE,
                   "DUMPING IdxDbMgr singleton instance: END.",
                   reinterpret_cast<long>(this));
    }



    size_t IdxDbMgr::getClHitsPageSize() const
    {
        return initParams_.getClHitsPageSize();
    }


    void IdxDbMgr::RecreateFsClIdx(const char * cluceneDir)
    {
        using namespace lucene::index;

        Impl::cl_auto_ptr<IndexWriter>
            tmpIdxWriter(new IndexWriter(cluceneDir,
                                         NULL, // &dummyAnalyzer,
                                         true, // create
                                         true));

        // close()-ing tmpIdxWriter is implied by cl_auto_ptr dtor
    }


    void IdxDbMgr::OpenFsIdx(const char * cpixDir)
    {
        using namespace lucene::index;
        using namespace lucene::analysis;

        Cpix::Impl::cl_auto_ptr<IndexReader>
            dummyReader(NULL);

        Cpix::Impl::CpixPaths
            paths(cpixDir);

        dummyReader.reset(IndexReader::open(paths.curDir_->c_str()));

        // close()-ing dummyReader is implied by cl_auto_ptr dtor
    }

    
    void IdxDbMgr::RecreateFsCpixIdx(const char * cpixDir)
    {
        Cpt::removeunder(cpixDir);

        using namespace lucene::index;
        using namespace lucene::analysis;

        const char
            * cluceneDir = cpixDir;

        // old machinery: cpix dir and clucene dir ARE the same
        // new machinery: cpix dir and clucene dir are NOT the same thing
        Cpix::Impl::CpixPaths
            paths(cpixDir);

        cluceneDir = paths.curDir_->c_str();

        Cpix::IdxDbMgr::RecreateFsClIdx(cluceneDir);
    }


    void IdxDbMgr::scheduleForDeletion(IIdxDb * toDestroy)
    {
        Cpt::SyncRegion
            sr(mutex_);

        idxDbsToDestroy_.push_back(IIdxDbToDestroy(toDestroy,
                                                   housekeepCounter_));
    }


    void IdxDbMgr::deleteAllScheduledIdxDbs()
    {
        Cpt::SyncRegion
            sr(mutex_);

        std::list<IIdxDbToDestroy>::iterator
            i = idxDbsToDestroy_.begin(),
            end = idxDbsToDestroy_.end();

        for (; i != end; ++i)
            {
                delete i->ptr_;
            }
    }
    
    
    void IdxDbMgr::deleteSomeScheduledIdxDbs()
    {
        Cpt::SyncRegion
            sr(mutex_);

        while (!idxDbsToDestroy_.empty())
            {
                IIdxDbToDestroy
                    tmp = idxDbsToDestroy_.front();

                if (tmp.housekeepCounter_ + DISCARD_SAFETY_PERIOD 
                    < housekeepCounter_)
                    {
                        // after tmp.ptr_ has been scheduled for
                        // destruction by undefineVolume, we have
                        // completed DISCARD_SAFETY_PERIOD number of
                        // housekeep rounds, so we can be confident
                        // that no active jobs use tmp.ptr_ anymore

                        tmp.ptr_->brutalClose();
                        delete tmp.ptr_;
                        idxDbsToDestroy_.pop_front();
                    }
                else
                    {
                        // idxDbsToDestroy_ is used as a queue (added
                        // to at the end, taken from at the front),
                        // therefore if a scheduled pointer is not
                        // ready for desstruction, the ones added
                        // later won't be ready either at this point

                        break;
                    }
            }
    }
    


    //
    // This is for single threaded use only
    //
    IdxDbMgr *  IdxDbMgr::instance_  = NULL;



    const char  IdxDbMgr::delimiter_ = '=';


}

