/*------------------------------------------------------------------------------
* Copyright (C) 2010 Nokia Corporation and/or its subsidiary(-ies). 
* 
* Distributable under the terms of either the Apache License (Version 2.0) or 
* the GNU Lesser General Public License, as specified in the COPYING file.
------------------------------------------------------------------------------*/

/*
 * delete.cpp
 *
 *  Created on: Jun 3, 2009
 *      Author: admin
 */


#include <iostream>
#include <fstream>
#include <sstream>
#include <bitset>

#include "itk.h"

#include "clucene.h"
#include "clucene/search/termquery.h"
#include "clucene/analysis/standard/StandardAnalyzer.h"

#include "cpixfstools.h"

#include "config.h"

#define FIELD L"field"
#define MARKER_FIELD L"marker"
#define MARKER L"marker"
#define DOCUMENTS 200

const int DELETES = 100;

using namespace lucene::analysis::standard; 
using namespace lucene::document; 
using namespace lucene::index; 
using namespace lucene::search; 

void addDocument(IndexWriter& writer, int n) {
	auto_ptr<Document> doc( new Document() );
	
	std::wostringstream out;
	out<<n; 
	doc->add( *new Field( FIELD, out.str().c_str(), Field::INDEX_TOKENIZED | Field::STORE_YES ) );
	doc->add( *new Field( MARKER_FIELD, MARKER, Field::INDEX_TOKENIZED | Field::STORE_YES ) );
	
	writer.addDocument( doc.get() );
}

bool documentExists(IndexSearcher& searcher, int i) {
	std::wostringstream out;
	out<<i; 
	
	auto_ptr<TermQuery> qry( new TermQuery( new Term( FIELD, out.str().c_str())) );
	auto_ptr<Hits> hits( searcher.search( qry.get() ) ); 
	bool exist = hits->length() > 0;
	return exist; 
}

void assertDocumentExist(Itk::TestMgr* testMgr, IndexSearcher& searcher, int i) {
	ITK_ASSERT(testMgr, documentExists(searcher, i), "Document %d not found", i);
}

void assertDocumentNotExist(Itk::TestMgr* testMgr, IndexSearcher& searcher, int i) {
	ITK_ASSERT(testMgr, !documentExists(searcher, i), "Document %d found", i);
}

void dumpContent(IndexSearcher& searcher) {
	auto_ptr<TermQuery> qry( new TermQuery( new Term( MARKER_FIELD, MARKER ) ) );
	auto_ptr<Hits> hits( searcher.search( qry.get() ) ); 
	
	wcout<<L"Database contains "<<hits->length()<<L" documents:"<<endl;
	int row = 0; 
	for (int i = 0; i < hits->length(); i++) {
		if (row++ == 10) {
			wcout<<endl; 
			row = 0; 
		}
		wcout<<hits->doc(i).get(FIELD)<<" "; 
	}
	wcout<<endl; 
}

void deletionTest(Itk::TestMgr* testMgr) {

	Cpt::removeall(INDEX_DIRECTORY);
	Cpt::mkdirs(INDEX_DIRECTORY, 7);
	
	time_t seed = time(NULL);
	wcout<<L"Random seed is "<<seed<<"."<<endl<<endl;
	srand(seed); 

	StandardAnalyzer a;
	
	std::bitset<DOCUMENTS> deletes; 
	
	for (int i = 0; i < DELETES; i++) {
		while (true) {
			int d = random() % DOCUMENTS; 
			if (!deletes[d]) {
				deletes[d] = true; 
				break; 
			}
		}
	}
	
	{	// Phase one. Setup the document
		IndexWriter writer(INDEX_DIRECTORY, &a, true);
	
		for (int i = 0; i < DOCUMENTS; i++) {
			addDocument(writer, i);
		}
		
		writer.close();
	}
	{	// Phase one+ Setup the document
		IndexSearcher searcher( INDEX_DIRECTORY ); 
	
		// Phase two. Assert document content		
		for (int i = 0; i < DOCUMENTS; i++) {
			assertDocumentExist(testMgr, searcher, i); 
		}

		dumpContent(searcher); 
		searcher.close();
		wcout<<endl; 
	}

	{	// Phase two. Delete document "1"sa
		auto_ptr<IndexReader> reader( IndexReader::open( INDEX_DIRECTORY ) );

		wcout<<"Deletes:"<<endl; 

		int deleted = 0; 
		int row = 0; 
		for (int i = 0; i < DOCUMENTS; i++) {
			if (deletes[i]) {
				if (row++ == 10) {
					wcout<<endl; 
					row = 0; 
				}
				wcout<<i<<L" "; 
				std::wostringstream buf;
				buf<<i;
				
				Term term( FIELD, buf.str().c_str() );
				deleted += reader->deleteDocuments( &term );
			}
		}
		wcout<<endl;
		wcout<<"Deleted "<<deleted<<" documents. "<<endl<<endl; 
		reader->close(); 
	}
		
	{ 	// Phase three. Assert document content
		IndexSearcher searcher( INDEX_DIRECTORY );
		
		dumpContent(searcher);

		for (int i = 0; i < DOCUMENTS; i++) {
			if (deletes[i]) {
				assertDocumentNotExist	(testMgr, searcher, i); 
			} else {
				assertDocumentExist		(testMgr, searcher, i);
			}
		}
		
		searcher.close();
	}
	
}

Itk::TesterBase * CreateDeleteUnitTest() {
	using namespace Itk;
	
	SuiteTester
		* testSuite = 
			new SuiteTester( "delete" );
	
	testSuite->add( "delete", deletionTest, "delete" );
	
	return testSuite;
}

