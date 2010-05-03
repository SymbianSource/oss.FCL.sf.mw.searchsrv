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


#ifndef ANALYSIS_H_
#define ANALYSIS_H_

#include <memory>

#include "common/refcountedbase.h"

// Forward declarations
namespace Cpt {
	namespace Parser {
		class Lexer;
	}
}
namespace lucene {
	namespace analysis {
		class TokenStream; 
	}
	namespace util {
		class Reader;
	}
}

namespace Cpix
{
	namespace AnalyzerExp {
		class Piping; 
	}
	struct TokenizerClassEntry;
	struct FilterClassEntry;
	
	class TokenStreamFactory; 
	
	class Document; 
	class Field; 
	class DocumentFieldIterator; 
}

// Class definitions
namespace Cpix
{

	/**
	 * This is a special filter that is used to generate prefixes
	 * of the searched words.
	 * 
	 * For example token "chapter" will be split into tokens "ch" and "c"
	 * if maxPrefixLength is set as 2. 
	 */
	class PrefixGenerator : public lucene::analysis::TokenFilter {
	
	public: 
	
		PrefixGenerator(lucene::analysis::TokenStream* in, 
						bool deleteTS, 
						size_t maxPrefixLength);

		virtual ~PrefixGenerator();
		
		/**
		 * Returns
		 */
		virtual bool next(lucene::analysis::Token* token);
		
	private: 
		
		lucene::analysis::Token token_; 
		
		size_t prefixLength_;
	
		size_t maxPrefixLength_; 
	
	};

    /**
     * Aggregates token streams from all fields marked from aggregation. 
     * Used to generate the contents of the _aggregate field. 
     */
    class AggregateFieldTokenStream : public lucene::analysis::TokenStream {
    public: 
	
        AggregateFieldTokenStream(lucene::analysis::Analyzer & analyzer, 
                                  Cpix::DocumentFieldIterator* fields);
	
        virtual ~AggregateFieldTokenStream();
	
    public: // from TokenStream
		
        virtual bool next(lucene::analysis::Token* token);
		
        virtual void close();
		
    private: 
		
        /**
         * Prepares the tokens stream from next field in line
         */
        void getNextStream(); 
		
    private: 
		
        lucene::analysis::TokenStream* stream_; 
					
        lucene::analysis::Analyzer& analyzer_; 

		lucene::util::Reader* reader_; 

        Cpix::DocumentFieldIterator* fields_; 
    };
	
    /**
     * Creates tokens streams, that aggregate the token streams from
     * all fields intended for aggregation.
     */
    class AggregateFieldAnalyzer : public lucene::analysis::Analyzer {
    public:
        AggregateFieldAnalyzer(Cpix::Document& document, 
                               Analyzer& analyzer); 
	
    public:
	
        virtual lucene::analysis::TokenStream * 
        tokenStream(const TCHAR          * fieldName, 
                    lucene::util::Reader * reader);
		
    private:
		
        lucene::analysis::Analyzer& analyzer_; 
		
        Cpix::Document& document_; 
    };
	
    /**
     * Analyzer, that provides correct analyzer for each system field
     */
    class SystemAnalyzer : public lucene::analysis::Analyzer, public RefCountedBase {
    public:
	
        SystemAnalyzer(lucene::analysis::Analyzer* analyzer); 
        ~SystemAnalyzer(); 
		
    public:
	
        virtual lucene::analysis::TokenStream* 
        tokenStream(const TCHAR          * fieldName, 
                    lucene::util::Reader * reader);
		
    private:
		
        lucene::analysis::Analyzer* analyzer_; 
    };



    /**
     * Forms a series of analyzers, tokenizers and filters based on textual 
     * analyzer definition. 
     */
    class CustomAnalyzer : public lucene::analysis::Analyzer
    {
    public:
			
        /**
         * Constructs a custom analyzer based on given definition string.
         * See CPix documentation to see, how proper analyzer definition
         * strings ought to be formed.  
         * 
         * Throws on failure, e.g. if definition parsing fails, if
         * declared identifiers are not found and if parameters are wrong.  
         */
        CustomAnalyzer(const wchar_t* definition);
			
        /**
         * For internal usage only. Constructs analyzer from a parsed
         * definition string or from a fragment of a parsed definition
         * string.
         */
        CustomAnalyzer(const Cpix::AnalyzerExp::Piping& definition);
			
        virtual ~CustomAnalyzer();
			
        /**
         * Token stream is based on the analyzer definition string
         */
        lucene::analysis::TokenStream* 
        tokenStream(const wchar_t        * fieldName, 
                    lucene::util::Reader * reader);

    private:
			
        /**
         * Setups the TokenStream factory based on the analyzer
         * definition stored in the piping
         */
        void setup(const Cpix::AnalyzerExp::Piping& definition); 

        /**
         * Return TokenizerClassEntry, which matches the given
         * identifier.
         */
        static TokenizerClassEntry& 
        CustomAnalyzer::getTokenizerEntry(std::wstring id);
			
        /**
         * Return FilterClassEntry, which matches the given
         * identifier.
         */
        static FilterClassEntry& 
        CustomAnalyzer::getFilterEntry(std::wstring id);

    private:
			
        std::auto_ptr<TokenStreamFactory> factory_;
    };

}

#endif /* ANALYSIS_H_ */
