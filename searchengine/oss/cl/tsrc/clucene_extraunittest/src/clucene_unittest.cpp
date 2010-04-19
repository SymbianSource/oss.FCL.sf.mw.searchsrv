/*
 * clucene_unittest.cpp
 *
 *  Created on: Jun 3, 2009
 *      Author: admin
 */
 
 /*------------------------------------------------------------------------------
* Copyright (C) 2010 Nokia Corporation and/or its subsidiary(-ies). 
* 
* Distributable under the terms of either the Apache License (Version 2.0) or 
* the GNU Lesser General Public License, as specified in the COPYING file.
------------------------------------------------------------------------------*/

#include <stdio.h>
#include <wchar.h>
#include <glib.h>
#include <algorithm>
#include <functional>
#include <memory>

#include "itk.h"

#include <iostream>

#ifdef __GCCE__
#include <staticlibinit_gcce.h>
#endif

using namespace Itk; 


Itk::TesterBase * CreateDeleteUnitTest();
Itk::TesterBase * CreateTermUnitTest();

Itk::TesterBase * CreateTestHierarchy()
{
    using namespace Itk;

    SuiteTester
        * all = new SuiteTester("all");

    all->add(CreateDeleteUnitTest());
    all->add(CreateTermUnitTest());

    return all;
}

int main(int          argc,
         const char * argv[])
{
    using namespace std;

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
                        "c:\\data\\clextraunittest\\");
     
            rv = testMgr.run(testCase.get(),
                            focus);
            
            testMgr.generateSummary(std::cout);
        }
    

    int
    	c = getchar();

    return rv;
}
