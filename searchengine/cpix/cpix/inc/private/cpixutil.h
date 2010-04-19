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

#ifndef CPIX_CPIXUTIL_H
#define CPIX_CPIXUTIL_H

#ifdef __SYMBIAN32__
#include "e32cmn.h"
#endif

#include "stdlib.h"

#include "cpixparsetools.h"

#include "cpixexc.h"
#include "wrappertraitsdb.h"


/**
 * Definition is in cpixerror.cpp, but other implementation C++
 * sources will use it too.
 */
extern cpix_Error * CreateError(cpix_ErrorType    errorType,
                                const wchar_t   * msg);

extern cpix_Error * CreateError(cpix_ErrorType    errorType,
                                const char      * msg);


#ifdef __SYMBIAN32__
enum 
    { 
        SYMBIAN_LEAVE_BUFSIZE = 32 
    };
#endif


//
//   ON EXCEPTION TRANSLATION
//
// Of the functions below, XlateExc, Create and Initialize are the
// most important ones and are to be understood first. These are all
// translating exceptions to the cpix way of communicating errors back
// through the C API interface. The fact that they are all templated,
// and want some sort of functor instance is due to the fact that in
// order to be able to catch exceptions and translate them, the actual
// call must happen inside a try-catch block.
//
// This way, there are *only* three places (XlateExc, Create,
// Initialize) where the exceptions dealt with are explicitly
// enumerated. Therefore, if ever there is a new exception type of
// lucene, or another std exception type becomes worthy of special
// handling, or cpix itself declares special new exceptions, then only
// these three places have to be revised. Any other alternative design
// would have been simpler to understand but possibly far more
// troublesome to implement (a LOT of typing) and even worse to
// maintain (duplication of information problem).
//
// TODO These three templates (XlateExc, Create, Initialize) are
// template-instantiated a gazillion times. That causes code
// bloat. So the code fragments using the functors and wrappers could
// be templates that derive from a runtime-polymorphic base class, so
// that the actual translation code would use only that
// runtime-polymorphic operation. That would:
//
//   (a) reduce the number of places where translation is programmed
//       from 3 to 1
//
//   (b) effectively reduce code bloat


/**
 * Translates exceptions that may result from calling functor() to the
 * wrapper instance err_ member.
 */
template<typename WRP,
         typename FUNCTOR>
typename FUNCTOR::result_type XlateExc(WRP     * thisObj,
                                       FUNCTOR         functor)
{
    typedef typename FUNCTOR::result_type RET;

    Cpix::IIdxDb::rotateLog();

    try
        {
            thisObj->err_ = NULL;
            return functor();
        }
    catch (LuceneError & clErr)
        {
            thisObj->err_ 
                = CreateError(ET_CLUCENE_EXC,
                              clErr.twhat());
        }
    catch (Cpt::ITxtCtxtExc & txtCtxtExc)
        {
            thisObj->err_ 
                = CreateError(ET_CPTSYNTAX_EXC,
                              txtCtxtExc.wWhat());
        }
    catch (CpixExc & cpixExc)
        {
            thisObj->err_ 
                = CreateError(ET_CPIX_EXC,
                              cpixExc.wWhat());
        }
    catch (std::exception & stdExc)
        {
            thisObj->err_ 
                = CreateError(ET_STD_CPP_EXC,
                              stdExc.what());
        }
#ifdef __SYMBIAN32__
    catch (XLeaveException & xlExc)
        {
            char
                buf[SYMBIAN_LEAVE_BUFSIZE];
            int
                res = snprintf(buf,
                               SYMBIAN_LEAVE_BUFSIZE,
                               "Symbian Leave: %d",
                               xlExc.GetReason());
            thisObj->err_
                = CreateError(ET_OS_EXC,
                              res > 0 ? buf : "Symbian Leave");
        }
#endif
    catch (...)
        {
            thisObj->err_ 
                = CreateError(ET_UNKNOWN_EXC,
                              static_cast<const wchar_t*>(NULL));
        }

    return RET();
}


/**
 * Is capable of making a call to a static member, or free function
 * taking no argument.
 */
template<typename RET>
struct FreeFuncCaller
{
    typedef RET    result_type;
    RET         (* func_)();

    FreeFuncCaller(RET (*func)())
    : func_(func)
    {
        ;
    }


    RET operator()()
    {
        return (*func_)();
    }
};


template<typename RET>
FreeFuncCaller<RET> CallFreeFunc(RET (*func)())
{
    return FreeFuncCaller<RET>(func);
}


template<typename RET,
         typename ARG1>
struct FreeFuncCaller1
{
    typedef RET      result_type;
    RET           (* func_)(ARG1);
    ARG1             arg1_;

    FreeFuncCaller1(RET (*func)(ARG1),
                    ARG1  arg1)
        : func_(func),
          arg1_(arg1)
    {
        ;
    }


    RET operator()()
    {
        return (*func_)(arg1_);
    }
};


template<typename RET,
         typename ARG1>
FreeFuncCaller1<RET, ARG1> CallFreeFunc(RET (*func)(ARG1),
                                                    ARG1  arg1)
{
    return FreeFuncCaller1<RET, ARG1>(func,
                                      arg1);
}


template<typename RET,
         typename ARG1,
         typename ARG2>
struct FreeFuncCaller2
{
    typedef RET      result_type;
    RET           (* func_)(ARG1, ARG2);
    ARG1             arg1_;
    ARG2             arg2_;

    FreeFuncCaller2(RET (*func)(ARG1, ARG2),
                    ARG1  arg1,
                    ARG2  arg2)
        : func_(func),
          arg1_(arg1),
          arg2_(arg2)
    {
        ;
    }


    RET operator()()
    {
        return (*func_)(arg1_, arg2_);
    }
};


template<typename RET,
         typename ARG1,
         typename ARG2>
FreeFuncCaller2<RET, ARG1, ARG2> CallFreeFunc(RET (*func)(ARG1, ARG2),
                                              ARG1  arg1,
                                              ARG2  arg2)
{
    return FreeFuncCaller2<RET, ARG1, ARG2>(func,
                                            arg1,
                                            arg2);
}


template<typename RET,
         typename ARG1,
         typename ARG2,
         typename ARG3>
struct FreeFuncCaller3
{
    typedef RET      result_type;
    RET           (* func_)(ARG1, ARG2, ARG3);
    ARG1             arg1_;
    ARG2             arg2_;
    ARG3             arg3_;

    FreeFuncCaller3(RET (*func)(ARG1, ARG2, ARG3),
                    ARG1  arg1,
                    ARG2  arg2,
                    ARG3  arg3)
        : func_(func),
          arg1_(arg1),
          arg2_(arg2),
          arg3_(arg3)
    {
        ;
    }


    RET operator()()
    {
        return (*func_)(arg1_, arg2_, arg3_);
    }
};


template<typename RET,
         typename ARG1,
         typename ARG2,
         typename ARG3>
FreeFuncCaller3<RET, ARG1, ARG2, ARG3> CallFreeFunc(RET (*func)(ARG1, ARG2, ARG3),
                                                    ARG1  arg1,
                                                    ARG2  arg2,
                                                    ARG3  arg3)
{
    return FreeFuncCaller3<RET, ARG1, ARG2, ARG3>(func,
                                                  arg1,
                                                  arg2,
                                                  arg3);
}


template<typename RET,
         typename ARG1,
         typename ARG2,
         typename ARG3,
         typename ARG4>
struct FreeFuncCaller4
{
    typedef RET      result_type;
    RET           (* func_)(ARG1, ARG2, ARG3, ARG4);
    ARG1             arg1_;
    ARG2             arg2_;
    ARG3             arg3_;
    ARG4             arg4_;

    FreeFuncCaller4(RET (*func)(ARG1, ARG2, ARG3, ARG4),
                    ARG1  arg1,
                    ARG2  arg2,
                    ARG3  arg3,
                    ARG4  arg4)
        : func_(func),
          arg1_(arg1),
          arg2_(arg2),
          arg3_(arg3),
          arg4_(arg4)
    {
        ;
    }


    RET operator()()
    {
        return (*func_)(arg1_, arg2_, arg3_, arg4_);
    }
};


template<typename RET,
         typename ARG1,
         typename ARG2,
         typename ARG3,
         typename ARG4>
FreeFuncCaller4<RET, ARG1, ARG2, ARG3, ARG4> CallFreeFunc(RET (*func)(ARG1, ARG2, ARG3, ARG4),
                                                          ARG1  arg1,
                                                          ARG2  arg2,
                                                          ARG3  arg3,
                                                          ARG4  arg4)
{
    return FreeFuncCaller4<RET, ARG1, ARG2, ARG3, ARG4>(func,
                                                        arg1,
                                                        arg2,
                                                        arg3,
                                                        arg4);
}



template<typename RET,
         typename ARG1,
         typename ARG2,
         typename ARG3,
         typename ARG4,
         typename ARG5>
struct FreeFuncCaller5
{
    typedef RET      result_type;
    RET           (* func_)(ARG1, ARG2, ARG3, ARG4, ARG5);
    ARG1             arg1_;
    ARG2             arg2_;
    ARG3             arg3_;
    ARG4             arg4_;
    ARG5             arg5_;

    FreeFuncCaller5(RET (*func)(ARG1, ARG2, ARG3, ARG4, ARG5),
                    ARG1  arg1,
                    ARG2  arg2,
                    ARG3  arg3,
                    ARG4  arg4,
                    ARG5  arg5)
        : func_(func),
          arg1_(arg1),
          arg2_(arg2),
          arg3_(arg3),
          arg4_(arg4),
          arg5_(arg5)
    {
        ;
    }


    RET operator()()
    {
        return (*func_)(arg1_, arg2_, arg3_, arg4_, arg5_);
    }
};


template<typename RET,
         typename ARG1,
         typename ARG2,
         typename ARG3,
         typename ARG4,
         typename ARG5>
FreeFuncCaller5<RET, ARG1, ARG2, ARG3, ARG4, ARG5> 
CallFreeFunc(RET (*func)(ARG1, ARG2, ARG3, ARG4, ARG5),
             ARG1  arg1,
             ARG2  arg2,
             ARG3  arg3,
             ARG4  arg4,
             ARG5  arg5)
{
    return FreeFuncCaller5<RET, ARG1, ARG2, ARG3, ARG4, ARG5>(func,
                                                              arg1,
                                                              arg2,
                                                              arg3,
                                                              arg4,
                                                              arg5);
}



template<typename RET,
         typename ARG1,
         typename ARG2,
         typename ARG3,
         typename ARG4,
         typename ARG5,
         typename ARG6>
struct FreeFuncCaller6
{
    typedef RET      result_type;
    RET           (* func_)(ARG1, ARG2, ARG3, ARG4, ARG5, ARG6);
    ARG1             arg1_;
    ARG2             arg2_;
    ARG3             arg3_;
    ARG4             arg4_;
    ARG5             arg5_;
    ARG6             arg6_;

    FreeFuncCaller6(RET (*func)(ARG1, ARG2, ARG3, ARG4, ARG5, ARG6),
                    ARG1  arg1,
                    ARG2  arg2,
                    ARG3  arg3,
                    ARG4  arg4,
                    ARG5  arg5,
                    ARG6  arg6)
        : func_(func),
          arg1_(arg1),
          arg2_(arg2),
          arg3_(arg3),
          arg4_(arg4),
          arg5_(arg5),
          arg6_(arg6)
    {
        ;
    }


    RET operator()()
    {
        return (*func_)(arg1_, arg2_, arg3_, arg4_, arg5_, arg6_);
    }
};


template<typename RET,
         typename ARG1,
         typename ARG2,
         typename ARG3,
         typename ARG4,
         typename ARG5,
         typename ARG6>
FreeFuncCaller6<RET, ARG1, ARG2, ARG3, ARG4, ARG5, ARG6> 
CallFreeFunc(RET (*func)(ARG1, ARG2, ARG3, ARG4, ARG5, ARG6),
             ARG1  arg1,
             ARG2  arg2,
             ARG3  arg3,
             ARG4  arg4,
             ARG5  arg5,
             ARG6  arg6)
{
    return FreeFuncCaller6<RET, ARG1, ARG2, ARG3, ARG4, ARG5, ARG6>(func,
                                                                    arg1,
                                                                    arg2,
                                                                    arg3,
                                                                    arg4,
                                                                    arg5,
                                                                    arg6);
}


/******************************************************************
 *
 * The "traits" HndlTraits can tell at compile time whether a public
 * api class (cpix_XXX) refers to its native counterpart via direct
 * pointer or handle.
 *
 */
template<typename WRP>
struct HndlTraits
{
    enum { V = 0 };
};


template<>
struct HndlTraits<cpix_IdxDb>
{
    enum { V = 1 };
};


template<>
struct HndlTraits<cpix_IdxSearcher>
{
    enum { V = 1 };
};


/*********************************************************************
 *
 * Accessing native instance from the wrapper instance can fail with
 * an exception (IIdxDb::getPtr() can throw). Therefore, native
 * pointer resolution must be done during a functor call (like
 * MemFuncCaller<>::operator()), and not before construction of the
 * functor itself (like MemFuncCaller<>::MemFuncCaller). The reason is
 * that if we did native pointer resolution at (actually, before)
 * FUNCTOR construction time, then any exceptions thrown will not be
 * translated by XlateExc - they will mess up the client, as it does
 * not expect to get exceptions out from a C API like CPIX.
 *
 * This template structure knows how to get the native pointer based
 * on a wrapper instance.
 */
template<typename WRP,
         int      ISHANDLEBASED>
struct PtrResolver
{
    typedef typename WrapperTraits<WRP>::NativeClass NativeClass;

    WRP         * ptr_;

    
    PtrResolver(WRP   * ptr)
        : ptr_(ptr)
    {
        ;
    }
    

    NativeClass * ptr()
    {
        return Cast2Native(ptr_);
    }
};


template<typename WRP>
struct PtrResolver<WRP, 1>
{
    typedef typename WrapperTraits<WRP>::NativeClass NativeClass;

    WRP         * ptr_;


    PtrResolver(WRP * ptr)
        : ptr_(ptr)
    {
        ;
    }


    NativeClass * ptr()
    {
        return NativeClass::getPtr(ptr_->handle_);
    }
};



/**
 * Is capable of making a call to a non-static, non-const member
 * function taking no argument.
 */
template<typename RET,
         typename PTR_RESOLVER,
         typename NAT>
struct MemFuncCaller
{
    typedef RET result_type;

    PTR_RESOLVER       ptrResolver_;
    RET (NAT:: * mFunc_)();

    MemFuncCaller(PTR_RESOLVER       ptrResolver,
                  RET (NAT:: * mFunc)())
        : ptrResolver_(ptrResolver),
          mFunc_(mFunc)
    {
        ;
    }

    RET operator()()
    {
        NAT
            * ptr = ptrResolver_.ptr();
        return (ptr->*mFunc_)();
    }
};


template<typename RET,
         typename PTR_RESOLVER,
         typename NAT>
struct ConstMemFuncCaller
{
    typedef RET result_type;

    PTR_RESOLVER       ptrResolver_;
    RET (NAT:: * mFunc_)() const;

    ConstMemFuncCaller(PTR_RESOLVER       ptrResolver,
                      RET (NAT:: * mFunc)() const)
        : ptrResolver_(ptrResolver),
          mFunc_(mFunc)
    {
        ;
    }

    RET operator()()
    {
        NAT
            * ptr = ptrResolver_.ptr();
        return (ptr->*mFunc_)();
    }
};


/**
 * Is capable of making a call to a non-static, non-const member
 * function taking one argument.
 */
template<typename RET,
         typename PTR_RESOLVER,
         typename NAT,
         typename ARG1>
struct MemFuncCaller1
{
    typedef RET result_type;

    PTR_RESOLVER       ptrResolver_;
    RET (NAT:: * mFunc_)(ARG1);
    ARG1               arg1_;

    MemFuncCaller1(PTR_RESOLVER       ptrResolver,
                   RET (NAT:: * mFunc)(ARG1),
                   ARG1               arg1)
        : ptrResolver_(ptrResolver),
          mFunc_(mFunc),
          arg1_(arg1)
    {
        ;
    }

    RET operator()()
    {
        NAT
            * ptr = ptrResolver_.ptr();
        return (ptr->*mFunc_)(arg1_);
    }
};


template<typename RET,
         typename PTR_RESOLVER,
         typename NAT,
         typename ARG1>
struct MemFuncCaller1r
{
    typedef RET result_type;

    PTR_RESOLVER       ptrResolver_;
    RET (NAT:: * mFunc_)(ARG1 &);
    ARG1             & arg1_;

    MemFuncCaller1r(PTR_RESOLVER       ptrResolver,
                    RET (NAT:: * mFunc)(ARG1 &),
                    ARG1             & arg1)
        : ptrResolver_(ptrResolver),
          mFunc_(mFunc),
          arg1_(arg1)
    {
        ;
    }

    RET operator()()
    {
        NAT
            * ptr = ptrResolver_.ptr();
        return (ptr->*mFunc_)(arg1_);
    }
};


template<typename RET,
         typename PTR_RESOLVER,
         typename NAT,
         typename ARG1>
struct ConstMemFuncCaller1
{
    typedef RET result_type;

    PTR_RESOLVER       ptrResolver_;
    RET (NAT:: * mFunc_)(ARG1) const;
    ARG1               arg1_;

    ConstMemFuncCaller1(PTR_RESOLVER       ptrResolver,
                        RET (NAT:: * mFunc)(ARG1) const,
                        ARG1               arg1)
        : ptrResolver_(ptrResolver),
          mFunc_(mFunc),
          arg1_(arg1)
    {
        ;
    }

    RET operator()()
    {
        NAT
            * ptr = ptrResolver_.ptr();
        return (ptr->*mFunc_)(arg1_);
    }
};


/**
 * Is capable of making a call to a non-static, non-const member
 * function taking two arguments.
 */
template<typename RET,
         typename PTR_RESOLVER,
         typename NAT,
         typename ARG1,
         typename ARG2>
struct MemFuncCaller2
{
    typedef RET result_type;

    PTR_RESOLVER       ptrResolver_;
    RET (NAT:: * mFunc_)(ARG1, ARG2);
    ARG1               arg1_;
    ARG2               arg2_;

    MemFuncCaller2(PTR_RESOLVER       ptrResolver,
                   RET (NAT:: * mFunc)(ARG1, ARG2),
                   ARG1               arg1,
                   ARG2               arg2)
        : ptrResolver_(ptrResolver),
          mFunc_(mFunc),
          arg1_(arg1),
          arg2_(arg2)
    {
        ;
    }

    RET operator()()
    {
        NAT
            * ptr = ptrResolver_.ptr();
        return (ptr->*mFunc_)(arg1_, arg2_);
    }
};

/**
 * Is capable of making a call to a non-static, non-const member
 * function taking two arguments.
 */
template<typename RET,
         typename PTR_RESOLVER,
         typename NAT,
         typename ARG1,
         typename ARG2,
         typename ARG3>
struct MemFuncCaller3
{
    typedef RET result_type;

    PTR_RESOLVER       ptrResolver_;
    RET (NAT:: * mFunc_)(ARG1, ARG2, ARG3);
    ARG1               arg1_;
    ARG2               arg2_;
    ARG3               arg3_;

    MemFuncCaller3(PTR_RESOLVER       ptrResolver,
                   RET (NAT:: * mFunc)(ARG1, ARG2, ARG3),
                   ARG1               arg1,
                   ARG2               arg2,
                   ARG3               arg3)
        : ptrResolver_(ptrResolver_),
          mFunc_(mFunc),
          arg1_(arg1),
          arg2_(arg2),
		  arg3_(arg3)
    {
        ;
    }

    RET operator()()
    {
        NAT
            * ptr = ptrResolver_.ptr();
        return (ptr->*mFunc_)(arg1_, arg2_, arg3_);
    }
};

/**
 * Is capable of making a call to a non-static, non-const member
 * function taking two arguments.
 */
template<typename RET,
         typename PTR_RESOLVER,
         typename NAT,
         typename ARG1,
         typename ARG2,
         typename ARG3,
         typename ARG4>
struct MemFuncCaller4
{
    typedef RET result_type;

    PTR_RESOLVER       ptrResolver_;
    RET (NAT:: * mFunc_)(ARG1, ARG2, ARG3, ARG4);
    ARG1               arg1_;
    ARG2               arg2_;
    ARG3               arg3_;
    ARG4               arg4_;

    MemFuncCaller4(PTR_RESOLVER       ptrResolver,
                   RET (NAT:: * mFunc)(ARG1, ARG2, ARG3, ARG4),
                   ARG1               arg1,
                   ARG2               arg2,
                   ARG3               arg3,
                   ARG4               arg4)
        : ptrResolver_(ptrResolver),
          mFunc_(mFunc),
          arg1_(arg1),
          arg2_(arg2),
		  arg3_(arg3),
		  arg4_(arg4)
    {
        ;
    }

    RET operator()()
    {
        NAT
            * ptr = ptrResolver_.ptr();
        return (ptr->*mFunc_)(arg1_, arg2_, arg3_, arg4_);
    }
};

template<typename RET,
         typename PTR_RESOLVER,
         typename NAT,
         typename ARG1,
         typename ARG2>
struct ConstMemFuncCaller2
{
    typedef RET result_type;

    PTR_RESOLVER       ptrResolver_;
    RET (NAT:: * mFunc_)(ARG1, ARG2) const;
    ARG1               arg1_;
    ARG2               arg2_;

    ConstMemFuncCaller2(PTR_RESOLVER       ptrResolver,
                        RET (NAT:: * mFunc)(ARG1, ARG2) const,
                        ARG1               arg1,
                        ARG2               arg2)
        : ptrResolver_(ptrResolver),
          mFunc_(mFunc),
          arg1_(arg1),
          arg2_(arg2)
    {
        ;
    }

    RET operator()()
    {
        NAT
            * ptr = ptrResolver_.ptr();
        return (ptr->*mFunc_)(arg1_, arg2_);
    }
};


/**
 * Is capable of making a call to a non-static, non-const member
 * function taking seven arguments.
 */
template<typename RET,
         typename PTR_RESOLVER,
         typename NAT,
         typename ARG1,
         typename ARG2,
         typename ARG3,
         typename ARG4,
         typename ARG5,
         typename ARG6,
         typename ARG7>
struct MemFuncCaller7
{
    typedef RET result_type;

    PTR_RESOLVER       ptrResolver_;
    RET (NAT:: * mFunc_)(ARG1, ARG2, ARG3, ARG4, ARG5, ARG6, ARG7);
    ARG1               arg1_;
    ARG2               arg2_;
    ARG3               arg3_;
    ARG4               arg4_;
    ARG5               arg5_;
    ARG6               arg6_;
    ARG7               arg7_;

    MemFuncCaller7(PTR_RESOLVER       ptrResolver,
                   RET (NAT:: * mFunc)(ARG1, ARG2, ARG3, ARG4, ARG5, ARG6, ARG7),
                   ARG1               arg1,
                   ARG2               arg2,
                   ARG3               arg3,
                   ARG4               arg4,
                   ARG5               arg5,
                   ARG6               arg6,
                   ARG7               arg7)
        : ptrResolver_(ptrResolver),
          mFunc_(mFunc),
          arg1_(arg1),
          arg2_(arg2),
          arg3_(arg3),
          arg4_(arg4),
          arg5_(arg5),
          arg6_(arg6),
          arg7_(arg7)
    {
        ;
    }

    RET operator()()
    {
        NAT
            * ptr = ptrResolver_.ptr();
        return (ptr->*mFunc_)(arg1_, arg2_, arg3_, arg4_, arg5_, arg6_, arg7_);
    }
};




/** 
 * The Caller family of functions create void-functors.
 *
 * The WRP wrapper may have ptr_ or handle_ member, it can resolve the
 * native instance through either of which.
 */
template<typename WRP,
         typename RET,
         typename NAT>
MemFuncCaller<RET, PtrResolver<WRP, HndlTraits<WRP>::V>, NAT>
Caller(WRP        * thisObj,
              RET (NAT:: * mFunc)())
{
    typedef PtrResolver<WRP, HndlTraits<WRP>::V> PR;

    typedef MemFuncCaller<RET, PR, NAT> MFC;

    return MFC(PR(thisObj),
               mFunc);
}


template<typename WRP,
         typename RET,
         typename NAT,
         typename ARG1>
MemFuncCaller1<RET, PtrResolver<WRP, HndlTraits<WRP>::V>, NAT, ARG1>
Caller(WRP        * thisObj,
              RET (NAT:: * mFunc)(ARG1),
              const ARG1       & arg1)
{
    typedef PtrResolver<WRP, HndlTraits<WRP>::V> PR;

    typedef MemFuncCaller1<RET, PR, NAT, 
        ARG1> MFC;

    return MFC(PR(thisObj),
               mFunc,
               arg1);
}

template<typename WRP,
         typename RET,
         typename NAT,
         typename ARG1,
         typename ARG2>
MemFuncCaller2<RET, PtrResolver<WRP, HndlTraits<WRP>::V>, NAT, ARG1, ARG2>
Caller(WRP        * thisObj,
              RET (NAT:: * mFunc)(ARG1, ARG2),
              const ARG1       & arg1,
              const ARG2       & arg2)
{
    typedef PtrResolver<WRP, HndlTraits<WRP>::V> PR;

    typedef MemFuncCaller2<RET, PR, NAT,
        ARG1,
        ARG2> MFC;

    return MFC(PR(thisObj),
               mFunc,
               arg1,
               arg2);
}

template<typename WRP,
         typename RET,
         typename NAT,
         typename ARG1,
         typename ARG2,
         typename ARG3>
MemFuncCaller3<RET, PtrResolver<WRP, HndlTraits<WRP>::V>, NAT, ARG1, ARG2, ARG3>
Caller(WRP        * thisObj,
              RET (NAT:: * mFunc)(ARG1, ARG2, ARG3),
              const ARG1       & arg1,
              const ARG2       & arg2,
              const ARG3       & arg3)
{
    typedef PtrResolver<WRP, HndlTraits<WRP>::V> PR;

    typedef MemFuncCaller3<RET, PR, NAT,
        ARG1,
        ARG2,
        ARG3> MFC;

    return MFC(PR(thisObj),
               mFunc,
               arg1,
               arg2,
               arg3);
}

template<typename WRP,
         typename RET,
         typename NAT,
         typename ARG1,
         typename ARG2,
         typename ARG3,
         typename ARG4>
MemFuncCaller4<RET, PtrResolver<WRP, HndlTraits<WRP>::V>, NAT, ARG1, ARG2, ARG3, ARG4>
Caller(WRP        * thisObj,
              RET (NAT:: * mFunc)(ARG1, ARG2, ARG3, ARG4),
              const ARG1       & arg1,
              const ARG2       & arg2,
              const ARG3       & arg3,
              const ARG4	   & arg4)
{
    typedef PtrResolver<WRP, HndlTraits<WRP>::V> PR;

    typedef MemFuncCaller4<RET, PR, NAT,
        ARG1,
        ARG2,
        ARG3,
        ARG4> MFC;

    return MFC(PR(thisObj),
               mFunc,
               arg1,
               arg2,
               arg3,
               arg4);
}

template<typename WRP,
         typename RET,
         typename NAT,
         typename ARG1,
         typename ARG2,
         typename ARG3,
         typename ARG4,
         typename ARG5,
         typename ARG6,
         typename ARG7>
MemFuncCaller7<RET, PtrResolver<WRP, HndlTraits<WRP>::V>, NAT, ARG1, ARG2, ARG3, ARG4, ARG5, ARG6, ARG7>
Caller(WRP        * thisObj,
              RET (NAT:: * mFunc)(ARG1, ARG2, ARG3, ARG4, ARG5, ARG6, ARG7),
              ARG1       arg1,
              ARG2       arg2,
              ARG3       arg3,
              ARG4       arg4,
              ARG5       arg5,
              ARG6       arg6,
              ARG7       arg7)
{
    typedef PtrResolver<WRP, HndlTraits<WRP>::V> PR;

    typedef MemFuncCaller7<RET, PR, NAT,
        ARG1,
        ARG2,
        ARG3,
        ARG4,
        ARG5,
        ARG6,
        ARG7> MFC;

    return MFC(PR(thisObj),
               mFunc,
               arg1,
               arg2,
               arg3,
               arg4,
               arg5,
               arg6,
               arg7);
}


template<typename WRP,
         typename RET,
         typename NAT>
ConstMemFuncCaller<RET, PtrResolver<WRP, HndlTraits<WRP>::V>, NAT>
Caller(WRP        * thisObj,
           RET (NAT:: * mFunc)() const)
{
    typedef PtrResolver<WRP, HndlTraits<WRP>::V> PR;

    typedef ConstMemFuncCaller<RET, PR, NAT> MFC;
    
    return MFC(PR(thisObj),
               mFunc);
}


template<typename WRP,
         typename RET,
         typename NAT,
         typename ARG1>
MemFuncCaller1r<RET, PtrResolver<WRP, HndlTraits<WRP>::V>, NAT, ARG1>
Caller(WRP        * thisObj,
           RET (NAT:: * mFunc)(ARG1 &),
           ARG1             & arg1)
{
    typedef PtrResolver<WRP, HndlTraits<WRP>::V> PR;

    typedef MemFuncCaller1r<RET, PR, NAT,
        ARG1> MFCr;

    return MFC(PR(thisObj),
               mFunc,
               arg1);
}


template<typename WRP,
         typename RET,
         typename NAT,
         typename ARG1>
ConstMemFuncCaller1<RET, PtrResolver<WRP, HndlTraits<WRP>::V>, NAT, ARG1>
Caller(WRP        * thisObj,
           RET (NAT:: * mFunc)(ARG1) const,
           ARG1               arg1)
{
    typedef PtrResolver<WRP, HndlTraits<WRP>::V> PR;

    typedef ConstMemFuncCaller1<RET, PR, NAT,
        ARG1> MFC;

    return MFC(PR(thisObj),
               mFunc,
               arg1);
}

template<typename WRP,
         typename RET,
         typename NAT,
         typename ARG1,
         typename ARG2>
ConstMemFuncCaller2<RET, PtrResolver<WRP, HndlTraits<WRP>::V>, NAT, ARG1, ARG2>
Caller(WRP        * thisObj,
           RET (NAT:: * mFunc)(ARG1, ARG2) const,
           ARG1               arg1,
           ARG2               arg2)
{
    typedef PtrResolver<WRP, HndlTraits<WRP>::V> PR;

    typedef ConstMemFuncCaller2<RET, PR, NAT,
        ARG1,
        ARG2> MFC;

    return MFC(PR(thisObj),
               mFunc,
               arg1,
               arg2);
}




/**
 * Creates a wrapper for an already existing native instance, and does
 * not do anything with it, except reporting errors through result if
 * it failed, or setting its error status to null if succeeded.
 *
 * @param result pointer to the structure through which errors are
 * communicated back, it is usually a pointer to a cpix_Result struct
 * (not always, though)
 */
template<typename WRP,
         typename RES_CLASS>
WRP * 
CreateWrapper_Pure(RES_CLASS  * result)
{
    WRP
        * rv = NULL;

    rv = reinterpret_cast<WRP*>(malloc(sizeof(WRP)));
    if (rv == NULL)
        {
            result->err_ 
                = CreateError(ET_STD_BAD_ALLOC_EXC,
                              L"Failed to malloc a wrapper");
        }
    else
        {
            result->err_ = NULL;
        }

    return rv;
}


/**
 * Creates a wrapper for an already existing native instance, and
 * initializes it with the native pointer.
 *
 * @param p a pointer to the existing native instance we need to wrap
 * here
 *
 * @param result pointer to the structure through which errors are
 * communicated back, it is usually a pointer to a cpix_Result struct
 * (not always, though)
 */
template<typename NAT_PTR,
         typename RES_CLASS,
         typename WRP>
void
CreateWrapper(NAT_PTR        p,
              RES_CLASS   *  result,
              WRP   *& wrapper)
{
    wrapper = NULL;
    wrapper = CreateWrapper_Pure<WRP, RES_CLASS>(result);

    if (wrapper != NULL)
        {
            wrapper->ptr_ = p;
            wrapper->err_ = NULL;
        }
    else
        {
            delete p;
        }
}


/**
 * Performs a creation of a native object and wraps it to it's
 * required wrapper, while translating all exceptions thrown to the
 * result instance given as first parameter.
 */
template<typename CTORFUNCTOR>
typename CTORFUNCTOR::WrpClass * Create(cpix_Result * result,
                                        CTORFUNCTOR   ctorFunctor)
{
    typedef typename CTORFUNCTOR::WrpClass WrpClass;

    Cpix::IIdxDb::rotateLog();

    WrpClass
        * rv = NULL;
    
    try
        {
            // NOTE CTORFUNCTOR returns, most of the time, a pointer
            // to a native instance - and that's what the
            // implementation of CreateWrapper above expects. However,
            // if your CTORFUNCTION has different return semantics,
            // like handlers, then you can create your own version of
            // CreateWrapper too.
            CreateWrapper(ctorFunctor(),
                          result,
                          rv);
        }
    catch (LuceneError & clErr)
        {
            result->err_ 
                = CreateError(ET_CLUCENE_EXC,
                              clErr.twhat());
        }
    catch (Cpt::ITxtCtxtExc & txtCtxtExc)
        {
            result->err_ 
                = CreateError(ET_CPTSYNTAX_EXC,
                              txtCtxtExc.wWhat());
        }
    catch (CpixExc & cpixExc)
        {
            result->err_ 
                = CreateError(ET_CPIX_EXC,
                              cpixExc.wWhat());
        }
    catch (std::exception & stdExc)
        {
            result->err_ 
                = CreateError(ET_STD_CPP_EXC,
                              stdExc.what());
        }
#ifdef __SYMBIAN32__
    catch (XLeaveException & xlExc)
        {
            char
                buf[SYMBIAN_LEAVE_BUFSIZE];
            int
                res = snprintf(buf,
                               SYMBIAN_LEAVE_BUFSIZE,
                               "Symbian Leave: %d",
                               xlExc.GetReason());
            result->err_
                = CreateError(ET_OS_EXC,
                              res > 0 ? buf : "Symbian Leave");
        }
#endif
    catch (...)
        {
            result->err_ 
                = CreateError(ET_UNKNOWN_EXC,
                              static_cast<const wchar_t*>(NULL));
        }
        
    return rv;
}



/**
 * Performs an initialization of an existing wrapper, by creating a
 * corresponding native instance. Translation of caught exceptions are
 * done, and they are communicated back through the given wrapper
 * instance.
 */
template<typename CTORFUNCTOR>
void Initialize(typename CTORFUNCTOR::WrpClass * wrapper,
                CTORFUNCTOR                      ctorFunctor)
{
    typedef typename CTORFUNCTOR::WrpClass WrpClass;

    Cpix::IIdxDb::rotateLog();

    try
        {
            wrapper->err_ = NULL;
            wrapper->ptr_ = NULL;

            typename WrapperTraits<WrpClass>::NativeClass
                * p = ctorFunctor();

            wrapper->ptr_ = p;
        }
    catch (LuceneError & clErr)
        {
            wrapper->err_ 
                = CreateError(ET_CLUCENE_EXC,
                              clErr.twhat());
        }
    catch (Cpt::ITxtCtxtExc & txtCtxtExc)
        {
            wrapper->err_ 
                = CreateError(ET_CPTSYNTAX_EXC,
                              txtCtxtExc.wWhat());
        }
    catch (CpixExc & cpixExc)
        {
            wrapper->err_ 
                = CreateError(ET_CPIX_EXC,
                              cpixExc.wWhat());
        }
    catch (std::exception & stdExc)
        {
            wrapper->err_ 
                = CreateError(ET_STD_CPP_EXC,
                              stdExc.what());
        }
#ifdef __SYMBIAN32__
    catch (XLeaveException & xlExc)
        {
            char
                buf[SYMBIAN_LEAVE_BUFSIZE];
            int
                res = snprintf(buf,
                               SYMBIAN_LEAVE_BUFSIZE,
                               "Symbian Leave: %d",
                               xlExc.GetReason());
            wrapper->err_
                = CreateError(ET_OS_EXC,
                              res > 0 ? buf : "Symbian Leave");
        }
#endif
    catch (...)
        {
            wrapper->err_ 
                = CreateError(ET_UNKNOWN_EXC,
                              static_cast<const wchar_t*>(NULL));
        }
}



template<typename WRP>
class CtorFunctor
{
public:
    typedef typename WrapperTraits<WRP>::NativeClass NativeClass;
    typedef WRP WrpClass;

    NativeClass * operator()()
    {
        return new NativeClass;
    }
};


template<typename WRP,
         typename ARG1>
class CtorFunctor1
{
    
    ARG1   arg1_;
public:
    typedef typename WrapperTraits<WRP>::NativeClass NativeClass;
    typedef WRP WrpClass;

    CtorFunctor1(ARG1        arg1)
        : arg1_(arg1)
    {
        ;
    }

    NativeClass * operator()()
    {
        return new NativeClass(arg1_);
    }
};


template<typename WRP,
         typename ARG1,
         typename ARG2>
class CtorFunctor2
{
    
    ARG1   arg1_;
    ARG2   arg2_;
public:
    typedef typename WrapperTraits<WRP>::NativeClass NativeClass;
    typedef WRP WrpClass;

    CtorFunctor2(ARG1        arg1,
                 ARG2        arg2)
        : arg1_(arg1),
          arg2_(arg2)
    {
        ;
    }


    NativeClass * operator()()
    {
        return new NativeClass(arg1_,
                               arg2_);
    }
};



template<typename WRP,
         typename ARG1,
         typename ARG2,
         typename ARG3>
class CtorFunctor3
{
    
    ARG1   arg1_;
    ARG2   arg2_;
    ARG3   arg3_;
public:
    typedef typename WrapperTraits<WRP>::NativeClass NativeClass;
    typedef WRP WrpClass;

    CtorFunctor3(ARG1        arg1,
                 ARG2        arg2,
                 ARG3        arg3)
        : arg1_(arg1),
          arg2_(arg2),
          arg3_(arg3)
    {
        ;
    }


    NativeClass * operator()()
    {
        return new NativeClass(arg1_,
                               arg2_,
                               arg3_);
    }
};

template<typename WRP,
         typename ARG1,
         typename ARG2,
         typename ARG3,
         typename ARG4>
class CtorFunctor4
{
    
    ARG1   arg1_;
    ARG2   arg2_;
    ARG3   arg3_;
    ARG4   arg4_;
public:
    typedef typename WrapperTraits<WRP>::NativeClass NativeClass;
    typedef WRP WrpClass;

    CtorFunctor4(ARG1        arg1,
                 ARG2        arg2,
                 ARG3        arg3,
                 ARG4        arg4)
        : arg1_(arg1),
          arg2_(arg2),
          arg3_(arg3),
		  arg4_(arg4)
    {
        ;
    }


    NativeClass * operator()()
    {
        return new NativeClass(arg1_,
                               arg2_,
                               arg3_,
                               arg4_);
    }
};


template<typename WRP>
CtorFunctor<WRP> CallCtor(WRP * /*dummy*/)
{
    return CtorFunctor<WRP>();
}


template<typename WRP,
         typename ARG1>
CtorFunctor1<WRP, ARG1> CallCtor(WRP * /*dummy*/,
                                       ARG1        arg1)
{
    return CtorFunctor1<WRP, ARG1>(arg1);
}


template<typename WRP,
         typename ARG1,
         typename ARG2>
CtorFunctor2<WRP, ARG1, ARG2> CallCtor(WRP * /*dummy*/,
                                             ARG1        arg1,
                                             ARG2        arg2)
{
    return CtorFunctor2<WRP, ARG1, ARG2>(arg1, arg2);
}


template<typename WRP,
         typename ARG1,
         typename ARG2,
         typename ARG3>
CtorFunctor3<WRP, ARG1, ARG2, ARG3> CallCtor(WRP * /*dummy*/,
                                                   ARG1        arg1,
                                                   ARG2        arg2,
                                                   ARG3        arg3)
{
    return CtorFunctor3<WRP, ARG1, ARG2, ARG3>(arg1, arg2, arg3);
}

template<typename WRP,
         typename ARG1,
         typename ARG2,
         typename ARG3,
         typename ARG4>
CtorFunctor4<WRP, ARG1, ARG2, ARG3, ARG4> CallCtor(WRP * /*dummy*/,
                                                   ARG1        arg1,
                                                   ARG2        arg2,
                                                   ARG3        arg3,
                                                   ARG4        arg4)
{
    return CtorFunctor4<WRP, ARG1, ARG2, ARG3, ARG4>(arg1, arg2, arg3, arg4);
}



template<typename NAT,
         int      REFCOUNTED>
struct NativeReleaser
{
    static void release(NAT * p)
    {
        delete p;
    }
};


template<typename NAT>
struct NativeReleaser<NAT, LUCENE_REFCOUNTED>
{
    static void release(NAT * p)
    {
        _CLDECDELETE(p);
    }
};


template<typename NAT>
struct NativeReleaser<NAT, CPIX_REFCOUNTED>
{
    static void release(NAT * p)
    {
        if (p != NULL)
            {
                p->decRef();
            }
    }
};



/**
 * Releasing the native instance, clearing up the wrapper, but
 * not releasing the wrapper.
 *
 * @param p the pointer to the wrapper class
 *
 * @param releasePolicy whether to use _CLDECDELETE for releasing a
 * native instance (it is necessary in certain cases, according to
 * cLucene) or the standard way.
 */
template <typename WRP>
void ReleaseNative(WRP     * p)
{
    typedef typename WrapperTraits<WRP>::NativeClass NativeClass;
    enum { REFCOUNTED = RefCountTraits<WRP>::REFCOUNTED };

    Cpix::IIdxDb::rotateLog();

    try
        {
            NativeClass
                * ptr = Cast2Native<WRP>(p);

            NativeReleaser<NativeClass, REFCOUNTED>::release(ptr);

            p->ptr_ = NULL;
            p->err_ = NULL;
        }
    catch (...)
        {
            ;
        }
}

/**
 * Destruction of the wrapper along with the native instance.
 *
 * @param releasePolicy whether to use _CLDECDELETE for releasing a
 * native instance (it is necessary in certain cases, according to
 * cLucene) or the standard way.
 */
template <typename WRP>
void DestroyWrapper(WRP     * p)
{
    if (p != NULL)
        {
            ReleaseNative(p);
            
            free(p);
        }
}



template<typename NAT,
         int      REFCOUNTED>
struct SharedCopier
{
    // NOTHING HERE ON PURPOSE, as most types should not be
    // shallow-copied, only CPIX_REFCOUNTED ones.
    //
    // NAT * shareCopy(NAT * p) { return NULL; }
};


template<typename NAT>
struct SharedCopier<NAT, CPIX_REFCOUNTED>
{
    static NAT * shareCopy(NAT * p)
    {
        p->incRef();

        return p;
    }
};



/**
 * Creates a shallow copy of a cpix-refcounted wrapped instance. That
 * is:
 *
 *  (a) the wrapped native instance is incRef()-ed
 *
 *  (b) a new wrapper is created, and it will wrapped the very same
 *  native instance
 *
 * The effect of which is that owners of shared copies can simply use
 * their usual method of destroying objects, and the whole thing along
 * with the specific release logic for cpix-refcounted objects will
 * result in proper and independent behaviour.
 *
 * Failure to create a new wrapper instance is signalled through
 * result.
 */
template <typename WRP,
          typename RES_CLASS>
WRP * SharedCopy(WRP * p,
                       RES_CLASS * result)
{
    typedef typename WrapperTraits<WRP>::NativeClass NativeClass;
    enum { REFCOUNTED = RefCountTraits<WRP>::REFCOUNTED };

    Cpix::IIdxDb::rotateLog();

    WRP
        * newP = CreateWrapper_Pure<WRP, RES_CLASS>(result);
    
    if (newP != NULL)
        {
            NativeClass
                * ptr = Cast2Native<WRP>(p);
            
            ptr = SharedCopier<NativeClass, REFCOUNTED>::shareCopy(ptr);
            
            newP->ptr_ = ptr;
            newP->err_ = NULL;
        }

    return newP;
}


#endif /* CPIX_CPIXUTIL_H */
