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
/*
 * cpixparsetools.cpp
 *
 *  Created on: Apr 14, 2009
 *      Author: admin
 */

#include "cpixparsetools.h"
#include "cpixtools.h"

#include <iostream>
#include <sstream>
#include <stdlib.h>
#include "wctype.h"

namespace {

	std::wstring describeException(std::wstring what, const wchar_t* context, const wchar_t* where, const wchar_t* where2) {
		std::wstring line;
		int l = 0;
		bool found = false; 
		 
		for (; ; context++) {
			if (context == where) {
				line += L"*here*";
				found = true; 
				if (!where2) break; 
			}
			if (context == where2) {
				line += L"*here*";
				break; 
			}
			if (!*context) {
				line += L"*here*";
				break;
			} else if (*context == '\n' && !found) {
				l++; 
				line = L"";  
			} else {
				line += *context;
			}
		}
		for (; *context && *context != '\n' && *context != '\r'; context++) {
			line += *context; 
		}
		 
		std::wostringstream tmp; 
		tmp<<what; 
		tmp<<L" at";
		if ( l ) {
			tmp<<L" line "<<(l+1);
		}
		tmp<<L": \n\"";
		tmp<<line;
		tmp<<L"\"";
		return tmp.str();        	
	}

}

namespace Cpt {


    namespace Lex {
    
		token_type_t TOKEN_UNKNOWN = L"unknown";
		token_type_t TOKEN_EOF = L"eof";
		token_type_t TOKEN_WS = L"whitespace"; 
		token_type_t TOKEN_COMMENT = L"comment";  
		token_type_t TOKEN_ID = L"identifier";	
		token_type_t TOKEN_STRLIT = L"string";
		token_type_t TOKEN_INTLIT = L"integer";
		token_type_t TOKEN_REALLIT = L"real number";
		token_type_t TOKEN_LIT = L"literal";
	
        const wchar_t ESCAPE_SYMBOL = '\\';
	
        Tokenizer::~Tokenizer() {}
	
        LexException::LexException(const wchar_t* wWhat, 
                                   const wchar_t* where) 
            : wWhat_(wWhat), 
              where_(where) {
            ;
        }

        LexException::~LexException() 
        {
            ;
        }

        const wchar_t* LexException::where() const {
            return where_;
        }

        const wchar_t* LexException::wWhat() const throw() {
            return wWhat_.c_str();
        }
        
        void LexException::setContext(const wchar_t * context) {
			wWhat_ = describeException(wWhat_, context, where_, NULL); 
        }

        Token::Token(const wchar_t* type, const wchar_t* begin, const wchar_t* end) 
            : type_(type), begin_(begin), end_(end) {
        }

        Token::Token() 
            : type_(0), begin_(0), end_(0) {
        }
		
        token_type_t Token::type() const { return type_; }; 
        const wchar_t* Token::begin() const { return begin_; };
        const wchar_t* Token::end() const { return end_; };
        int Token::length() const { return end_ - begin_; };
        std::wstring Token::text() const {
            std::wstring ret;
            for (const wchar_t* i = begin_; i != end_; i++) {
                ret += *i; 
            }
            return ret; 
        }

        StrLitTokenizer::StrLitTokenizer(wchar_t citate) 
            : 	citate_(citate)
        {	
            reset(); 
        }
						
        void StrLitTokenizer::reset() 
        { 
            escape_ = false, 
                opened_ = false, 
                begin_ = 0;
            end_ = 0; 
        }
        Token StrLitTokenizer::get() 
        { 
            return Token( TOKEN_STRLIT, begin_, end_ ); 
        }
        TokenizerState StrLitTokenizer::consume(const wchar_t* cursor) 
        {
            if (!*cursor) return TOKENIZER_FAILED; // fail always on EOF
            if (!opened_) 
                {
                    if (*cursor == citate_) 
                        {
                            opened_ = true;
                            begin_ = cursor; 
                        } else {
                        return TOKENIZER_FAILED; 
                    }
                } else if (escape_)  {
                escape_ = false;
            } else {
                if (*cursor == citate_) {
                    end_ = cursor+1; 
                    return TOKENIZER_FINISHED;
                } else if (*cursor == '\\') {
                    escape_ = true;
                }
            } 
            return TOKENIZER_HUNGRY; 
        }
		
        IntLitTokenizer::IntLitTokenizer() {
            reset();
        }

        void IntLitTokenizer::reset() {
            begin_ = NULL;
            end_ = NULL;
            beginning_ = true;
        }

        Token IntLitTokenizer::get() {
            return Token(TOKEN_INTLIT, begin_, end_);
        }

        TokenizerState IntLitTokenizer::consume(const wchar_t * cursor) {
            TokenizerState
                rv = TOKENIZER_HUNGRY;

            if (beginning_)
                {
                    if (*cursor != L'+'
                        && *cursor != L'-'
                        && !isdigit(*cursor))
                        {
                            rv = TOKENIZER_FAILED;
                        }
                    beginning_ = false;
                    begin_ = cursor;
                }
            else if (!isdigit(*cursor))
                {
                    rv = TOKENIZER_FINISHED;
                    end_ = cursor;
                }

            return rv;
        }

        RealLitTokenizer::RealLitTokenizer() {
            reset();
        }

        void RealLitTokenizer::reset() {
            begin_ = NULL;
            end_ = NULL;
            beginning_ = true;
            hadDotAlready_ = false;
        }

        Token RealLitTokenizer::get() {
            return Token(TOKEN_REALLIT, begin_, end_);
        }

        TokenizerState RealLitTokenizer::consume(const wchar_t * cursor) {
            TokenizerState
                rv = TOKENIZER_HUNGRY;

            if (beginning_)
                {
                    if (*cursor != L'+'
                        && *cursor != L'-'
                        && !isdigit(*cursor)
                        && *cursor != L'.')
                        {
                            rv = TOKENIZER_FAILED;
                        }
                    beginning_ = false;
                    begin_ = cursor;
                }
            else if (*cursor == L'.')
                {
                    if (hadDotAlready_)
                        {
                            rv = TOKENIZER_FINISHED;
                            end_ = cursor;
                        }

                    hadDotAlready_ = true;
                }
            else if (!isdigit(*cursor))
                {
                    rv = TOKENIZER_FINISHED;
                    end_ = cursor;
                }

            return rv;
        }

        WhitespaceTokenizer::WhitespaceTokenizer() { 
            reset(); 
        }

        void WhitespaceTokenizer::reset() 
        { 
            empty_ = true; 
            begin_ = 0;
            end_ = 0; 
        }
		
        Token WhitespaceTokenizer::get() 
        {
            return Token( TOKEN_WS, begin_, end_ );
        }
		
        TokenizerState WhitespaceTokenizer::consume(const wchar_t* cursor) 
        {
            if (!begin_) begin_ = cursor; 
			
            if (isspace(*cursor))  
                {
                    empty_ = false;
                } else {
                end_ = cursor; 
                return empty_ ? TOKENIZER_FAILED : TOKENIZER_FINISHED; 
            }
            return TOKENIZER_HUNGRY;  
        }
		
        IdTokenizer::IdTokenizer() 
        { 
            reset();
        }
		
        void IdTokenizer::reset() 
        {
            begin_ = 0; 
            end_ = 0;  
        }
		
		
        Token IdTokenizer::get() 
        {
            return Token( TOKEN_ID, begin_, end_ );
        }
		
        TokenizerState IdTokenizer::consume(const wchar_t* cursor) 
        {
            if (!begin_) begin_ = cursor; 
            if (cursor == begin_ && !isalpha(*cursor)) {
                return TOKENIZER_FAILED;
            } else if (cursor > begin_ && !isalnum(*cursor)) {  
                end_ = cursor;
                return TOKENIZER_FINISHED; 
            } 
            return TOKENIZER_HUNGRY; 
        }

        SymbolTokenizer::SymbolTokenizer(token_type_t tokenType, const wchar_t* symbol) 
            : tokenType_( tokenType ), 
              symbol_( symbol ) 
        {
        }
		
        void SymbolTokenizer::reset() {
            begin_ = 0; 
        }
		
        Token SymbolTokenizer::get() {
            return Token( tokenType_, begin_, end_ );
        }
		
        TokenizerState SymbolTokenizer::consume(const wchar_t* cursor) {
            if (!begin_) begin_ = cursor; 
            if (symbol_[cursor-begin_] == *cursor) {
                if (!symbol_[cursor-begin_+1]) {
                    // we reached end of symbol
                    end_ = cursor + 1; 
                    return TOKENIZER_FINISHED;
                } 
                return TOKENIZER_HUNGRY; 
            } else {
                return TOKENIZER_FAILED; 
            }
        }
        
        LineCommentTokenizer::LineCommentTokenizer() : state_( READY ) {}
        
        void LineCommentTokenizer::reset() {
        	state_ = READY; 
        }
        Token LineCommentTokenizer::get() {
        	return Token( TOKEN_COMMENT, begin_, end_ ); 
        }
        
        TokenizerState LineCommentTokenizer::consume(const wchar_t* cursor) {
        	switch (state_) {
        		case READY: 
        			if (*cursor == '/') {
						begin_ = cursor; 
						state_ = SLASH_CONSUMED; 
						return TOKENIZER_HUNGRY;
        			}
        			break;
        		case SLASH_CONSUMED:
					if (*cursor == '/') {
						state_ = COMMENT;
						return TOKENIZER_HUNGRY; 
					}
					break; 
        		case COMMENT:
        			if (*cursor == '\n' || *cursor == '\r' || *cursor == '\0') {
						state_ = FINISHED; 
						end_ = cursor; 
						return TOKENIZER_FINISHED;
        			}
					return TOKENIZER_HUNGRY; 
        	}
        	return TOKENIZER_FAILED; 
        }

        SectionCommentTokenizer::SectionCommentTokenizer() : state_( READY ) {}
           
        void SectionCommentTokenizer::reset() {
        	state_ = READY; 
        }
        Token SectionCommentTokenizer::get() {
        	return Token( TOKEN_COMMENT, begin_, end_ );
        }
        TokenizerState SectionCommentTokenizer::consume(const wchar_t* cursor) {
			if (*cursor == '\0') return TOKENIZER_FAILED;
        	switch (state_) {
        		case READY: 
        			if (*cursor == '/') {
						begin_ = cursor; 
						state_ = SLASH_CONSUMED; 
						return TOKENIZER_HUNGRY;
        			}
        			break;
        		case SLASH_CONSUMED: 
					if (*cursor == '*') {
						state_ = COMMENT;
						return TOKENIZER_HUNGRY; 
					}
					break; 
        		case COMMENT:
        			if (*cursor == '*') {
						state_ = STAR_CONSUMED; 
        			}
					return TOKENIZER_HUNGRY; 
        		case STAR_CONSUMED: 
        			if (*cursor == '/') {
						end_ = cursor+1; 
						return TOKENIZER_FINISHED;
        			} else {
						if (*cursor != '*') {
							state_ = COMMENT;
	        			}
						return TOKENIZER_HUNGRY;
        			}
        	}
        	return TOKENIZER_FAILED; 
        }
		
        MultiTokenizer::MultiTokenizer(Tokenizer** tokenizers, bool ownTokenizers) 
            : ownTokenizers_(ownTokenizers)
        {
            int len = 0; while (tokenizers[len]) len++; 
            tokenizers_.assign(tokenizers,
                               tokenizers + len);
            states_ = new TokenizerState[len]; 
            reset(); 
        }

        MultiTokenizer::~MultiTokenizer()
        {
            if (ownTokenizers_) 
                {
                    typedef std::vector<Tokenizer*>::iterator iterator; 
                    for (iterator i = tokenizers_.begin(); i != tokenizers_.end(); ) 
                        {
                            delete *(i++); 
                        }
                }
            delete[] states_; 
        }
		

        void MultiTokenizer::reset() 
        {
            TokenizerState* s = states_;
            running_ = 0; 
            std::vector<Tokenizer*>::iterator
                i = tokenizers_.begin(),
                end = tokenizers_.end();

            for (; i != end; ++i, ++s) {
                (*i)->reset();
                (*s) = TOKENIZER_HUNGRY;
                running_++; 
            }
            found_ = false;
        }
		 
        Token MultiTokenizer::get() 
        {
            Token token(TOKEN_UNKNOWN, 0, 0); 
            TokenizerState* s = states_;
            std::vector<Tokenizer*>::iterator
                i = tokenizers_.begin(),
                end = tokenizers_.end();

            for (; i != end; ++i, ++s ) {
                if (*s == TOKENIZER_FINISHED) {
                    Token c = (*i)->get(); 
                    if (c.length() > token.length()) {
                        token = c; 
                    }
                }
            }
            if (token.length() == 0) {
                // NOTE: not really a lexical exception, but logical one
                throw LexException(L"Trying to get token without a token ready.", 0); 
            }
            return token;
        }

        TokenizerState MultiTokenizer::consume(const wchar_t* cursor) {
            TokenizerState* s = states_;
            std::vector<Tokenizer*>::iterator
                i = tokenizers_.begin(),
                end = tokenizers_.end();

            for (; i != end; ++i, ++s) {
                if (*s == TOKENIZER_HUNGRY) 
                    {
                        *s = (*i)->consume(cursor);
                        if (*s != TOKENIZER_HUNGRY) running_--; 
                        if (*s == TOKENIZER_FINISHED) {
                            found_ = true; 
                        }
                    }
            }
            if (running_ == 0) {
                return found_ ? TOKENIZER_FINISHED : TOKENIZER_FAILED; 
            }
            return TOKENIZER_HUNGRY;
        }
		

        LitTokenizer::LitTokenizer(wchar_t citate)
            : multiTokenizer_(NULL)
        {
            using namespace std;

            auto_ptr<StrLitTokenizer>
                s(new StrLitTokenizer(citate));
            auto_ptr<IntLitTokenizer>
                i(new IntLitTokenizer);
            auto_ptr<RealLitTokenizer>
                r(new RealLitTokenizer);

            Tokenizer * tokenizers[] = {
                s.get(),
                i.get(),
                r.get(),
                NULL
            };

            multiTokenizer_ = new MultiTokenizer(tokenizers, true);
                
            s.release();
            i.release();
            r.release();

            reset();
        }


        LitTokenizer::~LitTokenizer()
        {
            delete multiTokenizer_;
        }

        void LitTokenizer::reset()
        {
            multiTokenizer_->reset();
        }

        Token LitTokenizer::get()
        {
            Token
                subToken = multiTokenizer_->get();

            return Token(TOKEN_LIT,
                         subToken.begin(),
                         subToken.end());
        }

        TokenizerState LitTokenizer::consume(const wchar_t * cursor)
        {
            return multiTokenizer_->consume(cursor);
        }
            
        TokenIterator::~TokenIterator() {}
        
        WhitespaceSplitter::WhitespaceSplitter(const wchar_t* text) 
        : begin_( text ), end_( 0 ) {}
        
        WhitespaceSplitter::operator bool() {
        	if ( !end_ && *begin_ ) {
				// skip whitespace
				while (iswspace(*begin_)) begin_++;
				end_ = begin_;
				// consume letters
				while (*end_ && !iswspace(*end_)) end_++; 
        	}
        	return *begin_; 
        }
        
        Token WhitespaceSplitter::operator++(int) {
        	if (!*this) throw LexException(L"Out of tokens.", begin_);
        	Token ret(TOKEN_UNKNOWN, begin_, end_); 
        	begin_ = end_; 
        	end_ = 0; 
        	return ret; 
        }

        Tokens::Tokens(Tokenizer& tokenizer, const wchar_t* text)
            :	cursor_(text),
                tokenizer_(tokenizer), 
                hasNext_(false)
        {}
		
        Tokens::operator bool() {
            prepareNext(); 
            return hasNext_;
        } 
		
        Token Tokens::operator++(int) {
            prepareNext();
            if (!hasNext_) {
                throw LexException(L"Out of tokens.", cursor_);
            }
            hasNext_ = false;
            // get the token
            Token ret = tokenizer_.get();
            cursor_ = ret.end();
            return ret;
        }
				
        void Tokens::prepareNext() {
            if (!hasNext_ && *cursor_) {
                const wchar_t* begin = cursor_; 
                tokenizer_.reset(); 
                TokenizerState state = TOKENIZER_HUNGRY;
                while (state == TOKENIZER_HUNGRY) {
                    state = tokenizer_.consume(cursor_);
                    if (*cursor_) cursor_++; // don't go beyond eof. 
                }
                if (state == TOKENIZER_FAILED) {
                    std::wostringstream msg; 
                    msg<<L"Unrecognized syntax: '";
                    for (int i = 0; &begin[i] < cursor_; i++) msg<<begin[i];
                    msg<<L"'";
                    throw LexException(msg.str().c_str(), begin); 
                } else { 
                    // Means that: state == TOKENIZER_FINISHED
                    hasNext_ = true; 
                }
            }
        }

        StdFilter::StdFilter(TokenIterator& tokens) 
            :	tokens_(tokens), next_(), hasNext_(false) {}
		
        StdFilter::operator bool()
        {
            prepareNext();
            return hasNext_; 
        }
		
        Token StdFilter::operator++(int)
        {
            prepareNext();
            if (!hasNext_) {
                throw LexException(L"Out of tokens", 0); 
            }
            hasNext_ = false;
            return next_;
        }
        void StdFilter::prepareNext()
        {
            while (!hasNext_ && tokens_) {
                next_ = tokens_++;
                if (next_.type() != TOKEN_WS 
                 && next_.type() != TOKEN_COMMENT) {
                    hasNext_ = true; 
                }
            }
        }

		
        TokenReader::TokenReader(TokenIterator& tokens) 
            :	tokens_(tokens), 
                location_(0),
                forward_(), 
                backward_(), 
                marks_()
        {}
		
		
        TokenReader::operator bool() {
            return !forward_.empty() || tokens_; 
        }
		
        Token TokenReader::operator++(int) {
            Token token; 
            if (forward_.size() > 0) {
                token = forward_.back();
                forward_.pop_back(); 
            } else {
                token = tokens_++; 
            }
            if (!marks_.empty()) {
                backward_.push_back(token);  
            }
            location_++; 
            return token; 
        }

        Token TokenReader::peek() {
            if (forward_.empty()) {
                Token token = (*this)++;
                forward_.push_back(token); 
                return token; 
            } else {
                return forward_.back(); 
            }
        }

        void TokenReader::pushMark() {
            marks_.push_back(location_); 
        }
		
        void TokenReader::popMark() {
            int mark = marks_.back(); marks_.pop_back();
            while (location_ > mark) {
                forward_.push_back(backward_.back()); 
                backward_.pop_back();
                location_--;
            }
        }
		
        void TokenReader::clearMark() {
            marks_.back(); marks_.pop_back();
            if (marks_.empty()) {
                backward_.clear(); 
            }
        }
		
    } // Lex 
	
    namespace Parser {
	
        ParseException::ParseException(const wchar_t* wWhat, 
                                       const Lex::Token& where) 
            : wWhat_(wWhat), 
              where_(where) {
            ;
        }
		

        Lex::Token ParseException::where() const {
            return where_;
        }


        const wchar_t* ParseException::wWhat() const throw() {
            return wWhat_.c_str();
        }
		
        void ParseException::setContext(const wchar_t * context)
        {
			wWhat_ = describeException(wWhat_, context, where_.begin(), where_.end()); 
        }
		
        namespace Lit {
		
            std::wstring ParseString(const Lex::Token& token) {
                if (token.type() != Lex::TOKEN_STRLIT) {
                    std::wostringstream msg; 
                    msg<<L"Expected literal instead of token '"<<token.text()<<"' of type "<<token.type(); 
                    throw ParseException(msg.str().c_str(), token);  
                }
                std::wstring ret; 
                const wchar_t* text = token.begin(); 
                // NOTE: We are assuming that the literal sitation marks are one character wide
                for (int i = 1; &text[i] < token.end()-1; i++) {// skip first and last characters
                    if (text[i] == Lex::ESCAPE_SYMBOL) {
                        i++; 
                        switch (text[i]) {
                        case '0':
                            ret += L"\0";
                            break;
                        case 'n':
                            ret += L"\n";
                            break;
                        case 'r':
                            ret += L"\r";
                            break;
                        case 't':
                            ret += L"\t";
                            break;
                        default: 
                            ret += text[i]; 
                        }
                    } else {
                        ret += text[i];
                    }
                }
                return ret; 
            }
            long ParseInteger(const Lex::Token& token) {
                if (token.type() != Lex::TOKEN_INTLIT) {
                    std::wostringstream msg; 
                    msg<<L"Expected literal instead of token '"<<token.text()<<"' of type "<<token.type(); 
                    throw ParseException(msg.str().c_str(), token);  
                }
                wchar_t* end = const_cast<wchar_t*>(token.end());
                return wcstol(token.begin(), &end, 10);
            }
            double ParseReal(const Lex::Token& token) {
                if (token.type() != Lex::TOKEN_REALLIT) {
                    std::wostringstream msg; 
                    msg<<L"Expected literal instead of token '"<<token.text()<<"' of type "<<token.type(); 
                    throw ParseException(msg.str().c_str(), token);  
                }
                wchar_t* end = const_cast<wchar_t*>(token.end());
                return wcstod(token.begin(), &end);
            }
        }
		

        Lexer::Lexer(Lex::TokenIterator& tokens) : Lex::TokenReader(tokens) {
        }
	
        Lex::Token Lexer::operator++(int) {
            if (*this) {
                return Lex::TokenReader::operator++(0); 
            }
            throw ParseException(L"Unexpected EOF", Lex::Token(Lex::TOKEN_EOF, 0, 0));  
        }

        Lex::Token Lexer::eat(Lex::token_type_t tokenType) {
            Lex::Token token = ((*this)++);
            if (token.type() != tokenType) {
                std::wostringstream msg; 
                msg<<"Expected "<<tokenType<<" instead of token '"<<token.text()<<"' of type "<<token.type();  
                throw ParseException(msg.str().c_str(), token);  
            }
            return token; 
        }
        std::wstring Lexer::eatId() {
            Lex::Token token = ((*this)++);
            if (token.type() != Lex::TOKEN_ID) {
                std::wostringstream msg; 
                msg<<L"Expected identifier instead of token '"<<token.text()<<"' of type "<<token.type(); 
                throw ParseException(msg.str().c_str(), token);  
            }
            return token.text(); 
        }

        void Lexer::eatEof() {
            if (*this) {
                Lex::Token token = ((*this)++);
                std::wostringstream msg; 
                msg<<L"Expected EOF instead of '"<<token.text()<<"' of type "<<token.type(); 
                throw ParseException(msg.str().c_str(), token);  
            }
        }

        std::wstring Lexer::eatString() {
            return Lit::ParseString((*this)++); 
        }

        long Lexer::eatInteger() {
            return Lit::ParseInteger((*this)++); 
        }

        double Lexer::eatReal() {
            return Lit::ParseReal((*this)++); 
        }

        StdLexer::StdLexer(Lex::Tokenizer& tokenizer, const wchar_t* text) 
            : Lexer(filter_),
              tokens_(tokenizer, text), 
              filter_(tokens_)
              
        {}
		
		
    } // Parser
} // Cpt

