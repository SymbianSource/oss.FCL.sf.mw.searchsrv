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

#include <assert.h>
#include <errno.h>
#include <unistd.h>

#include <fstream>
#include <iostream>
#include <iterator>
#include <list>
#include <string>

#include "cpixstrtools.h"
#include "cpixfstools.h"

#include "itktesters.h"
#include "itktestmgr.h"
#include "itkobservers.h"
#include "testevent.h"
#include "testexc.h"


namespace Itk
{
    class RootSentry
    {
    private:
        ITestRunObserver  * observer_;
        TestMgr           * testMgr_;
        size_t              testCount_;
        std::string         baseDirPath_;

    public:
        RootSentry(ITestRunObserver  * observer,
                   TestMgr           * testMgr,
                   size_t              testCount,
                   const char        * baseDirPath)
            : observer_(observer),
              testMgr_(testMgr),
              testCount_(testCount),
              baseDirPath_(baseDirPath)
        {
            observer_->beginRun(testMgr_,
                                testCount_,
                                baseDirPath_.c_str());
        }

        ~RootSentry()
        {
            observer_->endRun(testMgr_);
        }
    };
}


namespace std
{
    std::ostream & operator<<(std::ostream                    & os,
                              const Itk::TestMgr::CustomReport & customReport)
    {
        os 
            << customReport.first
            << ": "
            << customReport.second;
        
        return os;
    }
}


namespace
{

    void BadFocusMsg(std::list<std::string>::const_iterator i,
                     std::list<std::string>::const_iterator end)
    {
        using namespace std;

        string
            msg = "Cannot match test(s): ";
        
        bool
            first = true;

        for (; i != end; ++i)
            {
                if (first)
                    {
                        first = false;
                    }
                else
                    {
                        msg += '/';
                    }
                
                msg += *i;
            }

        cout << msg << endl;
    }


}



namespace Itk
{

    /****
     * TestMgr
     */
    class RootContextTester : public TesterBase
    {
    private:
        TesterBase   * testerBase_;
    public:
        RootContextTester(TesterBase * testerBase)
            : TesterBase("(root)"),
              testerBase_(testerBase)
        {
            ;
        }

        ~RootContextTester()
        {
            ;
        }

    protected:
        virtual void doRun(TestMgr * testMgr)
        {
            try
                {
                    testerBase_->run(testMgr);
                }
            catch (TestExc & exc)
                {
                    ;
                }
            catch (std::exception & exc)
                {
                    testMgr->unknownFailure(exc.what());
                }
            catch (...)
                {
                    testMgr->unknownFailure("Unknown exception");
                }
        }
    };




    bool TestMgr::run(TesterBase * testerBase,
                      const char * focus)
    {
        if (testerBase == NULL)
            {
                return false;
            }

        rootTesterBase_ = testerBase;
        testerBase = GetTesterBaseToRun(focus,
                                        testerBase);
        if (testerBase == NULL)
            {
                return false;
            }

        failedExpects_ = 0;
        failedAsserts_ = 0;

        testEvents_.clear();
        customReports_.clear();
        clearTesterBaseStack();

        bool
            rv = true;

        std::auto_ptr<RootContextTester>
            rootContextTester( new RootContextTester(testerBase) );

        {
            RootSentry
                sentry(observer_,
                       this,
                       testerBase->count(),
                       ioTestCasesDir_.c_str());
            rootContextTester->run(this);
        }

        rv = (failedExpects_ + failedAsserts_) == 0;

        return rv;
    }


    size_t TestMgr::getFailedExpects() const
    {
        return failedExpects_;
    }


    size_t TestMgr::getFailedAsserts() const
    {
        return failedAsserts_;
    }

    
    void TestMgr::expecting(bool         succeeded,
                            const char * expr,
                            const char * file,
                            size_t       line,
                            const char * msg)
    {
        Cpt::SyncRegion
            sr(mutex_);

        if (!succeeded)
            {
                testEvents_.push_back(TestEvent(expr,
                                                file,
                                                line,
                                                msg));
                ++failedExpects_;
            }
        observer_->expecting(this,
                             succeeded,
                             expr,
                             file,
                             line,
                             msg);
    }


    void TestMgr::asserting(bool         succeeded,
                            const char * expr,
                            const char * file,
                            size_t       line,
                            const char * msg)
    {
        Cpt::SyncRegion
            sr(mutex_);

        if (!succeeded)
            {
                testEvents_.push_back(TestEvent(expr,
                                                file,
                                                line,
                                                msg));
                ++failedAsserts_;
            }
        observer_->asserting(this,
                             succeeded,
                             expr,
                             file,
                             line,
                             msg);
    }


    void TestMgr::unknownFailure(const char * contextName)
    {
        Cpt::SyncRegion
            sr(mutex_);

        testEvents_.push_back(TestEvent(contextName));
        ++failedAsserts_;
        observer_->unknownFailure(this,
                                  contextName);
    }
    
    
    void TestMgr::msg(const char * file,
                      size_t       line,
                      const char * msg)
    {
        Cpt::SyncRegion
            sr(mutex_);

        observer_->msg(this,
                       file,
                       line,
                       msg);
    }

    
    void TestMgr::dbgMsg(const char * msg)
    {
        Cpt::SyncRegion
            sr(mutex_);

        ssize_t
            length = strlen(msg),
            writtenC = 0,
            res;

        while (writtenC < length)
            {
                Cpt_EINTR_RETRY(res,write(dbgConsoleFd_,
                                          msg + writtenC,
                                          length - writtenC));
                if (res == -1)
                    {
                        throw PanicExc(__FILE__, 
                                       __LINE__,
                                       "Debug console output (%d) failed.",
                                       dbgConsoleFd_);
                        
                    }
                else
                    {
                        writtenC += res;
                    }
            }
    }


    void TestMgr::report(const char * name,
                         const char * value)
    {
        Cpt::SyncRegion
            sr(mutex_);

        customReports_.push_back(CustomReport(name,
                                              value));
        observer_->report(name,
                          value);
    }


    void TestMgr::panic(const char * file,
                        size_t       line,
                        const char * msg)
    {
        Cpt::SyncRegion
            sr(mutex_);

        testEvents_.push_back(TestEvent("PANIC",
                                        file,
                                        line,
                                        msg));
        ++failedAsserts_;
        observer_->panic(file,
                         line,
                         msg);
    }


    void TestMgr::setDbgConsoleFd(int fd)
    {
        Cpt::SyncRegion
            sr(mutex_);

        dbgConsoleFd_ = fd;
    }
    

    void TestMgr::ioCaptureDefined(const char * file,
                                   const char * msg)
    {
        Cpt::SyncRegion
            sr(mutex_);

        testEvents_.push_back(TestEvent(file,
                                        msg));
        observer_->ioCaptureDefined(file,
                                    msg);
    }

    
    void TestMgr::ioCaptureError(const char * file,
                                 const char * msg)
    {
        Cpt::SyncRegion
            sr(mutex_);

        testEvents_.push_back(TestEvent(file,
                                        msg));
        ++failedExpects_;
        observer_->ioCaptureError(file,
                                  msg);
    }


    TestMgr::TestEventIterator TestMgr::beginEvents() const
    {
        return testEvents_.begin();
    }
    
    
    TestMgr::TestEventIterator TestMgr::endEvents() const
    {
        return testEvents_.end();
    }
    
    
    TestMgr::CustomReportIterator TestMgr::beginCustomReports() const
    {
        return customReports_.begin();
    }
    
    
    TestMgr::CustomReportIterator TestMgr::endCustomReports() const
    {
        return customReports_.end();
    }

    
    void TestMgr::generateSummary(std::ostream & os) const
    {
        using namespace std;

        bool
            successful = (failedExpects_ + failedAsserts_) == 0;

        os << "SUMMARY" << endl;
        os << "=======" << endl;
        os << "Test run " << (successful ? "OK." : "FAILED.") << endl;

        os << "Events:" << endl;
        copy(beginEvents(),
             endEvents(),
             ostream_iterator<TestEvent>(os, "\n"));

        os << "Custom reports:" << endl;
        copy(beginCustomReports(),
             endCustomReports(),
             ostream_iterator<CustomReport>(os, "\n"));

        if (!successful)
            {
                os 
                    << "Number of failed expects/asserts: "
                    << failedExpects_
                    << '/'
                    << failedAsserts_
                    << endl;
            }
    }



    bool TestMgr::generateSummary(const char * file) const
    {
        using namespace std;

        ofstream
            ofs(file);

        if (ofs.is_open())
            {
                generateSummary(ofs);
            }

        return ofs.is_open();
    }


    TestMgr::TestMgr(ITestRunObserver * observer,
                     const char       * ioTestCasesDir)
        : failedExpects_(0),
          failedAsserts_(0),
          observer_(observer),
          ioTestCasesDir_(ioTestCasesDir),
          dbgConsoleFd_(STDOUT_FILENO)
    {
        ;
    }


    TestMgr::~TestMgr()
    {
        clearTesterBaseStack();
    }

    
    void TestMgr::beginTestCase(TesterBase * testerBase)
    {
        Cpt::SyncRegion
            sr(mutex_);

        testerBaseStack_.push_back(testerBase);
        observer_->beginTestCase(this,
                                 testerBase);
    }


    void TestMgr::endTestCase(TesterBase * testerBase) throw ()
    {
        Cpt::SyncRegion
            sr(mutex_);

        TesterBase
            * tc = *testerBaseStack_.rbegin();
        assert(tc == testerBase);
        testerBaseStack_.pop_back();
        observer_->endTestCase(this,
                               testerBase);
    }

    
    void TestMgr::clearTesterBaseStack()
    {
        if (!testerBaseStack_.empty())
            {
                // the first test case is a RootContextTester,
                // owned and created by this, the rest is not
                // owned by this

                while (testerBaseStack_.size() > 1)
                    {
                        testerBaseStack_.pop_back();
                    }

                if (testerBaseStack_.size() == 1)
                    {
                        TesterBase
                            * root = *testerBaseStack_.rbegin();
                        delete root;
                        testerBaseStack_.pop_back();
                    }
            }
    }


    void TestMgr::ioTestCasesDir(std::string & path) const
    {
        using namespace std;

        path = ioTestCasesDir_;
        Cpt::pathappend(path,
                        skippedNames_.c_str());

        list<TesterBase*>::const_iterator
            i = testerBaseStack_.begin(),
            end = testerBaseStack_.end();

        list<TesterBase*>::const_reverse_iterator
            last = testerBaseStack_.rbegin();

        if (i == end)
            return;

        // root test context (artificially added) is
        // disregarded
        ++i;

        if (i == end)
            return;

        // the root test case is also disregarded
        if ((*i)->name() == rootTesterBase_->name())
            ++i;

        if (i == end)
            return;
        
        while (i != end
               && *i != *last)
            {
                Cpt::pathappend(path,
                                (*i)->name().c_str());
                ++i;
            }

        if (i != end
            && *i == *last)
            {
                TesterBase
                    * tc = *i;
                if (dynamic_cast<SuiteTester*>(tc) != NULL)
                    {
                        Cpt::pathappend(path,
                                        (*i)->name().c_str());
                    }
            }
    }


    TesterBase * TestMgr::GetTesterBaseToRun(const char  * focus,
                                             TesterBase  * rootTesterBase)
    {
        using namespace Itk;

        TesterBase
            * rv = rootTesterBase;

        skippedNames_ = "";

        using namespace std;

        if (focus == NULL)
            {
                return rv;
            }

        list<string>
            tokens;
        Cpt::splitstring(focus,
                         "/",
                         tokens);

        list<string>::const_iterator
            i = tokens.begin(),
            end = tokens.end();

        if (i == end)
            {
                return rv;
            }

        if (*i == rootTesterBase->name())
            {
                ++i;
            }

        string
            lastFragment = "";

        for (; i != end; ++i)
            {
                ContextTester
                    * tc = 
                    dynamic_cast<ContextTester*>(rv);
                
                if (tc != NULL)
                    {
                        break;
                    }

                SuiteTester
                    * suite = dynamic_cast<SuiteTester*>(rv);

                if (suite == NULL)
                    {
                        // we are trying to match the next fragment,
                        // and if this is not a suite, then it won't
                        // have children to match
                        BadFocusMsg(i,
                                    end);
                        rv = NULL;
                        break;
                    }
                else
                    {
                        if (lastFragment != "")
                            {
                                if (skippedNames_ == "")
                                    {
                                        skippedNames_ = lastFragment;
                                    }
                                else
                                    {
                                        Cpt::pathappend(skippedNames_,
                                                        lastFragment.c_str());
                                    }
                            }

                        TesterBase
                            * child = suite->getChild(i->c_str());

                        if (child != NULL)
                            {
                                lastFragment = *i;
                                rv = child;
                            }
                        else
                            {
                                BadFocusMsg(i,
                                            end);
                                rv = NULL;
                                break;
                            }
                    }
            }

        return rv;
    }


}
