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
		class Analyzer;
	}
	namespace util {
		class Reader;
	}
}

namespace Cpix
{
	class InitParams; 

	struct TokenizerClassEntry;
	struct FilterClassEntry;
	
	class TokenStreamFactory; 
	
	class Document; 
	class Field; 
	class DocumentFieldIterator;
	
	class LocaleSwitchStreamFactory;
	class CustomAnalyzer;
	
	namespace AnalyzerExp {
		class LocaleSwitch;
		class Piping; 
	}
}

// Class definitions
namespace Cpix
{

	class Analysis {
		
	public: 
		
		/**
		 * Initializes the Analysis. Uses init paremeters' resource dir
		 * to locate & load analysis & localization related resources.
		 * 
		 * NOTE: The init is made to work in a fault-tolerant fashion. 
		 * If needed resource files are not found, a warning is logged 
		 * (if logging is enabled) and some meaningful default is used instead.
		 * If logging is not enabled, init _may fail silently_.   
		 */
		static void init(InitParams& ip); 
		
		/**
		 * Releases all resources that are used by analysis.  
		 */
		static void shutdown();

		/**
		 * Returns the default analyzer. This analyzer is likely localized
		 * and will analyze differently depending of what locale is currently 
		 * active. 
		 */
		static lucene::analysis::Analyzer& getDefaultAnalyzer();

		/**
		 * Returns the query analyzer. This analyzer is likely localized
		 * and will analyze differently depending of what locale is currently 
		 * active. 
		 */
		static lucene::analysis::Analyzer& getQueryAnalyzer();

		/**
		 * Returns the query filter analyzer. This analyzer is likely localized
		 * and will analyze differently depending of what locale is currently 
		 * active. 
		 */
		static lucene::analysis::Analyzer& getPrefixAnalyzer(); 

	private:
		
		Analysis(InitParams& ip); 
		
		std::auto_ptr<AnalyzerExp::Piping> parse(std::string path);
		
		static Analysis* theInstance_;
		
		std::auto_ptr<CustomAnalyzer> defaultAnalyzer_;
		
		std::auto_ptr<CustomAnalyzer> queryAnalyzer_;
				
		std::auto_ptr<CustomAnalyzer> prefixAnalyzer_;
				
	};



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

    std::auto_ptr<lucene::analysis::Analyzer> CreateDefaultAnalyzer();

}

#endif /* ANALYSIS_H_ */
