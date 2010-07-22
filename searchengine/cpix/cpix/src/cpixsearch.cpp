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

// Own header
#include "cpixsearch.h"

// cLucene API
#include "CLucene.h"
#include "CLucene/queryParser/MultiFieldQueryParser.h"

// CPix internal
// OBS #include "cpixsearch.h"
#include "iidxdb.h"
#include "idxdb.h"
#include "cpixhits.h"
#include "initparams.h"
#include "cpixutil.h"
#include "iqrytype.h"
#include "document.h"
#include "analyzer.h"
#include "queryparser.h"


/***********************************************************
 * Actual implementation of CPix search API functions.
 */
void cpix_Query_destroy(cpix_Query * thisQuery)
{
    DestroyWrapper(thisQuery);
}



cpix_QueryParser * 
cpix_QueryParser_create(cpix_Result   * result,
                        const wchar_t * fieldName,
                        cpix_Analyzer * analyzer)
{
    lucene::analysis::Analyzer
        * a = Cast2Native<cpix_Analyzer>(analyzer);

    cpix_QueryParser
        * rv = NULL;

    Cpix::IQueryParser* parser = 
		XlateExc(
			result, 
			CallFreeFunc(
				&Cpix::CreateCLuceneQueryParser,
				fieldName, 
				a));
    
    if ( cpix_Succeeded( result ) ) {
		CreateWrapper(parser, result, rv); 
    }
    return rv;
}

cpix_QueryParser * 
    cpix_CreatePrefixQueryParser(cpix_Result   * result, 
								 const wchar_t * fieldName)
{
    using namespace lucene::analysis;

   cpix_QueryParser
        * rv = NULL;

    Cpix::IQueryParser* parser = 
		XlateExc(
			result, 
			CallFreeFunc(
				&Cpix::CreatePrefixQueryParser,
				fieldName));
    
    if ( cpix_Succeeded( result ) ) {
		CreateWrapper(parser, result, rv); 
    }
    return rv;
}



cpix_QueryParser *
	cpix_CreateMultiFieldQueryParser(cpix_Result   * result,
									 const wchar_t * fieldNames[],
									 cpix_Analyzer * analyzer,
									 cpix_BoostMap * boosts)
{
    cpix_QueryParser
        * rv = NULL;
    
    lucene::analysis::Analyzer
        * a = Cast2Native<cpix_Analyzer>(analyzer);

    Cpix::IQueryParser* parser = 
		XlateExc(
			result, 
			CallFreeFunc(
				&Cpix::CreateCLuceneMultiFieldQueryParser,
				fieldNames,
				a, 
				Cast2Native<cpix_BoostMap>(boosts)));
    
    if ( cpix_Succeeded( result ) ) {
		CreateWrapper(parser, result, rv); 
    }
    return rv;
}


cpix_BoostMap *
cpix_BoostMap_create(cpix_Result * result)
{
    cpix_BoostMap
        * rv = NULL;

    rv = Create(result,
                CallCtor(rv, true, false)); // own keys & values

    return rv;
}


// the only reason we do this operation in a separate function is that
// it performs dynamic memory allocation that may fail (STRDUP_TtoT),
// and it must be properly exception-translated, and the whole
// gadgetry of XlateExc function et al can be invoked upon functions
// only
void BoostMapPutImpl(lucene::queryParser::BoostMap * boostMap,
                     const wchar_t                 * fieldName,
                     float                           boost)
{
    const wchar_t
        * copy = STRDUP_TtoT(fieldName);
    boostMap->put(copy,
                  boost);
}


void cpix_BoostMap_put(cpix_BoostMap * thisMap,
                       const wchar_t * fieldName,
                       float           boost)
{
    XlateExc(thisMap,
             CallFreeFunc(&BoostMapPutImpl,
                          Cast2Native<cpix_BoostMap>(thisMap),
                          fieldName,
                          boost));
}



void cpix_BoostMap_destroy(cpix_BoostMap * thisMap)
{
    DestroyWrapper(thisMap);
}

void
cpix_QueryParser_setDefaultOperator(cpix_QueryParser * thisQueryParser,
                                    cpix_QP_Operator   op)
{
    XlateExc(thisQueryParser,
             Caller(thisQueryParser,
                    &Cpix::IQueryParser::setDefaultOperator,
                    op));
}

cpix_Query * 
cpix_QueryParser_parse(cpix_QueryParser * thisQueryParser,
                       const wchar_t    * queryStr)
{
    cpix_Query
        * rv = NULL;

    using namespace Cpix;

    IQryType
        * qryType = XlateExc(thisQueryParser,
                             CallFreeFunc(&IQryType::parseQry,
                                          thisQueryParser,
                                          queryStr));

    if (cpix_Succeeded(thisQueryParser))
        {
            CreateWrapper(qryType,
                          thisQueryParser,
                          rv);
        }

    return rv;
}



void cpix_QueryParser_destroy(cpix_QueryParser * thisQueryParser)
{
    DestroyWrapper(thisQueryParser);
}


int32_t cpix_Hits_length(cpix_Hits * thisHits)
{
    using namespace Cpix; 
    int32_t
        rv = 0;

    rv = XlateExc(thisHits,
                  Caller(thisHits,
                             &::Cpix::IHits::length));

    return rv;
}


void cpix_Hits_doc(cpix_Hits     * thisHits,
                   int32_t         index,
                   cpix_Document * target)
{
    Cpix::Document
        * pDoc = XlateExc(thisHits,
                          Caller(thisHits,
                                     &Cpix::IHits::getDocument,
                                     index));
    if (cpix_Succeeded(thisHits))
    {
    	target->ptr_ = pDoc;
        target->err_ = NULL;

        // TEMP (?) post-condition code
        if (target->ptr_ == NULL)
            {
                thisHits->err_
                    = CreateError(ET_CPIX_EXC,
                                  L"PANIC PANIC PANIC - NULL doc returned without any error message!");
            }
    }
}


/* OBS
int cpix_Hits_isValid(cpix_Hits * thisHits)
{
    using namespace Cpix;

    bool
        isValid = XlateExc(thisHits,
                           Caller(thisHits,
                                      &IHits::isValid));

    return static_cast<int>(isValid);
}
*/


void cpix_Hits_destroy(cpix_Hits * thisHits)
{
    DestroyWrapper(thisHits);
}



template<>
void
CreateWrapper<IdxDbHndl,cpix_Result,cpix_IdxSearcher>(IdxDbHndl      hndl,
													  cpix_Result  * result,
													  cpix_IdxSearcher  *& wrapper)
{
    wrapper = NULL;
    wrapper = CreateWrapper_Pure<cpix_IdxSearcher,cpix_Result>(result);
    if (wrapper != NULL)
        {
            wrapper->handle_ = hndl;
            wrapper->err_ = NULL;
        }
    else
        {
            Cpix::IIdxDb::release(hndl);
        }
}



// special constructor functor that does not return with a pointer to
// an instance but with a handle
class IdxSearcherCtorFunctor
{
    
    const char *   baseAppClass_;
public:
    typedef Cpix::IIdxDb NativeClass;
    typedef cpix_IdxSearcher WrpClass;  

    IdxSearcherCtorFunctor(const char * baseAppClass)
        : baseAppClass_(baseAppClass)
    {
        ;
    }


    IdxDbHndl operator()()
    {
        using namespace Cpix;

        // opening
        return IIdxDb::getIdxDbHndl(baseAppClass_,
                                    true); // allow multi search
    }
};



cpix_IdxSearcher * cpix_IdxSearcher_openDb(cpix_Result * result,
                                           const char  * domainSelector)
{
    cpix_IdxSearcher
        * rv = NULL;
    rv = Create(result,
                IdxSearcherCtorFunctor(domainSelector));
    return rv;
}



void cpix_IdxSearcher_releaseDb(cpix_IdxSearcher * thisIdxSearcher)
{
    if (thisIdxSearcher != NULL)
        {
            cpix_Result
                result;
            XlateExc(&result,
                     CallFreeFunc(&Cpix::IIdxDb::release,
                                  thisIdxSearcher->handle_));
            free(thisIdxSearcher);
        }
}


cpix_Hits * 
cpix_IdxSearcher_search(cpix_IdxSearcher * thisIdxSearcher,
                        cpix_Query       * query)
{
    cpix_Hits
        * rv = NULL;

    using namespace lucene::search;
    using namespace Cpix;

    cpix_Hits * (Cpix::IQryType:: * func)(cpix_IdxSearcher *) 
        = &Cpix::IQryType::search;

    // This is curious and unusual setup: the error reporting goes on
    // thisIdxSearcher instance, but the instance used as "this"
    // instance is the query instance. This is due to the extra level
    // of indirection added by unified search interface and hiding
    // various types of queries behind the query instance
    rv = XlateExc(thisIdxSearcher,                // report
                  Caller(query,               // "this"
                             func,
                             thisIdxSearcher));   // argument

    return rv;
}


