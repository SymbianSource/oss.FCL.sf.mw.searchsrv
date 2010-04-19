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
* Description: This file contains testclass implementation.
*
*/

// [INCLUDE FILES] - do not remove
#include "CPixMWTester.h"

#include <e32svr.h>
#include <StifParser.h>
#include <Stiftestinterface.h>

#include "CBlacklistMgr.h"

// EXTERNAL DATA STRUCTURES
//extern  ?external_data;

// EXTERNAL FUNCTION PROTOTYPES  
//extern ?external_function( ?arg_type,?arg_type );

// CONSTANTS
//const ?type ?constant_var = ?constant;
_LIT( KNoErrorString, "No Error" );
//_LIT( KErrorString, " *** Error ***" );
//Test Uid for testing Blacklist manager
const TUid KTestUid = { 0x101D6348 };

//For Watchdog
_LIT(KTestHarvesterServer,"CPixHarvesterServer");
_LIT(aEXeFileName , "WatchDog.exe");

// MACROS
//#define ?macro ?macro_def

// LOCAL CONSTANTS AND MACROS
//const ?type ?constant_var = ?constant;
//#define ?macro_name ?macro_def

// MODULE DATA STRUCTURES
//enum ?declaration
//typedef ?declaration

// LOCAL FUNCTION PROTOTYPES
//?type ?function_name( ?arg_type, ?arg_type );

// FORWARD DECLARATIONS
//class ?FORWARD_CLASSNAME;

// ============================= LOCAL FUNCTIONS ===============================

void doLog( CStifLogger* logger, TInt error, const TDesC& errorString )
    {
    if( KErrNone == error ) logger->Log( KNoErrorString );
    else logger->Log( errorString );
    }
// -----------------------------------------------------------------------------
// ?function_name ?description.
// ?description
// Returns: ?value_1: ?description
//          ?value_n: ?description_line1
//                    ?description_line2
// -----------------------------------------------------------------------------
//
/*
?type ?function_name(
    ?arg_type arg,  // ?description
    ?arg_type arg)  // ?description
    {

    ?code  // ?comment

    // ?comment
    ?code
    }
*/

// ============================ MEMBER FUNCTIONS ===============================

// -----------------------------------------------------------------------------
// CCPixMWTester::Delete
// Delete here all resources allocated and opened from test methods. 
// Called from destructor. 
// -----------------------------------------------------------------------------
//
void CCPixMWTester::Delete() 
    {

    }

// -----------------------------------------------------------------------------
// CCPixMWTester::RunMethodL
// Run specified method. Contains also table of test mothods and their names.
// -----------------------------------------------------------------------------
//
TInt CCPixMWTester::RunMethodL( 
    CStifItemParser& aItem ) 
    {

    static TStifFunctionInfo const KFunctions[] =
        {  
        // Copy this line for every implemented function.
        // First string is the function name used in TestScripter script file.
        // Second is the actual implementation member function.        
        ENTRY( "TestBlacklistPlugin", CCPixMWTester::TestBlacklistPluginL ),
        ENTRY( "TestBlacklistPluginVersion", CCPixMWTester::TestBlacklistPluginVersionL ),
        ENTRY( "TestWatchdog",CCPixMWTester::TestWatchdogL ),  
        //ADD NEW ENTRY HERE
        // [test cases entries] - Do not remove

        };

    const TInt count = sizeof( KFunctions ) / 
                        sizeof( TStifFunctionInfo );

    return RunInternalL( KFunctions, count, aItem );

    }

// -----------------------------------------------------------------------------
// CCPixMWTester::TestBlacklistPluginL
// -----------------------------------------------------------------------------
//
TInt CCPixMWTester::TestBlacklistPluginL( CStifItemParser& aItem )
    {    
    TInt err = KErrNone;        
    CBlacklistMgr* blacklistmanager = CBlacklistMgr::NewL();
    CleanupStack::PushL( blacklistmanager );
    TInt version = 0;
    aItem.GetNextInt(version);
    //Add an Uid to Blacklist DB
    blacklistmanager->AddL( KTestUid , version );
    //Check if the Uid is added to database or not
    TBool found = blacklistmanager->FindL(KTestUid , version );
    
    if(!found) err = KErrNotFound;
    //clear the UID from the database
    blacklistmanager->Remove(KTestUid);
    CleanupStack::PopAndDestroy( blacklistmanager ); 
    doLog( iLog, err, KNoErrorString );        
    return err;
    }

// -----------------------------------------------------------------------------
// CCPixMWTester::TestBlacklistPluginVersionL
// -----------------------------------------------------------------------------
//
TInt CCPixMWTester::TestBlacklistPluginVersionL( CStifItemParser& aItem )
    {
    TInt err = KErrNone;   
    CBlacklistMgr* blacklistmanager = CBlacklistMgr::NewL();
    CleanupStack::PushL( blacklistmanager );
    TInt oldversion = 0;
    TInt newversion = 0;
    aItem.GetNextInt(oldversion);
    aItem.GetNextInt(newversion);
    //Add an Uid to Blacklist DB with old version
    blacklistmanager->AddL( KTestUid , oldversion );
    //Add an Uid to Blacklist DB with new version
    blacklistmanager->AddL( KTestUid , newversion );
    //Check if the Uid with old version exists
    TBool found = blacklistmanager->FindL(KTestUid , oldversion );
    if( found )
        {
        err = KErrNotFound;
        }
    else
        {
        //check with new version
        found = blacklistmanager->FindL(KTestUid , newversion );
        if(!found) err = KErrNotFound;
        }    
    //clear the UID from the database
    blacklistmanager->Remove(KTestUid);
    CleanupStack::PopAndDestroy( blacklistmanager );    
    doLog( iLog, err, KNoErrorString );
    return err;
    }

// -----------------------------------------------------------------------------
// CCPixMWTester::TestWatchdogL
// -----------------------------------------------------------------------------
//
TInt CCPixMWTester::TestWatchdogL( CStifItemParser&)
    {
    TInt err = KErrNone;
    //Start the watchdog exe 
    RProcess p;
    //CleanupStack::PushL(&p);
    TInt res=p.Create(aEXeFileName,KNullDesC);
    if(res==KErrNone)
        {
        p.Resume();
        }
    //wait for a minute.
    User::After((TTimeIntervalMicroSeconds32)120000000);
    //check for the CPixHarvesterServer
    //If server exists, Test case  is PASS else FAIL
    TFindServer harvesterServer(KTestHarvesterServer);
    TFullName name;
    err = harvesterServer.Next(name);
    p.Close();
    doLog( iLog, err, KNoErrorString );
    return err;
    }
// -----------------------------------------------------------------------------
// CCPixMWTester::?member_function
// ?implementation_description
// (other items were commented in a header).
// -----------------------------------------------------------------------------
//
/*
TInt CCPixMWTester::?member_function(
   CItemParser& aItem )
   {

   ?code

   }
*/

// ========================== OTHER EXPORTED FUNCTIONS =========================
// None

//  [End of File] - Do not remove
