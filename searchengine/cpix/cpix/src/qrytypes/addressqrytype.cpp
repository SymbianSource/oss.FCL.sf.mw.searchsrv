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
#include "cpixdoc.h"
#include "cpixsearch.h"
#include "cpixidxdb.h"
#include "cpixexc.h"
#include "iidxdb.h"
#include "initparams.h"
#include "cpixutil.h"
#include "iqrytype.h"
#include "common/gpssort.h"

namespace Cpix
{

    /**
     * Implements the plumbing part for invoking the actual address
     * search algorightm. Implements also sorting based on distance
     * measured from a reference point given by its GPS coordinates.
     */
    class AddressQryType : public IQryType
    {
    private:
        //
        // private members
        //
        float        refPointGpsLat_;
        float        refPointGpsLong_;
        std::wstring qryStr_;
    
    public:
    
        //
        // from interface IQryType
        //
        virtual void setUp(cpix_QueryParser              * queryParser,
                           const std::list<std::wstring> & args,
                           const wchar_t                 * qryStr);
    
        virtual cpix_Hits * search(cpix_IdxSearcher * idxSearcher);
    
        virtual cpix_Hits * search(cpix_IdxDb * idxDb);
    };


    /**
     * Implements the algorithmic guts of address search using CPix
     * public API. It does not do ranking, only search.
     *
     * Format and semantics are described in iqrytype.cpp
     *
     * TODO NOSE has a newer version of address search to be taken
     * into use (to be ported).
     */
    class AddressSearch
    {
    private:

        // Searches are done on this. Also, it carries the error flags, if
        // any.
        cpix_IdxSearcher      * idxSearcher_;

        cpix_Analyzer         * analyzer_;

        // reference point Longitude and Longitude and stored as member variables.
        static const float   primaryTokensWeight_;

        static const wchar_t fieldPrimaryTokens_[];
        static const wchar_t fieldSecondaryTokens_[];
        static const float   citySecondaryTokensWeight_;

        static const wchar_t fieldStreetPrimaryTokens_[];
        static const wchar_t fieldStreetSecondaryTokens_[];
        static const float   streetSecondaryTokensWeight_;

    public:
        AddressSearch(cpix_IdxSearcher * idxSearcher)
            : idxSearcher_(idxSearcher),
              analyzer_(NULL)
        {
            cpix_Result
                result;

            analyzer_ = cpix_CreateSimpleAnalyzer(&result);

            if (cpix_Failed(&result))
                {
                    moveErrorCode(idxSearcher_,
                                  &result);
                }
        }


        ~AddressSearch()
        {
            cpix_Analyzer_destroy(analyzer_);
        }


        bool ok() const
        {
            return cpix_Succeeded(idxSearcher_);
        }


        cpix_Hits * search(const wchar_t    * qryStr)
        {
            int32_t
                hitCount = 0;

            cpix_Hits
                * rv = search1stRound(qryStr,
                                      hitCount);

            if (ok() && hitCount == 0)
                {
                    cpix_Hits_destroy(rv);
                    rv = NULL;
                    rv = search2ndRound(qryStr,
                                        hitCount);
                    
                    if (ok() && hitCount == 0)
                        {
                            cpix_Hits_destroy(rv);
                            rv = NULL;
                            rv = search3rdRound(qryStr,
                                                hitCount);

                            if (ok() && hitCount == 0)
                                {
                                    cpix_Hits_destroy(rv);
                                    rv = NULL;
                                    rv = search4thRound(qryStr);
                                }
                        }
                        
                }
            
            return rv;
        }

        static cpix_Hits * search(cpix_IdxSearcher * idxSearcher,
                                  const wchar_t    * qryStr,
                                  float              refPointGpsLat,
                                  float              refPointGpsLong)

        {
            cpix_Hits
                * rv = NULL;

            AddressSearch
                as(idxSearcher);

            if (as.ok())
                {
                    rv = as.search(qryStr);

                    try
                        {
                            cpix_Result
                                result;

                            cpix_Hits
                                * sorted = gpsSort(refPointGpsLat,
                                                   refPointGpsLong,
                                                   rv,
                                                   &result);
                            
                            if (cpix_Succeeded(idxSearcher))
                                {
                                    cpix_Hits_destroy(rv);
                                    rv = sorted;
                                }
                            else
                                {
                                    idxSearcher->err_ = result.err_;
                                }
                        }
                    catch (...)
                        {
                            cpix_Hits_destroy(rv);
                            rv = NULL;
                            throw;
                        }
                  
                }

            return rv;
        }

    private:

        template<typename CPIX_RESULT>
        void moveErrorCode(cpix_IdxSearcher * to,
                           CPIX_RESULT      * from)
        {
            to->err_ = from->err_;
        }


        cpix_BoostMap * createBoostMap(const wchar_t * primaryFieldName,
                                       const wchar_t * secondaryFieldName,
                                       float           secondaryTokensWeight)
        {
            cpix_BoostMap
                * rv = NULL;

            cpix_Result
                result;

            cpix_BoostMap
                * boosts = cpix_BoostMap_create(&result);

            if (cpix_Succeeded(boosts))
                {
                    cpix_BoostMap_put(boosts,
                                      primaryFieldName,
                                      primaryTokensWeight_);

                    if (cpix_Succeeded(boosts))
                        {
                            cpix_BoostMap_put(boosts,
                                              secondaryFieldName,
                                              secondaryTokensWeight);

                            if (cpix_Succeeded(boosts))
                                {
									rv = boosts;
                                }
                            else 
                            	{
									moveErrorCode(idxSearcher_,
												  boosts);
                            	}
                        }
                    else
                        {
                            moveErrorCode(idxSearcher_,
                                          boosts);
                        }
                }
            else
                {
                    moveErrorCode(idxSearcher_,
                                  &result);
                }
            
            if ( rv != boosts ) 
            	{
            	cpix_BoostMap_destroy( boosts ); 
            	}

            return rv;
        }


        cpix_Query * formSecondaryQuery(const wchar_t * qryStr,
                                        const wchar_t * primaryFieldName,
                                        const wchar_t * secondaryFieldName,
                                        float           secondaryTokensWeight)
        {
            cpix_Query
                * rv = NULL;

            cpix_BoostMap
                * boosts = createBoostMap(primaryFieldName,
                                          secondaryFieldName,
                                          secondaryTokensWeight);

            if (ok())
                {
                    cpix_Result
                        result;

                    const wchar_t
                        * fieldNames[] = {
                        primaryFieldName,
                        secondaryFieldName,
                        NULL
                    };

                    cpix_QueryParser
                        * qp = cpix_CreateMultiFieldQueryParser(&result,
                                                                fieldNames,
                                                                analyzer_,
                                                                boosts);
                                                 
                    if (cpix_Succeeded(&result))
                        {
                            cpix_QueryParser_setDefaultOperator(qp,
                                                                cpix_QP_AND_OPERATOR);
                        
                            if (cpix_Succeeded(qp))
                                {
                                    rv = cpix_QueryParser_parse(qp,
                                                                qryStr);
                                    if (cpix_Failed(qp))
                                        {
                                            moveErrorCode(idxSearcher_,
                                                          qp);
                                        }
                                }
                            else
                                {
                                    moveErrorCode(idxSearcher_,
                                                  qp);
                                }
                        
                            cpix_QueryParser_destroy(qp);
                        }
                    else
                        {
                            moveErrorCode(idxSearcher_,
                                          &result);
                        }

                    cpix_BoostMap_destroy(boosts);
                }

            return rv;
        }


        cpix_Query * formPrimaryQuery(const wchar_t * qryStr,
                                      const wchar_t * fieldName)
        {
            cpix_Query
                * rv = NULL;

            cpix_Result
                result;

            cpix_QueryParser
                * qp = cpix_QueryParser_create(&result,
                                               fieldName,
                                               analyzer_);
            if (cpix_Succeeded(&result))
                {
                    cpix_QueryParser_setDefaultOperator(qp,
                                                        cpix_QP_AND_OPERATOR);

                    if (cpix_Succeeded(qp))
                        {
                            rv = cpix_QueryParser_parse(qp,
                                                        qryStr);
                            if (cpix_Failed(qp))
                                {
                                    moveErrorCode(idxSearcher_,
                                                  qp);
                                }
                        }
                    else
                        {
                            moveErrorCode(idxSearcher_,
                                          qp);
                        }

                    cpix_QueryParser_destroy(qp);
                }
            else
                {
                    moveErrorCode(idxSearcher_,
                                  &result);
                }

            return rv;
        }


        cpix_Hits * doSearchIfOk(cpix_Query * qry,
                                 int32_t    & hitCount)
        {
            cpix_Hits
                * rv = NULL;

            if (ok())
                {
                    rv = cpix_IdxSearcher_search(idxSearcher_,
                                                 qry);
                    if (cpix_Succeeded(idxSearcher_))
                        {
                            hitCount = cpix_Hits_length(rv);
                        }
                }

            return rv;
        }


        /**
         * First round of searching.
         */
        cpix_Hits * search1stRound(const wchar_t * qryStr,
                                   int32_t       & hitCount)
        {
            cpix_Hits
                * rv = NULL;

            cpix_Query
                * qry = formPrimaryQuery(qryStr,
                                         fieldPrimaryTokens_);
            rv = doSearchIfOk(qry,
                              hitCount);

            cpix_Query_destroy(qry);

            return rv;
        }


        /**
         * Second round of searching.
         */
        cpix_Hits * search2ndRound(const wchar_t * qryStr,
                                   int32_t       & hitCount)
        {
            cpix_Hits
                * rv = NULL;

            cpix_Query
                * qry = formSecondaryQuery(qryStr,
                                           fieldPrimaryTokens_,
                                           fieldSecondaryTokens_,
                                           citySecondaryTokensWeight_);

            rv = doSearchIfOk(qry,
                              hitCount);

            cpix_Query_destroy(qry);

            return rv;
        }



        /**
         * Third round of searching.
         */
        cpix_Hits * search3rdRound(const wchar_t * qryStr,
                                   int32_t       & hitCount)
        {
            cpix_Hits
                * rv = NULL;

            cpix_Query
                * qry = formPrimaryQuery(qryStr,
                                         fieldStreetPrimaryTokens_);
            rv = doSearchIfOk(qry,
                              hitCount);

            cpix_Query_destroy(qry);

            return rv;
        }



        cpix_Hits * search4thRound(const wchar_t * qryStr)
        {
            cpix_Hits
                * rv = NULL;
            int32_t
                dummy;

            cpix_Query
                * qry = formSecondaryQuery(qryStr,
                                           fieldStreetPrimaryTokens_,
                                           fieldStreetSecondaryTokens_,
                                           streetSecondaryTokensWeight_);
            rv = doSearchIfOk(qry,
                              dummy);

            cpix_Query_destroy(qry);

            return rv;
        }

    };

    
    const float AddressSearch::primaryTokensWeight_           = 6.0f;

    const float   AddressSearch::citySecondaryTokensWeight_   = 1.0f;
    const wchar_t AddressSearch::fieldPrimaryTokens_[]        
    = L"PrimaryTokens";
    const wchar_t AddressSearch::fieldSecondaryTokens_[]      
    = L"SecondaryTokens";

    const float   AddressSearch::streetSecondaryTokensWeight_ = 0.000001f;
    const wchar_t AddressSearch::fieldStreetPrimaryTokens_[]  
    = L"StreetPrimaryTokens";
    const wchar_t AddressSearch::fieldStreetSecondaryTokens_[]
    = L"StreetSecondaryTokens";


    void AddressQryType::setUp(cpix_QueryParser              * ,
                               const std::list<std::wstring> & args,
                               const wchar_t                 * qryStr)
    {
        if (args.size() != 2)
            {
                THROW_CPIXEXC("Missing args: ref point GPS coords (two reals)");
            }
    
        std::list<std::wstring>::const_iterator
            i = args.begin();
    
        wconvertThrowing(&refPointGpsLat_,
                         i->c_str());
        ++i;
        wconvertThrowing(&refPointGpsLong_,
                         i->c_str());
    
        qryStr_ = qryStr;
    }
    
    
    cpix_Hits * AddressQryType::search(cpix_IdxSearcher * idxSearcher)
    {
        cpix_Hits * (*func)(cpix_IdxSearcher*, const wchar_t*, float, float) 
            = &Cpix::AddressSearch::search;
        
        return XlateExc(idxSearcher,
                        CallFreeFunc(func,
                                     idxSearcher,
                                     qryStr_.c_str(),
                                     refPointGpsLat_,
                                     refPointGpsLong_));
    }
    
    
    cpix_Hits * AddressQryType::search(cpix_IdxDb * )
    {
        THROW_CPIXEXC("Address qry is supported for searchers only");
    }
    
    
    
    IQryType * CreateAddressQryType()
    {
        return new AddressQryType;
    }

} // namespace 
