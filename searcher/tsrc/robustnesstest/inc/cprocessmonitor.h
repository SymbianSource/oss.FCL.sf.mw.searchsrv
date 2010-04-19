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


#ifndef CPROCESSMONITOR_H_
#define CPROCESSMONITOR_H_

#include <e32base.h>

class CProcessMonitor; 

class MProcessMonitorObserver 
	{
public:
	virtual void ProcessFinished( CProcessMonitor& aMonitor, 
							      TExitType aExitType ,
							      TExitCategoryName aCategory,
							      TInt aExitReason ) = 0; 
	virtual void Failed( CProcessMonitor& aMonitor,
					     TInt aError ) = 0; 
	};

class CProcessMonitor : public CActive
	{
public: 
	
	CProcessMonitor(); 
	
	~CProcessMonitor(); 
	
	void StartL( MProcessMonitorObserver& aObserver,
				 const TDesC& aProcessName );
	
	const TDesC& ProcessName(); 
	
protected: 

	void Cleanup(); 
		
	void DoCancel(); 

	void RunL(); 
	
private: 
	
	MProcessMonitorObserver* iObserver; 
	
	RProcess iProcess; 
	
	HBufC* iProcessName; 

	};

#endif /* CPROCESSMONITOR_H_ */
