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


#ifndef __CSEARCHSESSION_H__
#define __CSEARCHSESSION_H__

// INCLUDE FILES
#include <e32base.h>

// FORWARD DECLARATIONS
class CSearchServerSubSession;
class CSearchServer; 

// CLASS DECLARATION
/**
 * CSearchServerSession.
 *  An instance of class CSearchServerSession is created for each client.
 */
class CSearchServerSession : public CSession2
	{
	public: // Constructors and destructors		
	
		/**
		 * NewL.
		 * Two-phased constructor.
		 * @return Pointer to created CSearchServerSession object.
		 */
		static CSearchServerSession* NewL();
	
		/**
		 * NewLC.
		 * Two-phased constructor.
		 * @return Pointer to created CSearchServerSession object.
		 */
		static CSearchServerSession* NewLC();
	
		/**
		 * ~CSearchServerSession.
		 * Destructor.
		 */
		virtual ~CSearchServerSession();

	public: // Functions from base classes
	

		/**
		 * From CSession, ServiceL.
		 * Service request from client.
		 * @param aMessage Message from client
		 *                 (containing requested operation and any data).
		 */
		void ServiceL(const RMessage2& aMessage);
		
		/** 
		 * CreateL()
		 * Called by client/server framework after 
		 * session has been successfully created
		 */
	    void CreateL(); 
		
	    /**
	     * NewSubSessionL();
	     * Creates new subsession
	     */
	    void NewSubSessionL(const RMessage2& aMessage);  
	      
	    /**
	     * CloseSession();
	     * Closes the session
	     */
		void CloseSession(const RMessage2& aMessage);

	    /**
	     * NewSubSessionL();
	     * Delete the subsession object through its handle.
	     */
		void DeleteCounter(TInt aHandle);

	    /**
	     * SubSessionFromHandle();
	     * Utility to return the CSearchServerSubSession (subsession) object
	     */
		CSearchServerSubSession* SubSessionFromHandle(const RMessage2& aMessage,TInt aHandle);
		
	    /**
	     * Stop housekeeping
	     */
	    void StopHouseKeeping(const RMessage2& aMessage);
	    
	    /**
	     * Continue housekeeping
	     */
	    void ContinueHouseKeeping(const RMessage2& aMessage);
	    
	    /**
	     * Force housekeeping
	     */
	    void ForceHouseKeeping(const RMessage2& aMessage);
	    
	private: // Constructors and destructors
		
	
		/**
		 * CSearchServerSession.
		 * C++ default constructor.
		 * @param aServer The server.
		 */
		CSearchServerSession();
	
		/**
		 * ConstructL.
		 * 2nd phase constructor.
		 */
		void ConstructL();

		/**
		 * DefineVolumeL.
		 * Define a specific volume identified by aQualifiedBaseAppClass
		 * @param aMessage contains parameter containing qualified base AppClass to be
		 * defined.
		 */
		void DefineVolumeL(const RMessage2& aMessage);

		void DefineVolumeL(const TDesC& aQualifiedBaseAppClass,
						   const TDesC& aIndexDbPath);

		/**
		 * UnDefineVolumeL.
		 * Undefine a specific volume identified by aQualifiedBaseAppClass
		 * @param aMessage contains parameter containing qualified base AppClass to be
		 * undefined.
		 */
		void UnDefineVolumeL(const RMessage2& aMessage);

		void UnDefineVolumeL(const TDesC& aBaseAppClass);
		
	public: 
				
		/**
		 * PanicClient.
		 * Causes the client thread to panic.
		 * @param aMessage Message from client.
		 * @param aPanic Panic code.
		 */
		void PanicClient(const RMessage2& aMessage, TInt aPanic) const;

	private: // Data
		
		// Subsession members
		CObjectCon *iContainer;
		CObjectIx* iCountersObjectIndex;
	    TInt iResourceCount;
	};

#endif // __CSEARCHSESSION_H__

// End of File
