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

#include <wchar.h>

#include <cstring>
#include <exception>

#include <iostream>
#include <string>

#include "itk.h"

// This is a GCCE toolchain workaround needed when compiling with GCCE
// and using main() entry point
#ifdef __GCCE__
#include <staticlibinit_gcce.h>
#endif


/**
 * These demonstrates (and tests) ITK, the integration test kit. To be
 * able to test / demo both successful and failed runs, the option
 * "buggy" is used, in which case "implementation" functions behave
 * "buggily".
 *
 * ITK features:
 *
 * UNIT TEST FETAURES
 * ==================
 *
 *  (a) ITK_EXPECT - a macro that tells what should be true (see
 *      examples), but on failure, execution is continued
 *
 *  (b) ITK_ASSERT - the difference between expect and assert is that
 *      if an assert fails, then the rest of test cases in the same
 *      test context are not executed. (There is always a root context
 *      encompassing all the test cases).
 *
 *  (c) ITK_MSG - just producing a message, accepts printf like
 *      formatted output.
 *
 *  (d) One can define a context with setup and teardown. See example.
 *
 *
 * INTEGRATION TEST FEATURES
 * =========================
 *
 *  (a) Tests cases that are tested through what they produce in the
 *      standard output and error can also be run.
 *
 *  (b) See itktestcase.h for details, also the sample code here.
 *
 * 
 */


int Negate(int n)
{
    return -n;
}


class SimpleMath
{
private:
    bool buggy_;

public:
    SimpleMath(bool buggy)
        : buggy_(buggy)
    {
        ;
    }


    int add(int left,
            int right)
    {
        return buggy_ ? left - right : left + right;
    }
    
    int sub(int left,
            int right)
    {
        return buggy_ ? left + right : left - right;
    }

    void doSomeIO()
    {
        using namespace std;

        if (buggy_)
            {
                cout << "Producing InCoRrEcT output" << endl;
                cout << endl;
                cout << "Garble garble farble" << endl;
                cout << endl;

                cerr << "Producing InCoRrEct output on stderr too" << endl;
            }
        else
            {
                cout << "Producing CORRECT output" << endl;
                cout << endl;
                cout << "Sensible sensible sense" << endl;
                cout << endl;
            }
    }


    void doSomeMoreIO()
    {
        using namespace std;

        int
            a,b;

        cin >> a;
        cin >> b;

        cout << "Producing " << a << "+" << b << "=" << add(a,b) << endl;
    }

};


/**
 * Simple test function unit testing some functionality: making
 * computations and then making statements about the results.
 */
void TestNegate(Itk::TestMgr * testMgr)
{
    ITK_EXPECT(testMgr,
               5 == Negate(-5),
               "Negate(-5) does not work");
    ITK_EXPECT(testMgr,
               -5 == Negate(5),
               "Negate(5) does not work");
    ITK_EXPECT(testMgr,
               0 == Negate(0),
               "Negate(0) does not work");
}


class SimpleMathTest : public Itk::ITestContext
{
private:
    bool         buggy_;
    SimpleMath * simpleMath_;
public:
    SimpleMathTest(bool buggy)
        : buggy_(buggy),
          simpleMath_(NULL)
    {
        ;
    }

    
    ~SimpleMathTest()
    {
        ;
    }


    virtual void setup() throw (Itk::PanicExc)
    {
        try
            {
                simpleMath_ = new SimpleMath(buggy_);
            }
        catch (std::exception & exc)
            {
                ITK_PANIC("Could not construct SimpleMath: %s",
                          exc.what());
            }
        catch (...)
            {
                ITK_PANIC("Could not construct SimpleMath: unknown reason.");
            }
    }

    virtual void tearDown() throw()
    {
        delete simpleMath_;
        simpleMath_ = NULL;
    }

    
    void testAdd(Itk::TestMgr * testMgr)
    {
        ITK_EXPECT(testMgr,
                   simpleMath_->add(4,5) == 9,
                   "Addition does not work");
        ITK_EXPECT(testMgr,
                   simpleMath_->add(-11,11) == 0,
                   "Addition does not work");
        ITK_EXPECT(testMgr,
                   simpleMath_->add(-4,-5) == -9,
                   "Addition does not work");
    }


    void testSub(Itk::TestMgr * testMgr)
    {
        ITK_EXPECT(testMgr,
                   simpleMath_->sub(4,5) == -1,
                   "Subtraction does not work");
        ITK_ASSERT(testMgr,
                   simpleMath_->sub(11,11) == 0,
                   "Subtraction does not work");
        ITK_EXPECT(testMgr,
                   simpleMath_->sub(-4,-5) == 1,
                   "Subtraction does not work");
    }


    void testDoSomeIO(Itk::TestMgr * testMgr)
    {
        ITK_MSG(testMgr,
                "Testing Some IO");

        simpleMath_->doSomeIO();
    }


    void testDoSomeMoreIO(Itk::TestMgr * testMgr)
    {
        ITK_MSG(testMgr,
                "Testing Some MORE IO (with input too)");

        using namespace Itk;

        Timestamp
            before;
        getTimestamp(&before);

        simpleMath_->doSomeMoreIO();

        Timestamp
            after;
        getTimestamp(&after);

        long
            elapsedMs = getElapsedMs(&after,
                                     &before);

        ITK_MSG(testMgr,
                "Testing more IO took %d ms",
                elapsedMs);

        ITK_REPORT(testMgr,
                   "Elapsed time",
                   "%d (ms)",
                   elapsedMs);
        ITK_DBGMSG(testMgr,
                   "A message for the debug console.");
    }

};


bool runTests(bool buggy)
{
    bool
        rv = true;

    using namespace std;
    using namespace Itk;

    SuiteTester
        suiteTester("Arithmetic tests");

    // free testing function
    suiteTester.add("Negating",
                    TestNegate);

    // an IContext derivant (with setup and teardown
    SimpleMathTest
        * simpleMathTest = new SimpleMathTest(buggy);

    // test context
    ContextTester
        * contextTester = new ContextTester("simplemath",
                                            simpleMathTest);

    // member testing function with IO captured test definition
    contextTester->add("Doing some IO",
                       simpleMathTest,
                       &SimpleMathTest::testDoSomeIO,
                       "doSomeIO");
    contextTester->add("Doing some more IO",
                       simpleMathTest,
                       &SimpleMathTest::testDoSomeMoreIO,
                       "doSomeMoreIO",
                       "Testing more IO took"); // lenience

    // member testing function
    contextTester->add("Adding",
                       simpleMathTest,
                       &SimpleMathTest::testAdd);
    contextTester->add("Subtracting",
                       simpleMathTest,
                       &SimpleMathTest::testSub);
    

    suiteTester.add(contextTester);

    TestRunConsole
        ui(std::cout);
 
    TestMgr
        testMgr(&ui,
                "c:\\data\\itkdemo\\");

    rv = testMgr.run(&suiteTester);

    testMgr.generateSummary(std::cout);

    return rv;
}



void Usage(const char * exeName)
{
    using namespace std;

    cout << "Usage:" << endl;
    cout << exeName << " [b]" << endl;
    cout << "  b: 'BuGgY run - fail tests on purpose" << endl;
}



int main(int          argc,
         const char * argv[])
{
    bool
        buggy = false;
    const char
        * exeName = argv[0];

    if (argc == 2)
        {
            if (strcmp("b", argv[1]) == 0)
                {
                    buggy = true;
                }
            else
                {
                    Usage(exeName);
                    exit(1);
                }
        }
    else if (argc == 1)
        {
            ;
        }
    else
        {
            Usage(exeName);
            exit(1);
        }

    int
        rv = runTests(buggy);

    int
        c = getchar();
    c = getchar();
    c = getchar();

    return rv;
}
