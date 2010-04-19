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


//  Include Files  

#include "LogPlayerConsole.h"
#include <e32base.h>
#include <e32std.h>
#include <e32cons.h>			// Console

#include "LogPlayerManager.h"

//  Constants

_LIT(KTextConsoleTitle, "Console");
_LIT(KTextFailed, " failed, leave code = %d");
_LIT(KTextPressAnyKey, " [press any key]\n");

//  Global Variables

LOCAL_D CConsoleBase* console; // write all messages to this


//  Local Functions

LOCAL_C void MainL()
    {
    _LIT( KConfigurationFile, "c:\\data\\LPConf.txt" );
    _LIT( KDefaultLogFile, "c:\\data\\LPDefault.txt" );

	console->Printf( _L("Starting application\n") );
    
    // Get log file names from configuration file
    RFs fs;
    User::LeaveIfError( fs.Connect() );
    CleanupClosePushL( fs );

    RFile file;
    TInt err = file.Open( fs, KConfigurationFile, EFileRead );
    CleanupClosePushL( file );
    
    if ( err == KErrNone ) // Read configuration file
    {
    	TInt size;
    	file.Size( size );
    	HBufC8* data = HBufC8::NewLC( size );
    	TPtr8 dataPtr = data->Des();
    	
    	User::LeaveIfError( file.Read( dataPtr ) );
    	
    	HBufC* data16 = HBufC::NewLC( size );
    	data16->Des().Copy( dataPtr );
    	
    	TLex16 lex(*data16);
    	
    	while ( !lex.Eos() )
    	{
    		TPtrC pathName = lex.NextToken();
    		
    	    CLogPlayerManager* manager = CLogPlayerManager::NewLC( pathName );
    		TRAP_IGNORE( manager->ExecuteLogL( console ) );
    		CleanupStack::PopAndDestroy( manager );
    		
    		lex.SkipSpace();
    	}
    	CleanupStack::PopAndDestroy( data16 );
    	CleanupStack::PopAndDestroy( data );
    }
    else // Use default file
    {
	    CLogPlayerManager* manager = CLogPlayerManager::NewLC( KDefaultLogFile()  );
    	TRAP_IGNORE( manager->ExecuteLogL( console ) );
		CleanupStack::PopAndDestroy( manager );
    }
    
    CleanupStack::PopAndDestroy( &file );
    CleanupStack::PopAndDestroy( &fs );
    }

LOCAL_C void DoStartL()
    {
    // Create active scheduler (to run active objects)
    CActiveScheduler* scheduler = new (ELeave) CActiveScheduler();
    CleanupStack::PushL(scheduler);
    CActiveScheduler::Install(scheduler);

    MainL();

    // Delete active scheduler
    CleanupStack::PopAndDestroy(scheduler);
    }

//  Global Functions

GLDEF_C TInt E32Main()
    {
    // Create cleanup stack
    __UHEAP_MARK;
    CTrapCleanup* cleanup = CTrapCleanup::New();

    // Create output console
    TRAPD(createError, console = Console::NewL(KTextConsoleTitle, TSize(
            KConsFullScreen, KConsFullScreen)));
    if (createError)
        return createError;

    // Run application code inside TRAP harness, wait keypress when terminated
    TRAPD(mainError, DoStartL());
    if (mainError)
        console->Printf(KTextFailed, mainError);
    console->Printf(KTextPressAnyKey);
    console->Getch();

    delete console;
    delete cleanup;
    __UHEAP_MARKEND;
    return KErrNone;
    }

