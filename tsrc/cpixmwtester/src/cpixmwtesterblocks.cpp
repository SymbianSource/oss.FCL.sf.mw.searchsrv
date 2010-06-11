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
#include <cpixcontentinfocommon.h>
#include <sqldb.h>
#include "CBlacklistMgr.h"
#include "contentinfomgr.h"
#include "ccontentinfo.h"

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
_LIT(KDriveC, "c:");
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

void getcontentstatus ( TPtrC aContent , TInt& aBLstatus ,TInt& aINstatus)
    {
    RSqlDatabase sqlDB;
    RFs fssession;    
    User::LeaveIfError( fssession.Connect() );
    TFileName privatePath;
    TFileName datafile;    
    fssession.CreatePrivatePath(EDriveC);
    fssession.PrivatePath(privatePath);//data caged path of loading process 
    fssession.Close();
    datafile.Copy(KDriveC);
    datafile.Append(privatePath);    
    datafile.Append( KContentInfoFileName );
    TInt err = sqlDB.Open( datafile );
    
    if ( err  ==KErrNone )
        {
        TSqlScalarFullSelectQuery fullSelectQuery(sqlDB);

        TBuf<100> sql;        
        // Query with INS column 
        _LIT(KgetINstatusSqlFormat, "SELECT INS FROM table1 WHERE NAME = '");
        _LIT(Kendtag, "'");
        sql.Copy( KgetINstatusSqlFormat);
        sql.Append( aContent);
        sql.Append( Kendtag );
        //sql.Format( KgetINstatusSqlFormat , aContent );
        // Read INS as integer.
        aINstatus = fullSelectQuery.SelectIntL(sql);
        
        _LIT(KgetBLstatusSqlFormat, "SELECT BLS FROM table1 WHERE NAME = '");
        sql.FillZ();
        sql.Copy( KgetBLstatusSqlFormat);
        sql.Append( aContent);
        sql.Append( Kendtag );
        //sql.Format( KgetBLstatusSqlFormat , aContent );
        // Read BLS as integer.
        aBLstatus = fullSelectQuery.SelectIntL(sql);
        }
    sqlDB.Close();
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
        ENTRY( "TestDeleteContentInfoDB",CCPixMWTester::TestDeleteContentInfoDBL ),
        ENTRY( "TestAddContent",CCPixMWTester::TestAddContentL ),
        ENTRY( "TestRemoveContent",CCPixMWTester::TestRemoveContentL ),
        ENTRY( "TestResetContent",CCPixMWTester::TestResetContentL ),
        ENTRY( "TestUpdateBLStatus",CCPixMWTester::TestUpdateBLStatusL ),
        ENTRY( "TestUpdateINStatus",CCPixMWTester::TestUpdateINStatusL ),
        ENTRY( "TestAddUnloadlist",CCPixMWTester::TestAddUnloadlistL ),
        ENTRY( "TestRemovefromUnloadlist",CCPixMWTester::TestRemovefromUnloadlistL ),
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
TInt CCPixMWTester::TestWatchdogL( CStifItemParser& /*aItem*/)
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
// CCPixMWTester::TestDeleteContentInfoDBL
// -----------------------------------------------------------------------------
//
TInt CCPixMWTester::TestDeleteContentInfoDBL( CStifItemParser& /*aItem*/)
    {
    RFs fssession;
    CContentInfoMgr* contentInfoMgr = NULL;
    User::LeaveIfError( fssession.Connect() );
    TFileName privatePath;
    TFileName datafile;    
    fssession.CreatePrivatePath(EDriveC);
    fssession.PrivatePath(privatePath);//data caged path of loading process    
    datafile.Copy(KDriveC);
    datafile.Append(privatePath);    
    datafile.Append( KContentInfoFileName );
    //delete the database file
    fssession.Delete( datafile );
    TRAPD ( err , contentInfoMgr = CContentInfoMgr::NewL() );
    delete contentInfoMgr;
    doLog( iLog, err, KNoErrorString );
    return err;
    }
// -----------------------------------------------------------------------------
// CCPixMWTester::TestAddContentL
// -----------------------------------------------------------------------------
//
TInt CCPixMWTester::TestAddContentL( CStifItemParser& aItem)
    {
    RFs fssession;
    TPtrC content;
    TInt err = KErrNone;
    aItem.GetNextString( content );
    CContentInfoMgr* contentInfoMgr = CContentInfoMgr::NewL();
    CContentInfo* contentinfo = CContentInfo::NewL();
    //Add the content with given content name and 0 as BL status and 1 as IN status
    contentinfo->SetNameL( content );
    contentinfo->SetBlacklistStatus( 0 );
    contentinfo->SetIndexStatus( 1 );
    contentInfoMgr->AddL( contentinfo );
    delete contentinfo;
    //TBuf<50> name;
    //name.Copy( content.Ptr() );
    //Find if the content exists
    TBool found = contentInfoMgr->FindL( content );
    delete contentInfoMgr;
    if ( !found ) err = KErrNotFound;
    doLog( iLog, err, KNoErrorString );
    return err;
    }
// -----------------------------------------------------------------------------
// CCPixMWTester::TestRemoveContentL
// -----------------------------------------------------------------------------
//
TInt CCPixMWTester::TestRemoveContentL( CStifItemParser& aItem)
    {
    RFs fssession;
    TPtrC content;
    TInt err = KErrNotFound;
    aItem.GetNextString( content );
    CContentInfoMgr* contentInfoMgr = CContentInfoMgr::NewL();
    contentInfoMgr->ResetL();
    CContentInfo* contentinfo = CContentInfo::NewL();
    //Add the content with given content name and 0 as BL status and 1 as IN status
    contentinfo->SetNameL( content );
    contentinfo->SetBlacklistStatus( 0 );
    contentinfo->SetIndexStatus( 1 );
    contentInfoMgr->AddL( contentinfo );
    delete contentinfo;
    //check if the added content exists
    TBool found = contentInfoMgr->FindL( content );
    if ( found )
        {
        //remove the content from DB and find it
        contentInfoMgr->RemoveL( content );
        found = contentInfoMgr->FindL( content );
        if ( !found ) err = KErrNone;
        }
    delete contentInfoMgr;    
    doLog( iLog, err, KNoErrorString );
    return err;
    }
// -----------------------------------------------------------------------------
// CCPixMWTester::TestResetContentL
// -----------------------------------------------------------------------------
//
TInt CCPixMWTester::TestResetContentL( CStifItemParser& aItem)
    {
    RFs fssession;
    TPtrC content;
    TInt err = KErrNotFound;
    aItem.GetNextString( content );
    CContentInfoMgr* contentInfoMgr = CContentInfoMgr::NewL();
    CContentInfo* contentinfo = CContentInfo::NewL();
    //Add the content with given content name and 0 as BL status and 1 as IN status
    contentinfo->SetNameL( content );
    contentinfo->SetBlacklistStatus( 0 );
    contentinfo->SetIndexStatus( 1 );
    contentInfoMgr->AddL( contentinfo );
    delete contentinfo;
    //make sure there is some content exists in the database
    TInt count = contentInfoMgr->GetContentCountL();
    if ( count )
        {
        //reset the DB and get the count . the count should be 0
        contentInfoMgr->ResetL();
        count = contentInfoMgr->GetContentCountL( );
        if ( !count ) err = KErrNone;
        }
    delete contentInfoMgr;    
    doLog( iLog, err, KNoErrorString );
    return err;
    }
// -----------------------------------------------------------------------------
// CCPixMWTester::TestUpdateBLStatusL
// -----------------------------------------------------------------------------
//
TInt CCPixMWTester::TestUpdateBLStatusL( CStifItemParser& aItem)
    {
    RFs fssession;
    TPtrC content;
    TInt err = KErrNotFound;
    aItem.GetNextString( content );
    CContentInfoMgr* contentInfoMgr = CContentInfoMgr::NewL();
    contentInfoMgr->ResetL();
    CContentInfo* contentinfo = CContentInfo::NewL();
    //Add the content with given content name and 0 as BL status and 1 as IN status
    contentinfo->SetNameL( content );
    contentinfo->SetBlacklistStatus( 0 );
    contentinfo->SetIndexStatus( 1 );
    contentInfoMgr->AddL( contentinfo );
    delete contentinfo;
    //Update the blacklist status to 1
    contentInfoMgr->UpdateBlacklistStatusL( content , 1);
    TInt blstatus =0,instatus = 0;
    getcontentstatus ( content , blstatus , instatus);    
    if ( blstatus ) err = KErrNone;
    delete contentInfoMgr;    
    doLog( iLog, err, KNoErrorString );
    return err;
    }

// -----------------------------------------------------------------------------
// CCPixMWTester::TestUpdateINStatusL
// -----------------------------------------------------------------------------
//
TInt CCPixMWTester::TestUpdateINStatusL( CStifItemParser& aItem)
    {
    RFs fssession;
    TPtrC content;
    TInt err = KErrNotFound;
    aItem.GetNextString( content );
    CContentInfoMgr* contentInfoMgr = CContentInfoMgr::NewL();
    contentInfoMgr->ResetL();
    CContentInfo* contentinfo = CContentInfo::NewL();
    //Add the content with given content name and 0 as BL status and 1 as IN status
    contentinfo->SetNameL( content );
    contentinfo->SetBlacklistStatus( 0 );
    contentinfo->SetIndexStatus( 1 );
    contentInfoMgr->AddL( contentinfo );
    delete contentinfo;
    //Update the Indexing status to 0
    contentInfoMgr->UpdatePluginIndexStatusL( content , 0);
    TInt blstatus = 0,instatus = 1;
    getcontentstatus ( content , blstatus , instatus);    
    if ( !blstatus ) err = KErrNone;
    delete contentInfoMgr;    
    doLog( iLog, err, KNoErrorString );
    return err;
    }

// -----------------------------------------------------------------------------
// CCPixMWTester::TestAddUnloadlistL
// -----------------------------------------------------------------------------
//
TInt CCPixMWTester::TestAddUnloadlistL( CStifItemParser& aItem)
    {
    TInt err = KErrNone;        
    CBlacklistMgr* blacklistmanager = CBlacklistMgr::NewL();
    CleanupStack::PushL( blacklistmanager );
    //Add an Uid to Blacklist DB
    blacklistmanager->AddtoUnloadListL( KTestUid );
    //Check if the Uid is added to database or not
    TBool found = blacklistmanager->FindfromUnloadListL(KTestUid );
    
    if(!found) err = KErrNotFound;
    //clear the UID from the database
    blacklistmanager->RemoveFromUnloadListL(KTestUid);
    CleanupStack::PopAndDestroy( blacklistmanager ); 
    doLog( iLog, err, KNoErrorString );        
    return err;
    }

// -----------------------------------------------------------------------------
// CCPixMWTester::TestRemovefromUnloadlistL
// -----------------------------------------------------------------------------
//
TInt CCPixMWTester::TestRemovefromUnloadlistL( CStifItemParser& aItem)
    {
    TInt err = KErrNotFound;        
    CBlacklistMgr* blacklistmanager = CBlacklistMgr::NewL();
    CleanupStack::PushL( blacklistmanager );
    //Add an Uid to Blacklist DB
    blacklistmanager->AddtoUnloadListL( KTestUid );
    //Check if the Uid is added to database or not
    TBool found = blacklistmanager->FindfromUnloadListL(KTestUid );
    
    if(found)
        {
         //clear the UID from the database
        blacklistmanager->RemoveFromUnloadListL(KTestUid);
        found = blacklistmanager->FindfromUnloadListL(KTestUid );
        if ( !found )  err = KErrNone;
        }
    CleanupStack::PopAndDestroy( blacklistmanager ); 
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
