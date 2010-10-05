/*
* Copyright (c) 2009 Nokia Corporation and/or its subsidiary(-ies). 
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
* Description:  Monitoring application for servers restarting & IAD
*
*/

#ifndef SERVERMONITOR_H
#define SERVERMONITOR_H

#include <e32base.h>

class CServerMonitor : public CActive
    {
    public:
    
        enum TServerState
            {
            EIdle,
            EWaitingRendezvous,
            ERunning
        };

        static CServerMonitor* NewL( TDesC& aServerName , TUid aServerUid );
	    virtual ~CServerMonitor();

        // from base class CActive
	    void RunL();
        TInt RunError( TInt aError );
	    void DoCancel();
		
		void ShutdownServer();

    private:

        CServerMonitor();
        
        void ConstructL( TDesC& aServerName , TUid aServerUid );
        
        void StartL();
        
        void StartServerProcessL();
        
        void CreateServerProcess();

    private: // data

        TServerState iState;
        
        RProcess iProcess;
		
		HBufC* iServerName;
		
		TUid iServerUid;
  
    };


#endif //SERVERMONITOR_H
