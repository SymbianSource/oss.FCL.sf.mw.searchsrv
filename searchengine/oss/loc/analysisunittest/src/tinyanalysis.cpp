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

#include "tinyanalysis.h"
#include "tinyanalysis.inl"
#include "tinyunicode.h"

#include "itk.h"

#include <iostream>

#include "CLucene.h"

#include "wchar.h"

#include "analysisunittest.h"
#include "evaluationtool.h"

using namespace evaluationtool; 

template <class T>
void TestLetters(Itk::TestMgr* testMgr, T text) {
	using namespace analysis::tiny;
	
	CustomTokenizer<T> letters(iswalpha);
	RelaxedTokenizer<T> tokens(letters);
	
	Token<T> t; 
	while (t = tokens.consume(text)) {
        wchar_t buf[256];
        t.utf16(buf);
        wprintf(L"\"%S\" ", buf);
	}
	wprintf(L"\n");
}

int isnotspace(int c) {
    return !iswspace(c);
}

template <class T>
void TestNGram(Itk::TestMgr* testMgr, T text) {
	using namespace analysis::tiny;
	
	NGramTokenizer<T> ngram(2, isnotspace);
	RelaxedTokenizer<T> tokens(ngram);
	
	Token<T> t; 
	while (t = tokens.consume(text)) {
        wchar_t buf[256];
        t.utf16(buf);
        wprintf(L"\"%S\" ", buf);
	}
    wprintf(L"\n");
}

void TinyWcharTest(Itk::TestMgr* testMgr) {
	TestLetters(testMgr, L"foo bar foobar foo*bar foo_bar");
	TestNGram(testMgr, L"foo bar foobar foo*bar foo_bar");
}

void TinyReaderTest(Itk::TestMgr* testMgr) {
	{
		lucene::util::StringReader reader(L"foo bar foobar foo*bar foo_bar");
		analysis::tiny::cl::ReaderBuffer<8> buf(reader);
		TestLetters(testMgr, buf.begin());
	}
	{
		lucene::util::StringReader reader(L"foo bar foobar foo*bar foo_bar");
		analysis::tiny::cl::ReaderBuffer<8> buf(reader);
		TestNGram(testMgr, buf.begin());
	}
}

void TinyChinaTest(Itk::TestMgr* testMgr) {
    using namespace analysis::tiny;
    Corpus corpus(CHINESE_PRC_TEXTCORPUS); 
    typedef cl::ReaderBuffer<64> buffer;

    for (int i = 0; i < corpus.size(); i++) {
        lucene::util::StringReader reader(corpus[i]);
        buffer buf(reader);
        TestNGram( testMgr, Utf16Iterator<buffer::iterator>( buf.begin() ) );
    }
}

void TinyUtf16Test(Itk::TestMgr* testMgr) {
    using namespace analysis::tiny;
    Corpus corpus(CHINESE_PRC_TEXTCORPUS); 
    typedef cl::ReaderBuffer<512> buffer;
    typedef Utf16Iterator<buffer::iterator> u16iter;

    for (int i = 0; i < corpus.size(); i++) {
        {
            lucene::util::StringReader reader(corpus[i]);
            buffer buf(reader);
            {
                u16iter i( buf.begin() );
                for (; *i; ++i) {
                    int c = *i;
                    wcout<<(void*)c<<L" ";
                }
            }
        }
        wcout<<endl;
        {
            wchar_t c[512];
            {
                lucene::util::StringReader reader(corpus[i]);
                buffer buf(reader);
    
                buffer::iterator j = buf.begin();
                {
                    int i;
                    for (i = 0; *j; i++, ++j) {
                        c[i] = *j;
                    }
                    c[i] = '\0';
                }
            }
            lucene::util::StringReader reader(corpus[i]);
            buffer buf(reader);
            u16iter i( buf.begin() );
            wchar_t b[512];
            wcout<<flush;
            Utf16Writer<wchar_t*>(b)<<i<<L'\0';
            wprintf(L"%S\n", b);
            fflush(stdout);
            for (int k = 0; c[k] || b[k]; k++) {
                if (c[k] != b[k]) {
                    wcout<<"x";
                } else {
                    wcout<<".";
                }
            }
        }

        wcout<<endl;
    }
}

void TinyJamuTest(Itk::TestMgr* testMgr) {
    using namespace analysis::tiny;
    Corpus corpus(KOREAN_TEXTCORPUS);
    
    typedef cl::ReaderBuffer<512> buffer;
    typedef Utf16Iterator<buffer::iterator> u16iter;
    typedef JamuIterator<u16iter> iter;
    
    for (int line = 0; line < corpus.size(); line++) {
        lucene::util::StringReader reader(corpus[line]);
        buffer buf(reader);
        iter i(u16iter(buf.begin()));

        printf("%S\n", utf16str(i).c_str());
    }
}


void TinyHangulTest(Itk::TestMgr* testMgr) {
    using namespace analysis::tiny;
    Corpus corpus(KOREAN_TEXTCORPUS);
    
    typedef cl::ReaderBuffer<512> buffer;
    typedef Utf16Iterator<buffer::iterator> u16iter;
    typedef HangulIterator<u16iter> iter;
    
    for (int line = 0; line < corpus.size(); line++) {
        lucene::util::StringReader reader(corpus[line]);
        buffer buf(reader);
        iter i(u16iter(buf.begin()));

        printf("%S\n", utf16str(i).c_str());
    }
}
Itk::TesterBase * CreateTinyAnalysisUnitTest() 
{
	using namespace Itk;
	
	SuiteTester
		* testSuite = 
			new SuiteTester( "tiny" );
	
	testSuite->add( "wchar", TinyWcharTest, "wchar" );
	testSuite->add( "reader", TinyReaderTest, "reader" );
    testSuite->add( "cn", TinyChinaTest, "cn" );
    testSuite->add( "utf16", TinyUtf16Test, "utf16" );
    testSuite->add( "jamu", TinyJamuTest, "jamu" );
    testSuite->add( "hangul", TinyHangulTest, "hangul" );
    
	return testSuite;
}
