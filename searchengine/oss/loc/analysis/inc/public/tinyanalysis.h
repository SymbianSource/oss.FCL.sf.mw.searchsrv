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

#ifndef TINYANALYSIS_H_
#define TINYANALYSIS_H_

#include <string>
#include <sstream>

#include "tinyutf16.h"
#include "wctype.h"

/*
 * This file contains template based tokenization utilities. There 
 * are following rationales for this package: 
 * 
 *    * More flexibility was needed for various CJK analyzers.
 *     
 *       -> CLucene tokenizers are difficult to make work 
 *          together well. For example in practice you cannot using 
 *          generic n-gram tokenizer for cjk and standard tokenizer
 *          for non-cjk. This cannot be done in CLucene without
 *          making it very, very heavy operation. 
 *          
 *    * More flexibility was needed on the character reading level.
 *    
 *        * It is possible to encounter over unicodes that don't fit in 
 *          16 bit characters, when dealing with Chinese and Japanese. 
 *          For this reason, reading CJK should be done in unicode mode
 *          instead of reading individual 16 bit codepoints.
 *           
 *        * Also with Korean, there is alphabetic (Hangul Jamu) and 
 *          syllabic writing form (Hangul Syllables). Same text can be 
 *          expressed in either of these forms. For good behavior (and 
 *          some UX reasons), it was necessary to convert all encountered 
 *          text into one of these forms, so that text written in Jamu 
 *          could be found with Hangul Syllables and visa versa.
 * 
 * This package fulfills both of these requirements in a very speed
 * efficient way. Tokenizers can be easily combined to form a sort of 
 * 'aggregated tokenizer'. This kind of combination is supported by design
 * and done with PairTokenizer class. 
 *  
 * The ability to switch the way text is read on fly is supported by
 * having the reading done by rather abstract iterators. 
 * 
 * Performance is taken into account by having heavily used iterators
 * resolved run-time by making it a template parameter. Lot of inlines
 * are used, but perhaps biggest optimization of it all is that instead
 * extracted tokens holding the string inside, tokenizers simply hold
 * references (in a form of an iterator) into the original character 
 * buffer. So there is no heap usage, look-ups or string copying.
 * 
 * NOTE: Iterators may be surprisingly big objects. While wchar_t*
 * is only 4 bytes, e.g. HangulIterator<Utf16Iterator<ReaderBuffer<N>>>
 * is already 24 bytes. This size could be reduced to 8 bytes, but
 * it would bring performance implications. So copying of iterators
 * may be expensive. 
 * 
 * The design shown in here is actually very nice, flexible, simplistic, 
 * fast and uses very little memory. The same design could be used
 * e.g. for lexical analysis code. 
 */

namespace lucene {
    namespace analysis {
        class Token;
    }
}

namespace analysis {

	
    namespace tiny {

		/**
		 * Token is object, which identifies some sequence of characters in
		 * the original text stream. Holds iterator to the beginning of the 
		 * token and holds information of the tokens length. The length 
		 * is always the amount of unicode characters in the token. 
		 */
		template <typename Iterator>
		struct Token {
			
			typedef RangeIterator<Iterator> iter;
			
			Token() : begin_(), length_() {}
			Token(Iterator& begin, int length) : begin_(begin), length_(length) {}
			
			/** Length in unicode characters */
            inline int length() { return length_; };
            
            /** Gives iterator, that iterates over this token's characters */
            iter iterator() {
				return iter(begin_, length_); 
			}
            /** Informs, whether this token is non-empty */
			operator bool() {
				return length_;
			}
			/** Text size in 16 bit codewords */
			int utf16size() {
				return analysis::tiny::utf16size(iterator()); 
			}
			/** Copy text as 16 bit codewords */
            void utf16(wchar_t* buf) {
                Utf16Writer<wchar_t*>(buf)<<iterator()<<L'\0';
            }
			/** Copy text as 16 bit codewords */
            std::wstring utf16() {
                return utf16str(iterator());
            }
            /** Copy this token content to the Clucene token.*/
            void copyTo(lucene::analysis::Token* token);
		private: 
			Iterator begin_;
			int length_; 
		};

		typedef int (*Acceptor)(int c); 
		
		/** Skips all characters, that are accepted by the acceptor */
		template <class Iterator, typename Acceptor> 
		inline int skip(Iterator& i, Acceptor accept) {
			int ret = 0; 
			while ( *i && accept( *i ) ) { ++i; ret++; }
			return ret; 
		}

		/** Skips all characters, that are not accepted by the acceptor */
		template <class Iterator, typename Acceptor> 
		inline int skipbut(Iterator& i, Acceptor accept) {
			int ret = 0; 
			while ( *i && !accept( *i ) ) { ++i; ret++; }
			return ret; 
		}

		/** Consumes a token consisting of all characters accepted by the acceptor */
		template <class Iterator, typename Acceptor> 
		Token<Iterator> consume(Iterator& i, Acceptor accept) {
			Iterator begin = i;
			return Token<Iterator>( begin,  skip(i, accept) ); 
		}
		
		/** Abstract base class for tokenizers */
        template <class Iterator>
		class Tokenizer {
            public:
                virtual void reset() {};
                virtual Token<Iterator> consume(Iterator& i) = 0;
		};

		/** Consumes as accepted by the acceptor */
		template <class Iterator>
		class CustomTokenizer : public Tokenizer<Iterator> {
			public:
				CustomTokenizer(Acceptor accept) : accept_(accept) {}
				Token<Iterator> consume(Iterator& i) {
					return ::analysis::tiny::consume(i, accept_);  
				}
			private: 
				Acceptor accept_; 
		};
		
		/** 
		 * NGram tokenizer. Tokenizers NGram from any character sequence accepted 
		 * by acceptor. This class maintains internal state. It consumes either
		 * fully sized ngrams or entire word, if the word is smaller than defined
		 * ngram size. 
		 */
		template <class Iterator>
		class NGramTokenizer : public Tokenizer<Iterator> {
			public:
				NGramTokenizer(int size, Acceptor accept) : size_(size), accept_(accept), continue_(false) {}
				NGramTokenizer(int size) : size_(size), accept_(&iswalpha) {}
				void reset() { continue_ = false; }
				Token<Iterator> consume(Iterator& i) {
					if ( *i ) {
						Iterator end = i;
						int l = 0;
						while (l < size_ && *end && accept_( *end )) { l++; ++end; }
						if (l == size_ || (!continue_ && l)) {
							// properly sized token or whole word
							Token<Iterator> t(i, l);
							continue_ = true; 
							++i;
							return t;
						} 
					}
					continue_ = false;
					return Token<Iterator>(i, 0); 
				}
			private: 
				int size_; 
				Acceptor accept_;
				bool continue_;
		};

		/**
		 * Tokenizer, that returns ALWAYS a token, unless EOS is 
		 * reached. If the tokenizer given to this tokenizer fails, 
		 * relaxed tokenizer just moves one position further and 
		 * tries again. 
		 */
	    template <typename I> 
	    class RelaxedTokenizer : public Tokenizer<I> {
            public: 
				/** Uses given tokenizer to extract tokens.  */
                RelaxedTokenizer(Tokenizer<I>& t) : t_(t) {}
                void reset() {t_.reset();}
                /** 
                 * Always returns a token. If tokenization fails,
                 * moves forward a character and tries again. 
                 */
                Token<I> consume(I& i) {
                    Token<I> t;
                    while (*i && !t) {
                        t = t_.consume(i);
                        if (!t) {
                            ++i; t_.reset();
                        }
                    }
                    return t;
                }
            private: 
                Tokenizer<I>& t_;
	    };
	    
	    /**
	     * Tries to first tokenize with the first tokenizer, but if it 
	     * fails, the second tokenizer is tried. If first tokenizer fails, 
	     * it is reset.  
	     */
		template <typename I> 
		class PairTokenizer : public Tokenizer<I>{
		    public:
		        PairTokenizer(Tokenizer<I>& t1, Tokenizer<I>& t2) : t1_(t1), t2_(t2) {}
		        void reset() {
                    t1_.reset();
                    t2_.reset();
                }
	            /**
	             * Attempts to tokenizer with first tokenizer, then 
	             * with second. If both tokenizers fail, empty 
	             * token is returned. 
	             */
	 		    Token<I> consume(I& i) {
                    Token<I> t( t1_.consume( i ) );
                    if ( !t ) {
                        t1_.reset(); 
                        t = t2_.consume( i );
                    }   
                    return t;
                }
		    private:
                Tokenizer<I>& t1_; 
                Tokenizer<I>& t2_;
		};

	}
    
}

#endif /* TINYTOKENIZER_H_ */
