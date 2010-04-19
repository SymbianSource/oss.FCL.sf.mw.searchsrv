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

#ifndef CPIXIDXDBTEST_H_
#define CPIXIDXDBTEST_H_

#include "config.h"
#include "itk.h"
#include "cpixidxdb.h"
#include "testcorpus.h"

#include <set>
#include <vector>
#include <algorithm>
#include <functional>
#include <memory>

#include "testutils.h"

/**
 * returns ownership
 */
Itk::SuiteTester* CreateSuiteTesterRandom(); 

class RandomTest : public Itk::ITestContext
{
public: // Intialization & destruction
	
	RandomTest();
	
    ~RandomTest();
    
    virtual void setup() throw (Itk::PanicExc);
    
    virtual void tearDown() throw ();
    
    SchemaId addSmsSchema(cpix_IdxDb * idxDb);

public: // 

    virtual void * subject();

public: // items and their state
	
    /**
     * Return an index of an item, that is not yet in indexed
     */
    int getRandomItem(bool indexed); 
        
public: // is indexed tracing
	
    /**
     * Informs that the item identified by index, is indexed
     */
    void markIndexed(int index, bool indexed); 

    bool indexed(int index); 

public: // term tracing
	
	void traceTerm(Itk::TestMgr* testMgr, const wchar_t* term); 
	void untraceTerm(Itk::TestMgr* testMgr, const wchar_t* term); 
	void untraceTerms(Itk::TestMgr* testMgr); 
    
	bool isTokenChar(wchar_t character) const;
	bool containsTerm(const std::wstring& content, const std::wstring& term) const;

	void incTermFreqs(Itk::TestMgr* testMgr, const std::wstring& content); 
	void decTermFreqs(Itk::TestMgr* testMgr, const std::wstring& content);
	
public: // indexing deleting
   
	bool tryIndexItem(Itk::TestMgr* testMgr, size_t item); 
	void indexItem(Itk::TestMgr* testMgr, size_t item); 
	size_t indexRandom(Itk::TestMgr* testMgr);
    void indexRandoms(Itk::TestMgr* testMgr, size_t n);

	bool tryDeleteItem(Itk::TestMgr* testMgr, size_t item); 
	void deleteItem(Itk::TestMgr* testMgr, size_t item); 
    void deleteRandoms(Itk::TestMgr* testMgr, size_t n);
    size_t deleteRandom(Itk::TestMgr* testMgr);

public: // checks and assertions

    void assertContent(Itk::TestMgr* testMgr, size_t item);
    
    bool validState(Itk::TestMgr* testMgr, size_t item);
    void assertState(Itk::TestMgr* testMgr, size_t item);
    void assertAllItemsState(Itk::TestMgr* testMgr);
    void assertItemsState(Itk::TestMgr* testMgr, std::vector<size_t>& items);
    void assertIndexedState(Itk::TestMgr* testMgr);
    
    // Term traces
    size_t termMatchCount(Itk::TestMgr* testMgr, std::wstring& term); 
    void assertTracedTerms(Itk::TestMgr* testMgr); 
    
public: // Actual tests
    
    void testIndexAndDelete(Itk::TestMgr* testMgr); 
    void testDeleteAll(Itk::TestMgr* testMgr);
    void testContent(Itk::TestMgr* testMgr);
    void testPersistence(Itk::TestMgr* testMgr);
	void testSearches(Itk::TestMgr* testMgr); 
	void testCreatedIndex(Itk::TestMgr* testMgr); 

private: // core functionality
	
	unsigned int randomSeed_;

	SmsIdxUtil *idxUtil_;
	
	LineTestCorpusRef testCorpus_;
	
private: // text processing
	
	cpix_Analyzer* analyzer_;
	
	cpix_QueryParser* uidQueryParser_;

	cpix_QueryParser* contentQueryParser_;
	
private: // tracing

	std::set<size_t> indexed_; 
	
	std::vector<std::wstring> tracedTerms_;
	
	std::vector<int> tracedTermsFreq_;
	
};

#endif /*CPIXIDXDBTEST_H_*/
