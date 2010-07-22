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

#ifndef TINYITERATOR_H_
#define TINYITERATOR_H_

#include <exception>

namespace lucene {
    namespace util {
        class Reader; 
    }
}
namespace analysis {

	 /**
	  * This package provides basic 
	  */
     namespace tiny {
      
        /*
         *
         * Meta code for describing the iterator concept used here.
         * Used to iterate character streams. 
         * Follows closely the STL forward iterator.
         * Note: Comparisons of form x < y may not work properly.
         * Note: x - y will not provide distance in characters, 
         * but instead offset distance in the original text.  
         * 
         * Only operators of form ++i is provided. --i is not provided
         * because complications with utf16 or unicode decomposition/ 
         * composition. i++ would lead to innecessary code.
         * 
         
        concept Iterator {
           
		   // Accessor to the iterator character
           wchar_t operator*();     // 16 bit unicode
           OR
           int operator*();         // 32 bit unicode
       
           // Next location
		   Iterator& operator++();
		   
		   // Returns offset in the original text. Note that one character 
		   // may be transformed into a number of characters. This means 
		   // that comparisons of form (int)i < (int)j are unreliable and
		   // should not be used. Also lengths i - j are unreliable. With korean 
		   // i - j may be produce length 2, but iterating for (;i<j;++i); may
		   // iterate throught e.g. 6 characters. Or, with 32 bit unicode,
		   // length 2 may contain only one character. 
           operator int(); 
        
        };
         
         */
     
		
        /**
         * Iterates throught some T that provides array/pointer like interface
         */
        template <typename T>
        struct ArrayIterator {
            public:
                inline ArrayIterator(T& array, int i) : array_(&array), i_(i) {}
                inline ArrayIterator(T& array) : array_(&array), i_(0) {}
                inline ArrayIterator() : array_(0), i_(0) {}
                
                inline wchar_t operator*() const { return (*array_)[i_]; };  
                inline ArrayIterator<T>& operator++() { i_++; return *this; };  
                inline operator int() { return i_; }
            private: 
                T* array_;
                int i_;
                
        };

        /**
         * Iterates from starting position to up to length characters. 
         */
        template <typename Iterator>
        struct RangeIterator {
            public:
                RangeIterator(Iterator& begin, int length) : i_(begin), left_(length) {}
                inline int operator*() { return left_ ? *i_: '\0'; }
                inline RangeIterator& operator++() { 
                    if (left_ ) { 
                        ++i_; left_--;
                    }
                    return *this;
                }
                inline operator int() { return i_; }
            private: 
                Iterator i_;
                int left_; 
        };

        /**
         * Turns iterator into a C++ stream. Allows out<<'c'<<'\0'; 
         * kind of syntax to be used with iterators.  
         */
        template <typename Iterator>
        struct IteratorOutput {
            public:
                IteratorOutput(Iterator i) : i_(i) {}
                template <typename T>
                inline IteratorOutput& operator<<(T t) {
                    *i_= t;
                    ++i_;
                    return *this;
                }
            private:
                Iterator i_;
        };
        
        /**
         * CLucene IO support
         */
        namespace cl {
        
			/**
			 * Informs that caller has attempted to read a location
			 * from the reader source, that is no more stored in the 
			 * buffer.
			 */
			class TooOldIndexException : public std::exception {
            public: 
                const char* what() const;
            };
            
            /**
             * Provides buffer & array like interface to be used with
             * CLucene readers. If reader r provides access to file X, 
             * and we have buf(r), we can sort of 'random access' file 
             * X with buf[0], buf[X], buf[Z+3] syntaxes. Still, the buffer
             * is of limited size. There is always the most recent location
             * L that is read. Trying to access buf[L-SIZE-1] will raise 
             * exception, where SIZE is the buffer size. 
             */
            template<int SIZE>
            class ReaderBuffer {
                public:
					/** Iterator for iterating the underlying source */
                    typedef ArrayIterator<ReaderBuffer> iterator;
                    /* Constructs buffer for a reader reading some source. */
                    ReaderBuffer(lucene::util::Reader& reader);
                    /** Returns character at location i  */
                    wchar_t operator[](int i);
                    /** Returns iterator pointing to location i */
                    inline iterator at(int i);
                    /** Returns iterator pointing to the beginning of character source */
                    inline iterator begin();
                private:
                    /** Rotating buffer. */
                    wchar_t buf_[SIZE]; 
                    /** How many characters have been read from reader */
                    int read_; 
                    /** Points to the next character to be overwritten in buffer */
                    int cut_; 
                    /** Index of oldest character inside the original source */
                    int offset_; 
                    /** Reader reading original source */
                    lucene::util::Reader& reader_;
            };
            
        }

    }

}

#endif /* TINYITERATOR_H_ */
