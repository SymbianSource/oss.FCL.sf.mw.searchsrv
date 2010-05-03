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


#include "CBlacklistDb.h"
#include <HarvesterServerLogger.h>
#include "OstTraceDefinitions.h"
#ifdef OST_TRACE_COMPILER_IN_USE
#include "cblacklistdbTraces.h"
#endif


_LIT(KBlacklistFileName,"Blacklist.dat");

//Table name in blacklist database
_LIT( KBlacklistTableName , "table" );
//column name for Uid's of plugins in blacklist database
_LIT( Kuid , "uid" );
//column name for version numbers of plugins in blacklist database
_LIT( Kversion , "version" );
// The max length for creating sql query for KBlistSqlFormatSeek format
const TInt KBlistSqlStringMaxLength(40);
//SQL query to fetch the records with given uid
_LIT(KBlistSqlFormatSeek , "SELECT * FROM table WHERE uid=%d");
//SQL query to delete the records with given uid
_LIT(KBlistSqlDelete, "DELETE FROM table WHERE uid=%d");
//SQL query to fetch all the records in database
_LIT(KBlistSqlFormatAll , "SELECT * FROM table");
_LIT(KDriveC, "c:");
// -----------------------------------------------------------------------------
// CBlacklistDb::NewL()
// -----------------------------------------------------------------------------
//
CBlacklistDb* CBlacklistDb::NewL()
    {
    OstTraceFunctionEntry0( CBLACKLISTDB_NEWL_ENTRY );
    CPIXLOGSTRING("CBlacklistDb::NewL(): Entered");
    CBlacklistDb* instance = CBlacklistDb::NewLC();
    CleanupStack::Pop( instance );
    CPIXLOGSTRING("CBlacklistDb::NewL(): Exit");
    OstTraceFunctionExit0( CBLACKLISTDB_NEWL_EXIT );
    return instance;
    }

// -----------------------------------------------------------------------------
// CBlacklistDb::NewLC()
// -----------------------------------------------------------------------------
//
CBlacklistDb* CBlacklistDb::NewLC()
    {
    CBlacklistDb* instance = new (ELeave) CBlacklistDb();
    CleanupStack::PushL( instance );
    instance->ConstructL();
    return instance;
    }

// -----------------------------------------------------------------------------
// CBlacklistDb::CBlacklistDb()
// -----------------------------------------------------------------------------
//
CBlacklistDb::CBlacklistDb()
    {
    //Do the necessary initialisation
    iOpened = EFalse;
    }

// -----------------------------------------------------------------------------
// CBlacklistDb::~CBlacklistDb
// -----------------------------------------------------------------------------
//
CBlacklistDb::~CBlacklistDb()
    {
    iDatabase.Close();
    iFs.Close();
    }

// -----------------------------------------------------------------------------
// CBlacklistDb::ConstructL()
// -----------------------------------------------------------------------------
//
void CBlacklistDb::ConstructL()
    {
    OstTraceFunctionEntry0( CBLACKLISTDB_CONSTRUCTL_ENTRY );
    CPIXLOGSTRING("CBlacklistDb::ConstructL(): Entered");
    //Open the sql database.If doesn't exist,create the sql database and open it     
    User::LeaveIfError( iFs.Connect() );
       
    TFileName privatePath;
    TFileName datafile;    
    iFs.CreatePrivatePath(EDriveC);
    iFs.PrivatePath(privatePath);//data caged path of loading process
    datafile.Copy(KDriveC);
    datafile.Append(privatePath);    
    datafile.Append( KBlacklistFileName );
    
    TInt err = iDatabase.Open( iFs , datafile );
    
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
                OstTrace0( TRACE_NORMAL, CBLACKLISTDB_CONSTRUCTL, "CBlacklistDb::ConstructL(): Database Opened" );
                CPIXLOGSTRING("CBlacklistDb::ConstructL(): Database Opened");
                }
            }
            break;
            
        default:
            break;
        }
    
    CPIXLOGSTRING("CBlacklistDb::ConstructL(): Exit");
    OstTraceFunctionExit0( CBLACKLISTDB_CONSTRUCTL_EXIT );
    return;
    }

// -----------------------------------------------------------------------------
// CBlacklistDb::AddL()
// -----------------------------------------------------------------------------
//
TInt CBlacklistDb::AddL( TInt32 aPluginUid , TInt aVersion )
    {
    OstTraceFunctionEntry0( CBLACKLISTDB_ADDL_ENTRY );
    //Add the item record to database
    OstTraceExt2( TRACE_NORMAL, CBLACKLISTDB_ADDL, "CBlacklistDb::AddL;Uid=%x;Version=%d", aPluginUid, aVersion );
    CPIXLOGSTRING3("CBlacklistDb::AddL(): Uid = %x and Version = %d" , aPluginUid , aVersion);
    
    if ( !iOpened )
        return KErrNotReady;
    
    TInt err; 
    //Prepare the view
    RDbView dbView;
    CleanupClosePushL( dbView );

    err = dbView.Prepare( iDatabase , TDbQuery( KBlistSqlFormatAll ) ) ;
        
    if ( err == KErrNone )
       {
        TRAP( err , dbView.InsertL() );
        CDbColSet* colSet = dbView.ColSetL();
        TDbColNo uidcolno = colSet->ColNo( Kuid );
        TDbColNo versioncolno = colSet->ColNo( Kversion );
        dbView.SetColL( uidcolno , aPluginUid );
        dbView.SetColL( versioncolno , aVersion );
        dbView.PutL();
        //If addition failed, rollback
        if(err != KErrNone)
            {
            iDatabase.Rollback();
            }            
       }
    CleanupStack::PopAndDestroy( &dbView ); // dbView/    
    User::LeaveIfError( iDatabase.Compact() );    
    CPIXLOGSTRING("CBlacklistDb::AddL(): Exit");
    OstTraceFunctionExit0( CBLACKLISTDB_ADDL_EXIT );
    return err;
    }

// -----------------------------------------------------------------------------
// CBlacklistDb::RemoveL()
// -----------------------------------------------------------------------------
//
void CBlacklistDb::Remove( TInt32 aPluginUid )
    {
    OstTraceFunctionEntry0( CBLACKLISTDB_REMOVE_ENTRY );
    OstTrace1( TRACE_NORMAL, CBLACKLISTDB_REMOVE, "CBlacklistDb::Remove;Uid=%x", aPluginUid );
    CPIXLOGSTRING2("CBlacklistDb::RemoveL(): Uid = %x " , aPluginUid);
    
    if ( !iOpened )
       return;
    
    //Remove the item record to database
    // Create the sql statement.  KBlistSqlDelete
    TBuf<KBlistSqlStringMaxLength> sql;
    sql.Format( KBlistSqlDelete , aPluginUid );
    
    //delete the row.
    TInt rowCount( iDatabase.Execute(sql) );
    if(rowCount > 0)
        {        
        OstTrace0( TRACE_NORMAL, DUP1_CBLACKLISTDB_REMOVE, "CBlacklistDb::RemoveL(): Removed UID succesfully" );
        CPIXLOGSTRING("CBlacklistDb::RemoveL(): Removed UID succesfully");
        }
    else
        {
        OstTrace0( TRACE_NORMAL, DUP2_CBLACKLISTDB_REMOVE, "CBlacklistDb::RemoveL(): UID not found" );
        CPIXLOGSTRING("CBlacklistDb::RemoveL(): UID not found");
        }
    CPIXLOGSTRING("CBlacklistDb::RemoveL(): Exit");
    OstTraceFunctionExit0( CBLACKLISTDB_REMOVE_EXIT );
    return ;
    }

// -----------------------------------------------------------------------------
// CBlacklistDb::iSAvailableL()
// -----------------------------------------------------------------------------
//
TBool CBlacklistDb::FindWithVersionL( TInt32 aPluginUid , TInt aVersion )
    {
    OstTraceExt2( TRACE_NORMAL, CBLACKLISTDB_FINDWITHVERSIONL, "CBlacklistDb::FindWithVersionL;Uid=%x;Version=%d", aPluginUid, aVersion );
    CPIXLOGSTRING3("CBlacklistDb::FindWithVersionL(): Uid = %x and Version = %d" , aPluginUid , aVersion);
    
    if ( !iOpened )
            return EFalse;
    
    //Check if the item is available in database
    //Prepare the sql
    TBuf<KBlistSqlStringMaxLength> sql;
    sql.Format( KBlistSqlFormatSeek , aPluginUid );
    TBool found = EFalse;
    //Prepare the view
    RDbView dbView;
    CleanupClosePushL( dbView );

    User::LeaveIfError( dbView.Prepare( iDatabase , TDbQuery(sql) , RDbView::EReadOnly ) );
    User::LeaveIfError( dbView.EvaluateAll() );

    TInt isAtRow( dbView.FirstL() );
    
    if ( isAtRow )
       {
        //Get the current row data
        dbView.GetL();
        CDbColSet* colSet = dbView.ColSetL(); 
        TDbColNo versioncolno = colSet->ColNo(Kversion);        
        TInt version = dbView.ColInt32( versioncolno );
        if ( version == aVersion)
            {
            OstTrace0( TRACE_NORMAL, DUP1_CBLACKLISTDB_FINDWITHVERSIONL, "CBlacklistDb::FindWithVersionL(): UID found" );
            CPIXLOGSTRING("CBlacklistDb::FindWithVersionL(): UID found");
            found = ETrue;
            }        
       }    
    CleanupStack::PopAndDestroy( &dbView ); // dbView/
    return found;
    }

// -----------------------------------------------------------------------------
// CBlacklistDb::UpdateL
// -----------------------------------------------------------------------------
//
TInt CBlacklistDb::UpdateL( TInt32 aPluginUid , TInt aVersion )
	{
	OstTraceFunctionEntry0( CBLACKLISTDB_UPDATEL_ENTRY );
	//Add the item record to database
    OstTraceExt2( TRACE_NORMAL, CBLACKLISTDB_UPDATEL, "CBlacklistDb::UpdateL;Uid=%x;Version=%d", aPluginUid, aVersion );
    CPIXLOGSTRING3("CBlacklistDb::UpdateL(): Uid = %x and Version = %d" , aPluginUid , aVersion);
    
    if ( !iOpened )
       return KErrNotReady;
    
    //Check if the record with given plugin uid is already available in database or not
    //If available just update version number in the same record
    //If there is no record found in database with given uid, add new record with given details
    TInt err;
    TBuf<KBlistSqlStringMaxLength> sql;
    sql.Format( KBlistSqlFormatSeek , aPluginUid );    
    //Prepare the view
    RDbView dbView;
    CleanupClosePushL( dbView );

    User::LeaveIfError( dbView.Prepare( iDatabase , TDbQuery(sql) , RDbView::EUpdatable ) );
    User::LeaveIfError( dbView.EvaluateAll() );

    TInt isAtRow( dbView.FirstL() );
    
    if ( isAtRow )
       {
        //Found a record with passed uid
        //Get the current row data        
        CDbColSet* colSet = dbView.ColSetL(); 
        TDbColNo versioncolno = colSet->ColNo( Kversion );        
        //Update the same record with new version number
        TRAP( err , dbView.UpdateL());
        dbView.SetColL( versioncolno , aVersion );
        dbView.PutL();
        //If updation failed , Rollback
        if(err != KErrNone)
            {
            iDatabase.Rollback();
            }
        }        
       
    CleanupStack::PopAndDestroy( &dbView ); // dbView/    
    User::LeaveIfError( iDatabase.Compact() );    
    CPIXLOGSTRING("CBlacklistDb::UpdateL(): Exit");
    OstTraceFunctionExit0( CBLACKLISTDB_UPDATEL_EXIT );
    return KErrNone;
	}

// -----------------------------------------------------------------------------
// CBlacklistDb::FindL
// -----------------------------------------------------------------------------
//
TBool CBlacklistDb::FindL(TInt32 aPluginUid)
	{
	OstTrace1( TRACE_NORMAL, CBLACKLISTDB_FINDL, "CBlacklistDb::FindL;Uid=%x", aPluginUid );
	CPIXLOGSTRING2("CBlacklistDb::FindL(): Uid = %x " , aPluginUid );
	
	if ( !iOpened )
	        return EFalse;
	
    //Check if the item is available in database
    //Prepare the sql
    TBuf<KBlistSqlStringMaxLength> sql;
    sql.Format( KBlistSqlFormatSeek , aPluginUid );
    TBool found = EFalse;
    //Prepare the view
    RDbView dbView;
    CleanupClosePushL( dbView );

    User::LeaveIfError( dbView.Prepare( iDatabase , TDbQuery(sql) , RDbView::EReadOnly ) );
    User::LeaveIfError( dbView.EvaluateAll() );

    TInt isAtRow( dbView.FirstL() );
    
    if ( isAtRow )
       {        
		OstTrace0( TRACE_NORMAL, DUP1_CBLACKLISTDB_FINDL, "CBlacklistDb::FindL(): UID found" );
		CPIXLOGSTRING("CBlacklistDb::FindL(): UID found");
		found = ETrue;                 
       }    
    CleanupStack::PopAndDestroy( &dbView ); // dbView/
    return found;
	}
// -----------------------------------------------------------------------------
// CBlacklistDb::CreateDBL
// -----------------------------------------------------------------------------
//
void CBlacklistDb::CreateDBL()
    {
    OstTraceFunctionEntry0( CBLACKLISTDB_CREATEDBL_ENTRY );
    CPIXLOGSTRING("CBlacklistDb::CreateDBL(): Enter");
    
    iDatabase.Close();
    
    TFileName privatePath;
    TFileName datafile;    
    iFs.CreatePrivatePath(EDriveC);
    iFs.PrivatePath(privatePath);//data caged path of loading process
    datafile.Copy(KDriveC);
    datafile.Append(privatePath);    
    datafile.Append( KBlacklistFileName );
    
    //create the database
    User::LeaveIfError( iDatabase.Create( iFs , datafile ) );
    CDbColSet* columns = CreateColumnSetLC();//creates the columns and push to cleanupstack
    User::LeaveIfError( iDatabase.CreateTable( KBlacklistTableName , *columns ) );
    //clean up of variables (columns and dataFile)
    CleanupStack::PopAndDestroy( columns );
    
    CPIXLOGSTRING("CBlacklistDb::CreateDBL(): Exit");
    OstTraceFunctionExit0( CBLACKLISTDB_CREATEDBL_EXIT );
    }

// -----------------------------------------------------------------------------
// CBlacklistDb::CreateColumnSetLC
// -----------------------------------------------------------------------------
//
CDbColSet* CBlacklistDb::CreateColumnSetLC()
    {
    OstTraceFunctionEntry0( CBLACKLISTDB_CREATECOLUMNSETLC_ENTRY );
    CPIXLOGSTRING("CBlacklistDb::CreateColumnSetLC(): Enter");
    
    CDbColSet* columns = CDbColSet::NewLC();
    
    //Add uid column
    TDbCol col( Kuid , EDbColInt32 );
    col.iAttributes = TDbCol::ENotNull ;
    columns->AddL( col );
    
    //Add version column
    col.iName = Kversion;
    col.iType = EDbColInt32;
    col.iAttributes = TDbCol::ENotNull;
    columns->AddL( col );
    
    CPIXLOGSTRING("CBlacklistDb::CreateColumnSetLC(): Exit");
    
    OstTraceFunctionExit0( CBLACKLISTDB_CREATECOLUMNSETLC_EXIT );
    return columns; // columns stays on CleanupStack
    }
