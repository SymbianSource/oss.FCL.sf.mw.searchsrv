/*------------------------------------------------------------------------------
* Copyright (C) 2003-2006 Ben van Klinken and the CLucene Team
* 
* Distributable under the terms of either the Apache License (Version 2.0) or 
* the GNU Lesser General Public License, as specified in the COPYING file.
------------------------------------------------------------------------------*/
#ifndef _lucene_util_StringIntern_H
#define _lucene_util_StringIntern_H

#if defined(_LUCENE_PRAGMA_ONCE)
# pragma once
#endif

#include "CLucene/util/VoidMap.h"
CL_NS_DEF(util)
typedef CL_NS(util)::LHashMap<const TCHAR*,int,CL_NS(util)::Compare::TChar,CL_NS(util)::Equals::TChar,CL_NS(util)::Deletor::tcArray, CL_NS(util)::Deletor::DummyInt32 > __wcsintrntype;
typedef CL_NS(util)::LHashMap<const char*,int,CL_NS(util)::Compare::Char,CL_NS(util)::Equals::Char,CL_NS(util)::Deletor::acArray, CL_NS(util)::Deletor::DummyInt32 > __strintrntype;

  /** Functions for intern'ing strings. This
  * is a process of pooling strings thus using less memory,
  * and furthermore allows intern'd strings to be directly
  * compared:
  * string1==string2, rather than _tcscmp(string1,string2)
  */
  class LStringIntern{
#if defined (__SYMBIAN32__) && defined (RVCT22_STATICS_WORKAROUND)
      DECL_STATMVAR1(__wcsintrntype,stringPool,true)
#define stringPool GetInstance__wcsintrntypestringPool()

      DECL_STATMVAR1(__strintrntype,stringaPool,true)
#define stringaPool GetInstance__strintrntypestringaPool()

      // CPIXASYNC STATIC_DEFINE_MUTEX(THIS_LOCK)
      STATIC_DEFINE_CRUCIAL_MUTEX1(CLStringIntern_THIS_LOCK)
#define CLStringIntern_THIS_LOCK GetInstancemutex_pthreadCLStringIntern_THIS_LOCK()
#else
	  static __wcsintrntype stringPool;
	  static __strintrntype stringaPool;

      // CPIXASYNC STATIC_DEFINE_MUTEX(THIS_LOCK)
          STATIC_DEFINE_CRUCIAL_MUTEX(THIS_LOCK)
#endif
  public:
	/** 
	* Internalise the specified string.
	* \return Returns a pointer to the internalised string
	*/
	static const char* internA(const char* str CL_FILELINEPARAM);
	/** 
	* Uninternalise the specified string. Decreases
	* the reference count and frees the string if 
	* reference count is zero
	* \returns true if string was destroyed, otherwise false
	*/
	static bool uninternA(const char* str);

	/** 
	* Internalise the specified string.
	* \return Returns a pointer to the internalised string
	*/
	static const TCHAR* intern(const TCHAR* str CL_FILELINEPARAM);
	
	/** 
	* Uninternalise the specified string. Decreases
	* the reference count and frees the string if 
	* reference count is zero
	* \returns true if string was destroyed, otherwise false
	*/
	static bool unintern(const TCHAR* str);

    static void shutdown();
  };

CL_NS_END
#endif
