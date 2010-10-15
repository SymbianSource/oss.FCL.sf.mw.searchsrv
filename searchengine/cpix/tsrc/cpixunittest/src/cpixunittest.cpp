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
#include <wchar.h>
#include <glib.h>

#include <algorithm>
#include <functional>
#include <memory>

#include "cpixinit.h"
#include "config.h"

#include "randomtest.h"

#include "itk.h"

#include <iostream>
#include <fstream>

#include "std_log_result.h"

#ifdef __GCCE__
#include <staticlibinit_gcce.h>
#endif

using namespace Itk; 

Itk::TesterBase * CreatePartialSmsTests();
Itk::TesterBase * CreateDeletionTests();
Itk::TesterBase * CreateWhiteBoxTests();
Itk::TesterBase * CreateDestructiveTests(); 
Itk::TesterBase * CreatePrematureQryTests();
Itk::TesterBase * CreateCLuceneErrorTests(); // also from destructive.cpp
Itk::TesterBase * CreateStemTests();
Itk::TesterBase * CreateAggregateTests();
Itk::TesterBase * CreateGeoTests();
Itk::TesterBase * CreateMultiVolumeTests();
Itk::TesterBase * CreateDomainSelectionTests();
Itk::TesterBase * CreateUtf8Tests();
Itk::TesterBase * CreateUtf8PathTests();
Itk::TesterBase * CreateAnalysisTests();
Itk::TesterBase * CreateMapsTests();
Itk::TesterBase * CreateDocumentTests();
Itk::TesterBase * CreateTermSearchTests();
Itk::TesterBase * CreatePdfSearchTests();
Itk::TesterBase * CreateQueryTests();

Itk::TesterBase * CreateFlushTests();
Itk::TesterBase * CreateHeartbeatTests();
Itk::TesterBase * CreateNegativeTests();

// Clucene Qry
Itk::TesterBase * CreateClQryHierarchy();

Itk::TesterBase * CreateASyncTests();

Itk::TesterBase * CreateMiscTests();

// Avoid media Testcases if MDS not present. 
#ifdef MDS_PRESENT
// Spi 
Itk::TesterBase * CreateExifTests();
Itk::TesterBase * CreateId3Tests();

Itk::TesterBase * CreateSpiHierarchy() 
{
	using namespace Itk; 

	// "spi"
    SuiteTester
        * spi = new SuiteTester("spi");
    
    spi->add(CreateExifTests());  
    spi->add(CreateId3Tests()); 
    
    return spi; 
}
#endif


Itk::TesterBase * CreateTestHierarchy()
{
    using namespace Itk;

    // "all"
    SuiteTester
        * all = new SuiteTester("all");
    

    all->add(CreateSuiteTesterRandom()); 
    all->add(CreateDestructiveTests());
    all->add(CreatePrematureQryTests());
    // TODO enable when CLuceneErorr bug is found and fixed
    //all->add(CreateCLuceneErrorTests());  
    all->add(CreatePartialSmsTests()); 
    all->add(CreateDeletionTests());  
    all->add(CreateWhiteBoxTests()); 
    all->add(CreateAggregateTests()); 
	all->add(CreateNegativeTests());

    all->add(CreateMultiVolumeTests()); 	
    all->add(CreateDomainSelectionTests()); 
    all->add(CreateUtf8Tests());			
    all->add(CreateUtf8PathTests());
   // all->add(CreateAnalysisTests());		
		 
    all->add(CreatePdfSearchTests());
    all->add(CreateDocumentTests());		
    all->add(CreateFlushTests());			
    all->add(CreateHeartbeatTests());	    
    all->add(CreateTermSearchTests()); 		
    
    
    // TODO enable later Itk::TesterBase * CreateClQryHierarchy();

    all->add(CreateASyncTests());			

#ifdef MDS_PRESENT
    //all->add(CreateStemTests());    //Commented for STEMMER 
    //all->add(CreateGeoTests());     //Comment for GEO
    //all->add(CreateMapsTests());    //Comment for MAP	
    // all->add(CreateSpiHierarchy());  // Commented for JPG/EXIF files checks
#endif
	all->add(CreateQueryTests()); 
	all->add(CreateMiscTests());

    // add more top level test suites here
    // ...
    
    return all;
}


// required for heartbeat tests
const size_t       MaxIdleSec = 5;



#ifdef __SYMBIAN32__
#include "e32std.h" // For heap marks
#endif

int main(int          argc,
         const char * argv[])
{	
#ifdef __SYMBIAN32__
    __UHEAP_MARK;
#endif 

	using namespace std;

	int
		rv = 0;
	{
			
		cpix_Result
			result;
                cpix_InitParams
                    * initParams = cpix_InitParams_create(&result);
                if (cpix_Failed(&result))
                    {
                        printf("Failed to initialize Cpix\n");
                        return -1;
                    }
//                /* Added for decision coverage Test case */
                const char *cpix_Dir = cpix_InitParams_getCpixDir(initParams);
                
                const char *logFilebase = cpix_InitParams_getLogFileBase(initParams);
                
                size_t logSizeLimit = cpix_InitParams_getLogSizeLimit(initParams);
                
                size_t logSizeRecurrency = cpix_InitParams_getLogSizeCheckRecurrency(initParams);
                
                size_t maxIdleSec = cpix_InitParams_getMaxIdleSec(initParams);
                
                //size_t maxInsertBufferSize = cpix_InitParams_getMaxInsertBufSize(initParams);
                //cpix_InitParams_setMaxInsertBufferSize(initParams, maxInsertBufferSize);
                
                size_t insertBufMaxDocs = cpix_InitParams_getInsertBufMaxDocs(initParams);
                cpix_InitParams_setInsertBufMaxDocs(initParams, insertBufMaxDocs);
                
                size_t idxJobQueueSize = cpix_InitParams_getIdxJobQueueSize(initParams);
                cpix_InitParams_setIdxJobQueueSize(initParams, idxJobQueueSize);
                
                size_t qryJobQueueSize = cpix_InitParams_getQryJobQueueSize(initParams);
                cpix_InitParams_setQryJobQueueSize(initParams, qryJobQueueSize);
                
                int idxThreadPriorityDelta = cpix_InitParams_getIdxThreadPriorityDelta(initParams);
                cpix_InitParams_setIdxThreadPriorityDelta(initParams, idxThreadPriorityDelta);
                
                int qryThreadPriorityDelta = cpix_InitParams_getQryThreadPriorityDelta(initParams);
                size_t clHitsPageSize = cpix_InitParams_getClHitsPageSize(initParams);
                
                cpix_InitParams_setClHitsPageSize(initParams, clHitsPageSize);
                
                
                
                cpix_InitParams_setMaxIdleSec(initParams,
                                              MaxIdleSec);
                if (cpix_Failed(initParams))
                    {
                        printf("Failed to set max idle sec\n");
                        return -1;
                    }

                cpix_InitParams_setQryThreadPriorityDelta(initParams,
                                                          10);
                if (cpix_Failed(initParams))
                    {
                        printf("Failed to increase thread priority\n");
                        return -1;
                    }
		cpix_InitParams_setResourceDir(initParams,
									   RESOURCE_PATH);
		if (cpix_Failed(initParams))
			{
				printf("Failed to set resource dir\n");
				return -1;
			}

		cpix_init(&result,
                          initParams);
		if (cpix_Failed(&result))
			{
				wchar_t
					buffer[128];
				cpix_Error_report(result.err_,
                                                  buffer,
                                                  sizeof(buffer)/sizeof(wchar_t));
				printf("Failed to initialize CPix: %S\n",
					   buffer);
				
				std::wofstream out("c:\\data\\cpixunittest\\report.txt");
				out<<"Failed to initialize CPix:\n"<<buffer<<std::flush;
				return -1;
			}

                cpix_InitParams_destroy(initParams);
	
                cpix_dbgForceLogs();

		// cpix_setLogLevel(CPIX_LL_DEBUG);
	
		auto_ptr<TesterBase>
			testCase(CreateTestHierarchy());
	
		bool
			help = false;
		const char
			* focus = NULL;
	
		if (argc == 2)
			{ if (strcmp(argv[1], "h") == 0)
					{
						help = true;
					}
				else
					{
						focus = argv[1];
					}
			}
		
		
		if (help)
			{
				cout << "Test hierarchy:" << endl;
				testCase->printHierarchy(cout);
			}
		else
			{
				auto_ptr<Itk::CompositeTestRunObserver>
					observer(new Itk::CompositeTestRunObserver());
				observer->add(new Itk::TestRunConsole(std::cout));
	
				// do this only if you need a way to
				// confirm that the process is running
				// on the phone without proper std IO
				// console OBS observer->add(new
				// Itk::ProgressFsDisplayer());
	
				// this observer will dump every event
				// to a file, so it slows down
				// everything, but when things crash,
				// at least you get partial results on
				// the file system
				observer->add(new Itk::ProgressDumper("c:\\data\\cpixunittest\\dump.txt"));
	
				Itk::TestMgr 
					testMgr(observer.get(),
						   "c:\\data\\cpixunittest\\");
		 
				rv = testMgr.run(testCase.get(),
								focus);
				
				testMgr.generateSummary(std::cout);
	
				ofstream
					ofs("c:\\data\\cpixunittest\\report.txt");
				if (ofs)
					{
						testMgr.generateSummary(ofs);
					}
				
		        bool
		            verdict = (testMgr.getFailedExpects() + testMgr.getFailedAsserts()) == 0;
		        
		            testResultXml(focus, verdict);
		        
		        
			}
		
	
		cpix_shutdown();
	}

#ifdef __SYMBIAN32__
    __UHEAP_MARKEND; 
#endif 
	return rv;
}
