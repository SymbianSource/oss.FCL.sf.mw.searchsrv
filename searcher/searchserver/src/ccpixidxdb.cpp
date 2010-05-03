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

#include "CCPixIdxDb.h"
#include "CSearchDocument.h"
#include "common.h"
#include "SearchServerLogger.h"
#include "CCPixAsyncronizer.h"

#include <f32file.h>

#include "SearchServerConfiguration.h"
#include "SearchServerHelper.h"
#include "OstTraceDefinitions.h"
#ifdef OST_TRACE_COMPILER_IN_USE
#include "ccpixidxdbTraces.h"
#endif


namespace {

void DumpDocument(const CSearchDocument& aDocument)
	{
#ifdef CPIX_LOGGING_ENABLED
	OstTraceExt1( TRACE_NORMAL, CPIXIDXDB_DUMPDOCUMENT, "::DumpDocument;Start_docuid=%S", (aDocument.Id()) );
	CPIXLOGSTRING2("DumpDocument START _docuid=%S", &(aDocument.Id()));

	_LIT(KDocUid, CPIX_DOCUID_FIELD);
	const TInt fieldCount = aDocument.FieldCount();
	for (TInt i = 0; i < fieldCount; i++)
		{
		const CDocumentField& field = aDocument.Field(i);
		
		if (field.Name() == KDocUid())
			{
			continue; // lets not print this information out twice.
			}
		OstTraceExt2( TRACE_NORMAL, DUP1_CPIXIDXDB_DUMPDOCUMENT, "::DumpDocument;Fieldname=%S;Value=%S", (field.Name()), (field.Value()) );
		CPIXLOGSTRING3("DumpDocument FieldName=%S,Value=%S", &(field.Name()), &(field.Value()));
		}
	OstTraceExt1( TRACE_NORMAL, DUP2_CPIXIDXDB_DUMPDOCUMENT, "::DumpDocument;End_docuid=%S", (aDocument.Id()) );
	CPIXLOGSTRING2("DumpDocument END _docuid=%S", &(aDocument.Id()));
#endif // CPIX_LOGGING_ENABLED
	}

/**
 * InitParams destroyer for TCleanupItem
 * @param aCpixInitParams CPix related init parameter
 */
void CpixInitParamsDestroyer(TAny* aCpixInitParams)
    {
    cpix_InitParams_destroy( static_cast<cpix_InitParams*>( aCpixInitParams ) );
    }

/**
 * cpix_Document destroyer for TCleanupItem
 * @param aCpixDocument CPix document
 */
void CpixDocumentDestroyer(TAny* aCpixDocument)
    {
    cpix_Document_destroy( static_cast<cpix_Document*>( aCpixDocument ) );
    }

/**
 * cpix_DocFieldEnum destroyer for TCleanupItem
 * @param aCpixDocFieldEnum CPix document
 */
void CpixDocFieldEnumDestroyer(TAny* aCpixDocFieldEnum)
	{
	cpix_DocFieldEnum_destroy( static_cast<cpix_DocFieldEnum*>( aCpixDocFieldEnum ) );
	}

} // namespace

CCPixIdxDb* CCPixIdxDb::NewL()
	{
	CCPixIdxDb* self = CCPixIdxDb::NewLC();
	CleanupStack::Pop(self);
	return self;
	}

CCPixIdxDb* CCPixIdxDb::NewLC()
	{
	CCPixIdxDb* self = new (ELeave) CCPixIdxDb();
	CleanupStack::PushL(self);
	self->ConstructL();
	return self;
	}

CCPixIdxDb::CCPixIdxDb()
    : iIsPending(EFalse)
    {
    }

CCPixIdxDb::~CCPixIdxDb()
	{
	delete iAsyncronizer;
	delete iBaseAppClass;
	cpix_IdxDb_releaseDb(iIdxDb);
	cpix_Analyzer_destroy(iAnalyzer);
	}

void CCPixIdxDb::ConstructL()
	{
	iAsyncronizer = CCPixAsyncronizer::NewL();
	}

// Cancel any incomplete asyncronous operation
void CCPixIdxDb::CancelAll(const RMessage2& aMessage)
	{
    // Do nothing here. Wait until asynchronous functions
    // completes and complete request after that.
    iAsyncronizer->CancelWhenDone(aMessage);
	}

void CCPixIdxDb::CompletionCallback(void *aCookie, cpix_JobId aJobId)
	{
	CCPixIdxDb* object = (CCPixIdxDb*)aCookie;

	// Sanity check
	if (object == NULL || 
		object->iPendingJobId != aJobId)
		return;
	
	// Call the asyncronizers completion code
	CCPixAsyncronizer* asyncronizer = object->iAsyncronizer;
	asyncronizer->CompletionCallback();
	}

void CCPixIdxDb::InitializeL()
	{
#ifdef CPIX_LOGGING_ENABLED
	_LIT(KCPixLogDirectory, "c:\\logs\\CPix\\OpenC\\");
	const char* CPIX_LOG_FILE = "c:\\logs\\CPix\\OpenC\\libcpix";
	const TInt KLogSizeLimit = 400 * 1024;
	const TInt KLogSizeCheckRecurrency = 10; 
#endif // CPIX_LOGGING_ENABLED

	RFs fs;
	User::LeaveIfError(fs.Connect());
	CleanupClosePushL(fs);
#ifdef CPIX_LOGGING_ENABLED
	TInt err = fs.MkDirAll(KCPixLogDirectory);

	if ( err != KErrNone && err != KErrAlreadyExists )
		{
		User::Leave(err);
		}
#endif // CPIX_LOGGING_ENABLED	

	TBuf8<30> registryPath;
	
	fs.CreatePrivatePath(EDriveC);

	TFileName pathWithoutDrive;
	fs.PrivatePath(pathWithoutDrive);

	registryPath.Append(_L("C:"));
	registryPath.Append(pathWithoutDrive);

	cpix_Result result;

        cpix_InitParams
            * initParams = cpix_InitParams_create(&result);
                
        CleanupStack::PushL( TCleanupItem (CpixInitParamsDestroyer, initParams) );
        SearchServerHelper::CheckCpixErrorL(&result,
                                            KErrCPixInitializationFailed);
        
        cpix_InitParams_setCpixDir(initParams,
                                   reinterpret_cast<const char*>( registryPath.PtrZ() ));
        SearchServerHelper::CheckCpixErrorL(initParams,
                                            KErrCPixInitializationFailed);

        cpix_InitParams_setMaxIdleSec(initParams,
                                      IDXDB_MAXIDLE_SEC);
        SearchServerHelper::CheckCpixErrorL(initParams,
                                            KErrCPixInitializationFailed);

        cpix_InitParams_setQryThreadPriorityDelta(initParams,
                                                  QRY_THREAD_PRIORITY_DELTA);
        SearchServerHelper::CheckCpixErrorL(initParams,
                                            KErrCPixInitializationFailed);
        
        cpix_InitParams_setCluceneLockDir(initParams,
                                       "C:\\Private\\2001f6f7\\clucenelockdir\\");
        SearchServerHelper::CheckCpixErrorL(initParams,
                                            KErrCPixInitializationFailed);
        

#ifdef CPIX_LOGGING_ENABLED
        cpix_InitParams_setLogFileBase(initParams,
                                       CPIX_LOG_FILE);
        SearchServerHelper::CheckCpixErrorL(initParams,
                                            KErrCPixInitializationFailed);

        cpix_InitParams_setLogSizeLimit(initParams,
                                        KLogSizeLimit);
        SearchServerHelper::CheckCpixErrorL(initParams,
                                            KErrCPixInitializationFailed);

        cpix_InitParams_setLogSizeCheckRecurrency(initParams,
                                                  KLogSizeCheckRecurrency);
        SearchServerHelper::CheckCpixErrorL(initParams,
                                            KErrCPixInitializationFailed);
#endif

        cpix_init(&result,
                  initParams);
    
        SearchServerHelper::CheckCpixErrorL(&result,
                                            KErrCPixInitializationFailed);

        CleanupStack::PopAndDestroy(initParams);

#ifdef CPIX_LOGGING_ENABLED
#ifdef CPIX_OPENC_LOGDBG
    // Enable OpenC logging
    cpix_setLogLevel(CPIX_LL_DEBUG);
#endif
#endif // CPIX_LOGGING_ENABLED


	CleanupStack::PopAndDestroy(); //fs
	}

void CCPixIdxDb::Shutdown()
	{
	cpix_shutdown();
	}

void CCPixIdxDb::OpenDatabaseL(const TDesC& aBaseAppClass)
	{
	// Allocate base app class (space for zero terminated)
	cpix_IdxDb_releaseDb( iIdxDb );
	iIdxDb = NULL; 
	
	delete iBaseAppClass;
	iBaseAppClass = NULL;	
	
	iBaseAppClass = HBufC8::NewL(aBaseAppClass.Size() + 1);
	TPtr8 baseAppClassPtr = iBaseAppClass->Des();
	baseAppClassPtr.Copy(aBaseAppClass);
	    
    // Try to open database
    cpix_Result result;
    iIdxDb = cpix_IdxDb_openDb(
    		       &result,
				   reinterpret_cast<const char*>( baseAppClassPtr.PtrZ() ),
				   cpix_IDX_OPEN);
    SearchServerHelper::CheckCpixErrorL(&result, KErrCannotOpenDatabase);
     
	// Set default analyzer
	SetAnalyzerL(TPtrC16((TUint16*)CPIX_ANALYZER_DEFAULT)); 
	}

TBool CCPixIdxDb::IsOpen()
	{
	return (iIdxDb != NULL);
	}

TPtrC8 CCPixIdxDb::BaseAppClass()
	{
	TPtrC8 ptr;
	ptr.Set( *iBaseAppClass );
	return ptr;
	}

void CCPixIdxDb::AddL(const CSearchDocument& aDocument, MCPixAsyncronizerObserver* aObserver, const RMessage2& aMessage)
	{	
    if (iIsPending)
        User::Leave(KErrInUse);

    cpix_Document* document = NULL;
	document = ConvertToCpixDocumentLC(aDocument);

	OstTrace0( TRACE_NORMAL, CCPIXIDXDB_ADDL, "CCPixIdxDb::AddL" );
	CPIXLOGSTRING("CCPixIdxDb::AddL");
	DumpDocument(aDocument);
	iPendingJobId = cpix_IdxDb_asyncAdd(iIdxDb, document, iAnalyzer, (void*)this, &CompletionCallback);
	CleanupStack::PopAndDestroy(document);
	SearchServerHelper::CheckCpixErrorL(iIdxDb, KErrCannotAddDocument);
    iIsPending = ETrue;
	
	iAsyncronizer->Start(ECPixTaskTypeAdd, aObserver, aMessage);
	}
	
void CCPixIdxDb::AddCompleteL()
	{
    iIsPending = EFalse;
	cpix_IdxDb_asyncAddResults(iIdxDb, iPendingJobId);
	SearchServerHelper::CheckCpixErrorL(iIdxDb, KErrCannotAddDocument);
	}

void CCPixIdxDb::UpdateL(const CSearchDocument& aDocument, MCPixAsyncronizerObserver* aObserver, const RMessage2& aMessage)
	{
    if (iIsPending)
        User::Leave(KErrInUse);

	cpix_Document* document = NULL;	
	document = ConvertToCpixDocumentLC(aDocument);

	OstTrace0( TRACE_NORMAL, CCPIXIDXDB_UPDATEL, "CCPixIdxDb::UpdateL" );
	CPIXLOGSTRING("CCPixIdxDb::UpdateL");
	DumpDocument(aDocument);
    iPendingJobId = cpix_IdxDb_asyncUpdate(iIdxDb, document, iAnalyzer, (void*)this, &CompletionCallback);
	CleanupStack::PopAndDestroy(document);
	SearchServerHelper::CheckCpixErrorL(iIdxDb,KErrCannotUpdateDocument);
    iIsPending = ETrue;

    iAsyncronizer->Start(ECPixTaskTypeUpdate, aObserver, aMessage);
	}

void CCPixIdxDb::UpdateCompleteL()
	{
    iIsPending = EFalse;
	cpix_IdxDb_asyncUpdateResults(iIdxDb, iPendingJobId);
	SearchServerHelper::CheckCpixErrorL(iIdxDb, KErrCannotUpdateDocument);
	}

void CCPixIdxDb::DeleteDocumentsL(const TDesC& aDocUid, MCPixAsyncronizerObserver* aObserver, const RMessage2& aMessage)
	{
    if (iIsPending)
        User::Leave(KErrInUse);
	
	OstTraceExt1( TRACE_NORMAL, CCPIXIDXDB_DELETEDOCUMENTSL, "CCPixIdxDb::DeleteDocumentsL;aDocUid=%S", aDocUid );
	CPIXLOGSTRING2("CCPixIdxDb::DeleteDocumentsL aDocUid=%S", &aDocUid);

	HBufC* docUid = HBufC::NewLC(aDocUid.Length() + 1);
	TPtr docUidPtr = docUid->Des();
	docUidPtr.Copy(aDocUid);	
	
	const wchar_t* cDocumentId = reinterpret_cast<const wchar_t*>(docUidPtr.PtrZ());

    iIsPending = ETrue;
    iPendingJobId = cpix_IdxDb_asyncDeleteDocuments(iIdxDb, cDocumentId, (void*)this, &CompletionCallback);
    SearchServerHelper::CheckCpixErrorL(iIdxDb, KErrCannotDeleteDocument);
	CleanupStack::PopAndDestroy(docUid);	 
	
    iAsyncronizer->Start(ECPixTaskTypeDelete, aObserver, aMessage);
	}

void CCPixIdxDb::DeleteDocumentsCompleteL()
	{
    iIsPending = EFalse;
    cpix_IdxDb_asyncDeleteDocumentsResults(iIdxDb, iPendingJobId);
	SearchServerHelper::CheckCpixErrorL(iIdxDb, KErrCannotDeleteDocument);
	}

void CCPixIdxDb::FlushCompleteL()
	{
    iIsPending = EFalse;
    cpix_IdxDb_asyncFlushResults(iIdxDb, iPendingJobId);
	SearchServerHelper::CheckCpixErrorL(iIdxDb, KErrDatabaseFlushFailed);
	}

void CCPixIdxDb::SetAnalyzerL(const TDesC& aAnalyzer)
	{
	cpix_Analyzer_destroy( iAnalyzer ); 
	iAnalyzer = NULL; 
	
	// Create analyzer
	cpix_Result result;
	
	HBufC* analyzer = HBufC::NewLC(aAnalyzer.Length() + 1);
	TPtr analyzerPtr = analyzer->Des();
	analyzerPtr.Copy(aAnalyzer);
	const wchar_t* cAnalyzer = reinterpret_cast<const wchar_t*>(analyzerPtr.PtrZ());
	iAnalyzer = cpix_Analyzer_create(&result, cAnalyzer);
	
	CleanupStack::PopAndDestroy( analyzer ); 
	
	if ( !iAnalyzer || cpix_Failed( &result ) )
		{
		SearchServerHelper::LogErrorL(*result.err_);
		cpix_ClearError(&result);
		User::Leave(KErrCannotCreateAnalyzer);
		}
	}

void CCPixIdxDb::ResetL()
	{
    if (iIsPending)
        User::Leave(KErrInUse);
	
	cpix_IdxDb_releaseDb(iIdxDb);
	iIdxDb = NULL;

	cpix_Result result;
	iIdxDb = cpix_IdxDb_openDb( &result, 
			reinterpret_cast<const char*>( iBaseAppClass->Des().PtrZ() ), cpix_IDX_CREATE );

	// Leave if operation failed
	SearchServerHelper::CheckCpixErrorL(&result, KErrCannotOpenDatabase);
	}

void CCPixIdxDb::FlushL(MCPixAsyncronizerObserver* aObserver, const RMessage2& aMessage)
    {   
    if (iIsPending)
        User::Leave(KErrInUse);

    
    iPendingJobId = cpix_IdxDb_asyncFlush(iIdxDb, (void*)this, &CompletionCallback);
    SearchServerHelper::CheckCpixErrorL(iIdxDb, KErrDatabaseFlushFailed);
    iIsPending = ETrue;
    
    iAsyncronizer->Start(ECPixTaskTypeFlush, aObserver, aMessage);
    }

const char* CCPixIdxDb::GetFieldCStrLC(const CSearchDocument& aDocument, const TDesC& aFieldName )
	{
	const CDocumentField* field = aDocument.Field( aFieldName );
	if ( field )
		{
		HBufC8* fieldValue = HBufC8::NewLC( field->Value().Size() + 1);
		TPtr8 fieldValuePtr = fieldValue->Des();
		fieldValuePtr.Copy( field->Value() );	
		return reinterpret_cast<const char*>(fieldValuePtr.PtrZ());
		}
	else 
		{
		HBufC::NewLC( 1 ); // populate cleanup
		}
	return 0;
	}

const wchar_t* CCPixIdxDb::GetFieldWideCStrLC(const CSearchDocument& aDocument, const TDesC& aFieldName )
 	{
	const CDocumentField* field = aDocument.Field( aFieldName );
	if ( field )
		{
		HBufC* fieldValue = HBufC::NewLC( field->Value().Size() + 1);
		TPtr fieldValuePtr = fieldValue->Des();
		fieldValuePtr.Copy( field->Value() );	
		return reinterpret_cast<const wchar_t*>(fieldValuePtr.PtrZ());
		}
	else 
		{
		HBufC::NewLC( 1 ); // populate cleanup
		}
	return 0;
	}

cpix_Document* CCPixIdxDb::ConvertToCpixDocumentLC(const CSearchDocument& aDocument)
	{	
	cpix_Document* doc = NULL; // to return;  
	
	const wchar_t* cDocumentId = GetFieldWideCStrLC( aDocument, TPtrC((TUint16*)LCPIX_DOCUID_FIELD) ); 
	const char* cDocumentAppClass = GetFieldCStrLC( aDocument, TPtrC((TUint16*)LCPIX_APPCLASS_FIELD) ); 
	const wchar_t* cDocumentExcerpt = GetFieldWideCStrLC( aDocument, TPtrC((TUint16*)LCPIX_EXCERPT_FIELD) );
	const wchar_t* cDocumentMimeType = GetFieldWideCStrLC( aDocument, TPtrC((TUint16*)LCPIX_MIMETYPE_FIELD) );
	
	cpix_Result result;

	// ownership of cDocumentId is not transferred
	doc = cpix_Document_create(&result,
							   cDocumentId, 
							   cDocumentAppClass, 
							   cDocumentExcerpt, 
							   cDocumentMimeType);
	SearchServerHelper::CheckCpixErrorL(&result, KErrCannotCreateDocument);
	CleanupStack::PopAndDestroy(4);

	if (!doc)
		{
		SearchServerHelper::LogErrorL(*result.err_);
		cpix_ClearError(doc);
		User::Leave(KErrCannotCreateDocument);
		}
	// document created, push to cleanup stack.
	CleanupStack::PushL( TCleanupItem (CpixDocumentDestroyer, doc) );

	// Do the necessary magic, to copy the system field boosts 
	// from S60 document into CPix document
	cpix_Document_setBoost( doc, aDocument.Boost() );
	SearchServerHelper::CheckCpixErrorL(doc, KErrCannotCreateDocumentField);
	
	cpix_DocFieldEnum* fields = cpix_Document_fields( doc );
	// push to cleanup stack.
	CleanupStack::PushL( TCleanupItem (CpixDocFieldEnumDestroyer, fields) );

	if ( !fields || cpix_Failed( doc ) ) 
		{
		SearchServerHelper::LogErrorL(*doc->err_);
		cpix_ClearError(doc);
		User::Leave(KErrCannotCreateDocumentField);
		}
	
	while ( cpix_DocFieldEnum_hasMore( fields ) ) 
		{
		cpix_Field sysField;
		cpix_DocFieldEnum_next( fields, &sysField );
		
		if ( cpix_Failed( fields ) ) 
			{
			SearchServerHelper::LogErrorL(*fields->err_);
			cpix_ClearError(fields);
			User::Leave(KErrCannotCreateDocumentField);
			}
		
		const wchar_t* name = cpix_Field_name( &sysField ); 
		SearchServerHelper::CheckCpixErrorL(&sysField, KErrCannotCreateDocumentField);
		
		TPtrC16 nameDesc;
		nameDesc.Set( reinterpret_cast<const TUint16*>( name ), 
					  wcslen( name ) );
		
		const CDocumentField* field = aDocument.Field( nameDesc );
		if ( field )
			{
			cpix_Field_setBoost( &sysField, field->Boost() );
			SearchServerHelper::CheckCpixErrorL(&sysField, KErrCannotCreateDocumentField);
			}
		}
	
	CleanupStack::PopAndDestroy(fields);
	
	// Copy rest of the fiels
	// 
	
	for (TInt i = 0; i < aDocument.FieldCount(); i++)
		{
		const CDocumentField& srcField = aDocument.Field(i);
		
		if (    srcField.Name() == TPtrC((TUint16*)LCPIX_DOCUID_FIELD) 
		     || srcField.Name() == TPtrC((TUint16*)LCPIX_APPCLASS_FIELD)
		     || srcField.Name() == TPtrC((TUint16*)LCPIX_EXCERPT_FIELD)
		     || srcField.Name() == TPtrC((TUint16*)LCPIX_MIMETYPE_FIELD)) 
			{
		    continue;  // These fields have already been passed
			}
			
		HBufC* srcFieldName = HBufC::NewLC(srcField.Name().Length() + 1);
		TPtr srcFieldNamePtr = srcFieldName->Des();
		srcFieldNamePtr.Copy(srcField.Name());	

		HBufC* srcFieldValue = HBufC::NewLC(srcField.Value().Length() + 1);
		TPtr srcFieldValuePtr = srcFieldValue->Des();
		srcFieldValuePtr.Copy(srcField.Value());	

		const wchar_t* cStrName = reinterpret_cast<const wchar_t*>(srcFieldNamePtr.PtrZ());
		const wchar_t* cStrValue = reinterpret_cast<const wchar_t*>(srcFieldValuePtr.PtrZ());
	
		cpix_Field toField;
		// FIXME: Assumption that Config() is compatible with cpix config is ugly
		cpix_Field_initialize( &toField, cStrName, cStrValue,
							   srcField.Config() );

		CleanupStack::PopAndDestroy(srcFieldValue);
		CleanupStack::PopAndDestroy(srcFieldName);
        SearchServerHelper::CheckCpixErrorL(&toField, KErrCannotCreateDocumentField);
		
		cpix_Field_setBoost( &toField, srcField.Boost() ); 
		SearchServerHelper::CheckCpixErrorL(&toField, KErrCannotCreateDocumentField);
		
		cpix_Document_add(doc, ( &toField ));
		if (cpix_Failed( doc ))
			{
			cpix_Field_release(&toField);
			SearchServerHelper::LogErrorL(*doc->err_);
			cpix_ClearError(doc);
			User::Leave(KErrCannotCreateDocumentField);
			}
		}	

	return doc;	
	}

// End of File

