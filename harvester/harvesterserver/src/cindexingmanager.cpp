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
#include "CIndexingManager.h"
#include "HarvesterServerLogger.h"
#include "CBlacklistMgr.h"

_LIT(KManagerFileName, "CPixConfig.bin");
const TInt KManagerFileVersion = 1;

// How often harvester states are checked (in minutes)
const TUint KDefaultWaitTimeInMinutes = 1;

// How often harvester states are checked (in microseconds)
const TUint KDefaultWaitTime = KDefaultWaitTimeInMinutes*60*1000000; // 1 minute

// If time difference between RunL calls is less than this value (i.e system time
// changed to past) update harvesters start and complete times.
const TInt KMinTimeDifference = 0;

// If time difference between RunL calls is more than this value (i.e system time
// changed to future) update harvesters start and complete times. This value is
// default difference in RunL calls with additional buffer.
const TInt KMaxTimeDifference = 1 + KDefaultWaitTimeInMinutes;


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
	iState(EStateNone), iPreviousRun(0)
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
	
	delete iBlacklistMgr;
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

	// Load plugins
	LoadPluginsL();
	
	//release the Blacklist manager as the blacklist functionality is done
	delete iBlacklistMgr;

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
		        CPIXLOGSTRING2("CIndexingManager::RunL(): <%S>, waiting for scheduling...", &iHarvesterArray[i].iQualifiedBaseAppClass);
				break;
			case EHarvesterStatusHibernate:
		        CPIXLOGSTRING2("CIndexingManager::RunL(): <%S>, hibernating.", &iHarvesterArray[i].iQualifiedBaseAppClass);
				break;
			case EHarvesterStatusRunning:
		        CPIXLOGSTRING2("CIndexingManager::RunL(): <%S>, running...", &iHarvesterArray[i].iQualifiedBaseAppClass);
		        break;
			}
		TTimeIntervalMinutes mins_last_complete;
		TTimeIntervalMinutes mins_last_start;
		timenow.MinutesFrom(iHarvesterArray[i].iLastComplete, mins_last_complete);
		timenow.MinutesFrom(iHarvesterArray[i].iLastStart, mins_last_start);
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
			    CPIXLOGSTRING2("CIndexingManager::RunL(): Starting harvesting <%S>.", &iHarvesterArray[i].iQualifiedBaseAppClass);
				TRAPD(err, iHarvesterArray[i].iPlugin->StartHarvestingL(iHarvesterArray[i].iQualifiedBaseAppClass));

				// No need to continue, if something was already started 
				if (err == KErrNone)
					{
				    CPIXLOGSTRING("CIndexingManager::RunL(): Successfully started harvesting. ");
					break;
					}
				
				// Set the error status
				iHarvesterArray[i].iLastComplete.UniversalTime();
				iHarvesterArray[i].iError = err;
				// Do not run it again, unless CIndexingMgr informed to do so
				iHarvesterArray[i].iStatus = EHarvesterStatusHibernate;
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
	iTimer.After(iStatus, KDefaultWaitTime);
	SetActive();
	
	}

// -----------------------------------------------------------------------------
// CIndexingManager::LoadPluginsL()
// -----------------------------------------------------------------------------
//
void CIndexingManager::LoadPluginsL()
	{
    RImplInfoPtrArray infoArray;    
    TCleanupItem cleanupItem( CPixSearchECom::CleanupEComArray, &infoArray );
    CleanupStack::PushL( cleanupItem );
    
    CIndexingPlugin::ListImplementationsL( infoArray );
    TInt count( 0 );
    count = infoArray.Count();
    
    //FFLOGSTRING2( "CFastFindHarvesterPluginControl:: PLUGINS COUNT %d", count );
    CIndexingPlugin* plugin = NULL;
    
    for ( TInt i = 0; i < count; i++ )
        {
        TUid uid = infoArray[i]->ImplementationUid();    // Create the plug-ins
        TInt version = infoArray[i]->Version();
        //FFLOGSTRING2( "CFastFindHarvesterPluginControl:: PLUGINS UID %x", uid );
        plugin = NULL;
        TBool pluginblacklisted = EFalse;
        
        pluginblacklisted = iBlacklistMgr->FindL( uid , version );        
        
        if ( !pluginblacklisted )
            {
            // Plugin is not black listed. Add it to database and try to load the plugin
            iBlacklistMgr->AddL( uid , version );
            CPIXLOGSTRING2("CIndexingManager::LoadPluginsL(): Plugin with uid = %x is added to database", uid.iUid);
            TRAPD( err, plugin = CIndexingPlugin::NewL( uid ) );
            //FFLOGSTRING2( "CFastFindHarvesterPluginControl:: ERROR %d", err );
            if ( err == KErrNone )
                {
                // Plugin loaded succesfully. Remove it from the database
                iBlacklistMgr->Remove(uid);
                CPIXLOGSTRING2("CIndexingManager::LoadPluginsL(): Plugin with uid = %x is removed from database", uid.iUid);
                CleanupStack::PushL( plugin );
                plugin->SetObserver( *this );
                plugin->SetSearchSession( iSearchSession );
                iPluginArray.AppendL( plugin ); // and add them to array
                CleanupStack::Pop( plugin );
                CPIXLOGSTRING2("CIndexingManager::LoadPluginsL(): Plugin with uid = %x is loaded succesfully", uid.iUid);
                }
            }
        }    
    CleanupStack::PopAndDestroy( &infoArray ); // infoArray, results in a call to CleanupEComArray
    //FFLOGSTRING( "CFastFindHarvesterPluginControl::LoadPluginsL() plugin!" );    
    
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
					CPIXLOGSTRING("CIndexingManager::AddHarvestingQueue(): Needs Reharvesting.");
					// iLastCompete Time is reset so that will be reharvested as soon
					// as possible
					iHarvesterArray[i].iLastComplete = TTime(0);
					// No need to reset iLastStart or iError
					iHarvesterArrayChanged = ETrue;
					}
				}
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

    CPIXLOGSTRING("CIndexingManager::AddHarvestingQueue(): Harvester added successfully to the queue.");	
	}

// -----------------------------------------------------------------------------
// CIndexingManager::RemoveHarvestingQueue()
// -----------------------------------------------------------------------------
//
void CIndexingManager::RemoveHarvestingQueue(CIndexingPlugin* aPlugin, const TDesC& aQualifiedBaseAppClass)
	{
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
			CPIXLOGSTRING("CIndexingManager::RemoveHarvestingQueue(): Harvester de-queued successfully.");	
			return;
			}
		}

	CPIXLOGSTRING("CIndexingManager::RemoveHarvestingQueue(): Harvester in the queue.");	
	}

// -----------------------------------------------------------------------------
// CIndexingManager::HarvestingCompleted()
// -----------------------------------------------------------------------------
//
void CIndexingManager::HarvestingCompleted(CIndexingPlugin* aPlugin, const TDesC& aQualifiedBaseAppClass, TInt aError)
	{
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

			CPIXLOGSTRING("CIndexingManager::HarvestingCompleted(): Harvesting status changed successfully.");
			return;			
			}
		}
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

