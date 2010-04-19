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

Itk::TesterBase * CreateFlushTests();
Itk::TesterBase * CreateHeartbeatTests();
Itk::TesterBase * CreateNegativeTests();

// Clucene Qry
Itk::TesterBase * CreateClQryHierarchy();

Itk::TesterBase * CreateASyncTests();

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
    all->add(CreateStemTests()); 
    all->add(CreateAggregateTests()); 
	all->add(CreateNegativeTests());
    all->add(CreateGeoTests());
    all->add(CreateMultiVolumeTests()); 	
    all->add(CreateDomainSelectionTests()); 
    all->add(CreateUtf8Tests());			
    all->add(CreateUtf8PathTests());
    all->add(CreateAnalysisTests());		
    all->add(CreateMapsTests());			 

    all->add(CreateDocumentTests());		
    all->add(CreateFlushTests());			
    all->add(CreateHeartbeatTests());	    
    all->add(CreateTermSearchTests()); 		
    //all->add(CreatePdfSearchTests());
    
    // TODO enable later Itk::TesterBase * CreateClQryHierarchy();

    all->add(CreateASyncTests());			

    all->add(CreateSpiHierarchy());

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
			}
		
	
		cpix_shutdown();
	
		int
			c = getchar();
	}

#ifdef __SYMBIAN32__
    __UHEAP_MARKEND; 
#endif 
	return rv;
}
