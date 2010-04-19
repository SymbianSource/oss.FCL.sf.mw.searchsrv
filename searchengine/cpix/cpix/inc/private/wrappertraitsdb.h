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

#ifndef CPIX_WRAPPERTRAITSDB_H
#define CPIX_WRAPPERTRAITSDB_H



/**
 * Compile time type association between wrapper class and
 * wrapped clucene class.
 *
 * This struct is to be specialized for specific wrapper class -
 * native clucene class pairs. These will be found, in general, in the
 * implementation source files.
 */
template<typename WRP_CLASS>
struct WrapperTraits
{
    // All this struct ("CONCEPT") is to provide a typedef with name
    // NativeClass, telling what the native class associated to the
    // wrapper class is.
    //
    // typedef ??? NativeClass;

};


enum
    {
        NOT_REFCOUNTED    = 0,
        LUCENE_REFCOUNTED = 1,
        CPIX_REFCOUNTED   = 2
    };


/**
 * To enable correct release of native lucene types based on whether
 * they are refcounted or not.
 */
template<typename WRP_CLASS>
struct RefCountTraits
{
    enum 
        { 
            /**
             * This value here tells if a native is refcounted
             * *always* (derived with LUCENE_REFBASE macro). If it is
             * not, the value is NOT_REFCOUNTED (0), as in the case of
             * most lucene classes. For the reference counted clucene
             * classes use value LUCENE_REFCOUNTED (1). For Cpix::
             * reference counted classes (deriving from
             * RefCountedBase) use value CPIX_REFCOUNTED (2).
             *
             * (it seems like that: index/Term, store/Directory and
             * storey/FSDirectory are such classes, so far).
             */
            REFCOUNTED = NOT_REFCOUNTED
        };
};


/**
 * Yields the native pointer based on the wrapper instance (does the
 * reinterpret casting itself). For a NULL pointer wrapper instance it
 * yields a NULL pointer native instance.
 */
template<typename WRP_CLASS>
typename WrapperTraits<WRP_CLASS>::NativeClass * Cast2Native(WRP_CLASS * p)
{
    typedef typename WrapperTraits<WRP_CLASS>::NativeClass NativeClass;

    NativeClass
        * rv = NULL;

    if (p != NULL)
        {
            rv = reinterpret_cast<NativeClass*>(p->ptr_);
        }

    return rv;
}


/*******************************************************
 * Forward declarations
 */
namespace lucene { namespace analysis {
        class SnowballAnalyzer;
    }
}
namespace Cpix {
	class CustomAnalyzer; 
    class SystemAnalyzer;
}



/*******************************************************
 * Wrapper Traits "Database"
 */



template<>
struct WrapperTraits<cpix_InitParams>
{
    typedef Cpix::InitParams NativeClass;
};


template<>
struct WrapperTraits<cpix_IdxDb>
{
    typedef Cpix::IIdxDb NativeClass;
};


template<>
struct WrapperTraits<cpix_IdxSearcher>
{
    typedef Cpix::IIdxDb NativeClass;
};



// Conceptually, simple analyzer and snowball analyzers are both
// analyzers, and hence they are represented by cpix_Analyzer to the
// clients. The only thing we do here is to make sure that Create()
// template function (cpixutils.h) works properly.
struct cpix_SimpleAnalyzer : public cpix_Analyzer { };

struct cpix_SnowballAnalyzer : public cpix_Analyzer { };

struct cpix_CustomAnalyzer : public cpix_Analyzer { };

struct cpix_SystemAnalyzer : public cpix_Analyzer { };


template<>
struct WrapperTraits<cpix_Analyzer>
{
    // OBS typedef lucene::analysis::Analyzer NativeClass;

    // all native instances wrapped by cpix_Analyzer are actually
    // Cpix::SystemAnalyzer instances
    typedef Cpix::SystemAnalyzer NativeClass;
};


template<>
struct RefCountTraits<cpix_Analyzer>
{
    // Cpix::SystemAnalyzer is ref-counted (deriving from RefCountedBase)
    enum { REFCOUNTED = CPIX_REFCOUNTED };
};


template<>
struct WrapperTraits<cpix_SimpleAnalyzer>
{
    typedef lucene::analysis::SimpleAnalyzer NativeClass;
};


template<>
struct WrapperTraits<cpix_SnowballAnalyzer>
{
    typedef lucene::analysis::SnowballAnalyzer NativeClass;
};

template<>
struct WrapperTraits<cpix_CustomAnalyzer>
{
    typedef Cpix::CustomAnalyzer NativeClass;
};

template<>
struct WrapperTraits<cpix_SystemAnalyzer>
{
    typedef Cpix::SystemAnalyzer NativeClass;
};


struct cpix_MultiFieldQueryParser : public cpix_QueryParser { };


template<>
struct WrapperTraits<cpix_QueryParser>
{
    typedef lucene::queryParser::QueryParser NativeClass;
};


template<>
struct WrapperTraits<cpix_MultiFieldQueryParser>
{
    typedef lucene::queryParser::MultiFieldQueryParser NativeClass;
};


template<>
struct WrapperTraits<cpix_BoostMap>
{
    typedef lucene::queryParser::BoostMap  NativeClass;
};


// forward declaration
namespace Cpix
{
    class IQryType;
    class Field; 
    class DocumentFieldIterator;
    class Document;
}


template<>
struct WrapperTraits<cpix_Query>
{
    typedef Cpix::IQryType NativeClass;
};


template<>
struct RefCountTraits<cpix_Query>
{
    // Cpix::IQryType is ref-counted (derives from RefCountedBase)
    enum { REFCOUNTED = CPIX_REFCOUNTED };
};


template<>
struct WrapperTraits<cpix_Hits>
{
    typedef Cpix::IHits NativeClass;
};


template<>
struct RefCountTraits<cpix_Hits>
{
    // Cpix::IHits is ref-counted (derives from RefCountedBase)
    enum { REFCOUNTED = CPIX_REFCOUNTED };
};


template<>
struct WrapperTraits<cpix_Field>
{
    typedef Cpix::Field NativeClass;
};


template<>
struct WrapperTraits<cpix_DocFieldEnum>
{
    typedef Cpix::DocumentFieldIterator NativeClass;
};

template<>
struct WrapperTraits<cpix_Document>
{
    typedef Cpix::Document NativeClass;
};


template<>
struct RefCountTraits<cpix_Document>
{
    // Cpix::Document is ref-counted (derives from RefCountedBase)
    enum { REFCOUNTED = CPIX_REFCOUNTED };
};


template<>
struct WrapperTraits<cpix_Term>
{
    typedef lucene::index::Term NativeClass;

};

template<>
struct RefCountTraits<cpix_Term>
{
    // Term is ref-counted
    enum { REFCOUNTED = LUCENE_REFCOUNTED };
};







#endif // CPIX_WRAPPERTRAITSDB_H
