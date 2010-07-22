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

#include "itk.h"

#include "thaianalysis.h"

#include "CLucene.h"

#include <iostream>

#include "evaluationtool.h"
#include "analysisunittest.h"
#include "CJKAnalyzer.h"
#include "koreananalyzer.h"
#include "ngram.h"
#include "prefixfilter.h"

#include "testutils.h"

using namespace std; 
using namespace analysis; 
using namespace lucene::analysis; 
using namespace evaluationtool; 



void doEvaluate(Itk::TestMgr* testMgr, Analyzer& analyzer, const char* testName, const char* corpusFile, const char* evalFile, Analyzer* queryAnalyzer = NULL, Analyzer* prefixAnalyzer = NULL ) 
{
	Corpus corpus(corpusFile);
	PreparedCorpus prepared(corpus, analyzer, queryAnalyzer, prefixAnalyzer);
	EvaluationRecord record(evalFile);
	
	int failed = 0; 
	int improved = 0;
	
	int timeMs = 0;
	
	for (int i = 0; i < record.length(); i++)
	{
		const wchar_t* query = record.query(i);
		
		if (!*query) continue; // skip empty queries

		Results& ideal = record.ideal(i); 
		Results& java = record.measured(i); // results for Java implementation
		
		Itk::Timestamp begin;
		Itk::getTimestamp(&begin);
		
		Results results(prepared, query);
		
		Itk::Timestamp end;
		Itk::getTimestamp(&end);

		Evaluation control( ideal, java );
		Evaluation eval( ideal, results );
		
		timeMs += Itk::getElapsedMs(&end, &begin); 
		
		wprintf(L"Q '%S' - ", query);
		printTokens(queryAnalyzer?*queryAnalyzer:analyzer, query);
	
		wprintf(L"i:");
		for (int i = 0; i < results.length(); i++) {
			if (ideal.hit(i)) {
				wprintf(L"X");
			} else {
				wprintf(L".");
			}
		}
		wprintf(L"\n");
		
		wprintf(L"j:");
		for (int i = 0; i < results.length(); i++) {
			if (java.hit(i)) {
				wprintf(L"X");
			} else {
				wprintf(L".");
			}
		}
		wprintf(L"\n"); 

		wprintf(L"c:");
		for (int i = 0; i < results.length(); i++) {
			if (eval.error(i) && !control.error(i)) {
				wprintf(L"!");
			} else if (!eval.error(i) && control.error(i)){
				wprintf(L"+"); 
			} else {
				wprintf(L"."); 
			}
		}
		wprintf(L"\n"); 
		if ( eval.errors() == control.errors() ) {
			wprintf(L"ok\n"); 
		} else if ( eval.errors() < control.errors() ) {
			wprintf(L"improved\n");
			improved++; 
		} else {
			wprintf(L"more errors!\n");
			failed++; 
		}
		wprintf(L"\n"); 
	}
	
    wprintf(L"Index size was %d KB\n", prepared.indexSize() / 1000);
	wprintf(L"Improved in %d / %d\n", improved, record.length());
	wprintf(L"Deteriorated in %d / %d\n", failed, record.length());

	
	std::string title;
	title += testName; title += " search time";
    ITK_REPORT( testMgr, title.c_str(), "%d ms / query", timeMs / record.length());
    
    title = testName; title += " index size";
    ITK_REPORT( testMgr, title.c_str(), "%d KB", prepared.indexSize() / 1000);
}

void ThaiEvaluation(Itk::TestMgr* testMgr) 
{
	ThaiAnalyzer analyzer;
	doEvaluate(testMgr, analyzer, "thai", CORPUS_DIR "thai/corpus.txt", CORPUS_DIR "thai/eval.txt");
}

void GalicianEvaluation(Itk::TestMgr* testMgr) {	
//	GalicianAnalyzer analyzer;
	standard::StandardAnalyzer analyzer;
	doEvaluate(testMgr, analyzer, "galician", CORPUS_DIR "galician/corpus.txt", CORPUS_DIR "galician/eval.txt");
}

void KoreanCjkEvaluation(Itk::TestMgr* testMgr) {	
	cjk::CJKAnalyzer analyzer;
	doEvaluate(testMgr, analyzer, "korean_cjk", CORPUS_DIR "korean/corpus.txt", CORPUS_DIR "korean/eval.txt");
}

void KoreanBigramEvaluation(Itk::TestMgr* testMgr) {	
	CjkNGramAnalyzer analyzer(2);
	doEvaluate(testMgr, analyzer, "korean_2gram", CORPUS_DIR "korean/corpus.txt", CORPUS_DIR "korean/eval.txt");
}

void KoreanUnigramEvaluation(Itk::TestMgr* testMgr) {
    CjkNGramAnalyzer analyzer(1);
    doEvaluate(testMgr, analyzer, "korean_1gram", CORPUS_DIR "korean/corpus.txt", CORPUS_DIR "korean/eval.txt");
}

void KoreanJamuUnigramEvaluation(Itk::TestMgr* testMgr) {  
    JamuNGramAnalyzer analyzer(1);
    doEvaluate(testMgr, analyzer, "jamu_1gram", CORPUS_DIR "korean/corpus.txt", CORPUS_DIR "korean/eval.txt");
}

void KoreanJamuBigramEvaluation(Itk::TestMgr* testMgr) {    
    JamuNGramAnalyzer analyzer(2);
    doEvaluate(testMgr, analyzer, "jamu_2gram", CORPUS_DIR "korean/corpus.txt", CORPUS_DIR "korean/eval.txt");
}

void KoreanEvaluation(Itk::TestMgr* testMgr) {    
    KoreanAnalyzer analyzer;
    KoreanQueryAnalyzer queryAnalyzer;
    doEvaluate(testMgr, analyzer, "korean", CORPUS_DIR "korean/corpus.txt", CORPUS_DIR "korean/eval.txt", &queryAnalyzer);
}

void ChineseBigramEvaluation(Itk::TestMgr* testMgr) {	
	CjkNGramAnalyzer analyzer(2);
	doEvaluate(testMgr, analyzer, "chinese_2gram", CORPUS_DIR "chinese_prc/corpus.txt", CORPUS_DIR "chinese_prc/eval.txt");
}

void ChineseUnigramEvaluation(Itk::TestMgr* testMgr) {
	CjkNGramAnalyzer analyzer(1);
	doEvaluate(testMgr, analyzer, "chinese_1gram", CORPUS_DIR "chinese_prc/corpus.txt", CORPUS_DIR "chinese_prc/eval.txt");
}

void HebrewEvaluation(Itk::TestMgr* testMgr) {    
    HebrewAnalyzer analyzer;
    HebrewQueryAnalyzer queryAnalyzer;
    doEvaluate(testMgr, analyzer, "hebrew", CORPUS_DIR "hebrew/corpus.txt", CORPUS_DIR "hebrew/eval.txt", &queryAnalyzer, &queryAnalyzer);
}

void FrenchEvaluation(Itk::TestMgr* testMgr) {    
	FrenchAnalyzer analyzer;
    doEvaluate(testMgr, analyzer, "french", CORPUS_DIR "french/corpus.txt", CORPUS_DIR "french/eval.txt", &analyzer, &analyzer);
}

Itk::TesterBase * CreateEvaluationTest() 
{
	using namespace Itk;
	
	SuiteTester
		* testSuite = 
			new SuiteTester( "evaluation" );
	
	testSuite->add( "thai", ThaiEvaluation, "thai" );
	testSuite->add( "galician", GalicianEvaluation, "galician" );
	testSuite->add( "korean_cjk", KoreanCjkEvaluation, "korean_cjk" );
    testSuite->add( "korean", KoreanEvaluation, "korean" );
	testSuite->add( "korean_1gram", KoreanUnigramEvaluation, "korean_1gram" );
	testSuite->add( "korean_2gram", KoreanBigramEvaluation, "korean_2gram" );
	testSuite->add( "chinese_1gram", ChineseUnigramEvaluation, "chinese_1gram" );
	testSuite->add( "chinese_2gram", ChineseBigramEvaluation, "chinese_2gram" );

    testSuite->add( "jamu_1gram", KoreanJamuUnigramEvaluation, "jamu_1gram" );
    testSuite->add( "jamu_2gram", KoreanJamuBigramEvaluation, "jamu_2gram" );

    testSuite->add( "hebrew", HebrewEvaluation, "hebrew" );
    testSuite->add( "french", FrenchEvaluation, "french" );

	return testSuite;
}

