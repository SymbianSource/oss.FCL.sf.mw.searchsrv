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

#ifndef ITK_ITKOBSERVERS_H_
#define ITK_ITKOBSERVERS_H_

#include <stddef.h>
#include <iosfwd>
#include <fstream>

namespace Itk
{
    class TestMgr;
    class TesterBase;

    /**
     * Interface to observe events during a test ran by a testMgr.
     */
    class ITestRunObserver
    {
    public:
        //
        // public operators
        //

        /**
         * TODO
         */

        /**
         * At the very beginning of the running session.
         *
         * @param testCount the number of all (leaf) test cases
         *
         * @param baseDirPath the directory under which test cases are
         *        to be found
         */
        virtual void beginRun(TestMgr    * testMgr,
                              size_t       testCount,
                              const char * baseDirPath) = 0;

        virtual void endRun(TestMgr * testMgr) throw () = 0;

        virtual void beginTestCase(TestMgr     * testMgr,
                                   TesterBase  * testerBase) = 0;

        virtual void endTestCase(TestMgr     * testMgr,
                                 TesterBase  * testerBase) = 0;
        
        virtual void expecting(TestMgr     * testMgr,
                               bool          succeeded,
                               const char * expr,
                               const char * file,
                               size_t       line,
                               const char * msg) = 0;
        virtual void asserting(TestMgr     * testMgr,
                               bool          succeeded,
                               const char * expr,
                               const char * file,
                               size_t       line,
                               const char * msg) = 0;
        virtual void unknownFailure(TestMgr          * testMgr,
                                    const char       * contextName) = 0;

        virtual void msg(TestMgr     * testMgr,
                         const char  * file,
                         size_t        line,
                         const char  * msg) = 0;

        virtual void panic(const char * file,
                           size_t       line,
                           const char * msg) = 0;

        virtual void ioCaptureDefined(const char * file,
                                      const char * msg) = 0;

        virtual void ioCaptureError(const char * file,
                                    const char * msg) = 0;

        virtual void report(const char * name,
                            const char * value) = 0;

        /**
         * Destructor
         */
        virtual ~ITestRunObserver() = 0;
    };



    /**
     * Simple Console UI
     */
    class TestRunConsole : public ITestRunObserver
    {
    private:
        //
        // private members
        //
        std::ostream & os_;
        int            indent_;

    public:
        //
        // public operators
        //
        virtual void beginRun(TestMgr    * testMgr,
                              size_t       testCount,
                              const char * baseDirPath);

        virtual void endRun(TestMgr * testMgr) throw ();

        virtual void beginTestCase(TestMgr     * testMgr,
                                   TesterBase  * testerBase);

        virtual void endTestCase(TestMgr     * testMgr,
                                 TesterBase  * testerBase);
        
        virtual void expecting(TestMgr     * testMgr,
                               bool          succeeded,
                               const char * expr,
                               const char * file,
                               size_t       line,
                               const char * msg);
        virtual void asserting(TestMgr     * testMgr,
                               bool          succeeded,
                               const char * expr,
                               const char * file,
                               size_t       line,
                               const char * msg);
        virtual void unknownFailure(TestMgr          * testMgr,
                                    const char       * contextName);

        virtual void msg(TestMgr     * testMgr,
                         const char  * file,
                         size_t        line,
                         const char  * msg);

        virtual void panic(const char * file,
                           size_t       line,
                           const char * msg);

        virtual void ioCaptureDefined(const char * file,
                                      const char * msg);

        virtual void ioCaptureError(const char * file,
                                    const char * msg);

        virtual void report(const char * name,
                            const char * value);
        //
        // lifetime management
        //

        /**
         * Constructor
         */
        TestRunConsole(std::ostream & os);

        /**
         * Destructor
         */
        virtual ~TestRunConsole();


    private:
        //
        // private methods
        //
        void printIndent();
    };



    /**
     * When you want to make use of several test run observers at the
     * same time, you can compose them into one with the use of this
     * class.
     */
    class CompositeTestRunObserver : public ITestRunObserver
    {
    private:
        //
        // private members
        //
        std::list<ITestRunObserver*>   observers_;

    public:
        //
        // public operators
        //
        virtual void beginRun(TestMgr    * testMgr,
                              size_t       testCount,
                              const char * baseDirPath);

        virtual void endRun(TestMgr * testMgr) throw ();

        virtual void beginTestCase(TestMgr     * testMgr,
                                   TesterBase  * testerBase);

        virtual void endTestCase(TestMgr     * testMgr,
                                 TesterBase  * testerBase);
        
        virtual void expecting(TestMgr     * testMgr,
                               bool          succeeded,
                               const char * expr,
                               const char * file,
                               size_t       line,
                               const char * msg);
        virtual void asserting(TestMgr     * testMgr,
                               bool          succeeded,
                               const char * expr,
                               const char * file,
                               size_t       line,
                               const char * msg);
        virtual void unknownFailure(TestMgr          * testMgr,
                                    const char       * contextName);

        virtual void msg(TestMgr     * testMgr,
                         const char  * file,
                         size_t        line,
                         const char  * msg);

        virtual void panic(const char * file,
                           size_t       line,
                           const char * msg);

        virtual void ioCaptureDefined(const char * file,
                                      const char * msg);

        virtual void ioCaptureError(const char * file,
                                    const char * msg);

        virtual void report(const char * name,
                            const char * value);
        //
        // lifetime management
        //

        /**
         * Constructor
         *
         * @param path the absolute path to the file to which to dump
         * everything that happens.
         */
        CompositeTestRunObserver();

        /**
         * Destructor
         */
        virtual ~CompositeTestRunObserver();


        /**
         * Adds a test run observer to this composite. All events to
         * this interface are relayed further to it. This will be the
         * owner of testRunObserver (even if addition fails).
         */
        void add(ITestRunObserver * testRunObserver);
    };




    /**
     * This observer dumps everything to a file, flushing it after
     * each operation. This makes progress slower, but useful if/when
     * the program crashes - at least you can have the latest state
     * preserved.
     *
     * NOTE: !!!!!!!
     * TODO: implement it! This class is only declared, but not
     * implemented at the moment.
     */
    class ProgressDumper : public ITestRunObserver
    {
    private:
        //
        // private members
        //
        std::ofstream   ofs_;

    public:
        //
        // public operators
        //
        virtual void beginRun(TestMgr    * testMgr,
                              size_t       testCount,
                              const char * baseDirPath);

        virtual void endRun(TestMgr * testMgr) throw ();

        virtual void beginTestCase(TestMgr     * testMgr,
                                   TesterBase  * testerBase);

        virtual void endTestCase(TestMgr     * testMgr,
                                 TesterBase  * testerBase);
        
        virtual void expecting(TestMgr     * testMgr,
                               bool          succeeded,
                               const char * expr,
                               const char * file,
                               size_t       line,
                               const char * msg);
        virtual void asserting(TestMgr     * testMgr,
                               bool          succeeded,
                               const char * expr,
                               const char * file,
                               size_t       line,
                               const char * msg);
        virtual void unknownFailure(TestMgr          * testMgr,
                                    const char       * contextName);

        virtual void msg(TestMgr     * testMgr,
                         const char  * file,
                         size_t        line,
                         const char  * msg);

        virtual void panic(const char * file,
                           size_t       line,
                           const char * msg);

        virtual void ioCaptureDefined(const char * file,
                                      const char * msg);

        virtual void ioCaptureError(const char * file,
                                    const char * msg);

        virtual void report(const char * name,
                            const char * value);
        //
        // lifetime management
        //

        /**
         * Constructor
         *
         * @param path the absolute path to the file to which to dump
         * everything that happens.
         */
        ProgressDumper(const char * path);

        /**
         * Destructor
         */
        virtual ~ProgressDumper();
    };



    /**
     * This observer shows the progress via creating a file in the
     * file system. The status is sort of encoded in the file name:
     *
     * <baseDirPath>\itk_<B>.txt
     *
     * where:
     *
     *    o baseDirPath : the argument give to beginRun
     *    o B           : blinkenlicht character, any event will change
     *                    this: W->3->M->E->...
     *
     * On each event, the status is updated and a file with a name
     * reflecting the current status will be created (the old file
     * deleted). On destruction, the file is deleted.
     */
    class ProgressFsDisplayer : public ITestRunObserver
    {
    private:
        //
        // private members
        //
        std::string     baseFilePath_;
        std::string     curFilePath_;

        int             blinkenLichtStatus_;
        

    public:
        //
        // public operators
        //
        virtual void beginRun(TestMgr    * testMgr,
                              size_t       testCount,
                              const char * baseDirPath);

        virtual void endRun(TestMgr * testMgr) throw ();

        virtual void beginTestCase(TestMgr     * testMgr,
                                   TesterBase  * testerBase);

        virtual void endTestCase(TestMgr     * testMgr,
                                 TesterBase  * testerBase);
        
        virtual void expecting(TestMgr     * testMgr,
                               bool          succeeded,
                               const char * expr,
                               const char * file,
                               size_t       line,
                               const char * msg);
        virtual void asserting(TestMgr     * testMgr,
                               bool          succeeded,
                               const char * expr,
                               const char * file,
                               size_t       line,
                               const char * msg);
        virtual void unknownFailure(TestMgr          * testMgr,
                                    const char       * contextName);

        virtual void msg(TestMgr     * testMgr,
                         const char  * file,
                         size_t        line,
                         const char  * msg);

        virtual void panic(const char * file,
                           size_t       line,
                           const char * msg);

        virtual void ioCaptureDefined(const char * file,
                                      const char * msg);

        virtual void ioCaptureError(const char * file,
                                    const char * msg);

        virtual void report(const char * name,
                            const char * value);
        //
        // lifetime management
        //

        /**
         * Constructor
         *
         * @param path the absolute path to the file to which to dump
         * everything that happens.
         */
        ProgressFsDisplayer();

        /**
         * Destructor
         */
        virtual ~ProgressFsDisplayer();


    private:
        //
        // private methods
        //
        void blink();
        void deleteCurFile();
    };


}

#endif // ITK_ITKOBSERVERS_H_
