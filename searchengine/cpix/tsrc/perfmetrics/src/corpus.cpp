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

// OBS #include <assert.h>

#include <fstream>

#include "itk.h"

#include "corpus.h"


CorpusInputIt & CorpusInputIt::operator=(const CorpusInputIt & that)
{
    if (this != &that)
        {
            corpus_ = that.corpus_;
        }

    return *this;
}


CorpusInputIt &CorpusInputIt::operator++()
{
    if (corpus_ && !corpus_->hasMore())
        {
            corpus_ = NULL;
        }

    return *this;
}


CorpusInputIt  CorpusInputIt::operator++(int)
{
    CorpusInputIt
        rv = *this;

    ++(*this);

    return rv;
}


CorpusInputIt::reference CorpusInputIt::operator*()
{
    // OBS assert(corpus_ != NULL);
    if (corpus_ == NULL)
        {
            ITK_PANIC("corpus_ is NULL");
        }

    // OBS assert(corpus_->hasMore());
    if (!corpus_->hasMore())
        {
            ITK_PANIC("corpus has no more input");
        }
    
    
    curLine_ = corpus_->next();

    if (!corpus_->hasMore())
        {
            corpus_ = NULL;
        }

    return curLine_;
}

CorpusInputIt::CorpusInputIt()
    : corpus_(NULL)
{
    ;
}

    
CorpusInputIt::CorpusInputIt(const CorpusInputIt & that)
    : corpus_(that.corpus_)
{
    ;
}


    
CorpusInputIt::CorpusInputIt(Corpus * owner)
    : corpus_(owner)
{
    ;
}


bool operator==(const CorpusInputIt & left,
                const CorpusInputIt & right)
{
    return left.corpus_ == right.corpus_;
}


bool operator!=(const CorpusInputIt & left,
                const CorpusInputIt & right)
{
    return !(left == right);
}



CorpusInputIt Corpus::begin()
{
    return CorpusInputIt(this);
}
    

CorpusInputIt Corpus::end()
{
    return CorpusInputIt();
}


Corpus::Corpus(const char * filePath)
    : filePath_(filePath),
      ifs_(filePath),
      hasMore_(true)
{
    readNext();
}


bool Corpus::hasMore() const
{
    return hasMore_;
}


std::string Corpus::next()
{
    // OBS assert(hasMore_);
    if (!hasMore_)
        {
            ITK_PANIC("Corpus::next has no more input");
        }

    std::string
        rv = curLine_;

    readNext();

    return rv;
}


void Corpus::readNext()
{
    if (ifs_.is_open())
        {
            if (!getline(ifs_, curLine_))
                {
                    hasMore_ = false;
                    ifs_.close();
                }
        }
    else
        {
            hasMore_ = false;
        }
}

