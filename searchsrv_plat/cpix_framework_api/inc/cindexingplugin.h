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

#ifndef INDEXINGPLUGIN_H
#define INDEXINGPLUGIN_H

#include <e32base.h>
#include <ecom/ImplementationInformation.h>
#include <MIndexingPluginObserver.h>
#include <RSearchServerSession.h>
#include <CCPixIndexer.h>

/** @addtogroup PluginAPI */
/*\@{*/

/**
 * @file
 * @ingroup PluginAPI
 * @brief Contains CIndexingPlugin and related definitions
 */

// Uid for this interface
const TUid KIndexingPluginInterfaceUid = { 0x2001F700 };


/**
 * @ingroup PluginAPI
 * @brief Contains ECOM interface related methods
 */

namespace CPixSearchECom
    {   
   /**
     * CleanupPtrArray function is used for cleanup support of locally declared arrays.
     * @param aArray  An array to cleanup
     */
    template<typename T> inline void CleanupPtrArray( TAny* aArray )
        {
        static_cast<RPointerArray<T>*>(aArray)->ResetAndDestroy();
        static_cast<RPointerArray<T>*>(aArray)->Close();
        }
        
        /**
         * CleanupEComArray function is used for cleanup support of locally declared arrays.
         * @param aArray  An array to cleanup
         */
     inline void CleanupEComArray( TAny* aArray )
            {
            CleanupPtrArray<CImplementationInformation>( aArray );
            }
    }


/**
 * @brief Base class for CPix Search Server plugins, responsible for harvesting
 * 
 * CIndexPlugin is a base class for Search Server plugins, which are responsible
 * for maintaining index databases for various datasources up to date. The plugins are 
 * expected to maintain the databases through harvesting and monitoring activities, 
 * where all items of the datasources are first indexed and after that the datasource
 * is observed and the occured changes (insertions, updates, deletions) are applied
 * also to the index database. 
 *
 * Link against: CPixSearchPluginInterface.dll
 */
class CIndexingPlugin : public CBase
{
public:
	IMPORT_C static CIndexingPlugin* NewL(const TUid aUid);
	IMPORT_C virtual ~CIndexingPlugin();
	
	IMPORT_C static void ListImplementationsL(RImplInfoPtrArray& aImplInfoArray);	
	
	/**
	* Set observer
	*
	*/
	IMPORT_C void SetObserver(MIndexingService& aObserver);
	
	/**
	* SetSearchSession
	* Sets the RSearchServerSession for the plugin.
	*
	* @param aSearchSession search server session to be assigned to
	* harvester plugins
	*/
	IMPORT_C void SetSearchSession(RSearchServerSession& aSearchSession);

	/**
	* A pure virtual method which starts the monitoring/harvesting plugin
	* Observer must be set before harvesting is started. 
	*/
	virtual void StartPluginL() = 0;
	
	/**
	* A pure virtual method which instructs plugin to start harvesting for certain media.
	* Observer must be set before harvesting is started. 
	*
	* @param aQualifiedBaseAppClass database to harvest
	*/
	virtual void StartHarvestingL(const TDesC& aQualifiedBaseAppClass) = 0;
	
	/**
	 * Flush
	 * Issue CPix flush. Harvester should call this method after 
	 * harvesting is done.
	 * 
	 * @param aIndexer Indexer to be flushed
	 */
	IMPORT_C void Flush(CCPixIndexer& aIndexer);
	
	/**
    * A pure virtual method which instructs plugin to pause harvesting.
    * Data to be indexed has to be stored and later index when resume is called.    
    */
	virtual void PausePluginL() = 0;
	
	/**
    * A pure virtual method which instructs plugin to resume harvesting.
    * plugin should index the queued items if any.    
    */
	virtual void ResumePluginL() = 0;
	
private: // data members
	// Identification on cleanup.
	TUid iDtor_ID_Key;
protected:
	// Observer interface
	MIndexingService* iObserver;

    /* Search Server session member is used by harvester plugins */
    RSearchServerSession iSearchSession;
};

/*\@}*/

#endif // INDEXINGPLUGIN_H
