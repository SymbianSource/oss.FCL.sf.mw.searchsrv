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
#include "cpixparsetools.h"
#include "itk.h"

#include <iostream>
#include <memory>

using namespace Cpt::Lex; 
using namespace Cpt::Parser; 
using namespace std; 

const wchar_t* TOKEN_LEFT_BRACKET = L"left bracket";
const wchar_t* TOKEN_RIGHT_BRACKET = L"right bracket"; 
const wchar_t* TOKEN_COMMA = L"comma";
const wchar_t* TOKEN_PIPE = L"pipe";
const wchar_t* TOKEN_SWITCH = L"switch";
const wchar_t* TOKEN_CASE = L"case";
const wchar_t* TOKEN_DEFAULT = L"default";
const wchar_t* TOKEN_LEFT_BRACE = L"left brace";
const wchar_t* TOKEN_RIGHT_BRACE = L"right brace";
const wchar_t* TOKEN_COLON = L"colon";
const wchar_t* TOKEN_TERMINATOR = L"terminator";

void PrintToken(Cpt::Lex::Token token) {
	wcout<<token.type()<<L"('"<<token.text()<<L"')";  
}

void TestTokenization(Itk::TestMgr  * testMgr,
                      const wchar_t * inputStr)
{
	WhitespaceTokenizer ws; 
	LineCommentTokenizer line; 
	SectionCommentTokenizer section; 
	IdTokenizer ids; 
        IntLitTokenizer ints;
        RealLitTokenizer reals;
	LitTokenizer lits('\''); 
	SymbolTokenizer lb(TOKEN_LEFT_BRACKET, L"("); 
	SymbolTokenizer rb(TOKEN_RIGHT_BRACKET, L")"); 
	SymbolTokenizer cm(TOKEN_COMMA, L","); 
	SymbolTokenizer pp(TOKEN_PIPE, L">");

        // NOTE: ints and reals are before lits, so even if lits
        // itself can recognize strings, ints and reals, the ints and
        // reals are taking precedence - just for the test cases now
        // (to check if those types are recognized correctly). So
        // basically, in test cases, lit will mean string literals,
        // and int-lit, real-lit will mean integer and real literals,
        // respectively.
	Tokenizer* tokenizers[] = {
		&ws, &line, &section, &lb, &rb, &cm, &pp, &ids, &ints, &reals, &lits, 0
	};
	MultiTokenizer tokenizer(tokenizers);
	
	Tokens 
            source(tokenizer, 
                   inputStr);
	StdFilter tokens(source); 
	
	while (tokens) PrintToken(tokens++); 
	cout<<endl;
}


void TestTokenization1(Itk::TestMgr * testMgr)
{
    TestTokenization(testMgr,
                     L"stdtokens>lowercase>stopwords('a', 'an','the')>stem('en')");
}

void TestTokenization2(Itk::TestMgr * testMgr)
{
    TestTokenization(testMgr,
                     L"'foo' 0 1 -2 'bar' +234 -34 // side note");
}


void TestTokenization3(Itk::TestMgr * testMgr)
{
    TestTokenization(testMgr,
                     L"'hallo' 0.0 .0 .5 -1.0 -.05 45 'bar' +.123 +3.1415");
}


void TestTokenization4(Itk::TestMgr * testMgr)
{
    TestTokenization(testMgr,
                     L"'\\' ''\\\\' '\\a' '\\\n' // comment\n /*foobar*/");
}


void TestTokenization5(Itk::TestMgr * testMgr)
{
    WhitespaceTokenizer 
        ws; 
    IdTokenizer 
        ids; 
    SymbolTokenizer 
        for_(L"for", L"for"); 
    SymbolTokenizer 
        if_(L"if", L"if"); 
    Tokenizer* tokenizers[] = {
        &ws, &for_, &if_, &ids, 0
    };

    MultiTokenizer 
        tokenizer(tokenizers);

    Tokens 
        source(tokenizer, 
               L"fo for fore forth ofor oforo i if ifdom ifer fif fifi forfi fifor"); // test escape in literals
    StdFilter 
        tokens(source); 

    while (tokens) PrintToken(tokens++); 
    cout<<endl;
}

void TestTokenization6(Itk::TestMgr * testMgr)
{
    WhitespaceTokenizer 
        ws; 
    LineCommentTokenizer 
        line; 
    SectionCommentTokenizer 
        section; 
    IdTokenizer 
        ids; 
    IntLitTokenizer 
        intLit; 
    RealLitTokenizer 
        realLit; 
    SymbolTokenizer 
        div(L"slash", L"/"); 
    SymbolTokenizer 
        mul(L"star", L"*");
    SymbolTokenizer 
        plus(L"plus", L"+");
    SymbolTokenizer 
        minus(L"minus", L"-");
    SymbolTokenizer 
        equal(L"equals", L"=");
    
    Tokenizer* tokenizers[] = {
        &ws, &line, &section, &ids, &intLit, &realLit, &div, &mul, &plus, &minus, &equal, 0
    };

    MultiTokenizer 
        tokenizer(tokenizers);
    
    const wchar_t* text = 
    	L"4 + 6 = 2 * 5\n"
        L"6 / 2 = 1*3 // true\n"
		L"3 / x /*important thingie*/ = 2 * y\n"
		L"6 / x * / * / /* non sense / * / */ // zap"
		L"//\n"
		L"//"; 

    {
		cout<<"With whitespaces & comments visible"<<endl;
		Tokens 
			tokens(tokenizer, text);
	
		while (tokens) PrintToken(tokens++); 
		cout<<endl;
    }

    {
		cout<<"With whitespaces & comments filtered"<<endl;
		Tokens 
			source(tokenizer, text);
		
		StdFilter tokens(source); 
	
		while (tokens) PrintToken(tokens++); 
		cout<<endl;
    }

}

void TestTokenizationErrors(Itk::TestMgr* mgr) 
{
	WhitespaceTokenizer ws; 
	IdTokenizer ids; 
	LitTokenizer lits('\''); 
	SymbolTokenizer lb(TOKEN_LEFT_BRACKET, L"("); 
	SymbolTokenizer rb(TOKEN_RIGHT_BRACKET, L")"); 
	SymbolTokenizer cm(TOKEN_COMMA, L","); 
	SymbolTokenizer pp(TOKEN_PIPE, L">");
	Tokenizer* tokenizers[] = {
		&ws, &lb, &rb, &cm, &pp, &ids, &lits, 0
	};
	MultiTokenizer tokenizer(tokenizers);
	const wchar_t* text;
	{
		Tokens tokens(tokenizer, text = L"stdtokens>lowercase>stopwords('a', 'an','the)>stem('en')");
		try {
			while (tokens) PrintToken(tokens++); 
		} catch (LexException& exc) {
			exc.setContext(text);
			wcout<<endl<<L"LexException: "<<exc.wWhat()<<endl; 
		} catch (exception& exc) {
			cout<<endl<<"Exception: "<<exc.what()<<endl; 
		}
	}
	{
		Tokens tokens(tokenizer, text = L"fas-324we?`213ff3*21(+");
		try {
			while (tokens) PrintToken(tokens++); 
		} catch (LexException& exc) {
			exc.setContext(text);
			wcout<<endl<<L"LexException: "<<exc.wWhat()<<endl; 
		} catch (exception& exc) {
			cout<<endl<<"Exception: "<<exc.what()<<endl; 
		}
	}
}

void TestWhitespaceSplitter(Itk::TestMgr* mgr) 
{
	{
		WhitespaceSplitter tokens(L"foobar foo bar foo\tbar _*4 4bar foo*bar foo\nbar foo\rbar foo\0bar");
		while (tokens) printf(" \"%S\"", tokens++.text().c_str());
		printf("\n");
	}
	
	{
		WhitespaceSplitter tokens(L"foobar");
		while (tokens) printf(" \"%S\"", tokens++.text().c_str());
		printf("\n");
	}

	{
		WhitespaceSplitter tokens(L"   foobar  \r\n");
		while (tokens) printf(" \"%S\"", tokens++.text().c_str());
		printf("\n");
	}

	{
		WhitespaceSplitter tokens(L"   ");
		while (tokens) printf(" \"%S\"", tokens++.text().c_str());
		printf("\n");
	}

	{
		WhitespaceSplitter tokens(L"");
		while (tokens) printf(" \"%S\"", tokens++.text().c_str());
		printf("\n");
	}

}

Itk::TesterBase * CreateParsingTests()
{
    using namespace Itk;

    SuiteTester
        * parsingTests = new SuiteTester("parsing");
   
    parsingTests->add("tokenization1",
                      TestTokenization1,
                      "tokenization1");

    parsingTests->add("tokenization2",
                      TestTokenization2,
                      "tokenization2");

    parsingTests->add("tokenization3",
                      TestTokenization3,
                      "tokenization3");

    parsingTests->add("tokenization4",
                      TestTokenization4,
                      "tokenization4");

    parsingTests->add("tokenization5",
                      TestTokenization5,
                      "tokenization5");
    
    parsingTests->add("tokenization6",
                      TestTokenization6,
                      "tokenization6");

    parsingTests->add("syntaxerrors",
                      TestTokenizationErrors,
                      "syntaxerrors");

    parsingTests->add("whitespace",
					  TestWhitespaceSplitter,
                      "whitespace");

    return parsingTests;
}


