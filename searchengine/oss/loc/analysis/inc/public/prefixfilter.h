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

#ifndef PREFIXANALYSIS_H_ 
#define PREFIXANALYSIS_H_ 

#include "CLucene.h" 

namespace analysis {

    extern const wchar_t* HebrewPrefixes[];

    /**
     * Returns alternative versions of encountered tokens. First version
     * is without modifications. Other versions are with the potential prefixes
     * removed. So, if we have word with three prefixes P1, P2 and P3 and some 
     * word stem W, then first returned word is P1P2P3W, second is P2P3W, third
     * is P3W and last one is W. 
     */
    class PrefixFilter : public lucene::analysis::TokenFilter {
	
		public:
			
			PrefixFilter(lucene::analysis::TokenStream* input,
						 bool deleteTs, 
						 const wchar_t** prefixes);
			
			/** Sets token to the next token in the stream, returns false at the EOS. */
			virtual bool next(lucene::analysis::Token* token);
			
		private:
			const wchar_t** prefixes_;
			lucene::analysis::Token token_;
			
			bool prefixFound_; 
	
			
        
    };
    
    /**
     * Standard tokenizer + standard filter + lowercase analyzer + hebrew analyzer
     */
    class HebrewAnalyzer : public lucene::analysis::Analyzer {

    public: 
		virtual lucene::analysis::TokenStream* 
			tokenStream(const wchar_t* fieldName, lucene::util::Reader* reader);

    };
    
    /**
     * Standard tokenizer + standard filter + lowercase analyzer 
     */
    class HebrewQueryAnalyzer : public lucene::analysis::Analyzer {

    public: 
		virtual lucene::analysis::TokenStream* 
			tokenStream(const wchar_t* fieldName, lucene::util::Reader* reader);

    };
    
    extern const wchar_t* FrenchArticles[];

    /**
     * Elision filter drops article+apostrophe pairs from the beginning
     * of the words. 
     */
    class ElisionFilter : public lucene::analysis::TokenFilter {

    public:
    
		ElisionFilter(lucene::analysis::TokenStream* input,
                     bool deleteTs, 
                     const wchar_t** articles);
        
        /** Sets token to the next token in the stream, returns false at the EOS. */
        virtual bool next(lucene::analysis::Token* token);
        
    private:
        
        const wchar_t** articles_;
        
    };
    
    /** Standard analyzer + standard filter + lowercase filter + elision filter */
    class FrenchAnalyzer : public lucene::analysis::Analyzer {

    public: 
		virtual lucene::analysis::TokenStream* 
			tokenStream(const wchar_t* fieldName, lucene::util::Reader* reader);

    };
    
    class NonEnglishStopWords {
        
    public:
        static const TCHAR* FRENCH_STOP_WORDS[];
        static const TCHAR* BRAZILIAN_STOP_WORDS[];
        static const TCHAR* CZECH_STOP_WORDS[];
        static const TCHAR* GERMAN_STOP_WORDS[];
        static const TCHAR* GREEK_STOP_WORDS[];
        static const TCHAR* DUTCH_STOP_WORDS[];
        static const TCHAR* RUSSIAN_STOP_WORDS[];
        static const TCHAR* EXTENDED_ENGLISH_STOP_WORDS[];
        
    };
    
}

#endif /* PREFIXANALYSIS_H_ */
