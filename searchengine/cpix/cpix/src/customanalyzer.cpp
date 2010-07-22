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


// system library
#include "wchar.h"
#include <string>
#include <vector>
#include <sstream>
#include <iostream>
#include <glib.h>

// clucene 
#include "CLucene.h"
#include "CLucene/analysis/AnalysisHeader.h"
#include "CLucene/analysis/Analyzers.h"

// local libary
#include "thaianalysis.h"
#include "ngram.h"
#include "koreananalyzer.h"
#include "cjkanalyzer.h"
#include "cpixparsetools.h"
#include "prefixfilter.h"

// cpix internal
#include "customanalyzer.h"
#include "cpixanalyzer.h"
#include "analyzer.h"
#include "cluceneext.h"
#include "analyzerexp.h"
#include "indevicecfg.h"
#include "cpixexc.h"
#include "localization.h"

namespace Cpix {
	
	//
	// Following sections provide the glue code for connecting the 
	// analyzer definition syntax with analyzer, tokenizers and filter 
	// implementations. 
	//
	// The glue code is template heavy with the indent of providing 
	// automation for associating specific keywords with specific
	// analyzers, tokenizers and filters implementing corresponding 
	// CLucene abstractions. Additional classes are needed only if 
	// filters, tokenizers, etc. accept parameters.
	//
	// NOTE: To understand the analyzers, it is sufficient to understand
	// that an analyzer transforms characters stream into specific token streams 
	// (e.g. character stream 'foobarmetawords' can be transformed into token 
	// stream 'foo', 'bar' 'meta' 'words'). Analysis consist of two main
	// parts which are tokenization and filtering. Tokenization converts
	// the character stream into token stream (e.g. 'FoO bAr' -> 'FoO' 'bAr')
	// and filtering modifies the tokens (e.g. lowercase filtering 'FoO' -> 
	// 'foo', 'bAr' -> 'bar'). Analyzer as an object is responsible for
	// constructing a tokenizer and a sequence of filters to perform
	// these required tasks.  
	// 
	// See the documentation around TokenizerClassEntries and 
	// FilterClassEntries to see how implementations not taking parameters
	// can be easily added.  
	// 
	
	using namespace Cpix::AnalyzerExp;
	
// Safe assumption
#define MAX_LANGCODE_LENGTH 256
	
	class LocaleSwitchStreamFactory : public TokenStreamFactory {
	public: 
		
		LocaleSwitchStreamFactory(const AnalyzerExp::LocaleSwitch& sw, const wchar_t* config);
		
		~LocaleSwitchStreamFactory();
		
		virtual lucene::analysis::TokenStream* tokenStream(const wchar_t        * fieldName, 
														   lucene::util::Reader * reader);
		
		lucene::analysis::TokenStream* tokenStream(std::vector<std::wstring>& languages, 
												   const wchar_t            * fieldName, 
												   lucene::util::Reader     * reader);
		
	private: 
		std::map<std::wstring, CustomAnalyzer*> analyzers_;
		std::auto_ptr<CustomAnalyzer> default_;  
	};


	TokenStreamFactory::~TokenStreamFactory() {};

	LocaleSwitchStreamFactory::LocaleSwitchStreamFactory(const LocaleSwitch& sw, const wchar_t* config) {
		for (int i = 0; i < sw.cases().size(); i++) {
			const Case& cs = *sw.cases()[i];
			for (int j = 0; j < cs.cases().size(); j++) {
				std::wstring c = cs.cases()[j]; 
				if (analyzers_.count(c)) delete analyzers_[c]; 
				analyzers_[c] = new CustomAnalyzer(cs.piping(), config);
			}
		}
		default_.reset(new CustomAnalyzer(sw.def())); 
	}
	
	LocaleSwitchStreamFactory::~LocaleSwitchStreamFactory() {
		typedef std::map<std::wstring, CustomAnalyzer*>::iterator iter;
		for (iter i = analyzers_.begin(); i != analyzers_.end(); i++) {
			delete i->second;
		}
	}
		
	lucene::analysis::TokenStream* 
		LocaleSwitchStreamFactory::tokenStream(const wchar_t        * fieldName, 
											   lucene::util::Reader * reader) {
		std::vector<std::wstring> languages = 
				Localization::instance().getLanguageNames();
	 
		return tokenStream(languages, fieldName, reader); 
	}
	
	lucene::analysis::TokenStream* 
		LocaleSwitchStreamFactory::tokenStream(std::vector<std::wstring>& languages, 
											   const wchar_t            * fieldName, 
											   lucene::util::Reader     * reader) {
		for (int i = 0; i < languages.size(); i++) {
			if ( analyzers_.count(languages[i]) ) {
				return analyzers_[languages[i]]->tokenStream( fieldName, reader );
			}
		}		
		return default_->tokenStream( fieldName, reader ); 
	}
	
	class DefaultTokenStreamFactory : public TokenStreamFactory {
	public:
	
		enum Target {
			NORMAL, 
			INDEXING,
			QUERY,
			PREFIX
		};
	
		DefaultTokenStreamFactory(const Invokation& invokation) {
			if (invokation.params().size() == 1) {
				const Identifier* id = dynamic_cast<const Identifier*>( invokation.params()[0] ); 
				if ( id ) {
					if ( id->id() == CPIX_ID_INDEXING ) {
						target_ = INDEXING;
					} else if ( id->id() == CPIX_ID_QUERY ) {
						target_ = QUERY;
					} else if ( id->id() == CPIX_ID_PREFIX ) {
						target_ = PREFIX;
					} else {
						THROW_CPIXEXC(L"Default analyzer does not accept %S for parameter", id->id().c_str());
					}
				} else {
					THROW_CPIXEXC(L"Default accepts only identifier as a parameter.");
				}
			} else if (invokation.params().size() > 1) {
				THROW_CPIXEXC(L"Default analyzer does not accept more than one parameter");
			} else {
				target_ = NORMAL;
			}
		}
		
		virtual lucene::analysis::TokenStream* tokenStream(const TCHAR          * fieldName, 
														   lucene::util::Reader * reader) {
			switch (target_) {
				case QUERY: 
					return Analysis::getQueryAnalyzer().tokenStream( fieldName, reader );
				case PREFIX: 
					return Analysis::getPrefixAnalyzer().tokenStream( fieldName, reader );
			}
			return Analysis::getDefaultAnalyzer().tokenStream( fieldName, reader );
		}
		
	private:
	
		Target target_;
		
	};
		
	/**
	 * Template class used to create CLucene tokenizers. Template
	 * parameter T must implement lucene::analysis::Tokenizer abstraction.  
	 */    
	template<class T>
	class TokenizerFactory : public TokenStreamFactory 
	{
	public:
		TokenizerFactory(const Invokation& invokation) {
			if (invokation.params().size() > 0) {
				THROW_CPIXEXC(L"Tokenizer %S does not accept parameters",
							  invokation.id().c_str());
			}
		}
		virtual lucene::analysis::TokenStream* tokenStream(const TCHAR          * /*fieldName*/, 
														   lucene::util::Reader * reader) {
			return _CLNEW T(reader); 
		}
	};
	
	template<>
    class TokenizerFactory<analysis::CjkNGramTokenizer> : public TokenStreamFactory 
    {   
    public:
        static const int DefaultNgramSize = 1;
        TokenizerFactory(const Invokation& invokation) {
            using namespace Cpix::AnalyzerExp;
            if (invokation.params().size() > 1) {
                THROW_CPIXEXC(L"Cjk Ngram tokenizer does not accept more than one parameter",
                              invokation.id().c_str());
            }
            if (invokation.params().size() == DefaultNgramSize) {
                IntegerLit* ngramSize = dynamic_cast<IntegerLit*>(invokation.params()[0]);
                if ( ngramSize ) {
                    ngramSize_ = ngramSize->value();
                } else {
                    THROW_CPIXEXC(L"Cjk Ngram tokenizer parameter must be an integer");
                }
            } else {
                ngramSize_ = 1;
            }
        }
        virtual lucene::analysis::TokenStream* tokenStream(const TCHAR          * /*fieldName*/, 
                                                           lucene::util::Reader * reader) {
            return _CLNEW analysis::CjkNGramTokenizer(reader, ngramSize_); 
        }
        
    private:
        
        int ngramSize_;
    };

	
	/**
	 * Template class wrapping CLucene analyzers. Template parameter T must 
	 * implement lucene::analysis::Analyzer abstraction.  
	 */    
	template<class T>
	class AnalyzerWrap : public TokenStreamFactory 
	{
	public:
		AnalyzerWrap(const Invokation& invokation) : analyzer_() {
			if (invokation.params().size() > 0) {
				THROW_CPIXEXC(L"Tokenizer %S does not accept parameters",
							  invokation.id().c_str());
			}
		}
		virtual lucene::analysis::TokenStream* tokenStream(const TCHAR          * fieldName, 
														   lucene::util::Reader * reader) {
			return analyzer_.tokenStream(fieldName, reader); 
		}
	private: 
		T analyzer_;
	};
	
	/**
	 * Template class associated with CLucene filter and a TokenStreamFactory. 
	 * Uses TokenStreamFactory to transform given character stream into tokenstream
	 * and then applies the given Clucene filter to the token stream. 
	 * The template parameter T must implement lucene::analysis::Filter abstraction.     
	 */    
	template<class T>
	class FilterFactory : public TokenStreamFactory 
	{
	public:
		FilterFactory(const Invokation& invokation, auto_ptr<TokenStreamFactory> factory) : factory_(factory) {
			if (invokation.params().size() > 0) {
				THROW_CPIXEXC(L"Filter %S does not accept parameters",
							  invokation.id().c_str());
			}
		}
		virtual lucene::analysis::TokenStream* tokenStream(const TCHAR          * fieldName, 
														   lucene::util::Reader * reader) {
			return _CLNEW T(factory_->tokenStream(fieldName, reader), true); 
		}
	private: 
		std::auto_ptr<TokenStreamFactory> factory_; 
	};
	
	/**
	 * Specialized Analyzer wrap for CLucene's PerFieldAnalyzer. Specialized
	 * template is needed because perfield analyzer accepts parameters
	 * (specific analyzers for different field plus default analyzer)
	 */
	template<>
	class AnalyzerWrap<lucene::analysis::PerFieldAnalyzerWrapper> : public TokenStreamFactory {
	public:
		AnalyzerWrap(const Switch& sw, const wchar_t* config) : analyzer_(0) {
			using namespace Cpt::Parser;
			using namespace lucene::analysis;
			
			analyzer_ = _CLNEW PerFieldAnalyzerWrapper(_CLNEW CustomAnalyzer(sw.def()));
			
			for (int i = 0; i < sw.cases().size(); i++) {
				const Case& cs = *sw.cases()[i];
				for (int j = 0; j < cs.cases().size(); j++) {
					analyzer_->addAnalyzer( cs.cases()[j].c_str(), _CLNEW CustomAnalyzer( cs.piping(), config ) );
				}
			}
		}
		virtual ~AnalyzerWrap() {
			_CLDELETE(analyzer_);
		}
		virtual lucene::analysis::TokenStream* tokenStream(const TCHAR          * fieldName, 
														   lucene::util::Reader * reader) {
			return analyzer_->tokenStream(fieldName, reader); 
		}
	private: 
		lucene::analysis::PerFieldAnalyzerWrapper* analyzer_;
	};
		
	
	
	/**
	 * Specialized StopFilter factory. Specialized filter is needed
	 * because StopFilter needs parameters (stop word list or a language) 
	 */
	template<>
	class FilterFactory<lucene::analysis::StopFilter> : public TokenStreamFactory 
	{
	public:
		FilterFactory(const Invokation& invokation,
					  auto_ptr<TokenStreamFactory> factory)
			:words_(0),  ownWords_(0), factory_(factory) {
			using namespace Cpt::Parser;
			if (invokation.params().size() == 1 && dynamic_cast<Identifier*>(invokation.params()[0])) {
				Identifier* id = dynamic_cast<Identifier*>(invokation.params()[0]);
				//cpix_LangCode lang; 
				if (id->id() == CPIX_WLANG_EN) {
					words_ = lucene::analysis::StopAnalyzer::ENGLISH_STOP_WORDS;
                } else if (id->id() == CPIX_WLANG_FR) {
                    words_ = analysis::NonEnglishStopWords::FRENCH_STOP_WORDS;
				} else {
					THROW_CPIXEXC(L"No prepared stopword list for language code '%S'",
								  id->id().c_str());
				}
			} else {
				ownWords_ = new wchar_t*[invokation.params().size()+1];
				memset(ownWords_, 0, sizeof(wchar_t*)*(invokation.params().size()+1)); 
				// FIXE: args may leak
				for (int i = 0; i < invokation.params().size(); i++) {
					StringLit* lit = dynamic_cast<StringLit*>(invokation.params()[i]);
					if (lit) {
						const wstring& str = lit->text(); 
						ownWords_[i] = new wchar_t[str.length()+1]; 
						wcscpy(ownWords_[i], str.c_str());
					} else {
						THROW_CPIXEXC(L"StopFilter accepts only language identifer or list of strings as a parameters.");
					}
				}
			}
		
		}
		virtual ~FilterFactory() { 
			if (ownWords_) {
				for (int i = 0; ownWords_[i]; i++) {
					delete[] ownWords_[i]; 
				}
				delete[] ownWords_;
			}
		}
		virtual lucene::analysis::TokenStream* tokenStream(const TCHAR          * fieldName, 
														   lucene::util::Reader * reader) {
			return _CLNEW lucene::analysis::StopFilter(factory_->tokenStream(fieldName, reader), true, ownWords_ ? const_cast<const wchar_t**>(ownWords_) : words_); 
		}
	private: 
		const wchar_t **words_;
		wchar_t **ownWords_; // owned
		std::auto_ptr<TokenStreamFactory> factory_; 
	};
	
	/**
	 * Specialized SnowballFilter factory is needed, because SnowballFilter
	 * accepts parameters (the language). 
	 */
	template<>
	class FilterFactory<lucene::analysis::SnowballFilter> : public TokenStreamFactory 
	{
	public:
		FilterFactory(const Invokation& invokation, 		
					  auto_ptr<TokenStreamFactory> factory)
			: factory_(factory) {
			using namespace Cpt::Parser;
			if (invokation.params().size() != 1 || !dynamic_cast<Identifier*>(invokation.params()[0])) {
				THROW_CPIXEXC(L"Snowball filter takes exactly one identifier as a parameter." );
			}
			Identifier* id = dynamic_cast<Identifier*>(invokation.params()[0]);
			if (id->id() == CPIX_WLANG_EN) {
				lang_ = cpix_LANG_EN; 
			} else {
				THROW_CPIXEXC(L"Language identifier %S is not supported for stemming",
							  id->id().c_str());
			}
		}
		virtual lucene::analysis::TokenStream* tokenStream(const TCHAR          * fieldName, 
														   lucene::util::Reader * reader) {
			return _CLNEW lucene::analysis::SnowballFilter(factory_->tokenStream(fieldName, reader), true, lang_); 
		}
	private: 
		cpix_LangCode lang_;
		std::auto_ptr<TokenStreamFactory> factory_; 
	};
	
	/**
	 * Specialized LengthFilter factory is needed, because length filter 
	 * accepts parameters (minimum length and maximum length)
	 */
	template<>
	class FilterFactory<lucene::analysis::LengthFilter> : public TokenStreamFactory 
	{
	public:
		FilterFactory(const Invokation& invokation, 
					  auto_ptr<TokenStreamFactory> factory) 
			: factory_(factory) {
			using namespace Cpt::Parser;
			if (invokation.params().size() != 2 || 
				!dynamic_cast<IntegerLit*>(invokation.params()[0]) || 
				!dynamic_cast<IntegerLit*>(invokation.params()[1])) {
				THROW_CPIXEXC("Length filter takes exactly two integer parameters");
			}
			min_ = dynamic_cast<IntegerLit*>(invokation.params()[0])->value();
			max_ = dynamic_cast<IntegerLit*>(invokation.params()[1])->value();
		}
		virtual lucene::analysis::TokenStream* tokenStream(const TCHAR          * fieldName, 
														   lucene::util::Reader * reader) {
			return _CLNEW lucene::analysis::LengthFilter(factory_->tokenStream(fieldName, reader), true, min_, max_ ); 
		}
	private: 
		int min_, max_;
		std::auto_ptr<TokenStreamFactory> factory_; 
	};
	
	/**
	 * Specialized PrefixGenerator factory is needed, because PrefixGenerator
	 * requires the max prefix size. 
	 */
	template<>
	class FilterFactory<PrefixGenerator> : public TokenStreamFactory 
	{
	public:
		FilterFactory(const Invokation& invokation, 
					  auto_ptr<TokenStreamFactory> factory) 
			: factory_(factory) {
			using namespace Cpt::Parser;
			if (invokation.params().size() != 1 || 
				!dynamic_cast<IntegerLit*>(invokation.params()[0])) {
				THROW_CPIXEXC("Prefix generator takes exactly one integer parameter");
			}
			maxPrefixLength_ = dynamic_cast<IntegerLit*>(invokation.params()[0])->value();
		}
		virtual lucene::analysis::TokenStream* tokenStream(const TCHAR          * fieldName, 
														   lucene::util::Reader * reader) {
			return _CLNEW PrefixGenerator(factory_->tokenStream(fieldName, reader), true, maxPrefixLength_ ); 
		}
	private: 
		int maxPrefixLength_;
		std::auto_ptr<TokenStreamFactory> factory_; 
	};

	/**
	 * Specialized PrefixFilter factory is needed, because prefix filter 
	 * accepts parameters (language set or prefixes)
	 */
	template<>
	class FilterFactory<analysis::PrefixFilter> : public TokenStreamFactory 
	{
	public:
		FilterFactory(const Invokation& invokation,
					  auto_ptr<TokenStreamFactory> factory)
			:	prefixes_(0),  ownPrefixes_(0), factory_(factory) {
			using namespace Cpt::Parser;
			if (invokation.params().size() == 1 && 
				dynamic_cast<Identifier*>(invokation.params()[0])) {
				Identifier* id = dynamic_cast<Identifier*>(invokation.params()[0]);
				//cpix_LangCode lang; 
				if (id->id() == CPIX_WLANG_HE) {
					prefixes_ = analysis::HebrewPrefixes;
				} else {
					THROW_CPIXEXC(L"No prepared prefix list for language code '%S'",
								  id->id().c_str());
				}
			} else {
				ownPrefixes_ = new wchar_t*[invokation.params().size()+1];
				memset(ownPrefixes_, 0, sizeof(wchar_t*)*(invokation.params().size()+1)); 
				// FIXE: args may leak
				for (int i = 0; i < invokation.params().size(); i++) {
					StringLit* lit = dynamic_cast<StringLit*>(invokation.params()[i]);
					if (lit) {
						const wstring& str = lit->text(); 
						ownPrefixes_[i] = new wchar_t[str.length()+1]; 
						wcscpy(ownPrefixes_[i], str.c_str());
					} else {
						THROW_CPIXEXC(L"PrefixFilter accepts only language identifer or list of strings as a parameters.");
					}
				}
			}
		}
		virtual ~FilterFactory() { 
			if (ownPrefixes_) {
				for (int i = 0; ownPrefixes_[i]; i++) {
					delete[] ownPrefixes_[i]; 
				}
				delete[] ownPrefixes_;
			}
		}
		virtual lucene::analysis::TokenStream* tokenStream(const TCHAR          * fieldName, 
														   lucene::util::Reader * reader) {
			return _CLNEW analysis::PrefixFilter(factory_->tokenStream(fieldName, reader), true, ownPrefixes_ ? const_cast<const wchar_t**>(ownPrefixes_) : prefixes_); 
		}
	private: 
		const wchar_t **prefixes_;
		wchar_t **ownPrefixes_; // owned
		std::auto_ptr<TokenStreamFactory> factory_; 
	};
	
	/**
	 * Specialized ElisionFilter factory is needed, because elision filter 
	 * accepts parameters (language set or articles)
	 */
	template<>
	class FilterFactory<analysis::ElisionFilter> : public TokenStreamFactory 
	{
	public:
		FilterFactory(const Invokation& invokation,
					  auto_ptr<TokenStreamFactory> factory)
			:	articles_(0),  ownArticles_(0), factory_(factory) {
			using namespace Cpt::Parser;
			if (invokation.params().size() == 1 && 
				dynamic_cast<Identifier*>(invokation.params()[0])) {
				Identifier* id = dynamic_cast<Identifier*>(invokation.params()[0]);
				//cpix_LangCode lang; 
				if (id->id() == CPIX_WLANG_FR) {
					articles_ = analysis::FrenchArticles;
				} else {
					THROW_CPIXEXC(L"No prepared article list for language code '%S'",
								  id->id().c_str());
				}
			} else {
				ownArticles_ = new wchar_t*[invokation.params().size()+1];
				memset(ownArticles_, 0, sizeof(wchar_t*)*(invokation.params().size()+1)); 
				// FIXE: args may leak
				for (int i = 0; i < invokation.params().size(); i++) {
					StringLit* lit = dynamic_cast<StringLit*>(invokation.params()[i]);
					if (lit) {
						const wstring& str = lit->text(); 
						ownArticles_[i] = new wchar_t[str.length()+1]; 
						wcscpy(ownArticles_[i], str.c_str());
					} else {
						THROW_CPIXEXC(L"PrefixFilter accepts only language identifer or list of strings as a parameters.");
					}
				}
			}
		}
		virtual ~FilterFactory() { 
			if (ownArticles_) {
				for (int i = 0; ownArticles_[i]; i++) {
					delete[] ownArticles_[i]; 
				}
				delete[] ownArticles_;
			}
		}
		virtual lucene::analysis::TokenStream* tokenStream(const TCHAR          * fieldName, 
														   lucene::util::Reader * reader) {
			return _CLNEW analysis::ElisionFilter(factory_->tokenStream(fieldName, reader), true, ownArticles_ ? const_cast<const wchar_t**>(ownArticles_) : articles_); 
		}
	private: 
		const wchar_t **articles_;
		wchar_t **ownArticles_; // owned
		std::auto_ptr<TokenStreamFactory> factory_; 
	};
	
	typedef auto_ptr<TokenStreamFactory> (*TokenizerFactoryCreator)(const Invokation& invokation);
	typedef auto_ptr<TokenStreamFactory> (*FilterFactoryCreator)(const Invokation& invokation, 
																 auto_ptr<TokenStreamFactory> factory);

	template<class T>
	struct TokenStreamFactoryCtor
	{
		static auto_ptr<TokenStreamFactory> create(const Invokation& invokation) {
			return auto_ptr<TokenStreamFactory>(new T(invokation)); 
		}
	};

	/**
	 * Sets up a tokenizer factory with given invokation parameters
	 */
	template<class T>
	struct TokenizerFactoryCtor
	{
		static auto_ptr<TokenStreamFactory> create(const Invokation& invokation) {
			return auto_ptr<TokenStreamFactory>(new TokenizerFactory<T>(invokation)); 
		}
	};
	
	/**
	 * Sets up an analyzer wrap with given invokation parameters
	 */
	template<class T>
	struct AnalyzerWrapCtor
	{
		static auto_ptr<TokenStreamFactory> create(const Invokation& invokation) {
			return auto_ptr<TokenStreamFactory>(new AnalyzerWrap<T>(invokation)); 
		}
	};
	
	/**
	 * Sets up a filter factory with given invokation parameters
	 */
	template<class T>
	struct FilterFactoryCtor 
	{
		static auto_ptr<TokenStreamFactory> create(const Invokation& invokation,
												   auto_ptr<TokenStreamFactory> factory) {
			return auto_ptr<TokenStreamFactory>(new FilterFactory<T>(invokation, factory)); 
		}
	};
	
	struct TokenizerClassEntry {
		const wchar_t *id_;
		TokenizerFactoryCreator createFactory_;
	};
	
	//
	// Following TokenizerClassEntries and FilterClassEntries contain
	// the mapping from tokenizer/analyzer/filter names into glue code
	// templates providing the implementations. 
	// 
	
	TokenizerClassEntry TokenizerClassEntries[] = { 
		{CPIX_TOKENIZER_STANDARD, 	TokenizerFactoryCtor<lucene::analysis::standard::StandardTokenizer>::create},
		{CPIX_TOKENIZER_WHITESPACE, TokenizerFactoryCtor<lucene::analysis::WhitespaceTokenizer>::create},
		{CPIX_TOKENIZER_LETTER, 	TokenizerFactoryCtor<lucene::analysis::LetterTokenizer>::create},
		{CPIX_TOKENIZER_KEYWORD, 	TokenizerFactoryCtor<lucene::analysis::KeywordTokenizer>::create},
		{CPIX_TOKENIZER_CJK, 		TokenizerFactoryCtor<lucene::analysis::cjk::CJKTokenizer>::create},
        {CPIX_TOKENIZER_NGRAM,      TokenizerFactoryCtor<analysis::CjkNGramTokenizer>::create},
        {CPIX_TOKENIZER_KOREAN,     TokenizerFactoryCtor<analysis::KoreanTokenizer>::create},
        {CPIX_TOKENIZER_KOREAN_QUERY,TokenizerFactoryCtor<analysis::KoreanQueryTokenizer>::create},
        
		{CPIX_ANALYZER_STANDARD, 	AnalyzerWrapCtor<lucene::analysis::standard::StandardAnalyzer>::create},
		{CPIX_ANALYZER_DEFAULT, 	TokenStreamFactoryCtor<DefaultTokenStreamFactory>::create},
	
	// 	TODO: Add more Tokenizers/Analyzers
		
	// 	Example tokenizer (works as such if tokenizers don't take parameters)
	//  {CPIX_TOKENIZER_MYTOKENIZER,TokenizerFactoryCtor<MyTokenizer>::create},
	
	// 	Example analyzer (works as such if analyzer don't take parameters)
	//  {CPIX_ANALYZER_MYANALYZER,	AnalyzerWrapCtor<MyAnalyzer>::create},
	
		{0, 						0}
	};
	
	struct FilterClassEntry {
		const wchar_t *id_;
		FilterFactoryCreator createFactory_;
	};
	
	FilterClassEntry FilterClassEntries[] = {
		{CPIX_FILTER_STANDARD, 	FilterFactoryCtor<lucene::analysis::standard::StandardFilter>::create},
		{CPIX_FILTER_LOWERCASE, FilterFactoryCtor<lucene::analysis::LowerCaseFilter>::create},
		{CPIX_FILTER_ACCENT, 	FilterFactoryCtor<lucene::analysis::ISOLatin1AccentFilter>::create},
		{CPIX_FILTER_STOP, 		FilterFactoryCtor<lucene::analysis::StopFilter>::create},
		{CPIX_FILTER_STEM, 		FilterFactoryCtor<lucene::analysis::SnowballFilter>::create},
		{CPIX_FILTER_LENGTH, 	FilterFactoryCtor<lucene::analysis::LengthFilter>::create},
		{CPIX_FILTER_PREFIXES, 	FilterFactoryCtor<PrefixGenerator>::create},
		{CPIX_FILTER_THAI, 		FilterFactoryCtor<analysis::ThaiWordFilter>::create},
		{CPIX_FILTER_PREFIX, 	FilterFactoryCtor<analysis::PrefixFilter>::create},
		{CPIX_FILTER_ELISION, 	FilterFactoryCtor<analysis::ElisionFilter>::create},
		
	// 	TODO: Add more Filters
	
	// 	Example filter (works as such if filter don't take parameters)
	//  {CPIX_FILTER_MYFILTER,	FilterFactoryCtor<MyFilter>::create},
	
		{0, 					0}
	};
	
	CustomAnalyzer::CustomAnalyzer(const wchar_t* definition, const wchar_t* config) {
		std::auto_ptr<Piping> piping = AnalyzerExp::ParsePiping( definition );
		setup( *piping, config );
	}
	
	CustomAnalyzer::CustomAnalyzer(const Piping& definition, const wchar_t* config) {	
		setup(definition, config);
	}
	
	using namespace Cpt::Parser;
	
	void CustomAnalyzer::setup(const Piping& piping, const wchar_t* config) {
	
		// If the first item is invokation, create corresponding analyzer/tokenizer 
		if (dynamic_cast<const Invokation*>(&piping.tokenizer())) {
			const Invokation& tokenizer = dynamic_cast<const Invokation&>(piping.tokenizer());
			TokenizerClassEntry& tokenizerEntry = getTokenizerEntry( tokenizer.id() ); 
			factory_ = tokenizerEntry.createFactory_( tokenizer );
		} else if (dynamic_cast<const Switch*>(&piping.tokenizer())) {
			// If the first item is switch statement, create per-field analyzer 
			const Switch& tokenizer = dynamic_cast<const Switch&>(piping.tokenizer());
			factory_ = new AnalyzerWrap<lucene::analysis::PerFieldAnalyzerWrapper>( tokenizer, config );
		} else if (dynamic_cast<const LocaleSwitch*>(&piping.tokenizer())) {
			const LocaleSwitch& tokenizer = dynamic_cast<const LocaleSwitch&>(piping.tokenizer());
			factory_ = new LocaleSwitchStreamFactory( tokenizer, config );
		} else if (dynamic_cast<const ConfigSwitch*>(&piping.tokenizer())) {
			const ConfigSwitch& tokenizer = dynamic_cast<const ConfigSwitch&>(piping.tokenizer());
			factory_ = resolveConfigSwitch( tokenizer, config );
		} else {
			THROW_CPIXEXC(L"Analyzer definition syntax did not begin with valid tokenizer");
		}
		
		// Add filters
		const std::vector<Invokation*>& filters = piping.filters(); 
		for (int i = 0; i < filters.size(); i++) {
			FilterClassEntry& filterEntry = getFilterEntry( filters[i]->id() ); 
			factory_ = filterEntry.createFactory_( *filters[i], factory_ );
		}
	}

	std::auto_ptr<TokenStreamFactory> CustomAnalyzer::resolveConfigSwitch(const ConfigSwitch& csw, const wchar_t* config) {
		if (config) {
			for (int i = 0; i < csw.cases().size(); i++) {
				const Case& cs = *csw.cases()[i];
				for (int j = 0; j < cs.cases().size(); j++) {
					if (wcscmp(config, cs.cases()[j].c_str()) == 0) {
						return std::auto_ptr<TokenStreamFactory>(
							new CustomAnalyzer(cs.piping(), config)); 
					}
				}
			}
		}
		return std::auto_ptr<TokenStreamFactory>(new CustomAnalyzer(csw.def(), config));
	}

	TokenizerClassEntry& CustomAnalyzer::getTokenizerEntry(std::wstring id) {
	
		// Looks for a match in the TokenizerClassEntries. After finding 
		// a match it returns a proper tokenizer/analyzer implementation provider 
		// 
		for (int i = 0; TokenizerClassEntries[i].id_; i++) {
			if (id == std::wstring(TokenizerClassEntries[i].id_)) {
				return TokenizerClassEntries[i];
			}
		}
	
		THROW_CPIXEXC(L"Unknown tokenizer '%S'.",
					  id.c_str());
	}
	
	FilterClassEntry& CustomAnalyzer::getFilterEntry(std::wstring id) {
	
		// Looks for a match in the FilterClassEntries. After finding 
		// a match it returns a proper tokenizer/analyzer implementation 
		// provider 
		// 
		for (int i = 0; FilterClassEntries[i].id_; i++) {
			if (id == std::wstring(FilterClassEntries[i].id_)) {
				return FilterClassEntries[i];
			}
		}
	
		THROW_CPIXEXC(L"Unknown filter '%S'.",
					  id.c_str());
	}
	
	CustomAnalyzer::~CustomAnalyzer() {} 
	
	lucene::analysis::TokenStream* CustomAnalyzer::tokenStream(const wchar_t        * fieldName, 
															   lucene::util::Reader * reader) {
		// Utilizes the the token stream factory to form token stream. 
		// token stream factory is prepared during custom analyzer construction
		// and based on the analyzer definition string.
															   
		return factory_->tokenStream(fieldName, reader);
	}
	
	std::auto_ptr<lucene::analysis::Analyzer> CreateDefaultAnalyzer()
	{
		return 
			std::auto_ptr<lucene::analysis::Analyzer>(
				new SystemAnalyzer(_CLNEW lucene::analysis::standard::StandardAnalyzer()));  
	}

}
