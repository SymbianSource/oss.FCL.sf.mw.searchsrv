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

// internal (inc/private) headers from cpix - that is why these tests
// are whitebox tests
#include "cpixtools.h"
#include "iidxdb.h"
#include "iqrytype.h"
#include "cpixexc.h"
#include "cpixhits.h"
#include "idxdbdelta.h"

#include "std_log_result.h"

void TestBaseAppClassCollision(Itk::TestMgr * testMgr)
{
    cpix_Result
        result;
    char *xml_file = (char*)__FUNCTION__;
    assert_failed = 0;
    cpix_IdxDb_defineVolume(&result,
                            SMS_QBASEAPPCLASS,
                            NULL);
    if(!cpix_Succeeded(&result))
        assert_failed = 1;        
    ITK_ASSERT(testMgr,
               cpix_Succeeded(&result),
               "Definition of volume (%s, %s) failed.",
               SMS_QBASEAPPCLASS,
               "<default>");

    cpix_IdxDb_defineVolume(&result,
                            SMS_QBASEAPPCLASS,
                            NULL);
    if(!cpix_Succeeded(&result))
        assert_failed = 1; 
    ITK_ASSERT(testMgr,
               cpix_Succeeded(&result),
               "Re-definition of identical volume (%s, %s) failed.",
               SMS_QBASEAPPCLASS,
               "<default>");

    const char
        * dummyPath = "\\dummy\\path";

    cpix_IdxDb_defineVolume(&result,
                            SMS_QBASEAPPCLASS,
                            dummyPath);
    if(!cpix_Succeeded(&result))
        assert_failed = 1; 
    ITK_ASSERT(testMgr,
               cpix_Failed(&result),
               "Volume (%s, %s) definition should have failed.",
               SMS_QBASEAPPCLASS,
               dummyPath);

    cpix_IdxDb_undefineVolume("@0:root foo bar");
    testResultXml(xml_file);
}


#define DUMMY_QBASEAPPCLASS "@0:root dummy"
#define DUMMY2_QBASEAPPCLASS "@0:root dummy2"
#define DUMMY_IDXDBPATH     "c:\\Data\\indexing\\indexdb\\root\\dummy"


void TestIdxDbPathCollision(Itk::TestMgr * testMgr)
{
    cpix_Result
        result;
    char *xml_file = (char*)__FUNCTION__;
    assert_failed = 0;
    cpix_IdxDb_defineVolume(&result,
                      DUMMY_QBASEAPPCLASS,
                      DUMMY_IDXDBPATH);
    if(!cpix_Succeeded(&result))
        assert_failed = 1;
    ITK_ASSERT(testMgr,
               cpix_Succeeded(&result),
               "Defining volume %s, %s failed.",
               DUMMY_QBASEAPPCLASS,
               DUMMY_IDXDBPATH);

    cpix_IdxDb_defineVolume(&result,
                      DUMMY2_QBASEAPPCLASS,
                      DUMMY_IDXDBPATH);
    if(!cpix_Succeeded(&result))
        assert_failed = 1;
    ITK_ASSERT(testMgr,
               cpix_Failed(&result),
               "Defining volume %s, %s should have failed.",
               DUMMY2_QBASEAPPCLASS,
               DUMMY_IDXDBPATH);
    testResultXml(xml_file);
}



void TestScrapAll(Itk::TestMgr * testMgr)
{
    cpix_Result
        result;
    char *xml_file = (char*)__FUNCTION__;
    assert_failed = 0;

    cpix_IdxDb_defineVolume(&result,
                            SMS_QBASEAPPCLASS,
                            NULL);
    if (cpix_Succeeded(&result))
        {
            cpix_IdxDb
                * idxDb = cpix_IdxDb_openDb(&result,
                                            SMS_QBASEAPPCLASS,
                                            cpix_IDX_CREATE);

            if (cpix_Succeeded(&result))
                {
            assert_failed = 1;
                    cpix_IdxDb_releaseDb(idxDb);
                }
        }

    ITK_ASSERT(testMgr,
               cpix_Succeeded(&result),
               "Definition and creation of IdxDb (%s, %s) failed",
               SMS_QBASEAPPCLASS,
               "<default>");

    cpix_IdxDb_dbgScrapAll(&result);

    cpix_IdxDb
        * idxDb = cpix_IdxDb_openDb(&result,
                                    SMS_QBASEAPPCLASS,
                                    cpix_IDX_OPEN);
    
    if (cpix_Succeeded(&result))
        {
            cpix_IdxDb_releaseDb(idxDb);
        }
    else
        {
            assert_failed = 1;
        }

    ITK_ASSERT(testMgr,
               cpix_Failed(&result),
               "Opening IdxDb(%s) should have failed now.",
               SMS_QBASEAPPCLASS);
    testResultXml(xml_file);
}

const wchar_t * QryStrings[] = {

    // should pass:
    L"$foo",
    L"$bar<1.2,'blabla',3>",
    L" $ bar < 1.2 , 'blabla' , 3 > ",
    L"$barf(inner query)",
    L" $ barf ( inner query ) ",
    L"$barfology<'bloblo',3.1415>(inner query)",
    L" $ barfology < 'bloblo' , 3.1415 > ( inner query ) ",
    L"some query",
    L"*",
    L"* AND some more criteria", // special case: "innery query" is the whole

    // should fail:
    L"$barf<1.23(blo)",
    L"$farb<1.23>(blo",
    L"$farb<1.23>(blo ) oops more data",
    
    
    // end of test data
    NULL
};


void TestUnifiedSearchParse(Itk::TestMgr * )
{
    using namespace std;
    char *xml_file = (char*)__FUNCTION__;
    assert_failed = 0;
    printf("Whitebox testing parsing of unified search syntax\n\n");

    const wchar_t
        ** p = QryStrings;

    for (; *p != NULL; ++p)
        {
            printf("Trying to parse %S:\n",
                   *p);

            try {
                Cpix::QryCall
                    qc(*p);

                printf("\to qry type id: %S\n",
                       qc.qryTypeId_.c_str());

                printf("\to qry arguments: ");
                list<wstring>::const_iterator
                    i = qc.args_.begin(),
                    end = qc.args_.end();
                for (; i != end; ++i)
                    {
                        printf("%S ",
                               i->c_str());
                    }
                printf("\n");

                printf("\to inner qry: '%S'\n",
                       qc.innerQryStr_.c_str());
                
            } catch (CpixExc & cpixExc) {
                assert_failed = 1;
                printf("Failed to parse: %S\n",
                       cpixExc.wWhat());
            } catch (...) {
                assert_failed = 1;
                printf("Failed t parse: for unknown reasons\n");
            }
        }
    testResultXml(xml_file);
}


class PageContext : public Itk::ITestContext
{
private:
    //
    // private members
    //
    SmsIdxUtil         * util_;
    cpix_Analyzer      * analyzer_;
    cpix_Query         * query_;
    cpix_QueryParser   * queryParser_;

    size_t               pageSize_;
    size_t               expectedHitCount_;

public:
    //
    // from interface Itk::ITestContext
    //
    virtual void setup() throw (Itk::PanicExc)
    {
        SetupSentry
            ss(*this);

        util_ = new SmsIdxUtil;
        util_->init();

        cpix_Result
            result;

        analyzer_ = cpix_CreateSimpleAnalyzer(&result);

        if (analyzer_ == NULL)
            {
                ITK_PANIC("Could not create analyzer");
            }

        queryParser_ = cpix_QueryParser_create(&result,
                                               LBODY_FIELD,
                                               analyzer_);

        if (queryParser_ == NULL)
            {
                ITK_PANIC("Could not create query parser");
            }

        query_ = cpix_QueryParser_parse(queryParser_,
                                        L"abc");

        if (cpix_Failed(queryParser_) || query_ == NULL)
            {
                ITK_PANIC("Could not parse query string");
            }

        ss.setupComplete();
    }


    virtual void tearDown() throw ()
    {
        cleanup();
    }


    virtual ~PageContext()
    {
        cleanup();
    }


    //
    // public operations
    //
    PageContext()
        : util_(NULL),
          analyzer_(NULL),
          query_(NULL),
          queryParser_(NULL),
          pageSize_(Cpix::IdxDbMgr::instance()->getClHitsPageSize())
    {
        expectedHitCount_ = 4 * pageSize_ - 3;
    }


    void testAddSome(Itk::TestMgr  * mgr)
    {
        char *xml_file = (char*)__FUNCTION__;
        assert_failed = 0;
        std::wstring
            firstBody,
            secondBody;

        for (int i = 0; i < expectedHitCount_; ++i)
            {
                generateTwoSmsBodies(i,
                                     firstBody,
                                     secondBody);

                util_->indexSms(2*i,
                                firstBody.c_str(),
                                analyzer_,
                                mgr);

                util_->indexSms(2*i + 1,
                                secondBody.c_str(),
                                analyzer_,
                                mgr);
                if ((i % 5) == 0)
                    {
                        ITK_DBGMSG(mgr,
                                   ".");
                    }
            }
        util_->flush();
        testResultXml(xml_file);
    }


    void testSearchPages(Itk::TestMgr * mgr)
    {
        cpix_Hits
            * hits = cpix_IdxDb_search(util_->idxDb(),
                                       query_);
        char *xml_file = (char*)__FUNCTION__;
        assert_failed = 0;
        if (cpix_Failed(util_->idxDb()))
            {
                ITK_EXPECT(mgr,
                           false,
                           "Failed to search index");
                cpix_ClearError(util_->idxDb());
                assert_failed = 1;
            }
        else
            {
                int32_t
                    hitsLength = cpix_Hits_length(hits);

                ITK_EXPECT(mgr,
                           expectedHitCount_ == hitsLength,
                           "Did not get expected hitcounts (%d), but %d",
                           expectedHitCount_,
                           hitsLength);
                if(expectedHitCount_ != hitsLength)
                    {
                    assert_failed = 1;
                    }
                
                printf("Got %d hits\n",
                       hitsLength);

                search(hits,
                       0,
                       2 * pageSize_,
                       mgr);

                search(hits,
                       3 * pageSize_,
                       4 * pageSize_,
                       mgr);

                search(hits,
                       0,
                       pageSize_,
                       mgr);

                search(hits,
                       2 * pageSize_,
                       4 * pageSize_,
                       mgr);

            }
        
        cpix_Hits_destroy( hits );  
        testResultXml(xml_file);
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
        query_ = NULL;;

        cpix_QueryParser_destroy(queryParser_);
        queryParser_ = NULL;
    }


    void generateTwoSmsBodies(int            idx,
                              std::wstring & firstBody,
                              std::wstring & secondBody)
    {
        static wchar_t
            firstBuf[] =  L"hello abc                ",
            secondBuf[] = L"hola, xyz                ";
        //                            ^
        enum //                       |
        {    //                       |
            VARIANT_POS =             10
        };

        wchar_t
            * p1 = firstBuf + VARIANT_POS,
            * p2 = secondBuf + VARIANT_POS;

        static const int
            radix = 'z' - 'a' + 1;

        int
            length = sizeof(firstBuf) / sizeof(wchar_t) - VARIANT_POS;

        bool
            firstRun = true;

        while (length > 0)
            {
                if (!firstRun && idx == 0)
                    {
                        *p1 = L' ';
                        *p2 = L' ';
                        break;
                    }

                int
                    digit = idx % radix;

                wchar_t
                    charDigit = L'a' + digit;

                *p1 = charDigit;
                *p2 = charDigit;

                ++p1;
                ++p2;

                idx = idx / radix;
                firstRun = false;
            }

        firstBody = firstBuf;
        secondBody = secondBuf;
    }


    void search(cpix_Hits    * hits,
                int32_t        from,
                int32_t        to,
                Itk::TestMgr * mgr)
    {
        printf("Printing hit docs from hit doc idx %d to %d\n",
               from,
               to);

        for (int32_t hitDocIdx = from; hitDocIdx < to; ++hitDocIdx)
            {
                cpix_Document
                    **doc;
                ALLOC_DOC(doc, 1);

                cpix_Hits_doc(hits,
                              hitDocIdx,
                              doc,
                              1);

                if (cpix_Failed(hits) || (doc_Fetch_Failed(doc[0])))
                    {
                        wchar_t
                            buf[92];
                        cpix_Error_report(hits->err_,
                                          buf,
                                          sizeof(buf) / sizeof(wchar_t));
                        printf("Failed to get hit doc %d: %S\n",
                               hitDocIdx,
                               buf);
                        cpix_ClearError(hits);
                        break;
                    }

                util_->printHit(doc[0],
                                mgr);
                FREE_DOC(doc, 1);
            }
    }


};


Itk::TesterBase * CreatePageTests()
{
    using namespace Itk;

    PageContext
        * pageContext = new PageContext;
    ContextTester
        * ctxtTester = new ContextTester("page",
                                         pageContext);

#define TEST "addSome"
    ctxtTester->add(TEST,
                    pageContext,
                    &PageContext::testAddSome,
                    TEST);
#undef TEST

#define TEST "searchPages"
    ctxtTester->add(TEST,
                    pageContext,
                    &PageContext::testSearchPages,
                    TEST);
#undef TEST
    
    return ctxtTester;
}




const wchar_t DELTA_QRY_TERM[] = L"hello";

struct DeltaSms
{
    const wchar_t * id_;
    const wchar_t * body_;
};


static DeltaSms DeltaSmsesToStartWith[] = {
    { L"0", L"Hello it is a nice day" },
    { L"1", L"Goodbye and fare you well" },
    { L"2", L"Did you say hello to her?" },  // to delete
    { L"3", L"No I said hola" },             // to delete
};


const wchar_t * DeltaSmsesToDelete[] = {
    L"2",
    L"3"
};


static DeltaSms DeltaSmsesToAdd[] = {
    { L"4", L"Do hello, hallo and hullo mean the same?" },
    { L"5", L"Yes, they just spell different" },
};


const char DeltaIdxDbPathBase[] = "c:\\data\\cpixunittest\\_testidx";

const wchar_t DELTA_ID_FIELD[]   = L"id";
const wchar_t DELTA_BODY_FIELD[] = L"body";


class TestInsertBuf : public Cpix::Impl::IInsertBuf
{
private:
    lucene::index::IndexWriter                   * writer_;
    lucene::store::TransactionalRAMDirectory     * ramDir_;

public:
    //
    // from interface Cpix::Impl::IInsertBuf
    //
    virtual void close()
    {
        if (writer_ != NULL)
            {
                writer_->close();
                delete writer_;
                writer_ = NULL;
            }

        if (ramDir_ != NULL)
            {
                ramDir_->close();
                _CLDECDELETE(ramDir_);
                ramDir_ = NULL;
            }
    }


    virtual bool isEmpty() const
    {
        return ramDir_ == NULL;
    }


    virtual lucene::store::TransactionalRAMDirectory * getRAMDir()
    {
        if (isEmpty())
            {
                THROW_CPIXEXC(PL_ERROR "Accessing empty ram dir");
            }

        if (writer_ != NULL)
            {
                writer_->close();
                delete writer_;
                writer_ = NULL;
            }

        return ramDir_;
    }


    virtual ~TestInsertBuf()
    {
        close();
    }


    TestInsertBuf(lucene::analysis::standard::StandardAnalyzer * analyzer)
        : writer_(NULL),
          ramDir_(NULL)
    {
        ramDir_ = _CLNEW lucene::store::TransactionalRAMDirectory();
        writer_ = new lucene::index::IndexWriter(ramDir_,
                                                 analyzer,
                                                 true,      // create
                                                 false);
    }


    lucene::index::IndexWriter * getWriter()
    {
        if (writer_ == NULL)
            {
                THROW_CPIXEXC(PL_ERROR "messed up test case");
            }

        return writer_;
    }
};


class TestIdxReader : public Cpix::Impl::IIdxReader
{
private:
    lucene::index::IndexReader                   * reader_;

public:
    //
    // from Cpix::Impl::IIdxReader interface
    //
    virtual void open(const char * clIdxPath,
                      Cpt::Mutex & dirMutex)
    {
        if (reader_ != NULL)
            {
                THROW_CPIXEXC(PL_ERROR "re-opening a reader");
            }

        Cpt::SyncRegion
            sr(dirMutex);

        reader_ = lucene::index::IndexReader::open(clIdxPath);
    }


    virtual void reopen(Cpt::Mutex & dirMutex,
                        const char * clIdxPath,
                        bool         reRead)
    {
        if (reader_ != NULL)
            {
                Cpt::SyncRegion
                    sr(dirMutex);

                reader_->close();
                delete reader_;
                reader_ = NULL;

                if (reRead)
                    {
                        reader_ = lucene::index::IndexReader::open(clIdxPath);
                    }
            }
    }


    virtual bool commitIfNecessary(Cpt::Mutex & dirMutex) 
    {
        bool
            rv = false;

        if (reader_ != NULL && reader_->hasDeletions())
            {
                Cpt::SyncRegion
                    sr(dirMutex);

                reader_->commit();

                rv = true;
            }

        return rv;
    }
    
    //
    // Lifetime mgmt
    //
    virtual ~TestIdxReader()
    {
        close(NULL);
    }

    
    TestIdxReader()
        : reader_(NULL)
    {
        ;
    }


    //
    // Own operations
    //
    void close(Cpt::Mutex * dirMutex)
    {
        if (reader_ != NULL)
            {
                // lock if we have something to lock on
                std::auto_ptr<Cpt::SyncRegion>
                    sr;

                if (dirMutex != NULL)
                    {
                        sr.reset(new Cpt::SyncRegion(*dirMutex));
                    }

                reader_->close();
                delete reader_;
                reader_ = NULL;
            }
    }


    bool isOpen() const
    {
        return reader_ != NULL;
    }


    lucene::index::IndexReader * getReader()
    {
        if (!isOpen())
            {
                THROW_CPIXEXC(PL_ERROR "accessing a closed reader");
            }

        return reader_;
    }
};



class IdxDbDeltaContext : public Itk::ITestContext
{
private:
    std::string                                    test_;

    bool                                           haveInsertBuffer_;
    bool                                           haveReader_;
    bool                                           haveDels_;
    Cpix::Impl::CommitStage                        targetCommitStage_;

    TestInsertBuf                                * insertBuf_;
    TestIdxReader                                  reader_;

    lucene::analysis::standard::StandardAnalyzer   analyzer_;

    std::string                                    deltaIdxDbPath_;
    Cpt::Mutex                                     idxMutex_;

public:

    /**
     * @param test syntax: [nw][nr][nd]-(dels_committed|ibuf_committing|ibuf_committed|merging|merged|complete)
     *
     * Where 
     *
     * (a) n,w tell if there is an insertbuffer (writer + ramdir) or not
     * (b) n,r tell if there is a reader or not
     * (c) n,d tell if there were deletions on reader or not
     * (d) the keyword after dash tell the target commit stage (that
     *     should be reached)
     * 
     * 
     */
    IdxDbDeltaContext(const char * test)
        : test_(test),
          haveInsertBuffer_(false),
          haveReader_(false),
          haveDels_(false),
          targetCommitStage_(Cpix::Impl::CS_DELS_COMMITTED),
          insertBuf_(NULL),
          deltaIdxDbPath_(DeltaIdxDbPathBase)
    {
        if (test == NULL)
            {
                ITK_PANIC("NULL as test specification for IdxDbDeltaContext");
            }

        if (strlen(test) < 5)
            {
                ITK_PANIC("Test specification '%s' too short",
                          test);
            }

        if (test[0] == 'w')
            {
                haveInsertBuffer_ = true;
            }
        else if (test[0] != 'n')
            {
                ITK_PANIC("Test char in position %d: '%c' is wrong",
                          0,
                          test[0]);
            }

        if (test[1] == 'r')
            {
                haveReader_ = true;
            }
        else if (test[1] != 'n')
            {
                ITK_PANIC("Test char in position %d: '%c' is wrong",
                          1,
                          test[1]);
            }

        if (test[2] == 'd')
            {
                haveDels_ = true;
            }
        else if (test[2] != 'n')
            {
                ITK_PANIC("Test char in position %d: '%c' is wrong",
                          2,
                          test[2]);
            }

        if (test[3] != '-')
            {
                ITK_PANIC("Test char in position %d: '%c' is wrong",
                          3,
                          test[3]);
            }

        const char
            * stageStr = test + 4;

        if (strcmp("dels_committed", stageStr) == 0)
            {
                targetCommitStage_ = Cpix::Impl::CS_DELS_COMMITTED;
            }
        else if (strcmp("ibuf_committing", stageStr) == 0)
            {
                targetCommitStage_ = Cpix::Impl::CS_IBUF_COMMITTING;
            }
        else if (strcmp("ibuf_committed", stageStr) == 0)
            {
                targetCommitStage_ = Cpix::Impl::CS_IBUF_COMMITTED;
            }
        else if (strcmp("merging", stageStr) == 0)
            {
                targetCommitStage_ = Cpix::Impl::CS_MERGING;
            }
        else if (strcmp("merged", stageStr) == 0)
            {
                targetCommitStage_ = Cpix::Impl::CS_MERGED;
            }
        else if (strcmp("complete", stageStr) == 0)
            {
                targetCommitStage_ = Cpix::Impl::CS_COMPLETE;
            }
        else
            {
                ITK_PANIC("Test stage string '%s' is wrong",
                          stageStr);
            }

        deltaIdxDbPath_ += '\\';
        deltaIdxDbPath_ += Cpix::Impl::ONE_COMPLETE_SUBDIR;
    }


    virtual void setup() throw (Itk::PanicExc)
    {
        using namespace std;
        using namespace lucene::index;
        using namespace lucene::store;

        int
            result;

        if (Cpt::directoryexists(DeltaIdxDbPathBase))
            {

                result = Cpt::removeall(DeltaIdxDbPathBase);

                if (result != 0)
                    {
                        ITK_PANIC("could not remove dir recursively: %s",
                                  DeltaIdxDbPathBase);
                    }
            }

        result = Cpt::mkdirs(deltaIdxDbPath_.c_str(),
                             0666);
        
        if (result != 0)
            {
                ITK_PANIC("Could not create dir %s",
                          deltaIdxDbPath_.c_str());
            }
                
        try
            {
                auto_ptr<IndexWriter>
                    writer(new IndexWriter(deltaIdxDbPath_.c_str(),
                                           &analyzer_,
                                           true));

                addDocs(writer.get(),
                        DeltaSmsesToStartWith,
                        sizeof(DeltaSmsesToStartWith) / sizeof(DeltaSms));

                writer->optimize();
                writer->close();
            } 
        catch (...)
            {
                ITK_PANIC("Setting up start stage index failed");
            }

        if (haveInsertBuffer_)
            {
                insertBuf_ = new TestInsertBuf(&analyzer_);
            }

        if (haveReader_)
            {
                // reader_ = IndexReader::open(deltaIdxDbPath_.c_str());
                reader_.open(deltaIdxDbPath_.c_str(),
                             idxMutex_);
            }
    }


    virtual void tearDown() throw ()
    {
        cleanup();
    }


    ~IdxDbDeltaContext()
    {
        cleanup();
    }



    void testStartStage(Itk::TestMgr * mgr)
    {
        using namespace lucene::index;
        using namespace std;
        char *xml_file = (char*)__FUNCTION__;
          assert_failed = 0;
        printf("Start state of index:\n");

        {
            auto_ptr<IndexReader>
                reader(IndexReader::open(deltaIdxDbPath_.c_str()));

            searchAndPrint(reader.get());

            reader->close();
        }

        if (haveInsertBuffer_)
            {
                addDocs(insertBuf_->getWriter(),
                        DeltaSmsesToAdd,
                        sizeof(DeltaSmsesToAdd) / sizeof(DeltaSms));

                printf("... added extra docs to insert buffer\n");
            }

        if (haveDels_)
            {
                if (!reader_.isOpen())
                    {
                assert_failed = 1;
                        ITK_PANIC("Reader should be open");
                    }

                for (size_t i = 0; 
                     i < sizeof(DeltaSmsesToDelete) / sizeof(wchar_t*);
                     ++i)
                    {
                        auto_ptr<Term>
                            term(new Term(DELTA_ID_FIELD,
                                          DeltaSmsesToDelete[i]));

                        int
                            result = reader_.getReader()->deleteDocuments(term.get());
                        
                        ITK_EXPECT(mgr,
                                   result == 1,
                                   "Should have deleted exactly one doc");
                        if(result != 0)
                            assert_failed = 1;
                        printf("... deleted doc %S\n",
                               DeltaSmsesToDelete[i]);
                    }
            }
        testResultXml(xml_file);
    }


    
    struct TestCommitStageExc
    {
        Cpix::Impl::CommitStage      commitStage_;

        TestCommitStageExc(Cpix::Impl::CommitStage commitStage)
            : commitStage_(commitStage)
        {
            ;
        }
    };


    /**
     * Throws a TestCommitStageExc (with the current commit stage
     * info) if the current commit stage info is the same as the
     * target commit stage info we want to reach, except if the target
     * commit stage is the "complete" one.
     */
    struct TestCommitStageAction
    {
        Cpix::Impl::CommitStage      targetCommitStage_;


        void operator()(Cpix::Impl::CommitStage commitStage)
        {
            if (commitStage >= targetCommitStage_
                && targetCommitStage_ != Cpix::Impl::CS_COMPLETE)
                {
                    throw TestCommitStageExc(commitStage);
                }
        }


        TestCommitStageAction(Cpix::Impl::CommitStage  targetCommitStage)
            : targetCommitStage_(targetCommitStage)
        {
            ;
        }
    };


    void testCommitToDisk(Itk::TestMgr * mgr)
    {
        using namespace lucene::search;
        using namespace Cpix::Impl;
        char *xml_file = (char*)__FUNCTION__;
          assert_failed = 0;
        // The TestCommitStageAction instance we give is to emulate
        // interrupting the committing-to-disk process at different
        // stages, and see if we can recover whatever information can
        // be from whatever has been committed to disk up to that
        // point. The interrupt is done by throwing a test exception
        // and catching it.

        /*
		CommitStage
            commitStage = targetCommitStage_;
		*/
        bool
            properlyInterrupted = false;

        try
            {
                CommitToDisk_(DeltaIdxDbPathBase,
                              insertBuf_,
                              reader_,
                              true, // re-read reader_ if there was file i/o
                              idxMutex_,
                              TestCommitStageAction(targetCommitStage_));

                if (targetCommitStage_ == CS_COMPLETE)
                    {
                        properlyInterrupted = true;
                                
                        // if there was any file i/o operation, we
                        // must have a new version of idx now
                        if (haveDels_ || haveInsertBuffer_)
                            {
                                std::string
                                    deltaIdxDbPath(DeltaIdxDbPathBase);
                                deltaIdxDbPath += '\\';
                                deltaIdxDbPath += Cpix::Impl::OTHER_COMPLETE_SUBDIR;
                                
                                ITK_EXPECT(mgr,
                                           Cpt::directoryexists(deltaIdxDbPath.c_str()),
                                           "Complete, committed, updated idx dir should exist (%s): %s",
                                           test_.c_str(),
                                           deltaIdxDbPath.c_str());
                                if(!Cpt::directoryexists(deltaIdxDbPath.c_str()))
                                    {
                                    assert_failed = 1;
                                    }
                            }
                    }
                else
                    {
                assert_failed = 1;
                        ITK_EXPECT(mgr,
                                   false,
                                   "Should have been interrupted at stage %d",
                                   targetCommitStage_);
                    }
            }
        catch (TestCommitStageExc & exc)
            {
        assert_failed = 1;
                if (exc.commitStage_ == targetCommitStage_)
                    {
                        properlyInterrupted = true;
                    }
            }
        
        ITK_EXPECT(mgr,
                   properlyInterrupted,
                   "Did not test-interrupt committing to disk at the required point");

        ITK_EXPECT(mgr,
                   !haveReader_ || reader_.isOpen(),
                   "Expected an open reader back");

        ITK_EXPECT(mgr,
                   (insertBuf_ == NULL 
                    || insertBuf_->isEmpty()
                    || targetCommitStage_ < CS_IBUF_COMMITTING),
                   "Expected the (test) insert buffer to be closed");
        if(!properlyInterrupted || (haveReader_ || reader_.isOpen() ) ||  !((insertBuf_ == NULL 
                || insertBuf_->isEmpty() || targetCommitStage_ < CS_IBUF_COMMITTING)))
            {
            assert_failed = 1;
            }
        testResultXml(xml_file);
    }


    void testRecoveredStage(Itk::TestMgr * )
    {
        char *xml_file = (char*)__FUNCTION__;
          assert_failed = 0;
        if (reader_.isOpen())
            {
                printf("Reader (either the original or the re-created):\n");
                searchAndPrint(reader_.getReader());
                reader_.close(NULL);
            }

        printf("\n\nRecovering (most of the) state of the index:\n");

        Cpix::Impl::IdxDbDelta::RecoverReader(DeltaIdxDbPathBase,
                                              idxMutex_,
                                              &reader_);

        printf("The recovered state of index:\n");

        searchAndPrint(reader_.getReader());
        testResultXml(xml_file);
    }


private:
    void cleanup()
    {
        if (insertBuf_ != NULL)
            {
                insertBuf_->close();
                delete insertBuf_;
                insertBuf_ = NULL;
            }

        reader_.close(NULL);
    }


    void addDocs(lucene::index::IndexWriter * writer,
                 const DeltaSms             * p,
                 size_t                       size)
    {
        using namespace lucene::document;
        using namespace std;

        const DeltaSms
            * pEnd = p + size;

        for (; p < pEnd; ++p)
            {
                auto_ptr<Document>
                    doc(new Document());

                doc->add(* new Field(DELTA_ID_FIELD,
                                     p->id_,
                                     Field::STORE_YES | Field::INDEX_UNTOKENIZED));
                doc->add(* new Field(DELTA_BODY_FIELD,
                                     p->body_,
                                     Field::STORE_YES | Field::INDEX_TOKENIZED));
                writer->addDocument(doc.get());
                doc.reset();
            }
    }


    void searchAndPrint(lucene::index::IndexReader * reader)
    {
        using namespace lucene::document;
        using namespace lucene::index;
        using namespace lucene::queryParser;
        using namespace lucene::search;
        using namespace std;

        auto_ptr<QueryParser>
            queryParser(new QueryParser(DELTA_BODY_FIELD,
                                        &analyzer_));
        auto_ptr<Query>
            query(queryParser->parse(DELTA_QRY_TERM));

        auto_ptr<IndexSearcher>
            searcher(new IndexSearcher(reader));

        auto_ptr<Hits>
            hits(searcher->search(query.get()));

        int32_t
            length = hits->length();

        printf("SEARCHING for '%S' resulted in %d hits:\n",
               DELTA_QRY_TERM,
               length);

        for (int32_t i = 0; i < length; ++i)
            {
                Document
                    & doc(hits->doc(i));

                printf("  o  DOC %S : %S\n",
                       doc.get(DELTA_ID_FIELD),
                       doc.get(DELTA_BODY_FIELD));
            }

        searcher->close();
    }

};



Itk::TesterBase * CreateIdxDbDeltaTest(const char * test)
{
    using namespace Itk;

    IdxDbDeltaContext
        * context = new IdxDbDeltaContext(test);
    ContextTester
        * tester = new ContextTester(test,
                                     context);
    
#define TEST "startStage"
    tester->add(TEST,
                context,
                &IdxDbDeltaContext::testStartStage,
                TEST);
#undef TEST

#define TEST "commitToDisk"
    tester->add(TEST,
                context,
                &IdxDbDeltaContext::testCommitToDisk);
#undef TEST

#define TEST "recoveredStage"
    tester->add(TEST,
                context,
                &IdxDbDeltaContext::testRecoveredStage,
                TEST);
#undef TEST
    
    return tester;
}


Itk::TesterBase * CreateIdxDbDeltaTests()
{
    using namespace Itk;

    SuiteTester
        * deltaTests = new SuiteTester("delta");

    const char 
        * deltaTestSpecs[] = {
        "nnn-dels_committed",
        "nnn-ibuf_committing",
        "nnn-ibuf_committed",
        "nnn-merging",
        "nnn-merged",
        "nnn-complete",

        "nrn-dels_committed",
        "nrn-ibuf_committing",
        "nrn-ibuf_committed",
        "nrn-merging",
        "nrn-merged",
        "nrn-complete",
        
        "nrd-dels_committed",
        "nrd-ibuf_committing",
        "nrd-ibuf_committed",
        "nrd-merging",
        "nrd-merged",
        "nrd-complete",

        "wnn-dels_committed",
        "wnn-ibuf_committing",
        "wnn-ibuf_committed",
        "wnn-merging",
        "wnn-merged",
        "wnn-complete",

        "wrn-dels_committed",
        "wrn-ibuf_committing",
        "wrn-ibuf_committed",
        "wrn-merging",
        "wrn-merged",
        "wrn-complete",

        "wrd-dels_committed",
        "wrd-ibuf_committing",
        "wrd-ibuf_committed",
        "wrd-merging",
        "wrd-merged",
        "wrd-complete"
    };

    for (size_t i = 0; i < sizeof(deltaTestSpecs) / sizeof(char*); ++i)
        {
            deltaTests->add(CreateIdxDbDeltaTest(deltaTestSpecs[i]));
        }
    
    return deltaTests;
}






Itk::TesterBase * CreateWhiteBoxTests()
{
    using namespace Itk;

    SuiteTester
        * whiteBox = new SuiteTester("whitebox");

    whiteBox->add("baccoll",
                  &TestBaseAppClassCollision,
                  "baccoll");
    whiteBox->add("idpcoll",
                  &TestIdxDbPathCollision,
                  "idpcoll");
    whiteBox->add("scrapall",
                  &TestScrapAll,
                  "scrapall");
    
    whiteBox->add("unifiedSearchParse",
                  &TestUnifiedSearchParse,
                  "unifiedSearchParse");

    whiteBox->add(CreatePageTests());

    whiteBox->add(CreateIdxDbDeltaTests());

    // TODO add more

    return whiteBox;
}
