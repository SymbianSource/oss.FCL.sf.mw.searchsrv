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


#ifndef PREFIXQUERYPARSER_H_
#define PREFIXQUERYPARSER_H_

#include <memory>

#include "queryparser.h"

#include "cpixmaindefs.h"

namespace lucene {
	namespace analysis {
		class Token;
	}
	namespace search {
		class Query; 
	}
}
namespace Cpt {
	namespace Lex {
		class Token; 
	}
}

namespace Cpix {

	/**
	 * Cpix special query parser.
	 *
	 * Potential parameters 
	 *  
	 *    * Target Field (makes sense)
	 *    * QueryAnalyzer (doesn't make sense)
	 *    * PrefixAnalyzer (doesn't make sense) 
	 */
	class PrefixQueryParser : public IQueryParser {
			
		public:
			
			PrefixQueryParser(const wchar_t* field = LCPIX_DEFAULT_FIELD); 
			
			virtual ~PrefixQueryParser(); 
		
			virtual std::auto_ptr<lucene::search::Query> parse(const wchar_t* query);
			
			virtual const wchar_t* getField() const;
			
			virtual void setDefaultOperator(cpix_QP_Operator op);
			
		private:
		
			std::auto_ptr<lucene::search::Query> toQuery(Cpt::Lex::Token word);
			
			bool usePrefixFor(lucene::analysis::Token& token);
			
		private: 
			
			std::wstring field_; 

	};
	
}

#endif /* PREFIXQUERYPARSER_H_ */
