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


#ifndef LOGPLAYERMANAGER_H
#define LOGPLAYERMANAGER_H

//  Include Files
#include <e32base.h>
#include <s32file.h>
#include "RSearchServersession.h"
#include "MCPixSearcherObserver.h"
#include "LogPlayerTimer.h"

// Forward declarations
class CConsoleBase;
class CLogPlayerManager;
class CCPixIndexer;
class CCPixSearcher;

// Constants
const TChar KLogPlayerFieldSeparator = TChar(' ');
const TChar KLogPlayerFieldSeparatorEndLine = TChar('\n');
const TUint KLogPlayerMaxTextLength = 2048;



// Struct containing information related to Log Player Manager
// handling single handle
struct THandleInformation
{
    // Log data file name
	TFileName iLogFileName;
	
	// Position in file
	TInt iFilePosition;
	
	// Handle Id this Manger is responsible
	TInt iHandleId;

	// Start time of log execution
	TTime iStartTime;
};

class CLogPlayerManager : public CBase, public MLogPlayerTimeCallback, public MCPixSearchRequestObserver, public MCPixNextDocumentRequestObserver
{
public : // Constructors and destructors

    /**
    * NewL.
    * Two-phased constructor.
    * @param aLogFileName Name of log data file
    * @return Pointer to created CLogPlayerManager object.
    */
    static CLogPlayerManager* NewL( const TDesC& aLogFileName );

    /**
    * NewLC.
    * Two-phased constructor.
    * @param aLogFileName Name of log data file
    * @return Pointer to created CLogPlayerManager object.
    */
    static CLogPlayerManager* NewLC( const TDesC& aLogFileName );

    /**
    * Destructor.
    */
    virtual ~CLogPlayerManager();

public: // From MLogPlayerTimeCallback
	
	void TimerCallback();
	
public: // From MCPixSearchRequestObserver
    
    // Dummy implementation
    void HandleSearchResultsL(TInt /*aError*/, TInt /*aEstimatedResultCount*/) {}

public: // From MCPixNextDocumentRequestObserver
    
    // Dummy implementation
    void HandleDocumentL(TInt /*aError*/, CSearchDocument* /*aDocument*/) {}

public: // New functions
    
    /**
     * Simulate execution from stored log file
	 * @param aConsole Console
     * @param aPosition Starting position in file
     * @param aHandle Handle number. If -1 then handle execution as master. Master will 
     * 				  handle only lines without line numbers and create new handler threads
     * @param aStartTime Start time of log execution
     */
    void ExecuteLogL(CConsoleBase* aConsole,
    		         TInt aPosition=0,
    		         TInt aHandle=-1,
    		         const TTime& aStartTime=0 );

    /**
     * Continue Log Player execution
     */
    void ContinueExecutionL();
    
private: // New functions
    
    TInt ReadNumberL( RReadStream& aReadStream, TChar aSeparator = KLogPlayerFieldSeparator );
    
    void ReadTextL( RReadStream& aReadStream, TDes8& aCommand, TChar aSeparator = KLogPlayerFieldSeparator );

    void ReadTextL( RReadStream& aReadStream, TDes8& aCommand, TInt aLength );
    
    TInt64 ReadTimeStampL( RReadStream& aReadStream );

    /**
     * Read flush command
     * @param aReadStream Read stream
     * @param aIndex On return contain handle index
     */
    void ReadFlushL(RFileReadStream& aReadStream, TInt& aIndex);

    /**
     * Read add command
     * @param aReadStream Read stream
     * @param aIndex On return contain handle index
     * @return Pointer to CSearchDocument. Transfer ownership.
     */
    CSearchDocument* ReadAddL(RFileReadStream& aReadStream, TInt& aIndex);

    /**
     * Read delete command
     * @param aReadStream Read stream
     * @param aIndex On return contain handle index
     * @return Doc uid. Transfer ownership.
     */
    HBufC* ReadDeleteL(RFileReadStream& aReadStream, TInt& aIndex);

    /**
     * Read search command
     * @param aReadStream Read stream
     * @param aIndex On return contain handle index
     * @return Search query. Transfer ownership.
     */
    HBufC* ReadSearchL(RFileReadStream& aReadStream, TInt& aIndex);
    
    /**
     * Read get command
     * @param aReadStream Read stream
     * @param aIndex On return contain handle index
     * @return Doc index number
     */
    TInt ReadGetL(RFileReadStream& aReadStream, TInt& aIndex);

    /**
     * Read cancel command
     * @param aReadStream Read stream
     * @param aIndex On return contain handle index
     */
    void ReadCancelL(RFileReadStream& aReadStream, TInt& aIndex);
    
    /**
     * Read housekeeping command
     * @param aReadStream Read stream
     */
    void ReadHouseKeepingL(RFileReadStream& aReadStream);

    /**
     * Read cancel command
     * @param aReadStream Read stream
     * @param aIndex On return contain handle index
     * @param aIsSearcher On return ETrue if command for searcher, EFalse if for indexer
     * @return BaseAppClass. Transfer ownership.
     */
    HBufC* ReadOpenDbL(RFileReadStream& aReadStream, TInt& aIndex, TBool& aIsSearcher);

    /**
     * Read reset command
     * @param aReadStream Read stream
     * @param aIndex On return contain handle index
     */
    void ReadResetL(RFileReadStream& aReadStream, TInt& aIndex);

    /**
     * Check if handler for given index is already created
     * @param aIndex Index to be checked
     * @return ETrue if handler is created for given index, EFalse otherwise
     */
    TBool IsHandlerCreated(TInt aIndex);

    /**
     * Read time from log file and wait until correct time
     */
    void ReadTimeAndWait();
    
private: // Constructors and destructors

    /**
    * C++ default constructor.
    * @param aLogFileName Name of log data file
    */
    CLogPlayerManager( const TDesC& aLogFileName );

    /**
    * ConstructL.
    * 2nd phase constructor.
    */
    void ConstructL();

private: // Data

	// Synchronization
	CActiveSchedulerWait* iWait;
	
    // Buffer for reading from file stream
    TBuf8<KLogPlayerMaxTextLength> iTextBuffer;

    // Array of handle specific information. Only relevant to master thread.
    RArray<THandleInformation> iHandleInformationArray;

    // Timer for handling command execution timing
    CLogPlayerTimer* iLogPlayerTimer;

    // Search server session
    RSearchServerSession iSession;

    // Indexer
    CCPixIndexer* iIndexer;
  
    // Searcher
    CCPixSearcher* iSearcher;

    // File server session
    RFs iFs;
    
    // File handle
    RFile iFile;

    // File read stream
    RFileReadStream iReadStream;
    
    // Handle number for this Manager
    TInt iHandle;

    // Start position of stream for new thread 
    TStreamPos iStartPosition;
    
    // Start time of log execution
    TTime iStartTime;
    
    // Console
    CConsoleBase* iConsole;
    
    // Log data file name
   TFileName iLogFileName;
};

#endif  // LOGPLAYERMANAGER_H

