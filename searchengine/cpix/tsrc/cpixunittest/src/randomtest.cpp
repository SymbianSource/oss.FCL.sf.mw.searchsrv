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
#include "randomtest.h"
#include "CPixdoc.h"

#include <sstream>
#include <time.h>
#include "std_log_result.h"

using namespace std;
using namespace Itk;

SuiteTester* CreateSuiteTesterRandom()
	{
	// "all/random"
	RandomTest * idxDbTest = new RandomTest;
	ContextTester * random = new ContextTester("random", idxDbTest);
	random->add( "AddingDeleting", idxDbTest,
			    &RandomTest::testIndexAndDelete,
			    "AddingDeleting");

	random->add( "AddingDeletingAll", idxDbTest,
			    &RandomTest::testDeleteAll,
			    "AddingDeletingAll");

	random->add( "AssertContent", idxDbTest,
	            &RandomTest::testContent,
	            "AssertContent");

	random->add( "Persistence", idxDbTest,
			     &RandomTest::testPersistence,
			     "Persistence");

	random->add("Searches", idxDbTest, 
	        &RandomTest::testSearches,
	        "Searches");

	random->add("CreatedIndex", idxDbTest,
	        &RandomTest::testCreatedIndex,
	        "CreatedIndex");

	return random;
	}


RandomTest::RandomTest()
: // core
  randomSeed_(0),
  idxUtil_(NULL), 
  testCorpus_(DEFAULT_TEST_CORPUS_PATH),
  // text processing
  analyzer_(NULL),
  uidQueryParser_(NULL),
  contentQueryParser_(NULL),
  // tracing
  indexed_(),
  tracedTerms_(),
  tracedTermsFreq_()
	{
	}

RandomTest::~RandomTest()
	{
	}

void RandomTest::setup() throw (Itk::PanicExc)
{
	cpix_Result
		result;

        cpix_IdxDb_dbgScrapAll(&result);
        if (cpix_Failed(&result))
            {
                ITK_PANIC("Could not dbg scrapp all indexes");
            }

	randomSeed_ = (unsigned int)time(0); 
	srandom(randomSeed_);
	
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
	if ( !uidQueryParser_)
	{
		ITK_PANIC("Query parser could not be created");
	}

	contentQueryParser_ = cpix_QueryParser_create( &result,
	                                                               LBODY_FIELD, 
	                                                               analyzer_ );
	if ( !contentQueryParser_ )
	{
		ITK_PANIC("Query parser could not be created");
	}
}

void RandomTest::tearDown() throw ()
	{
	delete idxUtil_; 
	cpix_Analyzer_destroy( analyzer_ ); 
	cpix_QueryParser_destroy( uidQueryParser_ ); 
	cpix_QueryParser_destroy( contentQueryParser_ ); 
	}

void * RandomTest::subject()
	{
	return NULL;
	}


int RandomTest::getRandomItem(bool isIndexed)
	{
	if (isIndexed)
		{ // pick indexed one from list
		int i = rand() % indexed_.size();
		for (set<size_t>::iterator itr = indexed_.begin(); itr != indexed_.end(); itr++)
			{
			if (i-- == 0)
				{
				return *itr;
				}
			}
		// TODO: Panic
		return -1;
		}
	else
		{
		while (true)
			{
			int i = rand() % testCorpus_.size();

			if (indexed_.count(i) == 0)
				{
				return i;
				}
			}
		}
	}


//
// TRACING ->
//

void RandomTest::markIndexed(int index, bool indexed)
	{
	if (indexed_.count(index) == 1 && !indexed)
		{
		indexed_.erase(index);
		}
	else if (indexed_.count(index) == 0 && indexed)
		{
		indexed_.insert(index);
		}
	}

bool RandomTest::indexed(int item)
	{
	return indexed_.count(item)==1;
	}

void RandomTest::traceTerm(Itk::TestMgr* , const wchar_t* term)
	{
	tracedTerms_.push_back( wstring( term ) );
	tracedTermsFreq_.push_back( 0 );
	}

void RandomTest::untraceTerm(Itk::TestMgr* , const wchar_t* term)
	{
	int i = 0;
	for (; i < tracedTerms_.size(); i++)
		{
		if ( term == tracedTerms_[i] )
			{
			tracedTerms_.erase( tracedTerms_.begin() + i );
			tracedTermsFreq_.erase( tracedTermsFreq_.begin() + i );
			return;
			}
		}
	ITK_PANIC( "Untracing non-traced term" );
	}

void RandomTest::untraceTerms(Itk::TestMgr* )
	{
	tracedTerms_.empty();
	tracedTermsFreq_.empty();
	}

// Would be useful to get access to the tokenizers
bool RandomTest::isTokenChar(wchar_t character) const 
	{
	// only for ASCII
	if ( character >= 'a' && character <= 'z') 
		{
		return true;
		}
	if ( character >= 'A' && character <= 'Z') 
		{
		return true;
		}
	return false;
	}

bool RandomTest::containsTerm(const std::wstring& content, const std::wstring& term) const
{
	wostringstream tolowcase; 
	for (int i = 0; i < content.size(); i++) {
		wchar_t c = content[i];
		if (c >= 'A' && c <= 'Z') {
			c = (c - 'A' + 'a');
		} 
		tolowcase<<c;
	}
	wstring lowcase( tolowcase.str() ); 
	const wchar_t* lc = lowcase.c_str(); 
	
	int i = 0; 
	while (true)
	{
		i = lowcase.find( term, i );
		if ( i == string::npos )
		{
			return false;
		}
		// we didn't match e.g. 'hat' from hatchet
		if ((i == 0 || !isTokenChar(lowcase[i-1])) && 
			(i + term.size() >= lowcase.size() || !isTokenChar(lowcase[i+term.length()])))
		{
			return true; 
		}
		i++;
	} 	
}



void RandomTest::incTermFreqs(Itk::TestMgr* /*testMgr*/, const std::wstring& content)
	{
	for (int i = 0; i < tracedTerms_.size(); i++)
		{
		if ( containsTerm( content, tracedTerms_[i] ) ) 
			{
			tracedTermsFreq_[i]++;
			}
		}
	}

void RandomTest::decTermFreqs(Itk::TestMgr* , const std::wstring& content)
	{
	for (int i = 0; i < tracedTerms_.size(); i++)
		{
		if ( containsTerm( content, tracedTerms_[i] ) ) 
			{
			tracedTermsFreq_[i]--;
			}
		}
	}

//
// Indexing & deleting
//


bool RandomTest::tryIndexItem(Itk::TestMgr* testMgr, size_t item)
	{
	wstring id = GetItemId( item );
	wstring content = testCorpus_.item( item );

	const wchar_t
	* fields[4];
	fields[0] = L"+3585553412"; // to
	fields[1] = L"+3585559078"; // from
	fields[2] = L"inbox"; // folder
	fields[3] = content.c_str();// body

	cpix_IdxDb_add2( idxUtil_->idxDb(),
					 idxUtil_->schemaId(),
					 id.c_str(),
					 SMSAPPCLASS,
					 content.c_str(),
					 NULL,
					 fields,
					 analyzer_ );
	
	if ( cpix_Succeeded( idxUtil_->idxDb() ) )
		{
		markIndexed(item, true);
		incTermFreqs(testMgr, content);
		return true; 
		}
	else 
		{
		return false;
		}
	}

void RandomTest::indexItem(Itk::TestMgr* testMgr, size_t item) 
{
	ITK_ASSERT( testMgr, tryIndexItem(testMgr, item), "Inserting %d failed", item ); 
}

bool RandomTest::tryDeleteItem(Itk::TestMgr* testMgr, size_t item)
{
	wstring id = GetItemId(item );
	
	cpix_IdxDb_deleteDocuments(idxUtil_->idxDb(), id.c_str());
	
	if ( cpix_Succeeded( idxUtil_->idxDb() ) )
		{
		markIndexed( item, false );
		wstring content = testCorpus_.item( item );
		decTermFreqs( testMgr, content );
		return true; 
		}
	return false; 
}
void RandomTest::deleteItem(Itk::TestMgr* testMgr, size_t item)
{
	ITK_ASSERT( testMgr, tryDeleteItem(testMgr, item), "Deleting %d failed", item); 
}
    
size_t RandomTest::indexRandom(Itk::TestMgr* testMgr)
	{
	int ret = getRandomItem( false );
	indexItem( testMgr, ret );
	return ret;
	}

void RandomTest::indexRandoms(Itk::TestMgr* testMgr, size_t n)
	{
	for (int i = 0; i < n; i++)
		{
		indexRandom(testMgr);
		}
        idxUtil_->flush();
	}

size_t RandomTest::deleteRandom(Itk::TestMgr* testMgr)
	{
	int ret = getRandomItem( true );
	deleteItem( testMgr, ret );
	return ret;
	}

void RandomTest::deleteRandoms(Itk::TestMgr* testMgr, size_t n)
	{
	for (int i = 0; i < n; i++)
		{
		deleteRandom(testMgr);
		}
        idxUtil_->flush();
	}

bool RandomTest::validState(Itk::TestMgr* testMgr, size_t item)
	{
	bool ret = false;
	wstring id = GetItemId( item );

	cpix_Query* query =
	cpix_QueryParser_parse( uidQueryParser_, id.c_str() );

	if ( query )
		{
		cpix_Hits *hits =
		cpix_IdxDb_search(idxUtil_->idxDb(),
				query);

		if ( hits )
			{
			if ( indexed( item ) )
				{
				ret = ( cpix_Hits_length( hits ) == 1 );
				}
			else
				{
				ret = ( cpix_Hits_length( hits ) == 0 );
				}
			cpix_Hits_destroy( hits );
			}
		cpix_Query_destroy( query );
		}
	else
		{
		ITK_MSG( testMgr, "Creating query failed" );
		}
	return ret;
	}

void RandomTest::assertState(Itk::TestMgr* testMgr, size_t item)
	{
	bool valid = validState( testMgr, item );

	if ( indexed( item ) )
		{
		ITK_ASSERT( testMgr,
				    valid,
				    "Indexed item not found" );
		}
	else
		{
		ITK_ASSERT( testMgr,
				    valid,
				    "Removed item found" );
		}
	if(!valid)
	    {
            assert_failed = 1;
	    }
	}

void RandomTest::assertContent(Itk::TestMgr* testMgr, size_t item)
{
    wstring content = testCorpus_.item(item);

    wostringstream queryText;
    queryText<<"\"";
    queryText<<content;
    queryText<<"\"";

    cpix_Query* query =
	cpix_QueryParser_parse( contentQueryParser_, queryText.str().c_str() );

    if ( query )
        {
            cpix_Hits *hits =
		cpix_IdxDb_search(idxUtil_->idxDb(),
                                  query);

            if ( hits )
                {
                    if ( indexed( item ) )
                        {
                            bool found = false;
                            for ( int i = 0; i < cpix_Hits_length( hits ); i++ )
                                {
                                    cpix_Document doc;
                                    cpix_Hits_doc( hits, i, &doc );

                                    const wchar_t* id = cpix_Document_getFieldValue( &doc, LCPIX_DOCUID_FIELD );

                                    if ( id )
                                        {
                                            wstring str( id );
                                            if ( GetItemIndex( str.c_str() ) == item )
                                                {
                                                    found = true;
                                                    break;
                                                }
                                        }
                                }
                            if ( !found )
                                {
                                    assert_failed = 1;
                                    ITK_MSG( testMgr, "Failed content %d", item );
                                }

                            // TODO should not assert here, because then
                            // cpix_Hits instance leaks here. Use
                            // ITK_EXPECT - that does not throw
                            ITK_ASSERT( testMgr,
                                        found,
                                        "Content check failed for %d" );

                        }
                    else
                        {
                           assert_failed = 1;
                            ITK_MSG( testMgr, "Content asserting is unreliable for item not in index" );
                        }
                    cpix_Hits_destroy( hits );
                }
            cpix_Query_destroy( query );
        }
    else
        {
            assert_failed = 1;
            ITK_MSG( testMgr, "Creating query failed for %S", queryText.str().c_str() );
        }
}

void RandomTest::assertAllItemsState(Itk::TestMgr* testMgr)
	{
	for ( int i = 0; i < testCorpus_.size(); i++ )
		{
		assertState(testMgr, i);
		}
	}

void RandomTest::assertItemsState(Itk::TestMgr* testMgr, vector<size_t>& vector)
	{
	for (int i = 0; i < vector.size(); i++)
		{
		assertState(testMgr, vector[i]);
		}
	}

size_t RandomTest::termMatchCount(Itk::TestMgr* testMgr, wstring& term)
	{
	wostringstream queryText;
	queryText<<"\"";
	queryText<<term;
	queryText<<"\"";

	cpix_Query* query =
	cpix_QueryParser_parse( contentQueryParser_, queryText.str().c_str() );

        int
            matches = 0;

	if ( query )
		{
		cpix_Hits *hits =
		cpix_IdxDb_search(idxUtil_->idxDb(),
				query);

		if ( hits )
			{
			for ( int i = 0; i < cpix_Hits_length( hits ); i++ )
				{
				cpix_Document doc;
				cpix_Hits_doc( hits, i, &doc );

				const wchar_t* id = cpix_Document_getFieldValue( &doc, LCPIX_DOCUID_FIELD );

				if ( id )
					{
					wstring str( id );
					size_t index = GetItemIndex( str.c_str() );
					wstring content( testCorpus_.item( index ) );

                                        // TODO expect instead of
                                        // assert otherwise cpix_Hits
                                        // instance leaks
					ITK_ASSERT( testMgr,
								containsTerm( content, term ),
								"False positive? Term %S not found in %S", term.c_str(), content.c_str() );
					matches++;
					}
				}
			cpix_Hits_destroy( hits );
			}
		cpix_Query_destroy( query );
		}
	else
		{
		ITK_MSG( testMgr, "Creating query failed" );
		}
        return matches;
	}

void RandomTest::assertTracedTerms(Itk::TestMgr* testMgr)
	{
	for (int i = 0; i < tracedTerms_.size(); i++)
		{
		int matches = termMatchCount( testMgr, tracedTerms_[i] );
		ITK_ASSERT(testMgr,
				matches == tracedTermsFreq_[i],
				"Term %S frequence is %d. Expected %d",
				tracedTerms_[i].c_str(),
				matches,
				tracedTermsFreq_[i] );
		if(matches != tracedTermsFreq_[i])
		    {
                assert_failed = 1;
		    }
		}
	}

void RandomTest::assertIndexedState(Itk::TestMgr* testMgr)
	{
	bool valid = true;
	for (set<size_t>::iterator itr = indexed_.begin(); itr != indexed_.end(); itr++)
		{
		if ( !( valid &= validState( testMgr, *itr ) ) )
			{
			ITK_MSG( testMgr, "Indexed item %d not in index", *itr );
			}
		}
	ITK_ASSERT( testMgr, valid, "Operations were not succesful" );
	}


//
// Test cases
//

void RandomTest::testIndexAndDelete(Itk::TestMgr* testMgr)
	{
    ITK_REPORT(testMgr,
               "RandomSeed",
               "%d",
               randomSeed_);
    char *xml_file = (char*)__FUNCTION__;
    assert_failed = 0;
	indexRandoms(testMgr, 50);
	assertIndexedState(testMgr);

	bool valid = true;
	for (int i = 0; i < 5; i++)
		{
		for (int i = 0; i < 10; i++)
			{
                            // OBS int item = -1;
                            int 
                                item = indexRandom(testMgr);
                            idxUtil_->flush();
                            valid = valid && validState(testMgr,
                                                        item);
                            // OBS if ( !( valid &= validState( testMgr, item = indexRandom( testMgr ) ) ) )
                            if (!valid)
                            {
                            assert_failed = 1;
                            ITK_MSG( testMgr, "Indexing item %d had failed", item );
                            goto mainloop_exit;
                            // double break
                            }
			}
		for (int i = 0; i < 10; i++)
			{
                            // OBS int item = -1;
                            int
                                item = deleteRandom(testMgr);
                            idxUtil_->flush();
                            valid = valid && validState(testMgr,
                                                        item);
                            // OBS if ( !( valid &= validState( testMgr, item = deleteRandom( testMgr ) ) ) )
                            if (!valid)
				{
                assert_failed = 1;
				ITK_MSG( testMgr, "Deleting item %d had failed", item );
				goto mainloop_exit; // double break
				}
			}
		}
	deleteRandoms(testMgr, 50);

	mainloop_exit:
	testResultXml(xml_file);
	ITK_ASSERT( testMgr, valid, "Operations were not succesful" );
	}

void RandomTest::testDeleteAll(Itk::TestMgr* testMgr)
	{
	indexRandoms(testMgr, 50);
	assertIndexedState(testMgr);
	char *xml_file = (char*)__FUNCTION__;
	assert_failed = 0;
	vector<int> deleted;
	for (int i = 0; i < 50; i++)
		{
		deleted.push_back( deleteRandom(testMgr) );
		}
        idxUtil_->flush();
	bool valid = true;
	for (int i = 0; i < deleted.size(); i++)
		{
                    // OBS if ( !( valid &= validState( testMgr, deleted[i] ) ) )
	valid = valid && validState( testMgr, deleted[i] );
                    if ( !( valid ) )
			{
                    assert_failed = 1;
			ITK_MSG( testMgr, "Deleting item %d had failed", deleted[i] );
			break;
			}
		}
	testResultXml(xml_file);
	ITK_ASSERT( testMgr, valid, "Deletes were not succesful" );
	}

void RandomTest::testContent(Itk::TestMgr* testMgr)
	{
    char *xml_file = (char*)__FUNCTION__;
    assert_failed = 0;
	for (int i = 0; i < 10; i++)
		{
		int item = indexRandom(testMgr);
                idxUtil_->flush();
		assertContent(testMgr, item);
		}

	// clean  up 
	deleteRandoms( testMgr, 10 );
        idxUtil_->flush();
        testResultXml(xml_file);
	}

void RandomTest::testPersistence(Itk::TestMgr* testMgr)
	{
	indexRandoms(testMgr, 20);
	assertIndexedState(testMgr);
    char *xml_file = (char*)__FUNCTION__;
    assert_failed = 0;
	idxUtil_->reload();
	
	assertIndexedState(testMgr);
	vector<int> deleted;
	for (int i = 0; i < 20; i++)
		{
		deleted.push_back( deleteRandom(testMgr) );
		}

	idxUtil_->reload();

	bool valid = true;
	for (int i = 0; i < deleted.size(); i++)
		{
                    // OBS if ( !( valid &= validState( testMgr, deleted[i] ) ) )
                    valid = valid && validState( testMgr, deleted[i] );
                    if ( !( valid ) )
			{
                    assert_failed = 1;
			ITK_MSG( testMgr, "Deleting item %d had failed", deleted[i] );
			break;
			}
		}
	ITK_ASSERT( testMgr, valid, "Deletes were not succesful" );
	testResultXml(xml_file);
	}

void RandomTest::testSearches(Itk::TestMgr* testMgr)
{
    char *xml_file = (char*)__FUNCTION__;
    assert_failed = 0;
	traceTerm(testMgr, L"happy");
	traceTerm(testMgr, L"people");
	traceTerm(testMgr, L"come");
	traceTerm(testMgr, L"guy");	
	traceTerm(testMgr, L"done");
	traceTerm(testMgr, L"folder");
	traceTerm(testMgr, L"hello");
	traceTerm(testMgr, L"mister");
	traceTerm(testMgr, L"would");
	traceTerm(testMgr, L"could");
	traceTerm(testMgr, L"tomorrow");
	traceTerm(testMgr, L"sorry" );
	traceTerm(testMgr, L"about");
	traceTerm(testMgr, L"what");
	
	indexRandom( testMgr ); deleteRandom( testMgr ); 
        idxUtil_->flush();

	assertTracedTerms( testMgr );
	indexRandoms( testMgr, 20 );
	assertTracedTerms( testMgr );
	deleteRandoms( testMgr, 20 );
	assertTracedTerms( testMgr );

	untraceTerms(testMgr);
	testResultXml(xml_file);
}

void RandomTest::testCreatedIndex(Itk::TestMgr* testMgr) {
    char *xml_file = (char*)__FUNCTION__;
    assert_failed = 0;
	int item = indexRandom( testMgr ); 
        idxUtil_->flush();
	assertState( testMgr, item ); 

	assertContent( testMgr, item );

	// recreate
	idxUtil_->recreate();
    markIndexed(item,
                false);
	
	// test search
	assertState( testMgr, item ); 
	
	// test delete
	idxUtil_->recreate();
	
	wstring id = GetItemId(item );

        int32_t
            deleted = cpix_IdxDb_deleteDocuments(idxUtil_->idxDb(),
                                                 id.c_str());
	
        ITK_ASSERT(testMgr,
                   cpix_Succeeded(idxUtil_->idxDb()),
                   "Trying to delete failed");
        ITK_EXPECT(testMgr,
                   deleted == 0,
                   "There should not have been an item to delete");
        if(!cpix_Succeeded(idxUtil_->idxDb()) || deleted != 0)
            {
                assert_failed = 1;
            }
        testResultXml(xml_file);
}


