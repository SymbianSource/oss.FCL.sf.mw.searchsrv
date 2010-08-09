/*
 * CCPixAbstractSearcher.cpp
 *
 *  Created on: Oct 8, 2009
 *      Author: admin
 */
#include "CCpixAbstractSearcher.h"

#include "Common.h"
#include "SearchServerHelper.h"
#include "CSearchDocument.h"

RArray<TInt> CCPixAbstractSearcher::docSizeArray;

CCPixAbstractSearcher::~CCPixAbstractSearcher() {
docSizeArray.Close();
} 


namespace {

/**
 * cpix_DocFieldEnum destroyer for TCleanupItem
 * @param aCpixDocFieldEnum CPix document
 */
void CpixDocFieldEnumDestroyer(TAny* aCpixDocFieldEnum)
	{
	cpix_DocFieldEnum_destroy( static_cast<cpix_DocFieldEnum*>( aCpixDocFieldEnum ) );
	}

} // namespace

CSearchDocument* CCPixAbstractSearcher::ConvertDocumentL( cpix_Document* aDocument )
	{
	// Read first the system fields that are passed as constructor parameters 
	// 
	const wchar_t* documentId = cpix_Document_getFieldValue( aDocument, LCPIX_DOCUID_FIELD);
	SearchServerHelper::CheckCpixErrorL(aDocument, KErrDatabaseQueryFailed);
	
	TPtrC documentIdPtr(KNullDesC);
	if (documentId)
		documentIdPtr.Set(reinterpret_cast<const TUint16*>(documentId));

	const wchar_t* documentAppClass = cpix_Document_getFieldValue(aDocument, LCPIX_APPCLASS_FIELD);
	SearchServerHelper::CheckCpixErrorL(aDocument, KErrDatabaseQueryFailed);
	
	TPtrC documentAppClassPtr(KNullDesC);
	if (documentAppClass)
		documentAppClassPtr.Set(reinterpret_cast<const TUint16*>(documentAppClass));

	const wchar_t* documentExcerpt = cpix_Document_getFieldValue(aDocument, LCPIX_EXCERPT_FIELD);
	SearchServerHelper::CheckCpixErrorL(aDocument, KErrDatabaseQueryFailed);
	
	TPtrC documentExcerptPtr(KNullDesC);
	if (documentExcerpt)
		documentExcerptPtr.Set(reinterpret_cast<const TUint16*>(documentExcerpt));

	// Setup the document
	// 
	
	CSearchDocument* document = CSearchDocument::NewLC(documentIdPtr, documentAppClassPtr, documentExcerptPtr);
	
	// Enumerate the field of cpix_Document and add each of them 
	// into the CSearchDocument object. 
	// 
	
	cpix_DocFieldEnum* docFieldEnum = cpix_Document_fields(aDocument);
	SearchServerHelper::CheckCpixErrorL(aDocument, KErrDocumentAccessFailed);

	CleanupStack::PushL( TCleanupItem(CpixDocFieldEnumDestroyer, docFieldEnum) ); 
	
	cpix_Field field;
	while (cpix_DocFieldEnum_hasMore(docFieldEnum))
		{
		cpix_DocFieldEnum_next(docFieldEnum, &field);
		SearchServerHelper::CheckCpixErrorL(docFieldEnum, KErrDatabaseQueryFailed);
		
		const wchar_t* name = cpix_Field_name(&field);
		SearchServerHelper::CheckCpixErrorL(&field, KErrDatabaseQueryFailed);

		TPtrC namePtr( reinterpret_cast<const TUint16*>( name ) );
		if (    namePtr == TPtrC( (TUint16*)LCPIX_DOCUID_FIELD )
			 || namePtr == TPtrC( (TUint16*)LCPIX_APPCLASS_FIELD )
			 || namePtr == TPtrC( (TUint16*)LCPIX_EXCERPT_FIELD ) )
			{
			continue;  // These fields have already been added
			}

		const wchar_t* value = cpix_Field_stringValue(&field);
		SearchServerHelper::CheckCpixErrorL(&field, KErrDatabaseQueryFailed);
		
		TPtrC stringvalue( reinterpret_cast<const TUint16*>( value ) );
		
		// NOTE: Also system fields will be iterated. Because  
		//       the field name is not checked, all _appclass,
		//       _excerpt etc. fields  will be overwritten. 
		document->AddFieldL(namePtr, stringvalue);
		}
	
	CleanupStack::PopAndDestroy(docFieldEnum);	
	CleanupStack::Pop(document);
	delete aDocument;
	return document;
	}
	
RPointerArray<CSearchDocument> CCPixAbstractSearcher::ConvertBatchDocumentL( cpix_Document** aDocument, TInt aCount )
	{
	// Read first the system fields that are passed as constructor parameters 
	//
    RPointerArray<CSearchDocument> docArray;
    docSizeArray.Reset();    
    //docArray.Reset(); // delete each element
    
    for( TInt i= 0;i< aCount;i++ )
        {
        cpix_Document* nextDocument = aDocument[i];
        
        if( nextDocument->ptr_ )
            {
            const wchar_t* documentId = cpix_Document_getFieldValue( nextDocument, LCPIX_DOCUID_FIELD);
            SearchServerHelper::CheckCpixErrorL(nextDocument, KErrDatabaseQueryFailed);
            
            TPtrC documentIdPtr(KNullDesC);
            if (documentId)
                documentIdPtr.Set(reinterpret_cast<const TUint16*>(documentId));
        
            const wchar_t* documentAppClass = cpix_Document_getFieldValue(nextDocument, LCPIX_APPCLASS_FIELD);
            SearchServerHelper::CheckCpixErrorL(nextDocument, KErrDatabaseQueryFailed);
            
            TPtrC documentAppClassPtr(KNullDesC);
            if (documentAppClass)
                documentAppClassPtr.Set(reinterpret_cast<const TUint16*>(documentAppClass));
        
            const wchar_t* documentExcerpt = cpix_Document_getFieldValue(nextDocument, LCPIX_EXCERPT_FIELD);
            SearchServerHelper::CheckCpixErrorL(nextDocument, KErrDatabaseQueryFailed);
            
            TPtrC documentExcerptPtr(KNullDesC);
            if (documentExcerpt)
                documentExcerptPtr.Set(reinterpret_cast<const TUint16*>(documentExcerpt));
        
            // Setup the document
            // 
            
            CSearchDocument* document = CSearchDocument::NewLC(documentIdPtr, documentAppClassPtr, documentExcerptPtr);
            
            // Enumerate the field of cpix_Document and add each of them 
            // into the CSearchDocument object. 
            // 
            
            cpix_DocFieldEnum* docFieldEnum = cpix_Document_fields(nextDocument);
            SearchServerHelper::CheckCpixErrorL(nextDocument, KErrDocumentAccessFailed);
        
            CleanupStack::PushL( TCleanupItem(CpixDocFieldEnumDestroyer, docFieldEnum) ); 
            
            cpix_Field field;
            while (cpix_DocFieldEnum_hasMore(docFieldEnum))
                {
                cpix_DocFieldEnum_next(docFieldEnum, &field);
                SearchServerHelper::CheckCpixErrorL(docFieldEnum, KErrDatabaseQueryFailed);
                
                const wchar_t* name = cpix_Field_name(&field);
                SearchServerHelper::CheckCpixErrorL(&field, KErrDatabaseQueryFailed);
        
                TPtrC namePtr( reinterpret_cast<const TUint16*>( name ) );
                if (    namePtr == TPtrC( (TUint16*)LCPIX_DOCUID_FIELD )
                     || namePtr == TPtrC( (TUint16*)LCPIX_APPCLASS_FIELD )
                     || namePtr == TPtrC( (TUint16*)LCPIX_EXCERPT_FIELD ) )
                    {
                    continue;  // These fields have already been added
                    }
        
                const wchar_t* value = cpix_Field_stringValue(&field);
                SearchServerHelper::CheckCpixErrorL(&field, KErrDatabaseQueryFailed);
                
                TPtrC stringvalue( reinterpret_cast<const TUint16*>( value ) );
                
                // NOTE: Also system fields will be iterated. Because  
                //       the field name is not checked, all _appclass,
                //       _excerpt etc. fields  will be overwritten. 
                document->AddFieldL(namePtr, stringvalue);
                }	
            CleanupStack::PopAndDestroy(docFieldEnum);	
            CleanupStack::Pop(document);
            docSizeArray.AppendL( document->Size());
            docArray.AppendL( document );
            delete nextDocument;
            nextDocument = NULL;                      
            }
        else break;
        }
	delete aDocument;
	aDocument = NULL;
	return docArray;
	}
