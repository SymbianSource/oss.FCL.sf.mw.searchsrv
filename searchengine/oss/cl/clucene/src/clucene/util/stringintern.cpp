/*------------------------------------------------------------------------------
* Copyright (C) 2003-2006 Ben van Klinken and the CLucene Team
* 
* Distributable under the terms of either the Apache License (Version 2.0) or 
* the GNU Lesser General Public License, as specified in the COPYING file.
------------------------------------------------------------------------------*/
#include "clucene/stdheader.h"
#include "StringIntern.h"
CL_NS_DEF(util)

/* ??? FIX TODO TEMP CAN WE GET AWAY WITH THIS?
#if defined (__SYMBIAN32__) && defined (RVCT22_STATICS_WORKAROUND)
	typedef __wcsintrntype::iterator __wcsintrntype_iterator; 
	DECL_STATVAR(__wcsintrntype_iterator,wblank)
#define wblank GetInstance__wcsintrntype_iteratorwblank()
#else
	__wcsintrntype::iterator wblank;
// ??? FIX TODO #endif

	bool blanksinitd=false;
??? */

#if defined (__SYMBIAN32__) && defined (RVCT22_STATICS_WORKAROUND)
	DECL_STATMVAR2(__wcsintrntype,LStringIntern,stringPool)
	DECL_STATMVAR2(__strintrntype,LStringIntern,stringaPool)

// CPIXASYNC DEFINE_MUTEX(LStringIntern::THIS_LOCK)
        STATIC_DEFINE_CRUCIAL_MUTEX2(LStringIntern,CLStringIntern_THIS_LOCK)
#else
	__wcsintrntype LStringIntern::stringPool(true);
	__strintrntype LStringIntern::stringaPool(true);

// CPIXASYNC DEFINE_MUTEX(LStringIntern::THIS_LOCK)
        DEFINE_CRUCIAL_MUTEX(LStringIntern::CLStringIntern_THIS_LOCK)
#endif
	

    void LStringIntern::shutdown(){
#if defined (__SYMBIAN32__) && defined (RVCT22_STATICS_WORKAROUND)
            { // scope to make the mutex unlocked before its destroyed
#endif

#ifdef _DEBUG
            // CPIXASYNC SCOPED_LOCK_MUTEX(THIS_LOCK)
            SCOPED_LOCK_CRUCIAL_MUTEX(CLStringIntern_THIS_LOCK)
                if ( stringaPool.size() > 0 ){
                    printf("ERROR: stringaPool still contains intern'd strings (refcounts):\n");
                    __strintrntype::iterator itr = stringaPool.begin();
                    while ( itr != stringaPool.end() ){
                        printf(" %s (%d)\n",(itr->first), (itr->second));
                        ++itr;
                    }
                }
            
            if ( stringPool.size() > 0 ){
                printf("ERROR: stringPool still contains intern'd strings (refcounts):\n");
                __wcsintrntype::iterator itr = stringPool.begin();
                while ( itr != stringPool.end() ){
#ifdef __SYMBIAN32__
                    _tprintf(_T(" %S (%d)\n"),(itr->first), (itr->second));
#else 
                    _tprintf(_T(" %s (%d)\n"),(itr->first), (itr->second));
#endif
                    ++itr;
                }
            }
#endif
            
#if defined (__SYMBIAN32__) && defined (RVCT22_STATICS_WORKAROUND)
            StaticInstances::shutdown(&LStringIntern::stringaPool); 
            StaticInstances::shutdown(&LStringIntern::stringPool); 
            }

#if !defined (_CL_DISABLE_MULTITHREADING) || defined (CPIX_ASYNC_MT)
            // the lock is held by this thread, it should be able to
            // destroy it
            StaticInstances::shutdown(&LStringIntern::CLStringIntern_THIS_LOCK);
#endif
#endif
    }

	const TCHAR* LStringIntern::intern(const TCHAR* str CL_FILELINEPARAM){
		if ( str == NULL )
			return NULL;
		if ( str[0] == 0 )
			return LUCENE_BLANK_STRING;

		// CPIXASYNC SCOPED_LOCK_MUTEX(THIS_LOCK)
                SCOPED_LOCK_CRUCIAL_MUTEX(CLStringIntern_THIS_LOCK)

		__wcsintrntype::iterator itr = stringPool.find(str);
		if ( itr==stringPool.end() ){
#ifdef _UCS2
			TCHAR* ret = lucenewcsdup(str CL_FILELINEREF);
#else
			TCHAR* ret = lucenestrdup(str CL_FILELINEREF);
#endif
			stringPool[ret]= 1;
			return ret;
		}else{
			(itr->second)++;
			return itr->first;
		}
	}

	bool LStringIntern::unintern(const TCHAR* str){
		if ( str == NULL )
			return false;
		if ( str[0] == 0 )
			return false;

		// CPIXASYNC SCOPED_LOCK_MUTEX(THIS_LOCK)
                SCOPED_LOCK_CRUCIAL_MUTEX(CLStringIntern_THIS_LOCK)

		__wcsintrntype::iterator itr = stringPool.find(str);
		if ( itr != stringPool.end() ){
			if ( (itr->second) == 1 ){
				stringPool.removeitr(itr);
				return true;
			}else{
				(itr->second)--;
			}
		}
		return false;
	}
	
	const char* LStringIntern::internA(const char* str CL_FILELINEPARAM){
		if ( str == NULL )
			return NULL;
		if ( str[0] == 0 )
			return _LUCENE_BLANK_ASTRING;

		// CPIXASYNC SCOPED_LOCK_MUTEX(THIS_LOCK)
                SCOPED_LOCK_CRUCIAL_MUTEX(CLStringIntern_THIS_LOCK)

		__strintrntype::iterator itr = stringaPool.find(str);
		if ( itr==stringaPool.end() ){
			char* ret = lucenestrdup(str CL_FILELINE);
			stringaPool[ret] = 1;
			return ret;
		}else{
			(itr->second)++;
			return itr->first;
		}
	}
	
	bool LStringIntern::uninternA(const char* str){
		if ( str == NULL )
			return false;
		if ( str[0] == 0 )
			return false;

		// CPIXASYNC SCOPED_LOCK_MUTEX(THIS_LOCK)
                SCOPED_LOCK_CRUCIAL_MUTEX(CLStringIntern_THIS_LOCK)

		__strintrntype::iterator itr = stringaPool.find(str);
		if ( itr!=stringaPool.end() ){
			if ( (itr->second) == 1 ){
				stringaPool.removeitr(itr);
				return true;
			}else
				(itr->second)--;
		}
		return false;
	}

	/* removed because of multi-threading problems...
	__wcsintrntype::iterator LStringIntern::internitr(const TCHAR* str CL_FILELINEPARAM){
		if ( str[0] == 0 ){
			if ( !blanksinitd ){
				LStringIntern::stringPool.put(LUCENE_BLANK_STRING,1);
				wblank=stringPool.find(str); 
				blanksinitd=true;
			}
			return wblank;
		}
		__wcsintrntype::iterator itr = stringPool.find(str);
		if (itr==stringPool.end()){
#ifdef _UCS2
			TCHAR* ret = lucenewcsdup(str CL_FILELINEREF);
#else
			TCHAR* ret = lucenestrdup(str CL_FILELINEREF);
#endif
			stringPool.put(ret,1);
			return stringPool.find(str);
		}else{
			(itr->second)++;
			return itr;
		}
	}
	bool LStringIntern::uninternitr(__wcsintrntype::iterator itr){
		if ( itr!=stringPool.end() ){
			if ( itr==wblank )
				return false;	
			if ( (itr->second) == 1 ){
				stringPool.removeitr(itr);
				return true;
			}else
				(itr->second)--;
		}
		return false;
	}
*/

CL_NS_END
