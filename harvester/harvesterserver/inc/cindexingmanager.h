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

#ifndef CINDEXINGMANAGER_H
#define CINDEXINGMANAGER_H

#include <CIndexingPlugin.h>
#include <MIndexingPluginObserver.h>
#include <RSearchServerSession.h>

const TInt KFilePluginBaseAppClassMaxLen = 64;
//Forward Declaration
class CBlacklistMgr;
class CContentInfoMgr;

class CIndexingManager : public CActive, public MIndexingService
	{
public:	
	static CIndexingManager* NewL();
	virtual ~CIndexingManager();

protected:
	/*
	 * From CActive
	 */
	void DoCancel();
	void RunL();

protected:	
	/**
	 * derived from MIndexingService
	 * A callback from CIndexingPlugin, called when it wants to schedule a harvesting event
	 *
     * @param aQualifiedBaseAppClass database to harvest
	 * @param aMedia the media to be harvested
	 * @param aForceReharvest force the aQualifiedBaseAppClass to be re-harvested.
	 */
	void AddHarvestingQueue(CIndexingPlugin* aPlugin, const TDesC& aQualifiedBaseAppClass, TBool aForceReharvest = EFalse);

	/**
	 * 
	 * derived from MIndexingService
	 * A callback from CIndexingPlugin, called when it wants to unschedule a harvesting event
	 *
     * @param aQualifiedBaseAppClass database to harvest
	 * @param aMedia the media to be harvested
	 * @param aRemovePersist if plugin wants to remove from harvesting queue as well as config to be saved
	 */
	void RemoveHarvestingQueue(CIndexingPlugin* aPlugin, const TDesC& aQualifiedBaseAppClass,TBool aRemovePersist = EFalse);

	/**
	 * derived from MIndexingService
	 * A callback from CIndexingPlugin, called when it has ended harvesting
	 * 
	 * @param aPlugin the plugin object that calls this function 
     * @param aQualifiedBaseAppClass database to harvest
	 * @param aError KErrNone if harvesting completed successfully otherwise systemwide errorcodes
	 */
	void HarvestingCompleted(CIndexingPlugin* aPlugin, const TDesC& aQualifiedBaseAppClass, TInt aError);
						
private:

	/**
	 * LoadPluginsL loads all plugins
	 */
	void LoadPluginsL();

	/**
	 * StartPlugins starts all loaded plugins 
	 */
	void StartPlugins();

	/**
	 * Loading the state of the Plugins
	 */
	TInt Internalize();
	/**
     * saving the state of the Plugins
     */
	TInt Externalize();
	/**
     * Loading the state of the Plugins
     */
	void LoadL();
	/**
     * saving the state of the Plugins
     */
	void SaveL();
	/**
     * Update content info Db with the plugin details
     */
	void UpdateContentInfoDbL( const TDesC& aPluginName);
	/**
     * Update the unload list in a separate table in blacklist database
     */
	void UpdateUnloadListL();
	
private:
	CIndexingManager();
	void ConstructL();

private:
	enum THarvesterStatus
		{
		EHarvesterStatusWaiting = 0, // A Plugin has requested this item to be harvested
		EHarvesterStatusHibernate, // A plugin has requested for this item not to be harvested
		EHarvesterStatusRunning // This item is being harvested
		};

	/*
	 * Class to hold the details of a harvester plugin
	 */
	class THarvesterRecord
		{
	public:
	    /*
	     * Default constructor
	     */
		THarvesterRecord()
			{
			iPlugin = NULL;
			iError = KErrNone;
			iStatus = EHarvesterStatusHibernate;
			iLastStart = TTime(0);
			iLastComplete = TTime(0);
			}
		THarvesterRecord(CIndexingPlugin* aPlugin, const TDesC& aQualifiedBaseAppClass)
			{
			iPlugin = aPlugin;
			iQualifiedBaseAppClass = aQualifiedBaseAppClass;
			iError = KErrNone;
			iStatus = EHarvesterStatusWaiting;
			iLastStart = TTime(0); // Never started
			iLastComplete = TTime(0); // Never complete
			}
	public:
		/*
		 * Pointer to hold a harvester plugin
		 */
		CIndexingPlugin* iPlugin;
		/*
         * Variable to hold the error code of plugin harvesting process
         */
		TInt iError;
		/*
         * status[waiting/hibernate/running] a harvester plugin
         */
		THarvesterStatus iStatus;
		/*
		 * Time before starting the harvesting of a plugin
		 */
		TTime iLastStart;
		/*
		 * Time after completing the harvesting of a plugin
		 */
		TTime iLastComplete;
		/*
         * Base app class of a harvester plugin
         */
		TBuf<KFilePluginBaseAppClassMaxLen> iQualifiedBaseAppClass;
		};
	
private:
	enum TState
		{
		EStateNone = 0, // Initial state
		EStateCancelling, // Indexing manager is cancelled
		EStateRunning // Indexing manager is running
		};
	
private:	
	/* List of plugins currently loaded */
	RPointerArray<CIndexingPlugin> iPluginArray;
	
	/* List of harvester events */
	RArray<THarvesterRecord> iHarvesterArray;
	TBool iHarvesterArrayChanged;
	
	/* Timer event */
	RTimer iTimer;
	
	/* Manager state */
	TState iState;

	/* Search Server session is passed to harvester plugins */
	RSearchServerSession iSearchSession;
	
	/* Path to the manager state */
	TFileName iManagerFilePath;
	
	/* File server connection */ 
	RFs iFs;
	
	/* Time of previous RunL call */
	TTime iPreviousRun;
	/* Database to maintain blacklisted plugins.Owned */
	CBlacklistMgr* iBlacklistMgr;
	/* Database to maintain the content info all the plugins.owned*/
	CContentInfoMgr* iContentInfoMgr;
	};

#endif // CINDEXINGMANAGER_H
