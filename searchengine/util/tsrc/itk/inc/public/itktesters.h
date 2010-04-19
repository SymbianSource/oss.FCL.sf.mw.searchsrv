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
#ifndef ITK_ITKTESTERS_H_
#define ITK_ITKTESTERS_H_

#include <exception>
#include <iosfwd>
#include <list>
#include <string>

namespace Itk
{

    class TestMgr;

    
    class IOCaptureExc : public std::exception
    {
    private:
        //
        // private members
        //
        const std::string what_;

    public:
        //
        // public operators
        //
        virtual const char * what() const throw();

        //
        // lifetime mgmt
        //
        IOCaptureExc(const char * what);
        IOCaptureExc(const IOCaptureExc & that);
    };


    /**
     * Base class for all different flavours of test cases.
     */
    class TesterBase
    {
    private:
        //
        // private members
        //
        const std::string        name_;

    public:
        //
        // public operations
        //

        /**
         * Constructor
         *
         * @param name the name of this test case, must not be NULL.
         */
        TesterBase(const char * name);

        /**
         * @returns the human readable name of the test case
         */
        const std::string & name() const;


        /**
         * Prints this tests (and any sub-tests) in a nicely
         * indented form to the given output stream.
         *
         * @param os the output stream to print to
         * @param indent the starting indentation level, optional
         */
        virtual void printHierarchy(std::ostream & os,
                                    int            indent = 0) const;
        

        /**
         * @returns the number of actual (leaf) test cases under this
         * tester.
         */
        virtual size_t count() const;
        
        /**
         * Runs the test(s) embodied in this instance with
         * all the administrative housekeeping done wrt testMgr.
         */
        void run(TestMgr * testMgr);

        /**
         * Destructor
         */
        virtual ~TesterBase() = 0;

    protected:

        /**
         * Actually runs the test(s) embodied in this instance.
         */
        virtual void doRun(TestMgr * testMgr) = 0;


        void indentSpaces(int            indent,
                          std::ostream & os) const;
    };



    /**
     * A test suite is a set of test (cases, suites or contexts).
     */
    class SuiteTester : public TesterBase
    {
    private:
        //
        // private members
        //
        std::list<TesterBase*>      testerBases_;


    public:
        //
        // public operators
        //

        /**
         * Constructor
         *
         * @param name the name of this test suite, must not be NULL.
         */
        SuiteTester(const char * name);


        /**
         * From TesterBase.
         */
        virtual void printHierarchy(std::ostream & os,
                                    int            indent = 0) const;


        /**
         * @returns the number of actual (leaf) test cases under this
         * tester.
         */
        virtual size_t count() const;


        /**
         * Adds a test case into this suite.
         */
        void add(TesterBase * testerBase);

        /**
         * Adds a test case, implemented by a simple free function
         * (C-style) to this suite.
         *
         * @param name the name of the test case, must not be NULL.
         *
         * @param testFunc the testMgr function, may use ITK_EXPECT,
         * ITK_ASSERT and ITK_MSG macros
         */
        void add(const char * name,
                 void      (* testFunc)(TestMgr *));


        /**
         * Adds a test case, implemented by a non-static member
         * function of a C++ class to this suite.
         *
         * @param name the name of the test case, must not be NULL.
         *
         * @param ptr the instance on which the memFunc will have to
         * operate, being a non-static member function. Its ownership
         * is NOT transferred by this call. Must not be NULL. For
         * static member functions, you use the function above.
         *
         * @param memFunc the member function implementing the test,
         * may use ITK_EXPECT, ITK_ASSERT and ITK_MSG macros
         */
        template<typename C>
        void add(const char * name,
                 C          * ptr,
                 void  (C:: * memFunc)(TestMgr *));

        

        /**
         * Given to overloaded versions of add() below, it tells them
         * to generate IO only, not to compare / test for it.
         */
        static const char REDIRECT_ONLY[];


        /**
         * The reason for this homebred test framework is these next
         * two functions (the rest is provided by countless other
         * frameworks).
         *
         * In some cases, notably when integrating with existing tests
         * for 3rd party applications, it does not help to be able to
         * assert things, simply because 3rd party test applications
         * will produce a bunch of messages on stdout / stderr, and
         * that cannot be tested by assertions.
         *
         * This is also the case when one tests a functionality of a
         * text processor component. While it may be possible to
         * manually load text, perform the operation and then compare
         * it with what one should get, it is a manual, repetitive and
         * tedious labor - not suited for test developers.
         *
         * Hence the need for capturing IO for test purposes,
         * redirecting it to/from pre-defined test case files.
         *
         *
         * 1 Capturing Output
         * ==================
         *
         * Let's assume that defFilesBaseName is 'my-tests-1' and the
         * TestMgr instance was constructed with 'c:\Data\' as its
         * ioTestCasesDir argument. In this case we consider the base
         * filepath 'c:\Data\my-test-1'.
         *
         * The standard output is always redirected to file
         * 'c:\Data\my-test-1_res_out.txt', meaning that this file is
         * the RESult that has been sent to the standard OUTput.
         *
         * At the completion of the test func execution, file
         * 'c:\Data\my-test-1_exp_out.txt' (exp for EXPected) is
         * tested for. If it exists, then these two files are
         * compared, and the results are reported accordingly (success
         * on same content, failure on difference). If the pre-defined
         * file does not exist, then 'res' is saved as 'exp', and what
         * is reported that an IO test definition has been created
         * (neither pass, nor failure, but define).
         *
         * Obviously, you should get a 'success' instead of a 'define'
         * on a second testrun, since the pre-defined output is there
         * now - unless you have a test operation that produces
         * different output on different invocations in which case
         * you are almost beyond help, but see parameter 'lenience'.
         *
         * Once you have defined a test case thus, you have to
         * manually go through it to make sure that what you defined
         * is indeed the correct behaviour. 'Exp' files should be
         * versioned and deployed along with the test application.
         *
         *
         * 2 Capturing Error
         * =================
         *
         * The standard error is always redirected too, just like the
         * standard error. Files under considerations are
         * 'c:\Data\my-test-1_res_err.txt' and
         * 'c:\Data\my-test-1_exp_err.txt' in our example. The
         * semantics are almost exactly the same as with the standard
         * output, except if the 'res' (RESult) file is empty, then no
         * 'exp' file will be created either. This is because if a
         * program produces anything on the standard error, it usually
         * already marks error. If the 'exp' file is there, then 'res'
         * must be identical, if the 'exp' file is not there, then
         * 'res' must be empty too on a test run.
         *
         * If some output is expected on the standard error, then the
         * 'exp' file for the standard error should be versioned and
         * deployed along with the test applciation.
         * 
         *
         * 3 Capturing Input
         * ================
         *
         * If 'c:\Data\my-test-1_in.txt' exists, then the standard
         * input is 'redirected' to be read from this file. Therefore
         * if your test function invokes an operation that expects
         * some input on the console, it will come from that text
         * file. If there is no 'in' file yet input is expected from
         * the console, the testMgr user must enter that input
         * manually, during run, interactively.
         *
         * 'In' files should be versioned and deployed along with the
         * test application. There is no support for saving
         * interactively entered text into an 'in' file (definition of
         * input), that is, 'in' files must be created manually by the
         * test case developer.
         *
         *
         * @param name name of the test case, must not be NULL.
         *
         * @param testFunc the free function that can perform some
         * testing. This function may invoke operations that produce
         * output (or even demand input), and the testing is actually
         * done by capturing this IO and comparing with the
         * pre-defined output. The test function, of course, may also
         * use the ususual ITK_EXPECT, ITK_ASSERT and ITK_MSG macros
         * too.
         *
         * @param defFilesBaseName the base file name for the
         * pre-defined tests for this test case
         *
         * @param lenience if defined, will allow lines containing
         * that string differ. This is useful if you have a test ouput
         * that generates the date/time or elapsed time or such thing,
         * which will differ from execution to execution but should be
         * considered as a pass. (Unfortunately no regexp supported.)
         * A special case of leniency is if SuiteTester::REDIRECT_ONLY
         * is passed along - that means that the output/error streams
         * are captured and redirected to files, but they are not
         * tested for equivalence at all. Useful in cases where one
         * wants to generate performance output or other highly
         * volatile output that differs from run to run and the actual
         * content is not that important.
         *
         */
        void add(const char   * name,
                 void        (* testFunc)(TestMgr *),
                 const char   * defFilesBaseName,
                 const char   * lenience = NULL);


        /**
         * Exactly the same semantics of IO capturing as the add
         * overload above, but with member functions instead of free
         * functions.
         */
        template<typename C>
        void add(const char   * name,
                 C            * ptr,
                 void    (C:: * memFunc)(TestMgr *),
                 const char   * defFileBaseName,
                 const char   * lenience = NULL);



        /**
         * @param name the name of a child test
         *
         * @returns the pointer to the child test, if found, otherwise
         * NULL.
         */
        TesterBase * getChild(const char * name) const;


        /**
         * Destructs this test suite.
         */
        virtual ~SuiteTester();


    protected:
        virtual void doRun(TestMgr * testMgr);
    };


    
    /**
     * An exception class to be used to signal panic.
     */
    class PanicExc : public std::exception
    {
    private:
        //
        // private members
        //
        std::string file_;
        size_t      line_;
        std::string msg_;


    public:
        //
        // public operators
        //
        PanicExc(const char * file,
                 size_t       line,
                 const char * format,
                 ...);
            
        const char * file() const;
        size_t line() const;
        const char * msg() const;

        virtual const char * what() const throw();
    };
    


    /**
     * The concept of a common state carried on by a sequence of test
     * operations is abstracted by this interface. See comments for
     * ContextTester.
     */
    class ITestContext
    {
    public:
        //
        // public operators
        //

        /**
         * Sets up this context. If the setup has failed, use the
         * ITK_PANIC macro.
         */
        virtual void setup() throw (PanicExc) = 0;


        /**
         * Tears down this context.
         */
        virtual void tearDown() throw () = 0;


        /**
         * Destructs this context.
         */
        virtual ~ITestContext() = 0;
    };


    /**
     * Creates a context for a sequence (or even hierarchy) of test
     * cases.
     *
     * If the test cases as such that they carry on a common state,
     * that is, subsequent test operations assume a certain state as a
     * result of previous test operations, then you have to somehow
     * manage that state.
     *
     * That state is embodies by an ITestContext implementation. A state
     * can be an object in the process of a context, but it can be
     * equally something that is embodied in a file (or index
     * database) in the file system.
     *
     * When a test context is executed, its context (ITestContext) is
     * first set up, and when it completes execution, it is torn
     * down. The easiest way is to implement the test operations as
     * member functions of an ITestContext implementation, and add those
     * member functions to this test context for execution (see
     * SuiteTester::add()).
     *
     * ITK_ASSERT-s, when fail, always interrupt the execution of test
     * sequence within one test context. That is, a failed ITK_ASSERT
     * will abort the current test context. ITK_ASSERT has the
     * semantics of failing a test in such a way that the common state
     * is so corrupted that there is no whatsoever point in continuing
     * with further test operations within the same test
     * context. (There is always a root context, automatically
     * employed, when calling Itk::TestMgr::test(...)).
     */
    class ContextTester : public SuiteTester
    {
    private:
        //
        // private members
        //
        ITestContext    * context_;

    public:
        //
        // public
        //
        
        /**
         * Constructs a ContextTester.
         *
         * @param name the name of this test (context)
         *
         * @param context the context to use. Ownership of any
         * non-NULL context is transferred to this newly constructed
         * object (it will be deleted when this is destructed). May be
         * NULL, in which case a default context is used - useful in
         * situations when one wants to have a test context to wich
         * ITK_ASSERT and ITK_PANIC macros should fall back to.
         */
        ContextTester(const char      * name,
                      ITestContext    * context);


        /**
         * Destructor.
         */
        virtual ~ContextTester();

    protected:
        /**
         *
         */
        virtual void doRun(TestMgr * testMgr);
    };



}

#endif // ITK_ITKTESTERS_H_
