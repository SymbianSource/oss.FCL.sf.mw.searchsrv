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

#include <wchar.h>
#include <stddef.h>

#include <iostream>
#include <set>
#include <string>

#include "cpixtools.h"

#include "itk.h"

#include "cpixidxdb.h"
#include "cpixdoc.h"
#include "cpixsearch.h"
#include "cpixidxdb.h"

#include "config.h"
#include "testutils.h"
#include "setupsentry.h"

#include "std_log_result.h"
#include "..\..\..\cpix\src\qrytypes\prefixqrytype.cpp"
#include "..\..\..\cpix\src\qrytypes\termsqrytype.cpp"

#define TEST_DOCUMENT_QBASEAPPCLASS "@0:root test document"
#define TEST_DOCUMENT_INDEXDB_PATH "c:\\Data\\indexing\\indexdb\\root\\test\\document"

#define DOCUID1 "document1" // without boosting this document will never be first result
#define DOCUID2 "document2"
#define LDOCUID1 L"document1"
#define LDOCUID2 L"document2"

// Each document will have 2 fields, alpha
#define FIELD_ALPHA L"Alpha"


#define DOC1CONTENT L"mary had a little lamb its fleece was black as coal"
#define DOC2CONTENT L"mary had a little little little lamb its fleece was as white as snow"


// The term that will be present in multiple documents.
#define SEARCH_TERM L"little"

// black is present in doc1, white in doc2. 
#define BOOST_SEARCH_TERMS L"black^20 white"

#define DEF_BOOST 1.0


class DocumentContext : public Itk::ITestContext
{
private:
    //
    // private members
    //
    cpix_Analyzer * analyzer_;
    cpix_QueryParser * queryParser_;
    cpix_Query * query_;
    cpix_IdxDb * idxDb_;
    cpix_Hits * hits_;
    
private:
  //  Search for the term SEARCH_TERM in field Alpha
    void executeSearch(Itk::TestMgr * testMgr)
    {
        hits_ = cpix_IdxDb_search(idxDb_, query_);
        
        int32_t 
        hits_len = cpix_Hits_length(hits_);
        
        ITK_ASSERT(testMgr,
            hits_len == 2,
            "wrong amount of documents returned in hits");
    }

    void addDocument(Itk::TestMgr  * testMgr,
                     const wchar_t * docUid,
                     const wchar_t * fieldValue)
    {
        addDocument(testMgr,
                    docUid,
                    fieldValue,
                    DEF_BOOST,
                    DEF_BOOST);
    }
                     
    
    
    void addDocument(Itk::TestMgr  * testMgr,
                     const wchar_t * docUid,
                     const wchar_t * fieldValue,
                     float_t         docBoost,
                     float_t         fieldBoost) // TODO IMPLEMENT
    {
        cpix_Result
            result;
        cpix_Document
            * doc = cpix_Document_create(&result,
                                         docUid,
                                         NULL,      // app class
                                         NULL,      // excerpt
                                         NULL);     // mime type
        ITK_ASSERT(testMgr,
                   cpix_Succeeded(&result),
                   "Creating document %S failed",
                   docUid);

        if (docBoost != DEF_BOOST)
            {
                cpix_Document_setBoost(doc,
                                       docBoost);
                ITK_EXPECT(testMgr,
                           cpix_Succeeded(doc),
                           "Could not set doc boost");

                float_t
                    boost = cpix_Document_boost(doc);

                ITK_EXPECT(testMgr,
                           cpix_Succeeded(doc),
                           "Could not get doc boost");
                ITK_EXPECT(testMgr,
                           boost == docBoost,
                           "Incorrect doc boost value");
            }

        cpix_Field
            field;
        cpix_Field_initialize(&field,
                              FIELD_ALPHA,
                              fieldValue, 
                              cpix_STORE_YES |cpix_INDEX_TOKENIZED);
        if (cpix_Failed(&field))
            {
                cpix_Document_destroy(doc);
            }

        ITK_ASSERT(testMgr,
                   cpix_Succeeded(&field),
                   "Could not create field");

        if (fieldBoost != DEF_BOOST)
            {
                cpix_Field_setBoost(&field,
                                    fieldBoost);
                ITK_EXPECT(testMgr,
                           cpix_Succeeded(&field),
                           "Could not set field boost");

                float_t
                    boost = cpix_Field_boost(&field);

                ITK_EXPECT(testMgr,
                           cpix_Succeeded(&field),
                           "Could not get field boost");
                ITK_EXPECT(testMgr,
                           boost == fieldBoost,
                           "Incorrect field boost value");
            }
        
        cpix_Document_add(doc,
                          &field);
        bool
            succeeded = true;
        if (cpix_Failed(doc))
            {
                cpix_Document_destroy(doc);
                cpix_Field_release(&field);
                succeeded = false;
            }

        ITK_ASSERT(testMgr,
                   succeeded,
                   "Could not add field to document");

        cpix_IdxDb_add(idxDb_,
                       doc,
                       analyzer_);

        cpix_Document_destroy(doc);

        ITK_ASSERT(testMgr,
                   cpix_Succeeded(idxDb_),
                   "Could not add document to index");
    }

public:
    
    //
    // from ITestContext
    //
    virtual void setup() throw (Itk::PanicExc)
    {
        cpix_Result
            result;

        SetupSentry
            ss(*this);

        cpix_IdxDb_dbgScrapAll(&result);

        if (cpix_Failed(&result))
        {
            ITK_PANIC("cpix_IdxDb_dbgScrapAll problem");
        }
        
        analyzer_ = cpix_CreateSimpleAnalyzer(&result);
        if ( !analyzer_ )
        {
            ITK_PANIC("Analyzer could not be created");
        }

        queryParser_ = cpix_QueryParser_create(&result,
                                                FIELD_ALPHA,
                                               analyzer_);
        if (queryParser_ == NULL)
            {
                ITK_PANIC("Could not create query parser");
            }

        query_ = cpix_QueryParser_parse(queryParser_,
                                        SEARCH_TERM);
        if (cpix_Failed(queryParser_))
            {
                ITK_PANIC("Could not create query parser");
            }

        cpix_IdxDb_defineVolume(&result, 
                                TEST_DOCUMENT_QBASEAPPCLASS, 
                                TEST_DOCUMENT_INDEXDB_PATH);
        if (cpix_Failed(&result))
            {
            ITK_PANIC("Failed to define test index db");
            }

        idxDb_ = cpix_IdxDb_openDb(&result,
                                   TEST_DOCUMENT_QBASEAPPCLASS,
                                   cpix_IDX_CREATE);
        if (cpix_Failed(&result))
            {
            ITK_PANIC("Failed to open indexDb");
            }

        ss.setupComplete();
    }

    
    virtual void tearDown() throw()
    {
        cpix_Analyzer_destroy(analyzer_);
        analyzer_ = NULL;
        
        cpix_QueryParser_destroy(queryParser_);
        queryParser_ = NULL;
        
        cpix_Query_destroy(query_);
        query_ = NULL;

        cpix_IdxDb_releaseDb(idxDb_);
        idxDb_ = NULL;

        cpix_Hits_destroy(hits_);
        hits_ = NULL;
    }

    
    DocumentContext()
      : analyzer_(NULL),
        queryParser_(NULL),
        query_(NULL),
        idxDb_(NULL),
        hits_(NULL)
    {
        ;
    }

    ~DocumentContext()
    {
    }

    

    void TestPrefixQryType(Itk::TestMgr *testMgr )
        {
        char *xml_file = (char *)__FUNCTION__;
        assert_failed = 0;
        tearDown();
        setup();
        cpix_Result  result;
        addDocument(testMgr,
                    LDOCUID1,
                    DOC1CONTENT);
        addDocument(testMgr,
                    LDOCUID2,
                    DOC2CONTENT);
        
        cpix_IdxDb_flush(idxDb_);
        ITK_EXPECT(testMgr,
                   cpix_Succeeded(idxDb_),
                   "Flushing index has failed");
        if(!cpix_Succeeded(idxDb_))
            {
            assert_failed = 1;
            }
        Cpix::PrefixQryType *qryType = new Cpix::PrefixQryType;
        std::list<std::wstring> list(3,L"");
        std::list<std::wstring> list1;
        qryType->setUp(queryParser_, list, SEARCH_TERM);
        qryType->setUp(queryParser_, list1, SEARCH_TERM);
        cpix_IdxSearcher *
        searcher = cpix_IdxSearcher_openDb(&result,
                TEST_DOCUMENT_QBASEAPPCLASS);
        if (searcher == NULL)
            {
                ITK_PANIC("Could not create searcher");
            }
        cpix_Hits *Hits1 = qryType->search(searcher);
        cpix_Hits *Hits2 = qryType->search(idxDb_);
        testResultXml(xml_file);
        }
    
    void TestTermsQryType(Itk::TestMgr * )
        {
        char *xml_file = (char *)__FUNCTION__;
        assert_failed = 0;
        Cpix::TermsQryType qrytype;
        tearDown();
        setup();
        std::list<std::wstring> list(3, L"term");
        std::list<std::wstring> list1;
        qrytype.setUp(queryParser_, list, SEARCH_TERM);
        qrytype.setUp(queryParser_, list1, SEARCH_TERM);
        testResultXml(xml_file);
        }
    
    void testNoBoostingFields(Itk::TestMgr * testMgr)
    {
        // Don't boost Field Alpha in doc1
        char *xml_file = (char *)__FUNCTION__;
        assert_failed = 0;
        addDocument(testMgr,
                    LDOCUID1,
                    DOC1CONTENT);
        addDocument(testMgr,
                    LDOCUID2,
                    DOC2CONTENT);
        
        cpix_IdxDb_flush(idxDb_);
        ITK_EXPECT(testMgr,
                   cpix_Succeeded(idxDb_),
                   "Flushing index has failed");
        if(!cpix_Succeeded(idxDb_))
            {
            assert_failed = 1;
            }

        executeSearch(testMgr);
        //  EXPECTED result is that doc2 first, doc1 second.

        cpix_Document
            returnedDoc1;

        cpix_Hits_doc(hits_, 0, &returnedDoc1);

        const wchar_t* id = cpix_Document_getFieldValue(&returnedDoc1, 
                                                        LCPIX_DOCUID_FIELD);
        if (id)
        {
            std::wstring str(id);
            ITK_ASSERT(testMgr,
                str.compare(LDOCUID2) == 0,
                "wrong document");
            if(str.compare(LDOCUID2) != 0)
                {
                assert_failed = 1;
                }
        }
        else
        {
            assert_failed = 1;
            ITK_PANIC("failed to get _docuid");
        }

        cpix_Document
            returnedDoc2;

        cpix_Hits_doc(hits_, 1, &returnedDoc2);
        id = NULL;

        id = cpix_Document_getFieldValue(&returnedDoc2, LCPIX_DOCUID_FIELD);
        if (id)
        {
            std::wstring str(id);
            ITK_ASSERT(testMgr,
                str.compare(LDOCUID1) == 0,
                "wrong document");
            if(str.compare(LDOCUID1) != 0)
                {
                assert_failed = 1;
                }
        }
        else
        {
        assert_failed = 1;
            ITK_PANIC("failed to get _docuid");
        }
        testResultXml(xml_file);
    }



    void testBoostField(Itk::TestMgr * testMgr)
    {
        char *xml_file = (char *)__FUNCTION__;
        assert_failed = 0;
        tearDown();
        setup();

        addDocument(testMgr,
                    LDOCUID1,
                    DOC1CONTENT,
                    DEF_BOOST,
                    4.0);
        addDocument(testMgr,
                    LDOCUID2,
                    DOC2CONTENT);

        cpix_IdxDb_flush(idxDb_);
        ITK_EXPECT(testMgr,
                   cpix_Succeeded(idxDb_),
                   "Flushing index has failed");
        if(!cpix_Succeeded(idxDb_))
            {
            assert_failed = 1;
            }

        executeSearch(testMgr);
        //  EXPECTED result is that doc1 first, doc2 second.

        cpix_Document
            returnedDoc1;
        
        cpix_Hits_doc(hits_, 0, &returnedDoc1);

        const wchar_t* id = cpix_Document_getFieldValue(&returnedDoc1, 
                                                        LCPIX_DOCUID_FIELD);
        if ( id )
        {
            std::wstring str( id );
            ITK_ASSERT(testMgr,
                str.compare(LDOCUID1) == 0,
                "wrong document");
            if(str.compare(LDOCUID1) != 0)
                {
                assert_failed = 1;
                }
        }
        else
        {
            ITK_PANIC("failed to get _docuid");
            assert_failed = 1;
        }

        cpix_Document
            returnedDoc2;

        cpix_Hits_doc(hits_, 1, &returnedDoc2);
        id = NULL;

        id = cpix_Document_getFieldValue(&returnedDoc2, LCPIX_DOCUID_FIELD );
        if ( id )
        {
            std::wstring str( id );
            ITK_ASSERT(testMgr,
                str.compare(LDOCUID2) == 0,
                "wrong document");
            if(str.compare(LDOCUID2) != 0)
                {
                assert_failed = 1;
                }
        }
        else
        {
            ITK_PANIC("failed to get _docuid");
            assert_failed = 1;
        }
        testResultXml(xml_file);
    }

    void testBoostDocument(Itk::TestMgr * testMgr)
    {
        char *xml_file = (char *)__FUNCTION__;
        assert_failed = 0;
        tearDown();
        setup();
    
        addDocument(testMgr,
                    LDOCUID1,
                    DOC1CONTENT,
                    2.0,
                    DEF_BOOST);
        addDocument(testMgr,
                    LDOCUID2,
                    DOC2CONTENT);

        cpix_IdxDb_flush(idxDb_);
        ITK_EXPECT(testMgr,
                   cpix_Succeeded(idxDb_),
                   "Flushing index has failed");
        if(!cpix_Succeeded(idxDb_))
            {
            assert_failed = 1;
            }
        executeSearch(testMgr);
        //  EXPECTED result is that doc1 first, doc2 second.

        cpix_Document
            returnedDoc1;
        
        cpix_Hits_doc(hits_, 0, &returnedDoc1);

        const wchar_t* id = cpix_Document_getFieldValue(&returnedDoc1, 
                                                        LCPIX_DOCUID_FIELD);
        if (id)
        {
            std::wstring str(id);
            ITK_ASSERT(testMgr,
                str.compare(LDOCUID1) == 0,
                "wrong document");
            if(str.compare(LDOCUID1) != 0)
                {
                assert_failed = 1;
                }
        }
        else
        {
            ITK_PANIC("failed to get _docuid");
            assert_failed = 1;
        }

        cpix_Document
            returnedDoc2;

        cpix_Hits_doc(hits_, 1, &returnedDoc2);
        id = NULL;

        id = cpix_Document_getFieldValue(&returnedDoc2, LCPIX_DOCUID_FIELD);
        if (id)
        {
            std::wstring str(id);
            ITK_ASSERT(testMgr,
                str.compare(LDOCUID2) == 0,
                "wrong document");
            if(str.compare(LDOCUID2) != 0)
                {
                assert_failed = 1;
                }
        }
        else
        {
            ITK_PANIC("failed to get _docuid");
            assert_failed = 1;
        }
        testResultXml(xml_file);
    }


    void testBoostQuery(Itk::TestMgr * testMgr)
    {
        char *xml_file = (char *)__FUNCTION__;
        assert_failed = 0;
        tearDown();
        setup();

        addDocument(testMgr,
                    LDOCUID1,
                    DOC1CONTENT);
        addDocument(testMgr,
                    LDOCUID2,
                    DOC2CONTENT);

        cpix_IdxDb_flush(idxDb_);
        ITK_EXPECT(testMgr,
                   cpix_Succeeded(idxDb_),
                   "Flushing index has failed");
        if(!cpix_Succeeded(idxDb_))
            {
            assert_failed = 1;
            }
        // doc1_ should be the first result given the following query boost.
        cpix_Query_destroy(query_);

        query_ = cpix_QueryParser_parse(queryParser_,
                                        BOOST_SEARCH_TERMS);

        hits_ = cpix_IdxDb_search(idxDb_, query_);
      
        int32_t
            hits_len = cpix_Hits_length(hits_);

        ITK_ASSERT(testMgr,
                  hits_len == 2,
                  "wrong amount of documents returned in hits");
        if(hits_len != 2)
            {
            assert_failed = 1;
            }

        //  EXPECTED result is that doc2 first.
        cpix_Document
            returnedDoc1;
        
        cpix_Hits_doc(hits_, 0, &returnedDoc1);

        const wchar_t* id = cpix_Document_getFieldValue(&returnedDoc1, 
                                                        LCPIX_DOCUID_FIELD);
        if (id)
        {
            std::wstring str(id);
            ITK_ASSERT(testMgr,
                str.compare(LDOCUID1) == 0,
                "wrong document");
            if(str.compare(LDOCUID2) != 0)
                {
                assert_failed = 1;
                }
        }
        else
        {
            ITK_PANIC("failed to get _docuid");
            assert_failed = 1;
        }

        cpix_Document
            returnedDoc2;

        cpix_Hits_doc(hits_, 1, &returnedDoc2);
        id = NULL;

        id = cpix_Document_getFieldValue(&returnedDoc2, LCPIX_DOCUID_FIELD);
        if (id)
        {
            std::wstring str(id);
            ITK_ASSERT(testMgr,
                str.compare(LDOCUID2) == 0,
                "wrong document");
            if(str.compare(LDOCUID2) != 0)
                {
                assert_failed = 1;
                }
        }
        else
        {
            ITK_PANIC("failed to get _docuid");
            assert_failed = 1;
        }
        testResultXml(xml_file);
    }
};


Itk::TesterBase * CreateDocumentTests()
{
    using namespace Itk;

    DocumentContext
        * documentContext = new DocumentContext;
    ContextTester
        * contextTester = new ContextTester("document",
                                            documentContext);

#define TEST "sanity check no boosts"
    contextTester->add(TEST,
            documentContext,
                       &DocumentContext::testNoBoostingFields);
#undef TEST

#define TEST "boost field"
    contextTester->add(TEST,
            documentContext,
                       &DocumentContext::testBoostField);
#undef TEST

#define TEST "boost document"
    contextTester->add(TEST,
            documentContext,
                       &DocumentContext::testBoostDocument);
#undef TEST

#define TEST "boost query"
    contextTester->add(TEST,
            documentContext,
                       &DocumentContext::testBoostQuery);
#undef TEST
    // Both test throws the exception so need not to cover.
//#define TEST "perfixqrytype"
//    contextTester->add(TEST,
//            documentContext,
//                &DocumentContext::TestPrefixQryType);
//#undef TEST    
//        
//#define TEST "termqrytype"
//    contextTester->add(TEST,
//            documentContext,
//                &DocumentContext::TestTermsQryType);
//#undef TEST 
    
    return contextTester;
}
