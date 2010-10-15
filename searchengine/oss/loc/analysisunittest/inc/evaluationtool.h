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

#ifndef EVALUATIONTOOL_H_
#define EVALUATIONTOOL_H_

#include <vector>
#include <memory>
#include <string>
#include <bitset>

#include "cpixmemtools.h"

namespace lucene {
	namespace analysis {
		class Analyzer;
	}
	namespace store {
		class FSDirectory;
	}
	namespace search {
		class IndexSearcher;
	}
	namespace queryParser {
		class QueryParser;
	}
	namespace util {
		class Reader;
	}
}

namespace evaluationtool 
{
	
	/**
	 * Loads and provides access to a corpus in foreign language
	 */
	class Corpus 
	{
		public:
			Corpus(const char* file);
			
			const wchar_t* operator[](int i);
			
			int size(); 
		
		private: 
		
			std::vector<std::wstring> lines_; 
	};
	
#define MAXLINES 1024

	/**
	 * Contains the corpus in indexed form so that it can be searched
	 */
	class PreparedCorpus 
	{
	public:
		
		PreparedCorpus( Corpus& corpus, 
				        lucene::analysis::Analyzer& analyzer, 
				        lucene::analysis::Analyzer* queryAnalyzer = NULL,
				        lucene::analysis::Analyzer* prefixAnalyzer = NULL );
		
		int size();
		
		int indexSize();
		
		void search(const wchar_t* query, std::bitset<MAXLINES>& hits ); 

	public: 
		
		int size_;
		
		std::auto_ptr<lucene::store::FSDirectory> dir_;
		
		std::auto_ptr<lucene::queryParser::QueryParser> queryParser_;
		
		std::auto_ptr<lucene::search::IndexSearcher> searcher_;
		
		lucene::analysis::Analyzer* prefixAnalyzer_; 
	};
	
	class Results 
		{
		public: 
			Results();
		
			Results(std::bitset<MAXLINES>& hits, int lines);
			
			Results(PreparedCorpus& corpus, 
					const wchar_t* query);
		
			bool hit(int i);
			
			void append(bool hit);
						
			int length(); 
			
		private:
			
			std::bitset<MAXLINES> hits_;
			
			int lines_; 
			
		};
	
	/**
	 * An entry containing the ideal and measured results for a query.
	 */
	class EvaluationRecordEntry {
	public:
		EvaluationRecordEntry(const wchar_t* query, 
							  Results& ideal, 
							  Results& measured);
		
		EvaluationRecordEntry(lucene::util::Reader& reader);
		
		EvaluationRecordEntry();
		
		std::wstring query_; 
		Results ideal_; 
		Results measured_; 
	};
	
	/** 
	 * Contains ideal results and measured results for all queries. 
	 */
	class EvaluationRecord  {
		public: 
	
			EvaluationRecord(const char* file);
			
			int length();
			
			const wchar_t* query(int i); 
			
			Results& ideal(int i); 
			
			Results& measured(int i); 
		
		private:
			
			std::vector<EvaluationRecordEntry> entries_; 
		
	};
	
	/**
	 * Provides information of how the measured search compared
	 * to the ideal one. 
	 */
	class Evaluation
	{
		public:
		
			Evaluation(Results& ideal, Results& measured);

			bool falsePositive(int line);

			bool falseNegative(int line);

			bool error(int line);
		
			int errors();
			
			int falsePositives(); 
			
			int falseNegatives(); 
			
		
		private: 
		
			Results& ideal_; 
	
			Results& measured_; 
			
	};

}

#endif /* EVALUATIONTOOL_H_ */
