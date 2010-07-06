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

#ifndef KOREANANALYZER_H_
#define KOREANANALYZER_H_

#include "Clucene.h"

#include "ngram.h"

#include "tinyanalysis.h"
#include "tinyutf16.h"
#include "tinyunicode.h"

namespace analysis 
{
    // Forward declarations

	/**
	 * Special Korean analyzer that is designed so, that Cpix can 
	 * update the result list, when each individual Jamu character is
	 * entered.
	 * 
	 * The analyzer tries to first convert given character stream into 
	 * a form, where all Jamu characters are composed into Hangul form.
	 * This means, that character sequences of form LV and LVT are 
	 * eliminated and replaced with hangul syllables (L is leading Jamu
	 * consonant, V is for vocal and T is for trailing consonant).  
	 * 
	 * The idea behind the analyzer is that it produces up to 3 alternative 
	 * tokens for each hangul syllabic. All of these alternatives are returned
	 * to be located at the same position. Let's have some Hangul syllabic H1
	 * consisting of Jamu characters so that H1=J1J2J3. If H2=J1J2, then first
	 * returned token is H1, second token is H2 and third token is J1. This 
	 * means, that when user enters H1, H2 or J1, the term H1 will be found. 
	 * Also, if user enters J1J2J3 or J1J2, term will be found, because
	 * J1J2J3 is automatically turned to H1 and J1J2 is turned to H2.
	 * 
	 * NOTE: This analyzer MUST NOT be used, when searching, because 
	 * CLuceneQueryParser will break, when it faces tokens with zero 
	 * increment. Use KoreanQueryAnalyzer for searching material indexed
	 * with this analyzer. 
	 */
    class KoreanTokenizer : public lucene::analysis::Tokenizer {
   
        public:
            
			/** Used to read from buffer */
            typedef tiny::cl::ReaderBuffer<512>::iterator buffer_iterator;

            /** Turns utf16 code points into unicode */
            typedef tiny::Utf16Iterator<buffer_iterator> utf16_iterator;
            
            /** Turns Jamu alphabets into Hangul syllables */
            typedef tiny::HangulIterator<utf16_iterator> iterator;
            
            KoreanTokenizer(lucene::util::Reader* reader);
            
            virtual bool next(lucene::analysis::Token* token);

        private:
            
            /** Jamu form of last consumed hangul syllable */
            wchar_t jamu_[4];
            
            /** offsets of last consumed hangul syllable  */
            int begin_, end_;
            
            /**
             * The amount of jamu characters left in buffer. 
             * If this is non-zero, hangul syllable is being processed. 
             */
            int state_;
            
            /**
             * Tiny CJK tokenizer is used to construct 1-grams out of 
             * chinese and japanese characters and to turn latin script
             * into terms.
             */
            TinyCjkTokenizer<iterator> t_;
            
            /** 512 byte buffer for storing characters read with reader */
            tiny::cl::ReaderBuffer<512> in_;
            
            /** 
             * Reads utf16 from in_ buffer, turns it into unicode and 
             * then composes jamu alphabets into hangul syllables. 
             */
            iterator i_;
            
    
    };
    
    /** Korean tokenizer plus lowercase filter */
    typedef TemplateAnalyzer1F<KoreanTokenizer, lucene::analysis::LowerCaseFilter> 
        KoreanAnalyzer;
    
    /**
     * Turns Jamu characters into Hangul syllables and generates 1-grams for
     * all Chinese, Korean and Japanese text. 
     */
    class KoreanQueryTokenizer : public lucene::analysis::Tokenizer {
   
		public:
    
		    /** Used to read from buffer */
			typedef tiny::cl::ReaderBuffer<512>::iterator buffer_iterator;

			/** Turns utf16 code points into unicode */
            typedef tiny::Utf16Iterator<buffer_iterator> utf16_iterator;
            
            /** Turns Hangul syllables into Jamu alphabets */
            typedef tiny::HangulIterator<utf16_iterator> iterator;
            
		public:
       
            KoreanQueryTokenizer( lucene::util::Reader* reader );
            
            virtual bool next( lucene::analysis::Token* token );            
    
		private:

            /** Buffer for storing characters read with reader */
            TinyCjkTokenizer<iterator> t_;
       
            /** Buffer for storing characters read with reader */
            tiny::cl::ReaderBuffer<512> in_;
            
            /** 
             * Reads utf16 from in_ buffer, turns it into unicode and 
             * then composes jamu alphabets into hangul syllables. 
             */
            iterator i_; 
           
    };

    /** Korean query analyzer plus lowercase filter */
    typedef TemplateAnalyzer1F<KoreanQueryTokenizer, lucene::analysis::LowerCaseFilter> 
        KoreanQueryAnalyzer;

}

#endif /* KOREANANALYZER_H_ */
