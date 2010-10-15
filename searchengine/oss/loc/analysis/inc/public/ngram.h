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
#ifndef NGRAM_H_
#define NGRAM_H_

#include "tinyanalysis.h"
#include "tinyunicode.h"
#include "clutil.h"

namespace analysis {

	/**
	 * Returns true, if the character is non-cjk letter
	 */
    int IsNonCjk(int c);
    
    /**
     * TinyCjkTokenizer. Contains tiny analysis classes, that are 
     * used to turn Chinese, Korean and Japanese into 1-grams, while
     * using letter analyzer for other kinds of text (western, cyrillic,
     * etc.)   
     * 
     * @tparam I the iterator, that is used to read characters
     */
    template<typename I>
    struct TinyCjkTokenizer {
    
    	/** Deals with cjk */
        tiny::NGramTokenizer<I> cjk_;
    	/** Letter tokenizer for space separated language */
        tiny::CustomTokenizer<I> noncjk_; 
    	/** Combines cjk with noncjk */
        tiny::PairTokenizer<I> pair_;
        /** Moves forward, if tokenization fails */
        tiny::RelaxedTokenizer<I> t_;
        
        /** 
         * Constructs the tiny cjk tokenizer with given ngram size
         *
         * @param ngramsize cjk text is treated with n-gram analyzer of this size   
         */
        TinyCjkTokenizer(int ngramsize) 
        : cjk_(ngramsize, &unicode::IsCjk),
          noncjk_(&IsNonCjk),
          pair_(cjk_, noncjk_),
          t_(pair_) {}          
        
        /**
         * Consumes a token from given iterator. Returns n-grams
         * for cjk text, letter tokenized words for non-cjk text. 
         * Always returns something unless EOS has been reached.
         */
        inline tiny::Token<I> consume(I& i) {
            return t_.consume(i);
        }
    };

    
    /**
     * Constructs n-grams of Chinese, Korean and Japanese text. Uses
     * letter tokenization for other kinds of texts. 
     */
	class CjkNGramTokenizer : public lucene::analysis::Tokenizer {
	
		public: 
	
			/** Reads from buffer */
			typedef tiny::cl::ReaderBuffer<512>::iterator buffer_iterator;

			/** Turns utf16 to unicode */
	        typedef tiny::Utf16Iterator<buffer_iterator> iterator;

		public:
	
			CjkNGramTokenizer( lucene::util::Reader* reader, int gramSize );
			
			virtual bool next( lucene::analysis::Token* token );			
	
		private:

			/** The tokenizer */
            TinyCjkTokenizer<iterator> t_;

            /** Buffer */
			tiny::cl::ReaderBuffer<512> in_;

			/** Reads utf16 from buffer and transforms it to unicode*/
			iterator i_; 
			
	};
	
	/**
	 * The great difference of this class compared to CJK ngram, 
	 * that it decomposes Hangul syllables into Hangul Jamu letters.
	 * 
	 * This analyzer appeared to have bad performance in testing. 
	 */
	class JamuNGramTokenizer : public lucene::analysis::Tokenizer {
   
       public:
    
           typedef tiny::cl::ReaderBuffer<512>::iterator buffer_iterator;

           typedef tiny::Utf16Iterator<buffer_iterator> utf16_iterator;
            
           typedef tiny::JamuIterator<utf16_iterator> iterator;
            
       public:
    
           JamuNGramTokenizer( lucene::util::Reader* reader, int gramSize );
            
           virtual bool next( lucene::analysis::Token* token );            
    
       private:
            
           TinyCjkTokenizer<iterator> t_;
       
           tiny::cl::ReaderBuffer<512> in_;
            
           iterator i_; 
           
    };
	
	// Analyzers using the tokenizers
	//   * Provided mainly for testing
	//
	
	/** CjkNGramTokenizer plus lowercase filter */
    typedef TemplateAnalyzer1A1F<CjkNGramTokenizer, int, lucene::analysis::LowerCaseFilter> 
        CjkNGramAnalyzer;
    
	/** JamuNGramTokenizer plus lowercase filter */
    typedef TemplateAnalyzer1A1F<JamuNGramTokenizer, int, lucene::analysis::LowerCaseFilter> 
        JamuNGramAnalyzer;

}


#endif /* NGRAM_H_ */
