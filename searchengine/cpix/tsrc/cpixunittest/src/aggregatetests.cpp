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
#include "setupsentry.h"

#include "testcorpus.h"

#include "std_log_result.h"

cpix_FieldDesc MultiFieldSchema[] = {
    
    // filter field
    {
        LCPIX_FILTERID_FIELD,                  // name_
        cpix_STORE_YES | cpix_INDEX_NO,       // cfg_
    },

    // dummy field 
    {
        L"dummy",                             // name_
        cpix_STORE_YES | cpix_INDEX_TOKENIZED // cfg_
    },
    {
        L"dummy2",                            // name_
        cpix_STORE_YES | cpix_INDEX_NO | cpix_AGGREGATE_YES// cfg_
    },
    {
        L"dummy3",                            // name_
        cpix_STORE_YES | cpix_INDEX_TOKENIZED | cpix_AGGREGATE_NO // cfg_
    }
};


const wchar_t * DummyWords[] = {
    L"happy words",
    L"look at you",
    L"happening there",
    L"important happiness"
};

// NOTE must be of the same size (or bigger) as DummyWords
const wchar_t * Dummy2Words[] = {
    L"christmas easter",
    L"summer's midnight eve",
    L"thanksgiving",
    L"vappu"
};

// NOTE must be of the same size (or bigger) as DummyWords
const wchar_t * Dummy3Words[] = {
    L"rabid happening",
    L"look, important and happy robot",
    L"christmas for ever",
    L"christmas is an important and happy happening"
};



class MultiFieldIdxUtil : public FileIdxUtil
{
public:
    virtual ~MultiFieldIdxUtil() throw ()
    {
        ;
    }


    void indexMultiField(const char     * path,
                         cpix_Analyzer  * analyzer,
                         Itk::TestMgr   * testMgr)
    {
        wchar_t
            wpath[256];

        size_t
            res = mbstowcs(wpath,
                           path,
                           sizeof(wpath) / sizeof(wchar_t) - 1);
        ITK_ASSERT(testMgr,
                   //res >= 0,
				   1,
                   "mbstowcs failed, errno: %d",
                   errno);
        wpath[sizeof(wpath) / sizeof(wchar_t) - 1] = wchar_t(0);


        static int
            dummyWordIdx = 0;

        const wchar_t *fields[4] = {
            LCPIX_FILEPARSER_FID,      // filter id field
            DummyWords[dummyWordIdx],  // dummy field
            Dummy2Words[dummyWordIdx], // dummy2 field
            Dummy3Words[dummyWordIdx] // dummy3 field
        };

        ++dummyWordIdx;
        if (dummyWordIdx == sizeof(DummyWords) / sizeof(wchar_t*))
            {
                dummyWordIdx = 0;
            }

        // NOTE: app class, excerpt and mime type have to be defined
        // here, but this is using the file parser functionality
        // inside Cpix, which re-defines these anyway
        cpix_IdxDb_add2(idxDb(),
                        schemaId(),
                        wpath,                   // docUid,
                        NULL,                    // app class
                        NULL,                    // excerpt
                        NULL,                    // mime type
                        (const wchar_t**)fields, // fields
                        analyzer);

        if (cpix_Failed(idxDb()))
            {
                wchar_t
                    report[256];
                cpix_Error_report(idxDb()->err_,
                                  report,
                                  sizeof(report));
                ITK_ASSERT(testMgr,
                           false,
                           "Failed to add document %S: %S",
                           wpath,
                           report);
                cpix_ClearError(idxDb());
            }
        else
            {
                wpath[0] = '!';
                ITK_MSG(testMgr,
                        "Indexed file (multifield): %S",
                        wpath);
            }
    }


protected:
    virtual void printHit(cpix_Document  * doc,
                          Itk::TestMgr   * testMgr)
    {
        using namespace std;
        
        std::wstring
            idStr(getIdStr(doc,
                           testMgr));
        
        fprintf(stdout,
                "DOC (multifield) (%S):\n",
                idStr.c_str());
        
        cpix_DocFieldEnum
            * dfe = cpix_Document_fields(doc);
        if (cpix_Succeeded(doc))
            {
                cpix_Field
                    field;

                while (cpix_DocFieldEnum_hasMore(dfe))
                    {
                        cpix_DocFieldEnum_next(dfe,
                                               &field);

                        const wchar_t
                            * name = cpix_Field_name(&field);

                        if (wcscmp(name, LCPIX_DOCUID_FIELD) == 0)
                            {
                                continue;
                            }

                        fprintf(stdout,
                                " o %S: ",
                                name);

                        bool
                            binary = static_cast<bool>(cpix_Field_isBinary(&field));

                        fprintf(stdout,
                                "%S\n",
                                (binary ? 
                                 L"(binary)" 
                                 : cpix_Field_stringValue(&field)
                                 )
                                );
                    }

                cpix_DocFieldEnum_destroy(dfe);
            }
        else
            {
                wchar_t
                    report[128];
                cpix_Error_report(doc->err_,
                                  report,
                                  sizeof(report)/sizeof(wchar_t));
                ITK_EXPECT(testMgr,
                           false,
                           "Could not create doc field enum: %S",
                           report);
                cpix_ClearError(doc);
            }
    }


    virtual SchemaId addSchema() throw (Itk::PanicExc)
    {
        return cpix_IdxDb_addSchema(idxDb(),
                                    MultiFieldSchema,
                                    sizeof(MultiFieldSchema)/sizeof(cpix_FieldDesc));    
    }

};



class MultiFieldContext : public Itk::ITestContext, public Cpt::IFileVisitor
{
protected:
    MultiFieldIdxUtil     * util_;
    cpix_Analyzer         * analyzer_;
    cpix_QueryParser      * queryParser_;

    Itk::TestMgr          * testMgr_;


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

        util_ = new MultiFieldIdxUtil;
        util_->init();
        
        analyzer_ = cpix_CreateSimpleAnalyzer(&result);

        if (analyzer_ == NULL)
            {
                ITK_PANIC("Could not create analyzer");
            }

        queryParser_ = cpix_QueryParser_create(&result,
                                               LCPIX_DEFAULT_FIELD,
                                               analyzer_);

        if (queryParser_ == NULL)
            {
                ITK_PANIC("Could not create query parser");
            }

        ss.setupComplete();
    }


    virtual void tearDown() throw ()
    {
        cleanup();
    }


    virtual ~MultiFieldContext()
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

        util_->indexMultiField(path,
                               analyzer_,
                               testMgr_);
        
        return goOn;
    }


    virtual DirVisitResult visitDirPre(const char * /* path */)
    {
        return IFV_CONTINUE;
    }


    virtual bool visitDirPost(const char * /* path */)
    {
        return true;
    }


    MultiFieldContext()
        : util_(NULL),
          analyzer_(NULL),
          queryParser_(NULL),
          testMgr_(NULL)
    {
        ;
    }


    //
    // Test operations
    //
    void testAddFiles(Itk::TestMgr * testMgr)
    {
        testMgr_ = testMgr;
        Cpt::traverse(FILE_TEST_CORPUS_PATH "\\en",
                      this);

        util_->flush();
       
    }


    void testSearchForHappy(Itk::TestMgr * testMgr)
    {
        const wchar_t
            * word = L"happy";
        testSearchFor(testMgr,
                      word);
       
        
    }

    void testSearchForImportant(Itk::TestMgr * testMgr)
    {
        const wchar_t
            * word = L"important";
        testSearchFor(testMgr,
                      word);
       
    }

    void testSearchForHappening(Itk::TestMgr * testMgr)
    {
        const wchar_t
            * word = L"happening";
        testSearchFor(testMgr,
                      word);
       
    }

    void testSearchForLook(Itk::TestMgr * testMgr)
    {
        const wchar_t
            * word = L"look";
        testSearchFor(testMgr,
                      word);
       
    }

    void testSearchForChristmas(Itk::TestMgr * testMgr)
    {
        const wchar_t
            * word = L"christmas";
        testSearchFor(testMgr,
                      word);
       
    }


private:
    //
    // private methods
    //
    void testSearchFor(Itk::TestMgr  * testMgr,
                       const wchar_t * word, int expected = 1)
    {
        wchar_t
            report[512];

        cpix_Query
            * query = cpix_QueryParser_parse(queryParser_,
                                             word);

        if (cpix_Succeeded(queryParser_))
            {
                cpix_Hits
                    * hits = cpix_IdxDb_search(util_->idxDb(),
                                               query);

                if (cpix_Succeeded(util_->idxDb()))
                    {
                        util_->printHits(hits,
                                         testMgr);
                    }
                else
                    {
                        cpix_Error_report(util_->idxDb()->err_,
                                          report,
                                          sizeof(report)/sizeof(wchar_t));
 
                        ITK_EXPECT(testMgr,
                                   false,
                                   "Failed to search: %S",
                                   report);
                        cpix_ClearError(util_->idxDb());
                    }

                cpix_Hits_destroy(hits);
            }
        else
            {
                cpix_Error_report(query->err_,
                                  report,
                                  sizeof(report)/sizeof(wchar_t));

                ITK_EXPECT(testMgr,
                           false,
                           "Failed to parse '%S': %S",
                           word,
                           report);

                cpix_ClearError(query);
            }
        
        cpix_Query_destroy(query);
    }


    void cleanup()
    {
        delete util_;
        util_ = NULL;

        cpix_Analyzer_destroy(analyzer_);
        analyzer_ = NULL;

        cpix_QueryParser_destroy(queryParser_);
        queryParser_ = NULL;
    }

};


Itk::TesterBase * CreateAggregateTests()
{
    using namespace Itk;

    MultiFieldContext
        * context = new MultiFieldContext;

    ContextTester
        * contextTester = new ContextTester("aggregate",
                                            context);


#define TEST "adding"
    contextTester->add(TEST,
                       context,
                       &MultiFieldContext::testAddFiles,
                       TEST);
#undef TEST

#define TEST "searchHappy"
    contextTester->add(TEST,
                       context,
                       &MultiFieldContext::testSearchForHappy,
                       TEST);
#undef TEST

#define TEST "searchImportant"
    contextTester->add(TEST,
                       context,
                       &MultiFieldContext::testSearchForImportant,
                       TEST);
#undef TEST

#define TEST "searchHappening"
    contextTester->add(TEST,
                       context,
                       &MultiFieldContext::testSearchForHappening,
                       TEST);
#undef TEST

#define TEST "searchLook"
    contextTester->add(TEST,
                       context,
                       &MultiFieldContext::testSearchForLook,
                       TEST);
#undef TEST

#define TEST "searchChristmas"
    contextTester->add(TEST,
                       context,
                       &MultiFieldContext::testSearchForChristmas,
                       TEST);
#undef TEST


    // TODO add more tests to suite

    return contextTester;    
}
