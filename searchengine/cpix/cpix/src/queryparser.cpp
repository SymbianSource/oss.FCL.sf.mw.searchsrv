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


#include "queryParser.h"

#include "cpixmaindefs.h"

#include "initparams.h"

#include "prefixqueryparser.h"

#include "cpixexc.h"

namespace Cpix {
	
	IQueryParser::~IQueryParser() {}
	
	CLuceneQueryParser::CLuceneQueryParser(const wchar_t* defaultField, 
										  lucene::analysis::Analyzer& analyzer) 
	: parser_() {
		parser_.reset(_CLNEW lucene::queryParser::QueryParser(defaultField, &analyzer));
	}
	
	CLuceneQueryParser::~CLuceneQueryParser() {}
	
	std::auto_ptr<lucene::search::Query> CLuceneQueryParser::parse(const wchar_t* query) {
		return std::auto_ptr<lucene::search::Query>( parser_->parse(query) ); 
	}

	const wchar_t* CLuceneQueryParser::getField() const {
		return parser_->getField();
	}
	
	void CLuceneQueryParser::setDefaultOperator(cpix_QP_Operator op) {
 		parser_->setDefaultOperator(static_cast<int>(op)); 
	}

	PrefixOptQueryParser::PrefixOptQueryParser(std::auto_ptr<IQueryParser> parser) 
	:	 
	 	prefixOpt_(OPTIMIZED_PREFIX_MAX_LENGTH, 
				   LCPIX_DEFAULT_FIELD, 
				   LCPIX_DEFAULT_PREFIX_FIELD ),
				           parser_( parser )
	{}
	
	PrefixOptQueryParser::~PrefixOptQueryParser() {}
	
	std::auto_ptr<lucene::search::Query> PrefixOptQueryParser::parse(const wchar_t* query) {
		return prefixOpt_.rewrite( parser_->parse(query) );
	}
	
	const wchar_t* PrefixOptQueryParser::getField() const {
		return parser_->getField();
	}
	
	void PrefixOptQueryParser::setDefaultOperator(cpix_QP_Operator op) {
		parser_->setDefaultOperator(op); 
	}

	CLuceneMultiFieldQueryParser::CLuceneMultiFieldQueryParser(
			const wchar_t** fields, 
			lucene::analysis::Analyzer& analyzer, 
			lucene::queryParser::BoostMap& boostMap)
	: parser_() {
		parser_.reset(
				_CLNEW lucene::queryParser::MultiFieldQueryParser( fields, &analyzer, &boostMap ));
		
	}
	
	CLuceneMultiFieldQueryParser::~CLuceneMultiFieldQueryParser() {}
	
	std::auto_ptr<lucene::search::Query> 
		CLuceneMultiFieldQueryParser::parse(const wchar_t* query) {
		return std::auto_ptr<lucene::search::Query>( parser_->parse( query ) );
		
	}
	
	const wchar_t* CLuceneMultiFieldQueryParser::getField() const {
		THROW_CPIXEXC("Multi field query parser does not support getField operation"); 	
	}
	
	void CLuceneMultiFieldQueryParser::setDefaultOperator(cpix_QP_Operator op) {
		parser_->setDefaultOperator(static_cast<int>(op)); 
	}
	
	IQueryParser* CreateCLuceneQueryParser(const wchar_t* defaultField, 
										   lucene::analysis::Analyzer* analyzer) {
		return 
			new PrefixOptQueryParser(
				std::auto_ptr<IQueryParser>(
					new CLuceneQueryParser(defaultField, *analyzer)));
	}
	
	IQueryParser* CreateCLuceneMultiFieldQueryParser(
											const wchar_t* fields[], 
											lucene::analysis::Analyzer* analyzer, 
											lucene::queryParser::BoostMap* boostMap) {
		return 
			new PrefixOptQueryParser(
				std::auto_ptr<IQueryParser>(
					new CLuceneMultiFieldQueryParser(fields, 
													 *analyzer,
													 *boostMap)));
		
	}
	
	IQueryParser* CreatePrefixQueryParser(const wchar_t* field) {
		return 
			new PrefixOptQueryParser(
				std::auto_ptr<IQueryParser>(
					new PrefixQueryParser(field)));
	}

	
}
