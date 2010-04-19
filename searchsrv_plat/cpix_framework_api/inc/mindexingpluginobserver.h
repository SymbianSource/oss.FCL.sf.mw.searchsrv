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

#ifndef MINDEXINGPLUGINOBSERVER_H
#define MINDEXINGPLUGINOBSERVER_H

#include <e32base.h>
#include <f32file.h>

/**
 * @file 
 * @ingroup PluginAPI
 * @brief Contains CIndexDatabase and MIndexingService
 */

class CIndexingPlugin;

/**
 * @brief The interface providing access to indexing services for search server plugins
 * @ingroup PluginAPI
 * 
 * MIndexingService provides access to the indexing services for CPix Search Server plugins. 
 * 
 * @note The role of this class has changed -> name should change also. The actual indexing services are now provided directly by CCPixIndexer and this object has become interface to Indexing/Harvesting management/control
 */
class MIndexingService
{
public:
	    	
	/**
	 * A pure virtual method which is called by the plug-in when it wants to schedule a harvesting event
	 *
	 * @param aPlugin the plugin object that calls this function 
     * @param aQualifiedBaseAppClass database to harvest
     * @param aForceReharvest for a reharvest of aQualifiedBaseAppClass
	 */
	virtual void AddHarvestingQueue(CIndexingPlugin* aPlugin, const TDesC& aQualifiedBaseAppClass, TBool aForceReharvest = EFalse) = 0;

	/**
	 * A pure virtual method which is called by the plug-in when it wants to unschedule a harvesting event
	 *
	 * @param aPlugin the plugin object that calls this function 
	 * @param aQualifiedBaseAppClass database to harvest
	 */
	virtual void RemoveHarvestingQueue(CIndexingPlugin* aPlugin, const TDesC& aQualifiedBaseAppClass) = 0;

	/**
	 * A pure virtual method which is called by the plug-in when it has ended harvesting
	 * 
	 * @param aPlugin the plugin object that calls this function 
	 * @param aQualifiedBaseAppClass database to harvest
	 * @param aError KErrNone if harvesting completed successfully otherwise systemwide errorcodes.
	 */
	virtual void HarvestingCompleted(CIndexingPlugin* aPlugin, const TDesC& aQualifiedBaseAppClass, TInt aError) = 0;
};

#endif // MINDEXINGPLUGINOBSERVER_H
