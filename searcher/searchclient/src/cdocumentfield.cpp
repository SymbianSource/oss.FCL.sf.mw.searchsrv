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

#include "CDocumentField.h"

#include <S32STRM.H>
#include <e32std.h>

// Same as CLucene's
const TReal32 CDocumentField::KDefaultBoost = 1.0f; 

EXPORT_C CDocumentField* CDocumentField::NewL( const TDesC& aName, const TDesC& aValue, TInt aConfig )
	{
	CDocumentField* instance = CDocumentField::NewLC( aName, aValue, aConfig );
	CleanupStack::Pop(instance);
	return instance;
	}

EXPORT_C CDocumentField* CDocumentField::NewLC( const TDesC& aName, const TDesC& aValue, TInt aConfig )
	{
	CDocumentField* instance = new (ELeave)CDocumentField();
	CleanupStack::PushL(instance);
	instance->ConstructL(aName, aValue, aConfig  );
	return instance;
	}

EXPORT_C CDocumentField* CDocumentField::NewL(RReadStream& aReadStream)
	{
	CDocumentField* instance = CDocumentField::NewLC(aReadStream);
	CleanupStack::Pop(instance);
	return instance;
	}

EXPORT_C CDocumentField* CDocumentField::NewLC(RReadStream& aReadStream)
	{
	CDocumentField* instance = new (ELeave)CDocumentField();
	CleanupStack::PushL(instance);
	instance->ConstructL(aReadStream);
	return instance;
	}

EXPORT_C const TDesC& CDocumentField::Name() const
	{
	return *iName;
	}

EXPORT_C const TDesC& CDocumentField::Value() const
	{
	return *iValue;
	}

EXPORT_C TInt CDocumentField::Config() const
	{
	return iConfig;
	}

EXPORT_C void CDocumentField::SetBoost( TReal32 aBoost )
	{
	iBoost = aBoost; 
	}

EXPORT_C TReal32 CDocumentField::Boost() const
	{
	return iBoost; 
	}

CDocumentField::CDocumentField()
    : iName(), 
      iValue(),
      iBoost( KDefaultBoost )
	{
	}

EXPORT_C CDocumentField::~CDocumentField()
	{
	delete iName;
	delete iValue;
	}

void CDocumentField::ConstructL( const TDesC& aName, const TDesC& aValue, TInt aConfig )
	{
	iName = aName.AllocL();
	iValue = aValue.AllocL();
	iConfig = aConfig; 
	}

void CDocumentField::ConstructL(RReadStream& aReadStream)
	{
	InternalizeL(aReadStream);
	}

EXPORT_C TInt CDocumentField::Size() const
	{
	return sizeof(TReal32) + sizeof(TInt32) + iName->Size() + sizeof(TInt32) + iValue->Size() + sizeof(TInt32);
	}

EXPORT_C void CDocumentField::ExternalizeL(RWriteStream& aWriteStream) const
	{
	aWriteStream.WriteInt32L(iName->Length());
	aWriteStream.WriteL(*iName, iName->Length());
	
	aWriteStream.WriteInt32L(iValue->Length());
	aWriteStream.WriteL(*iValue, iValue->Length());

	aWriteStream.WriteInt32L( iConfig );	
	aWriteStream.WriteReal32L( iBoost ); 
	}

EXPORT_C void CDocumentField::InternalizeL(RReadStream& aReadStream)
	{
	delete iName;
	iName = NULL;
	TInt nameLenght = aReadStream.ReadInt32L();
	iName = HBufC::NewL(nameLenght);
	
	TPtr namePtr = iName->Des();
	aReadStream.ReadL(namePtr, nameLenght);
	
	delete iValue;
	iValue = NULL;
	TInt valueLenght = aReadStream.ReadInt32L();
	iValue = HBufC::NewL(valueLenght);
	

	TPtr valuePtr = iValue->Des();
	aReadStream.ReadL(valuePtr, valueLenght);

	iConfig = aReadStream.ReadInt32L();
	iBoost = aReadStream.ReadReal32L(); 
	}

