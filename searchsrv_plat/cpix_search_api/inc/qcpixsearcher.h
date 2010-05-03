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
* Description:  Qt search APIs
*
*/

#ifndef _QCPIXSEARCHER_H
#define _QCPIXSEARCHER_H

//Uncomment the following line to enable performance measurements
//#define OST_TRACE_COMPILER_IN_USE

#ifdef OST_TRACE_COMPILER_IN_USE

#include <qdatetime.h>
#include <qdebug.h>
#define PERF_SEARCH_START_TIMER  searchTimer.start();
#define PERF_SEARCH_RESTART_TIMER searchTimer.restart();
#define PERF_SEARCH_ENDLOG qDebug() << "Search QT API took: " << searchTimer.elapsed() << "msec";

#define PERF_GETDOC_START_TIMER  getDocumentTimer.start();
#define PERF_GETDOC_RESTART_TIMER getDocumentTimer.restart();
#define PERF_GETDOC_ENDLOG qDebug() << "Search QT API took: " << getDocumentTimer.elapsed() << "msec";

#define PERF_TIME_NOW(message) qDebug() << "Search QT API: " << QString(message) << ": " << QTime::currentTime().toString("hh:mm:ss.zzz");

#else 

#define PERF_SEARCH_START_TIMER  
#define PERF_SEARCH_RESTART_TIMER 
#define PERF_SEARCH_ENDLOG 
#define PERF_GETDOC_START_TIMER
#define PERF_GETDOC_RESTART_TIMER
#define PERF_GETDOC_ENDLOG
#define PERF_TIME_NOW(message)

#endif

/**
 * @file
 * @ingroup Search Client API fpr Qt Clients
 * @brief Contains CCPixSearcher used for searching
 */

#ifdef BUILD_DLL
#define DLL_EXPORT Q_DECL_EXPORT
#else
#define DLL_EXPORT Q_DECL_IMPORT
#endif

#include <QObject>
#include <qcpixcommon.h>

//forward declarations
class QCPixDocument;
class QCPixSearcherPrivate;

// CLASS DECLARATION
/**
 * @brief Used for searching.
 * @ingroup Qt Search Client API
 * Link against: qcpixsearchclient.lib 
 * 
 * An instance of QCPixSearcher is used to commit search operations.
 * 
 * Example code:
 * 
 * Usecase 1: Sync calls. 
 * \code
 * QCPixSearcher* searcher = QCPixSearcher::newInstance("root");
 * if(searcher){
 *		int hitCount = searcher->Search("search for me");
 *		for(int i=0; i<hitCount; i++) {
 *		QCPixDocument* doc = GetDocument(0);
 *		// do something with doc.
 *		delete doc;
 *		}
 *	}
 * \endcode
 * 
 * Usecase 2: Sync calls with explicit SetDatabase().
 * \code
 * QCPixSearcher* searcher = QCPixSearcher::newInstance();
 * searcher->SetDatabase("root");
 * int hitCount = searcher->Search("search for me");
 * for(int i=0; i<hitCount; i++) {
 *		try{
 *			QCPixDocument* doc = GetDocument(i);
 *			// do something with doc.
 *			delete doc;
 *		catch(...){
 *      //Do Cleanup
 *		}
 * }
 * \endcode
 * 
 * Usecase 3: Async Calls
 * \code
 * 
 * iCurrentDocumentCount = 0;
 * 
 * QCPixSearcher* searcher = QCPixSearcher::newInstance("root");
 * connect(searcher, SIGNAL(handleSearchResults(int,int)), this, SLOT(ClientHandleSearchCompleteSlot(int,int)) );
 * connect(searcher, SIGNAL(handleDocument(int,QCPixDocument*)), this, SLOT(ClientHandleGetDocumentCompleteSlot(int,QCPixDocument*)) );
 * int hitCount = searcher->Search("search for me");
 * GetDocumentAsync( iCurrentDocumentCount++ );
 * 
 * ClientClass::ClientHandleGetDocumentCompleteSlot(int aError, QCPixDocument* aDocument){
 *  if( KErrNone != aError ){
 *  //do something with aDocument
 *  }
 *  GetDocumentAsync( iCurrentDocumentCount++ ); //Now get the next document.
 * }
 * 
 * \endcode
 * 
 */
class DLL_EXPORT QCPixSearcher: public QObject
    {
    Q_OBJECT
public:
    /**
       * Constructor.
       * Creates a QCPixSearcher object and return a pointer to the created object.
       * @return A pointer to the created instance of CCPixSearcher.
       * 
       * @note After using this constructor, the client has to mandatorily call 
       * SetDatabase() before invoking any search.
       */
    static QCPixSearcher* newInstance();
    
    /**
     * Overloaded constructor
     * Creates a CCPixSearcher object and return a pointer to the created object.
     * If this constructor is used, the client can directly invoke Search without
     * the need to call SetDatabase.
     * @param aBaseAppClass The baseAppClass on which to invoke searches on.
     * @param aDefaultSearchField Default field where the keywords are searched from.
     * @return A pointer to the created instance of CCPixSearcher.
     */
    static QCPixSearcher* newInstance( QString aBaseAppClass, QString aDefaultSearchField=NULL );

    /**
     * Destructor. 
     */
    ~QCPixSearcher();
    
    /**
     * Synchronously set (or change the database, if already set) on which to invoke subsequent searches.
     * @param aBaseAppClass baseAppClass whose corresponding database is to be opened.
     */
    void setDatabase(QString aBaseAppClass) THROWS_EXCEPTION;

    /**
     * Asynchronously set (or change the database, if already set) on which to invoke subsequent searches.
     * @param aBaseAppClass baseAppClass whose corresponding database is to be opened.
     *
     * @note Client is notified on completion of this call via HandleDatabaseSet signal.
     */
    void setDatabaseAsync(QString aBaseAppClass) THROWS_EXCEPTION;

    /**
     * Syncronously search for aSearchString.
     * @param aSearchString keywords to be searched for.
     * @param aDefaultSearchField Default field where the keywords are searched from.
     * @return Estimated number of documents containing aSearchString.
     */
    int search(QString aSearchString, QString aDefaultSearchField=NULL) THROWS_EXCEPTION;

    /**
     * Asyncronously search for aSearchString.
     * @param aSearchString keywords to be searched for.
     * @param aDefaultSearchField Default field where the keywords are searched from.
     * @return Estimated number of documents containing aSearchString.
     *
     * @note Client is notified on completion of this call via HandleSearchResults signal.
     */
    void searchAsync(QString aSearchString, QString aDefaultSearchField=NULL) THROWS_EXCEPTION;

    /**
     * Synchronously get the document with index aIndex.
     * @param aIndex Index of document to be retrieved
     * @return A pointer to QCPixDocument that has been retrieved. Null on error.
     *
     * @note This should be called only after the synchronous search call has returned
     * 		and aIndex should be between 0 and estimated count returned by Search().
     */		
    QCPixDocument* getDocument(int aIndex) THROWS_EXCEPTION;

    /**
     * Asynchronously get the document with index aIndex.
     * @param aIndex Index of document to be retrieved
     * @return A pointer to QCPixDocument that has been retrieved. Null on error.
     *
     * @note This should be called only after the synchronous search call has returned
     * 		and aIndex should be between 0 and estimated count returned by Search().
     */		
    void getDocumentAsync(int aIndex) THROWS_EXCEPTION;
    
    /**
     * Cancels any outstanding searches.
     */
    void cancelSearch();

signals:
    /**
     * Notify completion of SetDatabaseAsyc
     * @param aError Completion (error) code of SetDatabaseAsync
     */
    void handleDatabaseSet(int aError);

    /**
     * Notify completion of SearchAsyc
     * @param aError Completion (error) code of SearchAsyc
     * @param aEstimatedResultCount Estimated number of documents found after SearchAsync
     */		
    void handleSearchResults(int aError, int aEstimatedResultCount);

    /**
     * Notify completion of GetDatabaseAsyc
     * @param aError Completion (error) code of GetDatabaseAsyc
     * @param aDocument The requested document.
     */		
    void handleDocument(int aError, QCPixDocument* aDocument);

private:
    /**
     * Default Constructor.
     */
    QCPixSearcher();
    
    /**
       * Constructor.
       * Creates a QCPixSearcher object and return a pointer to the created object.
       * @param aDefaultSearchField Default field where the keywords are searched from.
       * @return A pointer to the created instance of CCPixSearcher.
       * 
       * @note After using this constructor, the client has to mandatorily call 
       * SetDatabase() before invoking any search.
       */
    QCPixSearcher( QString aDefaultSearchField=NULL );
    
    QCPixSearcherPrivate* const iPvtImpl;
    Q_DECLARE_PRIVATE_D( iPvtImpl, QCPixSearcher )
    
#ifdef OST_TRACE_COMPILER_IN_USE
    QTime searchTimer; 
    QTime getDocumentTimer;
#endif
    };

#endif //_QCPIXSEARCHER_H
