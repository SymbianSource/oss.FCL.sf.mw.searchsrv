/*------------------------------------------------------------------------------
* Copyright (C) 2003-2006 Ben van Klinken and the CLucene Team
* 
* Distributable under the terms of either the Apache License (Version 2.0) or 
* the GNU Lesser General Public License, as specified in the COPYING file.
------------------------------------------------------------------------------*/
#ifndef _lucene_util_ThreadLocal_H
#define _lucene_util_ThreadLocal_H

#if defined(_LUCENE_PRAGMA_ONCE)
# pragma once
#endif

#include "CLucene/util/VoidMap.h"
#include "CLucene/LuceneThreads.h" // Needed for mutexes

CL_NS_DEF(util)

class ThreadLocalBase: LUCENE_BASE{
public:
	/** 
	* A hook called when CLucene is starting or shutting down, 
	* this can be used for setting up and tearing down static
	* variables
	*/
	typedef void ShutdownHook(bool startup);

protected:
	typedef CL_NS(util)::CLMultiMap<_LUCENE_THREADID_TYPE, ThreadLocalBase*, 
			CL_NS(util)::LuceneThreadIdCompare, 
			CL_NS(util)::Deletor::ConstNullVal<_LUCENE_THREADID_TYPE>, 
			CL_NS(util)::Deletor::ConstNullVal<ThreadLocalBase*> > ThreadLocalsType;
	typedef CL_NS(util)::CLSetList<ShutdownHook*, 
			CL_NS(util)::Compare::Void<ShutdownHook>, 
			CL_NS(util)::Deletor::ConstNullVal<ShutdownHook*> > ShutdownHooksType;
	
#if defined (__SYMBIAN32__) && defined (RVCT22_STATICS_WORKAROUND)

// CPIXASYNC
        STATIC_DEFINE_CRUCIAL_MUTEX1(ThreadLocalBase_THIS_LOCK)
#define ThreadLocalBase_THIS_LOCK GetInstancemutex_pthreadThreadLocalBase_THIS_LOCK()
        
	DECL_STATMVAR1(ThreadLocalsType,threadLocals,false,false)
#define threadLocals GetInstanceThreadLocalsTypethreadLocals()
	
	DECL_STATMVAR1(ShutdownHooksType,shutdownHooks,false)
#define shutdownHooks GetInstanceShutdownHooksTypeshutdownHooks()
	
#else
	// CPIXASYNC STATIC_DEFINE_MUTEX(ThreadLocalBase_THIS_LOCK)
        STATIC_DEFINE_CRUCIAL_MUTEX(ThreadLocalBase_THIS_LOCK)

	static ThreadLocalsType threadLocals;
	//todo: using http://en.wikipedia.org/wiki/Thread-local_storage#Pthreads_implementation
	//would work better... but lots of testing would be needed first...
	static ShutdownHooksType shutdownHooks;
#endif

	ThreadLocalBase();
public:
	virtual ~ThreadLocalBase();

	/**
	* Call this function to clear the local thread data for this
	* ThreadLocal. Calling set(NULL) does the same thing, except
	* this function is virtual and can be called without knowing
	* the template.
	*/
	virtual void setNull() = 0;

	/**
	* If you want to clean up thread specific memory, then you should
	* make sure this thread is called when the thread is not going to be used
	* again. This will clean up threadlocal data which can contain quite a lot
	* of data, so if you are creating lots of new threads, then it is a good idea
	* to use this function, otherwise there will be many memory leaks.
	*/
	static void UnregisterCurrentThread();

	/**
	* Call this function to shutdown CLucene
	*/
	static void shutdown();

	/**
	* Add this function to the shutdown hook list. This function will be called
	* when CLucene is shutdown.
	*/
	static void registerShutdownHook(ShutdownHook* hook);
};

template<typename T,typename _deletor>
class ThreadLocal: public ThreadLocalBase{
	typedef CL_NS(util)::CLSet<_LUCENE_THREADID_TYPE, T, 
			CL_NS(util)::LuceneThreadIdCompare, 
			CL_NS(util)::Deletor::ConstNullVal<_LUCENE_THREADID_TYPE>, 
			_deletor > LocalsType;
	LocalsType locals;
//  DEFINE_MUTEX(locals_LOCK)	
	DEFINE_CRUCIAL_MUTEX(locals_LOCK) // marked as crucial
public:
	ThreadLocal();
	~ThreadLocal();
	T get();
	void setNull();
	void set(T t);
};

template<typename T,typename _deletor>
ThreadLocal<T,_deletor>::ThreadLocal():
	locals(false,true)
{
	//add this object to the base's list of threadlocals to be
	//notified in case of UnregisterThread()
/*  OBS: Obsoleted as plainly harmful pieces of code. 
 *       Main effect of registration seems to be preventing 
 *       non-constructor threads destructive this object
    _LUCENE_THREADID_TYPE id = _LUCENE_CURRTHREADID;
	SCOPED_LOCK_MUTEX(ThreadLocalBase_THIS_LOCK)
	threadLocals.insert( pair<const _LUCENE_THREADID_TYPE, ThreadLocalBase*>(id, this) );*/
}

template<typename T,typename _deletor>
ThreadLocal<T,_deletor>::~ThreadLocal(){
	//remove this object to the base's list of threadlocals

/*  OBS: Obsoleted as plainly harmful pieces of code. 
 *       Main effect of registration seems to be preventing 
 *       non-constructor threads destructive this object
//	_LUCENE_THREADID_TYPE id = _LUCENE_CURRTHREADID;
//	SCOPED_LOCK_MUTEX(ThreadLocalBase_THIS_LOCK)*/
	
// NOTE: We are not using mutexes to guard the access to locals-object, 
//       because if there was another thread accessing this object right
// 		 now then much greater lifetime problem would emerge. 
	//remove all the thread local data for this object
	locals.clear();

//  OBS: Plain harmful code
/*	//remove this object from the ThreadLocalBase threadLocal list
	ThreadLocalsType::iterator itr = threadLocals.lower_bound(id);
	ThreadLocalsType::iterator end = threadLocals.upper_bound(id);
	while ( itr != end ){
		if ( itr->second == this){
			threadLocals.erase(itr);
			break;
		}
		++itr;
	}*/
}

template<typename T,typename _deletor>
T ThreadLocal<T,_deletor>::get(){
	SCOPED_LOCK_CRUCIAL_MUTEX(locals_LOCK)

	return locals.get(_LUCENE_CURRTHREADID);
}

template<typename T,typename _deletor>
void ThreadLocal<T,_deletor>::setNull(){
	SCOPED_LOCK_CRUCIAL_MUTEX(locals_LOCK)

	set(NULL);
}

template<typename T,typename _deletor>
void ThreadLocal<T,_deletor>::set(T t){
	SCOPED_LOCK_CRUCIAL_MUTEX(locals_LOCK)

	_LUCENE_THREADID_TYPE id = _LUCENE_CURRTHREADID;
	locals.remove(id);
	if ( t != NULL )
		locals.insert( pair<_LUCENE_THREADID_TYPE,T>(id, t) );
}

CL_NS_END
#endif
