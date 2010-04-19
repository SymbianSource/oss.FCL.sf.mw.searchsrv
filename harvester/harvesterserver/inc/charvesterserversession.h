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

#ifndef CHARVESTERSERVERSESSION_H
#define CHARVESTERSERVERSESSION_H

// INCLUDE FILES
#include <e32base.h>

// CLASS DECLARATION
/**
 * CHarvesterServerSession.
 *  An instance of class CHarvesterServerSession is created for each client.
 */
class CHarvesterServerSession : public CSession2
	{
	public: // Constructors and destructors		
	
		/**
		 * NewL.
		 * Two-phased constructor.
		 * @return Pointer to created CHarvesterServerSession object.
		 */
		static CHarvesterServerSession* NewL();
	
		/**
		 * NewLC.
		 * Two-phased constructor.
		 * @return Pointer to created CHarvesterServerSession object.
		 */
		static CHarvesterServerSession* NewLC();
	
		/**
		 * ~CHarvesterServerSession.
		 * Destructor.
		 */
		virtual ~CHarvesterServerSession();

	public: // Functions from base classes
	

		/**
		 * From CSession, ServiceL.
		 * Service request from client.
		 * @param aMessage Message from client
		 *                 (containing requested operation and any data).
		 */
		void ServiceL(const RMessage2& aMessage);

	private: // Constructors and destructors
		
	
		/**
		 * CHarvesterServerSession.
		 * C++ default constructor.
		 * @param aServer The server.
		 */
		CHarvesterServerSession();
	
		/**
		 * ConstructL.
		 * 2nd phase constructor.
		 */
		void ConstructL();

	private: // New methods
				
		/**
		 * PanicClient.
		 * Causes the client thread to panic.
		 * @param aMessage Message from client.
		 * @param aPanic Panic code.
		 */
		void PanicClient(const RMessage2& aMessage, TInt aPanic) const;

	private: // Data
		
		// No data
	};

#endif // CHARVESTERSERVERSESSION_H

// End of File
