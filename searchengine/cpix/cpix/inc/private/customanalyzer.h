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

#ifndef CUSTOMANALYZER_H_
#define CUSTOMANALYZER_H_

// Forward declarations
namespace Cpt {
	namespace Parser {
		class Lexer;
	}
}
namespace Cpix {
	namespace AnalyzerExp {
		class Piping;
		class LocaleSwitch; 
		class ConfigSwitch; 
	}
	struct TokenizerClassEntry;
	struct FilterClassEntry;
}


namespace Cpix {

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
	
	/**
	 * Forms a series of analyzers, tokenizers and filters based on textual 
	 * analyzer definition. 
	 */
	class CustomAnalyzer : public lucene::analysis::Analyzer, public TokenStreamFactory
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
		CustomAnalyzer(const wchar_t* definition, const wchar_t* config = NULL);
			
		/**
		 * For internal usage only. Constructs analyzer from a parsed
		 * definition string or from a fragment of a parsed definition
		 * string.
		 */
		CustomAnalyzer(const Cpix::AnalyzerExp::Piping& definition, const wchar_t* config = NULL);
			
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
		void setup(const Cpix::AnalyzerExp::Piping& definition, const wchar_t* config); 

		/**
		 * Return TokenizerClassEntry, which matches the given
		 * identifier.
		 */
		static std::auto_ptr<TokenStreamFactory> resolveConfigSwitch(const Cpix::AnalyzerExp::ConfigSwitch& csw, const wchar_t* config);

		/**
		 * Return TokenizerClassEntry, which matches the given
		 * identifier.
		 */
		static TokenizerClassEntry& getTokenizerEntry(std::wstring id);
			
		/**
		 * Return FilterClassEntry, which matches the given
		 * identifier.
		 */
		static FilterClassEntry& getFilterEntry(std::wstring id);
	
	private:
			
		std::auto_ptr<TokenStreamFactory> factory_;
	};

}

#endif /* CUSTOMANALYZER_H_ */
