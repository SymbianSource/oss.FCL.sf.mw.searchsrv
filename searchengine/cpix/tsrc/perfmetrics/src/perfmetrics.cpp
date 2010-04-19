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
* Description: Main application class
*
*/

// INCLUDE FILES
#include <stdio.h>
#include <algorithm>
#include <functional>
#include <iostream.h>
#include <fstream>
#include <memory>

#include "idxutil.h"
#include "itk.h"
#include "corpustester.h"

#include "testvector.h"

// This is a GCCE toolchain workaround needed when compiling with GCCE
// and using main() entry point
#ifdef __GCCE__
#include <staticlibinit_gcce.h>
#endif

/*  OBS
TestVector   testVectors[] = {
    { "plain", 0, 60000, 5000, 100, 100, SOME_QUERY_TERMS, 5, 5},
    // { "plain", 0, 120000, 15000, 100, 100, SOME_QUERY_TERMS, 1, 0},
    // { "plain", 0, 30000, 5000, 100, 100, SOME_QUERY_TERMS, 5, 5},
    // ORIGINAL { "plain", 0, 30, 5, 2, 2, SOME_QUERY_TERMS, 5, 5},
    
    // penalty due to search - write state switch
    { "swA", 30000, 20, 20, 0, 0, SOME_QUERY_TERMS, 20, 0},
    { "swB", 30000, 20, 1,  0, 0, SOME_QUERY_TERMS, 1,  0}, 

    // penalty due to write - delete state switch
    { "wdA", 30000, 20, 20, 0, 20, EMPTY_QUERY_TERMS, 0, 0},
    { "wdB", 30000, 20, 1,  0, 1,  EMPTY_QUERY_TERMS, 0, 0},

    // sys performance
    { "sys", 0, 2001, 100, 10, 10, SOME_QUERY_TERMS, 5, 5}
};
*/


#define BASEDIR "c:\\data\\perfmetrics\\"

#define TESTVECTOR "c:\\data\\perfmetrics\\testvector.txt"


extern const char BaseDir[] = BASEDIR;


//
// Install util/hugetestcorpus/....sis !
//
// There is no install package for pois.txt: you have to use one POI
// dump file and copy it manually to the right directory
//

#ifdef __WINS__ 
const char TEST_CORPUS_FILE[] = "c:\\data\\perfmetricscorpus\\hugetestcorpus.txt";
const char POI_CORPUS_FILE[] = "c:\\data\\perfmetricscorpus\\pois.txt";
const char STREET_CORPUS_FILE[] = "c:\\data\\perfmetricscorpus\\streets.csv";
#else
const char TEST_CORPUS_FILE[] = "e:\\data\\perfmetricscorpus\\hugetestcorpus.txt";
const char POI_CORPUS_FILE[] = "e:\\data\\perfmetricscorpus\\pois.txt";
const char STREET_CORPUS_FILE[] = "e:\\data\\perfmetricscorpus\\streets.csv";
#endif

class TestVectorContext : public Itk::ITestContext
{
    TestVector            tv_;
    CorpusTester        * corpusTester_;
    IdxUtil             * idxUtil_;

public:

    TestVectorContext(const TestVector & testVector)
        : tv_(testVector),
          corpusTester_(NULL),
          idxUtil_(NULL)
    {
        ;
    }


    virtual void setup() throw (Itk::PanicExc)
    {
        using namespace std;

        /* OBS
        bool
            poi = (tv_.testName == "poi");
        const char
            * corpusPath = poi ? POI_CORPUS_FILE : TEST_CORPUS_FILE;
            */
        try
            {
                const char
                    * corpusPath = NULL;
                auto_ptr<IdxUtil>
                    iu;
                
                if (tv_.testName == "plain")
                    {
                        corpusPath = TEST_CORPUS_FILE;
                        iu.reset(new IdxUtil());
                    }
                else if (tv_.testName == "poi")
                    {
                        corpusPath = POI_CORPUS_FILE;
                        iu.reset(new PoiIdxUtil());
                    }
                else if (tv_.testName == "street")
                    {
                        corpusPath = STREET_CORPUS_FILE;
                        iu.reset(new StreetIdxUtil());
                    }

                iu->init(static_cast<bool>(tv_.create));
                cpix_IdxDb_setMaxInsertBufSize( iu->idxDb(),
                                                tv_.bufferSize ); 

                auto_ptr<CorpusTester>
                    ct(new CorpusTester(corpusPath,
                                        tv_.numPreindexedItems,
                                        tv_.testName,
                                        tv_.bufferSize,
                                        tv_.maxInputSize,
                                        tv_.lumpSize,
                                        tv_.first,
                                        tv_.last,
                                        tv_.queryTerms,
                                        tv_.search,
                                        tv_.incrementalSearch,
                                        tv_.minIncrementalLength,
                                        tv_.suggestionSearch,
                                        tv_.minSuggestionLength,
                                        tv_.takeIdxSnapshot,
                                        tv_.addLasts,
                                        tv_.delReAddLasts,
                                        tv_.updateLasts));
                /* OBS
                auto_ptr<IdxUtil>
                    iu(poi ? new PoiIdxUtil() : new IdxUtil());
                    iu->init(static_cast<bool>(tv_.create));
                */

                corpusTester_ = ct.get();
                ct.release();
                
                idxUtil_ = iu.get();
                iu.release();
            }
        catch (std::exception & exc)
            {
                ITK_PANIC("Cant' initialize corpus tester for %s: %s",
                          tv_.testName.c_str(),
                          exc.what());
            }
        catch (...)
            {
                ITK_PANIC("Can't initialize corpus tester for %s: unknown reason.",
                          tv_.testName.c_str());
            }
    }


    virtual void tearDown() throw()
    {
        cleanup();
    }


    virtual ~TestVectorContext()
    {
        cleanup();
    }


    void measure(Itk::TestMgr * testMgr)
    {
        corpusTester_->run(testMgr,
                           idxUtil_);
    }


private:
    void cleanup() throw()
    {
        delete corpusTester_;
        corpusTester_ = NULL;

        delete idxUtil_;
        idxUtil_ = NULL;
    }
};


Itk::TesterBase * CreateTestHierarchy()
{
    using namespace Itk;

    // "all"
    SuiteTester
        * all = new SuiteTester("all");

    std::ifstream
        ifs(TESTVECTOR);
    TestVector
        tv;
    ifs >> tv;

    TestVectorContext
        * tvc = new TestVectorContext(tv);
    ContextTester
        * contextTester = new ContextTester(tv.testName.c_str(),
                                            tvc);
    contextTester->add("measure",
                       tvc,
                       &TestVectorContext::measure);
    
    all->add(contextTester);

    return all;
}



void initBaseDir(const char * path)
{
    if (!Cpt::directoryexists(path))
        {
            if (Cpt::mkdirs(path, 0777) != 0)
                {
                    printf("Could not create base directory: %s\n",
                           path);
                    exit(2);
                }
        }
}



void checkCorpus(const char * path)
{
    if (!Cpt::isreadable(path))
        {
            printf("Could not open test corpus for reading: %s\n",
                   path);
            exit(2);
        }
}



int main(int          ,
         const char * )
{
    using namespace std;

    int
        rv = 0;

    cpix_Result 
        result;
    cpix_init(&result,
              NULL);
    if (cpix_Failed(&result))
        {
            wchar_t
                buffer[128];
            cpix_Error_report(result.err_,
                              buffer,
                              sizeof(buffer)/sizeof(wchar_t));
            printf("Failed to initialize CPix: %S\n", buffer);
            return -1;
        }
    
    std::auto_ptr<Itk::TesterBase> 
        testCase(CreateTestHierarchy());
    
    string
        cmd("all");
    
    initBaseDir(BASEDIR);
    checkCorpus(TEST_CORPUS_FILE);

    bool
        help = false;
    const char
        * focus = NULL;
    string
        outPath(BASEDIR);

    if (cmd == "h")
        {
            help = true;
            outPath += "help.txt";
        }
    else
        {
            focus = cmd.c_str();
            outPath += focus;
            outPath += ".txt";
        }

    ofstream
        ofs(outPath.c_str());

    if (help)
        {
            std::cout << "Test hierarchy:" << endl;
            testCase->printHierarchy(std::cout);

            if (ofs)
                {
                    ofs << "Test hierarchy:" << endl;
                    testCase->printHierarchy(ofs);
                }
        }
    else
        {
            auto_ptr<Itk::CompositeTestRunObserver>
                observer(new Itk::CompositeTestRunObserver());
            observer->add(new Itk::TestRunConsole(std::cout));

            // do this only if you need a way to confirm that the
            // process is running on the phone without proper std IO
            // console
            // observer->add(new Itk::ProgressFsDisplayer());

            // this observer will dump every event to a file, so it
            // slows down everything, but when things crash, at least
            // you get partial results on the file system
            observer->add(new Itk::ProgressDumper(BASEDIR "dump.txt"));

            Itk::TestMgr 
                testMgr(observer.get(),
                        BASEDIR);
 
            rv = testMgr.run(testCase.get(), focus);
		
            // TMP DISABLED testMgr.generateSummary(std::cout);

            if (ofs)
                {
                    testMgr.generateSummary(ofs);
                }
        }

    // OBS int c = getchar();
    return rv;
}

