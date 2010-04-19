/*------------------------------------------------------------------------------
* Copyright (C) 2010 Nokia Corporation and/or its subsidiary(-ies). 
* 
* Distributable under the terms of either the Apache License (Version 2.0) or 
* the GNU Lesser General Public License, as specified in the COPYING file.
------------------------------------------------------------------------------*/

#include <assert.h>

#include <fstream>

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
    assert(corpus_ != NULL);
    assert(corpus_->hasMore());
    
    
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

    assert(hasMore());
}


const char * Corpus::filePath() const
{
    return filePath_.c_str();
}


bool Corpus::hasMore() const
{
    return hasMore_;
}


std::string Corpus::next()
{
    assert(hasMore_);

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

