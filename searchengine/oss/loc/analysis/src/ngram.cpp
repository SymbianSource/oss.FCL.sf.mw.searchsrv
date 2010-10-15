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


#include "ngram.h"
#include "tinyunicode.h"
#include "tinyanalysis.inl"


namespace analysis {

	using namespace unicode; 

	int IsNonCjk(int c) {
		return iswalnum(c) && !IsCjk(c); 
	}

	CjkNGramTokenizer::CjkNGramTokenizer( 
			lucene::util::Reader* reader, 
			int gramSize ) 
		: lucene::analysis::Tokenizer(reader),
          t_( gramSize ),
		  in_( *reader ),
	      i_( buffer_iterator( in_ ) ){
	}
		
	bool CjkNGramTokenizer::next( lucene::analysis::Token* token ) {
		using namespace tiny;

		Token<iterator> t = t_.consume(i_);
        if ( t ) {
            t.copyTo( token );
            return true; 
        } 
		return false; 
	}
	
	JamuNGramTokenizer::JamuNGramTokenizer( lucene::util::Reader* reader, 
                                            int gramSize ) 
    : lucene::analysis::Tokenizer( reader ),
       t_( gramSize ),
       in_( *reader ), 
       i_( utf16_iterator( buffer_iterator( in_ ) ) ) {}
	
    bool JamuNGramTokenizer::next( lucene::analysis::Token* token ) {
        using namespace tiny;
        
        Token<iterator> t = t_.consume(i_);
        if ( t ) {
            t.copyTo( token );
            return true; 
        } 
        return false; 
    }
} 
