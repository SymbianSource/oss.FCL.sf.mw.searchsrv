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

#ifndef __CCPIXSEARCHER_H__
#define __CCPIXSEARCHER_H__

// INCLUDE FILES
#include <e32base.h>
#include "RSearchServerSession.h"

/**
 * @file
 * @ingroup ClientAPI
 * @brief Contains CCPixSearcher used for searching
 */

// FORWARD DECLARATIONS
class MCPixOpenDatabaseRequestObserver;
class MCPixSearchRequestObserver;
class MCPixNextDocumentRequestObserver;
class MCPixSetAnalyzerRequestObserver;
class MCPixSetQueryParserRequestObserver;

// CLASS DECLARATION
/**
 * @brief Used for committing searches into an index database
 * @ingroup ClientAPI
 * 
 * An instance of CCPixSearcher is used to commit search operations to CSearchServer
 * 
 * Link against: CPixSearchClient.lib 
 */
class CCPixSearcher : public CActive
	{
	public: 
		
		enum TQueryParser {
			/**
			 * Database query parser provides advanced syntax support
			 * for creating complex and powerful queries. This query parser
			 * is intended for accessing the CPix databases. It is not
			 * localized and it should not be used to form queries
			 * directly from user input. It may simply fail with some
			 * locales.  
			 */
			EDatabaseQueryParser = 0,
					
			/**
			 * Query parser aimed for incremental queries provided directly
			 * by user. This query parser is localized and should 
			 * work properly for all locales. The language accepted
			 * by this parser is always the language specified by 
			 * the current locale. May behave internally somewhat 
			 * differently depending of language, but as a rule it 
			 * should always provides meaningful results for direct
			 * user input.    
			 */
			EIncrementalQueryParser = 1
		};

	public: // Constructors and destructors
		
	    /**
	    * NewL.
	    * Two-phase constructor.
	    * Creates a CCPixSearcher object using two phase construction,
	    * and return a pointer to the created object.
	    * @param aDefaultSearchField Default field where the keywords are searched from.
	    * @return A pointer to the created instance of CCPixSearcher.
	    */
	    IMPORT_C static CCPixSearcher* NewL(RSearchServerSession& aSearchServerSession, const TDesC& aDefaultSearchField = KCpixDefaultField);
	
	    /**
	    * NewLC.
	    * Two-phase constructor.
	    * Creates a CCPixSearcher object using two phase construction,
	    * and return a pointer to the created object.
	    * @param aDefaultSearchField Default field where the keywords are searched from. 
	    * @return A pointer to the created instance of CCPixSearcher.
	    */
	    IMPORT_C static CCPixSearcher* NewLC(RSearchServerSession& aSearchServerSession, const TDesC& aDefaultSearchField = KCpixDefaultField);
	
	    /**
	    * ~CCPixSearcher.
	    * Destructor.
	    * Destroys the object and release memory.
	    */
	    IMPORT_C virtual ~CCPixSearcher();
	
	public: // New functions

		/**
		 * Returns true, if OpenDatabaseL method has been called succesfully. 
		 */
		IMPORT_C TBool IsDatabaseOpen() const; 

		/**
		 * Returns KNullDesC if database not open, otherwise the base app class for the opened database 
		 */		
		IMPORT_C const TDesC& GetBaseAppClass() const;
	    /**
	     * OpenDatabaseL. Synchronous version
	     * Opens a database to be searched by this searcher. Leaves with KErrInUse, 
	     * if asynchronous request is pending.
		 *
		 * Database must be succesfully opened before any searching functionality can be used. 
		 * Method can be used, if database has already been opened. In this case, connection to the old 
		 * database is closed.
		 * 
	     * @param aBaseAppClass Base application class that identifies the database this searcher is connected to.
	     */
		IMPORT_C void OpenDatabaseL(const TDesC& aBaseAppClass);
		
		/**
		 * OpenDatabaseL. Asynchronous version
		 * Requests opening a database to be searched by this searcher. When the request
		 * has completed or failed, the error code is passed to the HandleOpenDatabaseResultL-
		 * method of the observer. Leaves with KErrInUse, if asynchronous request is pending. 
		 * 
		 * Database must be succesfully opened before any searching functionality can be used. 
		 * Method can be used, if a database has already been opened. In this case, connection 
		 * to the old database is closed.
		 * 
		 * @param aObserver The observer to notify of the request's completion or failure
		 * @param aBaseAppClass Base application class that identifies the database this searcher is connected to.
		 */
		IMPORT_C void OpenDatabaseL(MCPixOpenDatabaseRequestObserver& aObserver, const TDesC& aBaseAppClass);
		
	    /**
	     * SetAnalyzer. Synchronous version
	     */
		IMPORT_C void SetAnalyzerL( const TDesC& aAnalyzer ); 

	    /**
	     * SetAnalyzer. Asynchronous version
	     */
		IMPORT_C void SetAnalyzerL( MCPixSetAnalyzerRequestObserver& aObserver, const TDesC& aAnalyzer );
		
	    /**
	     * SetAnalyzer. Synchronous version
	     * 
	     * The set QueryParser defines the query syntax used for searching. Two
	     * different query parsers are supported for two main use cases, that 
	     * are powerful accessing of the database for document and the other is
	     *     
	     * 
	     * @note SetAnalyzerL MUST NOT be used, when searching using discovery services
	     */
		IMPORT_C void SetQueryParserL( TQueryParser aQueryParser ); 

	    /**
	     * SetQueryParser. Synchronous version
	     * 
	     * @note Query parser cannot be set for discovery service
	     */
		IMPORT_C void SetQueryParserL( MCPixSetQueryParserRequestObserver& aObserver, TQueryParser aQueryParser ); 

	    /**
	     * SearchL. Synchronous version. 
	     * Commits a search and prepares a list of documents matching the query to be retrieved
	     * from the server. Use NextDocumentL to iterate throught the document list. 
		 *
	     * Leaves with KErrInUse, if asynchronous request is already pending and 
	     * KErrNotReady, if no database has been succefullly opened.
	     * @param aQueryString Query string, e.g. "j* smith". May include wild card characters '*' or '?'. See documentation for exact syntax
	     * @param aSearchField Database field where the keywords are searched from. If not given, default field is used.
	     * @return Estimated hit document count, so the number of documents matching the query
	     * 
	     * @note Is the search field necessary? Search field can also be embedded into the query 
	     * string and in case it already is there, and given search parameter is added there
	     * as an addition, an error may raise or the user may be suprised that the field 
	     * he sought to search over was not searched. 
	     */
		IMPORT_C TInt SearchL(const TDesC& aQueryString, const TDesC& aSearchField = KNullDesC);

		/**
		 * SearchL. Asynchronous version. 
	     * Issues a search request. When request has been completed a callback to 
	     * MSearchObserver::HandleSearchResultL is issued. 
	     * 
	     * Leaves with KErrInUse, if asynchronous request is already pending and 
	     * KErrNotReady, if no database has been succefullly opened.
	     * 
	     * @param aObserver The observer, which receives the result of this request
	     * @param aQueryString Query string, e.g. "j* smith". May include wild card characters '*' or '?'. See documentation for exact syntax
	     * @param aSearchField Database field where the keywords are searched from. If not given, default field is used.
	     * 
	     * @note Is the search field necessary? Search field can also be embedded into the query 
	     * string and in case it already is there, and given search parameter is added there
	     * as an addition, an error may raise or the user may be suprised that the field 
	     * he sought to search over was not searched. 
	     */
		IMPORT_C void SearchL(MCPixSearchRequestObserver& aObserver, const TDesC& aQuery, const TDesC& aSearchField = KNullDesC);

	    /**
	     * GetDocumentL
	     * Iterates throught the search result's document list (hits), that is stored in the 
	     * server side. The accessed list is ranked and the best matching document is heading 
	     * the result lest. First call of this method, return best matching, then second best 
	     * matching and so forth until all documents have been found. Method returns NULL, 
	     * when no more documents can be found.
	     *
	     * Leaves with KErrInUse, if asynchronous request is pending and 
	     * KErrNotReady, if no database has been succefullly opened.
         *
         * @todo Having also GetDocumentLC would be nice
         * @return Next available document or NULL, if no more documents can be found. Ownership is transferred
	     */
		IMPORT_C CSearchDocument* GetDocumentL(TInt aIndex);
		
		/**
	     * GetDocumentL
		 * Iterates throught the search result's document list (hits), that is stored in the 
	     * server side. The accessed list is ranked and the best matching document isheading 
	     * the result lest. When the request has been completed a callback to 
	     * MSearchObserver::HandleDocumentL is issued. 
	     *
	     * Leaves with KErrInUse, if asynchronous request is pending and 
	     * KErrNotReady, if no database has been succefullly opened.
	     *
         * @return Next available document or NULL, if no more documents can be found
		 */
		IMPORT_C void GetDocumentL(TInt aIndex, MCPixNextDocumentRequestObserver& aObserver);
		
		/**
	     * GetBatchDocumentL
	     * Iterates throught the search result's document list (hits), that is stored in the 
	     * server side. The accessed list is ranked and the best matching document is heading 
	     * the result list. First call of this method, return best matching, then second best 
	     * matching and so forth until all documents have been found. Method returns NULL, 
	     * when no more documents can be found.
	     *
	     * Leaves with KErrInUse, if asynchronous request is pending and 
	     * KErrNotReady, if no database has been succefullly opened.
         *
         * @todo Having also GetDocumentLC would be nice
         * @return Next available document or NULL, if no more documents can be found. Ownership is transferred
	     */
		IMPORT_C CSearchDocument** GetBatchDocumentL(TInt aIndex, TInt& aReturnDoc, TInt aCount = 1);
		
		/**
	     * GetBatchDocumentL
		 * Iterates throught the search result's document list (hits), that is stored in the 
	     * server side. The accessed list is ranked and the best matching document isheading 
	     * the result lest. When the request has been completed a callback to 
	     * MSearchObserver::HandleDocumentL is issued. 
	     *
	     * Leaves with KErrInUse, if asynchronous request is pending and 
	     * KErrNotReady, if no database has been succefullly opened.
	     *
         * @return Next available document or NULL, if no more documents can be found
		 */
		IMPORT_C void GetBatchDocumentL(TInt aIndex, MCPixNextDocumentRequestObserver& aObserver, TInt aCount = 1);
				
	protected: // Functions from base classes
	
	    /**
	    * From CActive, RunL.
	    * Callback function.
	    * Invoked to handle responses from the server.
	    */
	    void RunL();
	
	    /**
	    * From CActive, DoCancel.
	    * Cancels any outstanding operation.
	    */
	    void DoCancel();

	    /**
	    * From CActive, RunError.
	    * Don't propagate error.
	    */
	    TInt RunError(TInt aError);

	protected: // Supporting functionality
		
		/**
		 * Used to form a CLucene query targeting the field identified by 'aField'
		 */
		HBufC* FormQueryStringL(const TDesC& aQueryString, const TDesC& aField); 

	private: // Constructors and destructors
	
	    /**
	    * CCPixSearcher.
	    * Performs the first phase of two phase construction.
	    */
		CCPixSearcher(RSearchServerSession& aSearchServerSession);
	
	    /**
	    * ConstructL.
	    * Performs the second phase construction of a
	    * CCPixSearcher object.
	    */
	    void ConstructL(const TDesC& aDefaultSearchField);
	
	private: // Data
		
		enum TState 
			{ 
			EStateNone = 0,
			EStateOpenDatabase, 
			EStateSearch, 
			EStateGetDocument,
			EStateGetBatchDocument,
			EStateSetAnalyzer,
			EStateSetQueryParser
			};
		
		union TObserver 
			{
			void* iAny; 
			MCPixOpenDatabaseRequestObserver* iOpenDatabase; 
			MCPixSearchRequestObserver* iSearch; 
			MCPixNextDocumentRequestObserver* iNextDocument; 
			MCPixSetAnalyzerRequestObserver* iSetAnalyzer;
			MCPixSetQueryParserRequestObserver* iSetQueryParser;
			};
		
	private:
		
		/**
		 * iState, the internal state
		 */
		TState iState;
		
		/**
		 * State for checking whether database is open
		 */
		TBool iIsDatabaseOpen; 
	
	    /**
	    * iSession, the time server session.
	    */
		RSearchServerSession& iSearchServerSession;
	    RSearchServerSubSession iSubSession;

	    /**
	     * Stores the assigned default search field for future OpenDatabase
	     * requests
	     */
	    HBufC* iDefaultSearchField;
	    /**
	     * Stores asynchronous request argument until the request 
	     * has finished
	     */
	    HBufC* iQueryString;

	    /**
	     * iEstimatedResultsCount, contains the estimated results count.
	     */
	    TInt iEstimatedResultsCount;
	
	    /**
	    * iObserver, observer which handles updates from the server.
	    */
	    TObserver iObserver;
	    
	    /**
	     * Used to store the base application class
	     */
	    HBufC* iBaseAppClass; 	    
	    
	    /**
	     * Used to store the analyzer definition
	     */
	    HBufC* iAnalyzer; 

	};


#endif //__CCPIXSEARCHER_H__

// End of File
