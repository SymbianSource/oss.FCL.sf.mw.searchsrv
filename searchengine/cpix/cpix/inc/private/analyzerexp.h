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


#ifndef ANALYZEREXP_H_
#define ANALYZEREXP_H_

#include "cpixparsetools.h"

#include "cpixmemtools.h"

namespace Cpix {

    //
    // NOTE: Following pieces of code form the analyzer definition language
    //       parsing of the CPix analysis. Other of CPix analysis are 
    //
    //       1) Actual implementation with classes implementing CLucene 
    //          Analyzer/Tokenizer/Filter abstractions
    //
    //       2) Glue code, reading the parsed analyzer definition and 
    //          constructing corresponding implementations. The glue is 
    //          C++ template heavy. 
    //

    /**
     * Describes the expression that is used to define cpix analyzers. 
     */
    namespace AnalyzerExp {
 
        /** Identifiers for the tokens. Extends the list present in the cpixparsetools.h */
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

        /**
         * Tokenizer used for analyzer definition strings' lexical analysis
         */
        class Tokenizer : public Cpt::Lex::Tokenizer {
        public:
            Tokenizer();
            virtual ~Tokenizer();
        public: 
            virtual void reset();
            virtual Cpt::Lex::Token get();
            virtual Cpt::Lex::TokenizerState consume(const wchar_t* cursor);
        private: // data
            Cpt::Lex::WhitespaceTokenizer ws_;	 
            Cpt::Lex::IdTokenizer ids_; 
            Cpt::Lex::StrLitTokenizer strlits_;
            Cpt::Lex::IntLitTokenizer intlits_;
            Cpt::Lex::RealLitTokenizer reallits_;
            Cpt::Lex::SymbolTokenizer lb_;	// left bracket
            Cpt::Lex::SymbolTokenizer rb_;	// right bracket
            Cpt::Lex::SymbolTokenizer cm_;	// comma
            Cpt::Lex::SymbolTokenizer pp_;      // pipe symbol '>'
            Cpt::Lex::SymbolTokenizer sw_;      // switch
            Cpt::Lex::SymbolTokenizer cs_;	// case
            Cpt::Lex::SymbolTokenizer df_;	// default
            Cpt::Lex::SymbolTokenizer lbc_;	// left brace
            Cpt::Lex::SymbolTokenizer rbc_;     // right brace
            Cpt::Lex::SymbolTokenizer cl_;	// colon ':'
            Cpt::Lex::SymbolTokenizer tr_;	// semicolon ';'
            Cpt::Lex::Tokenizer** tokenizers_;
            std::auto_ptr<Cpt::Lex::MultiTokenizer> tokenizer_;
        };

        /**
         * Expression of the analyzer definition language. Result of parsing
         */
        class Exp {
        public:
            virtual ~Exp();
        };

        /** Represents a string literal of form: 'abc 123' */
        class StringLit : public Exp 
        {
        public:
            StringLit(const std::wstring& text);
            /**
             * The content of the literal with escapes interpreted and
             * and citation marks removed.
             */
            const std::wstring& text() const;
        private: 
            std::wstring text_;
        };

        /** Represents an integer literal of form: 123 */
        class IntegerLit : public Exp 
        {
        public:
            IntegerLit(long value);
            long value() const;
        private: 
            long value_; 
        };

        /** Represents a real number literal of form: 443.122  */
        class RealLit : public Exp 
        {
        public:
            RealLit(double value);
            double value() const;
        private: 
            double value_; 
        };

        /**
         * Identifier of form: identifier
         */
        class Identifier : public Exp 
        {	
        public:
            Identifier(const std::wstring& id);
            const std::wstring& id() const;
        private: 
            std::wstring id_; 
        };

        /**
         * Invokation's parameters, (param, param, param)
         * Parameters may be only literals or identifiers. 
         */
        class Parameters : public Exp
        {
        public: 
            virtual ~Parameters(); 
            Parameters(Cpt::auto_vector<Exp>& params);
            const std::vector<Exp*>& params() const;
        private:
            Cpt::auto_vector<Exp> params_; 
        };

        /**
         * Invokation describes a form of call or construction 
         * (e.g. identifier(param, param, param)). 
         */
        class Invokation : public Exp 
        {
        public:
            Invokation(Identifier id, std::auto_ptr<Parameters> parameters);
            virtual ~Invokation(); 
            const std::wstring& id() const; 
            const std::vector<Exp*>& params() const; 
        private:
            std::wstring id_; 
            std::auto_ptr<Parameters> parameters_; 
        };

        std::auto_ptr<Invokation> ParseInvokation(Cpt::Parser::Lexer& lexer) ;


        /**
         * Piping contains an actual analyzer definition.  Why it is
         * called 'piping' is because it's textual expression is a
         * sequence of piped items
         * (e.g. tokenizer>filter>filter>filter), where the first item
         * is always tokenizer and the rest are filters.  In the
         * sequence, the tokenstream output of previous item is piped
         * as the token stream input of the the next item.
         */
        class Piping : public Exp {
        public: 
            Piping(std::auto_ptr<Exp>             tokenizer, 
                   Cpt::auto_vector<Invokation> & filters);
            virtual ~Piping(); 
            const Exp& tokenizer() const; 
            const std::vector<Invokation*>& filters() const;
        private: 
            std::auto_ptr<Exp> tokenizer_; 
            Cpt::auto_vector<Invokation> filters_; 
        };

        /**
         * A case of switch statement. Of form: "case 'field':
         * tokenizer>filter>filter;"
         */
        class Case : public Exp {
        public: 
            Case(const std::vector<std::wstring> & fields, 
                 std::auto_ptr<Piping>             piping); 
            virtual ~Case();
            const std::vector<std::wstring>& fields() const; 
            const Piping& piping() const; 
        private:
            std::vector<std::wstring> fields_; 
            std::auto_ptr<Piping> piping_;
        };
        
        /**
         * Switch expression. Contains 0...n cases and exactly one default. 
         * Having no default will raise parse exception. 
         */
        class Switch : public Exp {
        public: 
            Switch(Cpt::auto_vector<Case> & cases, 
                   std::auto_ptr<Piping>    def); 
            virtual ~Switch(); 
            const std::vector<Case*>& cases() const; 
            const Piping& def() const;
        public: 
            Cpt::auto_vector<Case> cases_; 
            std::auto_ptr<Piping> def_;
        };
        std::auto_ptr<Piping> ParsePiping(Cpt::Parser::Lexer& lexer);

    }
}


#endif /* ANALYZEREXP_H_ */
