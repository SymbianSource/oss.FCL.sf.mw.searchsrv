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

#include <stdio.h>
#include <stdlib.h>
#include <wchar.h>
#include <glib.h>

#include <exception>
#include <memory>
#include <sstream>

// CPix API
#include "cpixanalyzer.h"

// cLucene API
#include "CLucene.h"
#include "CLucene/queryParser/MultiFieldQueryParser.h"
#include "CLucene/debug/mem.h"

// CPix impl details
#include "cluceneext.h"
#include "analyzer.h"
#include "cpixsearch.h"
#include "cpixidxdb.h"
#include "idxdb.h"
#include "initparams.h"
#include "cpixutil.h"


#include "indevicecfg.h" 
 

/**
 * See ISO 639-1
 * http://www.loc.gov/standards/iso639-2/php/English_list.php
 */
const cpix_LangCode cpix_LANG_EN = { "en" };
const cpix_LangCode cpix_LANG_FI = { "fi" };
const cpix_LangCode cpix_LANG_HU = { "hu" };
const cpix_LangCode cpix_LANG_RU = { "ru" };


namespace
{
    const cpix_LangCode * LANGCODES[] = {
        &cpix_LANG_EN,
        NULL
    };
}


const cpix_LangCode ** cpix_supportedLanguages()
{
    return LANGCODES;
}


const wchar_t* SIMPLE_ANALYZER_DEFINITION = L"letter>lowercase"; 
const wchar_t* SNOWBALL_ANALYZER_PREFIX = L"standard>lowercase>stem("; 
const wchar_t* SNOWBALL_ANALYZER_MIDFIX = L")>stop("; 
const wchar_t* SNOWBALL_ANALYZER_POSTFIX = L")"; 

/***********************************************************
 * Actual implementation of CPix search API functions.
 */
cpix_Analyzer * cpix_CreateSimpleAnalyzer(cpix_Result * result)
{
	return cpix_Analyzer_create( result, SIMPLE_ANALYZER_DEFINITION ); 
}

const wchar_t* cpix_ToWideLangCode(cpix_LangCode langCode) {
	if (strcmp( langCode.code_, cpix_LANG_EN.code_) == 0) {
		return CPIX_WLANG_EN;
	}
	return NULL; 
}

cpix_Analyzer * cpix_CreateSnowballAnalyzer(cpix_Result   * result,
                                            cpix_LangCode   langCode)
{
    const wchar_t* wLangCode = cpix_ToWideLangCode( langCode );
    
    if ( wLangCode ) 
    {
		std::wostringstream def;
		def<<SNOWBALL_ANALYZER_PREFIX;
		def<<wLangCode; 
		def<<SNOWBALL_ANALYZER_MIDFIX; 
		def<<wLangCode; 
		def<<SNOWBALL_ANALYZER_POSTFIX; 
	
		return cpix_Analyzer_create( result, def.str().c_str() ); 
    }
	result->err_ 
		= CreateError(ET_CPIX_EXC,
					  "Language not supported");
	return 0;
}

cpix_Analyzer * cpix_Analyzer_create(cpix_Result   * result,
                                     const wchar_t * definition)
{
    cpix_CustomAnalyzer
        * custom = NULL;
    cpix_SystemAnalyzer
        * system = NULL;
        
    custom = Create(result,
                    CallCtor(custom,
                             definition));

    if (cpix_Succeeded(result))
        {
            system = Create(result,
                            CallCtor(system,
                                     Cast2Native(custom)));

            if (cpix_Succeeded(result))
                {
                    // on success, the native instance wrapped by
                    // system will own the native instance wrapped
                    // by custom, so we "release" it from the
                    // ownership of the wrapper custom
                    custom->ptr_ = NULL;
                }

            // the wrapper custom must be released in any case, as
            // it was just a first step in the construction
            // sequence            
        }
    cpix_Analyzer_destroy(custom);

    return system;
}


void cpix_Analyzer_destroy(cpix_Analyzer * thisAnalyzer)
{
    DestroyWrapper(thisAnalyzer);
}



