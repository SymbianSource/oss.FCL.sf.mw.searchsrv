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
* Description:  MDE harvester utility class
*
*/

#ifndef CPIXSTATEOBSERVER_H_
#define CPIXSTATEOBSERVER_H_

#include <e32def.h>
#include <e32base.h>
#include <f32file.h>
#include <usbmsshared.h>
#include <mpxcollectionobserver.h>
#include <harvesterclient.h>

class MMPXCollectionUtility;

class MCPiXStateObserver
    {
public:
    /* A pure virtual function that need to be handled for cases of MMC harvesting,
     * Musicplayer sync events by pausing the currect indexing operations
     */
    virtual void HandleStateObserverChangeL(const TBool aActive) = 0;
    };

class CPiXStateObserver : public CBase,
                          public MHarvesterEventObserver,
                          public MMPXCollectionObserver
{
public:
    /**
     * NewL construction.
     * @return Pointer to created CPiXStateObserver object.
     */
    static CPiXStateObserver* NewL( MCPiXStateObserver* aObserver, RFs* aFsSession );    

    /**
     * Destructor.
     */
    virtual ~CPiXStateObserver();
    
    void StartMonitoringL();
    
private:  // Constructors and destructors

    /**
     * C++ default constructor.
     * @param aObserver Observer
     */
    CPiXStateObserver(MCPiXStateObserver* aObserver);

    /**
     * 2nd phase constructor.
     */
    void ConstructL( RFs* aFsSession );    
    
    //void UpdatePSValue();
    
    // from MHarvesterEventObserver
    void HarvestingUpdated( HarvesterEventObserverType aHEObserverType, 
                            HarvesterEventState aHarvesterEventState,
                            TInt aItemsLeft );
        
    // from MMPXCollectionObserver
    void HandleCollectionMessage( CMPXMessage* aMessage,  TInt aError );
    
    //from MMPXCollectionObserver::HandleOpenL
    void HandleOpenL(const CMPXMedia& aEntries, TInt aIndex, TBool aComplete, TInt aError);

    //from MMPXCollectionObserver::HandleOpenL
    void HandleOpenL(const CMPXCollectionPlaylist& aPlaylist, TInt aError);  
    
    //from MMPXCollectionObserver::HandleCollectionMediaL
    void HandleCollectionMediaL( const CMPXMedia& aMedia, TInt aError );
    
    TBool GetStatus();
    
    void NotifyObserverL();
    
private:
    
    //MDS Harvester client
   RHarvesterClient iHarvesterClient;
   
   MMPXCollectionUtility* iCollectionUtility;
   
   MCPiXStateObserver* iObserver;
   
   //RProperty iDriveState;
   
   TBool iMMCHarvesting;
   TBool iMPXHarvesting;
   // inserted/ejected status of mmc
   //TBool iMassStorageMode;
   
   TInt iPreviousState;
   TInt  itemsleft;   
   RFs* iFsSession;
  
};
#endif /* CPIXSTATEOBSERVER_H_ */
