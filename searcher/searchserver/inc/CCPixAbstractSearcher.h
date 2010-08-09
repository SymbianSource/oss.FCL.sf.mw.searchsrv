/*
 * CCPixAbstractSearcher.h
 *
 *  Created on: Oct 8, 2009
 *      Author: admin
 */

#ifndef CCPIXABSTRACTSEARCHER_H_
#define CCPIXABSTRACTSEARCHER_H_

#include <e32base.h>

#include "cpixmaindefs.h"
#include "cpixdoc.h"

// FORWARD DECLARATIONS
class CSearchDocument;
class CCPixAsyncronizer;
class MCPixAsyncronizerObserver;

_LIT(KCpixDefaultSearchField, CPIX_DEFAULT_FIELD);

class CCPixAbstractSearcher : public CBase
	{
public: 
	
	enum TQueryParser {
		EDatabaseQueryParser = 0,
		EIncrementalQueryParser = 1
	};
	
	
public: 
	
	virtual ~CCPixAbstractSearcher();
	
	/**
	 * Cancel any incomplete asyncronous operation
	 * @param aMessage RMessage2 of CancelAll request
	 */
	virtual void CancelAll(const RMessage2& aMessage) = 0;	
	
	/**
	 * Search given terms from the default field.
	 * @param aSearchTerms Terms to look for.
	 * @param aObserver Observing object for this asyncronous call
	 * @param aMessage The requesting message 
	 * @return ETrue if search was commited, EFalse if it was stop word for example and search was not commited
	 */
	virtual TBool SearchL(const TDesC& aSearchTerms, MCPixAsyncronizerObserver* aObserver, const RMessage2& aMessage) = 0;
	
	/**
	 * Complete previous call to SearchL
	 * @return Count of result documents.
	 */
	virtual TInt SearchCompleteL() = 0;

	/**
	 * Gets document from the current search results.
	 * @param aObserver Observing object for this asyncronous call
	 * @param aMessage The requesting message 
	 * @parma aIndex index of the requested document 
	 */
	virtual void GetDocumentL(TInt aIndex, MCPixAsyncronizerObserver* aObserver, const RMessage2& aMessage) = 0;
	
	/**
	 * Complete the previous GetDocumentL
	 * @return Document. Ownership is transferred to the caller of this function.
	 */
	virtual CSearchDocument* GetDocumentCompleteL() = 0;
	
	/**
	 * Gets list of documents from the current search results.
	 * @param aObserver Observing object for this asyncronous call
	 * @param aMessage The requesting message 
	 * @parma aIndex index of the requested document 
	 */
	virtual void GetBatchDocumentL(TInt aIndex, MCPixAsyncronizerObserver* aObserver, const RMessage2& aMessage,  TInt count) = 0;
	
	/**
	 * Complete the previous GetBatchDocumentL
	 * @return array of Documents. Ownership is transferred to the caller of this function.
	 */
	virtual RPointerArray<CSearchDocument> GetBatchDocumentCompleteL() = 0;

	/**
	 * Creates new database (and destroys existing) if path is given. Otherwise, opens existing database.
	 * @param aDefaultSearchField Default field to which query results are looked from.
	 * @param aBaseAppClass Application class of this database handle.
	 * 	      Defines which database this handle connects to.
	 */
	virtual void OpenDatabaseL(const TDesC& aSearchableId, const TDesC& aDefaultSearchField = KCpixDefaultSearchField) = 0;
	
	virtual void SetQueryParserL( TInt aQueryParser ) = 0; 
	
	/**
	 * IsOpen 
	 * @returns ETrue if the database is currently open
	 */
	virtual TBool IsOpen() = 0;
	
	/**
	 * Sets the analyzer for this searcher. 
	 * 
	 * @param aAnalyzer analyzer definition string. See analyzer definition 
	 * 		  syntax in the documentation
	 */
	virtual void SetAnalyzerL(const TDesC& aAnalyzer) = 0; 
	
public: 
	
	static CSearchDocument* ConvertDocumentL( cpix_Document* aDocument );
	
	static RPointerArray<CSearchDocument> ConvertBatchDocumentL( cpix_Document** aDocument, TInt count ); 
	
	static RArray<TInt> docSizeArray;

	};


#endif /* CCPIXABSTRACTSEARCHER_H_ */
