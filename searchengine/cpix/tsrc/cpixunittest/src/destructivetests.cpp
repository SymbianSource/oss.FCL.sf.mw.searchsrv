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

#include "testutils.h"
#include "testcorpus.h"
#include "config.h"
#include "itk.h"
#include "setupsentry.h"

#include <iostream>

#include "cpixsearch.h"

#include "std_log_result.h"

// Disable test cases, which prevent running other cases
#define DISABLE_CRASHING_TEST_CASES

class DestructiveTests : public Itk::ITestContext
{
private: // data
	
    SmsIdxUtil* idxUtil_; 
    LineTestCorpusRef testCorpus_; 
		
    cpix_Analyzer* analyzer_;
    cpix_QueryParser* uidQueryParser_;
    cpix_QueryParser* contentQueryParser_;
    cpix_IdxSearcher * searcher_;
		
public: // Constructors & destructors
	
    DestructiveTests() 
        : idxUtil_(NULL),
          testCorpus_(DEFAULT_TEST_CORPUS_PATH),
          analyzer_(NULL),
          uidQueryParser_(NULL),
          contentQueryParser_(NULL),
          searcher_(NULL)
    {
        ;
    }


    ~DestructiveTests() 
    {
        cleanup();
    }
		

    void setup() throw (Itk::PanicExc) 
    {
        SetupSentry
            ss(*this);

        cpix_Result
            result;

        // cpix_setLogLevel(CPIX_LL_DEBUG);

        cpix_IdxDb_dbgScrapAll(&result);

        if (cpix_Failed(&result))
            {
                ITK_PANIC("Could not scrap all");
            }

        idxUtil_ = new SmsIdxUtil; 
        idxUtil_->init( true );

        analyzer_ = cpix_CreateSimpleAnalyzer(&result);
        if ( !analyzer_ )
            {
                ITK_PANIC("Analyzer could not be created");
            }
        
        uidQueryParser_ = cpix_QueryParser_create( &result,
                                                                   LCPIX_DOCUID_FIELD, 
                                                                   analyzer_ );
        if ( !( uidQueryParser_ ) )
            {
                ITK_PANIC("Query parser could not be created");
            }
	
        contentQueryParser_ = cpix_QueryParser_create( &result,
                                                                       LBODY_FIELD, 
                                                                       analyzer_ );
        if ( !( contentQueryParser_ ) )
            {
                ITK_PANIC("Query parser could not be created");
            }

        ss.setupComplete();
    }
		


    void tearDown() throw () 
    {
        cleanup();

        // cpix_setLogLevel(CPIX_LL_TRACE);
    }
		
public: // Test cases

    void cleanup()
    {
        cpix_Analyzer_destroy( analyzer_ ); 
        analyzer_ = NULL;
        cpix_QueryParser_destroy( uidQueryParser_ ); 
        uidQueryParser_ = NULL;
        cpix_QueryParser_destroy( contentQueryParser_ ); 
        contentQueryParser_ = NULL;
        delete idxUtil_;
        idxUtil_ = NULL;
        cpix_IdxSearcher_releaseDb(searcher_);
        searcher_ = NULL;
    }
		
    void testWritingWhenHitIterating(Itk::TestMgr* testMgr) {
        testWritingWhenHitIterating(testMgr,
                                    idxUtil_->idxDb(),
                                    &cpix_IdxDb_search,
                                    3);
       
    }


    void testWritingWhenHitIterating2(Itk::TestMgr* testMgr) {
        testWritingWhenHitIterating(testMgr,
                                    searcher(testMgr),
                                    &cpix_IdxSearcher_search,
                                    3);
       
    }

    void testInvalidation(Itk::TestMgr* testMgr) 
    {
        for (int i = 0; i < 25; i++) 
            {
                idxUtil_->indexSms( i, 
                                    testCorpus_.item(i).c_str(), 
                                    analyzer_, 
                                    testMgr, 
                                    false ); 
            }
        idxUtil_->flush();

        cpix_Query* query = cpix_QueryParser_parse( contentQueryParser_, L"ok" );
	
        ITK_ASSERT( testMgr, query, "Query parsing failed" );

        cpix_Hits *hits =
            cpix_IdxDb_search(idxUtil_->idxDb(), query );
		
        if ( cpix_Failed( idxUtil_->idxDb() ) ) {
            cpix_Query_destroy( query );
            ITK_PANIC( "Search failed" );
            
        }
        printf("Accessing hits before closing... ");

        cpix_Hits_length( hits ); 
        ITK_ASSERT( testMgr, cpix_Succeeded( hits  ), "Accessing hit length failed" ); 
        printf("OK. Hits could be accessed.\n");

        /* OBS
        // cleanup
        idxUtil_->close(); 
        */


        for (int i = 0; i < 5; i++) 
            {
                idxUtil_->indexSms( i, 
                                    testCorpus_.item(i).c_str(), 
                                    analyzer_, 
                                    testMgr, 
                                    false ); 
            }
        cpix_IdxDb_flush(idxUtil_->idxDb());
        ITK_EXPECT(testMgr,
                   cpix_Succeeded(idxUtil_->idxDb()),
                   "Flushing failed");


        cpix_IdxSearcher_releaseDb(searcher_);
        searcher_ = NULL;
        idxUtil_->close();

        // Confirm no crash and that access fails

        printf("Accessing hits after closing... \n");
        cpix_Document
            **doc;                    
        ALLOC_DOC(doc, 1);        
        
        printf("doc #0: \n");
        cpix_Hits_doc(hits,
                      0,
                      doc,
                      1);
        if (doc[0]->ptr_ != NULL) {
        ITK_EXPECT( testMgr, 
                cpix_Succeeded( hits ), 
                "Accessing hit(0) should succeeded for closed database (hits still holds a reference to its originator)." ); 

        if (cpix_Failed(hits))
            {
        wchar_t
        buf[256];
        cpix_Error_report(hits->err_,
                buf,
                sizeof(buf) / sizeof(wchar_t));
        printf("%S\n", buf);
        cpix_ClearError(hits);

            }
        }
        FREE_DOC(doc, 1);    
                    

        ALLOC_DOC(doc, 1)
        printf("doc #20: ");
        cpix_Hits_doc(hits,
                      20,
                      doc,
                      1);
        if (doc[0]->ptr_ != NULL) {
        ITK_EXPECT( testMgr, 
                cpix_Failed( hits ), 
                "Accessing hit(20) should NOT succeeded for closed database (hits still holds a reference to its originator)." ); 
        }
        
        if (cpix_Failed(hits))
            {
            wchar_t
            buf[256];
            cpix_Error_report(hits->err_,
                    buf,
                    sizeof(buf) / sizeof(wchar_t));
            printf("%S\n", buf);
            cpix_ClearError(hits);
            
            }

        
        FREE_DOC(doc, 1)
       
        cpix_Hits_destroy( hits );
        cpix_Query_destroy( query );
    }


    /**
     * The purpose of this test is to test the stack unwinding problem, 
     * which occurs e.g. if we provide bad schema and the idxdb will react 
     * by throwing an exception.
     */
    void testStackunwinding(Itk::TestMgr* testMgr) 
    {
        SchemaId wrongSchema = idxUtil_->schemaId();
        cpix_Result
            result;
		
        delete idxUtil_;
        idxUtil_ = NULL;
		
        cpix_IdxDb
            * idxDb = cpix_IdxDb_openDb(&result,
                                        SMS_QBASEAPPCLASS,
                                        cpix_IDX_OPEN);
        // Open index without redefining schema
        
        if (cpix_Failed(&result))
            {
                ITK_PANIC("Index could not be opened");
                
            }

        // Try to index things
        for (int i = 0; i < 5; i++) 
            {
                std::wstring id = GetItemId( i );
                std::wstring content = testCorpus_.item( i );

                const wchar_t
                    * fields[4];
                fields[0] = L"+3585553412"; // to
                fields[1] = L"+3585559078"; // from
                fields[2] = L"inbox"; // folder
                fields[3] = content.c_str();// body

					
                cpix_IdxDb_add2( idxDb,
                                 wrongSchema,
                                 id.c_str(),
                                 // OBS DEFAULT_APPTYPE,
                                 SMSAPPCLASS,
                                 content.c_str(),
                                 NULL,
                                 fields,
                                 analyzer_ );

                bool
                    succeeded = cpix_Succeeded(idxDb);

                ITK_ASSERT( testMgr, !succeeded, "Schema is persistent?" );

            }
       	
        cpix_IdxDb_releaseDb(idxDb);
        idxDb = NULL; 
			
        idxUtil_ = new SmsIdxUtil; 
        idxUtil_->init( true );
			
    }


private:
    template<typename IDX>
    void testWritingWhenHitIterating(Itk::TestMgr * testMgr,
                                     IDX          * idx,
                                     cpix_Hits * (* searcher)(IDX*,cpix_Query*),
                                     int32_t       matches) 
    {
        wprintf(L"Testing writing to index, while hit object is continuosly read.\n");

        // Index 
        for (int i = 0; i < 25; i++) {

            idxUtil_->indexSms( i, 
                                testCorpus_.item(i).c_str(), 
                                analyzer_, 
                                testMgr, 
                                false ); 
        }
        idxUtil_->flush();
	
        printf("25 items indexed.\n");
	
        cpix_Query* query = cpix_QueryParser_parse( contentQueryParser_, L"ok" );
			
        if ( query != NULL ) {
            cpix_Hits *hits =
                (*searcher)(idx, query );
		
            if ( hits )
                {
                    // We know that 25 first message contain this many 'ok's. 
                    ITK_EXPECT( testMgr, cpix_Hits_length( hits ) == matches,
                                "There should be %d matches instead of %d",
                                matches,
                                cpix_Hits_length(hits));
                    int length = 0; 
					 
                    printf("Hits after indexing: \n");
                    idxUtil_->printHits( hits, testMgr ); 
										
                    // Should this also crash? By the way, this is one of the document results
                    idxUtil_->indexSms( 25, 
                                        testCorpus_.item(25).c_str(), 
                                        analyzer_, 
                                        testMgr, 
                                        false ); 
					 
                    ITK_EXPECT( testMgr, cpix_Succeeded( idx ), "Adding 25th item failed" ); 
	
                    printf("1 item indexed. \n");
	
                    cpix_Hits_length( hits );
                    ITK_EXPECT( testMgr, cpix_Succeeded( hits  ), "Accessing hit of index 1 failed" ); 

                    printf("Hits after adding 1 item.\n");
                    idxUtil_->printHits( hits, testMgr ); 
								
                    cpix_IdxDb_deleteDocuments(idxUtil_->idxDb(), GetItemId( 15 ).c_str() );
                    ITK_EXPECT( testMgr, cpix_Succeeded( idxUtil_->idxDb() ), "Deleting document 15 failed" ); 

                    printf("Line 16 deleted.\n");
	
                    printf("Hits after deletion: \n");
                    idxUtil_->printHits( hits, testMgr ); 
					
                    cpix_IdxDb_flush(idxUtil_->idxDb() );
                    ITK_EXPECT( testMgr, cpix_Succeeded( idxUtil_->idxDb() ), "Flushing failed" );
                    printf("Flushed.\n");

					
                    printf("Hits after flush:\n");
                    idxUtil_->printHits( hits, testMgr, true ); 
					 
                    cpix_IdxDb_deleteDocuments(idxUtil_->idxDb(), GetItemId( 14 ).c_str() );
                    ITK_EXPECT( testMgr, cpix_Succeeded( idxUtil_->idxDb() ), "Deleting document 14 failed" ); 

                    printf("Line 15 deleted.\n");
                    printf("Hits after deletion:\n");
                    idxUtil_->printHits( hits, testMgr ); 
	
                    cpix_IdxDb_deleteDocuments(idxUtil_->idxDb(), GetItemId( 9 ).c_str() );
                    ITK_EXPECT( testMgr, cpix_Succeeded( idxUtil_->idxDb() ), "Deleting document 9 failed" ); 

                    printf("Line 10 deleted.\n");
                    printf("Hits after deletion:\n");
                    idxUtil_->printHits( hits, testMgr ); 
	
                    cpix_IdxDb_flush(idxUtil_->idxDb() );
                    ITK_EXPECT( testMgr, cpix_Succeeded( idxUtil_->idxDb() ), "Flushing failed" );
                    printf("Flushed.\n");

                    printf("Hits after flush:\n");
                    idxUtil_->printHits( hits, testMgr, true ); 
					 
                    ITK_EXPECT( testMgr, cpix_Succeeded( hits  ), "Accessing hit length failed" ); 
                    ITK_EXPECT( testMgr, length == 0, "The items were not deleted." ); 
					
                    cpix_Hits_destroy( hits );
                }
            else 
                {
                    ITK_PANIC("Hits was null"); 
           
                }
            cpix_Query_destroy( query );
        } else  {
            ITK_PANIC("Could not create query"); 

        }
        
    }


    cpix_IdxSearcher * searcher(Itk::TestMgr * )
    {
        if (searcher_ == NULL)
            {
                cpix_Result
                    result;
                searcher_ = cpix_IdxSearcher_openDb(&result,
                                                    SMSAPPCLASS);
                if (searcher_ == NULL)
                    {
                        ITK_PANIC("Searcher could not be created");
                    }
            }
        return searcher_;
    }
};





/**
 * This test case is incomplete, it is supposed to test for a fix for
 * the LuceneError flying through the catch(...) problem - but the
 * real cause is still unidentified, so this test is not enabled yet.
 */
class CLuceneErrorBugCtxt : public Itk::ITestContext
{
private:
    SmsIdxUtil * idxUtil_;
    
    cpix_Analyzer * analyzer_;
    cpix_Query    * query_;
    cpix_QueryParser * queryParser_;


public:

    CLuceneErrorBugCtxt()
        : idxUtil_(NULL),
          analyzer_(NULL),
          query_(NULL),
          queryParser_(NULL)
    {
        ;
    }


    ~CLuceneErrorBugCtxt()
    {
        cleanup();
    }


    void setup() throw (Itk::PanicExc)
    {
        SetupSentry
            ss(*this);

        idxUtil_ = new SmsIdxUtil;
        idxUtil_->init(true);

        cpix_Result
            result;

        analyzer_ = cpix_CreateSimpleAnalyzer(&result);
        if (cpix_Failed(&result))
            {
                ITK_PANIC("Analyzer could not be created");
            }


        queryParser_ = cpix_QueryParser_create(&result,
                                               LBODY_FIELD,
                                               analyzer_);

        if (cpix_Failed(&result))
            {
                ITK_PANIC("QueryParser could not be create4d");
            }

        query_ = cpix_QueryParser_parse(queryParser_,
                                        L"c*");

        if (query_ == NULL)
            {
                ITK_PANIC("Query could not be created for 'c*'");
            }
    
        ss.setupComplete();
    }
    

    void tearDown() throw ()
    {
        cleanup();
    }


    void indexGeneratedLines(Itk::TestMgr * testMgr)
    {
        static const int
            wordsPerLine = 10;

        // we generate a lot of lines, like "a... a... a...",
        // "b... b... b...", ..., "j... j... j..." even if what we are
        // interested in is "c... c... c...". Reason: we need a big
        // index to produce crash-failure.
        static const int
            prefixCount = 10;

        std::wstring
            lines[prefixCount];

        for (int i = 0; i < 1030; ++i)
            {
                if (i % 50 == 0)
                    {
                        ITK_DBGMSG(testMgr,
                                   ".");
                    }
                

                if (i % wordsPerLine == 0)
                    {
                        for (int j = 0; j < prefixCount; ++j)
                            {
                                lines[j] = L'a' + j;
                            }
                    }
                else
                    {
                        for (int j = 0; j < prefixCount; ++j)
                            {
                                lines[j] += L' ';
                                lines[j] += L'a' + j;
                            }
                    }
                
                std::wstring
                    postfix;
                
                static const int
                    charNum = int('z') - int('a') + 1;
                static const int
                    digits = 4;
                
                // we generate a postfix
                int
                    generator = i;
                for (int d = 0; d < digits; ++d)
                    {
                        postfix += char('a' + generator % charNum);
                        generator /= charNum;
                    }
                
                for (int j = 0; j < prefixCount; ++j)
                    {
                        lines[j] += postfix;
                        
                        if (i % wordsPerLine == (wordsPerLine - 1))
                            {
                                idxUtil_->indexSms(i - j,
                                                   lines[j].c_str(),
                                                   analyzer_,
                                                   testMgr,
                                                   false); // addition, not update

                                ITK_ASSERT(testMgr,
                                           cpix_Succeeded(idxUtil_->idxDb()),
                                           "Addition of c... words should have succeeded");
                            }
                    }
            }
    }


    void searchCWildCard(Itk::TestMgr * testMgr)
    {
        cpix_Hits
            * hits = cpix_IdxDb_search(idxUtil_->idxDb(),
                                       query_);

        ITK_EXPECT(testMgr,
                   cpix_Failed(idxUtil_->idxDb()),
                   "Wildcard search 'c*' should fail gracefully");

        cpix_Hits_destroy(hits);
    }



private:
    void cleanup()
    {
        cpix_Analyzer_destroy(analyzer_);
        analyzer_ = NULL;

        cpix_Query_destroy(query_);
        query_ = NULL;

        delete idxUtil_;
        idxUtil_ = NULL;

        cpix_QueryParser_destroy(queryParser_);
        queryParser_ = NULL;
    }
};


Itk::TesterBase * CreateCLuceneErrorTests()
{
    using namespace Itk;

    CLuceneErrorBugCtxt
        * cebc = new CLuceneErrorBugCtxt;
    ContextTester
        * ct = new ContextTester("clerror",
                                 cebc);

    ct->add("indexGeneratedLines",
            cebc,
            &CLuceneErrorBugCtxt::indexGeneratedLines,
            "indexGeneratedLines");
    ct->add("searchCWildCard",
            cebc,
            &CLuceneErrorBugCtxt::searchCWildCard);

    return ct;
}


Itk::TesterBase * CreateDestructiveTests()
{
    using namespace Itk;
    
    DestructiveTests* context = new DestructiveTests(); 

    SuiteTester
        * suiteTester = new Itk::ContextTester("destructive", context);

    suiteTester->add( "stackUnwinding", 
                    context,
                    &DestructiveTests::testStackunwinding,
                    "stackUnwinding");

#define TEST "writingWhenHitIterating"
    suiteTester->add(TEST, 
                     context,
                     &DestructiveTests::testWritingWhenHitIterating,
                     TEST);
#undef TEST

#define TEST "writingWhenHitIterating2"
    suiteTester->add(TEST, 
                     context,
                     &DestructiveTests::testWritingWhenHitIterating2,
                     TEST);
#undef TEST

    suiteTester->add("invalidation", 
					 context,
                     &DestructiveTests::testInvalidation,
                     "invalidation");
                
    return suiteTester;
}
