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

#include "CLucene.h"
#include "CLucene\queryParser\Multifieldqueryparser.h"

#include "cpixidxdb.h"

#include "cpixstrtools.h"
#include "cpixhits.h"
#include "cpixsearch.h"
#include "iidxdb.h"
#include "cpixutil.h"
#include "localization.h"

#include "spi/locale.h"

#include "glib.h"

const char* cpix_LOCALE_AUTO = "auto";
const wchar_t* cpix_WIDE_LOCALE_AUTO = L"auto";

namespace Cpix {

	Localization* Localization::theInstance_ = NULL;
	
	Localization& Localization::instance() {
		if ( !theInstance_ ) {
			theInstance_ = new Localization(); 
		}
		return *theInstance_; 
	}
	
	void Localization::shutdown() {
		delete theInstance_; 
		theInstance_ = false; 
	}

	Localization::Localization() 
	: mutex_(), 
	  auto_( true ),
	  languageNames_(){}	          

	
	void Localization::setLocale(const wchar_t* locale) {
		Cpt::SyncRegion lock( mutex_ );
		
		languageNames_.resize(0);
		if ( wcscmp( locale, cpix_WIDE_LOCALE_AUTO ) == 0 ) {
			auto_ = true; 
		} else {
			auto_ = false;
			languageNames_.push_back(locale);
		}
	}

	void Localization::setLocale(const char* locale) {
		Cpt::auto_array<wchar_t> wlocale(locale, strlen(locale));
		setLocale(wlocale.get()); 
	}
	
	
	std::vector<std::wstring> Localization::getLanguageNames() {
		Cpt::SyncRegion lock( mutex_ );

		if ( auto_ ) {
			// might be slow
			return Spi::GetLanguageNames();
		} else {
			return languageNames_; 
		}
	}
	
}

class SetLocaleFunctor
{
	private: 
	
		const char* locale_;
		
	public:
		
		typedef void result_type; // returns nothing
		
		SetLocaleFunctor(const char * locale)
		: 	locale_(locale) {}
	
		void operator()() {
			Cpix::Localization::instance().setLocale( locale_ ); 
		}
};

void cpix_SetLocale(cpix_Result* result, const char* locale) {
	XlateExc(result, 
 			 SetLocaleFunctor(locale));
}

