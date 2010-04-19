/*------------------------------------------------------------------------------
* Copyright (C) 2010 Nokia Corporation and/or its subsidiary(-ies). 
* 
* Distributable under the terms of either the Apache License (Version 2.0) or 
* the GNU Lesser General Public License, as specified in the COPYING file.
------------------------------------------------------------------------------*/

/*
 * selectivetermsearch.cpp
 *
 *  Created on: Jun 3, 2009
 *      Author: admin
 */

#include <iostream>
#include <fstream>
#include <sstream>
#include <bitset>
#include <exception>
#include <stdexcept>
#include <queue>

#include "itk.h"
#include "itkperf.h"
#include "cpixfstools.h"

#include "clucene.h"
#include "clucene/search/termquery.h"
#include "clucene/analysis/standard/StandardAnalyzer.h"

#include "corpus.h"
#include "config.h"

#define CONTENTS_FIELD 	L"Contents"
#define GROUP_FIELD 	L"_appclass"
#define GROUP_1 		L"root file usrdoc text fiction"
#define GROUP_2 		L"root file usrdoc text poetry"
#define FILTER_FIELD    GROUP_FIELD

const wchar_t* TestTerms[] = {
	L"exception",
	L"everywhere",
	L"realized",
	L"terrible",
	L"mieleni",
	L"merelliselle",
	L"p‰‰st‰m‰h‰n",
	L"venehen",	
	NULL
};

using namespace lucene::analysis::standard; 
using namespace lucene::document; 
using namespace lucene::index; 
using namespace lucene::search;

// GetTerms implementation details
namespace {

    // Following definitions are needed for memory management and sorting of 
    // getTerms operations: 

    // Document entry definition. First integer contains the document frequence
    typedef std::pair<int, std::wstring> TermEntry;

    // Comparation method for sorting document entries
    /*bool operator<(TermEntry first, TermEntry second) {
        return first.first < second.first; 
    }
    */

    typedef std::vector<TermEntry> TermEntryVector;
    
    typedef std::priority_queue<TermEntry> TermEntryQueue;
    
}


/**
 * NOTE: This was copied form CPix. It is used for performance comparison.  
 */
std::auto_ptr<std::vector<TermEntry> > 
GetTerms(lucene::index::IndexReader * reader,
		 const wchar_t              * field,
		 const wchar_t              * wildcard,
		 int                          maxHits)
{
    using namespace std; 
    using namespace lucene::document; 
    using namespace lucene::index;
    using namespace lucene::search;
			
    // Check that wilcard string is valid, WildcardTermEnum will panic otherwise
    const wchar_t* sindex = wcschr( wildcard, 
                                    LUCENE_WILDCARDTERMENUM_WILDCARD_STRING );
    const wchar_t* cindex = wcschr( wildcard, 
                                    LUCENE_WILDCARDTERMENUM_WILDCARD_CHAR );
    if (!sindex && !cindex)
    {
		throw std::invalid_argument(std::string("missing wildcard in term")); 
    }
		
    typedef std::vector<int> IntVector;
    typedef std::greater<int> GreaterInt;
		
    TermEntryQueue found; 
    std::priority_queue<int, IntVector, GreaterInt> docFreqList; 
		
    // Setup wildcard term enumeration
    Term
        * term = _CLNEW Term( field, wildcard ); // increases reference

    try 
        {
            WildcardTermEnum terms( reader, term ); // increases reference
            
            do {
                Term
                    * t = terms.term(false);
            
                if (t == NULL)
                    {
                        break;
                    }

                // Extract information out of current term. Document
                // frequence needs to be converted into as string
                const wchar_t
                    * termText = t->text();
            
                int 
                    docFreq = terms.docFreq();
            
                if (found.size() < maxHits || docFreq > docFreqList.top()) 
                {   // filter out all documents with low document frequence 
					found.push( TermEntry( docFreq, std::wstring( termText ) ) ); 
				
					// maintain knowledge of the last document frequence
					if (found.size() >= maxHits) docFreqList.pop();
					docFreqList.push(docFreq); 
                } 
            
            } while (terms.next());
        }
    catch (...)
        {
            // who knows if auto_ptr is compatible with _CL??? macros
            // - doing it manually
            _CLDECDELETE( term );
            throw;
        }

    _CLDECDELETE( term );

    // extract the sorted documents into hit document list safely
    std::auto_ptr<std::vector<TermEntry> > hits( new std::vector<TermEntry>() ) ; 

    while (!found.empty() && maxHits--) 
        {
            // Pick documents one by one. This will reverse the order
			TermEntry
                entry = found.top(); 

            found.pop(); // releases ownership
            hits->push_back( entry ); // grants ownership
        }

    return hits; 
}

/**
 * The Brand new GetTerms method. Only documents, which filterField
 * matches filter wildcard will pass. 
 * 
 * @param filterField 
 */
std::auto_ptr<std::vector<TermEntry> > 
AdvGetTerms(lucene::index::IndexReader * reader,
		 const wchar_t              * field,
		 const wchar_t              * wildcard,
		 const wchar_t			    * filterField, 
		 const wchar_t 				* filterPrefix,
		 int                          maxHits)
{
    using namespace std; 
    using namespace lucene::document; 
    using namespace lucene::index;
    using namespace lucene::search;
			
    // Check that wilcard string is valid, WildcardTermEnum will panic otherwise
    const wchar_t* sindex = wcschr( wildcard, 
                                    LUCENE_WILDCARDTERMENUM_WILDCARD_STRING );
    const wchar_t* cindex = wcschr( wildcard, 
                                    LUCENE_WILDCARDTERMENUM_WILDCARD_CHAR );
    if (!sindex && !cindex)
    {
		throw std::invalid_argument(std::string("missing wildcard in term")); 
    }
		
    typedef std::vector<int> IntVector;
    typedef std::greater<int> GreaterInt;
		
    TermEntryQueue found; 
    std::priority_queue<int, IntVector, GreaterInt> docFreqList; 
		
    // Setup wildcard term enumeration
    Term
        * term = _CLNEW Term( field, wildcard ); // increases reference
    
    std::auto_ptr<TermDocs> docs( reader->termDocs() );

    FieldCacheAuto
       * cache = FieldCache::DEFAULT->getStrings(reader, filterField); // not owned.
    
    try 
        {
            WildcardTermEnum terms( reader, term ); // increases reference
            
            do {
                Term
                    * t = terms.term(false);
            
                if (t == NULL)
                    {
                        break;
                    }
                
                int docFreq = 0; 
                
                docs->seek(t); 
                while (docs->next()) {
					int doc = docs->doc();
					const wchar_t* docField = cache->stringArray[doc];
					int i = 0; 
					for (;filterPrefix[i]; i++) {
						if (docField[i] != filterPrefix[i]) break;
					}
					if (!filterPrefix[i]) {
						docFreq++; 
					}
                }
                
                if (docFreq > 0) {
					// Extract information out of current term. Document
					// frequence needs to be converted into as string
					const wchar_t
						* termText = t->text();
				
					if (found.size() < maxHits || docFreq > docFreqList.top()) 
					{   // filter out all documents with low document frequence 
						found.push( TermEntry( docFreq, std::wstring( termText ) ) ); 
					
						// maintain knowledge of the last document frequence
						if (found.size() >= maxHits) docFreqList.pop();
						docFreqList.push(docFreq); 
					} 
                }
				
            } while (terms.next());
        }
    catch (...)
        {
            // who knows if auto_ptr is compatible with _CL??? macros
            // - doing it manually
            _CLDECDELETE( term );
            throw;
        }
    
    _CLDECDELETE( term );

    // extract the sorted documents into hit document list safely
    std::auto_ptr<std::vector<TermEntry> > hits( new std::vector<TermEntry>() ) ; 

    while (!found.empty() && maxHits--) 
        {
            // Pick documents one by one. This will reverse the order
			TermEntry
                entry = found.top(); 

            found.pop(); // releases ownership
            hits->push_back( entry ); // grants ownership
        }
    
    return hits; 
}


void termSearchTest(Itk::TestMgr* ) {
	std::auto_ptr<IndexReader> reader( IndexReader::open( INPUT_INDEX_DIRECTORY ) );

	wcout<<L"Traditional term search."<<endl<<flush; 


	long totalMs = 0; 
	long totalOperations = 0; 
	

	for (int i = 0; TestTerms[i]; i++) {
		std::wstring term( TestTerms[i] ); 
		
		for (int j = 1; j < term.length(); j++) {
			std::wstring card;
			card += term.substr(0, j) + L"*";
			
			std::auto_ptr<std::vector<TermEntry> >
				terms;
			
			std::string name;
			name += "getterms-";
			wcout<<L"Searching terms for "<<card; 
			for (int k = 0; k < card.length(); k++) name += card[k]; 
			{
				Itk::Timestamp begin, end;
				Itk::getTimestamp(&begin);

				terms = GetTerms( reader.get(), CONTENTS_FIELD, card.c_str(), 5);
				
				Itk::getTimestamp(&end);
				long operationMs = Itk::getElapsedMs(&end, &begin);
				totalMs += operationMs; 
				totalOperations++; 
				cout<<" took "<<operationMs<<" ms."<<endl; 
				
				// Dump terms
				for (int i = 0; i < terms->size(); i++) {
					wcout<<(*terms)[i].second<<L" ("<<(*terms)[i].first<<L")"<<endl;  
				}
			}
			
		}
	}
	reader->close(); 
	cout<<"All term searches together took "<<totalMs<<" ms."<<endl; 
	cout<<totalMs<<" ms per search."<<endl<<endl; 
}

void advTermSearchTest(Itk::TestMgr* , const wchar_t* group) {

	std::auto_ptr<IndexReader> reader( IndexReader::open( INPUT_INDEX_DIRECTORY ) );

	wcout<<L"Advanced term search."<<endl<<flush; 
	
	{
		Itk::Timestamp begin, end;
		Itk::getTimestamp(&begin);
		FieldCacheAuto
		       * cache = FieldCache::DEFAULT->getStrings(reader.get(), FILTER_FIELD); // not owned.
		Itk::getTimestamp(&end);
		long operationMs = Itk::getElapsedMs(&end, &begin);
		wcout<<L"Loading field cache took "<<operationMs<<" ms."<<endl;
	}

	long totalMs = 0; 
	long totalOperations = 0; 

	for (int i = 0; TestTerms[i]; i++) {
		std::wstring term( TestTerms[i] ); 
		
		for (int j = 1; j < term.length(); j++) {
			std::wstring card;
			card += term.substr(0, j) + L"*";
			
			std::auto_ptr<std::vector<TermEntry> >
				terms;
			
			wcout<<L"Searching terms for "<<card; 
	
			Itk::Timestamp begin, end;
			Itk::getTimestamp(&begin);
			terms = AdvGetTerms( reader.get(), CONTENTS_FIELD, card.c_str(), FILTER_FIELD, group, 5 );
			Itk::getTimestamp(&end);
			long operationMs = Itk::getElapsedMs(&end, &begin);
			
			totalMs += operationMs; 
			totalOperations++; 
			cout<<" took "<<operationMs<<" ms."<<endl; 
			
			// Dump terms
			for (int i = 0; i < terms->size(); i++) {
				wcout<<(*terms)[i].second<<L" ("<<(*terms)[i].first<<L")"<<endl;  
			}
			
		}
	}
	reader->close(); 
	cout<<"All term searches together took "<<totalMs<<" ms."<<endl<<endl; 
	cout<<totalMs/totalOperations<<" ms per search."<<endl<<endl; 
}

void searchGroup1Test(Itk::TestMgr* testMgr) {
	advTermSearchTest(testMgr, GROUP_1); 
}

void searchGroup2Test(Itk::TestMgr* testMgr) {
	advTermSearchTest(testMgr, GROUP_2); 
}

Itk::TesterBase * CreateTermUnitTest() {
	using namespace Itk;
	
	SuiteTester
		* testSuite = 
			new SuiteTester( "terms" );

	testSuite->add( "allterms", termSearchTest, "allterms" );
	testSuite->add( "group1terms", searchGroup1Test, "group1terms" );
	testSuite->add( "group2terms", searchGroup2Test, "group2terms" );
	
	return testSuite;
}
