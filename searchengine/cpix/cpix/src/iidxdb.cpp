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


#include "iidxdb.h"
#include "idxdbmgr.h"
#include "idxdb.h"

#include "common/cpixlog.h"

#include "iqrytype.h" // 'unnecessary' dependency for static instance releasing
#include "ifieldfilter.h" // for static instance releasing

namespace Cpix {


    /********************
     * From file cpix_async.cpp - asynchronous engine starting and stopping.
     */
    void InitAsyncEngine(Cpix::InitParams & ip);
    void ShutdownAsyncEngine();

    /********************
     * From file cpixerror.cpp - error infor lifetime mgmt
     */
    void InitErrorInfoPool();
    void ShutdownErrorInfoPool(); 
}

namespace Cpix
{


    IIdxDb::~IIdxDb()
    {
        ;
    }


    IIdxDb * IIdxDb::getPtr(IdxDbHndl handle)
    {
	IIdxDb
            * rv = IdxDbMgr::instance()->get(handle);

        return rv;
    }



    void IIdxDb::release(IdxDbHndl handle)
    {
        IdxDbMgr::instance()->releaseHndl(handle);
    }


    void IIdxDb::defineVolume(const char * qualBaseAppClass,
                              const char * path)
    {
        IdxDbMgr::instance()->defineVolume(qualBaseAppClass,
                                           path);
    }


    void IIdxDb::undefineVolume(const char * qualBaseAppClass)
    {
        IdxDbMgr::instance()->undefineVolume(qualBaseAppClass);
    }


}



namespace
{
    /**
     * Protects a compoment of Cpix that involves global / static
     * member instances that need initialization at library init
     * time. Since there are many such components, any of which may
     * fail (throw), we need to make sure that the ones that have
     * already succeeded are rolled back.
     */
    class ShutdownSentry
    {
    private:
        //
        // private members
        //
        void         (*shutdown_)();

    public:
        //
        // public operators
        //
        
        /**
         * Constructs this sentry with a static function to call on
         * destruction unless succeeded() is called.
         */
        ShutdownSentry(void         (*shutdown)())
            : shutdown_(shutdown)
        {
            ;
        }

        
        /**
         * A certain sequence of event have succeeded, so we don't
         * need to call the shutdown function.
         */
        void succeeded()
        {
            shutdown_ = NULL;
        }


        /**
         * Destructs this shutdown sentry instnance, and invoke the
         * stored shutdown function in case the sequence of events
         * unsuccessful (exception thrown).
         */
        ~ShutdownSentry()
        {
            if (shutdown_ != NULL)
                {
                    (*shutdown_)();
                }
        }
        
    };
}


namespace Cpix
{

    void IIdxDb::init(InitParams * ip)
    {
        std::auto_ptr<InitParams>
            defaultIp;

        if (ip == NULL)
            {
                defaultIp.reset(new InitParams);
                ip = defaultIp.get();
            }

        GlobalLogLevel = CPIX_LL_ERROR;

        RotLogger::instance()->createLog(ip->getLogFileBase(),
                                         ip->getLogSizeLimit(),
                                         ip->getLogSizeCheckRecurrency());
        ShutdownSentry
            rl(&RotLogger::shutdownAll);
        
        try 
            {
                InitErrorInfoPool();
                ShutdownSentry
                    eip(&ShutdownErrorInfoPool);

                FieldFilterPool::init();
                ShutdownSentry
                    ffp(&FieldFilterPool::cleanup);

                QryCall::init();
                ShutdownSentry
                    qc(&QryCall::releaseCache);

                cleanupClLockDir();

                // TODO init lucene (??? operation not provided)

                IdxDbMgr::init(*ip);
                ShutdownSentry
                    idm(&IdxDbMgr::shutdownAll);

                InitAsyncEngine(*ip);
                ShutdownSentry
                    ae(&ShutdownAsyncEngine);

                logMsg(CPIX_LL_TRACE,
                       "Cpix init: OK. ########\n");

                rl.succeeded();
                eip.succeeded();
                ffp.succeeded();
                qc.succeeded();
                idm.succeeded();
                ae.succeeded();
            } 
        catch (...)
            {
                logMsg(CPIX_LL_ERROR,
                       "Cpix init: FAILED. ########\n");
                throw;
            }
    }



    void IIdxDb::cleanupClLockDir()
    {
        int 
            result = Cpt::removeunder(LUCENE_LOCK_DIR_ENV_FALLBACK);

        if (result != 0)
            {
                logMsg(CPIX_LL_ERROR,
                       "Could not remove all files under CL LOCK DIR %s, some indexes may be unmodifiable\n",
                       LUCENE_LOCK_DIR_ENV_FALLBACK);
            }
    }


    void IIdxDb::rotateLog() throw()
    {
        RotLogger::instance()->rotateLog();
    }
    
    

    IdxDbHndl IIdxDb::getIdxDbHndl(const char  * domainSelector,
                                   bool          allowMultiSearch)
    {
        IdxDbHndl
            rv = IdxDbMgr::instance()->getHndl(domainSelector,
                                               allowMultiSearch);

        return rv;
    }



    IdxDbHndl IIdxDb::getIdxDbHndlCreating(const char * qualBaseAppClass)
    {
        IdxDbHndl
            rv = IdxDbMgr::instance()->create(qualBaseAppClass);

        return rv;
    }

    void IIdxDb::shutdownAll() throw()
    {
        try
            {
                ShutdownAsyncEngine();
            }
        catch (...)
            {
                logMsg(CPIX_LL_ERROR,
                   "Cpix shutdownAll: FAILED. ShutdownAsyncEngine ########\n");
            }

        try
            {
                IdxDbMgr::shutdownAll();
            }
        catch (...)
            {
                logMsg(CPIX_LL_ERROR,
                   "Cpix shutdownAll: FAILED. IdxDbMgr::shutdownAll ########\n");
            }

        try
            {
                _lucene_shutdown();
            }
        catch (...)
            {
                logMsg(CPIX_LL_ERROR,
                   "Cpix shutdownAll: FAILED. _lucene_shutdown ########\n");
            }

        try
            {
                QryCall::releaseCache(); 
            }
        catch (...)
            {
                logMsg(CPIX_LL_ERROR,
                   "Cpix shutdownAll: FAILED. QryCall::releaseCache ########\n");
            }

        try
            {
                FieldFilterPool::cleanup();
            }
        catch (...)
            {
                logMsg(CPIX_LL_ERROR,
                   "Cpix shutdownAll: FAILED. FieldFilterPool ########\n");
            }

        try
            {
                ShutdownErrorInfoPool(); 
            }
        catch (...)
            {
                logMsg(CPIX_LL_ERROR,
                   "Cpix shutdownAll: FAILED. ShutdownErrorInfoPool ########\n");
            }

        try
            {
                RotLogger::shutdownAll();
            }
        catch (...)
            {
                logMsg(CPIX_LL_ERROR,
                   "Cpix shutdownAll: FAILED. RotLogger::shutdownAll ########\n");
            }
    }



    void IIdxDb::scrapAll()
    {
        IdxDbMgr::scrapAll();
    }



    void IIdxDb::doHousekeepingOnAll()
    {
        IdxDbMgr::instance()->doHousekeepingOnAll();
    }

    

    void IIdxDb::dbgDumpAllState()
    {
        IdxDbMgr::instance()->dbgDumpState();
    }
    


} // namespace CPix 
