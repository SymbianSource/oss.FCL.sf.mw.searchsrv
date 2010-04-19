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

#ifndef LOG_H_
#define LOG_H_

#include <e32base.h>

#include <f32file.h>

#include <wchar.h>

class CConsoleBase; 
class RFileWriteStream; 

/**
 *
 */
class MLog 
	{
	public: 
		virtual void LogL( TRefByValue<const TDesC> aDesc, ... );
		virtual void VLogL( const TDesC& aDesc, VA_LIST list ) = 0;

		virtual void FlushL() = 0; 
	};

class CLog : public CBase, public MLog
	{
	};

class CFileConsoleLog : public CLog
	{
	public: 
		CFileConsoleLog( CConsoleBase* aConsole );
		void ConstructL( RFs& aFs, const TDesC& aFile ); 
		void ConstructL( const TDesC& aFile ); 
		~CFileConsoleLog();

		static CFileConsoleLog* NewL( CConsoleBase* aConsole,
									  const TDesC& aFile );

	public: 
		virtual void VLogL( const TDesC& aDesc, VA_LIST aArgs );
		
		virtual void FlushL(); 

		
	private: 
		// Nulls are used to mark that the output target is not used
		
		CConsoleBase* iConsole; 
		RFile* iFile; 
		RFs* iFs; 
	};

class TShortTimeStampLog : public MLog
	{
	public:
		TShortTimeStampLog(); 
		TShortTimeStampLog( MLog& aLog );
		
		void SetLog(MLog& aLog);

		virtual void VLogL( const TDesC& aDesc, VA_LIST aArgs );
		virtual void FlushL(); 

	private: 
		
		MLog* iLog; 
	};


#endif /*LOG_H_*/
