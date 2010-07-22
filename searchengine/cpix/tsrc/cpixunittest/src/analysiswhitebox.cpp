#include <wchar.h>
#include <stddef.h>

#include <iostream>

#include "cpixidxdb.h"

#include "itk.h"

#include "config.h"
#include "testutils.h"

// For testing custom analyzer
#include "CLucene.h"
#include "CLucene\analysis\AnalysisHeader.h"
#include "CLucene\util\stringreader.h"
#include "analyzerexp.h"
#include "customanalyzer.h"

#include "localetestinfos.h"

#include "spi/locale.h"
#include "cpixstrtools.h"

using namespace Cpt::Lex; 
using namespace Cpt::Parser; 
using namespace Cpix::AnalyzerExp; 

void PrintToken(Cpt::Lex::Token token) {
	printf("%S('%S')", token.type(), token.text());  
}


void TestTokenization6(Itk::TestMgr * testMgr)
{
	Cpix::AnalyzerExp::Tokenizer tokenizer; 
	Tokens source(tokenizer, 
		L"switch { "
		  L"case '_docuid', '_mimetype': keywords;"
		  L"case '_baseappclass':        whitespace>lowercase;"
		  L"default: 					 natural(en); "
		L"}");
    StdFilter 
        tokens(source); 

    while (tokens) PrintToken(tokens++); 
}

void TestParsing(Itk::TestMgr* mgr)
{ 
	Cpix::AnalyzerExp::Tokenizer tokenizer; 
	
	Tokens source(tokenizer, L"foobar(zap, foo, 'bar', 'a', raboof)");
	StdFilter tokens(source);
	Lexer lexer(tokens);
	
	const wchar_t* text = L" stdtokens >lowercase>stopwords(fin)>stopwords('a', 'an','the')>stem(fin)  ";
	
	Tokens source3(tokenizer, L"foobar(zap, 0, 0.0045, 4, 'a', 9223.031)");
	StdFilter tokens3(source3);
	Lexer lexer3(tokens3);

	try {
		auto_ptr<Invokation> invoke = ParseInvokation(lexer); 
		lexer.eatEof(); 
		printf("Invoke identifier: %S\n", invoke->id()); 
		printf("%d parameters\n", invoke->params().size()); 
		auto_ptr<Piping> piping = ParsePiping(text); 
		printf("piping done.\n"); 
		if (dynamic_cast<const Invokation*>(&piping->tokenizer())) {
			printf("Tokenizer: %S\n", dynamic_cast<const Invokation&>(piping->tokenizer()).id()); 
		}
		printf("%d filters\n", piping->filters().size()); 
		invoke = ParseInvokation(lexer3);
		lexer3.eatEof(); 
		printf("Invoke identifier: %S\n", invoke->id()); 
		printf("%d parameters\n", invoke->params().size()); 
	} catch (ParseException& e) {
		printf("ParseException: %S\n", e.wWhat()); 
	} catch (LexException& e) {
		printf("LexException: %S\n", e.wWhat()); 
	}
}

void TestSwitch(Itk::TestMgr* mgr)
{ 
	Cpix::AnalyzerExp::Tokenizer tokenizer; 
	
	const wchar_t* text = 
		L"switch { "
		  L"case '_docuid', '_mimetype': keywords;"
		  L"case '_baseappclass':        whitespace>lowercase;"
		  L"default: 					 natural(en); "
		L"}";

	try {
		auto_ptr<Piping> sw = ParsePiping(text); 
		if (dynamic_cast<const Switch*>(&sw->tokenizer())) {
			const Switch* s = dynamic_cast<const Switch*>(&sw->tokenizer());
			for (int i = 0; i < s->cases().size(); i++) {
				const Case* c = s->cases()[i]; 
				printf("case "); 
				for (int j = 0; j < c->cases().size(); j++) {
					printf("%S", c->cases()[j]);
				}
				printf(": ...\n"); 
 //				wcout<<L":"<<s->def().tokenizer().id();
			}
			printf("default: ...\n");//<<s->def().tokenizer().id()<<"...;";
		}
	} catch (ParseException& e) {
		// OBS wcout<<L"ParseException: "<<e.describe(text)<<endl; 
		e.setContext(text);
		printf("ParseException: %S\n", e.wWhat()); 
	} catch (LexException& e) {
		// OBS wcout<<L"LexException: "<<e.describe(text)<<endl; 
		e.setContext(text);
		printf("LexException: %S\n", e.wWhat()); 
	}
}

void TestConfigSwitch(Itk::TestMgr* mgr)
{ 
	Cpix::AnalyzerExp::Tokenizer tokenizer; 
	
	const wchar_t* text = 
		L"config_switch { "
		  L"case 'indexing': 	korean;"
		  L"case 'query':       koreanquery;"
		  L"case 'prefix':      letter;"
		  L"default: 			korean;"
		L"}";

	try {
		auto_ptr<Piping> sw = ParsePiping(text); 
		if (dynamic_cast<const ConfigSwitch*>(&sw->tokenizer())) {
			const ConfigSwitch* s = dynamic_cast<const ConfigSwitch*>(&sw->tokenizer());
			for (int i = 0; i < s->cases().size(); i++) {
				const Case* c = s->cases()[i]; 
				printf("case "); 
				for (int j = 0; j < c->cases().size(); j++) {
					printf("%S", c->cases()[j]);
				}
				printf(": ...\n"); 
 //				wcout<<L":"<<s->def().tokenizer().id();
			}
			printf("default: ...\n");//<<s->def().tokenizer().id()<<"...;";
		}
	} catch (ParseException& e) {
		// OBS wcout<<L"ParseException: "<<e.describe(text)<<endl; 
		e.setContext(text);
		printf("ParseException: %S\n", e.wWhat()); 
	} catch (LexException& e) {
		// OBS wcout<<L"LexException: "<<e.describe(text)<<endl; 
		e.setContext(text);
		printf("LexException: %S\n", e.wWhat()); 
	}
}


void TestParsingErrors(Itk::TestMgr* mgr)
{
	Cpix::AnalyzerExp::Tokenizer tokenizer; 
	// eof
	const wchar_t* text;
	try {
		ParsePiping( text = L"foobar(zap, foo, 'bar', 'raf', do, " ); 
	} catch (ParseException& e) {
		printf("ParseException: %S\n", e.wWhat()); 
	}
	
	// Unfinished literal
	try {
		ParsePiping(text = L"foobar(zap, foo, 'bar', 'a, raboof)"); 
	} catch (LexException& e) { // syntax error
		printf("LexException: %S\n", e.wWhat()); 
	} catch (ParseException& e) { // syntax error
		printf("ParseException: %S\n", e.wWhat()); 
	} 

	// Unknown token
	try {
		ParsePiping(text = L"foobar(!zap, foo, 'bar', 'a', raboof)"); 
	} catch (LexException& e) { // syntax error
		printf("LexException: %S\n", e.wWhat()); 
	} 
	
	// Missing comma
	try {
		ParsePiping(text = L"foobar(zap, foo, 'bar', 'a' raboof)"); 
	} catch (ParseException& e) {
		printf("ParseException: %S\n", e.wWhat()); 
	} 

}


const char * CustomAnalyzerTestDocs[] = {
    STEM_TEST_CORPUS_PATH "\\en\\1.txt",
    STEM_TEST_CORPUS_PATH "\\en\\2.txt",
    STEM_TEST_CORPUS_PATH "\\en\\3.txt",
    STEM_TEST_CORPUS_PATH "\\en\\4.txt",
        
    STEM_TEST_CORPUS_PATH "\\fi\\1.txt",
    STEM_TEST_CORPUS_PATH "\\fi\\2.txt",
    LOC_TEST_CORPUS_PATH "\\th\\1.txt",
    LOC_TEST_CORPUS_PATH "\\th\\2.txt",
    
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

void TestCustomAnalyzer(Itk::TestMgr * testMgr, 
					    const char** files, 
					    const wchar_t* definition)
{
	using namespace lucene::analysis; 
	using namespace lucene::util; 
	using namespace Cpix; 
	using namespace std; 
	CustomAnalyzer analyzer(definition);
	
	printf("Analyzer \"%S\":\n", definition); 
	for (int i = 0; files[i]; i++) 
	{
		printf("File !%s tokenized:\n", (files[i]+1));
		FileReader file( files[i], DEFAULT_ENCODING ); 
		
		TokenStream* stream = analyzer.tokenStream( L"field", &file ); 
		PrintTokenStream( stream ); 
		stream->close(); 
		_CLDELETE( stream ); 
	}
	printf("\n");
}

void TestCustomAnalyzer(Itk::TestMgr * testMgr, const wchar_t* definition) {
	TestCustomAnalyzer(testMgr, CustomAnalyzerTestDocs, definition);
}

void TestCustomAnalyzers(Itk::TestMgr * testMgr)
{
	TestCustomAnalyzer(testMgr, L"stdtokens");
	TestCustomAnalyzer(testMgr, L"whitespace");
	TestCustomAnalyzer(testMgr, L"whitespace>lowercase");
	TestCustomAnalyzer(testMgr, L"whitespace>accent");
	TestCustomAnalyzer(testMgr, L"letter");
	TestCustomAnalyzer(testMgr, L"letter>lowercase");
	TestCustomAnalyzer(testMgr, L"keyword");
	TestCustomAnalyzer(testMgr, L"keyword>lowercase");
//	TestCustomAnalyzer(testMgr, L"stdtokens>lowercase>stem(en)"); // Does not work with NON-ASCII
	TestCustomAnalyzer(testMgr, L"letter>lowercase>stop(en)"); 
	TestCustomAnalyzer(testMgr, L"letter>lowercase>stop('i', 'oh', 'nyt', 'n�in')"); 
	TestCustomAnalyzer(testMgr, L"letter>length(2, 4)");
	TestCustomAnalyzer(testMgr, L"standard>prefixes(1)");
	TestCustomAnalyzer(testMgr, L"standard>prefixes(2)");
	TestCustomAnalyzer(testMgr, L"standard>prefixes(3)");
	TestCustomAnalyzer(testMgr, L"stdtokens>stdfilter>lowercase>thai>stop(en)");
	TestCustomAnalyzer(testMgr, L"cjk>stop(en)");
    TestCustomAnalyzer(testMgr, L"ngram(1)>lowercase>stop(en)");
    TestCustomAnalyzer(testMgr, L"ngram(2)>lowercase>stop(en)");
}

void TestTokenizationWithLocales(Itk::TestMgr * testMgr) {
	printf("locale=en\n"); 
	cpix_Result result; 
	cpix_SetLocale( &result, "en" ); 
	TestCustomAnalyzer(testMgr, L"natural");
	
	printf("locale=th\n"); 
	cpix_SetLocale( &result, "th" ); 
	TestCustomAnalyzer(testMgr, L"natural");

	printf("locale=ko\n");
	cpix_SetLocale( &result, "ko" ); 
	TestCustomAnalyzer(testMgr, L"natural");
	
	printf("locale=zh\n");
	cpix_SetLocale( &result, "zh" );
	TestCustomAnalyzer(testMgr, L"natural");
	
	printf("locale=jp\n");
	cpix_SetLocale( &result, "jp" ); 
	TestCustomAnalyzer(testMgr, L"natural");

	cpix_SetLocale( &result, cpix_LOCALE_AUTO ); 
}

template<typename T> 
void TestTokenizationWithLocale(Itk::TestMgr * testMgr) {
	cpix_Result result; 
	cpix_SetLocale( &result, T::LOCALE ); 
    TestCustomAnalyzer(testMgr, EnglishLocale::FILES, L"natural");
	TestCustomAnalyzer(testMgr, T::FILES, L"natural");
	cpix_SetLocale( &result, cpix_LOCALE_AUTO ); 
}


template<typename T>
void AddTokenizationWithLocaleTest(Itk::SuiteTester* suite) {
    suite->add(T::LOCALE,
               &TestTokenizationWithLocale<T>,
               T::LOCALE);
}

void TestTokenizationWithCurrentLocale(Itk::TestMgr * testMgr) {
	cpix_Result result; 
	cpix_SetLocale( &result, cpix_LOCALE_AUTO ); 
	TestCustomAnalyzer(testMgr, L"natural");
}

void TestAnalyzerWithField(Itk::TestMgr * testMgr, const wchar_t* definition, const wchar_t* field) 	
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
}

void TestLocaleSwitchAnalyzers(Itk::TestMgr * testMgr)
{
	const wchar_t* sw = L"\n"
		L"locale_switch {\n"
		L"    case 'en':       stdtokens>stdfilter>lowercase>stop(en);\n"
		L"    case 'th':       stdtokens>stdfilter>lowercase>thai>stop(en);\n"
		L"    case 'ca':       stdtokens>stdfilter>lowercase>accent;\n"
		L"    default:         stdtokens>stdfilter>lowercase;\n"
		L"}";
	cpix_Result result; 
	printf("locale=en:\n");
	cpix_SetLocale( &result, "en" ); 
	TestCustomAnalyzer(testMgr, sw);
	printf("\n");
	printf("locale=th:\n");
	cpix_SetLocale( &result, "th" ); 
	TestCustomAnalyzer(testMgr, sw);
	printf("\n");
	printf("locale=ca:\n");
	cpix_SetLocale( &result, "ca" ); 
	TestCustomAnalyzer(testMgr, sw);
	printf("\n");
	printf("default locale:\n");
	cpix_SetLocale( &result, "fail" ); 
	TestCustomAnalyzer(testMgr, sw);
	cpix_SetLocale( &result, cpix_LOCALE_AUTO ); 
}


Itk::TesterBase * CreateAnalysisWhiteBoxLocalizationTests() {
    using namespace Itk;
    
	SuiteTester
		* tests = new SuiteTester("loc");

	std::string locale;
	locale = "currentlocale_"; 
    
    Cpt::auto_array<char> name( Cpix::Spi::GetLanguageNames()[0].c_str() );
    locale += name.get();
    
	tests->add(locale.c_str(),
				  &TestTokenizationWithCurrentLocale,
				  locale.c_str());
	
	AddTokenizationWithLocaleTest<EnglishLocale>(tests);
	AddTokenizationWithLocaleTest<FrenchLocale>(tests);
	AddTokenizationWithLocaleTest<HebrewLocale>(tests);
	AddTokenizationWithLocaleTest<ThaiLocale>(tests);
	AddTokenizationWithLocaleTest<KoreanLocale>(tests);
	AddTokenizationWithLocaleTest<ChineseLocale>(tests);
	AddTokenizationWithLocaleTest<JapaneseLocale>(tests);
	    
	return tests;
}

Itk::TesterBase * CreateAnalysisWhiteBoxTests()
{
    using namespace Itk;

    SuiteTester
        * analysisTests = new SuiteTester("whitebox");
    
    analysisTests->add("analyzer",
					   &TestCustomAnalyzers,
					   "analyzer");
    analysisTests->add("switchAnalyzer",
					   &TestSwitchAnalyzers,
					   "switchAnalyzer");
    analysisTests->add("localeSwitchAnalyzer",
					   &TestLocaleSwitchAnalyzers,
					   "localeSwitchAnalyzer");
    analysisTests->add("tokenization",
    				   TestTokenization6,
    				   "tokenization");
  	analysisTests->add("parsing",
                      TestParsing,
                      "parsing");
    analysisTests->add("parsing2",
                      TestSwitch,
                      "parsing2");
    analysisTests->add("parsing3",
                      TestConfigSwitch,
                      "parsing3");
    analysisTests->add("parsingerrors",
                      TestParsingErrors,
                      "parsingerrors");

    analysisTests->add(CreateAnalysisWhiteBoxLocalizationTests());
    return analysisTests;
}



