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

#include "CSearchDocument.h"
#include "CDocumentField.h"
#include "HarvesterServerLogger.h"

#include "indevicecfg.h"

/*
 * This is exactly the same as in CLucene
 */
const TReal32 CSearchDocument::KDefaultBoost = 1.0f;

EXPORT_C CSearchDocument* CSearchDocument::NewL(RReadStream& aReadStream)
	{
	CSearchDocument* self = CSearchDocument::NewLC(aReadStream);
	CleanupStack::Pop(self);
	return self;
	}

EXPORT_C CSearchDocument* CSearchDocument::NewL(const TDesC& aDocumentId, 
												const TDesC& aAppClass, 
												const TDesC& aExcerpt,
												const TFilterId aFilterId)
	{
	CSearchDocument* self = CSearchDocument::NewLC(aDocumentId, aAppClass, aExcerpt, aFilterId);
	CleanupStack::Pop(self);
	return self;
	}

EXPORT_C CSearchDocument* CSearchDocument::NewLC(RReadStream& aReadStream)
	{
	CSearchDocument* self = new (ELeave)CSearchDocument();
	CleanupStack::PushL(self);
	self->ConstructL(aReadStream);
	return self;
	}

EXPORT_C CSearchDocument* CSearchDocument::NewLC(const TDesC& aDocumentId, 
												 const TDesC& aAppClass, 
												 const TDesC& aExcerpt,
												 const TFilterId aFilterId)
	{
	CSearchDocument* self = new (ELeave)CSearchDocument();
	CleanupStack::PushL(self);
	self->ConstructL(aDocumentId, aAppClass, aExcerpt, aFilterId);
	return self;
	}

CSearchDocument::CSearchDocument()
:   iFields(),
	iBoost( KDefaultBoost )
	{
	}

EXPORT_C CSearchDocument::~CSearchDocument()
	{
	iFields.ResetAndDestroy();
	iFields.Close();
	}

void CSearchDocument::ConstructL(RReadStream& aReadStream)
	{
	InternalizeL(aReadStream);
	}

void CSearchDocument::ConstructL(const TDesC& aDocumentId, 
								 const TDesC& aAppClass, 
								 const TDesC& aExcerpt,
								 const TFilterId aFilterId)
	{
	// creating unique string identifier:
	AddFieldL( _L( CPIX_DOCUID_FIELD ), aDocumentId, CDocumentField::EStoreYes | CDocumentField::EIndexUnTokenized );
	if ( KNullDesC() != aAppClass )
		{
		AddFieldL( _L( CPIX_APPCLASS_FIELD ), aAppClass, CDocumentField::EStoreYes | CDocumentField::EIndexTokenized );
		}
	if ( KNullDesC() != aExcerpt )
		{
		AddFieldL( _L( CPIX_EXCERPT_FIELD ), aExcerpt, CDocumentField::EStoreYes | CDocumentField::EIndexNo );
		}

	switch (aFilterId)
		{
		case ENoFilter:
			{
			// Nothing to do
			break;
			}
		case EFileParser:
			{
			// TODO: These must match to definitions in cpixdoc.cpp
			_LIT(KFilterFieldName, CPIX_FILTERID_FIELD);
			_LIT(KFilterFieldValue, "FileParser");
			AddFieldL(KFilterFieldName, KFilterFieldValue, CDocumentField::EStoreYes | CDocumentField::EIndexNo );
			break;
			}
		default:
			User::Leave(KErrArgument);
		}
	}

EXPORT_C TInt CSearchDocument::Size() const
	{
	TInt r = sizeof(iFields.Count());
	
	r += sizeof(TReal); 

	for ( TInt i = 0; i < iFields.Count(); ++i )
		{
		r += iFields[i]->Size();
		}
	
	return r;
	}

EXPORT_C void CSearchDocument::ExternalizeL(RWriteStream& aWriteStream) const
	{
	aWriteStream.WriteReal32L( iBoost ); 
	
	aWriteStream.WriteInt32L(iFields.Count());
	for (TInt i=0; i<iFields.Count(); i++)
		{
		iFields[i]->ExternalizeL(aWriteStream);
		}
	}

EXPORT_C void CSearchDocument::InternalizeL(RReadStream& aReadStream)
	{
	iBoost = aReadStream.ReadReal32L(); 

	TInt32 count = aReadStream.ReadInt32L();
	for (TInt i=0; i<count; i++)
		{
		CDocumentField* new_field = CDocumentField::NewLC(aReadStream);
		iFields.AppendL(new_field);
		CleanupStack::Pop(new_field);
		}
	}

EXPORT_C CDocumentField& CSearchDocument::AddFieldL(const TDesC& aName, const TDesC& aStringValue, TInt aConfig)
	{
	RemoveField( aName );

	CDocumentField* new_field = CDocumentField::NewLC(aName, aStringValue, aConfig);
	iFields.AppendL(new_field);
	CleanupStack::Pop(new_field);
	return *new_field;
	}

EXPORT_C TBool CSearchDocument::RemoveField(const TDesC& aName)
	{
	for ( TInt i = 0; i < iFields.Count(); ++i )
		{
		if ( aName == iFields[i]->Name() )
			{
			delete iFields[i];
			iFields.Remove(i);
			return true;  
			}
		}
	return false; 
	}

EXPORT_C void CSearchDocument::AddExcerptL(const TDesC& aExcerpt)
	{
	AddFieldL( _L( CPIX_EXCERPT_FIELD ), aExcerpt, CDocumentField::EStoreYes | CDocumentField::EIndexNo ); 
	}

EXPORT_C const CDocumentField* CSearchDocument::Field(const TDesC& aName) const
	{
	for ( TInt i = 0; i < iFields.Count(); ++i )
		{
		if ( aName == iFields[i]->Name() )
			{
			return iFields[i];
			}
		}

	return NULL;
	}

EXPORT_C TInt CSearchDocument::FieldCount() const
	{
	return iFields.Count();
	}

EXPORT_C const CDocumentField& CSearchDocument::Field( const TInt aIndex ) const
	{
	return *iFields[ aIndex ];
	}

EXPORT_C const TDesC& CSearchDocument::Id() const
	{
	const CDocumentField* field = Field( _L( CPIX_DOCUID_FIELD ) ); 
	if ( field )
		{
		return field->Value(); 
		}
	return KNullDesC();
	}

EXPORT_C const TDesC& CSearchDocument::AppClass() const
	{
	const CDocumentField* field = Field( _L( CPIX_APPCLASS_FIELD ) ); 
	if ( field )
		{
		return field->Value(); 
		}
	return KNullDesC();
	}

EXPORT_C const TDesC& CSearchDocument::Excerpt() const
	{
	const CDocumentField* field = Field( _L( CPIX_EXCERPT_FIELD ) ); 
	if ( field )
		{
		return field->Value(); 
		}
	return KNullDesC();
	}

EXPORT_C void CSearchDocument::SetBoost( TReal32 aBoost )
	{
	iBoost = aBoost; 
	}


EXPORT_C TReal32 CSearchDocument::Boost() const
	{
	return iBoost; 
	}

