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

#include <f32file.h>
#include <s32file.h>
#include <centralrepository.h>
#include "CIndexingManager.h"
#include "HarvesterServerLogger.h"
#include "CBlacklistMgr.h"
#include "contentinfodbupdate.h"
#include "OstTraceDefinitions.h"
#ifdef OST_TRACE_COMPILER_IN_USE
#include "cindexingmanagerTraces.h"
#endif


_LIT(KManagerFileName, "CPixConfig.bin");
const TInt KManagerFileVersion = 1;

// How often harvester states are checked (in minutes)
const TUint KDefaultWaitTimeInMinutes = 1;

// How often harvester states are checked (in microseconds)
//const TUint KDefaultWaitTime = KDefaultWaitTimeInMinutes*60*1000000; // 1 minute

// If time difference between RunL calls is less than this value (i.e system time
// changed to past) update harvesters start and complete times.
const TInt KMinTimeDifference = 0;

// If time difference between RunL calls is more than this value (i.e system time
// changed to future) update harvesters start and complete times. This value is
// default difference in RunL calls with additional buffer.
const TInt KMaxTimeDifference = 1 + KDefaultWaitTimeInMinutes;
//constants for enable and disable status
const TInt KEnable = 1;
const TInt KDisable = 0;
//Uid of Harvester server cetral repository database
const TUid KCPIXHSrepoUidMenu = {0x2001f6fb};

//Length of uid string in cenrep
const TInt KCenrepUidLength = 8;
// -----------------------------------------------------------------------------
// CHarvesterServer::NewL()
// -----------------------------------------------------------------------------
//
CIndexingManager* CIndexingManager::NewL()
	{
	CIndexingManager* instance = new (ELeave)CIndexingManager();
	CleanupStack::PushL(instance);
	instance->ConstructL();
	CleanupStack::Pop(instance);
	return instance;
	}

// -----------------------------------------------------------------------------
// CIndexingManager::CIndexingManager()
// -----------------------------------------------------------------------------
//
CIndexingManager::CIndexingManager()
	: CActive(CActive::EPriorityStandard),
	iState(EStateNone), iPreviousRun(0),
	iHarvestState(EFalse)
	{
	CActiveScheduler::Add(this);
	}

// -----------------------------------------------------------------------------
// CIndexingManager::~CIndexingManager()
// -----------------------------------------------------------------------------
//
CIndexingManager::~CIndexingManager()
	{
	// Active Object cancel
	Cancel();

	iHarvesterArray.Close();

	// Delete all plugins
	iPluginArray.ResetAndDestroy();

	// Close the array handle
	iPluginArray.Close();
	
	// Close the timer
	iTimer.Close();
	
	// Close search server connection
	iSearchSession.Close();
	
	// Close file system connection
	iFs.Close();
	if(iBlacklistMgr)
	    {
	    delete iBlacklistMgr;
	    iBlacklistMgr = NULL;
	    }
	if(iContentInfodb)
	    {
	    delete iContentInfodb;
	    iContentInfodb = NULL;
	    }	
	delete iActivityManager;
	delete iStateObserver;	
	delete iGaurdTimer;
	}

// -----------------------------------------------------------------------------
// CIndexingManager::ConstructL()
// -----------------------------------------------------------------------------
//
void CIndexingManager::ConstructL()
	{
	// connect to file system
	User::LeaveIfError(iFs.Connect());
	
	// Load the configuration
	TFileName pathWithoutDrive;
	iFs.CreatePrivatePath(EDriveC);
	iManagerFilePath = _L("C:");
		
	iFs.PrivatePath( pathWithoutDrive );
	iManagerFilePath.Append(pathWithoutDrive);
	iManagerFilePath.Append(KManagerFileName);
	
	// Loads the saved list of file transfers
	Internalize();
	
	// Create the timer
	User::LeaveIfError(iTimer.CreateLocal());

	// This server must be connected before passing to harvester plugins
	User::LeaveIfError(iSearchSession.Connect());
	
	//instantiate blacklist database
	iBlacklistMgr = CBlacklistMgr::NewL();
	
	//Instantiate ContentInfoDbUpdate
	iContentInfodb = new ContentInfoDbUpdate();
	
	UpdateDontloadListL();

	// Load plugins
	LoadPluginsL();
	
	//release the Blacklist manager as the blacklist functionality is done
	delete iBlacklistMgr;
	iBlacklistMgr = NULL;
	//release the content info manager as all the content details of all plugins are stored
	delete iContentInfodb;
	iContentInfodb = NULL;
	
	//Start the activity manager to monitor the user activity status
	iActivityManager = CActivityManager::NewL( this );
	if(iActivityManager)
        {
        iActivityManager->Start();
        }
	iGaurdTimer = CGaurdTimer::NewL( this );
	iStateObserver = CPiXStateObserver::NewL(this, &iFs);
	if(iStateObserver)
	    {
	    //iStateObserver->StartMonitoringL();
	    }
	StartPlugins();
	
	// Wait before running RunL
	iState = EStateRunning;
	iTimer.After(iStatus, 10000000); // 10s
	SetActive();
	}

// -----------------------------------------------------------------------------
// CIndexingManager::DoCancel()
// -----------------------------------------------------------------------------
//
void CIndexingManager::DoCancel()
	{
	// Cancel the timer
	if (iState == EStateRunning)
		{
		iTimer.Cancel();
		iState = EStateCancelling;
		}
	}

// -----------------------------------------------------------------------------
// CIndexingManager::RunL()
// -----------------------------------------------------------------------------
//
void CIndexingManager::RunL()
	{

	// Take next harvester from the list to be run
    OstTrace0( TRACE_NORMAL, CINDEXINGMANAGER_RUNL, "CIndexingManager::RunL" );
    CPIXLOGSTRING("CIndexingManager::RunL() ");

    // If system time is changed update harvester last complete and start time accordingly.
    // This ensures that no extra reharvesting is done if system time is changed.
    TTime timenow;
    timenow.UniversalTime();
    
    TTimeIntervalMinutes timeDifference(0);
    TInt err = timenow.MinutesFrom(iPreviousRun, timeDifference);

    if ( err == KErrNone && iPreviousRun.Int64() != 0 &&
         ( timeDifference.Int() < KMinTimeDifference || timeDifference.Int() > KMaxTimeDifference ) )
        {
        OstTrace0( TRACE_NORMAL, DUP1_CINDEXINGMANAGER_RUNL, "CIndexingManager::RunL() time changed. Update harvester completion and start times" );
        CPIXLOGSTRING("CIndexingManager::RunL() time changed. Update harvester completion and start times");
        for (TInt i=0; i<iHarvesterArray.Count(); i++)
            {
            iHarvesterArray[i].iLastComplete += timeDifference;
            iHarvesterArray[i].iLastStart += timeDifference;
            }
        }

    iPreviousRun = timenow;

    
    // Do nothing if already harvesting
    if (iState != EStateRunning)
    	{
        OstTrace0( TRACE_NORMAL, DUP2_CINDEXINGMANAGER_RUNL, "CIndexingManager::RunL() the indexing manager is not running anymore" );
        CPIXLOGSTRING("CIndexingManager::RunL() the indexing manager is not running anymore ");
    	return;
    	}

	// Check if harvesters running
	TBool harvesterRunning = EFalse;
	for (TInt i=0; i<iHarvesterArray.Count(); i++)
		{
		switch (iHarvesterArray[i].iStatus)
			{
			case EHarvesterStatusWaiting:
		        OstTraceExt1( TRACE_NORMAL, DUP3_CINDEXINGMANAGER_RUNL, "CIndexingManager::RunL;<%S>, waiting for scheduling..", iHarvesterArray[i].iQualifiedBaseAppClass );
		        CPIXLOGSTRING2("CIndexingManager::RunL(): <%S>, waiting for scheduling...", &iHarvesterArray[i].iQualifiedBaseAppClass);
				break;
			case EHarvesterStatusHibernate:
			    OstTraceExt1( TRACE_NORMAL, DUP4_CINDEXINGMANAGER_RUNL, "CIndexingManager::RunL;<%S>, hibernating..", iHarvesterArray[i].iQualifiedBaseAppClass );
		        CPIXLOGSTRING2("CIndexingManager::RunL(): <%S>, hibernating.", &iHarvesterArray[i].iQualifiedBaseAppClass);
				break;
			case EHarvesterStatusRunning:
			    OstTraceExt1( TRACE_NORMAL, DUP5_CINDEXINGMANAGER_RUNL, "CIndexingManager::RunL;<%S>, running...", iHarvesterArray[i].iQualifiedBaseAppClass );
		        CPIXLOGSTRING2("CIndexingManager::RunL(): <%S>, running...", &iHarvesterArray[i].iQualifiedBaseAppClass);
		        break;
			}
		TTimeIntervalMinutes mins_last_complete;
		TTimeIntervalMinutes mins_last_start;
		timenow.MinutesFrom(iHarvesterArray[i].iLastComplete, mins_last_complete);
		timenow.MinutesFrom(iHarvesterArray[i].iLastStart, mins_last_start);
        OstTraceExt2( TRACE_NORMAL, DUP6_CINDEXINGMANAGER_RUNL, "CIndexingManager::RunL;^^^Previous start %d min ago;Previous complete %d min ago", mins_last_start.Int(), mins_last_complete.Int() );
        CPIXLOGSTRING3("CIndexingManager::RunL():  ^^^ previous start %d mins ago, previous complete %d mins ago.", mins_last_start.Int(), mins_last_complete.Int());

        if (iHarvesterArray[i].iStatus == EHarvesterStatusRunning)
			harvesterRunning = ETrue;
		}

    
    // If not already running, just pick the next harvester and run it
	if (!harvesterRunning)
		{
		for (TInt i=0; i<iHarvesterArray.Count(); i++)
			{
			if ((iHarvesterArray[i].iPlugin != NULL) && // Must be valid plugin
				(iHarvesterArray[i].iStatus == EHarvesterStatusWaiting) && // Must be waiting
				((iHarvesterArray[i].iLastComplete > timenow) || // Last completion after timenow (invalid time!) 
				 (iHarvesterArray[i].iLastComplete < timenow - TTimeIntervalDays(7)))) 
				// Last completion over 1 week ago (change to TTimeIntervalMinutes for debugging) 
				{
				// Set new status
				iHarvesterArrayChanged = ETrue;
				iHarvesterArray[i].iStatus = EHarvesterStatusRunning;
				iHarvesterArray[i].iLastStart.UniversalTime();

				// TODO Why are we trapping StartHarvestingL()? 
				// Consider handling the leave in ::RunError instead. 

				// Run the harvester
			    OstTraceExt1( TRACE_NORMAL, DUP7_CINDEXINGMANAGER_RUNL, "CIndexingManager::RunL;Starting harvesting <%S>.", iHarvesterArray[i].iQualifiedBaseAppClass );
			    CPIXLOGSTRING2("CIndexingManager::RunL(): Starting harvesting <%S>.", &iHarvesterArray[i].iQualifiedBaseAppClass);
				TRAPD(err, iHarvesterArray[i].iPlugin->StartHarvestingL(iHarvesterArray[i].iQualifiedBaseAppClass));

				// No need to continue, if something was already started 
				if (err == KErrNone)
					{
				    OstTrace0( TRACE_NORMAL, DUP8_CINDEXINGMANAGER_RUNL, "CIndexingManager::RunL(): Successfully started harvesting." );
				    CPIXLOGSTRING("CIndexingManager::RunL(): Successfully started harvesting. ");
					break;
					}
				
				// Set the error status
				iHarvesterArray[i].iLastComplete.UniversalTime();
				iHarvesterArray[i].iError = err;
				// Do not run it again, unless CIndexingMgr informed to do so
				iHarvesterArray[i].iStatus = EHarvesterStatusHibernate;
			    OstTrace1( TRACE_NORMAL, DUP9_CINDEXINGMANAGER_RUNL, "CIndexingManager::RunL;Error %d in starting harvesting", err );
			    CPIXLOGSTRING2("CIndexingManager::RunL(): Error %d in starting harvesting. ", err);
				}
			}
		}
	
	// Save status here if changed.
	if (iHarvesterArrayChanged)
		{
		iHarvesterArrayChanged = EFalse;
		Externalize();
		}

	// Always issue new wait
	timenow += TTimeIntervalDays(1);
	//iTimer.After(iStatus, KDefaultWaitTime);
	iTimer.At(iStatus, timenow);
	SetActive();
	
	}

// -----------------------------------------------------------------------------
// CIndexingManager::LoadPluginsL()
// -----------------------------------------------------------------------------
//
void CIndexingManager::LoadPluginsL()
	{
    OstTraceFunctionEntry0( CINDEXINGMANAGER_LOADPLUGINSL_ENTRY );
    RImplInfoPtrArray infoArray;    
    TCleanupItem cleanupItem( CPixSearchECom::CleanupEComArray, &infoArray );
    CleanupStack::PushL( cleanupItem );
    
    CIndexingPlugin::ListImplementationsL( infoArray );
    TInt count( 0 );
    count = infoArray.Count();
    
    CImplementationInformation *info = NULL;
    for(TInt i = 0; i < count; i++ )
        {
        TUid uid = infoArray[i]->ImplementationUid();
        if( uid.iUid == 0x2001F703) // files
            {
            info = infoArray[i];
            infoArray.Remove(i);
            break;
            }            
        }    
    // Now add file plugin at the end of the list
    infoArray.AppendL(info);       
    for ( TInt i = 0; i < count; i++ )
        {
        TUid uid = infoArray[i]->ImplementationUid();    // Create the plug-ins
        TInt version = infoArray[i]->Version();
        
        //Update the details of the plugin in Contentinfo DB
        TBuf<255> filepath;
        filepath.Copy(infoArray[i]->OpaqueData());
        UpdateContentInfoDbL(filepath);
        
        //Get the load status of the plugin.
        TBool pluginloadstatus = GetPluginLoadStatusL ( uid, version, infoArray[i]->DisplayName() );        
        
        if ( pluginloadstatus )
            {
            LoadHarvesterpluginL (uid, version, infoArray[i]->DisplayName() );//Load the harvester plugin
            }
        }
    CleanupStack::PopAndDestroy( &infoArray ); // infoArray, results in a call to CleanupEComArray    
	OstTraceFunctionExit0( CINDEXINGMANAGER_LOADPLUGINSL_EXIT );
	}

// -----------------------------------------------------------------------------
// CIndexingManager::StartPlugins()
// -----------------------------------------------------------------------------
//
void CIndexingManager::StartPlugins()
	{
	for (TInt i = 0; i < iPluginArray.Count(); i++)
		{
		TRAPD(err, iPluginArray[i]->StartPluginL());
		if (err != KErrNone)
			{
			// Failed to start the plugin
			}
		}
	}

// -----------------------------------------------------------------------------
// CIndexingManager::AddHarvestingQueue()
// -----------------------------------------------------------------------------
//
void CIndexingManager::AddHarvestingQueue(CIndexingPlugin* aPlugin,
										  const TDesC& aQualifiedBaseAppClass,
										  TBool aForceReharvest)
	{
    OstTraceExt1( TRACE_NORMAL, CINDEXINGMANAGER_ADDHARVESTINGQUEUE, "CIndexingManager::AddHarvestingQueue;Queuing requested for <%S>", aQualifiedBaseAppClass );
    CPIXLOGSTRING2("CIndexingManager::AddHarvestingQueue(): Queuing requested for <%S>.", &aQualifiedBaseAppClass);	
	
	// Find if this harvesting task already exists
	for (TInt i=0; i<iHarvesterArray.Count(); i++)
		{
		// Not check the iPlugin here, only qualified, as iPlugin is inserted later
		if (iHarvesterArray[i].iQualifiedBaseAppClass.Compare(aQualifiedBaseAppClass)==0)
			{
			// Already exists, change the status if hibernating 
			if (iHarvesterArray[i].iStatus == EHarvesterStatusHibernate)
				{
				// No need to set iHarvesterArrayChanged when changing the status only (which is not saved)
				iHarvesterArray[i].iPlugin = aPlugin;
				iHarvesterArray[i].iStatus = EHarvesterStatusWaiting;
				if (aForceReharvest)
					{
					OstTrace0( TRACE_NORMAL, DUP1_CINDEXINGMANAGER_ADDHARVESTINGQUEUE, "CIndexingManager::AddHarvestingQueue(): Needs Reharvesting." );
					CPIXLOGSTRING("CIndexingManager::AddHarvestingQueue(): Needs Reharvesting.");
					// iLastCompete Time is reset so that will be reharvested as soon
					// as possible
					iHarvesterArray[i].iLastComplete = TTime(0);
					// No need to reset iLastStart or iError
					iHarvesterArrayChanged = ETrue;
					}
				}
		    OstTrace0( TRACE_NORMAL, DUP2_CINDEXINGMANAGER_ADDHARVESTINGQUEUE, "CIndexingManager::AddHarvestingQueue(): Harvester already in the queue." );
		    CPIXLOGSTRING("CIndexingManager::AddHarvestingQueue(): Harvester already in the queue.");	
			return;
			}
		}	
	
	// Create new harvesting task
	iHarvesterArray.Append(THarvesterRecord(aPlugin, aQualifiedBaseAppClass));
	iHarvesterArrayChanged = ETrue;

	// Cancel current wait to launch RunL immediately
	if (iState == EStateRunning)
		iTimer.Cancel();

    OstTrace0( TRACE_NORMAL, DUP3_CINDEXINGMANAGER_ADDHARVESTINGQUEUE, "CIndexingManager::AddHarvestingQueue(): Harvester added successfully to the queue." );
    CPIXLOGSTRING("CIndexingManager::AddHarvestingQueue(): Harvester added successfully to the queue.");	
	}

// -----------------------------------------------------------------------------
// CIndexingManager::RemoveHarvestingQueue()
// -----------------------------------------------------------------------------
//
void CIndexingManager::RemoveHarvestingQueue(CIndexingPlugin* aPlugin, const TDesC& aQualifiedBaseAppClass,TBool aRemovePersist)
	{
    OstTraceExt1( TRACE_NORMAL, CINDEXINGMANAGER_REMOVEHARVESTINGQUEUE, "CIndexingManager::RemoveHarvestingQueue;De-Queuing requested for <%S>", aQualifiedBaseAppClass );
    CPIXLOGSTRING2("CIndexingManager::RemoveHarvestingQueue(): De-queuing requested for <%S>.", &aQualifiedBaseAppClass);	

    // Find if this harvesting task exists
	for (TInt i=0; i<iHarvesterArray.Count(); i++)
		{
		if (iHarvesterArray[i].iPlugin == aPlugin && 
			iHarvesterArray[i].iQualifiedBaseAppClass.Compare(aQualifiedBaseAppClass)==0)
			{
			// Found, hibernate it
			if (iHarvesterArray[i].iStatus == EHarvesterStatusWaiting)
				{
				// No need to set iHarvesterArrayChanged when changing the status only (which is not saved)
				iHarvesterArray[i].iStatus = EHarvesterStatusHibernate;
				}
			if(aRemovePersist) //Plugin want this to be removed from queue and saved to disk as well
				{
				CPIXLOGSTRING("CIndexingManager::RemoveHarvestingQueue(): aRemovePersist is true so removing");		
				iHarvesterArray.Remove(i);
				Externalize(); //Save to disk	
				CPIXLOGSTRING("CIndexingManager::RemoveHarvestingQueue(): aRemovePersist Externalize successful");		
				}
			OstTrace0( TRACE_NORMAL, DUP1_CINDEXINGMANAGER_REMOVEHARVESTINGQUEUE, "CIndexingManager::RemoveHarvestingQueue(): Harvester de-queued successfully." );
			CPIXLOGSTRING("CIndexingManager::RemoveHarvestingQueue(): Harvester de-queued successfully.");	
			return;
			}
		}

	OstTrace0( TRACE_NORMAL, DUP2_CINDEXINGMANAGER_REMOVEHARVESTINGQUEUE, "CIndexingManager::RemoveHarvestingQueue(): Harvester in the queue." );
	CPIXLOGSTRING("CIndexingManager::RemoveHarvestingQueue(): Harvester in the queue.");	
	}

// -----------------------------------------------------------------------------
// CIndexingManager::HarvestingCompleted()
// -----------------------------------------------------------------------------
//
void CIndexingManager::HarvestingCompleted(CIndexingPlugin* aPlugin, const TDesC& aQualifiedBaseAppClass, TInt aError)
	{
    OstTraceExt1( TRACE_NORMAL, CINDEXINGMANAGER_HARVESTINGCOMPLETED, "CIndexingManager::HarvestingCompleted;Harvesting completed for <%S>", aQualifiedBaseAppClass  );
    CPIXLOGSTRING2("CIndexingManager::HarvestingCompleted(): Harvesting completed reported for <%S>.", &aQualifiedBaseAppClass);	

    // Find this task 
	for (TInt i=0; i<iHarvesterArray.Count(); i++)
		{
		if (iHarvesterArray[i].iPlugin == aPlugin && 
			iHarvesterArray[i].iQualifiedBaseAppClass.Compare(aQualifiedBaseAppClass)==0)
			{
			// Found the task, now its waiting
			iHarvesterArray[i].iError = aError;
			iHarvesterArray[i].iStatus = EHarvesterStatusWaiting;
			iHarvesterArray[i].iLastComplete.UniversalTime();
			iHarvesterArrayChanged = ETrue;
			
			// Cancel current wait to launch RunL immediately
			if (iState == EStateRunning)
				iTimer.Cancel();

			OstTrace0( TRACE_NORMAL, DUP1_CINDEXINGMANAGER_HARVESTINGCOMPLETED, "CIndexingManager::HarvestingCompleted(): Harvesting status changed successfully." );
			CPIXLOGSTRING("CIndexingManager::HarvestingCompleted(): Harvesting status changed successfully.");
			return;			
			}
		}
	OstTrace0( TRACE_NORMAL, DUP2_CINDEXINGMANAGER_HARVESTINGCOMPLETED, "CIndexingManager::HarvestingCompleted(): Harvester not in the queue." );
	CPIXLOGSTRING("CIndexingManager::HarvestingCompleted(): Harvester not in the queue.");
	}

// -----------------------------------------------------------------------------
// CIndexingManager::Internalize()
// -----------------------------------------------------------------------------
//
TInt CIndexingManager::Internalize()
{
	TRAPD(err, LoadL());
	return err;
}

// -----------------------------------------------------------------------------
// CIndexingManager::Externalize()
// -----------------------------------------------------------------------------
//
TInt CIndexingManager::Externalize()
{
	TRAPD(err, SaveL());
	return err;
}

// -----------------------------------------------------------------------------
// CIndexingManager::LoadL()
// -----------------------------------------------------------------------------
//
void CIndexingManager::LoadL()
{
	// Open the stream
	RFile file;
	User::LeaveIfError(file.Open(iFs, iManagerFilePath, EFileRead));
	CleanupClosePushL(file);
	RFileReadStream rd(file);
	rd.PushL();
	
	// Read the version
	TInt version = rd.ReadInt32L();
	if (version != KManagerFileVersion)
		{
		User::Leave(KErrGeneral);
		}

	// Read harvester count
	TInt count = rd.ReadInt32L();

	// Read the harvesters
	for (TInt i=0; i<count; i++)
		{
		THarvesterRecord record;
		TInt length = rd.ReadInt32L();
		rd.ReadL(record.iQualifiedBaseAppClass, length);
		TInt64 laststart;
		TInt64 lastcomplete;		
		rd.ReadL((TUint8*)&laststart, sizeof(TInt64));
		rd.ReadL((TUint8*)&lastcomplete, sizeof(TInt64));
		record.iLastStart = laststart;
		record.iLastComplete = lastcomplete;
		record.iError = rd.ReadInt32L();
		record.iStatus = EHarvesterStatusHibernate;
		iHarvesterArray.AppendL(record);
		}
	
	// Cleanup
	CleanupStack::PopAndDestroy(2, &file);
}

// -----------------------------------------------------------------------------
// CIndexingManager::SaveL()
// -----------------------------------------------------------------------------
//
void CIndexingManager::SaveL()
{
	// Open the stream
	RFile file;
	User::LeaveIfError(file.Replace(iFs, iManagerFilePath, EFileWrite));
	CleanupClosePushL(file);
	
	RFileWriteStream wr(file);
	wr.PushL();
		
	// Write the version
	wr.WriteInt32L(KManagerFileVersion);
	
	// Write harvester count
	wr.WriteInt32L(iHarvesterArray.Count());
	
	// Write harvesters
	for (TInt i=0; i<iHarvesterArray.Count(); i++)
		{
		wr.WriteInt32L(iHarvesterArray[i].iQualifiedBaseAppClass.Length());
		wr.WriteL(iHarvesterArray[i].iQualifiedBaseAppClass);
		TInt64 laststart = iHarvesterArray[i].iLastStart.Int64();
		TInt64 lastcomplete = iHarvesterArray[i].iLastComplete.Int64();
		wr.WriteL((TUint8*)&laststart, sizeof(TInt64));
		wr.WriteL((TUint8*)&lastcomplete, sizeof(TInt64));
		wr.WriteInt32L(iHarvesterArray[i].iError);
		}
	
	// Commit and cleanup
	wr.CommitL();
	CleanupStack::PopAndDestroy(2, &file);
}

// -----------------------------------------------------------------------------
// CIndexingManager::UpdateContentInfoDbL()
// -----------------------------------------------------------------------------
//
void CIndexingManager::UpdateContentInfoDbL( const TDesC& aXmlPath)
{
    OstTraceFunctionEntry0( CINDEXINGMANAGER_UPDATECONTENTINFODBL_ENTRY );
    
    if(iContentInfodb)
        iContentInfodb->UpdateDb(QString::fromUtf16(aXmlPath.Ptr(),aXmlPath.Length()));
    
    OstTraceFunctionExit0( CINDEXINGMANAGER_UPDATECONTENTINFODBL_EXIT );
}

// -----------------------------------------------------------------------------
// CIndexingManager::UpdateDontloadListL()
// -----------------------------------------------------------------------------
//
void CIndexingManager::UpdateDontloadListL()
    {
    OstTraceFunctionEntry0( CINDEXINGMANAGER_UPDATEDONTLOADLISTL_ENTRY );
    CPIXLOGSTRING("CIndexingManager::UpdateDontloadList : Start");
    //Read the list of Uid's from the cenrep and update blacklist database
    //Open the unload list common repository
    CRepository* unloadrepo = NULL;
    TRAPD( cerror , unloadrepo = CRepository::NewL( KCPIXHSrepoUidMenu ));
    if ( cerror != KErrNone)
        return;
    RArray<TUint32> uidlist;    
    //Read all the key list
    //Matches occur whenever (key & mask) == (partialKey & mask). 
    //The partial key is guaranteed to be masked before use
    // To fetch all the keys we have done masking with '0'
    TInt error = unloadrepo->FindL( 0, 0, uidlist);
    if ( error == KErrNone )
        {
        TBuf<KCenrepUidLength> temp;
        //get the Uid of each and every plugin and add it to blacklist database
        TInt count = uidlist.Count();
        for (int i = 0; i < count; i++ )
            {
            TUid uid;
            TInt64 value;
            unloadrepo->Get( uidlist[i], temp );
            TLex uidvalue(temp);
            TInt xerr = uidvalue.Val( value,EHex );
            uid.iUid = value;
            (void)iBlacklistMgr->AddtoDontloadListL( uid );            
            }
        }
    CPIXLOGSTRING("CIndexingManager::UpdateDontloadList : End");
    OstTraceFunctionExit0( CINDEXINGMANAGER_UPDATEDONTLOADLISTL_EXIT );
    }

// -----------------------------------------------------------------------------
// CIndexingManager::GetPluginLoadStatus()
// -----------------------------------------------------------------------------
//
TBool CIndexingManager::GetPluginLoadStatusL (TUid aPluginUid, TInt aVersion, const TDesC& aPluginName)
    {
    OstTraceFunctionEntry0( DUP1_CINDEXINGMANAGER_GETPLUGINLOADSTATUSL_ENTRY );
    //status of plugin in blacklist table
    TBool pluginblacklisted = iBlacklistMgr->FindL( aPluginUid, aVersion);
    //status of plugin in unload table
    TBool loadstatus =  iBlacklistMgr->FindInDontloadListL( aPluginUid );  
    //Check the Uid in both the tables of the blacklist db 
//    if ( loadstatus || pluginblacklisted )
//        loadplugin = EFalse;
    
    if ( loadstatus )
        {
        //Found in unload list.Update the indexing and blacklist status in contentinfo DB
        iContentInfodb->UpdateIndexStatus( QString::fromUtf16(aPluginName.Ptr(), aPluginName.Length()) , KDisable );
        iContentInfodb->UpdateBlacklisted( QString::fromUtf16(aPluginName.Ptr(), aPluginName.Length()), KDisable );
        }
    if ( pluginblacklisted )
        //Update the blacklist status in content info db
        iContentInfodb->UpdateBlacklisted( QString::fromUtf16(aPluginName.Ptr(), aPluginName.Length()), KEnable );
    
    return (! (loadstatus | pluginblacklisted));
    }

// -----------------------------------------------------------------------------
// CIndexingManager::LoadHarvesterpluginL()
// -----------------------------------------------------------------------------
//
void CIndexingManager::LoadHarvesterpluginL (TUid aPluginUid, TInt aVersion, const TDesC& aPluginName)
    {
    OstTraceFunctionEntry0( CINDEXINGMANAGER_LOADHARVESTERPLUGINL_ENTRY );
    CIndexingPlugin* plugin = NULL;
    // Plugin is not black listed. Add it to blacklist database
    iBlacklistMgr->AddL( aPluginUid , aVersion );
    OstTrace1( TRACE_NORMAL, DUP1_CINDEXINGMANAGER_LOADHARVESTERPLUGINL, "CIndexingManager::LoadPluginsL;Plugin with uid=%x is added to DB", aPluginUid.iUid );
    CPIXLOGSTRING2("CIndexingManager::LoadHarvesterpluginL(): Plugin with uid = %x is added to database", aPluginUid.iUid);
    //try to load the plugin
    TRAPD( err, plugin = CIndexingPlugin::NewL( aPluginUid ) );
    if ( err == KErrNone )
        {
        // Plugin loaded succesfully. Remove it from the blacklist database
        iBlacklistMgr->Remove(aPluginUid);
        OstTrace1( TRACE_NORMAL, CINDEXINGMANAGER_LOADHARVESTERPLUGINL, "CIndexingManager::LoadPluginsL;Plugin with uid=%x is removed from DB", aPluginUid.iUid );
        CPIXLOGSTRING2("CIndexingManager::LoadHarvesterpluginL(): Plugin with uid = %x is removed from database", aPluginUid.iUid);
        iContentInfodb->UpdateBlacklisted( QString::fromUtf16(aPluginName.Ptr(), aPluginName.Length()) , KDisable );// new implementation
        CleanupStack::PushL( plugin );
        plugin->SetObserver( *this );
        plugin->SetSearchSession( iSearchSession );
        iPluginArray.AppendL( plugin ); // and add them to array
        CleanupStack::Pop( plugin );
        OstTrace1( TRACE_NORMAL, DUP2_CINDEXINGMANAGER_LOADHARVESTERPLUGINL, "CIndexingManager::LoadPluginsL;Plugin with uid=%x is loaded successfully", aPluginUid.iUid );
        CPIXLOGSTRING2("CIndexingManager::LoadHarvesterpluginL(): Plugin with uid = %x is loaded succesfully", aPluginUid.iUid);
        }
    OstTraceFunctionExit0( CINDEXINGMANAGER_LOADHARVESTERPLUGINL_EXIT );
    }

void CIndexingManager::PausePluginsL()
    {
    if(!iGaurdTimer->IsActive())
        iGaurdTimer->StartgaurdTimer();
    
    for (TInt i = 0; i < iPluginArray.Count(); i++)
        {
        iPluginArray[i]->PausePluginL();
        OstTrace0( TRACE_NORMAL, DUP1_CINDEXINGMANAGER_ACTIVITYCHANGED, "CIndexingManager::PausePluginsL" );
        
        }
    }

void CIndexingManager::ResumePluginsL()
    {
    if( !iHarvestState )
        {
        if( iGaurdTimer->IsActive())
              iGaurdTimer->Cancel();
        
        for (TInt i = 0; i < iPluginArray.Count(); i++)
            {
            iPluginArray[i]->ResumePluginL();
            OstTrace0( TRACE_NORMAL, CINDEXINGMANAGER_ACTIVITYCHANGED, "CIndexingManager::ResumePluginsL" );
            
            }
        }
    }
// -----------------------------------------------------------------------------
// CIndexingManager::ActivityChanged()
// -----------------------------------------------------------------------------
//
void CIndexingManager::ActivityChangedL(const TBool aActive)
    {
    //User is Inactive,so continue with harvesting
    if(aActive)
        {                
        ResumePluginsL();
        }
        else
        {        
        //call pause on all the plugins and restart the gaurd timer
        PausePluginsL();        
        }
    
    }

void CIndexingManager::HandleStateObserverChangeL(const TBool aActive)
    {
    if (aActive)
        {
        iHarvestState = ETrue;
        PausePluginsL();
        }
    else
        {
        iHarvestState = EFalse;
        ResumePluginsL();        
        }
    }
	
// -----------------------------------------------------------------------------
// CIndexingManager::HandleGaurdTimerL()
// -----------------------------------------------------------------------------
//
void CIndexingManager::HandleGaurdTimerL()
    {
     OstTraceFunctionEntry0( CINDEXINGMANAGER_HANDLEGAURDTIMERL_ENTRY );
     //On gaurd timer expiry, Call save on all plugins to store
	 //data to temporary file
     
     // Start timer
     for(TInt i = 0; i < iPluginArray.Count(); i++)
        {
        iPluginArray[i]->SaveL();
        }
     
     if(!iGaurdTimer->IsActive())
         iGaurdTimer->StartgaurdTimer();

    OstTraceFunctionExit0( CINDEXINGMANAGER_HANDLEGAURDTIMERL_EXIT );
    }
