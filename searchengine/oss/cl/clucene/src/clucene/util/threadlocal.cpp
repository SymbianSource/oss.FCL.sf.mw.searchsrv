/*------------------------------------------------------------------------------
* Copyright (C) 2003-2006 Ben van Klinken and the CLucene Team
* 
* Distributable under the terms of either the Apache License (Version 2.0) or 
* the GNU Lesser General Public License, as specified in the COPYING file.
------------------------------------------------------------------------------*/
#include "clucene/stdheader.h"
#include "CLucene/LuceneThreads.h"
#include "ThreadLocal.h"

CL_NS_DEF(util)


#if defined (__SYMBIAN32__) && defined (RVCT22_STATICS_WORKAROUND)
// CPIXASYNC
STATIC_DEFINE_CRUCIAL_MUTEX2(ThreadLocalBase,ThreadLocalBase_THIS_LOCK)
DECL_STATMVAR2(ThreadLocalBase::ShutdownHooksType,ThreadLocalBase,shutdownHooks)
DECL_STATMVAR2(ThreadLocalBase::ThreadLocalsType,ThreadLocalBase,threadLocals)
#else
// CPIXASYNC DEFINE_MUTEX(ThreadLocalBase::ThreadLocalBase_THIS_LOCK)
DEFINE_CRUCIAL_MUTEX(ThreadLocalBase::ThreadLocalBase_THIS_LOCK)
ThreadLocalBase::ShutdownHooksType ThreadLocalBase::shutdownHooks(false);
ThreadLocalBase::ThreadLocalsType ThreadLocalBase::threadLocals(false,false);
#endif

ThreadLocalBase::ThreadLocalBase(){
}
ThreadLocalBase::~ThreadLocalBase(){
}

void ThreadLocalBase::UnregisterCurrentThread(){
	_LUCENE_THREADID_TYPE id = _LUCENE_CURRTHREADID;
	// CPIXASYNC SCOPED_LOCK_MUTEX(ThreadLocalBase_THIS_LOCK)
        SCOPED_LOCK_CRUCIAL_MUTEX(ThreadLocalBase_THIS_LOCK)
	
	ThreadLocalsType::iterator itr = threadLocals.lower_bound(id);
	ThreadLocalsType::iterator end = threadLocals.upper_bound(id);
	while ( itr != end ){
		itr->second->setNull();
		++itr;
	}
}
void ThreadLocalBase::shutdown(){
    { // scope to make the mutex unlocked before its destroyed
        // CPIXASYNC SCOPED_LOCK_MUTEX(ThreadLocalBase_THIS_LOCK)
        SCOPED_LOCK_CRUCIAL_MUTEX(ThreadLocalBase_THIS_LOCK)
            
            ThreadLocalsType::iterator itr = threadLocals.begin();
	while ( itr != threadLocals.end() ){
            itr->second->setNull();
            ++itr;
	}
        
	ShutdownHooksType::iterator itr2 = shutdownHooks.begin();
	while ( itr2 != shutdownHooks.end() ){
            ShutdownHook* hook = *itr2;
            hook(false);
	}
        
    }

#if defined (__SYMBIAN32__) && defined (RVCT22_STATICS_WORKAROUND)
#if !defined (_CL_DISABLE_MULTITHREADING) || defined (CPIX_ASYNC_MT)
        // this thread holds this lock, so it should be able to
        // destroy it
        StaticInstances::shutdown(&CL_NS(util)::ThreadLocalBase::ThreadLocalBase_THIS_LOCK);
#endif
#endif
}

void ThreadLocalBase::registerShutdownHook(ShutdownHook* hook){
    //CPIXASYNC SCOPED_LOCK_MUTEX(ThreadLocalBase_THIS_LOCK)
    SCOPED_LOCK_CRUCIAL_MUTEX(ThreadLocalBase_THIS_LOCK)
	shutdownHooks.insert(hook);
}


CL_NS_END
