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
#ifndef ITK_ITKTESTMGR_H_
#define ITK_ITKTESTMGR_H_

#include <iosfwd>
#include <list>
#include <string>
#include <utility>

#include "cpixsynctools.h"

namespace Itk
{
    class TestEvent;
    class ITestRunObserver;

    namespace Impl 
    {
        class OutputRedirector;
        class IOCaptureEvaluator;
        template<typename TESTFUNC>
        void EvaluateIOCapture(const std::string  & defFilesBaseName,
                               TestMgr            * testMgr,
                               TESTFUNC             testFunc,
                               const std::string  & lenience);
    }


    /**
     * The class that executes the tests and collects, interprets and
     * reports the results.
     */
    class TestMgr
    {
    public:
        //
        // public typedefs
        //

        /**
         * Iterator with wich one can get the test events.
         */
        typedef std::list<TestEvent>::const_iterator  TestEventIterator;

        /**
         * Custom report item is basically a name or short comment and
         * a value/or explanatory string pair.
         */
        typedef std::pair<std::string, std::string> CustomReport;

        /**
         * To iterate over custom report items with.
         */
        typedef std::list<CustomReport>::const_iterator
        CustomReportIterator;

    private:
        //
        // private members
        //

        // it is called a stack because that's what it is mainly used
        // for, however sequential access to all testcases in the
        // execution stack is also needed, so we use list for the
        // purposes of a stack.
        std::list<TesterBase*>  testerBaseStack_;

        size_t                  failedExpects_;
        size_t                  failedAsserts_;

        std::list<TestEvent>    testEvents_;

        ITestRunObserver      * observer_;
        const std::string       ioTestCasesDir_;

        std::list<CustomReport> customReports_;

        int                     dbgConsoleFd_;
        std::string             skippedNames_;
        TesterBase            * rootTesterBase_;


        /**
         * Operations invoked during test programs, possibly executing
         * multiple threads should be protected by this mutex.
         */
        Cpt::Mutex              mutex_;
        

    public:
        //
        // public operations
        //


        /**
         * Executes test(s), or a subset of a whole test hierarchy.
         * 
         * @param testerBase the test(s) to execute. May be NULL, in
         * which case nothing is really done, and the return value is
         * false in this case.
         *
         * @param focus if given, it can determine a subset of tests
         * from the whole hierarchy. For instance, let's assume the
         * following test case hierarhcy pointed by argument
         * 'testerBase':
         *
         * <code>
         * all
         *   |
         *   +-- myContext1
         *   |     |
         *   |     +-- case1.1
         *   |     +-- case1.2
         *   |
         *   +-- myContext2
         *         |
         *         +-- case2.1
         *         +-- case2.2
         * </code>
         *
         * If nothing, or "all" is given as focus, then all test cases
         * will be executed. If "myContext1" or "all/myContext1" is
         * given as a focus, then case1.1 and case1.2 are
         * executed. Since it makes no sense to execute test cases
         * outside their contexts, giving "myContext/case1.2" (or
         * "all/myContext/case1.2") is equivalent to giving
         * "myContext1" (or "all/myContext1") simply because case1.2
         * may (and is likely to) depend on a state that has been
         * established in the setup of myContext1 and further evolved
         * by case1.1.
         * 
         * Concluding: the focus specifier is optional as is the name
         * of the root test case itself as a prefix, and there is no
         * point focusing further than the highest enclosing context.
         *
         * A bad focus specifier will abort running any test cases and
         * false is returned and message is printed on std err
         * concerning the bad focus.
         *
         *@returns true if completely successful execution, false
         *otherwise (failed test cases or bad focus specifier was
         *given).
         */
        bool run(TesterBase   * testerBase,
                 const char   * focus = NULL);
        

        /**
         * @returns the number of failed expectations (ITK_EXPECT)
         */
        size_t getFailedExpects() const;

        
        /**
         * @returns the number of failes asserts (ITK_ASSERT)
         */
        size_t getFailedAsserts() const;


        /**
         * @returns the iterator to the first test event
         */
        TestEventIterator beginEvents() const;


        /**
         * @returns the iterator the (past) last test event
         */
        TestEventIterator endEvents() const;


        /**
         * @returns the iterator to the first custom report
         */
        CustomReportIterator beginCustomReports() const;

        
        /**
         * @returns the iterator (past) the last custom report.
         */
        CustomReportIterator endCustomReports() const;

 
        /**
         * Generates report on the given ostream, in case a client
         * does not want to iterate over the test event manually.
         */
        void generateSummary(std::ostream & os) const;

        /**
         * Generates report to a file.
         */
        bool generateSummary(const char * file) const;
        

        /**
         * Gives the path to the directory under which test case
         * definition files are to be found at the moment of
         * invocation (taking into account the current position in the
         * test hierarhcy).
         *
         * The name of the root test case is never included, and the
         * name of the last test case on the "execution stack" is only
         * included if it is a test suite (or test context).
         */
        void ioTestCasesDir(std::string & path) const;


        //
        // lifetime management
        //

        /**
         * Constructor
         */
        TestMgr(ITestRunObserver * observer,
                const char       * ioTestCasesDir);


        /**
         * Destructor
         */
        ~TestMgr();



    private:
        //
        // private methods
        //

        friend void itk_expect(TestMgr     * testMgr,
                               bool          succeeded,
                               const char  * expr,
                               const char  * file,
                               size_t        line,
                               const char  * format,
                               ...);


        friend void itk_assert(TestMgr     * testMgr,
                               bool          succeeded,
                               const char  * expr,
                               const char  * file,
                               size_t        line,
                               const char  * format,
                               ...);

        friend void itk_msg(TestMgr     * testMgr,
                            const char  * file,
                            size_t        line,
                            const char  * format,
                            ...);


        friend void itk_dbgMsg(TestMgr     * testMgr,
                               const char  * format,
                               ...);


        friend void itk_report(TestMgr     * testMgr,
                               const char  * name,
                               const char  * format,
                               ...);

        friend class Impl::OutputRedirector;
        friend class Impl::IOCaptureEvaluator;
        friend class ContextTester;

        template<typename TESTFUNC>
        friend void Impl::EvaluateIOCapture(const std::string  & defFilesBaseName,
                                            TestMgr            * testMgr,
                                            TESTFUNC             testFunc,
                                            const std::string  & lenience);
        friend class RootContextTester;


        /**
         * Handles an expectation at a certain location (expression,
         * file, line) and message in case the expectation failed.
         *
         * Thread-safe.
         */
        void expecting(bool         succeeded,
                       const char * expr,
                       const char * file,
                       size_t       line,
                       const char * msg);

        /**
         * Handles an assertion at a certain location (expression,
         * file, line) and message in case the expectation failed.
         *
         * Thread-safe.
         */
        void asserting(bool         succeeded,
                       const char * expr,
                       const char * file,
                       size_t       line,
                       const char * msg);

        /**
         * Handles an unknown failure in a context.
         *
         * Thread-safe.
         */
        void unknownFailure(const char * contextName);


        /**
         * Dumps a message (may be redirected if the test case is
         * redirected).
         *
         * Thread-safe.
         */
        void msg(const char * file,
                 size_t       line,
                 const char * msg);


        /**
         * Dumps a message that never gets redirected to a test
         * output, always goes to the console. Great tool to provide
         * progress information (dost, etc.).
         *
         * Thread-safe.
         */
        void dbgMsg(const char * msg);


        /**
         * Define a name value pair. Reports are later collected and
         * dumped at the end of the overall testrun. Name/value pairs
         * need not be unique.
         *
         * Thread-safe.
         */
        void report(const char * name,
                    const char * value);

        /**
         * Handles a panic event.
         *
         * Thread-safe.
         */
        void panic(const char * file,
                   size_t       line,
                   const char * msg);


        /**
         * Sets the file descriptor that would print directly to the
         * console. Required for dbgMsg events, and set by the entity
         * doing the IO redirection.
         *
         * Thread-safe. (Needed?)
         */
        void setDbgConsoleFd(int fd);
        

        /**
         * In case a test case was redirected and there was no
         * expectations yet - to notify the user about the creation a
         * new test case that needs to be reviewed / confirmed.
         *
         * Thread-safe. (Needed?)
         */
        void ioCaptureDefined(const char * file,
                              const char * msg);


        /**
         * In case a test case was redirected and it did not match the
         * expectations.
         *
         * Thread-safe. (Needed?)
         */
        void ioCaptureError(const char * file,
                            const char * msg);




        friend class BeginEndSentry;

        
        /**
         * Marks the beginning of a test case
         *
         * Thread-safe. (Needed?)
         */
        void beginTestCase(TesterBase * testerBase);


        /**
         * Marks the end of a test case.
         *
         * Thread-safe. (Needed?)
         */
        void endTestCase(TesterBase * testerBase) throw ();


        void clearTesterBaseStack();


        TesterBase * GetTesterBaseToRun(const char      * focus,
                                        TesterBase      * rootTesterBase);
        
    };

    
}

#endif // ITK_ITKTESTMGR_H_
