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


#include "evaluationtool.h"
#include "analysisunittest.h"

#include "testutils.h"

#include "cpixstrtools.h"

#define MAX_LINE_LENGTH 512

namespace evaluationtool {

	using namespace lucene::analysis; 
	using namespace lucene::util; 
	using namespace lucene::index; 
	using namespace lucene::store; 
	using namespace lucene::search; 
	using namespace lucene::document; 
	using namespace lucene::queryParser; 


	static const wchar_t HIT_MARK_CHAR = 'X';
	static const int HIT_MARK_IDX = 2;
	static const wchar_t ERROR_MARK_CHAR = '!';
	static const int ERROR_MARK_IDX = 0;
	
	static const wchar_t* SUMMARY_STR = L"--- Summary ---";
	static const wchar_t* SECTION_END_STR = L"--- Section End ---";
	static const wchar_t* SEARCH_SECTION_STR = L"--- Search ---";
	static const wchar_t* FILE_END_STR = L"--- File End ---";
	
	static const wchar_t* ID_FIELD = L"id";
	static const wchar_t* CONTENT_FIELD = L"content";

	
	Corpus::Corpus(const char* file)
	:	lines_() {
		FileReader reader(file, "UTF-8");
			
		wchar_t line[MAX_LINE_LENGTH]; 
	
		while (readLine(reader, line, MAX_LINE_LENGTH)) {
			if (wcslen(line)) lines_.push_back( std::wstring( line ) ); 
		}
	}
	
	const wchar_t* Corpus::operator[](int i) {
		return lines_[i].c_str();
	}
	
	int Corpus::size() {
		return lines_.size();
	}

	
#define MAX_ID_LENGTH 10

	PreparedCorpus::PreparedCorpus(Corpus& corpus, 	
								   Analyzer& analyzer,
								   Analyzer* queryAnalyzer,
								   Analyzer* prefixAnalyzer)
	:   size_( corpus.size() ),
	    prefixAnalyzer_( prefixAnalyzer ),
	    dir_() {
		
		dir_.reset( FSDirectory::getDirectory( INDEX_DIRECTORY, true ) ); 
		
		IndexWriter writer(dir_.get(), &analyzer, true, false);

		wchar_t id[MAX_ID_LENGTH];
		
		for (int i = 0; i < corpus.size(); i++) {
			Document doc;
			snwprintf(id, MAX_ID_LENGTH, L"%d", i);
			doc.add(*new Field( ID_FIELD, id, Field::INDEX_NO | Field::STORE_YES));
			doc.add(*new Field( CONTENT_FIELD, corpus[i], Field::INDEX_TOKENIZED | Field::STORE_NO));
			writer.addDocument(&doc);
		}

		writer.optimize(); 
		writer.close();
		
		queryParser_.reset(new QueryParser(CONTENT_FIELD, queryAnalyzer ? queryAnalyzer : &analyzer));
		
		searcher_.reset(new IndexSearcher(dir_.get()));
	}
	
	int PreparedCorpus::size() {
		return size_; 
	}

	int PreparedCorpus::indexSize() {
        std::vector<std::string> v;
        dir_->list(&v);
        int ret = 0;
        for (int i = 0; i < v.size(); i++) {
            ret += dir_->fileLength(v[i].c_str());
        }
        return ret;
    }

	void PreparedCorpus::search(const wchar_t* query, std::bitset<MAXLINES>& hits ) {
		int qlen = wcslen( query ); 
		while (qlen > 0 && iswspace(query[qlen-1])) qlen--;
		auto_ptr<Query> q;
		if ( query[qlen-1] == '*' && prefixAnalyzer_ ) {
			// Simplified prefix query parser
			wchar_t buf[512]; 
			memcpy(buf, query, sizeof(wchar_t)*(qlen-1)); 
			buf[qlen-1] = '\0'; 
			// Assume, that prefix query contains only one word
			auto_ptr<TokenStream> t( prefixAnalyzer_->tokenStream(NULL, new StringReader(buf)) );
			Token token; 
			t->next(&token);
			Term* term = new Term( CONTENT_FIELD, token.termText() );
			q.reset( new PrefixQuery( term ) );
			_CLDECDELETE( term ); 
		} else {
			q.reset( queryParser_->parse(query) );
		}
		if ( q.get() ) {
			auto_ptr<Hits> h( searcher_->search( q.get() ) );
			for (int i = 0; i < h->length(); i++) {
				int id; 
				Cpt::wconvertInteger(&id, h->doc(i).get(ID_FIELD));
				hits[id] = true; 
			}
		}
	}

	
	Results::Results(std::bitset<MAXLINES>& hits, int lines)
	:	hits_(hits), lines_(lines) {}

	Results::Results() 
	:	hits_(), lines_(0) {}

	Results::Results(PreparedCorpus& corpus, 
					 const wchar_t* query) 
	: 	lines_(corpus.size()) {
		corpus.search(query, hits_);
	}
	
	bool Results::hit(int i) {
		return hits_[i]; 
	}
	
	void Results::append(bool hit) {
		hits_[lines_++] = hit;
	}

	
	int Results::length() {
		return lines_;
	}
	
	EvaluationRecordEntry::EvaluationRecordEntry(
		const wchar_t* query, 
		Results& ideal, 
		Results& measured)
	: query_( query ), 
	  ideal_( ideal ), 
	  measured_( measured ) {}
	
	EvaluationRecordEntry::EvaluationRecordEntry(Reader& reader) {
		wchar_t line[MAX_LINE_LENGTH];

		readLine(reader, line, MAX_LINE_LENGTH); // corpusName
		readLine(reader, line, MAX_LINE_LENGTH); // analyzerName
		readLine(reader, line, MAX_LINE_LENGTH); // query
		wchar_t* cut = line; while (*cut && *cut != ':') cut++;
		cut++; while (*cut == ' ') cut++;
		query_ = cut; 
		readLine(reader, line, MAX_LINE_LENGTH); // status
		readLine(reader, line, MAX_LINE_LENGTH); // hits
		readLine(reader, line, MAX_LINE_LENGTH); // errors
		readLine(reader, line, MAX_LINE_LENGTH); // false positives
		readLine(reader, line, MAX_LINE_LENGTH); // false negatives
	
		while (readLine(reader, line, MAX_LINE_LENGTH)) {
			if (wcscmp(line, SECTION_END_STR) == 0) break; 
			bool found = (line[HIT_MARK_IDX] == HIT_MARK_CHAR);   
			bool error = (line[ERROR_MARK_IDX] == ERROR_MARK_CHAR);   

			measured_.append(found);
			ideal_.append((!error)?found:!found);
		}
	}

	EvaluationRecordEntry::EvaluationRecordEntry()
	: query_(), ideal_(), measured_() {}
		
	EvaluationRecord::EvaluationRecord(const char* file) 
	: 	entries_() {
		FileReader reader(file, "UTF-8");
	
		wchar_t line[MAX_LINE_LENGTH];

		while (readLine(reader, line, MAX_LINE_LENGTH)) {
			// Skip summary
			if (wcscmp(line, SUMMARY_STR) == 0) {
				while (readLine(reader, line, MAX_LINE_LENGTH) 
					&& wcscmp(line, SECTION_END_STR) != 0); 
			}

			// Eof
			if (wcscmp(line, FILE_END_STR) == 0) break;
			
			// Search section
			if (wcscmp(line, SEARCH_SECTION_STR) == 0) {
				entries_.push_back( EvaluationRecordEntry( reader ) );
			}
		}
	}
		
	int EvaluationRecord::length() {
		return entries_.size(); 
	}
			
	const wchar_t* EvaluationRecord::query(int i) {
		return entries_[i].query_.c_str(); 
	}
			
	Results& EvaluationRecord::ideal(int i) {
		return entries_[i].ideal_; 
	}
			
	Results& EvaluationRecord::measured(int i) {
		return entries_[i].measured_; 
	}
		
	Evaluation::Evaluation(Results& ideal, Results& measured) 
	: ideal_( ideal ), 
	  measured_( measured ) {
	}

	bool Evaluation::falsePositive(int line) {
		return (!ideal_.hit(line))&&measured_.hit(line);
	}

	bool Evaluation::falseNegative(int line) {
		return ideal_.hit(line)&&(!measured_.hit(line));
	}

	bool Evaluation::error(int line) 
	{
		return (ideal_.hit(line)!=measured_.hit(line)?1:0);
	}
		
	int Evaluation::errors()
	{
		int ret = 0;
		for (int i = 0; i < ideal_.length(); i++) {
			if (error(i)) ret++; 
		}
		return ret;
	}
			
	int Evaluation::falsePositives()
	{
		int ret = 0;
		for (int i = 0; i < ideal_.length(); i++) {
			if (falsePositive(i)) ret++; 
		}
		return ret;
	}
			
	int Evaluation::falseNegatives()
	{
		int ret = 0;
		for (int i = 0; i < ideal_.length(); i++) {
			if (falseNegative(i)) ret++;
		}
		return ret;
	}
		
}
