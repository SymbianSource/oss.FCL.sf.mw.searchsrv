/*------------------------------------------------------------------------------
* Copyright (C) 2003-2006 Ben van Klinken and the CLucene Team
* 
* Distributable under the terms of either the Apache License (Version 2.0) or 
* the GNU Lesser General Public License, as specified in the COPYING file.
------------------------------------------------------------------------------*/
#ifndef _lucene_debug_lucenebase_
#define _lucene_debug_lucenebase_

#ifdef _LUCENE_PRAGMA_ONCE
# pragma once
#endif

CL_NS_DEF(debug)

#if defined (__SYMBIAN32__) && defined (RVCT22_STATICS_WORKAROUND)
// Plain create is used to disable memory leak tracing etc. for individual objects
// Meant to be used to instantiate memory leak tracing infra. Using some other mean
// leads rather automatically to stack overflow

#ifdef LUCENE_ENABLE_MEMLEAKTRACKING
struct nomemtrace_t{};
extern const nomemtrace_t nomemtrace; 
#define _CLNEW_NOMEMTRACE new (CL_NS(debug)::nomemtrace)
#else // !LUCENE_ENABLE_MEMLEAKTRACKING
#define _CLNEW_NOMEMTRACE _CLNEW
#endif
#endif

//Lucenebase is the superclass of all clucene objects. It provides
//memory debugging tracking and/or reference counting
class LuceneBase{
public:
#ifdef LUCENE_ENABLE_MEMLEAKTRACKING
	static void* operator new (size_t size);

#if defined (__SYMBIAN32__) && defined (RVCT22_STATICS_WORKAROUND)
	static void* operator new (size_t size, const nomemtrace_t&);
	static void operator delete (void* p, const nomemtrace_t&);
#endif // RVCT22_STATICS_WORKAROUND
	
	static void operator delete (void *p);
	int32_t __cl_initnum; ///< The order that the object was created at. This is then used to do a lookup in the objects list

	static void* operator new (size_t size, char const * file, int32_t line);
	static void operator delete (void *p, char const * file, int32_t line);

	static void* __cl_voidpadd(void* data, const char* file, int line, size_t size); ///<add arbitary data to the lucenbase_list and returns the same data
	static void __cl_voidpremove(const void* data, const char* file, int line);///<remove arbitary data to the lucenbase_list
	static void __cl_unregister(const void* obj); ///<un register object from the mem leak and ref count system

	static int32_t __cl_GetUnclosedObjectsCount();  ///< gets the number of unclosed objects

    /* CPIXASYNC The function with this signature is obsolete, as they
       are not nice as they leak "const char *" objects stored by a
       global thing (LuceneBase_Pionters). Please use the version below.

	static const char* __cl_GetUnclosedObject(int32_t item);  ///< get the name of the nth unclosed object
    */
    /**
     * CPIXASYNC
     * A safer version of the old & obsolete __cl_GetUnclosedObject
     * version (signature const char (*)(int32_t). Instead of
     * returning the C string value of ht unclosed object, it copies
     * it to the buffer provided by the caller.
     */
    void __cl_GetUnclosedObject(int32_t item,
                                char *  dest,
                                size_t  destSize);


	static char* __cl_GetUnclosedObjects();  ///< get a string with the names of the unclosed objects
	static void __cl_PrintUnclosedObjects(); ///< print unclosed objects to the stdout
  
  	///This will clear memory relating to refcounting
	///other tools can be used to more accurately identify
	///memory leaks. This should only be called just
	///before closing, and after retrieving the
	///unclosed object list
  	static void __cl_ClearMemory();

#endif //LUCENE_ENABLE_MEMLEAKTRACKING

	int __cl_refcount;
	LuceneBase(){
		__cl_refcount=1;
	}
	inline int __cl_getref(){
		return __cl_refcount;
	}
	inline int __cl_addref(){
		__cl_refcount++;
		return __cl_refcount;
	}
	inline int __cl_decref(){
		__cl_refcount--;
		return __cl_refcount;
	}
    virtual ~LuceneBase(){};
};

class LuceneVoidBase{
	public:
	#ifdef _DEBUG
		//a compile time check to make sure that _CLDELETE and _CLDECDELETE is being
		//used correctly.
		int dummy__see_mem_h_for_details; 
	#endif
        virtual ~LuceneVoidBase(){};
};


// THIS CHANGE IS HERE ONLY TO BE ABLE TO MEASURE MEMORY CONSUMPTION
// DURING PERFORMANCE TESTS
#ifdef DBG_MEM_USAGE
extern uint32_t PeakDynMemUsage;
#endif


CL_NS_END
#endif //_lucene_debug_lucenebase_
