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
#include <iostream>

#include "itktesters.h"
#include "itktestmgr.h"
#include "testexc.h"
#include "itkimpl.h"

namespace Itk
{
    class BeginEndSentry
    {
    private:
        Itk::TestMgr     * testMgr_;
        Itk::TesterBase  * testerBase_;

    public:
        BeginEndSentry(Itk::TestMgr   * testMgr,
                       Itk::TesterBase * testerBase)
            : testMgr_(testMgr),
              testerBase_(testerBase)
        {
            testMgr_->beginTestCase(testerBase_);
        }

        
        ~BeginEndSentry()
        {
            testMgr_->endTestCase(testerBase_);
        }
    };



    /**
     * Adapts the conventional C-style interface of a free function to
     * the TesterBase interface.
     */
    class FreeFuncAdapter : public Itk::TesterBase
    {
    private:
        void (* testFunc_)(Itk::TestMgr *);

    public:
        FreeFuncAdapter(const char * name,
                        void      (* testFunc)(Itk::TestMgr*))
            : Itk::TesterBase(name),
              testFunc_(testFunc)
        {
            ;
        }

        virtual void doRun(Itk::TestMgr * testMgr)
        {
            (*testFunc_)(testMgr);
        }

        virtual ~FreeFuncAdapter()
        {
            ;
        }
    };



    /**
     * Adapts the conventional C-style interface of a free function to
     * the TesterBase interface. The function will do (most) of its test
     * based on output produced and compared.
     */
    class FreeIOFuncAdapter : public Itk::TesterBase
    {
    private:
        void              (* testFunc_)(Itk::TestMgr *);
        const std::string    defFilesBaseName_;
        const std::string    lenience_;

    public:
        FreeIOFuncAdapter(const char * name,
                          void      (* testFunc)(Itk::TestMgr*),
                          const char * defFilesBaseName,
                          const char * lenience)
            : Itk::TesterBase(name),
              testFunc_(testFunc),
              defFilesBaseName_(defFilesBaseName),
              lenience_(lenience == NULL ? "" : lenience)
        {
            ;
        }

        virtual void doRun(Itk::TestMgr * testMgr)
        {
            Itk::Impl::EvaluateIOCapture(defFilesBaseName_,
                                         testMgr,
                                         testFunc_,
                                         lenience_);
        }

        virtual ~FreeIOFuncAdapter()
        {
            ;
        }
    };


    class SetupTearDownSentry
    {
    private:
        Itk::ITestContext   * context_;

    public:
        SetupTearDownSentry(Itk::ITestContext * context)
            : context_(context)
        {
            context_->setup();
        }

        ~SetupTearDownSentry()
        {
            context_->tearDown();
        }
    };

}



namespace Itk
{
    /****
     * IOCaptureExc
     */
    const char * IOCaptureExc::what() const throw()
    {
        return what_.c_str();
    }


    IOCaptureExc::IOCaptureExc(const char * what)
        : what_(what)
    {
        ;
    }


    IOCaptureExc::IOCaptureExc(const IOCaptureExc & that)
        : what_(that.what_)
    {
        ;
    }



    /****
     * TesterBase
     */
    TesterBase::TesterBase(const char * name)
        : name_(name)
    {
        ;
    }


    const std::string & TesterBase::name() const
    {
        return name_;
    }


    

    void TesterBase::run(TestMgr * testMgr)
    {
        BeginEndSentry
            sentry(testMgr,
                   this);

        doRun(testMgr);
    }


    void TesterBase::printHierarchy(std::ostream & os,
                                    int            indent) const
    {
        indentSpaces(indent,
                     os);
        os << name() << std::endl;
    }


    size_t TesterBase::count() const
    {
        return 1;
    }


    TesterBase::~TesterBase()
    {
        ;
    }


    
    void TesterBase::indentSpaces(int            indent,
                                  std::ostream & os) const
    {
        for (; indent > 0; --indent)
            {
                os << "  ";
            }
    }


    /****
     * SuiteTester
     */
    const char SuiteTester::REDIRECT_ONLY[] = "_ReDiReCtOnLy_";


    SuiteTester::SuiteTester(const char * name)
        : TesterBase(name)
    {
        ;
    }


    void SuiteTester::printHierarchy(std::ostream & os,
                                     int            indent) const
    {
        using namespace std;

        TesterBase::printHierarchy(os,
                                   indent);

        const ContextTester
            * contextTester = dynamic_cast<const ContextTester*>(this);

        // we don't print children of context testers as they
        // cannot be used for partial testing anyway
        if (contextTester == NULL)
            {
                list<TesterBase*>::const_iterator
                    i = testerBases_.begin(),
                    end = testerBases_.end();
                
                for (; i != end; ++i)
                    {
                        (*i)->printHierarchy(os,
                                             indent + 1);
                    }
            }
    }


    size_t SuiteTester::count() const
    {
        size_t
            rv = 0;

        std::list<TesterBase*>::const_iterator
            i = testerBases_.begin(),
            end = testerBases_.end();
        
        for (; i != end; ++i)
            {
                rv += (*i)->count();
            }

        return rv;
    }


    void SuiteTester::add(TesterBase * testerBase)
    {
        testerBases_.push_back(testerBase);
    }


    void SuiteTester::add(const char * name,
                          void (     * testFunc)(TestMgr * testMgr))
    {
        testerBases_.push_back(new FreeFuncAdapter(name,
                                                   testFunc));
    }


    
    void SuiteTester::add(const char * name,
                          void      (* testFunc)(TestMgr *),
                          const char * defFilesBaseName,
                          const char * lenience)
    {
        testerBases_.push_back(new FreeIOFuncAdapter(name,
                                                     testFunc,
                                                     defFilesBaseName,
                                                     lenience));
    }


    void SuiteTester::doRun(TestMgr * testMgr)
    {
        using namespace std;

        list<TesterBase*>::iterator
            i = testerBases_.begin(),
            end = testerBases_.end();

        for (; i != end; ++i)
            {
                (*i)->run(testMgr);
            }
    }


    
    TesterBase * SuiteTester::getChild(const char * name) const
    {
        using namespace std;

        TesterBase
            * rv = NULL;

        list<TesterBase*>::const_iterator
            i = testerBases_.begin(),
            end = testerBases_.end();

        for (; i != end; ++i)
            {
                if ((*i)->name() == name)
                    {
                        break;
                    }
            }

        if (i != end)
            {
                rv = *i;
            }

        return rv;
    }


    SuiteTester::~SuiteTester()
    {
        using namespace std;

        list<TesterBase*>::iterator
            i = testerBases_.begin(),
            end = testerBases_.end();

        for (; i != end; ++i)
            {
                delete *i;
            }
        
        testerBases_.clear();
    }



    /****
     * PanicExc
     */
    PanicExc::PanicExc(const char * file,
                       size_t       line,
                       const char * format,
                       ...)
        : file_(file),
          line_(line)
    {
        char
            msg[256];

        va_list
            args;
        va_start(args,
                 format);

        vsnprintf(msg,
                  sizeof(msg),
                  format,
                  args);

        va_end(args);

        msg_ = msg;
    }
    
    
    const char * PanicExc::file() const
    {
        return file_.c_str();
    }

    
    size_t PanicExc::line() const
    {
        return line_;
    }


    const char * PanicExc::msg() const
    {
        return msg_.c_str();
    }


    const char * PanicExc::what() const throw()
    {
        return msg_.c_str();
    }


    /****
     * ITestContext
     */
    ITestContext::~ITestContext()
    {
        ;
    }


    
    /****
     * DefaultContext
     */
    class DefaultContext : public ITestContext
    {
    public:
        virtual void setup() throw (PanicExc)
        {
            ;
        }

        virtual void tearDown() throw ()
        {
            ;
        }

        virtual ~DefaultContext()
        {
            ;
        }
    };


    /****
     * ContextTester
     */
    ContextTester::ContextTester(const char * name,
                                 ITestContext   * context)
        : SuiteTester(name),
          context_(context == NULL ? new DefaultContext : context)
    {
        ;
    }


    void ContextTester::doRun(TestMgr * testMgr)
    {
        try
            {
                SetupTearDownSentry
                    sentry(context_);

                SuiteTester::doRun(testMgr);
            }
        catch (TestExc & exc)
            {
                ;
            }
        catch (PanicExc & exc)
            {
                testMgr->panic(exc.file(),
                               exc.line(),
                               exc.msg());
            }
        catch (...)
            {
                testMgr->unknownFailure(name().c_str());
                throw;
            }
    }


    ContextTester::~ContextTester()
    {
        delete context_;
    }


}
