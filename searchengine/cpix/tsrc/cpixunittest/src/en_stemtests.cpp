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

#include "cpixfstools.h"

#include "itk.h"

#include "cpixidxdb.h"

#include "config.h"
#include "testutils.h"

#include "testcorpus.h"
#include "setupsentry.h"

#include "std_log_result.h"

class StemContext : public Itk::ITestContext, public Cpt::IFileVisitor
{


protected:
    FileIdxUtil      * util_;
    cpix_Analyzer    * analyzer_;
    cpix_Query       * query_;

    cpix_QueryParser * queryParser_;

    Itk::TestMgr     * testMgr_;

    virtual cpix_LangCode langCode() const = 0;
    virtual wchar_t * queryTerm() const = 0;

public:


    //
    // From ITestContext
    //
    virtual void setup() throw (Itk::PanicExc)
    {
        SetupSentry
            ss(*this);

        cpix_Result
            result;

        cpix_IdxDb_dbgScrapAll(&result);

        using namespace std;

        util_ = new FileIdxUtil;
        util_->init();
        
        analyzer_ = cpix_CreateSnowballAnalyzer(&result,
                                                langCode());
        if (analyzer_ == NULL)
            {
                ITK_PANIC("Could not create analyzer");
            }

        queryParser_ = cpix_QueryParser_create(&result,
                                              CONTENTS_FIELD,
                                              analyzer_);
        if (queryParser_ == NULL)
            {
                ITK_PANIC("Could not create query parser");
            }

        query_ = cpix_QueryParser_parse(queryParser_,
                                        queryTerm());
        if (cpix_Failed(queryParser_)
            || query_ == NULL)
            {
                ITK_PANIC("Could not parse query string");
            }

        ss.setupComplete();
    }



    virtual void tearDown() throw ()
    {
        cleanup();
    }


    StemContext()
        : util_(NULL),
          analyzer_(NULL),
          query_(NULL),
          queryParser_(NULL),
          testMgr_(NULL)
    {
        ;
    }


    virtual ~StemContext()
    {
        cleanup();
    }


    
    //
    // from Cpt::IFileVisitor
    //
    virtual bool visitFile(const char * path)
    {
        bool
            goOn = true;

        util_->indexFile(path,
                         analyzer_,
                         testMgr_);

        return goOn;
    }
    
    
    virtual DirVisitResult visitDirPre(const char * /*path*/)
    {
        return IFV_CONTINUE;
    }


    virtual bool visitDirPost(const char * /*path*/)
    {
        return true;
    }



    //
    // public operations
    //
    void deleteFiles(Itk::TestMgr * testMgr,
                     const char  ** docUids,
                     size_t         count,
                     size_t         expectedDelCount)
    {
        using namespace Itk;

        static const int FIELD_SIZE = 128;

        wchar_t
            ** fieldValues = new wchar_t* [count];
        
        for (size_t i = 0; i < count; ++i)
            {
                fieldValues[i] = NULL;
            }
        
        for (size_t i = 0; i < count; ++i)
            {
                fieldValues[i] = new wchar_t[FIELD_SIZE];
                mbstowcs(fieldValues[i],
                         docUids[i],
                         FIELD_SIZE-1);
                fieldValues[i][FIELD_SIZE-1] = wchar_t(0);
            }
        
        int32_t
            result = 0;

        for (size_t i = 0; i < count; ++i)
            {
                result += cpix_IdxDb_deleteDocuments(util_->idxDb(),
                                                     fieldValues[i]);
                if (cpix_Failed(util_->idxDb()))
                    {
                        printf("Failed to delete\n");
                        cpix_ClearError(util_->idxDb());
                        assert_failed = 1;
                        break;
                    }
                else
                    {
                        std::string
                            docUid(docUids[i]);
                        docUid[0] = '!';
                        ITK_MSG(testMgr,
                                "Deleted document %s",
                                docUid.c_str());
                    }
            }
        
        ITK_EXPECT(testMgr,
                   result == expectedDelCount,
                   "Only %d docs instead of %d have been deleted",
                   result,
                   expectedDelCount);
        if(result != expectedDelCount)
            {
                assert_failed = 1;
            }

        for (size_t i = 0; i < count; ++i)
            {
                delete[] fieldValues[i];
            }
        
        delete[] fieldValues;
    }


private:
    //
    // private methods
    //

    void cleanup()
    {
        delete util_;
        util_ = NULL;

        cpix_Analyzer_destroy(analyzer_);
        analyzer_ = NULL;

        cpix_Query_destroy(query_);
        query_ = NULL;

        cpix_QueryParser_destroy(queryParser_);
        queryParser_ = NULL;
    }


};


const char * EnglishDocsToDelete[2] = {
    FILE_TEST_CORPUS_PATH "\\en\\1.txt",
    FILE_TEST_CORPUS_PATH "\\en\\4.txt",
};


class EnglishStemContext : public StemContext
{
protected:
    virtual cpix_LangCode langCode() const
    {
        return cpix_LANG_EN;
    }


    virtual wchar_t * queryTerm() const
    {
        return L"happy";
    }


public:
    //
    // public operations
    //

    void testAddFiles(Itk::TestMgr * testMgr)
    {
        char *xml_file = (char *)__FUNCTION__;
        assert_failed = 0;
        testMgr_ = testMgr;
        Cpt::traverse(FILE_TEST_CORPUS_PATH "\\en",
                      this);
        util_->flush();
        testResultXml(xml_file);
    }


    
    void testSearchFiles(Itk::TestMgr * testMgr)
    {
        using namespace Itk;
        char *xml_file = (char *)__FUNCTION__;
        assert_failed = 0;
        cpix_Hits
            * hits = cpix_IdxDb_search(util_->idxDb(),
                                       query_);
        
        if (cpix_Failed(util_->idxDb()))
            {
                ITK_EXPECT(testMgr,
                           false,
                           "Failed to search index");
                cpix_ClearError(util_->idxDb());
                assert_failed = 1;
            }
        else
            {
                util_->printHits(hits,
                                 testMgr);

                cpix_Hits_destroy(hits);
            }
        testResultXml(xml_file);

    }


    void testDeleteFiles(Itk::TestMgr * testMgr)
    {
        char *xml_file = (char *)__FUNCTION__;
        assert_failed = 0;
        deleteFiles(testMgr,
                    EnglishDocsToDelete,
                    sizeof(EnglishDocsToDelete)/sizeof(wchar_t*),
                    2);
        util_->flush();
        testResultXml(xml_file);
    }


};


Itk::TesterBase * CreateEnglishStemTests()
{
    using namespace Itk;

    EnglishStemContext
        * context = new EnglishStemContext;

    ContextTester
        * contextTester = new ContextTester("en",
                                            context);


#define TEST "adding"
    contextTester->add(TEST,
                       context,
                       &EnglishStemContext::testAddFiles,
                       TEST);
#undef TEST

#define TEST "searching"
    contextTester->add(TEST,
                       context,
                       &EnglishStemContext::testSearchFiles,
                       TEST);
#undef TEST


#define TEST "deleting"
    contextTester->add(TEST,
                       context,
                       &EnglishStemContext::testDeleteFiles,
                       TEST);
#undef TEST

#define TEST "searching2"
    contextTester->add(TEST,
                       context,
                       &EnglishStemContext::testSearchFiles,
                       TEST);
#undef TEST


    return contextTester;
}


Itk::TesterBase * CreateStemTests()
{
    using namespace Itk;

    SuiteTester
        * stemTests = new SuiteTester("stem");

    stemTests->add(CreateEnglishStemTests());

    // TODO add more tests to suite

    return stemTests;
}

