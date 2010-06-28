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


#ifndef QUERYPARSER_H_
#define QUERYPARSER_H_

#include <memory>

#include "CLucene.h"
#include "CLucene/queryParser/MultiFieldQueryParser.h"

#include "prefixopt.h"

#include "cpixsearch.h"

namespace Cpix {

	class IQueryParser {
		
		public:
		
			virtual ~IQueryParser();
		
			virtual std::auto_ptr<lucene::search::Query> parse(const wchar_t* query) = 0;
			
			// Not really a fundamental property of a query parser:
			virtual const wchar_t* getField() const = 0;
			
			virtual void setDefaultOperator(cpix_QP_Operator op) = 0;
	};

	class CLuceneQueryParser : public IQueryParser {
		
		public: 
		
			~CLuceneQueryParser();
	
			CLuceneQueryParser(const wchar_t* field, lucene::analysis::Analyzer& analyzer);
			
			virtual std::auto_ptr<lucene::search::Query> parse(const wchar_t* query);

			virtual const wchar_t* getField() const;
			
			virtual void setDefaultOperator(cpix_QP_Operator op);

		private: 
			
			std::auto_ptr<lucene::queryParser::QueryParser> parser_;  
		
	};
	
	class CLuceneMultiFieldQueryParser : public IQueryParser {
		
		public: 
		
			~CLuceneMultiFieldQueryParser();
	
			CLuceneMultiFieldQueryParser(const wchar_t** fields, 
										 lucene::analysis::Analyzer& analyzer, 
										 lucene::queryParser::BoostMap& boostMap);
			
			virtual std::auto_ptr<lucene::search::Query> parse(const wchar_t* query);
			
			virtual const wchar_t* getField() const;
			
			virtual void setDefaultOperator(cpix_QP_Operator op);
			
		private: 
			
			std::auto_ptr<lucene::queryParser::QueryParser> parser_;  
		
	};
	

	class PrefixOptQueryParser : public IQueryParser {
		
		public:
		
			PrefixOptQueryParser(std::auto_ptr<IQueryParser> parser);
			
			~PrefixOptQueryParser(); 
			
			virtual std::auto_ptr<lucene::search::Query> parse(const wchar_t* query);

			virtual const wchar_t* getField() const;
			
			virtual void setDefaultOperator(cpix_QP_Operator op);

		private:
			
			PrefixOptQueryRewriter prefixOpt_; 
			
			std::auto_ptr<IQueryParser> parser_;
		
	};
	
	// 
	// Following factory methods apply necessary optimization wraps
	// over the query parsers.
	// 
	
	
	IQueryParser* CreateCLuceneQueryParser(const wchar_t* defaultField, 
									       lucene::analysis::Analyzer* analyzer);
	
	IQueryParser* CreateCLuceneMultiFieldQueryParser(
											const wchar_t* fields[], 
											lucene::analysis::Analyzer* analyzer, 
											lucene::queryParser::BoostMap* boostMap);
	
	IQueryParser* CreatePrefixQueryParser(const wchar_t* field); 

}

#endif /* QUERYPARSER_H_ */
