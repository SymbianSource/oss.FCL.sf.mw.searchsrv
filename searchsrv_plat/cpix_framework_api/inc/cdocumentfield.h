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

#ifndef CDOCUMENTFIELD_H_
#define CDOCUMENTFIELD_H_

#include <e32base.h>
#include <S32STRM.H>

/**
 * @file
 * @ingroup Common
 * @brief Contains CDocumentField 
 */

/**
 * @brief Represents a field in a search document
 * @ingroup Common
 * 
 * Instance of this class describes one field of a CSearchDocument. 
 * Each field has name and value. Value can be indexed as untokenized or tokenized string(s), 
 * when it can be found by submitting query. Value can also be stored to database in order 
 * to retrieve it via founded CSearchDocument. 
 * 
 * Link against: CPixSearchClient.lib 
 */
class CDocumentField : public CBase
	{
public: // Data types and constants

	/**
	 * TStored defines wheter the value is stored to database.
	 * If value is stored to database, it can be retrieved from search result. 
	 */
	enum TStored
		{
		EStoreYes = 1,
		EStoreNo = 2
		};

	/**
	 * TIndexed defines how the value of the field is indexed. 
	 * If value is indexed, it can be searched. 
	 */
	enum TIndexed
		{
		EIndexNo = 16,
		EIndexTokenized = 32,
		EIndexUnTokenized = 64
		};

	/**
	 * TAggregated defines how the value of the field are exposed 
	 * for aggregation. Aggregated field can be found with generic searches. 
	 * If aggregation is not specified, all indexed field are aggeregated by
	 * default. Note: that also non-indexed fields can be aggregated.
	 */
	enum TAggregated
		{
		EAggregateNo = 1<<30,
		EAggregateYes = 1<<31,
		EAggregateDefault = 0
		};
	/**
	 * By default, field value is stored to database and it's indexed as tokenized strings.
	 */
	static const TInt KDefaultConfig = EStoreYes | EIndexTokenized | EAggregateDefault;
	
	static const TReal32 KDefaultBoost; 
	
public: // Symbian constructors and destructor

	/**
	 * Creates a new CDocumentField and returns a pointer to it.
	 * @param aName Name of this field.
	 * @param aValue Value of this field.
	 * @param aConfig Configuration of this field.
	 * @return A pointer to the new CDocumentField
	 */
	IMPORT_C static CDocumentField* NewL( const TDesC& aName, const TDesC& aValue, TInt aConfig = KDefaultConfig );

	/**
	 * Creates a new CDocumentField and returns a pointer to it.
	 * @param aName Name of this field.
	 * @param aValue Value of this field.
	 * @param aConfig Configuration of this field.
	 * @return A pointer to the new CDocumentField.
	 */
	IMPORT_C static CDocumentField* NewLC( const TDesC& aName, const TDesC& aValue, TInt aConfig = KDefaultConfig );

	/**
	 * Creates a new CDocumentField and returns a pointer to it.
	 * @param aReadStream Stream from where CDocument is created.
	 * @return A pointer to the new CDocumentField.
	 */
	IMPORT_C static CDocumentField* NewL( RReadStream& aReadStream );	
	
	/**
	 * Creates a new CDocumentField and returns a pointer to it.
	 * @param aReadStream Stream from where CDocument is created.
	 * @return A pointer to the new CDocumentField.
	 */
	IMPORT_C static CDocumentField* NewLC( RReadStream& aReadStream );

	/**
	 * Destructor.
	 */
	IMPORT_C virtual ~CDocumentField();

public: // New functions

	/**
	 * Name of the field.
	 * @return Reference to the name of the field.
	 */
	IMPORT_C const TDesC& Name() const;

	/**
	 * Value of the field.
	 * @return Reference to the value of the field.
	 */
	IMPORT_C const TDesC& Value() const;

	/**
	 * Config. Configuration bitmask of this field.
	 * @return Configuration bitmask. 
	 */
	IMPORT_C TInt Config() const;

	/**
	 * Sets the boost. Documents, which field match the query, are ranked 
	 * higher, if the found field has high value, and lower if boost is small
	 * @param aBoost The new boost
	 */
	IMPORT_C void SetBoost( TReal32 aBoost );
	
	/**
	 * Gets the boost. Documents, which field match the query, are ranked 
	 * higher, if the found field has high value, and lower if boost is small
	 * @return Field's boost value
	 */
	IMPORT_C TReal32 Boost() const; 
	
	/**
	 * Number of bytes occupied by the data represented by this field.
	 * @return The size of the data represented by this field.
	 */
	IMPORT_C TInt Size() const;

	/**
	 * ExternalizeL. Externalize this field to a write stream.
	 * @param aWriteStream Write stream for externalization.
	 */
	IMPORT_C void ExternalizeL(RWriteStream& aWriteStream) const;

	/**
	 * InternalizeL. Internalize this field from a read stream.
	 * aReadStream Read stream for externalization.
	 */
	IMPORT_C void InternalizeL(RReadStream& aReadStream);

private: // Constructors

	/**
	 * Default contructor
	 */
	CDocumentField();

	/**
	 * Symbian second phase constructor.
	 */
	void ConstructL(const TDesC& aName, const TDesC& aValue, TInt aConfig = KDefaultConfig);
	
	/**
	 * Symbian second phase constructor.
	 */
	void ConstructL(RReadStream& aReadStream);	

private: // Data

	/**
	 * The name of the field.
	 */
	HBufC* iName;
	
	/**
	 * The value of the field. 
	 */
	HBufC* iValue;

	/**
	 * Configuration bitmask of this field. See enumerations TStored and TIndexed
	 */
	TInt iConfig;
	
	/**
	 * Documents, which given field contains a match, is ranked higher 
	 * if the field has high boost value and lower, if the field has low boost value.
	 */
	TReal32 iBoost; 
	};

#endif /*CDOCUMENTFIELD_H_*/
