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

#include "ifieldfilter.h"
#include "cpixexc.h"
#include "indevicecfg.h"

// filters:
#include "fileparser/fileparser.h"
#include "filters/quadfilter.h"


namespace Cpix
{


    /******
     * IFieldFilter implementation
     */
    IFieldFilter::~IFieldFilter()
    {
        ;
    }

    /*********
     * FieldFilterPool implementation
     */
    FieldFilterPool * FieldFilterPool::instance_ = NULL;

    IFieldFilter * FieldFilterPool::getFieldFilter(const wchar_t * ffid)
    {
        return instance()->getFieldFilter_(ffid);
    }


    void FieldFilterPool::releaseFieldFilter(IFieldFilter * ff)
    {
        instance()->releaseFieldFilter_(ff);
    }


    void FieldFilterPool::init()
    {
        // discard the return value - it's only to bring the singleton
        // instance to life
        instance();
    }


    void FieldFilterPool::cleanup()
    {
        instance()->cleanup_();
        delete instance_;
        instance_ = NULL; 
    }

    FieldFilterPool::FieldFilterPool()
    {
        ;
    }


    FieldFilterPool * FieldFilterPool::instance()
    {
        if (instance_ == NULL)
            {
                instance_ = new FieldFilterPool();
            }
        return instance_;
    }


    IFieldFilter * 
    FieldFilterPool::getFieldFilter_(const wchar_t * ffid)
    {
        IFieldFilter
            * rv = NULL;

        std::wstring
            wffid(ffid);

        Cpt::SyncRegion
            sr(mutex_);

        if (filters_.find(wffid) == filters_.end())
            {
                rv = createFieldFilter(ffid);
                filters_[wffid] = rv;
            }
        else
            {
                rv = filters_[wffid];
            }
                
        return rv;
    }


    void FieldFilterPool::releaseFieldFilter_(IFieldFilter * ff)
    {
        ; // NOP (for the time being);
    }


    void FieldFilterPool::cleanup_()
    {
        using namespace std;

        Cpt::SyncRegion
            sr(mutex_);

        FilterMap::iterator
            i = filters_.begin(),
            end = filters_.end();

        for (; i != end; ++i)
            {
                delete i->second;
            }

        filters_.clear();
    }


    IFieldFilter * 
    FieldFilterPool::createFieldFilter(const wchar_t * ffid)
    {
        IFieldFilter
            * rv = NULL;

        if (wcscmp(LCPIX_FILEPARSER_FID, ffid) == 0)
            {
                rv = new FileParser();
            }
        else if (wcscmp(LCPIX_QUADFILTER_FID, ffid) == 0)
            {
                rv = new QuadFilter();
            }
        else
            {
                THROW_CPIXEXC("Unknown filter id '%S'",
                              ffid);
            }

        return rv;
    }


    
    /***********
     *
     * FieldFieldSentry implementation
     */
    FieldFilterSentry::FieldFilterSentry(IFieldFilter * ff)
        : fieldFilter_(ff)
    {
        ;
    }


    FieldFilterSentry::~FieldFilterSentry()
    {
        FieldFilterPool::releaseFieldFilter(fieldFilter_);
    }


    IFieldFilter * FieldFilterSentry::get()
    {
        return fieldFilter_;
    }


}

