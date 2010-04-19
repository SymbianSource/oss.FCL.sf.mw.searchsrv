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

enum TokenType {
	TOKEN_LEFT_BRACKET = Cpt::Lex::TOKEN_LAST_RESERVED,  // 8
	TOKEN_RIGHT_BRACKET, 
	TOKEN_COMMA, // 10
	TOKEN_PIPE,
	TOKEN_SWITCH,
	TOKEN_CASE,
	TOKEN_DEFAULT,
	TOKEN_LEFT_BRACE, // 15
	TOKEN_RIGHT_BRACE,
	TOKEN_COLON,
	TOKEN_TERMINATOR
};

void PrintToken(Cpt::Lex::Token token) {
	switch (token.type()) {
		case TOKEN_WS: wcout<<L"space"; break; 
		case TOKEN_ID: wcout<<"id"; break;
		case TOKEN_LIT: wcout<<"lit"; break;
		case TOKEN_STRLIT: wcout<<"str-lit"; break;
		case TOKEN_REALLIT: wcout<<"real-lit"; break;
		case TOKEN_INTLIT: wcout<<"int-lit"; break;
		case TOKEN_LEFT_BRACKET: wcout<<"lbr"; break;
		case TOKEN_RIGHT_BRACKET: wcout<<"rbr"; break;
		case TOKEN_COMMA: wcout<<"comma"; break;
		case TOKEN_PIPE: wcout<<"pipe"; break;
		case TOKEN_SWITCH : wcout<<"sw"; break;
		case TOKEN_CASE : wcout<<"case"; break;
		case TOKEN_DEFAULT : wcout<<"default"; break;
		case TOKEN_LEFT_BRACE : wcout<<"lbc"; break;
		case TOKEN_RIGHT_BRACE : wcout<<"rbc"; break;
		case TOKEN_COLON : wcout<<"cl"; break;
		case TOKEN_TERMINATOR : wcout<<"tr"; break;

		default: wcout<<"unknown"; break;
	}
	wcout<<L"('"<<token.text()<<L"')";  
}

void TestTokenization(Itk::TestMgr  * ,
                      const wchar_t * inputStr)
{
	WhitespaceTokenizer ws; 
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
		&ws, &lb, &rb, &cm, &pp, &ids, &ints, &reals, &lits, 0
	};
	MultiTokenizer tokenizer(tokenizers);
	
	Tokens 
            source(tokenizer, 
                   inputStr);
	WhiteSpaceFilter tokens(source); 
	
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
                     L"'foo' 0 1 -2 'bar' +234 -34");
}


void TestTokenization3(Itk::TestMgr * testMgr)
{
    TestTokenization(testMgr,
                     L"'hallo' 0.0 .0 .5 -1.0 -.05 45 'bar' +.123 +3.1415");
}


void TestTokenization4(Itk::TestMgr * testMgr)
{
    TestTokenization(testMgr,
                     L"'\\' ''\\\\' '\\a' '\\\n'");
}


void TestTokenization5(Itk::TestMgr * )
{
    WhitespaceTokenizer 
        ws; 
    IdTokenizer 
        ids; 
    SymbolTokenizer 
        for_(0xf00, L"for"); 
    SymbolTokenizer 
        if_(0xbeef, L"if"); 
    Tokenizer* tokenizers[] = {
        &ws, &for_, &if_, &ids, 0
    };

    MultiTokenizer 
        tokenizer(tokenizers);

    Tokens 
        source(tokenizer, 
               L"fo for fore forth ofor oforo i if ifdom ifer fif fifi forfi fifor"); // test escape in literals
    WhiteSpaceFilter 
        tokens(source); 

    while (tokens) PrintToken(tokens++); 
    cout<<endl;
}

void TestTokenizationErrors(Itk::TestMgr* ) 
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
                    /* OBS
			wcout<<endl<<L"LexException: "<<exc.describe(text)<<endl; 
                    */
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
                    /* OBS
			wcout<<endl<<L"LexException: "<<exc.describe(text)<<endl; 
                    */
                    exc.setContext(text);
                    wcout<<endl<<L"LexException: "<<exc.wWhat()<<endl; 
		} catch (exception& exc) {
			cout<<endl<<"Exception: "<<exc.what()<<endl; 
		}
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

    parsingTests->add("syntaxerrors",
                      TestTokenizationErrors,
                      "syntaxerrors");
	    
    return parsingTests;
}


