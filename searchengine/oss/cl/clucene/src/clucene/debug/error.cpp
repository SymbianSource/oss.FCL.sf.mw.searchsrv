/*------------------------------------------------------------------------------
* Copyright (C) 2003-2006 Ben van Klinken and the CLucene Team
* 
* Distributable under the terms of either the Apache License (Version 2.0) or 
* the GNU Lesser General Public License, as specified in the COPYING file.
------------------------------------------------------------------------------*/
#include "clucene/stdheader.h"

CL_NS_USE(util)


#ifdef _LUCENE_DISABLE_EXCEPTIONS
	#ifdef _LUCENE_PRAGMA_WARNINGS
	 #pragma message ("==================Lucene exceptions are disabled==================")
	#else
	 #warning "==================Lucene exceptions are disabled=================="
	#endif
#else
	LuceneError::LuceneError(int num, const char* str, bool ownstr)
	{
		error_number = num;
		_awhat=STRDUP_AtoA(str);
		_twhat=NULL;
		if ( ownstr )
			_CLDELETE_CaARRAY(str);
    }
	
	LuceneError::LuceneError(const LuceneError& clone)
	{
		this->error_number = clone.error_number;
		this->_awhat = NULL;
		this->_twhat = NULL;

		if ( clone._awhat != NULL )
			this->_awhat = STRDUP_AtoA(clone._awhat);
		if ( clone._twhat != NULL )
			this->_twhat = STRDUP_TtoT(clone._twhat);
	}
	LuceneError::~LuceneError() throw(){
		_CLDELETE_CARRAY(_twhat);
		_CLDELETE_CaARRAY(_awhat);
	}
	char* LuceneError::what(){
#ifdef _ASCII
		if ( _twhat != NULL )
			return _twhat;
#endif
		if ( _awhat == NULL )
			_awhat = STRDUP_TtoA(_twhat);
		return _awhat;
	}
	TCHAR* LuceneError::twhat(){
#ifdef _ASCII
		if ( _awhat != NULL )
			return _awhat;
#endif
		if ( _twhat == NULL )
			_twhat = STRDUP_AtoT(_awhat);
		return _twhat;
	}

#ifndef _ASCII
	LuceneError::LuceneError(int num, const TCHAR* str, bool ownstr)
	{
		error_number = num;
		_awhat=NULL;
		_twhat=STRDUP_TtoT(str);
		if ( ownstr )
			_CLDELETE_CARRAY(str);
    }
#endif

#endif //_LUCENE_DISABLE_EXCEPTIONS
