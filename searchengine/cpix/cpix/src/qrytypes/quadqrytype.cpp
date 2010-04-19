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

#include <set>

#include "CLucene.h"
#include "CLucene/queryParser/MultiFieldQueryParser.h"

#include "indevicecfg.h"

#include "cpixtools.h"

#include "cpixhits.h"
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
    
    class QuadQryType : public IQryType
    {
    private:
        //
        // private members
        //
        float               radiusKm_;
        int                 maxHitCount_;
        double              refPointGpsLat_;
        double              refPointGpsLong_;
        std::list<Cpt::QNr> centerQNrs_;
        std::wstring        qryStr_;
        cpix_QueryParser  * queryParser_;

        cpix_Query        * innerQuery_;
        

    public:
        QuadQryType()
            : radiusKm_(0.0),
              maxHitCount_(0),
              refPointGpsLat_(0.0),
              refPointGpsLong_(0.0),
              queryParser_(NULL),
              innerQuery_(NULL)
        {
            ;
        }


        ~QuadQryType()
        {
            cpix_Query_destroy(innerQuery_);
            innerQuery_ = NULL;
        }


        //
        // from interface IQryType
        //
        virtual void setUp(cpix_QueryParser              * queryParser,
                           const std::list<std::wstring> & args,
                           const wchar_t                 * qryStr)
        {
            if (args.size() < 6)
                {
                    THROW_CPIXEXC("Too few arguments for quad search");
                }
            else if ((args.size() % 2) != 0)
                {
                    THROW_CPIXEXC("Last GPS coordinate pair broken - only latitude given");
                }

            using namespace std;

            list<wstring>::const_iterator
                i = args.begin(),
                end = args.end();

            wconvertThrowing(&radiusKm_,
                             i->c_str());
            ++i;
                         
            wconvertThrowing(&maxHitCount_,
                             i->c_str());
            ++i;

            wconvertThrowing(&refPointGpsLat_,
                             i->c_str());
            ++i;
            
            wconvertThrowing(&refPointGpsLong_,
                             i->c_str());
            ++i;
            
            while (i != end)
                {
                    double 
                        gpsLat = 0.0,
                        gpsLong  = 0.0;

                    wconvertThrowing(&gpsLat,
                                     i->c_str());
                    ++i;

                    wconvertThrowing(&gpsLong,
                                     i->c_str());
                    ++i;

                    Cpt::QNr
                        qnr(gpsLat,
                            gpsLong);
                    
                    centerQNrs_.push_back(qnr);
                }

            qryStr_ = qryStr;

            queryParser_ = queryParser;
        }


        virtual cpix_Hits * search(cpix_IdxSearcher * idxSearcher)
        {
            return searchIDX(idxSearcher,
                             &cpix_IdxSearcher_search);
        }


        virtual cpix_Hits * search(cpix_IdxDb * idxDb)
        {
            return searchIDX(idxDb,
                             &cpix_IdxDb_search);
        }
        

    private:
        //
        // private members
        //

        // TODO sorting based on GPS distance from a reference point
        template<typename IDX>
        cpix_Hits * searchIDXUnsorted(IDX         * idx,
                                      cpix_Hits* (* searchFunc)(IDX*, cpix_Query*))
        {
            using namespace Cpt;
            using namespace std;

            double
                curRadius = 0.0;
            int32_t
                hitCount = 0;
            cpix_Hits
                * hits = NULL;
            
            while (curRadius < radiusKm_ && hitCount < maxHitCount_)
                {
                    set<QNr>
                        targetQNrs;

                    cpix_Hits_destroy(hits);
                    hits = NULL;
                    cpix_Query_destroy(innerQuery_);
                    innerQuery_ = NULL;

                    // getting adjacents for all center QNr-s
                    for (list<QNr>::const_iterator it = centerQNrs_.begin();
                         it != centerQNrs_.end();
                         ++it)
                        {
                            // set targetQNrs will contain the centers too
                            it->getAdjacents(targetQNrs);
                        }
                
                    // go through all QNrs and compute curRadius (as the
                    // minimum of all widths/heights) and query string
                    curRadius = 40075.0; // guaranteed to be bigger than any
                    wstring
                        qryStr(QNR_FIELD L":(");
                    for (set<QNr>::const_iterator it = targetQNrs.begin();
                         it != targetQNrs.end();
                         ++it)
                        {
                            curRadius = min(min(it->getKmWidth(),
                                                it->getKmHeight()),
                                            curRadius);
                            qryStr += it->toWString();
                            qryStr += L' ';
                        }

                    if (qryStr_.length() > 0)
                        {
                            qryStr += L") AND (";
                            qryStr += qryStr_;
                        }
                    qryStr += L")";

                    innerQuery_ = cpix_QueryParser_parse(queryParser_,
                                                         qryStr.c_str());
                    if (cpix_Failed(queryParser_))
                        {
                            // "re-throw a la C": transfer error from
                            // queryParser_ to idx
                            idx->err_ = queryParser_->err_;
                            return NULL;
                        }
                    
                    // innerQuery_ must be kept around
                    hits = (*searchFunc)(idx,
                                         innerQuery_);

                    if (cpix_Failed(idx))
                        {
                            // "re-throw": error is already on idx
                            return NULL;
                        }

                    hitCount = cpix_Hits_length(hits);

                    // if there will be a next loop, we remove the center
                    // QNrs and replace them with their parents
                    if (curRadius < radiusKm_ && hitCount < maxHitCount_)
                        {
                            for (size_t i = centerQNrs_.size(); i > 0; --i)
                                {
                                    QNr
                                        center = centerQNrs_.front();
                                    QNr
                                        parent = center.getParentQNr();

                                    centerQNrs_.push_back(parent);
                                    centerQNrs_.pop_front();
                                }
                        }
                }

            return hits;
        }


        template<typename IDX>
        cpix_Hits * searchIDX(IDX         * idx,
                              cpix_Hits* (* searchFunc)(IDX*, cpix_Query*))
        {
            cpix_Hits
                * rv = searchIDXUnsorted(idx,
                                         searchFunc);

            if (cpix_Succeeded(idx))
                {
                    try
                        {
                            cpix_Result
                                result;

                            cpix_Hits
                                * sortedHits = gpsSort(refPointGpsLat_,
                                                       refPointGpsLong_,
                                                       rv,
                                                       &result);
                            
                            if (cpix_Succeeded(idx))
                                {
                                    cpix_Hits_destroy(rv);
                                    rv = sortedHits;
                                }
                            else
                                {
                                    idx->err_ = result.err_;
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

    };



    IQryType * CreateQuadQryType()
    {
        return new QuadQryType;
    }
}
