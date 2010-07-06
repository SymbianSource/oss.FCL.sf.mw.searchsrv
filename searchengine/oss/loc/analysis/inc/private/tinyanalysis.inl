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
#ifndef TINYANALYSIS_INL_
#define TINYANALYSIS_INL_

#include "CLucene.h"

/*
 * TODO: 
 * 
 *   Move more functionality in here
 *   
 */

namespace analysis {
    namespace tiny {
        namespace cl {
    
            template<int SIZE>
            wchar_t ReaderBuffer<SIZE>::operator[](int i)
            {
                if ( i < read_ - SIZE ) throw TooOldIndexException();
                // fill
                while (i >= read_) {
                    int c = reader_.read(); 
                    buf_[cut_++] = ( c == -1 ? '\0' : c );
                    cut_ %= SIZE;
                    if (read_++ >= SIZE) {
                        offset_++;
                    } 
                }
                // guaranteed that i < read
                return buf_[i % SIZE]; 
            }
           
            template<int SIZE>
            ReaderBuffer<SIZE>::ReaderBuffer(lucene::util::Reader& reader)
            : reader_(reader), read_(0), cut_(0), offset_(0) {}
            
            template<int SIZE>
            typename ReaderBuffer<SIZE>::iterator ReaderBuffer<SIZE>::at(int i) {
                return  iterator(*this, i); 
            }
            
            template<int SIZE>
            typename ReaderBuffer<SIZE>::iterator ReaderBuffer<SIZE>::begin() { 
                return iterator(*this, 0); 
            }
            
        }
        
        template <typename Iterator>
        void Token<Iterator>::copyTo(lucene::analysis::Token* token) {
            token->resetTermTextLen();
            token->growBuffer(utf16size()+1);
            token->setPositionIncrement(1);
            Utf16Writer<wchar_t*> out(token->_termText); 
            Iterator i = begin_; 
            token->setStartOffset(i);
            for (int n = length_; n; n--) {
				out<<*i; ++i; 
            }
            out<<L'\0';
            token->setEndOffset(i);
        }

    }
}

#endif /* TINYANALYSIS_INL_ */
