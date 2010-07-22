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

#include <wchar.h>
#include <stddef.h>

#include <memory>
#include <iostream>
#include <list>
#include <map>
#include <vector>

#include "cpixfstools.h"

#include "itk.h"

#include "cpixidxdb.h"
#include "cpixsearch.h"
#include "idxdbmgr.h" // from private folder: whitebox testing

#include "config.h"
#include "suggestion.h"
#include "testutils.h"
#include "testcorpus.h"
#include "setupsentry.h"

#include "std_log_result.h"

struct MatchTest
{
    const char * domainSelector_;
    const char * baseAppClass_;
};



void testDomainSelection(Itk::TestMgr    * testMgr,
                         const MatchTest * matchTest,
                         bool              shouldMatch)
{
    for (; matchTest->domainSelector_ != NULL; ++matchTest)
        {
            bool
                matched =  Cpix::IdxDbMgr::match(matchTest->domainSelector_,
                                                 matchTest->baseAppClass_);
            ITK_EXPECT(testMgr,
                       matched == shouldMatch,
                       "Domain selector '%s' and baseappclass: '%s' "
                       "should %s match",
                       matchTest->domainSelector_,
                       matchTest->baseAppClass_,
                       (shouldMatch ? "" : "NOT"));
        }
}



const MatchTest PositiveMatchTests[] = {
    // multiple volume cases
    { "root file",                       "@c:root file" },
    { "root file",                       "@d:root file" },
    { "@c:root file",                    "@c:root file" },
    { "@d:root file",                    "@d:root file" },

    // generic/widening search cases
    { "root",                            "@c:root file" },
    { "root",                            "@d:root file" },
    { "root",                            "@0:root msg phone sms" },
    { "root msg",                        "@0:root msg phone sms" },
    { "@c:root",                         "@c:root file" },
    { "@d:root",                         "@d:root file" },

    // selecting a list of volumes to use
    { "@FIN:root maps,@GBR:root maps",   "@FIN:root maps" },
    { "@FIN:root maps,@GBR:root maps",   "@GBR:root maps" },
    { "root file,root msg phone sms",    "@c:root file" },
    { "root file,root msg phone sms",    "@d:root file" },
    { "root file,root msg phone sms",    "@0:root msg phone sms" },
    { "@c:root file,root msg phone sms", "@c:root file" },
    { "@c:root file,root msg phone sms", "@0:root msg phone sms" },

    // end
    { NULL,                              NULL }
};


void testPositiveDomainSelection(Itk::TestMgr * testMgr)
{
    testDomainSelection(testMgr,
                        PositiveMatchTests,
                        true); // should match
}


const MatchTest NegativeMatchTests[] = {
    { "root file",                       "@0:root msg phone sms" },
    { "@c:root file",                    "@0:root msg phone sms" },
    { "root file",                       "@FIN:root maps" },
    { "@c:root file",                    "@FIN:root maps" },

    // end
    { NULL,                              NULL }
};


void testNegativeDomainSelection(Itk::TestMgr * testMgr)
{
    testDomainSelection(testMgr,
                        NegativeMatchTests,
                        false); // should NOT match
}



class DomainSelectionContext : public Itk::ITestContext
{
    cpix_Analyzer              * analyzer_;
    cpix_Query                 * query_;
    cpix_Query                 * termsQuery_;
    cpix_Query                 * dumpQuery_;

    cpix_QueryParser           * queryParser_;

    std::list<cpix_IdxSearcher*> searchers_;

    static const char          * domainSpecifiers_[];
    static const MVFTest         filesAndVols_[];

public:
    virtual void setup() throw (Itk::PanicExc)
    {
        SetupSentry
            ss(*this);

        cpix_Result
            result;

        cpix_IdxDb_dbgScrapAll(&result);
        if (cpix_Failed(&result))
            {
                cpix_LogLevel
                    ll = cpix_setLogLevel(CPIX_LL_TRACE);
                cpix_dbgDumpState();
                cpix_setLogLevel(ll);
                ITK_PANIC("Could not dbg scrapp all indexes");
            }

        analyzer_ = cpix_CreateSimpleAnalyzer(&result);
        if (analyzer_ == NULL)
            {
                ITK_PANIC("Could not create analyzer");
            }


        queryParser_ = cpix_QueryParser_create(&result,
                                               LCPIX_DEFAULT_FIELD,
                                               analyzer_);
        if (queryParser_ == NULL)
            {
                ITK_PANIC("Could not create query parser");
            }

        query_ = cpix_QueryParser_parse(queryParser_,
                                        L"happ*");
        if (cpix_Failed(queryParser_))
            {
                ITK_PANIC("Could not create query parser");
            }

        // terms (suggestions) query using unified search API
        termsQuery_ = cpix_QueryParser_parse(queryParser_,
                                             L"$terms<50>(ha*)");

        if (cpix_Failed(queryParser_)
            || termsQuery_ == NULL)
            {
                ITK_PANIC("Could not parse terms query string");
            }

        dumpQuery_ = cpix_QueryParser_parse(queryParser_,
                                            L"*");

        if (cpix_Failed(queryParser_))
            {
                ITK_PANIC("Could not parse terms query string");
            }

        ss.setupComplete();
    }


    virtual void tearDown() throw()
    {
        cleanup();
    }


    DomainSelectionContext()
        : analyzer_(NULL),
          query_(NULL),
          termsQuery_(NULL),
          dumpQuery_(NULL),
          queryParser_(NULL)
    {
        ;
    }


    virtual ~DomainSelectionContext()
    {
        cleanup();
    }


    void testCreateIdxs(Itk::TestMgr * testMgr)
    {
        char *xml_file = (char *)__FUNCTION__;
        assert_failed = 0;
        const MVFTest
            * mvfTest = filesAndVols_;

        for (; mvfTest->qualifiedBaseAppClass_ != NULL; ++mvfTest)
            {
                using namespace std;

                printf("Creating volume %s ...\n",
                       mvfTest->qualifiedBaseAppClass_);

                {
                    auto_ptr<VolumeFileIdxUtil>
                        vfiUtil(new VolumeFileIdxUtil(mvfTest));
                    vfiUtil->init();

                    vfiUtil->indexFile(mvfTest->textFilePath_,
                                       analyzer_,
                                       testMgr);
                }

                printf("... created volume %s\n",
                       mvfTest->qualifiedBaseAppClass_);
            }

        printf("Indexing some SMS-es\n");

        LineTestCorpusRef
            corpus(DEFAULT_TEST_CORPUS_PATH);
        std::auto_ptr<SmsIdxUtil>
            siUtil(new SmsIdxUtil);
        siUtil->init();
        

        for (size_t i = 0; i < 200; ++i)
            {
                std::wstring
                    body = corpus.item(i);
                siUtil->indexSms(i,
                               body.c_str(),
                               analyzer_,
                               testMgr);
                if ((i % 5) == 0)
                    {
                        ITK_DBGMSG(testMgr,
                                   ".");
                    }
            }
        siUtil->flush();

        searchAll(testMgr);
        suggestAll(testMgr);
        dumpAll(testMgr);
        testResultXml(xml_file);
    }

    
    void unmountC(Itk::TestMgr * testMgr)
    {
        unmount(filesAndVols_[0].qualifiedBaseAppClass_);
        
        searchAll(testMgr);
        suggestAll(testMgr);
    }


    void unmountE(Itk::TestMgr * testMgr)
    {
        unmount(filesAndVols_[2].qualifiedBaseAppClass_);

        searchAll(testMgr);
        suggestAll(testMgr);
    }


    void unmountD(Itk::TestMgr * testMgr)
    {
        unmount(filesAndVols_[1].qualifiedBaseAppClass_);

        searchAll(testMgr);
        suggestAll(testMgr);
    }


    void mountC(Itk::TestMgr * testMgr)
    {
        mount(testMgr,
              filesAndVols_[0].qualifiedBaseAppClass_,
              filesAndVols_[0].idxDbPath_);

        searchAll(testMgr);
        suggestAll(testMgr);
    }

    void mountE(Itk::TestMgr * testMgr)
    {
        mount(testMgr,
              filesAndVols_[2].qualifiedBaseAppClass_,
              filesAndVols_[2].idxDbPath_);

        searchAll(testMgr);
        suggestAll(testMgr);
    }
               

    void mountD(Itk::TestMgr * testMgr)
    {
        mount(testMgr,
              filesAndVols_[1].qualifiedBaseAppClass_,
              filesAndVols_[1].idxDbPath_);

        searchAll(testMgr);
        suggestAll(testMgr);
    }


private:

    
    void unmount(const char   * baseAppClass)
    {
        printf("Unmounting volume %s ...\n",
               baseAppClass);
        cpix_IdxDb_undefineVolume(baseAppClass);
        printf("... unmounted volume %s.\n",
               baseAppClass);
    }


    void mount(Itk::TestMgr * testMgr,
               const char   * baseAppClass,
               const char   * idxDbPath)
    {
        printf("mounting volume %s/%s ...\n",
               baseAppClass,
               idxDbPath);
        cpix_Result
            result;
        cpix_IdxDb_defineVolume(&result,
                                baseAppClass,
                                idxDbPath);
        ITK_ASSERT(testMgr,
                   cpix_Succeeded(&result),
                   "Failed to mount volume %s/%s",
                   baseAppClass,
                   idxDbPath);
        printf("... mounted volume %s/%s.\n",
               baseAppClass,
               idxDbPath);
    }


    void createSearchersIfNecessary(Itk::TestMgr * testMgr)
    {
        if (searchers_.size() > 0)
            {
                return;
            }

        const char
            * * domainSpecifierPtr = domainSpecifiers_;
        for (; *domainSpecifierPtr != NULL; ++domainSpecifierPtr)
            {
                cpix_Result
                    result;

                cpix_IdxSearcher
                    * searcher = cpix_IdxSearcher_openDb(&result,
                                                         *domainSpecifierPtr);

                ITK_ASSERT(testMgr,
                           cpix_Succeeded(&result),
                           "Could not create searcher for '%s'",
                           *domainSpecifierPtr);
                searchers_.push_back(searcher);
            }
    }

    
    void unifiedSearchAll(Itk::TestMgr * testMgr,
                          cpix_Query   * query,
                          const char   * msg,
                          void        (* printer)(cpix_Hits*, Itk::TestMgr*))
    {
        createSearchersIfNecessary(testMgr);

        printf("\n\n%s\n",
               msg);

        size_t
            idx = 0;

        std::list<cpix_IdxSearcher*>::iterator
            i = searchers_.begin(),
            end = searchers_.end();
        for (; i != end; ++i)
            {
                printf("Searching with domain selector '%s':\n",
                       domainSpecifiers_[idx]);

                cpix_Hits
                    * hits = cpix_IdxSearcher_search(*i,
                                                     query);
                ITK_EXPECT(testMgr,
                           cpix_Succeeded(*i),
                           "Could not search idx searcher (%s)",
                           domainSpecifiers_[idx]);
                if (cpix_Succeeded(*i))
                    {
                        /* 
                        PrintHits(hits,
                                  testMgr);
                        */
                        printer(hits,
                                testMgr);
                    }
                cpix_Hits_destroy(hits);
                printf("\n");

                ++idx;
            }
    }
    


    void searchAll(Itk::TestMgr * testMgr)
    { 
       unifiedSearchAll(testMgr,
                         query_,
                         "And now searching with all searchers",
                         &PrintHits);
    }


    void suggestAll(Itk::TestMgr * testMgr)
    {
        unifiedSearchAll(testMgr,
                         termsQuery_,
                         "And now suggesting for 'ha*' with all searchers",
                         &Suggestion::printSuggestions);
    }


    void dumpAll(Itk::TestMgr * testMgr)
    {
        unifiedSearchAll(testMgr,
                         dumpQuery_,
                         "And now dumping with all searchers",
                         &PrintHits);
    }


    void cleanup()
    {
        cpix_Analyzer_destroy(analyzer_);
        analyzer_ = NULL;

        cpix_Query_destroy(query_);
        query_ = NULL;

        cpix_Query_destroy(termsQuery_);
        termsQuery_ = NULL;

        cpix_Query_destroy(dumpQuery_);
        dumpQuery_ = NULL;

        cpix_QueryParser_destroy(queryParser_);
        queryParser_ = NULL;

        std::list<cpix_IdxSearcher*>::iterator
            i = searchers_.begin(),
            end = searchers_.end();
        for (; i != end; ++i)
            {
                cpix_IdxSearcher_releaseDb(*i);
            }
        searchers_.clear();
    }
};


const MVFTest DomainSelectionContext::filesAndVols_[] = {
    {
        FILE_TEST_CORPUS_PATH "\\en\\1.txt",
        "@c:root file",
        CPIX_TEST_INDEVICE_INDEXDB_PHMEM CPIX_FILE_IDXDB "_\\c",
    },

    {
        FILE_TEST_CORPUS_PATH "\\en\\2.txt",
        "@d:root file",
        CPIX_TEST_INDEVICE_INDEXDB_PHMEM CPIX_FILE_IDXDB "_\\d",
    },

    {
        FILE_TEST_CORPUS_PATH "\\en\\3.txt",
        "@e:root file",
        CPIX_TEST_INDEVICE_INDEXDB_PHMEM CPIX_FILE_IDXDB "_\\e",
    },

    {
        FILE_TEST_CORPUS_PATH "\\en\\4.txt",
        "@f:root file",
        CPIX_TEST_INDEVICE_INDEXDB_PHMEM CPIX_FILE_IDXDB "_\\f",
    },

    {
        NULL,
        NULL,
        NULL
    },
};


const char * DomainSelectionContext::domainSpecifiers_[] = {
    "root",
    "root msg",
    "@d:root file,root msg phone sms",
    "@d:root file,@f:root file",
    NULL
};




// TODO implement actual test cases for generic / widening search
// (defining volumes and other stuff and searching them) 

// TODO implement actual test cases for selecting list of app classes
// to use





Itk::TesterBase * CreateDomainSelectionTests()
{
    using namespace Itk;

    SuiteTester
        * domainTester = new SuiteTester("domain");

    // some white box test testing matcing logic

    SuiteTester
        * whiteBox = new SuiteTester("whitebox");

#define TEST "positive"
    whiteBox->add(TEST,
                  &testPositiveDomainSelection);
#undef TEST

#define TEST "negative"
    whiteBox->add(TEST,
                  &testNegativeDomainSelection);
#undef TEST


    domainTester->add(whiteBox);

    // some blackbox tests doing idx manipulation & searches
    
    DomainSelectionContext
        * domainSelectionContext = new DomainSelectionContext;
    ContextTester
        * contextTester = new ContextTester("selection",
                                            domainSelectionContext);

#define TEST "createIdxs"
    contextTester->add(TEST,
                       domainSelectionContext,
                       &DomainSelectionContext::testCreateIdxs,
                       TEST);
#undef TEST


#define TEST "unmountC"
    contextTester->add(TEST,
                       domainSelectionContext,
                       &DomainSelectionContext::unmountC,
                       TEST);
#undef TEST

#define TEST "unmountE"
    contextTester->add(TEST,
                       domainSelectionContext,
                       &DomainSelectionContext::unmountE,
                       TEST);
#undef TEST

#define TEST "unmountD"
    contextTester->add(TEST,
                       domainSelectionContext,
                       &DomainSelectionContext::unmountD,
                       TEST);
#undef TEST

#define TEST "mountC"
    contextTester->add(TEST,
                       domainSelectionContext,
                       &DomainSelectionContext::mountC,
                       TEST);
#undef TEST

#define TEST "mountE"
    contextTester->add(TEST,
                       domainSelectionContext,
                       &DomainSelectionContext::mountE,
                       TEST);
#undef TEST

#define TEST "mountD"
    contextTester->add(TEST,
                       domainSelectionContext,
                       &DomainSelectionContext::mountD,
                       TEST);
#undef TEST

    domainTester->add(contextTester);

    // ... add more

    return domainTester;
}


