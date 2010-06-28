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


#include "analyzerexp.h"

#include "indevicecfg.h" 

namespace Cpix {

	namespace AnalyzerExp {
	
		const wchar_t TOKEN_LEFT_BRACKET[] = L"(";
		const wchar_t TOKEN_RIGHT_BRACKET[] = L")"; 
		const wchar_t TOKEN_COMMA[] = L"comma"; 
		const wchar_t TOKEN_PIPE[] = L">";
		const wchar_t TOKEN_SWITCH[] = L"switch";
		const wchar_t TOKEN_LOCALE_SWITCH[] = L"locale_switch";
		const wchar_t TOKEN_CONFIG_SWITCH[] = L"config_switch";
		const wchar_t TOKEN_CASE[] = L"case";
		const wchar_t TOKEN_DEFAULT[] = L"default";
		const wchar_t TOKEN_LEFT_BRACE[] = L"{";
		const wchar_t TOKEN_RIGHT_BRACE[] = L"}";
		const wchar_t TOKEN_COLON[] = L";";
		const wchar_t TOKEN_TERMINATOR[] = L";"; 

	
		std::auto_ptr<Piping> ParsePiping(Cpt::Parser::Lexer& lexer);  

	
		Tokenizer::Tokenizer()
		:	ws_(),
		 	lcomment_(), 
		 	scomment_(), 
			ids_(), 
			strlits_('\''), 
			intlits_(), 
			reallits_(), 
			lb_(TOKEN_LEFT_BRACKET, L"("),
			rb_(TOKEN_RIGHT_BRACKET, L")"),
			cm_(TOKEN_COMMA, L","),
			pp_(TOKEN_PIPE, CPIX_PIPE),
			sw_(TOKEN_SWITCH, CPIX_SWITCH),
			lsw_(TOKEN_LOCALE_SWITCH, CPIX_LOCALE_SWITCH),  
			csw_(TOKEN_CONFIG_SWITCH, CPIX_CONFIG_SWITCH),  
			cs_(TOKEN_CASE, CPIX_CASE),
			df_(TOKEN_DEFAULT, CPIX_DEFAULT),
			lbc_(TOKEN_LEFT_BRACE, L"{"),
			rbc_(TOKEN_RIGHT_BRACE, L"}"),
			cl_(TOKEN_COLON, L":"),
			tr_(TOKEN_TERMINATOR, L";")
		{
			int i = 0; 
			tokenizers_ = new Cpt::Lex::Tokenizer*[21];
			tokenizers_[i++] = &ws_;
			tokenizers_[i++] = &lcomment_;
			tokenizers_[i++] = &scomment_;
			tokenizers_[i++] = &lb_;
			tokenizers_[i++] = &rb_;
			tokenizers_[i++] = &cm_;
			tokenizers_[i++] = &pp_;
			tokenizers_[i++] = &sw_;
			tokenizers_[i++] = &lsw_;
			tokenizers_[i++] = &csw_;
			tokenizers_[i++] = &cs_;
			tokenizers_[i++] = &df_;
			tokenizers_[i++] = &lbc_;
			tokenizers_[i++] = &rbc_;
			tokenizers_[i++] = &cl_;
			tokenizers_[i++] = &tr_;
			tokenizers_[i++] = &ids_;
			tokenizers_[i++] = &strlits_;
			tokenizers_[i++] = &intlits_;
			tokenizers_[i++] = &reallits_;
			tokenizers_[i++] = 0; 
			tokenizer_.reset( new Cpt::Lex::MultiTokenizer(tokenizers_) );
		}
		
		Tokenizer::~Tokenizer() {
			delete[] tokenizers_;
		}		
	
		void Tokenizer::reset()
		{
			tokenizer_->reset(); 
		}
		
		Cpt::Lex::Token Tokenizer::get()
		{
			return tokenizer_->get(); 
		}
		
		Cpt::Lex::TokenizerState Tokenizer::consume(const wchar_t* cursor)
		{
			return tokenizer_->consume(cursor); 
		}

	
		Exp::~Exp() {}
	
		StringLit::StringLit(const std::wstring& text) : text_(text) {}
		const std::wstring& StringLit::text() const { return text_; }
	
		IntegerLit::IntegerLit(long value) : value_(value) {}
		long IntegerLit::value() const { return value_; }
		
		RealLit::RealLit(double value) : value_(value) {}
		double RealLit::value() const { return value_; }
	
		Identifier::Identifier(const std::wstring& id) : id_(id) {}
		const std::wstring& Identifier::id() const { return id_; }
		
		Parameters::Parameters(Cpt::auto_vector<Exp>& params) 
		: params_(params) {}
		
		Parameters::~Parameters() {}; 
	
		const std::vector<Exp*>& Parameters::params() const {
			return params_;
		}
	
		Invokation::Invokation(Identifier id, std::auto_ptr<Parameters> parameters)
		: id_( id.id() ), 
		  parameters_( parameters)
		{ }
	
		Invokation::~Invokation() {
		}
	
		const std::wstring& Invokation::id() const {
			return id_;
		}
	
		const std::vector<Exp*>& Invokation::params() const {
			return parameters_->params();
		}
		
		Piping::Piping(std::auto_ptr<Exp> tokenizer, Cpt::auto_vector<Invokation>& filters) 
		: tokenizer_(tokenizer),
		  filters_(filters) 
			{}
	
		Piping::~Piping() {
		}
		const Exp& Piping::tokenizer() const {
			return *tokenizer_;
		}
		const std::vector<Invokation*>& Piping::filters() const {
			return filters_;
		}
	
		Case::Case(const std::vector<std::wstring>& cases, std::auto_ptr<Piping> piping) 
		:	cases_(cases), piping_(piping) {}
		Case::~Case() {}; 
		const std::vector<std::wstring>& Case::cases() const   { return cases_; }
		const Piping& Case::piping() const 					   { return *piping_; }
		
		Switch::Switch(Cpt::auto_vector<Case>& cases, std::auto_ptr<Piping> def) 
		: cases_(cases), def_(def) {
		}
		
		Switch::~Switch() {
		}
		
		const std::vector<Case*>& Switch::cases() const { return cases_; }
		const Piping& Switch::def() const { return *def_; }

		LocaleSwitch::LocaleSwitch(Cpt::auto_vector<Case>& cases, std::auto_ptr<Piping> def) 
		: cases_(cases), def_(def) {
		}
		
		LocaleSwitch::~LocaleSwitch() {
		}
		
		const std::vector<Case*>& LocaleSwitch::cases() const { return cases_; }
		const Piping& LocaleSwitch::def() const { return *def_; }

		ConfigSwitch::ConfigSwitch(Cpt::auto_vector<Case>& cases, std::auto_ptr<Piping> def) 
		: cases_(cases), def_(def) {
		}
		
		ConfigSwitch::~ConfigSwitch() {
		}
		
		const std::vector<Case*>& ConfigSwitch::cases() const { return cases_; }
		const Piping& ConfigSwitch::def() const { return *def_; }

		//
		// Parsing methods   
		// ---------------
		//
		
		// 
		// How the parsing is implemented? 
		// --
		//  
		// Parsing uses the Lexer - object from Cpt::Parser package.
		// The basic way how lexer operates is that the lexer  
		// converts a source stream of characters lazily into 
		// stream of tokens. If the lexer object fails at tokenizing 
		// the character stream because syntax error, LexException 
		// is thrown.
		// 
		// The produced stream of tokens can be iterated
		// with 'eat' methods. Typically one moves forward in the 
		// token stream by 'eating' specific tokens, e.g. by 
		// command lexer.eat(TOKEN_LEFT_BRACKET). If the 'eaten'
		// token is not of the specified type, parse exception is
		// raised. In cases, where token can be of a number of types,
		// use of lexer.peek() is adviced.
		//
		
		// 
		// Example code of using lexer for parsing syntax '(ID[, STRING])': 
		//
		//     lexer.eat(TOKEN_LEFT_BRACKET);
		//     std::string id = lexer.parseId();
		//     if (lexer.peek().type() == TOKEN_COMMA) {
		//       lexer.eat(TOKEN_COMMA);
		//       std::string str = lexer.parseString();
		//     )
		//     lexer.eat(TOKEN_RIGHT_BRACKET);
		// 
		
		// Atomic expressions, e.g. "'foo'", "4", "4.5", "id" 
		//
		
		std::auto_ptr<StringLit> ParseString(Cpt::Parser::Lexer& lexer)
		{
			return std::auto_ptr<StringLit>(new StringLit(lexer.eatString())); 
		}
	
		std::auto_ptr<IntegerLit> ParseInteger(Cpt::Parser::Lexer& lexer) 
		{
			return std::auto_ptr<IntegerLit>(new IntegerLit(lexer.eatInteger())); 
		}
	
		std::auto_ptr<RealLit> ParseReal(Cpt::Parser::Lexer& lexer)  
		{
			return std::auto_ptr<RealLit>(new RealLit(lexer.eatReal())); 
		}
	
		std::auto_ptr<Identifier> ParseIdentifier(Cpt::Parser::Lexer& lexer)  
		{
			return std::auto_ptr<Identifier>(new Identifier(lexer.eatId())); 
		}
	
		std::auto_ptr<Exp> ParseParameter(Cpt::Parser::Lexer& lexer)  
		{
			Cpt::Lex::token_type_t type = lexer.peek().type(); 
			if (type == Cpt::Lex::TOKEN_ID) 	return std::auto_ptr<Exp>( ParseIdentifier(lexer).release() );
			if (type == Cpt::Lex::TOKEN_STRLIT) return std::auto_ptr<Exp>( ParseString(lexer).release() );
			if (type == Cpt::Lex::TOKEN_INTLIT)	return std::auto_ptr<Exp>( ParseInteger(lexer).release() );
			if (type == Cpt::Lex::TOKEN_REALLIT)return std::auto_ptr<Exp>( ParseReal(lexer).release() );
			throw Cpt::Parser::ParseException(L"Expected literal. ", lexer.peek()); 
		}
	
		std::auto_ptr<Parameters> ParseParameters(Cpt::Parser::Lexer& lexer) 
		{
			Cpt::auto_vector<Exp> exps; 
			lexer.eat(TOKEN_LEFT_BRACKET);
			while (lexer.peek().type() != TOKEN_RIGHT_BRACKET) {
				if (!exps.empty()) lexer.eat(TOKEN_COMMA); 
				exps.donate_back( ParseParameter(lexer) );
			}
			lexer.eat(TOKEN_RIGHT_BRACKET);
			return std::auto_ptr<Parameters>(new Parameters(exps)); 
		}
	
		std::auto_ptr<Parameters> ParseRelaxedParameters(Cpt::Parser::Lexer& lexer) 
		{
			if (!lexer || lexer.peek().type() != TOKEN_LEFT_BRACKET) {
				Cpt::auto_vector<Exp> exps; 
				return std::auto_ptr<Parameters>(new Parameters(exps)); 
			}
			return ParseParameters(lexer); 
		}
	
		std::auto_ptr<Invokation> ParseInvokation(Cpt::Parser::Lexer& lexer)  
		{
			std::wstring id = lexer.eatId();
			return std::auto_ptr<Invokation>(new Invokation(id, ParseParameters(lexer)));
		}
	
		std::auto_ptr<Invokation> ParseRelaxedInvokation(Cpt::Parser::Lexer& lexer)  
		{
			std::wstring id = lexer.eatId();
			return std::auto_ptr<Invokation>(new Invokation(id, ParseRelaxedParameters(lexer)));
		}
	
		std::auto_ptr<Case> ParseCase(Cpt::Parser::Lexer& lexer)  
		{
			lexer.eat(TOKEN_CASE); 
			std::vector<std::wstring> fields;
			fields.push_back(lexer.eatString()); // at leat one item expected
			while (lexer && lexer.peek().type() != TOKEN_COLON) {
				lexer.eat(TOKEN_COMMA); 
				fields.push_back(lexer.eatString()); 
			}
			lexer.eat(TOKEN_COLON);
			std::auto_ptr<Piping> def = ParsePiping(lexer); 
			lexer.eat(TOKEN_TERMINATOR); 
			return std::auto_ptr<Case>(new Case(fields, def)); 
		}
		
		std::auto_ptr<Piping> ParseDefault(Cpt::Parser::Lexer& lexer)  
		{
			lexer.eat(TOKEN_DEFAULT); 
			lexer.eat(TOKEN_COLON); 
			std::auto_ptr<Piping> def = ParsePiping(lexer); 
			lexer.eat(TOKEN_TERMINATOR); 
			return def;
		}
				
		std::auto_ptr<Switch> ParseSwitch(Cpt::Parser::Lexer& lexer)  
		{
			lexer.eat(TOKEN_SWITCH); 
			lexer.eat(TOKEN_LEFT_BRACE);
			Cpt::auto_vector<Case> cases;
			while (lexer && lexer.peek().type() == TOKEN_CASE) {
				cases.donate_back(ParseCase(lexer));
			}
			std::auto_ptr<Piping> def = ParseDefault(lexer); 
			lexer.eat(TOKEN_RIGHT_BRACE); 
	
			return std::auto_ptr<Switch>(new Switch(cases, def)); 
		}

		std::auto_ptr<LocaleSwitch> ParseLocaleSwitch(Cpt::Parser::Lexer& lexer)  
		{
			lexer.eat(TOKEN_LOCALE_SWITCH); 
			lexer.eat(TOKEN_LEFT_BRACE);
			Cpt::auto_vector<Case> cases;
			while (lexer && lexer.peek().type() == TOKEN_CASE) {
				cases.donate_back(ParseCase(lexer));
			}
			std::auto_ptr<Piping> def = ParseDefault(lexer); 
			lexer.eat(TOKEN_RIGHT_BRACE); 
	
			return std::auto_ptr<LocaleSwitch>(new LocaleSwitch(cases, def)); 
		}
		
		std::auto_ptr<ConfigSwitch> ParseConfigSwitch(Cpt::Parser::Lexer& lexer)  
		{
			lexer.eat(TOKEN_CONFIG_SWITCH); 
			lexer.eat(TOKEN_LEFT_BRACE);
			Cpt::auto_vector<Case> cases;
			while (lexer && lexer.peek().type() == TOKEN_CASE) {
				cases.donate_back(ParseCase(lexer));
			}
			std::auto_ptr<Piping> def = ParseDefault(lexer); 
			lexer.eat(TOKEN_RIGHT_BRACE); 
	
			return std::auto_ptr<ConfigSwitch>(new ConfigSwitch(cases, def)); 
		}


		// Tokenizer can be either in Invocation form or switch-case 
		// structure
		//
		
		std::auto_ptr<Exp> ParseTokenizer(Cpt::Parser::Lexer& lexer)  {
			if (lexer.peek().type() == TOKEN_SWITCH) {
				return std::auto_ptr<Exp>(ParseSwitch(lexer).release()); 
			} else if (lexer.peek().type() == TOKEN_LOCALE_SWITCH) {
				return std::auto_ptr<Exp>(ParseLocaleSwitch(lexer).release()); 
			} else if (lexer.peek().type() == TOKEN_CONFIG_SWITCH) {
				return std::auto_ptr<Exp>(ParseConfigSwitch(lexer).release()); 
			} else {
				return std::auto_ptr<Exp>(ParseRelaxedInvokation(lexer).release());
			}
		}
	
		std::auto_ptr<Piping> ParsePiping(Cpt::Parser::Lexer& lexer)  
		{
			std::auto_ptr<Exp> tokenizer = ParseTokenizer(lexer); 
			Cpt::auto_vector<Invokation> filters; 
			
			while (lexer && lexer.peek().type() == TOKEN_PIPE) {
				lexer.eat(TOKEN_PIPE); 
				filters.donate_back(ParseRelaxedInvokation(lexer));
			}
			return std::auto_ptr<Piping>(new Piping(tokenizer, filters)); 
		}
		
		std::auto_ptr<Piping> ParsePiping(const wchar_t* definition) {
			using namespace Cpt::Lex;
			using namespace Cpt::Parser;
					
			try {
				// 1. Setup an tokenizer
				Cpix::AnalyzerExp::Tokenizer 
					tokenizer; 
				StdLexer 
					lexer(tokenizer, definition);
				
				// 2. Parse 
				std::auto_ptr<Piping> 
					def = ParsePiping(lexer); 
				lexer.eatEof();
				
				return def; 
			} catch (Cpt::ITxtCtxtExc & exc) {
				// provide addition info for thrown exception
				exc.setContext(definition);
	
				// throw it fwd
				throw;
			}
		}

	}

}	
