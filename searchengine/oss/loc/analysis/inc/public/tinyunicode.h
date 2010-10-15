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
#ifndef UNICODEINFO_H_
#define UNICODEINFO_H_

#include "tinyiterator.h"

namespace analysis {

	/**
	 * The package contains various unicode related functionality as
	 * needed by the remaining analysis package
	 */
	namespace unicode {
	
		/** Returns true, if character c is either Hangul Jamo or Syllable */
        int IsHangul(int c);
        
        /** Returns true, if character c is Hangul Syllable */
        int IsHangulSyllable(int c);
        
        /** Returns true, if character c is Hangul Jamo */
        int IsHangulJamo(int c);
        
        /** 
         * Returns true, if character is of either Chinese, 
         * Japanese or Korean writing systems
         */
        int IsCjk(int c);
        
        /**
         * Returns true, if characters is on the Thai unicode block
         */
        int IsThai(int c);
        
        namespace hangul {
        
			/* First Hangul Syllable code */
            static const int SyllableBase = 0xAC00;
            
            // Jamu Alphabets
            /** First leading Jamu consonant */
            static const int LeadingBase = 0x1100;
            
            /** First vowel  */
            static const int VowelBase = 0x1161;
            
            /** First trailing Jamu consonant */
            static const int TrailingBase = 0x11A7;
            
            /** Leading consonants count */
            static const int LeadingCount = 19;
            
            /** Vowel count */
            static const int VowelCount = 21;
            
            /** Trailing consonant count */
            static const int TrailingCount = 28;
            
            /** 
             * Amount of syllables that are composed of a leading 
             * consonant and a vowel 
             */
            static const int LvSyllableCount = VowelCount * TrailingCount; // 588
            
            /**
             * Amount of syllables that are composed of a leading
             * consonant, a vowel and a trailing consonant 
             */
            static const int SyllableCount = LvSyllableCount * TrailingCount; // 11172 
        }
        
        /**
         * Decomposes hangul syllable into jamu alphabets
         */
        template<typename Output>
        void DecomposeHangul(Output out, int c) {
            using namespace hangul;
            int sindex = c - SyllableBase;
            if (sindex < 0 || sindex >= SyllableCount) {
                out<<c<<'\0';
            } else {
                // Leading
                out<<(LeadingBase + sindex / LvSyllableCount);
                // Vocal
                out<<(VowelBase + (sindex % LvSyllableCount) / TrailingCount);
                // Trailing (voluntary)
                int toffset = sindex % TrailingCount;
                if (toffset) out<<(TrailingBase + toffset);
                // Finish
                out<<'\0';
            }
        }
        
        /**
         * Composes encountered jamu alphabets into hangul syllable.
         * Moves given iterator over the consumed unicode character.
         */
        template<typename Iterator> 
        int ConsumeComposedJamu(Iterator& i) {
            using namespace hangul;
            int c = *i; ++i;
            int lindex = c - LeadingBase;
            if (0 <= lindex && lindex < LeadingCount) {
                int vindex = *i - VowelBase;
                if (0 <= vindex && vindex < VowelCount) {
                    ++i;
                    int tindex = *i - TrailingBase;
                    c = (SyllableBase + (lindex * VowelCount + vindex) * TrailingCount);
                    if (0 <= tindex && tindex < TrailingCount) {
                        ++i;
                        c += tindex;
                    } 
                }
            }
            return c;
        }
        
        /**
         * Composes encountered jamu alphabets into hangul syllable.
         */
        template<typename Iterator> 
        inline int ComposeJamu(Iterator i) {
        	return ConsumeComposedJamu(i); 
        }        
	}
	
	namespace tiny { // tiny analysis
	
        using namespace analysis::unicode::hangul;
        using namespace analysis::unicode;
	
        /**
         * Composes encountered Hangul Jamu characters into 
         * Hangul syllables.  
         */
        template <typename Iterator> 
        struct HangulIterator {
            public:
                HangulIterator() : i_(), c_(), offset_(0) {}
                HangulIterator(Iterator i) : i_(i) {
                    ++(*this); // populate c_
                }
                int operator*() {
                    return c_;
                }
                operator int() {
                    return offset_;
                }
                HangulIterator& operator++() {
                    offset_ = i_;
                    c_ = ConsumeComposedJamu(i_);
                    return *this;
                }
            private:
                Iterator i_;
                int c_;
                int offset_;
        };
        
        /**
         * Decomposes encountered Hangul syllables into 
         * Hangul Jamu characters  
         */
        template <typename Iterator> 
        struct JamuIterator {
            public:
                JamuIterator() : i_(), b_(0), offset_(0) { buf_[0];}
                JamuIterator(Iterator i) : i_(i), b_(0) {
                    buf_[1] = '\0';
                    ++(*this); // populate buffer
                }
                int operator*() {
                    return buf_[b_];
                }
                JamuIterator& operator++() {
                    offset_ = i_;
                    if (!buf_[++b_]) {
                        b_ = 0; // reset buf
                        tiny::IteratorOutput<int*> out(buf_);
                        DecomposeHangul(out, *i_); ++i_;
                    }
                    return *this;
                }
                operator int() {
                    return offset_;
                }
            private:
                Iterator i_;
                int buf_[4];
                int b_;
                int offset_;

        };
	}
}


#endif /* UNICODEINFO_H_ */
