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
 
#include <functional>

#include "itkimpl.h"

/**
 * Implementation of internal templates stuff for itk.h
 */

namespace Itk
{


    /**
     * Adapts the C++-style interface of a member function to the
     * TesterBase interface.
     */
    template<typename C>
    class MemFuncAdapter : public TesterBase
    {
    private:
        C         * ptr_;
        void (C:: * memFunc_)(TestMgr *);
            
    public:
        MemFuncAdapter(const char * name,
                       C          * ptr,
                       void  (C:: * memFunc)(TestMgr *))
            : TesterBase(name),
              ptr_(ptr),
              memFunc_(memFunc)
        {
            ;
        }


    protected:
        virtual void doRun(TestMgr * testMgr)
        {
            (ptr_->*memFunc_)(testMgr);
        }
    };


    template<typename C>
    void SuiteTester::add(const char * name,
                        C          * ptr,
                        void  (C:: * memFunc)(TestMgr *))
        {
            testerBases_.push_back(new MemFuncAdapter<C>(name,
                                                         ptr,
                                                         memFunc));
        }


    /**
     * Adapts the C++-style interface of a member function to the
     * TesterBase interface. The function will do (most) of its test
     * based on output produced and compared.
     */
    template<typename C>
    class MemIOFuncAdapter : public TesterBase
    {
    private:
        C         * ptr_;
        void (C:: * memFunc_)(TestMgr *);
        const std::string    defFilesBaseName_;
        const std::string    lenience_;
        
    public:
        MemIOFuncAdapter(const char * name,
                         C          * ptr,
                         void  (C:: * memFunc)(TestMgr *),
                         const char * defFilesBaseName,
                         const char * lenience)
            : TesterBase(name),
              ptr_(ptr),
              memFunc_(memFunc),
              defFilesBaseName_(defFilesBaseName),
              lenience_(lenience == NULL ? "" : lenience)
        {
            ;
        }


    protected:
        virtual void doRun(TestMgr * testMgr)
        {
            using namespace std;
            Itk::Impl::EvaluateIOCapture(defFilesBaseName_,
                                         testMgr,
                                         bind1st(mem_fun(memFunc_),
                                                 ptr_),
                                         lenience_);
        }
    };
    

    template<typename C>
    void SuiteTester::add(const char   * name,
                          C            * ptr,
                          void    (C:: * memFunc)(TestMgr *),
                          const char   * defFileBaseName,
                          const char   * lenience)
    {
        testerBases_.push_back(new MemIOFuncAdapter<C>(name,
                                                       ptr,
                                                       memFunc,
                                                       defFileBaseName,
                                                       lenience));
    }



}
