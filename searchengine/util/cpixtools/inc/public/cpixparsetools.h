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
 * cpixparsetools.h
 *
 *  Created on: Apr 14, 2009
 *      Author: admin
 */

#ifndef CPIXPARSETOOLS_H_
#define CPIXPARSETOOLS_H_

#include <string>
#include <wchar.h>
#include <ctype.h>
#include <exception>
#include <vector>
#include <memory>

namespace Cpt {


    /**
     * Exception class that can gather some contextual information as
     * it ascends in the call stack.
     */
    class ITxtCtxtExc
    {
    public:
        /**
         * TODO comment
         */
        virtual const wchar_t * wWhat() const throw () = 0;

        /**
         * TODO comment: after an invocation of setContext, the string
         * returned by wWhat() should be far more informative.
         */
        virtual void setContext(const wchar_t * context) = 0;


        /**
         * TODO comment and move to .cpp file
         */
        virtual ~ITxtCtxtExc() { ; }
    };



    /**
     * Tools for lexical analysis, that is parsing regular expression
     * strength language and converting the character stream into
     * language token stream.  Note: Regular expression syntax
     * (e.g. "file*.tx?") itself is not supported)
     */
    namespace Lex {
    
		typedef const wchar_t* token_type_t; 


		extern token_type_t TOKEN_UNKNOWN;
		extern token_type_t TOKEN_EOF;
		extern token_type_t TOKEN_WS; 
		extern token_type_t TOKEN_COMMENT;  
		extern token_type_t TOKEN_ID;	
		extern token_type_t TOKEN_STRLIT;
		extern token_type_t TOKEN_INTLIT;
		extern token_type_t TOKEN_REALLIT;
		extern token_type_t TOKEN_LIT;
		
        class LexException : public ITxtCtxtExc {
        public: 
            LexException(const wchar_t* what, const wchar_t* where);
            virtual ~LexException(); 
            const wchar_t * where() const;
            virtual const wchar_t * wWhat() const throw ();
            virtual void setContext(const wchar_t * context);
        private:
            std::wstring wWhat_; 
            const wchar_t* where_;
        };
	
        /**
         * A token provided by regular expression. Contains pointers
         * into the original data string. NOTE: this object becomes
         * hazardous, if the original tokenized string is modified or
         * released.
         */
        class Token {
        public: 
            Token(token_type_t type, const wchar_t* begin, const wchar_t* end);
            Token();
            const wchar_t* type() const;
            const wchar_t* begin() const;
            const wchar_t* end() const;
            int length() const;
            std::wstring text() const; 
        private: 
            token_type_t type_;
            const wchar_t* begin_;
            const wchar_t* end_;
        };
		
        /**
         * Describes the state of the tokenizer. 
         */
        enum TokenizerState { 
            TOKENIZER_HUNGRY, 	/// ok, but don't have yet a proper token formed.
            TOKENIZER_FINISHED, /// has ready token
            TOKENIZER_FAILED 	/// failed
        };
		
        /**
         * Provides mechanism for extracting nearly regular expression
         * strength tokens.
         */
        class Tokenizer {
        public: 
            virtual ~Tokenizer(); 
            /**
             * Prepares the tokenizer for consuming a new token.
             */
            virtual void reset() = 0;
            /**
             * Returns the token. MUST be called only, if tokenizer
             * has returned TOKENIZER_FINISHED with last call to
             * consume and reset has not been called.
             */
            virtual Token get() = 0; 
				
            /**
             * Consumes a character and returns the state of the
             * tokenizer. If the tokenizer returns TOKENIZER_FINISHED,
             * the read token can be requested by using get().
             */
            virtual TokenizerState consume(const wchar_t* cursor) = 0; 
        };
		
        class StrLitTokenizer : public Tokenizer {
        public: 
            StrLitTokenizer(wchar_t citate); 
            virtual void reset();
            virtual Token get(); 
            virtual TokenizerState consume(const wchar_t* cursor);
        private:
            const wchar_t* begin_; 
            const wchar_t* end_; 
            wchar_t citate_;
            bool opened_; 
            bool escape_;
        };
	
        class IntLitTokenizer : public Tokenizer {
        public:
            IntLitTokenizer();
            virtual void reset();
            virtual Token get();
            virtual TokenizerState consume(const wchar_t * cursor);
                
        private:
            const wchar_t * begin_;
            const wchar_t * end_;
            bool            beginning_;
        };


        class RealLitTokenizer : public Tokenizer {
        public:
            RealLitTokenizer();
            virtual void reset();
            virtual Token get();
            virtual TokenizerState consume(const wchar_t * cursor);
                
        private:
            const wchar_t * begin_;
            const wchar_t * end_;
            bool            beginning_;
            bool            hadDotAlready_;
        };

        class WhitespaceTokenizer : public Tokenizer {
        public: 
            WhitespaceTokenizer();
            virtual void reset();
            virtual Token get();
            virtual TokenizerState consume(const wchar_t* cursor);
        private:
            const wchar_t* begin_; 
            const wchar_t* end_; 
            bool empty_;
        };
	
        class IdTokenizer : public Tokenizer {
        public: 
            IdTokenizer(); 
            virtual void reset();
            virtual Token get();
            virtual TokenizerState consume(const wchar_t* cursor);
        private:
            const wchar_t* begin_; 
            const wchar_t* end_; 
        };
	
        class SymbolTokenizer : public Tokenizer {
        public: 
            SymbolTokenizer(const wchar_t* tokenType, const wchar_t* symbol);
            virtual void reset();
            virtual Token get();
            virtual TokenizerState consume(const wchar_t* cursor);
        private:
            const wchar_t* begin_;
            const wchar_t* end_; 
            token_type_t tokenType_; 
            const wchar_t* symbol_;
        };
 
        /**
         * C style line comment, e.g. // comment
         */
        class LineCommentTokenizer : public Tokenizer {
        public: 
        	LineCommentTokenizer();
            virtual void reset();
            virtual Token get();
            virtual TokenizerState consume(const wchar_t* cursor);
        private:
            enum State {
				READY,
				SLASH_CONSUMED, 
				COMMENT,
				FINISHED
            };
        	State state_;
        	const wchar_t* begin_; 
        	const wchar_t* end_;
        };

        /**
         * C++ style section comments. Like the one's surrounding this comment
         */
        class SectionCommentTokenizer : public Tokenizer {
        public: 
        	SectionCommentTokenizer();
            virtual void reset();
            virtual Token get();
            virtual TokenizerState consume(const wchar_t* cursor);
        private:
            enum State {
				READY,
				SLASH_CONSUMED, 
				COMMENT, 
				STAR_CONSUMED, 
				FINISH
            };
        	State state_;
        	const wchar_t* begin_; 
        	const wchar_t* end_;
        	
        };

        /**
         * Tokenizes text by using given tokenizers. Text is consumed
         * until no tokenizer is in hungry state e.g., all tokenizers
         * are either failed or finished. In case a number of
         * tokenizers have finished, the longest token is used. If a
         * number of tokens have the same length, the order is
         * determined based on order of the tokenizers array. The
         * tokenizers first in the list have highest priority.
         */
        class MultiTokenizer : public Tokenizer {
        public: 
            /**
             * @param tokenizers zero-terminated array. Nothing passes
             * ownership. The array itself need not be alive later,
             * but the contained tokenizers do.
             */
            MultiTokenizer(Tokenizer** tokenizers, bool ownTokenizers = false);
            virtual ~MultiTokenizer(); 
            void reset();
            Token get();
            TokenizerState consume(const wchar_t* cursor);
        private:
            int running_;
            bool found_; 
            TokenizerState* states_;
            std::vector<Tokenizer*> tokenizers_;
            bool ownTokenizers_;
        };

        /**
         * Tokenizes whatever literals: string, real or int. The token
         * type of the tokens returned by this class are TOKEN_LIT
         * (you don't know if they are str, int or real).
         */
        class LitTokenizer : public Tokenizer {
        public:
            /**
             * Strings literals will be tokenized according to citate
             * (see StrLitTokenizer constructor).
             */
            LitTokenizer(wchar_t citate);
            virtual ~LitTokenizer();
            virtual void reset();
            virtual Token get();
            virtual TokenizerState consume(const wchar_t * cursor);

        private:
            MultiTokenizer   * multiTokenizer_;
        };

        /**
         * Not-so-C++ style iterator for iterating through tokens.
         * When having a variable i of type TokenIterator, check if
         * (i) can be used to see
         */
        class TokenIterator {
        public:
            /**
             * Returns true, if token iterator contains more tokens
             */
            virtual operator bool() = 0;
				
            /**
             * Returns the next token in the stream.  FIXME should be
             * operator of form ++i. Current syntax breaks C++
             * standard
             */
            virtual Token operator++(int) = 0;
				
            virtual ~TokenIterator(); 
        };
        
        class WhitespaceSplitter : public TokenIterator {
        public:
        	WhitespaceSplitter(const wchar_t* text);
            virtual operator bool();
            virtual Token operator++(int);
        public: 
            const wchar_t* begin_;
            const wchar_t* end_;
        };
		
        /**
         * Uses tokenizer for converting given text into token stream
         * and provides means for iterating throught the token
         * stream's tokens.
         */
        class Tokens : public TokenIterator {
        public:
            Tokens(Tokenizer& tokenizer, const wchar_t* text);
            virtual operator bool();
            virtual Token operator++(int);
        private:
            void prepareNext();
        private: // data
            /**
             * Current location in the text
             */
            const wchar_t* cursor_; 
            Tokenizer& tokenizer_;
            bool hasNext_; 
        };
		
        /**
         * Filters out all tokens of type TOKEN_WS
         */
        class StdFilter : public TokenIterator {
        public:
        	StdFilter(TokenIterator& tokens);
            virtual operator bool();
            virtual Token operator++(int);
        private:
            void prepareNext();
        private: // data
            TokenIterator& tokens_; 
            Token next_;  
            bool hasNext_;
        };
 
        /**
         * Provides mechanism for marking a location in the token
         * stream and returning to it. Essential tool for parsing
         * context free grammar.
         */
        class TokenReader : public TokenIterator {
        public: 
            TokenReader(TokenIterator& tokens); 
            virtual operator bool();
            virtual Token operator++(int);
            Token peek(); 
            void pushMark();   // stores this position
            void popMark();    // pops position and returns reader to point it
            void clearMark();  // pops position and frees it; may also free tokens in backward buf.
        private: 
            TokenIterator& tokens_;
            int location_;
            std::vector<Token> forward_; // buffer for tokens front of this location
            std::vector<Token> backward_; // buffer for tokens behind this location
            std::vector<int> marks_; 
        };

        /**
         * Marks the position in the token stream and returns the
         * token stream position, when going out of scope unless
         * release is called.
         */
        class TokenPositionSentry {
        public:
            inline TokenPositionSentry(TokenReader& reader) 
                : reader_(reader), released_(false) { 
                reader_.pushMark(); 
            }
            inline ~TokenPositionSentry() {
                if (released_) {
                    reader_.clearMark(); 
                } else {
                    reader_.popMark(); 
                }
            }
            inline void release() { released_ = true; } 
        private:
            bool released_; 
            TokenReader& reader_; 
        };


    } // Lex 
	
    /**
     * Tools for parsing context-free grammar
     */
    namespace Parser {

        /**
         * Informs of an parsing error. 
         */
        class ParseException : public ITxtCtxtExc {
        public: 
            ParseException(const wchar_t* wWhat, const Lex::Token& where);
            Lex::Token where() const; 
            virtual const wchar_t * wWhat() const throw ();
            virtual void setContext(const wchar_t * context);
				
        private:
            std::wstring wWhat_; 
            Lex::Token where_;
        };
		
        namespace Lit {
            std::wstring ParseString(const Lex::Token& token) ;  
            long ParseInteger(const Lex::Token& token);
            double ParseReal(const Lex::Token& token);
        }
		
        /**
         * Utility, which is used for parsing. Throws parse
         * exceptions, when underlying assumptions don't hold.
         */
        class Lexer : public Lex::TokenReader {
        public: 
            Lexer(Lex::TokenIterator& tokens); 
            // throws ParseException instead of LexException on EOF. 
            virtual Lex::Token operator++(int);
            Lex::Token eat(Lex::token_type_t tokenType);
            void eatEof();
            std::wstring eatId();
            std::wstring eatString();
            long eatInteger();
            double eatReal();
        };
	
        /*
         * Creates token stream iterator and whitespace filter
         */ 
        class StdLexer : public Lexer {
        public: 
            StdLexer(Lex::Tokenizer& tokens, const wchar_t* text); 
        private: 
            Lex::Tokens tokens_; 
            Lex::StdFilter filter_;
        };
		
    } // Parser
} // Cpt

#endif /* CPIXPARSETOOLS_H_ */
