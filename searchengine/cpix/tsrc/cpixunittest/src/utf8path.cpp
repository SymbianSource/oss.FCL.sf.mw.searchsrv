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

#include "cpixidxdb.h"

#include "itk.h"

#include "config.h"
#include "testutils.h"
#include "setupsentry.h"


struct TestPathAndContent
{
    const char    * utf8Dir_;
    const char    * utf8File_;
    const char    * utf8Content_;
};


const struct TestPathAndContent TestPathAndContents[] = 
    {
        { "english 1\\",
          "great ideas.txt",
          "Some great ideas never catch on. London. Foobar."
        },

        { "russian 2\\",
          "Русский язык.txt",
          "Подарок обошелся в $12 тысяч Голливудский актер Брэд Питт попал в аварию в Лос-Анджелесе. Moscow. Foobar."
        },

        { "chinese 3\\",
          "中文.txt",
          "解放军今年演习强度密度\"罕见\" 开放透明度空前 高清 Beijing. Foobar."
        },

        { "japanese 4\\",
          "日本語.txt",
          "10月25日（日曜日）、バグダッドにおいて連続爆発テロが発生し、 Tokyo. Foobar."
        },

        { "finnish 5\\",
          "äÄöÖ.txt",
          "Syksyllä satoi vettä. Helsinki. Foobar."
        },

        { NULL,
          NULL,
          NULL
        },
    };





class Utf8PathContext : public Itk::ITestContext, public Cpt::IFileVisitor
{
private:
    FileIdxUtil             * util_;
    cpix_Analyzer           * analyzer_;
    cpix_Query              * query_;

    cpix_QueryParser        * queryParser_;

    Itk::TestMgr            * testMgr_;



    static const char         BaseDir_[];


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
        
        analyzer_ = cpix_CreateSimpleAnalyzer(&result);
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
                                        L"foobar");
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


    Utf8PathContext()
        : util_(NULL),
          analyzer_(NULL),
          query_(NULL),
          queryParser_(NULL),
          testMgr_(NULL)
    {
        ;
    }


    ~Utf8PathContext()
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



    /**
     * We have to generate text files manually, because even if
     * Symbian itself supports unicode, its build tools like bldmake
     * etc do not, so we can't even export test files etc. DAMN
     * SYMBIAN!
     */
    void testCreateUtf8PathFiles(Itk::TestMgr * mgr)
    {
        printf("Creating files with utf8 paths and content\n");

        using namespace std;

        for (const struct TestPathAndContent * tpac = TestPathAndContents;
             tpac->utf8Dir_ != NULL;
             ++tpac)
            {
                string
                    utf8Path(BaseDir_);

                utf8Path += tpac->utf8Dir_;

                int
                    result = Cpt::mkdirs(utf8Path.c_str(), 0666);

                ITK_ASSERT(mgr,
                           result == 0,
                           "Could not create base dir %s",
                           utf8Path.c_str());

                utf8Path += tpac->utf8File_;

                ofstream
                    ofs(utf8Path.c_str());

                ofs << tpac->utf8Content_;

                ofs.flush();

                printf("Created file at : %s\n",
                       utf8Path.c_str());
            }
    }


    void testHarvesting(Itk::TestMgr * mgr)
    {
        printf("Harvesting files with utf8 paths and content\n");

        testMgr_ = mgr;
        Cpt::traverse(BaseDir_,
                      this);
        util_->flush();
    }

    
    void testSearching(Itk::TestMgr * mgr)
    {
        printf("Searching files with utf8 paths and content\n");

        using namespace Itk;

        cpix_Hits
            * hits = cpix_IdxDb_search(util_->idxDb(),
                                       query_);
        
        if (cpix_Failed(util_->idxDb()))
            {
                ITK_EXPECT(mgr,
                           false,
                           "Failed to search index");
                cpix_ClearError(util_->idxDb());
            }
        else
            {
                util_->printHits(hits,
                                 mgr);

                cpix_Hits_destroy(hits);
            }
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


const char Utf8PathContext::BaseDir_[] = "c:\\Data\\cpixunittest\\_txt\\";



Itk::TesterBase * CreateUtf8PathTests()
{
    using namespace Itk;

    Utf8PathContext
        * ctxt = new Utf8PathContext;

    ContextTester
        * utf8Path = new ContextTester("utf8path",
                                       ctxt);
    
#define TEST "createUtf8PathFiles"
    utf8Path->add(TEST,
                  ctxt,
                  &Utf8PathContext::testCreateUtf8PathFiles,
                  TEST);
#undef TEST

#define TEST "harvest"
    utf8Path->add(TEST,
                  ctxt,
                  &Utf8PathContext::testHarvesting,
                  TEST);
#undef TEST

#define TEST "search"
    utf8Path->add(TEST,
                  ctxt,
                  &Utf8PathContext::testSearching,
                  TEST);
#undef TEST
    
    // TODO add more

    return utf8Path;
}

