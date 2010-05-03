/*
* Copyright (c) 2009 Nokia Corporation and/or its subsidiary(-ies). 
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
* Description: STIF testclass declaration
*
*/

#ifndef CPIXSEARCHERTEST_H
#define CPIXSEARCHERTEST_H

//  INCLUDES
#include <StifLogger.h>
#include <TestScripterInternal.h>
#include <StifTestModule.h>
#include <TestclassAssert.h>

// CONSTANTS
//const ?type ?constant_var = ?constant;

// MACROS
//#define ?macro ?macro_def
#define TEST_CLASS_VERSION_MAJOR 0
#define TEST_CLASS_VERSION_MINOR 0
#define TEST_CLASS_VERSION_BUILD 0

// Logging path
_LIT( KcpixsearchertestLogPath, "\\logs\\testframework\\cpixsearchertest\\" ); 
// Log file
_LIT( KcpixsearchertestLogFile, "cpixsearchertest.txt" ); 
_LIT( KcpixsearchertestLogFileWithTitle, "cpixsearchertest_[%S].txt" );

// FUNCTION PROTOTYPES
//?type ?function_name(?arg_list);

// FORWARD DECLARATIONS
//class ?FORWARD_CLASSNAME;
class Ccpixsearchertest;

// DATA TYPES
//enum ?declaration
//typedef ?declaration
//extern ?data_type;

// CLASS DECLARATION

/**
*  Ccpixsearchertest test class for STIF Test Framework TestScripter.
*  ?other_description_lines
*
*  @lib ?library
*  @since ?Series60_version
*/
NONSHARABLE_CLASS(Ccpixsearchertest) : public CScriptBase
    {
    public:  // Constructors and destructor

        /**
        * Two-phased constructor.
        */
        static Ccpixsearchertest* NewL( CTestModuleIf& aTestModuleIf );

        /**
        * Destructor.
        */
        virtual ~Ccpixsearchertest();

    public: // New functions

        /**
        * ?member_description.
        * @since ?Series60_version
        * @param ?arg1 ?description
        * @return ?description
        */
        //?type ?member_function( ?type ?arg1 );

    public: // Functions from base classes

        /**
        * From CScriptBase Runs a script line.
        * @since ?Series60_version
        * @param aItem Script line containing method name and parameters
        * @return Symbian OS error code
        */
        virtual TInt RunMethodL( CStifItemParser& aItem );

    protected:  // New functions

        /**
        * ?member_description.
        * @since ?Series60_version
        * @param ?arg1 ?description
        * @return ?description
        */
        //?type ?member_function( ?type ?arg1 );

    protected:  // Functions from base classes

        /**
        * From ?base_class ?member_description
        */
        //?type ?member_function();

    private:

        /**
        * C++ default constructor.
        */
        Ccpixsearchertest( CTestModuleIf& aTestModuleIf );

        /**
        * By default Symbian 2nd phase constructor is private.
        */
        void ConstructL();

        // Prohibit copy constructor if not deriving from CBase.
        // ?classname( const ?classname& );
        // Prohibit assigment operator if not deriving from CBase.
        // ?classname& operator=( const ?classname& );

        /**
        * Frees all resources allocated from test methods.
        * @since ?Series60_version
        */
        void Delete();

        /**
        * Test methods are listed below. 
        */

        /**
        * Example test method.
        * @since ?Series60_version
        * @param aItem Script line containing parameters.
        * @return Symbian OS error code.
        */
        virtual TInt TestOpenInvalidIndexdbL( CStifItemParser& aItem );
        virtual TInt TestOpenValidIndexdbL( CStifItemParser& aItem );
        virtual TInt TestAddDocumentL( CStifItemParser& aItem );
        virtual TInt TestAddCancelL( CStifItemParser& aItem );
        virtual TInt TestAsyncAddDocumentL( CStifItemParser& aItem );
        virtual TInt TestUpdateDocumentL( CStifItemParser& aItem );
        virtual TInt TestAsyncUpdateL( CStifItemParser& aItem );
        virtual TInt TestDeleteDocumentL( CStifItemParser& aItem );
        virtual TInt TestAsyncDeleteL( CStifItemParser& aItem );
        virtual TInt TestResetL( CStifItemParser& aItem );
        virtual TInt TestFlushL( CStifItemParser& aItem );
        virtual TInt TestAsyncFlushL( CStifItemParser& aItem );
        virtual TInt TestAsyncResetL( CStifItemParser& aItem );
        virtual TInt TestHandleIndexingResultL( CStifItemParser& aItem );
        virtual TInt TestSetAnalyzerAsyncL( CStifItemParser& aItem );
        virtual TInt TestSearchSessionL( CStifItemParser& aItem );
        virtual TInt TestMultiThreadingL( CStifItemParser& aItem );
        virtual TInt TestNoBoostL( CStifItemParser& aItem );
        virtual TInt TestBoostL( CStifItemParser& aItem );
        virtual TInt TestFieldBoostL( CStifItemParser& aItem );
        virtual TInt TestStandardAnalyzerL( CStifItemParser& aItem );
        virtual TInt TestWhitespaceTokenizerL( CStifItemParser& aItem );
        virtual TInt TestRemoveSnowballAnalyzerL( CStifItemParser& aItem );
        virtual TInt TestKnownTermL( CStifItemParser& aItem );
        virtual TInt TestWildcardTermL( CStifItemParser& aItem );
        virtual TInt TestDefineVolumeWorksNonEmptyPathL( CStifItemParser& aItem );
        virtual TInt TestDefineVolumeWorksWithEmptyPathL( CStifItemParser& aItem );
        virtual TInt TestDefineVolumeErrorScenariosL( CStifItemParser& aItem );
        virtual TInt TestSearchCancellationL( CStifItemParser& aItem );
        virtual TInt TestSearchAsynchronousL( CStifItemParser& aItem );
        virtual TInt TestSearchLeavesIfNotCancelledL( CStifItemParser& aItem );
        virtual TInt TestOpenIndexDbL( CStifItemParser& aItem );
        virtual TInt TestDeleteIndexDbWhileSearchingL( CStifItemParser& aItem );
        virtual TInt TestGetDocumentAsyncL( CStifItemParser& aItem );
        virtual TInt TestGetInvalidDocumentAsyncL( CStifItemParser& aItem );
        virtual TInt TestExerptLengthL( CStifItemParser& aItem );
        /**
         * Method used to log version of test class
         */
        void SendTestClassVersion();

        //ADD NEW METHOD DEC HERE
        //[TestMethods] - Do not remove

    public:     // Data
        // ?one_line_short_description_of_data
        //?data_declaration;

    protected:  // Data
        // ?one_line_short_description_of_data
        //?data_declaration;

    private:    // Data
        
        // ?one_line_short_description_of_data
        //?data_declaration;

        // Reserved pointer for future extension
        //TAny* iReserved;

    public:     // Friend classes
        //?friend_class_declaration;
    protected:  // Friend classes
        //?friend_class_declaration;
    private:    // Friend classes
        //?friend_class_declaration;

    };

#endif      // CPIXSEARCHERTEST_H

// End of File
