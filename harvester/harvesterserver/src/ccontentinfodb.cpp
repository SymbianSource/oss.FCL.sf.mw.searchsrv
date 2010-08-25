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

#include "ccotentinfodb.h"
#include "ccontentinfo.h"
#include <cpixcontentinfocommon.h>
#include "OstTraceDefinitions.h"
#ifdef OST_TRACE_COMPILER_IN_USE
#include "ccontentinfodbTraces.h"
#endif


// The max length for creating sql query for KCISqlFormatSeek format
const TInt KCISqlStringMaxLength(100);
//SQL query to fetch the records with given Plugin name
_LIT(KCISqlFormatSeek , "SELECT * FROM table1 WHERE NAME='%S'");
//SQL query to delete the records with given plugin name
_LIT(KCISqlDelete, "DELETE FROM table1 WHERE NAME='%S'");
//Syntax for adding a row to sql database
_LIT(KInsertRowFormat,"INSERT INTO table1(NAME,INS,BLS) values('%S',%d,%d)");
//syntax to retrieve all the rows from the sql database
_LIT(KSelectAllRowsFormat,"SELECT * FROM table1");
//syntax to update Index status for a given plugin
_LIT(KUpdateISFormat,"UPDATE table1 SET INS=%d WHERE NAME = '%S'");
//syntax to update Blacklist status for a given plugin
_LIT(KUpdateBSFormat,"UPDATE table1 SET BLS=%d WHERE NAME = '%S'");
_LIT(KDriveC, "c:");
//syntax for removing all the records in the table
_LIT(KRemoveAll, "DELETE FROM table1" );
//syntax for creating table with plugin name, indexing status and blacklist status as the columns
_LIT(KCreateTableFormat,"CREATE TABLE table1(NAME TEXT NOT NULL, INS INTEGER, BLS INTEGER, PRIMARY KEY(NAME))");

// -----------------------------------------------------------------------------
// CContentInfoDb::NewL()
// -----------------------------------------------------------------------------
//
CContentInfoDb* CContentInfoDb::NewL()
    {
    OstTraceFunctionEntry0( CCONTENTINFODB_NEWL_ENTRY );
    CContentInfoDb* instance = CContentInfoDb::NewLC();
    CleanupStack::Pop( instance );
    OstTraceFunctionExit0( CCONTENTINFODB_NEWL_EXIT );
    return instance;
    }

// -----------------------------------------------------------------------------
// CContentInfoDb::NewLC()
// -----------------------------------------------------------------------------
//
CContentInfoDb* CContentInfoDb::NewLC()
    {
    OstTraceFunctionEntry0( CCONTENTINFODB_NEWLC_ENTRY );
    CContentInfoDb* instance = new (ELeave) CContentInfoDb();
    CleanupStack::PushL( instance );
    instance->ConstructL();
    OstTraceFunctionExit0( CCONTENTINFODB_NEWLC_EXIT );
    return instance;
    }

// -----------------------------------------------------------------------------
// CContentInfoDb::CContentInfoDb()
// -----------------------------------------------------------------------------
//
CContentInfoDb::CContentInfoDb()
    {
    //Do the necessary initialisation
    iOpened = EFalse;
    }

// -----------------------------------------------------------------------------
// CContentInfoDb::~CContentInfoDb
// -----------------------------------------------------------------------------
//
CContentInfoDb::~CContentInfoDb()
    {
    iDatabase.Close();
    }

// -----------------------------------------------------------------------------
// CBlacklistDb::ConstructL()
// -----------------------------------------------------------------------------
//
void CContentInfoDb::ConstructL()
    {
    OstTraceFunctionEntry0( CCONTENTINFODB_CONSTRUCTL_ENTRY );
    //Open the sql database.If doesn't exist,create the sql database and open it  
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
    
    TInt err = iDatabase.Open( datafile );
    
    switch ( err )
        {
        case KErrNone:
            iOpened = ETrue;
            break;
            
        case KErrNotFound:
        case KErrPathNotFound:
            {
            //Create the database
            TRAPD( error , CreateDBL() );
            
            if ( error == KErrNone )
                {                
                iOpened = ETrue;
                }
            }
            break;
            
        default:
            break;
        }
    
    OstTraceFunctionExit0( CCONTENTINFODB_CONSTRUCTL_EXIT );
    return;
    }

// -----------------------------------------------------------------------------
// CContentInfoDb::AddL()
// -----------------------------------------------------------------------------
//
TInt CContentInfoDb::AddL( CContentInfo* aContentInfo )
    {
    OstTraceFunctionEntry0( CCONTENTINFODB_ADDL_ENTRY );
    //Add the item record to database
    
    if ( !iOpened )
        return KErrNotReady;
    
    //Prepare the sql
    TBuf<KCISqlStringMaxLength> sql;
    HBufC* contentname = aContentInfo->GetNameL();
    
    TInt indexstatus = aContentInfo->GetIndexStatus();
    TBool blstatus = aContentInfo->GetBlacklistStatus();
    sql.Format( KInsertRowFormat , contentname , indexstatus , blstatus );
    
    TInt err = iDatabase.Exec(sql) ;
    
    delete contentname;
    OstTraceFunctionExit0( CCONTENTINFODB_ADDL_EXIT );
    return err;
    }

// -----------------------------------------------------------------------------
// CContentInfoDb::UpdateBlacklistStatusL
// -----------------------------------------------------------------------------
//
TInt  CContentInfoDb::UpdateBlacklistStatusL( const TDesC& aContentName , TInt aBlacklistStatus )
    {
    OstTraceFunctionEntry0( CCONTENTINFODB_UPDATEBLACKLISTSTATUSL_ENTRY );
    //Update the item record to database    
    if ( !iOpened )
        return KErrNotReady;
    
    //Prepare the sql
    TBuf<KCISqlStringMaxLength> sql;
    sql.Format( KUpdateBSFormat , aBlacklistStatus, &aContentName );
    
    TInt err = iDatabase.Exec(sql) ;
    
    OstTraceFunctionExit0( CCONTENTINFODB_UPDATEBLACKLISTSTATUSL_EXIT );
    return err;
    }

// -----------------------------------------------------------------------------
// CContentInfoDb::UpdatePluginIndexStatusL
// -----------------------------------------------------------------------------
//
TInt  CContentInfoDb::UpdatePluginIndexStatusL( const TDesC& aContentName , TInt aIndexStatus )
    {
    OstTraceFunctionEntry0( CCONTENTINFODB_UPDATEPLUGININDEXSTATUSL_ENTRY );
    //Update the item record to database
    
    if ( !iOpened )
        return KErrNotReady;
    
    //Prepare the sql
    TBuf<KCISqlStringMaxLength> sql;
    sql.Format( KUpdateISFormat , aIndexStatus, &aContentName );
    
    TInt err = iDatabase.Exec(sql) ;
    
    OstTraceFunctionExit0( CCONTENTINFODB_UPDATEPLUGININDEXSTATUSL_EXIT );
    return err;
    }

// -----------------------------------------------------------------------------
// CContentInfoDb::RemoveL
// -----------------------------------------------------------------------------
//
void CContentInfoDb::RemoveL( const TDesC& aContentName )
    {
    OstTraceFunctionEntry0( CCONTENTINFODB_REMOVEL_ENTRY );
    //remove the item record to database
        
    if ( !iOpened )
        User::Leave( KErrNotReady );
    
    //Prepare the sql
    TBuf<KCISqlStringMaxLength> sql;
    sql.Format( KCISqlDelete , &aContentName );
    
    User::LeaveIfError( iDatabase.Exec(sql) );
    OstTraceFunctionExit0( CCONTENTINFODB_REMOVEL_EXIT );
    return ;
    }

// -----------------------------------------------------------------------------
// CContentInfoDb::ResetDatabaseL
// -----------------------------------------------------------------------------
//
void CContentInfoDb::ResetDatabaseL( )
    {
    OstTraceFunctionEntry0( CCONTENTINFODB_RESETDATABASEL_ENTRY );
    //remove all the item record to database
    
    if ( !iOpened )
        User::Leave( KErrNotReady );
    
    //Prepare the sql
    TBuf<KCISqlStringMaxLength> sql;
    sql.Copy( KRemoveAll );
    
    User::LeaveIfError( iDatabase.Exec(sql) );
    
    OstTraceFunctionExit0( CCONTENTINFODB_RESETDATABASEL_EXIT );
    return ;
    }

// -----------------------------------------------------------------------------
// CContentInfoDb::FindL
// -----------------------------------------------------------------------------
//
TBool CContentInfoDb::FindL(const TDesC& aContentName)
    {
    OstTraceFunctionEntry0( CCONTENTINFODB_FINDL_ENTRY );
    if ( !iOpened )
       User::Leave( KErrNotReady );
    
    //Prepare the sql
    TBuf<KCISqlStringMaxLength> sql;
    sql.Format( KCISqlFormatSeek, &aContentName );
    
    RSqlStatement stmt;
    TBool isfound = EFalse;
    //Error check necessary to avoid sqldb 2 panic, 
    //if sqlstatement preparation fails, call to Next() raises this panic
    TInt err = stmt.Prepare( iDatabase , sql );
    if ( err == KErrNone)
        isfound = ( KSqlAtRow == stmt.Next() )?ETrue:EFalse; 
    OstTraceFunctionExit0( CCONTENTINFODB_FINDL_EXIT );
    return isfound;
    }

// -----------------------------------------------------------------------------
// CContentInfoDb::GetContentCountL
// -----------------------------------------------------------------------------
//
TInt CContentInfoDb::GetContentCountL()
    {
    OstTraceFunctionEntry0( CCONTENTINFODB_GETCONTENTCOUNTL_ENTRY );
    if ( !iOpened )
       User::Leave( KErrNotReady );
    
    TInt count = 0;
    
    //Prepare the sql
    TBuf<KCISqlStringMaxLength> sql;
    sql.Copy( KSelectAllRowsFormat );
    
    RSqlStatement stmt;
    TInt err = stmt.Prepare( iDatabase , sql );
    //Error check necessary to avoid sqldb 2 panic, 
    //if sqlstatement preparation fails, call to Next() raises this panic     
    if( err == KErrNone)
        {
        while ( KSqlAtEnd != stmt.Next() )
            ++count;
        }
    OstTraceFunctionExit0( CCONTENTINFODB_GETCONTENTCOUNTL_EXIT );
    return count;
    }

// -----------------------------------------------------------------------------
// CContentInfoDb::CreateDBL
// -----------------------------------------------------------------------------
//
void CContentInfoDb::CreateDBL()
    {
    OstTraceFunctionEntry0( CCONTENTINFODB_CREATEDBL_ENTRY );
    
    iDatabase.Close();
    
    TFileName privatePath;
    TFileName datafile;
    RFs fssession;
    User::LeaveIfError( fssession.Connect() );
    fssession.CreatePrivatePath(EDriveC);
    fssession.PrivatePath(privatePath);//data caged path of loading process
    fssession.Close();
    datafile.Copy(KDriveC);
    datafile.Append(privatePath);    
    datafile.Append( KContentInfoFileName );
    
    //create the database
    User::LeaveIfError( iDatabase.Create( datafile ) );
    
    //Add table
    TBuf<KCISqlStringMaxLength> sql;
    sql.Copy( KCreateTableFormat );
    
    User::LeaveIfError( iDatabase.Exec( sql ) );
    
    OstTraceFunctionExit0( CCONTENTINFODB_CREATEDBL_EXIT );
    }
//EOF
