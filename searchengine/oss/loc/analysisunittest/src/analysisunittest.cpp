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
#include <algorithm>
#include <functional>
#include <memory>

#include "itk.h"

#include <iostream>

#include "thaianalysis.h"

using namespace Itk;

#define THAISTATEMACHINE_FILE "c:\\data\\analysisunittestcorpus\\thai\\thaidict.sm"

Itk::TesterBase * CreateThaiAnalysisUnitTest();
Itk::TesterBase * CreateEvaluationTest(); 
Itk::TesterBase * CreateCjkAnalyzerUnitTest(); 
Itk::TesterBase * CreateTinyAnalysisUnitTest(); 
Itk::TesterBase * CreateNgramAnalyzerUnitTest();
Itk::TesterBase * CreateKoreanAnalyzerUnitTest(); 
Itk::TesterBase * CreatePrefixAnalyzerUnitTest();

Itk::TesterBase * CreateTestHierarchy()
{
    using namespace Itk;

    SuiteTester
        * all = new SuiteTester("all");

    all->add(CreateThaiAnalysisUnitTest());
    all->add(CreateEvaluationTest()); 
    all->add(CreateCjkAnalyzerUnitTest());  
    all->add(CreateTinyAnalysisUnitTest());  
    all->add(CreateNgramAnalyzerUnitTest());  
    all->add(CreateKoreanAnalyzerUnitTest());
	all->add(CreatePrefixAnalyzerUnitTest());  

    return all;
}

int main(int          argc,
         const char * argv[])
{
    using namespace std;
    
    analysis::InitThaiAnalysis(THAISTATEMACHINE_FILE);

    int
        rv = 0;

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
            TestRunConsole
                ui(std::cout);
    
            TestMgr
                testMgr(&ui,
                        "c:\\data\\analysisunittest\\");
     
            rv = testMgr.run(testCase.get(),
                            focus);
            
            testMgr.generateSummary(std::cout);
            
			ofstream
					ofs("c:\\data\\analysisunittest\\report.txt");
			if (ofs)
				{
					testMgr.generateSummary(ofs);
				}
        }
    

    int
    	c = getchar();
    
    analysis::ShutdownThaiAnalysis();

    return rv;
}
