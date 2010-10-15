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

#include "koreananalyzer.h"
#include "tinyanalysis.inl"

namespace analysis {

    KoreanTokenizer::KoreanTokenizer(lucene::util::Reader* reader) :
        begin_(0),
        end_(0),
        state_(0),
        t_(1), 
        in_(*reader),
        i_(iterator(utf16_iterator(in_.begin()))) {}

    bool KoreanTokenizer::next(lucene::analysis::Token* token) {
        using namespace unicode;
        using namespace tiny;
         
        if ( state_ ) {
            jamu_[state_--] = '\0';
            const wchar_t buf[] = { ComposeJamu(jamu_), '\0' }; 
            token->set( buf, begin_, end_);
            token->setPositionIncrement(0);
            return true;
        } else {
            while ( *i_ ) {
                if ( IsHangulSyllable( *i_ ) ) {
                    DecomposeHangul( IteratorOutput<wchar_t*>(jamu_), *i_ );
                    state_ = wcslen(jamu_)-1;
					wchar_t buf[] = {*i_, '\0'};
                    begin_ = i_;
                    end_ = ++i_;
                    token->set( buf, begin_, end_ );
                    return true;
                } else {
                    Token<iterator> t = t_.consume( i_ );
                    if ( t ) {
                        t.copyTo(token);
                        return true;
                    }
                }
                ++i_;
            }
            return false;
        }
    }
    
	
	KoreanQueryTokenizer::KoreanQueryTokenizer( lucene::util::Reader* reader ) 
    : lucene::analysis::Tokenizer( reader ),
       t_( 1 ),
       in_( *reader ), 
       i_( utf16_iterator( buffer_iterator( in_ ) ) ) {}
	
    bool KoreanQueryTokenizer::next( lucene::analysis::Token* token ) {
        using namespace tiny;
        
        Token<iterator> t = t_.consume(i_);
        if ( t ) {
            t.copyTo( token );
            return true; 
        } 
        return false; 
    }   
}
