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
#ifndef TINYUTF16_H_
#define TINYUTF16_H_

#include "tinyiterator.h"

namespace analysis {

    namespace tiny {

		/**
		 * Translates given unicode character as utf16 and 
		 * stores utf16 codes in the output stream
		 */
        template <typename Stream>
        void utf16put(Stream& out, int c) {
            if ( c >= 0x00010000L ) {
                c -= 0x00010000L;
                out<<(wchar_t)(0xd800 + ((c >> 10) & 0x03ffL));
                out<<(wchar_t)(0xd800 + (c & 0x03ffL));
            } else {
                out<<(wchar_t)(c);
            }
        }

        /**
         * Writes unicode characters into the output 
         * stream as utf16 codes. 
         */
        template <typename Output> 
        struct Utf16Output {
            public:
                Utf16Output(const Output& out) : out_(out) {};
                inline Utf16Output& operator<<(int c) {
                    utf16put(out_, c);
                    return *this;
                }

                inline Utf16Output& operator<<(wchar_t c) {
                    return (*this)<<(int)c;
                }
                template <typename I>
                Utf16Output& write(I source, int length) {
                    for (int i = 0; i < length; i++) { 
                        (*this)<<source; ++source;
                    }
                    return *this;
                }
                template <typename I>
                Utf16Output& operator<<(I source) {
                     for (;*source; ++source) {
                         (*this)<<*source;
                     }
                     return *this;
                 }
            private:
                Output out_;
        };
        
        /**
         * Writes unicode characters into the given iterator as utf16 codes
         */
        template <typename Iterator> 
        struct Utf16Writer : public Utf16Output<IteratorOutput<Iterator> > {
            public:
                Utf16Writer(Iterator i) : Utf16Output<IteratorOutput<Iterator> >(IteratorOutput<Iterator>(i)) {}
            };
                
        /** 
         * Calculates the size of all characters with the iterator as utf16 
         * code points
         */
        template<typename Iterator>
        int utf16size(Iterator i) {
            int rv = 0;
            for (;*i; ++i) {
                rv += (*i >= 0x10000 ? 2 : 1);
            }
            return rv;
        }
        
        /**
         * Reads utf16 code points from given iterator and translates them 
         * as unicode characters.  
         */
        template <typename Iterator> 
        struct Utf16Iterator {
            public:
                Utf16Iterator(Iterator i) : i_(i) { 
                    operator++(); // cache first character
                }
                Utf16Iterator() : i_(), c_(0), offset_(0) {}
                inline int operator*() const {
                    return c_;
                };  
                Utf16Iterator& operator++() {
                    offset_ = i_;
                    c_ = *i_; ++i_;
                    if ( c_ >= 0xd800 && c_ <= 0xdfff ) {
                        int c2 = *i_; ++i_;
                        if ( c2 >= 0xdc00 && c2 <= 0xdfff ){
                            c_ = (((c_ & 0x03ffL) << 10) | ((c2 & 0x03ffL) << 0)) + 0x00010000L;
                        }
                    }
                    return *this;
                }
                operator int() {return offset_;}
            private:
                Iterator i_;
                int c_; // current utf cached
                int offset_; // characters read
        };
        
        /**
         * Copies the iterator content into a wstring
         */
        template<class Iterator> 
        std::wstring utf16str(Iterator i) {
            std::wostringstream ret;
            while (*i) {
                utf16put(ret, *i);
                ++i;
            }
            return ret.str();
        }
        
    }
}

#endif /* TINYUTF16_H_ */
