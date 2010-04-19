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

#ifndef CSEARCHDOCUMENT_H_
#define CSEARCHDOCUMENT_H_

#include "CDocumentField.h"

#include <e32base.h>

/**
 * @file
 * @ingroup Common
 * @brief Contains CSearchDocument
 */
/**
 * @brief Represents an entity in the index database
 * @ingroup Common
 * 
 * CSearchDocument represents one entity in index database, e.g. it can appear in search result.
 * Each document may have multiple fields, which values may or may not be indexed and stored to database.
 * See CDocumentField for details.
 * 
 * Link against: CPixSearchClient.lib 
 */
class CSearchDocument : public CBase
	{
public: // Datatypes
	
	enum TFilterId
		{
		ENoFilter,
		EFileParser
		};	
	
	/**
	 * The default boost value for documents
	 */
	const static TReal32 KDefaultBoost; 
	
public: // Constructors and destructor
	
	/**
	 * Creates a new CSearchDocument and returns a pointer to it.
	 * @return A pointer to the new CSearchDocument
	 */
	IMPORT_C static CSearchDocument* NewL(RReadStream& aReadStream);
	
	/**
	 * Creates a new CSearchDocument and returns a pointer to it.
	 * @return A pointer to the new CSearchDocument
	 */
	IMPORT_C static CSearchDocument* NewL(const TDesC& aDocumentId, 
										  const TDesC& aAppClass, 
										  const TDesC& aExcerpt = KNullDesC,
										  const TFilterId aFilterId = ENoFilter );
	
	/**
	 * Creates a new CSearchDocument and returns a pointer to it.
	 * @return A pointer to the new CSearchDocument
	 */	
	IMPORT_C static CSearchDocument* NewLC(RReadStream& aReadStream);
		
	/**
	 * Creates a new CSearchDocument and returns a pointer to it.
	 * @return A pointer to the new CSearchDocument
	 */	
	IMPORT_C static CSearchDocument* NewLC(const TDesC& aDocumentId, 
										   const TDesC& aAppClass, 
										   const TDesC& aExcerpt = KNullDesC,
										   const TFilterId aFilterId = ENoFilter );

	/**
	 * Destructor.
	 */
	IMPORT_C virtual ~CSearchDocument();

public: // New functions
	
	/**
	 * Number of bytes occupied by the data represented by this document.
	 * @return The size of the data represented by this document.
	 */
	IMPORT_C TInt Size() const;
	
	/**
	 * Externalize this document field to a write stream.
	 * @param aWriteStream Write stream for externalization.
	 */
	IMPORT_C void ExternalizeL(RWriteStream& aWriteStream) const;
	
	/**
	 * Internalize this document from a read stream.
	 * @param aReadStream Read stream for internalization.
	 */	
	IMPORT_C void InternalizeL(RReadStream& aReadStream);
	
	/**
	 * Adds new field to this document.
	 * @param aName Name of the new field.
	 * @param aValue Value of the field.
	 * @param aConfig Storing and indexing configuration of this field.
	 * @return Reference to created field.
	 */
	IMPORT_C CDocumentField& AddFieldL(const TDesC& aName, const TDesC& aValue, const TInt aConfig = CDocumentField::KDefaultConfig);

	 /**
	 * TODO
	 */	
	IMPORT_C TBool RemoveField(const TDesC& aName);
		
	/**
	 * Adds excerpt to the document
	 * Overwrites current excerpt
	 * To be used if no exceprt was included on document creation
	 * @param aExcerpt Excerpt text to add to the document.
	 */	
	IMPORT_C void AddExcerptL(const TDesC& aExcerpt);
	
	/**
	 * Find field by name. Returns NULL, if field is not found with given name.
	 * @param aName Name of the field.
	 * @return First field with given name or NULL if no field is found.
	 */
	IMPORT_C const CDocumentField* Field(const TDesC& aName) const;
	
	/**
	 * Id of this document.
	 * @return Id of this document.
	 */
	IMPORT_C const TDesC& Id() const;

	/**
	 * Application class of this document.
	 * @return Application class of this document.
	 */
	IMPORT_C const TDesC& AppClass() const;

	/**
	 * Excerpt of this document.
	 * @return Excerpt of this document.
	 */
	IMPORT_C const TDesC& Excerpt() const;

	/**
	 * Number of fields in this document.
	 * @return Number of fields in this document.
	 */
	IMPORT_C TInt FieldCount() const;
	
	/**
	 * Returns a field specified by the index. 
	 * Raises panic USER 130 if index is negative or greater than the number of fields currently in document.
	 * @param aIndex Index of a field to be returned.
	 * @return Field pointed by the index.
	 */
	IMPORT_C const CDocumentField& Field( const TInt aIndex ) const;
	
	/**
	 * Sets the document boost. Documents with high boost value are
	 * ranked higher in the search results. 
	 */
	IMPORT_C void SetBoost( TReal32 aBoost ); 

	/**
	 * Gets the document boost. Documents with high boost value are
	 * ranked higher in the search results. 
	 */
	IMPORT_C TReal32 Boost() const; 

private: // Constructors
	
	/**
	 * C++ default constructor
	 */
	CSearchDocument();
	
	/**
	 * For documents objects which ID is already known (f.e. Document objects to be created from Hits)
	 */
	void ConstructL(const TDesC& aDocumentId, 
					const TDesC& aAppClass,
					const TDesC& aExcerpt,
					const TFilterId aFilterId );
	
	/**
	 * Symbian second phase constructor.
	 */
	void ConstructL(RReadStream& aReadStream);

private: // Data
	
	/**
	 * Array for the fields of this document.
	 */
	RPointerArray<CDocumentField> iFields;
	
	TReal32 iBoost; 
	
	};

#endif /*CSEARCHDOCUMENT_H_*/
