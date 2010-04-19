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

#ifndef CPIX_CLUCENEEXT_H
#define CPIX_CLUCENEEXT_H

// This define is for dumping the value of aggregate field to stdout,
// in case there is a bug to fix
//
// #define DBG_AGGREGATE_FIELD 1

#ifdef DBG_AGGREGATE_FIELD
#include <vector>
#endif

namespace lucene 
{ 

    namespace analysis {

        

        /**
         * Fashioned after lucene::analysis::StandardFilter
         */
        class SnowballFilter : public TokenFilter
        {
        private:
            struct sb_stemmer   * stemmer_;

        public:
            SnowballFilter(TokenStream * in,
                           bool          deleteTokenStream,
                           cpix_LangCode langCode);
            ~SnowballFilter();
            
            /** Returns the next token in the stream, or NULL at EOS.
             * <p>Removes <tt>'s</tt> from the end of words.
             * <p>Removes dots from acronyms.
             */
            bool next(Token * token);
        };

        
        /**
         * Fashioned after lucene::analysis::StandardAnalyzer
         */
        class SnowballAnalyzer : public Analyzer
        {
	private:
            lucene::util::CLSetList<const TCHAR*> stopSet_;
            cpix_LangCode                         langCode_;
	public:
            /** Builds an analyzer.*/
            explicit SnowballAnalyzer(cpix_LangCode langCode);

            /** Builds an analyzer with the given stop words. */
            SnowballAnalyzer(const TCHAR   ** stopWords,
                             cpix_LangCode    langCode);

            ~SnowballAnalyzer();


            /**
             * Constructs a StandardTokenizer filtered by a 
             * SnowballFilter, a LowerCaseFilter and a StopFilter.
             */
            TokenStream* tokenStream(const TCHAR          * fieldName, 
                                     lucene::util::Reader * reader);
        };

    }
}


namespace lucene
{
    namespace util
    {

        /**
         * This class is almost like clucene::util::FileReader,
         * as it:
         *
         *  o implements interface Reader (well, Reader is not an
         *    interface, but an idiotic base class)
         *
         *  o and behaves like FileReader, since it is implemented in
         *    terms of a FileReader (through aggregation).
         *
         * The difference is that this class is
         *
         *  o cloneable
         *
         *  o does not create a FileReader instance to use on
         *    construction, only on demand.
         *
         * This way this class can take place of a (file) Reader and be
         * cloneable for AggregateStream purposes as well it will not try
         * to open the same file twice at the same time.
         */
        class FileReaderProxy : public Reader
        {
        private:
            //
            // private members
            //
            FileReader         * fileReader_;

            const std::string    path_;
            const std::string    enc_;

        public:
            //
            // public operators
            //
            FileReaderProxy * clone() const;


            // from interface Reader
            /**
             * Destructor
             */
            virtual ~FileReaderProxy();


            /**
             * Constructor - taking the exact same parameters
             * as FileReader.
             */
            FileReaderProxy(const char * path,
                            const char * enc);


        protected:
            //
            // From Reader (modified)
            //
            /**
             * Initializes this instance with a file reader instance, if
             * this is not initialized yet.
             */
            void initialize();
        };


    }
}

#endif // CPIX_CLUCENEEXT_H
