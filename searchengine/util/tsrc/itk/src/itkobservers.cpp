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
#include <fcntl.h>
#include <stdio.h>
#include <sys/stat.h>
#include <unistd.h>


#include <exception>
#include <fstream>
#include <iostream>
#include <memory>

#include "cpixfstools.h"

#include "itktesters.h"
#include "itkobservers.h"

namespace Itk
{
    /**
     * ITestRunObserver
     */
    ITestRunObserver::~ITestRunObserver()
    {
        ;
    }


    /*****************************************************************
     * TestRunConsole
     */
    void TestRunConsole::beginRun(TestMgr    * testMgr,
                                  size_t       /* testCount */,
                                  const char * /* baseDirPath */)
    {
        indent_ = 0;
        os_ << "TEST RUN STARTING." << std::endl;
    }

    
    void TestRunConsole::endRun(TestMgr * testMgr) throw ()
    {
        using namespace std;
        os_ << endl << "TEST RUN COMPLETED." << endl;
    }


    void TestRunConsole::beginTestCase(TestMgr     * testMgr,
                                       TesterBase  * testerBase)
    {
        ++indent_;
        
        using namespace std;

        os_ << endl;
        printIndent();
        os_ << testerBase->name() << ' ';
    }


    void TestRunConsole::endTestCase(TestMgr     * testMgr,
                                     TesterBase  * testerBase)
    {
        --indent_;
    }

        
    void TestRunConsole::expecting(TestMgr     * testMgr,
                                   bool          succeeded,
                                   const char  * expr,
                                   const char  * file,
                                   size_t        line,
                                   const char  * msg)
    {
        if (!succeeded)
            {
                os_ << 'E';
            }
    }


    void TestRunConsole::asserting(TestMgr     * testMgr,
                                   bool          succeeded,
                                   const char  * expr,
                                   const char  * file,
                                   size_t        line,
                                   const char  * msg)
    {
        if (!succeeded)
            {
                os_ << 'A';
            }
    }


    void TestRunConsole::unknownFailure(TestMgr          * testMgr,
                                        const char       * contextName)
    {
        os_ << "UNKNOWN ERROR IN CONTEXT: " << contextName;
    }


    void TestRunConsole::msg(TestMgr     * testMgr,
                             const char  * file,
                             size_t        line,
                             const char  * msg)
    {
        using namespace std;

        os_ << endl << msg << endl;
    }


    void TestRunConsole::panic(const char * file,
                               size_t       line,
                               const char * msg)
    {
        using namespace std;

        os_ 
            << "PANIC at "
            << file
            << '/'
            << line
            << ": "
            << msg
            << endl;
    }
    

    void TestRunConsole::ioCaptureDefined(const char * file,
                                          const char * msg)
    {
        using namespace std;

        os_ << "Please check manually and version "
            << file
            << endl;
    }

    
    void TestRunConsole::ioCaptureError(const char * file,
                                        const char * msg)
    {
        using namespace std;

        os_ << "Please check problem with file "
            << file
            << endl;
    }


    void TestRunConsole::report(const char * name,
                                const char * value)
    {
        ;
    }

    
    TestRunConsole::TestRunConsole(std::ostream & os)
        : os_(os),
          indent_(0)
    {
        ;
    }


    TestRunConsole::~TestRunConsole()
    {
        ;
    }


    void TestRunConsole::printIndent()
    {
        for (int i = indent_; i >= 0; --i)
            {
                os_ << "  ";
            }
    }
    


    /*****************************************************************
     * CompositeTestRunObserver
     */
    void CompositeTestRunObserver::beginRun(TestMgr    * testMgr,
                                            size_t       testCount,
                                            const char * baseDirPath)
    {
        std::list<ITestRunObserver*>::iterator
            i = observers_.begin(),
            end = observers_.end();

        for (; i != end; ++i)
            {
                (*i)->beginRun(testMgr,
                               testCount,
                               baseDirPath);
            }
    }


    void CompositeTestRunObserver::endRun(TestMgr * testMgr) throw ()
    {
        std::list<ITestRunObserver*>::iterator
            i = observers_.begin(),
            end = observers_.end();

        for (; i != end; ++i)
            {
                (*i)->endRun(testMgr);
            }
    }


    void CompositeTestRunObserver::beginTestCase(TestMgr     * testMgr,
                                                 TesterBase  * testerBase)
    {
        std::list<ITestRunObserver*>::iterator
            i = observers_.begin(),
            end = observers_.end();

        for (; i != end; ++i)
            {
                (*i)->beginTestCase(testMgr,
                                    testerBase);
            }
    }


    void CompositeTestRunObserver::endTestCase(TestMgr     * testMgr,
                                               TesterBase  * testerBase)
    {
        std::list<ITestRunObserver*>::iterator
            i = observers_.begin(),
            end = observers_.end();

        for (; i != end; ++i)
            {
                (*i)->endTestCase(testMgr,
                                  testerBase);
            }
    }


    void CompositeTestRunObserver::expecting(TestMgr     * testMgr,
                                             bool          succeeded,
                                             const char  * expr,
                                             const char  * file,
                                             size_t        line,
                                             const char  * msg)
    {
        std::list<ITestRunObserver*>::iterator
            i = observers_.begin(),
            end = observers_.end();

        for (; i != end; ++i)
            {
                (*i)->expecting(testMgr,
                                succeeded,
                                expr,
                                file,
                                line,
                                msg);
            }
    }


    void CompositeTestRunObserver::asserting(TestMgr     * testMgr,
                                             bool          succeeded,
                                             const char  * expr,
                                             const char  * file,
                                             size_t        line,
                                             const char  * msg)
    {
        std::list<ITestRunObserver*>::iterator
            i = observers_.begin(),
            end = observers_.end();

        for (; i != end; ++i)
            {
                (*i)->asserting(testMgr,
                                succeeded,
                                expr,
                                file,
                                line,
                                msg);
            }
    }


    void CompositeTestRunObserver::unknownFailure(TestMgr          * testMgr,
                                                  const char       * contextName)
    {
        std::list<ITestRunObserver*>::iterator
            i = observers_.begin(),
            end = observers_.end();

        for (; i != end; ++i)
            {
                (*i)->unknownFailure(testMgr,
                                     contextName);
            }
    }


    void CompositeTestRunObserver::msg(TestMgr     * testMgr,
                                       const char  * file,
                                       size_t        line,
                                       const char  * msg)
    {
        std::list<ITestRunObserver*>::iterator
            i = observers_.begin(),
            end = observers_.end();

        for (; i != end; ++i)
            {
                (*i)->msg(testMgr,
                          file,
                          line,
                          msg);
            }
    }


    void CompositeTestRunObserver::panic(const char * file,
                                         size_t       line,
                                         const char * msg)
    {
        std::list<ITestRunObserver*>::iterator
            i = observers_.begin(),
            end = observers_.end();

        for (; i != end; ++i)
            {
                (*i)->panic(file,
                            line,
                            msg);
            }
    }


    void CompositeTestRunObserver::ioCaptureDefined(const char * file,
                                                    const char * msg)
    {
        std::list<ITestRunObserver*>::iterator
            i = observers_.begin(),
            end = observers_.end();

        for (; i != end; ++i)
            {
                (*i)->ioCaptureDefined(file,
                                       msg);
            }
    }


    void CompositeTestRunObserver::ioCaptureError(const char * file,
                                                  const char * msg)
    {
        std::list<ITestRunObserver*>::iterator
            i = observers_.begin(),
            end = observers_.end();

        for (; i != end; ++i)
            {
                (*i)->ioCaptureError(file,
                                     msg);
            }
    }


    void CompositeTestRunObserver::report(const char * name,
                                          const char * value)
    {
        std::list<ITestRunObserver*>::iterator
            i = observers_.begin(),
            end = observers_.end();

        for (; i != end; ++i)
            {
                (*i)->report(name,
                             value);
            }
    }

    
    CompositeTestRunObserver::CompositeTestRunObserver()
    {
        ;
    }


    CompositeTestRunObserver::~CompositeTestRunObserver()
    {
        std::list<ITestRunObserver*>::iterator
            i = observers_.begin(),
            end = observers_.end();

        for (; i != end; ++i)
            {
                delete *i;
            }
    }


    void CompositeTestRunObserver::add(ITestRunObserver * testRunObserver)
    {
        std::auto_ptr<ITestRunObserver>
            safeGuard(testRunObserver);

        observers_.push_back(testRunObserver);
        safeGuard.release();
    }




    /*****************************************************************
     * ProgressDumper
     */
    void ProgressDumper::beginRun(TestMgr    * testMgr,
                                  size_t       testCount,
                                  const char * baseDirPath)
    {
        ofs_ << "TEST RUN STARTING." << std::endl;
        ofs_.flush();
    }


    void ProgressDumper::endRun(TestMgr * testMgr) throw ()
    {
        ofs_ << "TEST RUN COMPLETED." << std::endl;
        ofs_.flush();
    }


    void ProgressDumper::beginTestCase(TestMgr     * testMgr,
                                       TesterBase  * testerBase)
    {
        ofs_ << "TESTCASE BEGIN: " << testerBase->name() << std::endl;
        ofs_.flush();
    }


    void ProgressDumper::endTestCase(TestMgr     * testMgr,
                                     TesterBase  * testerBase)
    {
        ofs_ << "TESTCASE END." << std::endl;
        ofs_.flush();
    }


    void ProgressDumper::expecting(TestMgr     * testMgr,
                                   bool          succeeded,
                                   const char  * expr,
                                   const char  * file,
                                   size_t        line,
                                   const char  * msg)
    {
        if (!succeeded)
            {
                ofs_ << "EXPECTATION "
                     << expr
                     << " at ("
                     << file
                     << ":"
                     << line
                     << ") failed"
                     << std::endl;
                ofs_.flush();
            }
    }


    void ProgressDumper::asserting(TestMgr     * testMgr,
                                   bool          succeeded,
                                   const char  * expr,
                                   const char  * file,
                                   size_t        line,
                                   const char  * msg)
    {
        if (!succeeded)
            {
                ofs_ << "ASSERTION "
                     << expr
                     << " at ("
                     << file
                     << ":"
                     << line
                     << ") failed"
                     << std::endl;
                ofs_.flush();
            }
    }


    void ProgressDumper::unknownFailure(TestMgr          * testMgr,
                                        const char       * contextName)
    {
        ofs_ << "UNKNOWN FAILURE IN CONTEXT " << contextName << std::endl;
        ofs_.flush();
    }


    void ProgressDumper::msg(TestMgr     * testMgr,
                             const char  * file,
                             size_t        line,
                             const char  * msg)
    {
        ofs_ << "MSG "
             << msg
             << " at ("
             << file
             << ":"
             << line
             << std::endl;
        ofs_.flush();
    }


    void ProgressDumper::panic(const char * file,
                               size_t       line,
                               const char * msg)
    {
        ofs_ << "PANIC "
             << msg
             << " at ("
             << file
             << ":"
             << line
             << std::endl;
        ofs_.flush();
    }


    void ProgressDumper::ioCaptureDefined(const char * file,
                                          const char * msg)
    {
        ofs_ << "IO CAPTURE DEFINED at ("
             << file
             << "): "
             << msg
             << std::endl;
        ofs_.flush();
    }


    void ProgressDumper::ioCaptureError(const char * file,
                                        const char * msg)
    {
        ofs_ << "IO CAPTURE ERROR at ("
             << file
             << "): "
             << msg
             << std::endl;
        ofs_.flush();
    }


    void ProgressDumper::report(const char * name,
                                const char * value)
    {
        ofs_ << "REPORT "
             << name
             << ": "
             << value
             << std::endl;
        ofs_.flush();
    }

    
    ProgressDumper::ProgressDumper(const char * path)
        : ofs_(path)
    {
        if (!ofs_.is_open())
            {
                throw std::exception();
            }
    }


    ProgressDumper::~ProgressDumper()
    {
        ;
    }


    /*****************************************************************
     * ProgressFsDisplayer
     */

    const size_t  BLINKENLICHT_CHARNUM = 4;
    const char    BLINKENLICHT_CHARS[] = "W3ME";


    void ProgressFsDisplayer::beginRun(TestMgr    * testMgr,
                                       size_t       testCount,
                                       const char * baseDirPath)
    {
        baseFilePath_ = baseDirPath;
        if (baseFilePath_[baseFilePath_.length() - 1] != '\\'
            && baseFilePath_[baseFilePath_.length() - 1] != '/')
            {
                // TODO FIX platform-dependent code here
                baseFilePath_ += '\\';
            }
        baseFilePath_ += "itk_";
        
        curFilePath_ = "";

        blinkenLichtStatus_ = 0;

        blink();
    }


    void ProgressFsDisplayer::endRun(TestMgr * testMgr) throw ()
    {
        deleteCurFile();
    }


    void ProgressFsDisplayer::beginTestCase(TestMgr     * testMgr,
                                            TesterBase  * testerBase)
    {
        blink();
    }


    void ProgressFsDisplayer::endTestCase(TestMgr     * testMgr,
                                          TesterBase  * testerBase)
    {

        blink();
    }


    void ProgressFsDisplayer::expecting(TestMgr     * testMgr,
                                        bool          succeeded,
                                        const char  * expr,
                                        const char  * file,
                                        size_t        line,
                                        const char  * msg)
    {
        blink();
    }


    void ProgressFsDisplayer::asserting(TestMgr     * testMgr,
                                        bool          succeeded,
                                        const char  * expr,
                                        const char  * file,
                                        size_t        line,
                                        const char  * msg)
    {
        blink();
    }


    void ProgressFsDisplayer::unknownFailure(TestMgr          * testMgr,
                                             const char       * contextName)
    {
        blink();
    }


    void ProgressFsDisplayer::msg(TestMgr     * testMgr,
                                  const char  * file,
                                  size_t        line,
                                  const char  * msg)
    {
        blink();
    }


    void ProgressFsDisplayer::panic(const char * file,
                                    size_t       line,
                                    const char * msg)
    {
        blink();
    }


    void ProgressFsDisplayer::ioCaptureDefined(const char * file,
                                               const char * msg)
    {
        blink();
    }


    void ProgressFsDisplayer::ioCaptureError(const char * file,
                                             const char * msg)
    {
        blink();
    }


    void ProgressFsDisplayer::report(const char * name,
                                     const char * value)
    {
        blink();
    }

     
    ProgressFsDisplayer::ProgressFsDisplayer()
    {
        ;
    }


    ProgressFsDisplayer::~ProgressFsDisplayer()
    {
        deleteCurFile();
    }


    void ProgressFsDisplayer::blink()
    {
        deleteCurFile();

        // computing next file name to display status with
        ++blinkenLichtStatus_;
        if (blinkenLichtStatus_ == BLINKENLICHT_CHARNUM)
            {
                blinkenLichtStatus_ = 0;
            }

        curFilePath_ = baseFilePath_;
        curFilePath_ += BLINKENLICHT_CHARS[blinkenLichtStatus_];
        curFilePath_ += ".txt";

        // "touch"
        std::ofstream
            ofs(curFilePath_.c_str());
    }


    void ProgressFsDisplayer::deleteCurFile()
    {
        int
            success;
        Cpt_EINTR_RETRY(success,remove(curFilePath_.c_str()));
    }




}
