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

#include <iostream>
#include <fstream>
#include <memory>

#include "itk.h"


// This is a GCCE toolchain workaround needed when compiling with GCCE
// and using main() entry point
#ifdef __GCCE__
#include <staticlibinit_gcce.h>
#endif

Itk::TesterBase * CreateGeoTests();
Itk::TesterBase * CreateAutoArrayTests();
Itk::TesterBase * CreateParsingTests();
Itk::TesterBase * CreateConversionTests();
Itk::TesterBase * CreateMemTests(); 
Itk::TesterBase * CreateSyncQueueTests();
Itk::TesterBase * CreateJobQueueTests();
Itk::TesterBase * CreatePoolTests();
Itk::TesterBase * CreateMutexTests();


Itk::TesterBase * CreateTestHierarchy()
{
    using namespace Itk;

    SuiteTester
        * all = new SuiteTester("all");

    all->add(CreateGeoTests());
    all->add(CreateAutoArrayTests());
    all->add(CreateParsingTests());
    all->add(CreateConversionTests());
    all->add(CreateMemTests());
    all->add(CreateMutexTests());
    all->add(CreateSyncQueueTests());
    all->add(CreateJobQueueTests());
    all->add(CreatePoolTests());

    // add more ...

    return all;
}


#define TEST_DIR "c:\\data\\cpixtoolsunittest\\"


int main(int          argc,
         const char * argv[])
{
    using namespace std;
    using namespace Itk;

    int
        rv = 0;

    auto_ptr<TesterBase>
        testCase(CreateTestHierarchy());

    bool
        help = false;
    const char 
        * focus = NULL;

    if (argc == 2)
        {
            if (strcmp(argv[1], "h") == 0)
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
                        TEST_DIR);

            rv = testMgr.run(testCase.get(),
                             focus);

            testMgr.generateSummary(std::cout);

            ofstream
                ofs(TEST_DIR "report.txt");
            if (ofs)
                {
                    testMgr.generateSummary(ofs);
                }
        }

    int
        c = getchar();

    return rv;
}
