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
#include <assert.h>
#include <wchar.h>

#include <cmath>
#include <fstream>

#include "cpixstrtools.h"

#include "corpustester.h"

#include "indevicecfg.h"

#define INDEX_DIR "c:\\data\\indexing\\indexdb\\root\\file\\"

#include "itkperf.h"

namespace
{
    class DynMemSampler
    {
        const std::string       name_;
        Itk::TestMgr          * testMgr_;


    public:
        DynMemSampler(const char   * name,
                      Itk::TestMgr * testMgr)
            : name_(name),
              testMgr_(testMgr)
        {
            cpix_dbgResetDynMemUsage();
        }


        ~DynMemSampler()
        {
            ITK_REPORT(testMgr_,
                       name_.c_str(),
                       "Memory : %d",
                       Cpt::getUsedDynamicMemory());
            ITK_REPORT(testMgr_,
                       name_.c_str(),
                       "Peak memory : %d",
                       cpix_dbgPeakDynMemUsage());
        }
    };

}


// Don't change any of these 3 values without updating the others
const char LoopIdPlaceHolder[] =  "000";
const int  LoopIdLength        =      3; // == strlen(LoopIdPlaceHolder);
const char LoopIdFormatStr[]   = "%03d";


template <int N>
struct TenOnThePowerOf
{
    enum { VALUE = 10 * TenOnThePowerOf<N-1>::VALUE };
};

template<>
struct TenOnThePowerOf<0>
{
    enum { VALUE = 1 };
};

CorpusTester::CorpusTester(const char             * corpusPath,
                           int                      numOfPreIndexedItems,
                           const std::string      & runName,
                           int                      bufferSize,
                           int                      maxInputSize,
                           int                      lumpSize,
                           int                      numOfFirstItems,
                           int                      numOfLastItems,
                           const std::list<std::string> & queryTerms,
                           int                      numOfSearches,
                           int                      numOfIncrementalSearches,
                           int                      minIncrementalLength,
                           int                      numOfSuggestionSearches,
                           int                      minSuggestionLength,
                           bool                     takeIdxSnapshots,
                           bool                     addLasts,
                           bool                     delReAddLasts,
                           bool                     updateLasts)
    : corpus_(corpusPath),
      numOfPreIndexedItems_(numOfPreIndexedItems),
      runName_(runName),
      bufferSize_(bufferSize),
      maxInputSize_(maxInputSize),
      lumpSize_(lumpSize),
      numOfFirstItems_(numOfFirstItems),
      numOfLastItems_(numOfLastItems),
      queryTerms_(queryTerms),
      numOfSearches_(numOfSearches),
      numOfIncrementalSearches_(numOfIncrementalSearches),
      minIncrementalLength_(minIncrementalLength),
      numOfSuggestionSearches_(numOfSuggestionSearches),
      minSuggestionLength_(minSuggestionLength),
      takeIdxSnapshots_(takeIdxSnapshots),
      addLasts_(addLasts),
      delReAddLasts_(delReAddLasts),
      updateLasts_(updateLasts),
      testMgr_(NULL),
      idxUtil_(NULL),
      processed_(0),
      indexSize_(0)
{
    int
        absNumOfFirsts = std::abs((long)numOfFirstItems_);

    assert(absNumOfFirsts + numOfLastItems <= lumpSize_);
    assert(lumpSize_ <= maxInputSize);

    assert((maxInputSize_ / lumpSize_) < TenOnThePowerOf<LoopIdLength>::VALUE);

}


void CorpusTester::run(Itk::TestMgr * testMgr,
                       IdxUtil      * idxUtil)
{
    processed_ = 0;

    testMgr_ = testMgr;
    idxUtil_ = idxUtil;

    char
        buffer[256];
    snprintf(buffer,
             sizeof(buffer),
             "(buffer: %d, preidx %d, max %d, lump %d, first %d, last %d, "
             "qterms: %s, searches: %d, incrsearches: %d, mininclength: %d)",
             bufferSize_,
             numOfPreIndexedItems_,
             maxInputSize_,
             lumpSize_,
             numOfFirstItems_,
             numOfLastItems_,
             queryTerms_.empty() ? "{}" : "{...}",
             numOfSearches_,
             numOfIncrementalSearches_,
             minIncrementalLength_);

    ITK_REPORT(testMgr,
               runName_.c_str(),
               buffer);

    it_ = corpus_.begin();

    preIndex();
    runLoop();
}


bool CorpusTester::visitFile(const char * path)
{
    indexSize_ += Cpt::filesize(path);
    return true;
}


Cpt::IFileVisitor::DirVisitResult 
CorpusTester::visitDirPre(const char * /* path */ ) 
{ 
    return IFV_CONTINUE; 
}

bool CorpusTester::visitDirPost(const char * /* path */) 
{ 
    return true; 
}


void CorpusTester::preIndex()
{
    bool
        discard = numOfPreIndexedItems_ < 0;
    int
        count = std::abs((long)numOfPreIndexedItems_);
    CorpusInputIt
        end = corpus_.end();

    std::string
        name(runName_);
    name += "-preidx";
    
    {
        Itk::Timestamper
            ts(runName_.c_str(),
               testMgr_);

        for (; count > 0 && it_ != end; ++it_, --count)
            {
                std::string
                    line = *it_;

                if (!discard)
                    {
                        idxUtil_->indexItem(line.c_str(),
                                            testMgr_);
                    }
            }
        idxUtil_->flush(); 
    }
}


void CorpusTester::runLoop()
{

    std::string
        lumpName(runName_);
    lumpName += "-";
    lumpName += LoopIdPlaceHolder;

    // This looks dangerous, but this should keep itself changing
    // what's already there, never beyond (rendering the running
    // number to the end of the lumpName string).
    //
    // Rationale: this way it is much faster.
    char
        * digits = const_cast<char*>(lumpName.c_str() 
                                     + lumpName.length() 
                                     - LoopIdLength);

    CorpusInputIt
        end = corpus_.end();

    while (it_ != end && ( processed_ < maxInputSize_ ) )
        {
            snprintf(digits,
                     LoopIdLength + 1,
                     LoopIdFormatStr,
                     processed_ / lumpSize_);

            Itk::Timestamper
                ts(lumpName.c_str(),
                   testMgr_);

            runLump(lumpName.c_str() );
            processed_ += lumpSize_;
        }
}


void CorpusTester::runLump(const char * lumpName)
{
    if (numOfFirstItems_ != 0)
        {
            addFirstItems(lumpName);
        }


    addMiddleItems();

    if (numOfLastItems_ > 0)
        {
            std::vector<std::string>
                lastItems;

            lastItems.reserve(numOfLastItems_);

            if (addLasts_)
                {
                    addLastItems(lumpName,
                                 lastItems);
                }

            if (delReAddLasts_)
                {
                    deleteLastItems(lumpName);
                    
                    reAddLastItems(lumpName,
                                   lastItems);
                }

            if (updateLasts_)
                {
                    updateLastItems(lumpName,
                                    lastItems);
                }
            
            flushLastItems(lumpName); 
        }

    if (queryTerms_.size())
        {
            if (numOfSearches_ > 0)
                {
                    search(lumpName);
                }

            if (numOfIncrementalSearches_ > 0)
                {
                    incrementalSearch(lumpName);
                }
            if (numOfSuggestionSearches_ > 0) 
            	{
                    suggestSearch(lumpName);
            	}
        }

    indexSize_ = 0;
    Cpt::traverse(INDEX_DIR, this);

    ITK_REPORT(testMgr_,
               lumpName,
               "Index database size %d",
               indexSize_ );

    if (takeIdxSnapshots_)
        {
            takeIdxSnapshot();
        }
}



void CorpusTester::addFirstItems(const char * lumpName)
{
    std::string
        metric(lumpName);
    metric += "-firsts";

    CorpusInputIt
        end = corpus_.end();
    bool
        discard = numOfFirstItems_ < 0;
    int
        count = std::abs((long)numOfFirstItems_);
        
    {
		Itk::Timestamper
            ts(metric.c_str(),
               testMgr_);
        DynMemSampler
            dms(metric.c_str(),
                testMgr_);

        for (; 
             it_ != end && processed_ < maxInputSize_ && count > 0; 
             --count, ++it_)
            {
                std::string
                    line = *it_;
            
                if (!discard)
                    {
                        idxUtil_->indexItem(line.c_str(),
                                            testMgr_);
                    }
            }
        idxUtil_->flush(); 
    }
}

#include <fcntl.h>
#include <unistd.h>
#include <sstream>
#include <string>

void CorpusTester::addMiddleItems()
{
	using namespace std;

	CorpusInputIt
        end = corpus_.end();
    int
        absNumOfFirsts = std::abs((long)numOfFirstItems_);
    int
        count = lumpSize_ - absNumOfFirsts - numOfLastItems_;
        
    for (; 
         it_ != end && processed_ < maxInputSize_ && count > 0; 
         --count, ++it_)
        {
            std::string
                line = *it_;
            
            idxUtil_->indexItem(line.c_str(),
                                testMgr_);
        }
    idxUtil_->flush(); 
}


void CorpusTester::addLastItems(const char                * lumpName,
                                std::vector<std::string>  & lastItems)
{
    std::string
        metric(lumpName);
    metric += "-lasts";

    CorpusInputIt
        end = corpus_.end();
    int
        count = numOfLastItems_;

    {
		Itk::Timestamper
            ts(metric.c_str(),
               testMgr_);
        DynMemSampler
            dms(metric.c_str(),
                testMgr_);

        for (; 
             it_ != end && processed_ < maxInputSize_ && count > 0; 
             --count, ++it_)
            {
                std::string
                    line = *it_;
            
                idxUtil_->indexItem(line.c_str(),
                                    testMgr_);
                lastItems.push_back(line);
            }
    }
}


void CorpusTester::deleteLastItems(const char * lumpName)
{
    std::string
        metric(lumpName);
    metric += "-lasts-del";

    int
        endIdx = idxUtil_->curIdx();
    int
        idx = endIdx - numOfLastItems_;


    {
		Itk::Timestamper
            ts(metric.c_str(),
               testMgr_);
        DynMemSampler
            dms(metric.c_str(),
                testMgr_);
               
        for (; idx < endIdx; ++idx)
            {
                std::wstring
                    idStr = id2Wstr(idx);

                int32_t
                    result = cpix_IdxDb_deleteDocuments(idxUtil_->idxDb(),
                                                        idStr.c_str());
            
                ITK_ASSERT(testMgr_,
                           cpix_Succeeded(idxUtil_->idxDb()),
                           "Failed to delete document %d (%s)",
                           idx,
                           lumpName);
                ITK_EXPECT(testMgr_,
                           result == 1,
                           "Deleted %d items instead of 1 (%s)",
                           result,
                           lumpName);
            }
    }
}


void CorpusTester::reAddLastItems(const char                      * lumpName,
                                  const std::vector<std::string>  & lastItems)
{
    std::string
        metric(lumpName);
    metric += "-lasts-re";
    
    std::vector<std::string>::const_iterator
        it = lastItems.begin(),
        end = lastItems.end();
    int
        count = numOfLastItems_;
    idxUtil_->setCurIdx(idxUtil_->curIdx() - numOfLastItems_);
        
    {
		Itk::Timestamper
            ts(metric.c_str(),
               testMgr_);
        DynMemSampler
            dms(metric.c_str(),
                testMgr_);
        
        for (; it != end && processed_ < maxInputSize_ && count > 0; 
             ++it, --count)
            {
                std::string
                    line = *it;
            
                idxUtil_->indexItem(line.c_str(),
                                    testMgr_);
            }
    }
}


void CorpusTester::updateLastItems(const char                      * lumpName,
                                   const std::vector<std::string>  & lastItems)
{
    std::string
        metric(lumpName);
    metric += "-lasts-upd";

    std::vector<std::string>::const_iterator
        it = lastItems.begin(),
        end = lastItems.end();
    int
        count = numOfLastItems_;
    idxUtil_->setCurIdx(idxUtil_->curIdx() - numOfLastItems_);

    {
		Itk::Timestamper
            ts(metric.c_str(),
               testMgr_);
        DynMemSampler
            dms(metric.c_str(),
                testMgr_);

        for (; it != end && processed_ < maxInputSize_ && count > 0; 
             ++it, --count)
            {
                std::string
                    line = *it;
            
                idxUtil_->updateItem(line.c_str(),
                                     testMgr_);
            }
    }
}

void CorpusTester::flushLastItems(const char* lumpName) 
{
	std::string
		metric(lumpName);
	metric += "-lasts-flush";
	{
		Itk::Timestamper
			ts(metric.c_str(),
			   testMgr_);
		DynMemSampler
			dms(metric.c_str(),
				testMgr_);
		idxUtil_->flush(); 
	}
}

void CorpusTester::search(const char * lumpName)
{
    std::list<std::string>::const_iterator
        end = queryTerms_.end(),
        i = queryTerms_.begin();

    // for each term
    for (; i != end; ++i)
        {
            std::string
                metric(lumpName);
            metric += "-search-";
            metric += *i;
           
            Cpt::auto_array<wchar_t>
                wterm(new wchar_t[i->length() + 1]);
            mbstowcs(wterm.get(),
                     i->c_str(),
                     i->length() + 1);
     
            cpix_Query
                * query = cpix_QueryParser_parse(idxUtil_->queryParser(),
                                                 wterm.get());
            ITK_ASSERT(testMgr_,
                       cpix_Succeeded(idxUtil_->queryParser()) 
                       && query != NULL,
                       "Could not create query (%s)",
                       lumpName);

            doQuery(metric.c_str(),
                    query);

            cpix_Query_destroy(query);
        }
}


void CorpusTester::incrementalSearch(const char * lumpName)
{
    std::list<std::string>::const_iterator
        end = queryTerms_.end(),
        i = queryTerms_.begin();

    // for each term
    for (; i != end; ++i)
        {
            std::string
                metricBase(lumpName);
            metricBase += "-insearch-";

            size_t
                termLength = i->length();

            // for each letter in a term
            for (size_t curLength = minIncrementalLength_;
                 curLength < termLength; 
                 ++curLength)
                {
                    std::string
                        metric(metricBase);
                    metric += i->substr(0,
                                        curLength);
                    metric += '*';

                    Cpt::auto_array<wchar_t>
                        wterm(new wchar_t[curLength + 2]);
                    mbstowcs(wterm.get(),
                             i->c_str(),
                             curLength + 2);
                    wterm.get()[curLength] = L'*';
                    wterm.get()[curLength + 1] = L'\0';
     
                    cpix_Query
                        * query = cpix_QueryParser_parse(idxUtil_->queryParser(),
                                                         wterm.get());
                    ITK_ASSERT(testMgr_,
                               cpix_Succeeded(idxUtil_->queryParser()) 
                               && query != NULL,
                               "Could not create query for %S (%s)",
                               wterm.get(),
                               lumpName);

                    doQuery(metric.c_str(),
                            query);

                    cpix_Query_destroy(query);
                }
        }
}



void CorpusTester::suggestSearch(const char * lumpName)
{
    std::list<std::string>::const_iterator
        end = queryTerms_.end(),
        i = queryTerms_.begin();

    // for each term
    for (; i != end; ++i)
        {
            std::string
                metricBase(lumpName);
            metricBase += "-suggest-";

            size_t
                termLength = i->length();

            // for each letter in a term
            for (size_t curLength = minSuggestionLength_;
                 curLength < termLength; 
                 ++curLength)
                {
                    std::string
                        metric(metricBase);
                    metric += i->substr(0,
                                        curLength);
                    metric += '*';

                    Cpt::auto_array<wchar_t>
                        wterm(new wchar_t[curLength + 2]);
                    mbstowcs(wterm.get(),
                             i->c_str(),
                             curLength + 2);
                    wterm.get()[curLength] = L'*';
                    wterm.get()[curLength + 1] = L'\0';

                    std::wstring
                        wTermsQryStr(L"$terms<10>(");
                    wTermsQryStr += wterm.get();
                    wTermsQryStr += L")";

                    cpix_Query
                        * query = cpix_QueryParser_parse(idxUtil_->queryParser(),
                                                         // OBS wterm.get());
                                                         wTermsQryStr.c_str());
                    ITK_ASSERT(testMgr_,
                               cpix_Succeeded(idxUtil_->queryParser()) 
                               && query != NULL,
                               "Could not create query for %S (%s)",
                               // OBS wterm.get(),
                               wTermsQryStr.c_str(),
                               lumpName);

                    doSuggestQuery(metric.c_str(),
                                   // OBS LCPIX_DEFAULT_FIELD,
                                   query);

                    cpix_Query_destroy(query);
                }
        }
}



void CorpusTester::doQuery(const char * metricName,
                           cpix_Query * query)
{
	Itk::Timestamper
        ts(metricName,
           testMgr_);
    DynMemSampler
        dms(metricName,
            testMgr_);

    bool
        goOn = true;

    for (int rounds = numOfSearches_; rounds > 0 && goOn; --rounds)
        {
            /* OBS
            cpix_Hits
                * hits = cpix_IdxDb_search(idxUtil_->idxDb(),
                                           query);
            */
            cpix_Hits
                * hits = cpix_IdxSearcher_search(idxUtil_->idxSearcher(),
                                                 query);

            // OBS if (cpix_Succeeded(idxUtil_->idxDb()))
            if (cpix_Succeeded(idxUtil_->idxSearcher()))
                {
                    if (rounds == numOfSearches_)
                        {
                            int32_t
                                hitCount = cpix_Hits_length(hits);
                            ITK_REPORT(testMgr_,
                                       metricName,
                                       "hitcount: %d",
                                       hitCount);
                        }
                    cpix_Hits_destroy(hits);
                }
            else
                {
                    goOn = false;
                    wchar_t
                        report[256];
                    cpix_Error_report(hits->err_,
                                      report,
                                      sizeof(report)/sizeof(wchar_t));
                    ITK_EXPECT(testMgr_,
                               false,
                               "Search failure: %S (%s)",
                               report,
                               metricName);
                }
        }

    ITK_EXPECT(testMgr_,
               goOn,
               "Could not search index (%s)",
               metricName);
}


void CorpusTester::doSuggestQuery(const char    * metricName,
                                  // OBS const wchar_t * fieldName,
                                  // OBS const wchar_t * wildcard)
                                  cpix_Query    * termsQry)
{
	Itk::Timestamper
        ts(metricName,
           testMgr_);
    DynMemSampler
        dms(metricName,
            testMgr_);

    bool
        goOn = true;

    for (int rounds = numOfSuggestionSearches_; rounds > 0 && goOn; --rounds)
        {
            // getting terms (suggestions) using unified search API
            /* OBS
            cpix_Hits
                * hits = cpix_IdxDb_search(idxUtil_->idxDb(),
                                           termsQry);
            */
            cpix_Hits
                * hits = cpix_IdxSearcher_search(idxUtil_->idxSearcher(),
                                                 termsQry);

            // OBS if (cpix_Succeeded(idxUtil_->idxDb()))
            if (cpix_Succeeded(idxUtil_->idxSearcher()))
                {
                    if (rounds == numOfSuggestionSearches_)
                        {
                            int32_t
                                hitCount = cpix_Hits_length(hits);
                            ITK_REPORT(testMgr_,
                                       metricName,
                                       "hitcount: %d",
                                       hitCount);
                        }
                    cpix_Hits_destroy(hits);
                }
            else
                {
                    goOn = false;
                    wchar_t
                        report[256];
                    cpix_Error_report(hits->err_,
                                      report,
                                      sizeof(report)/sizeof(wchar_t));
                    ITK_EXPECT(testMgr_,
                               false,
                               "Suggest failure: %S (%s)",
                               report,
                               metricName);
                }
        }

    ITK_EXPECT(testMgr_,
               goOn,
               "Could not search index for suggestions (%s)",
               metricName);
}


// from perfmetrics.cpp
extern const char BaseDir[];

void CorpusTester::takeIdxSnapshot()
{
    int
        lump = processed_ / lumpSize_;

    char
        lumpStr[LoopIdLength + 1];
    snprintf(lumpStr,
             sizeof(lumpStr),
             LoopIdFormatStr,
             lump);

    std::string
        dstDir(BaseDir);
    dstDir += lumpStr;
    dstDir += '\\';

    int 
        result = Cpt::copy(dstDir.c_str(),
                           idxUtil_->idxDbPath(),
                           false); // exclusive: don't copy src dir,
                                   // only its content

    ITK_ASSERT(testMgr_,
               result == Cpt::CPT_CP_OK,
               "Taking snapshot of index database (%s -> %s) failed: %d",
               idxUtil_->idxDbPath(),
               dstDir.c_str(),
               result);
}
