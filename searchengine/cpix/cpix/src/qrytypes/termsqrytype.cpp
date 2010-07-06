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

#include "CLucene.h"
#include "CLucene/queryParser/MultiFieldQueryParser.h"

#include "cpixtools.h"

#include "cpixhits.h"
#include "cpixsearch.h"
#include "cpixidxdb.h"
#include "cpixexc.h"
#include "iidxdb.h"
#include "initparams.h"
#include "cpixutil.h"
#include "iqrytype.h"

#include "queryparser.h"


namespace Cpix
{
    
    /**
     * Format and semantics are described in iqrytype.cpp
     */
    class TermsQryType : public IQryType
    {
    private:
        //
        // private members
        //
        std::wstring           fieldName_;
        std::wstring           qryStr_;
        std::wstring           appclassPrefix_;
        int                    maxHitCount_;


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
            if (args.size() < 1)
                {
                    THROW_CPIXEXC("Needs argument for max hit count");
                }
            if (args.size() > 2)
                {
					THROW_CPIXEXC("Too many arguments for terms search");
                }

            IQueryParser
                * qp = Cast2Native<cpix_QueryParser>(queryParser);

            fieldName_ = qp->getField();

            wconvertThrowing(&maxHitCount_,
                             args.front().c_str());
            
            if (args.size() >= 2) {
				std::wstring lit = *(++args.begin());  // drop citation marks
				appclassPrefix_ = lit.substr(1, lit.length()-2); 
            }

            qryStr_ = qryStr;
        }


        virtual cpix_Hits * search(cpix_IdxSearcher * idxSearcher)
        {
            return searchIDX(idxSearcher);
        }


        virtual cpix_Hits * search(cpix_IdxDb * idxDb)
        {
            return searchIDX(idxDb);
        }

    private:
        //
        // private implementation details
        //
        
        template<typename IDX>
        cpix_Hits * searchIDX(IDX * idx)
        {
            cpix_Hits
                * rv = NULL;
            
            using namespace Cpix;
            
            IHits
                * hits = XlateExc(idx,
                                  Caller(idx,
                                                &IIdxDb::getTerms,
                                                fieldName_.c_str(), 
                                                qryStr_.c_str(), 
                                                appclassPrefix_.length() ? appclassPrefix_.c_str() : 0, 
                                                maxHitCount_));
            
            if (cpix_Succeeded(idx))
                {
                    CreateWrapper(hits,
                                  idx,
                                  rv);
                }
            
            return rv;
        }
    };



    IQryType * CreateTermsQryType()
    {
        return new TermsQryType;
    }

}
