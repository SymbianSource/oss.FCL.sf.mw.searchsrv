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

#include "clucene.h"
#include "clucene/queryParser/multifieldqueryparser.h"

#include "cpixtools.h"

#include "cpixhits.h"
#include "cpixsearch.h"
#include "cpixidxdb.h"
#include "cpixexc.h"
#include "iidxdb.h"
#include "initparams.h"
#include "cpixutil.h"
#include "iqrytype.h"
#include "analyzer.h"
#include "customanalyzer.h"

#include "cpixmaindefs.h"
#include "queryparser.h"


namespace Cpix
{
    
    /**
     * Format and semantics are described in iqrytype.cpp
     */
    class PrefixQryType : public IQryType
    {
    private:
        //
        // private members
        //
        IQueryParser * clQueryParser_;
        lucene::search::Query            * clQuery_;

    public:
        //
        // public operators
        //

        //
        // from interface IQryType
        //
        
        virtual void setUp(cpix_QueryParser              * queryParser,
                           const std::list<std::wstring> & args,
                           const wchar_t                 * qryStr)
        {
            wchar_t *mQryStr = NULL;
            
            if (args.size() > 0)
            {
                THROW_CPIXEXC(PL_ERROR "No arguments needed here");
            }

            mQryStr = (wchar_t*) malloc(sizeof(wchar_t)* (2 * wcslen(qryStr)));
            
            if(mQryStr == NULL)
                {
                    THROW_CPIXEXC("Memory allocation failed. Query parsing failed.");
                }
            
            wmemset(mQryStr,0,(2 * wcslen(qryStr)));
            getAnalyzedString(qryStr, mQryStr );
            
            clQueryParser_ = Cast2Native<cpix_QueryParser>(queryParser);
            clQuery_ = clQueryParser_->parse((const wchar_t *)mQryStr).release();
            
            
            free(mQryStr);
            
            if (clQuery_ == NULL)
                {
                    THROW_CPIXEXC("Query reduced to empty query.");
                }
        }


        virtual cpix_Hits * search(cpix_IdxSearcher * idxSearcher)
        {
            return CLuceneSearchIdx(idxSearcher,
                                    clQuery_);
        }


        virtual cpix_Hits * search(cpix_IdxDb * idxDb)
        {
            return CLuceneSearchIdx(idxDb,
                                    clQuery_);
        }

        void getAnalyzedString(const wchar_t* input, wchar_t* output)
            {
                CL_NS_USE(index)
                CL_NS_USE(util)
                CL_NS_USE(store)
                CL_NS_USE(search)
                CL_NS_USE(document)
                CL_NS_USE(queryParser)
                CL_NS_USE(analysis)
                CL_NS_USE2(analysis,standard)
                
                /*
                 * StandardAnalyzer sAnalyser;
                 * Used before but this but this includes stopwords filters
                 */
                CustomAnalyzer sAnalyser((const wchar_t*)L"stdtokens>stdfilter>lowercase");
                
                Reader* reader = _CLNEW StringReader(input);
                TokenStream* ts = sAnalyser.tokenStream(_T("dummy"), reader );
                Token t;
    
                while(ts->next(&t))
                    {
                    wcscat(output,t.termText());
                    wcscat(output,L"* ");
                    }
                size_t len = wcslen(output);
    
                if(len == 0)
                wcscpy(output,L"*");
                else
                    {
                    if(output[len-1] == L' ')
                    output[len-1] = L'\0';
                    }
    
                ts->close();
                _CLDELETE(ts);
                _CLDELETE(reader);
            }


    private:
        //
        // private implementation details
        //
    };



    IQryType * CreatePrefixQryType()
    {
        return new PrefixQryType;
    }

}
