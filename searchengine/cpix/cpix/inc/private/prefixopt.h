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

#ifndef PREFIXOPT_H_
#define PREFIXOPT_H_

#include <string>
#include <wchar.h>

// Forward declarations
namespace lucene { 
	namespace search {
		class Query;  
	}
}

namespace Cpix 
{

	/**
	 * Goes throught the query hiearchy and converts queries of 
	 * form "field:c*" into form "field_prefix:c".
	 * 
	 */
	class PrefixOptQueryRewriter
	{
		public:
		
			PrefixOptQueryRewriter(int maxPrefixLength, 
								   const wchar_t* optimizedField, 
								   const wchar_t* prefixField);
			
			~PrefixOptQueryRewriter();
		
			std::auto_ptr<lucene::search::Query>
				rewrite(std::auto_ptr<lucene::search::Query> query);
			
		private: 
			
			int maxPrefixLength_; 
			
			std::wstring optimizedField_; 
			
			std::wstring prefixField_;
			
	};

}

#endif /* PREFIXOPT_H_ */
