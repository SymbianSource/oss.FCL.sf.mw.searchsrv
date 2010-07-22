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

#include "prefixopt.h"
 
#include "CLucene.h"

#include "cpixsearch.h"

#include "cpixstrtools.h"

#include "cluceneext.h"

namespace Cpix {

	using namespace lucene::search;

	using namespace lucene::index;
	
	PrefixOptQueryRewriter::PrefixOptQueryRewriter(
			int maxPrefixLength, 
			const wchar_t* optimizedField, 
			const wchar_t* prefixField )
	: 	maxPrefixLength_( maxPrefixLength ),
		optimizedField_( optimizedField ),
		prefixField_( prefixField )
	{}


	PrefixOptQueryRewriter::~PrefixOptQueryRewriter()
	{}
	

	std::auto_ptr<Query> PrefixOptQueryRewriter::rewrite(auto_ptr<Query> query)
	{
		PrefixQuery* wildq = 
			dynamic_cast<PrefixQuery*>( query.get() );

		if ( wildq )
		{
			Term* term = wildq->getPrefix(false);
			
			if ( optimizedField_ == term->field() )
			{
				int length = wcslen( term->text() );
				if ( length != -1 && length <= maxPrefixLength_ )
				{
					// rewrite term
					std::wstring text( term->text() ); 
					text = text.substr(0, length); 
					term = lucene::util::freeref( _CLNEW Term(prefixField_.c_str(), text.c_str()) );
				
					// rewrite query
					std::auto_ptr<Query> ret( new TermQuery( term ) );
					query.reset(); // delete old
					return ret; // return ownership 
				}
			}
		}
		BooleanQuery* boolq = 
			dynamic_cast<BooleanQuery*>( query.get() ); 
		
		if ( boolq ) 
		{
			// Just modify the query
			Cpt::auto_array<BooleanClause*> clauses( new BooleanClause*[boolq->getClauseCount() + 1]);  
			
			boolq->getClauses( clauses.get() );
			
			for ( int i = 0; i < boolq->getClauseCount(); i++ ) 
			{
				// Transfer ownership to stack
				std::auto_ptr<Query> clauseq(
					clauses.get()[i]->query );
				clauses.get()[i]->query = 0;
				
				// Rewrite and restore ownership
				clauseq = rewrite( clauseq );
				clauses.get()[i]->query = clauseq.release(); 
			}
		}
		
		return query; // return ownership 
	}
	
}
