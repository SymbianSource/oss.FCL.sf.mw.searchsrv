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

#include "std_log_result.h"

// For testing custom analyzer
#include "CLucene.h"
#include "CLucene\analysis\AnalysisHeader.h"
#include "CLucene\util\stringreader.h"
#include "analyzer.h"
#include "analyzerexp.h"

using namespace Cpt::Lex; 
using namespace Cpt::Parser; 
using namespace Cpix::AnalyzerExp; 

void PrintToken(Cpt::Lex::Token token) {
	switch (token.type()) {
		case TOKEN_WS: printf("space"); break; 
		case TOKEN_ID: printf("id"); break;
		case TOKEN_LIT: printf("lit"); break;
		case TOKEN_STRLIT: printf("str-lit"); break;
		case TOKEN_REALLIT: printf("real-lit"); break;
		case TOKEN_INTLIT: printf("int-lit"); break;
		case TOKEN_LEFT_BRACKET: printf("lbr"); break;
		case TOKEN_RIGHT_BRACKET: printf("rbr"); break;
		case TOKEN_COMMA: printf("comma"); break;
		case TOKEN_PIPE: printf("pipe"); break;
		case TOKEN_SWITCH : printf("sw"); break;
		case TOKEN_CASE : printf("case"); break;
		case TOKEN_DEFAULT : printf("default"); break;
		case TOKEN_LEFT_BRACE : printf("lbc"); break;
		case TOKEN_RIGHT_BRACE : printf("rbc"); break;
		case TOKEN_COLON : printf("cl"); break;
		case TOKEN_TERMINATOR : printf("tr"); break;

		default: printf("unknown"); break;
	}
	printf("('%S')", (token.text()).c_str());  
}


void TestTokenization6(Itk::TestMgr * )
{
    char *xml_file = (char*)__FUNCTION__;
        assert_failed = 0;
    Cpix::AnalyzerExp::Tokenizer tokenizer; 
	Tokens source(tokenizer, 
		L"switch { "
		  L"case '_docuid', '_mimetype': keywords;"
		  L"case '_baseappclass':        whitespace>lowercase;"
		  L"default: 					 natural(en); "
		L"}");
    WhiteSpaceFilter 
        tokens(source); 

    while (tokens) PrintToken(tokens++);
    testResultXml(xml_file);
}

void TestParsing(Itk::TestMgr* )
{ 
	Cpix::AnalyzerExp::Tokenizer tokenizer; 
    char *xml_file = (char*)__FUNCTION__;
        assert_failed = 0;
	Tokens source(tokenizer, L"foobar(zap, foo, 'bar', 'a', raboof)");
	WhiteSpaceFilter tokens(source);
	Lexer lexer(tokens);

	Tokens source2(tokenizer, L" stdtokens >lowercase>stopwords(fin)>stopwords('a', 'an','the')>stem(fin)  ");
	WhiteSpaceFilter tokens2(source2);
	Lexer lexer2(tokens2);
	
	Tokens source3(tokenizer, L"foobar(zap, 0, 0.0045, 4, 'a', 9223.031)");
	WhiteSpaceFilter tokens3(source3);
	Lexer lexer3(tokens3);

	try {
		auto_ptr<Invokation> invoke = ParseInvokation(lexer); 
		lexer.eatEof(); 
		printf("Invoke identifier: %S\n", (invoke->id()).c_str()); 
		printf("%d parameters\n", invoke->params().size()); 
		auto_ptr<Piping> piping = ParsePiping(lexer2); 
		lexer2.eatEof(); 
		printf("piping done.\n"); 
		if (dynamic_cast<const Invokation*>(&piping->tokenizer())) {
			printf("Tokenizer: %S\n", dynamic_cast<const Invokation&>(piping->tokenizer()).id().c_str()); 
		}
		printf("%d filters\n", piping->filters().size()); 
		invoke = ParseInvokation(lexer3);
		lexer3.eatEof(); 
		printf("Invoke identifier: %S\n", (invoke->id()).c_str()); 
		printf("%d parameters\n", invoke->params().size()); 
	} catch (ParseException& e) {
        assert_failed = 1;
		printf("ParseException: %S\n", e.wWhat()); 
	} catch (LexException& e) {
        assert_failed = 1;	
		printf("LexException: %S\n", e.wWhat()); 
	}
	testResultXml(xml_file);
}

void TestSwitch(Itk::TestMgr* )
{ 
	Cpix::AnalyzerExp::Tokenizer tokenizer; 
    char *xml_file = (char*)__FUNCTION__;
        assert_failed = 0;
	const wchar_t* text; 
	Tokens source(tokenizer, text = 
		L"switch { "
		  L"case '_docuid', '_mimetype': keywords;"
		  L"case '_baseappclass':        whitespace>lowercase;"
		  L"default: 					 natural(en); "
		L"}");
	WhiteSpaceFilter tokens(source);
	Lexer lexer(tokens);

	try {
		auto_ptr<Piping> sw = ParsePiping(lexer); 
		lexer.eatEof(); 
		if (dynamic_cast<const Switch*>(&sw->tokenizer())) {
			const Switch* s = dynamic_cast<const Switch*>(&sw->tokenizer());
			for (int i = 0; i < s->cases().size(); i++) {
				const Case* c = s->cases()[i]; 
				printf("case "); 
				for (int j = 0; j < c->fields().size(); j++) {
					printf("%S", (c->fields()[j]).c_str());
				}
				printf(": ...\n"); 
//				wcout<<L":"<<s->def().tokenizer().id();
			}
			printf("default: ...\n");//<<s->def().tokenizer().id()<<"...;";
		}
	} catch (ParseException& e) {
		// OBS wcout<<L"ParseException: "<<e.describe(text)<<endl; 
        assert_failed = 1;
		e.setContext(text);
		printf("ParseException: %S\n", e.wWhat()); 
	} catch (LexException& e) {
		// OBS wcout<<L"LexException: "<<e.describe(text)<<endl; 
        assert_failed = 1;
		e.setContext(text);
		printf("LexException: %S\n", e.wWhat()); 
	}
	testResultXml(xml_file);
}

void TestParsingErrors(Itk::TestMgr* )
{
    char *xml_file = (char*)__FUNCTION__;
            assert_failed = 0;
	Cpix::AnalyzerExp::Tokenizer tokenizer; 
	// eof
	const wchar_t* text; 
	StdLexer eof(tokenizer, text = L"foobar(zap, foo, 'bar', 'raf', do, ");
	try {
		ParsePiping(eof); 
		eof.eatEof(); 
	} catch (ParseException& e) {
		// OBS wcout<<L"ParseException: "<<e.describe(text)<<endl; 
		e.setContext(text);
		printf("ParseException: %S\n", e.wWhat()); 
	}

	
	// Unfinished literal
	StdLexer lit(tokenizer, text = L"foobar(zap, foo, 'bar', 'a, raboof)");
	try {
		ParsePiping(lit); 
		lit.eatEof(); 
	} catch (LexException& e) { // syntax error
		// OBS wcout<<L"LexException: "<<e.describe(text)<<endl; 
		e.setContext(text);
		printf("LexException: %S\n", e.wWhat()); 
	} catch (ParseException& e) { // syntax error
		// OBS wcout<<L"ParseException: "<<e.describe(text)<<endl; 
		e.setContext(text);
		printf("ParseException: %S\n", e.wWhat()); 
	} 

	// Unknown token
	StdLexer unknown(tokenizer, text = L"foobar(!zap, foo, 'bar', 'a', raboof)");
	try {
		ParsePiping(unknown); 
		unknown.eatEof(); 
	} catch (LexException& e) { // syntax error
		// OBS wcout<<L"LexException: "<<e.describe(text)<<endl; 
		e.setContext(text);
		printf("LexException: %S\n", e.wWhat()); 
	} 
	
	// Missing comma
	StdLexer comma(tokenizer, text = L"foobar(zap, foo, 'bar', 'a' raboof)");
	try {
		ParsePiping(comma); 
		comma.eatEof(); 
	} catch (ParseException& e) {
		// OBS wcout<<L"ParseException: "<<e.describe(text)<<endl; 
		e.setContext(text);
		printf("ParseException: %S\n", e.wWhat()); 
	} 
	testResultXml(xml_file);
}


const char * CustomAnalyzerTestDocs[] = {
    FILE_TEST_CORPUS_PATH "\\en\\1.txt",
    FILE_TEST_CORPUS_PATH "\\en\\2.txt",
    FILE_TEST_CORPUS_PATH "\\en\\3.txt",
    FILE_TEST_CORPUS_PATH "\\fi\\1.txt",
    FILE_TEST_CORPUS_PATH "\\fi\\2.txt",
    NULL
};

const char DEFAULT_ENCODING[] = "UTF-8";

void PrintTokenStream(lucene::analysis::TokenStream* stream) 
{
	using namespace lucene::analysis; 
	lucene::analysis::Token token; 
	while (stream->next(&token)) {
		int pos = token.getPositionIncrement(); 
		if (pos == 0) {
			printf("|"); 
		} else {
			for (int i = 0; i < pos; i++) printf(" "); 
		}
		printf("'%S'", token.termText());
	}
	printf("\n");
}

void TestCustomAnalyzer(Itk::TestMgr * , const wchar_t* definition)
{
	using namespace lucene::analysis; 
	using namespace lucene::util; 
	using namespace Cpix; 
	using namespace std; 
	CustomAnalyzer analyzer(definition);
	
	printf("Analyzer \"%S\":\n", definition); 
	for (int i = 0; CustomAnalyzerTestDocs[i]; i++) 
	{
		printf("File !%s tokenized:\n", (CustomAnalyzerTestDocs[i]+1));
		FileReader file( CustomAnalyzerTestDocs[i], DEFAULT_ENCODING ); 
		
		TokenStream* stream = analyzer.tokenStream( L"field", &file ); 
		PrintTokenStream( stream ); 
		stream->close(); 
		_CLDELETE( stream ); 
	}
}

void TestCustomAnalyzers(Itk::TestMgr * testMgr)
{
    char *xml_file = (char*)__FUNCTION__;
        assert_failed = 0;
	TestCustomAnalyzer(testMgr, L"stdtokens");
	TestCustomAnalyzer(testMgr, L"whitespace");
	TestCustomAnalyzer(testMgr, L"whitespace>lowercase");
	TestCustomAnalyzer(testMgr, L"whitespace>accent");
	TestCustomAnalyzer(testMgr, L"letter");
	TestCustomAnalyzer(testMgr, L"letter>lowercase");
	TestCustomAnalyzer(testMgr, L"keyword");
	TestCustomAnalyzer(testMgr, L"keyword>lowercase");
	TestCustomAnalyzer(testMgr, L"stdtokens>lowercase>accent>stem(en)"); 
	TestCustomAnalyzer(testMgr, L"letter>lowercase>accent>stop(en)"); 
	TestCustomAnalyzer(testMgr, L"letter>lowercase>stop('i', 'oh', 'nyt', 'näin')"); 
	TestCustomAnalyzer(testMgr, L"letter>length(2, 4)");
	testResultXml(xml_file);
}

void TestAnalyzerWithField(Itk::TestMgr * , const wchar_t* definition, const wchar_t* field)
{
	using namespace lucene::analysis; 
	using namespace lucene::util; 
	using namespace Cpix; 
	using namespace std; 
	CustomAnalyzer analyzer(definition);
	
	printf("File !%s tokenized for field %S:\n", (CustomAnalyzerTestDocs[0]+1), field);
	FileReader file( CustomAnalyzerTestDocs[0], DEFAULT_ENCODING ); 
	
	TokenStream* stream = analyzer.tokenStream( field, &file ); 
	PrintTokenStream( stream ); 
	stream->close(); 
	_CLDELETE( stream ); 
}

void TestSwitchAnalyzers(Itk::TestMgr * testMgr)
{
    char *xml_file = (char*)__FUNCTION__;
        assert_failed = 0;
    const wchar_t* sw = L"\n"
		L"switch {\n"
		L"    case '_docuid':          keyword;\n"
		L"    case '_appclass':        whitespace>lowercase;\n"
		L"    case 'title', 'message': stdtokens>accent>lowercase>stem(en)>stop(en);\n"
		L"    default:                 letter>lowercase>stop('i');\n"
		L"}";
	TestAnalyzerWithField(testMgr, sw, L"_docuid");
	TestAnalyzerWithField(testMgr, sw, L"_appclass");
	TestAnalyzerWithField(testMgr, sw, L"Title"); 
	TestAnalyzerWithField(testMgr, sw, L"message"); 
	TestAnalyzerWithField(testMgr, sw, L"field"); 
	testResultXml(xml_file);
}


Itk::TesterBase * CreateAnalysisWhiteBoxTests()
{
    using namespace Itk;

    SuiteTester
        * analysisTests = new SuiteTester("analysiswhitebox");
    
    analysisTests->add("analyzer",
					   &TestCustomAnalyzers,
					   "analyzer");
    analysisTests->add("switchanalyzer",
					   &TestSwitchAnalyzers,
					   "switchanalyzer");
    analysisTests->add("tokenization",
    				   TestTokenization6,
    				   "tokenization");
  	analysisTests->add("parsing",
                      TestParsing,
                      "parsing");
    analysisTests->add("parsing2",
                      TestSwitch,
                      "parsing2");
    analysisTests->add("parsingerrors",
                      TestParsingErrors,
                      "parsingerrors");
    
    return analysisTests;
}



