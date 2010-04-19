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
#include "CLucene/analysis/AnalysisHeader.h"
#include "CLucene/analysis/Analyzers.h"

#include "analyzer.h"
#include "analyzerexp.h"
#include "cpixanalyzer.h"
#include "cluceneext.h"

#include "cpixexc.h"
#include "cpixparsetools.h"

#include "wchar.h"
#include <string>
#include <vector>
#include <sstream>
#include <iostream>

#include "document.h"

#include "indevicecfg.h" 

namespace
{
    const char AGGR_NONFILEREADERPROXY_ERR[] 
    = "Aggregated reader field should be FileReaderProxy instance";

    const char AGGR_STREAMREADER_ERR[] 
    = "Aggregating streamValue-fields not implemented";
}


namespace Cpix {


    AggregateFieldTokenStream::AggregateFieldTokenStream(lucene::analysis::Analyzer& analyzer, 
                                                         DocumentFieldIterator* fields) 
	: stream_(), analyzer_( analyzer ), reader_(), fields_( fields ) {
        getNextStream(); 
    }
													   
    AggregateFieldTokenStream::~AggregateFieldTokenStream() {
        _CLDELETE( stream_ ); 
        delete fields_; 
    }
	
    bool AggregateFieldTokenStream::next(lucene::analysis::Token* token) {
        while ( stream_ ) {
            if ( stream_->next( token ) ) {
                return true;
            }
            getNextStream();
        }
        return false;
    }
		
    void AggregateFieldTokenStream::close() {
        if (stream_) stream_->close(); 
        _CLDELETE( stream_ ); 
        _CLDELETE( reader_ ); 
    }
	
    void AggregateFieldTokenStream::getNextStream()
    {
        using namespace lucene::document;
        using namespace lucene::util; 
	
        if ( stream_ ) stream_->close(); 
        _CLDELETE( stream_ ); 
        _CLDELETE( reader_ );
		
        Field* field = 0; 
        while (*fields_ && field == NULL)
            {
                field = (*fields_)++;
	
                if (!field->isAggregated()) 
                    {
						field = 0;
                    }
            }
        if (field) {
            if (field->stringValue() != NULL)
                {
                    reader_ = _CLNEW CL_NS(util)::StringReader(field->stringValue(),_tcslen(field->stringValue()),false);
                }
            else if (field->native().readerValue() != NULL)
                {
                    Reader* r = field->native().readerValue();
                    FileReaderProxy
                        * frp = 
                        dynamic_cast<FileReaderProxy*>(r);
                    if (frp == NULL)
                        {
                            _CLTHROWA(CL_ERR_IO, AGGR_NONFILEREADERPROXY_ERR);
                        }
                    else
                        {
                            reader_ = frp->clone();
                        }
                }
            else
                {
                    _CLTHROWA(CL_ERR_IO, AGGR_STREAMREADER_ERR);
                }
            stream_ = analyzer_.tokenStream( field->name(), reader_ ); 
        }
    }
	
		
    AggregateFieldAnalyzer::AggregateFieldAnalyzer(Cpix::Document& document, 
                                                   lucene::analysis::Analyzer& analyzer) 
	:	analyzer_(analyzer), document_(document)
    {
    }
	
    lucene::analysis::TokenStream* AggregateFieldAnalyzer::tokenStream(const TCHAR     * fieldName, 
                                                                       lucene::util::Reader * reader) {
        if ( wcscmp( fieldName, LCPIX_DEFAULT_FIELD ) == 0 ) {
            return new AggregateFieldTokenStream( analyzer_, document_.fields()); 
        } else {
            return analyzer_.tokenStream( fieldName, reader ); 
        }
    }
	
    SystemAnalyzer::SystemAnalyzer(lucene::analysis::Analyzer* analyzer) : analyzer_(analyzer) {} 
    SystemAnalyzer::~SystemAnalyzer() { _CLDELETE(analyzer_); }
	
    lucene::analysis::TokenStream* SystemAnalyzer::tokenStream(const TCHAR      	* fieldName, 
                                                               lucene::util::Reader * reader) {
        using namespace lucene::analysis; 
        if ( wcscmp( fieldName, LCPIX_DEFAULT_FIELD ) == 0 ) {
            // Use standard analyzer without stop filter for this task
            TokenStream* ret = _CLNEW standard::StandardTokenizer(reader);
            ret = _CLNEW standard::StandardFilter(ret,true);
            ret = _CLNEW LowerCaseFilter(ret,true);
            return ret;
        } else if (wcscmp( fieldName, LCPIX_DOCUID_FIELD) == 0){
            // Use standard analyzer without stop filter for this task
            return  _CLNEW KeywordTokenizer(reader);
        } else if (wcscmp( fieldName, LCPIX_APPCLASS_FIELD )  == 0){
            // Use standard analyzer without stop filter for this task
            TokenStream* ret = _CLNEW WhitespaceTokenizer(reader);
            ret = _CLNEW LowerCaseFilter(ret,true);
            return ret;
        } else if (wcscmp( fieldName, LCPIX_MIMETYPE_FIELD ) == 0) {
            TokenStream* ret = _CLNEW KeywordTokenizer(reader);
            return ret;
        } else {
            return analyzer_->tokenStream( fieldName, reader ); 
        }									 
    }

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
    
    /**
     * Creates token stream for the given reader and fieldName.
     * This class in in many ways similar to CLucene analyzer class 
     * definition.   
     */
    class TokenStreamFactory {
    public: 
        virtual ~TokenStreamFactory(); 
        virtual lucene::analysis::TokenStream* tokenStream(const wchar_t        * fieldName, 
                                                           lucene::util::Reader * reader) = 0;
    };
	
    TokenStreamFactory::~TokenStreamFactory() {};
	
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
        AnalyzerWrap(const Switch& sw) : analyzer_(0) {
            using namespace Cpt::Parser;
            using namespace lucene::analysis;
			
            analyzer_ = _CLNEW PerFieldAnalyzerWrapper(_CLNEW CustomAnalyzer(sw.def()));
			
            for (int i = 0; i < sw.cases().size(); i++) {
                const Case& cs = *sw.cases()[i];
                for (int j = 0; j < cs.fields().size(); j++) {
                    analyzer_->addAnalyzer( cs.fields()[j].c_str(), _CLNEW CustomAnalyzer( cs.piping() ) );
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

    typedef auto_ptr<TokenStreamFactory> (*TokenizerFactoryCreator)(const Invokation& invokation);
    typedef auto_ptr<TokenStreamFactory> (*FilterFactoryCreator)(const Invokation& invokation, 
                                                                 auto_ptr<TokenStreamFactory> factory);
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
        {CPIX_ANALYZER_STANDARD, 	AnalyzerWrapCtor<lucene::analysis::standard::StandardAnalyzer>::create},

// 		TODO: Add more Tokenizers/Analyzers
        
// 		Example tokenizer (works as such if tokenizers don't take parameters)
//      {CPIX_TOKENIZER_MYTOKENIZER,TokenizerFactoryCtor<MyTokenizer>::create},

// 		Example analyzer (works as such if analyzer don't take parameters)
//      {CPIX_ANALYZER_MYANALYZER,	AnalyzerWrapCtor<MyAnalyzer>::create},

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

// 		TODO: Add more Filters

// 		Example filter (works as such if analyzer don't take parameters)
//      {CPIX_FILTER_MYFILTER,	FilterFactoryCtor<MyFilter>::create},

        {0, 					0}
    };
	
    CustomAnalyzer::CustomAnalyzer(const wchar_t* definition)
    {
        using namespace Cpt::Lex;
        using namespace Cpt::Parser;


        try
            {
				// 1. Setup an tokenizer
                Cpix::AnalyzerExp::Tokenizer 
                    tokenizer; 
                StdLexer 
                    lexer(tokenizer, definition);
                
                // 2. Parse 
                std::auto_ptr<Piping> 
                    def = ParsePiping(lexer); 
                lexer.eatEof();
                
                // 3. Setup this item based on parsed definition
                setup(*def);
            }
        catch (Cpt::ITxtCtxtExc & exc)
            {
                // provide addition info for thrown exception
                exc.setContext(definition);

                // throw it fwd
                throw;
            }
    }

    CustomAnalyzer::CustomAnalyzer(const Piping& definition)
    {	
        setup(definition);
    }
    using namespace Cpt::Parser;
	
    void CustomAnalyzer::setup(const Piping& piping) {
    
		// If the first item is invokation, create corresponding analyzer/tokenizer 
        if (dynamic_cast<const Invokation*>(&piping.tokenizer())) 
        {
            const Invokation& tokenizer = dynamic_cast<const Invokation&>(piping.tokenizer());
            TokenizerClassEntry& tokenizerEntry = getTokenizerEntry( tokenizer.id() ); 
            factory_ = tokenizerEntry.createFactory_( tokenizer );
        } else {
            // If the first item is switch statement, create per-field analyzer 
            const Switch& tokenizer = dynamic_cast<const Switch&>(piping.tokenizer());
            factory_ = new AnalyzerWrap<lucene::analysis::PerFieldAnalyzerWrapper>( tokenizer );
        }
        
        // Add filters
        const std::vector<Invokation*>& filters = piping.filters(); 
        for (int i = 0; i < filters.size(); i++) {
            FilterClassEntry& filterEntry = getFilterEntry( filters[i]->id() ); 
            factory_ = filterEntry.createFactory_( *filters[i], factory_ );
        }
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

}

