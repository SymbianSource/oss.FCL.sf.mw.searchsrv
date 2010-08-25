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


#include "CLucene.h"

#include "cpixmaindefs.h"

// internal libs
#include "cpixparsetools.h"

// internal
#include "analyzer.h"

#include "prefixqueryparser.h"

#include "cpixanalyzer.h"
#include "cluceneext.h"

#include "tinyunicode.h"

#include "cpixexc.h"

namespace Cpix {
	
	using namespace lucene::analysis; 
	using namespace lucene::search; 
	using namespace lucene::document; 
	using namespace lucene::util; 
	using lucene::index::Term; 
	using namespace std; 

	namespace {
	
		/**
		 * Small optimization to avoid creating extra boolean queries
		 */
		class QueryConstructor {
			
		public: 
			QueryConstructor() : q_(), bq_(0) {}
			
			auto_ptr<Query> operator()() {
				return q_; 
			}
			void add(auto_ptr<Query> q) {
				if ( q.get() ) {
					if ( bq_ ) {
						bq_->add( q.release(), true, false, false ); 
					} else {
						if ( q_.get() ) {
							auto_ptr<BooleanQuery> bq( new BooleanQuery() );
							bq_ = bq.get();
							bq_->add( q_.release(), true, false, false ); 
							bq_->add( q.release(), true, false, false ); 
							q_.reset( bq.release() ); 
						} else {
							q_ = q;  
						}
					}
				}
			}
			inline void add(Query* q) {
				add( auto_ptr<Query>( q ) );
			}
	
		private: 
			
			auto_ptr<Query> q_; 
			BooleanQuery* bq_; 
			
		};
		
		/**
		 * TokenStream interface with one modification: 
		 *   * Ability to check if returned token was last one in the stream 
		 */
		class HasNextTokenStream {
			
			public:
			
				HasNextTokenStream(TokenStream* tokens)
				:   i_(true), 
					next_(),
					buf_(),
					tokens_( tokens ){
					next_ = tokens_->next(&buf_[0]);
				}
		
				inline Token& next() {
					next_ = tokens_->next(&buf_[i_]); 
					i_ = !i_;
					return buf_[i_]; 
				}
			
				inline bool hasNext() {
					return next_; 
				}
				
			private:
				bool i_, next_; 
				Token buf_[2]; 
				auto_ptr<TokenStream> tokens_; 
		};
				
	
	}
	
	PrefixQueryParser::PrefixQueryParser(const wchar_t* field) 
	: field_(field) {}
		
	PrefixQueryParser::~PrefixQueryParser() {}
	
	auto_ptr<Query> PrefixQueryParser::parse(const wchar_t* query) {
		Cpt::Lex::WhitespaceSplitter split(query);
		QueryConstructor ret;
		while ( split ) {
			ret.add( toQuery( split++ ) ); 
		}	
		return ret(); 
	}
	
	const wchar_t* PrefixQueryParser::getField() const {
		return field_.c_str(); 
	}
	
	void PrefixQueryParser::setDefaultOperator(cpix_QP_Operator op) {
		THROW_CPIXEXC("Prefix query parser does not support setting the default operator.");  
	}

	bool PrefixQueryParser::usePrefixFor(lucene::analysis::Token& token) {
		return !analysis::unicode::IsCjk(token.termText()[0]);
	}

	auto_ptr<Query> 
		PrefixQueryParser::toQuery(Cpt::Lex::Token word) {
		Analyzer& preAnalyzer( Analysis::getPrefixAnalyzer() ); 
		StringReader reader( word.begin(), word.length() );
		HasNextTokenStream tokens(
			preAnalyzer.tokenStream( field_.c_str(), 
									 &reader ) );

		QueryConstructor ret; 
		
		while ( tokens.hasNext() ) {
			lucene::analysis::Token& token = tokens.next();
			
			if ( usePrefixFor(token) ) {
				if (!tokens.hasNext()) {
					// Turn only last token of this word into prefix query
					ret.add(
						_CLNEW PrefixQuery( freeref( _CLNEW Term( field_.c_str(), 
								                                  token.termText() ) ) ) );  
				} else {
					// Others tokens can be normal term queries
					ret.add( 
						_CLNEW TermQuery( freeref( _CLNEW Term( field_.c_str(), 
															    token.termText() ) ) ) );  
				}
			} else {
				Analyzer& termAnalyzer = Analysis::getQueryAnalyzer();
				StringReader reader( token.termText(), token.termTextLength() );
				HasNextTokenStream tokens(
					termAnalyzer.tokenStream( field_.c_str(), 
											  &reader ) );
				
				Token& first = tokens.next();
				if (tokens.hasNext()) { // more than one
					auto_ptr<PhraseQuery> phrase( _CLNEW PhraseQuery() );
					phrase->add( freeref( _CLNEW Term( field_.c_str(), 
													   first.termText() ) ) ); 
					while (tokens.hasNext()) {
						phrase->add( freeref( _CLNEW Term( field_.c_str(), 
														   tokens.next().termText() ) ) ); 
					}
					ret.add( std::auto_ptr<Query>( phrase.release() ) ); 
				} else {
					ret.add( 
					        _CLNEW TermQuery( freeref( _CLNEW Term( field_.c_str(), 
													                first.termText() ) ) ) );
				}
			}
		}
		return ret(); 
	}

}
