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
* Description: STIF testclass declaration
*
*/

#ifndef CPIXMWTESTER_H
#define CPIXMWTESTER_H

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
_LIT( KCPixMWTesterLogPath, "\\logs\\testframework\\CPixMWTester\\" ); 
// Log file
_LIT( KCPixMWTesterLogFile, "CPixMWTester.txt" ); 
_LIT( KCPixMWTesterLogFileWithTitle, "CPixMWTester_[%S].txt" );

// FUNCTION PROTOTYPES
//?type ?function_name(?arg_list);

// FORWARD DECLARATIONS
//class ?FORWARD_CLASSNAME;
class CCPixMWTester;

// DATA TYPES
//enum ?declaration
//typedef ?declaration
//extern ?data_type;

// CLASS DECLARATION

/**
*  CCPixMWTester test class for STIF Test Framework TestScripter.
*  ?other_description_lines
*
*  @lib ?library
*  @since ?Series60_version
*/
NONSHARABLE_CLASS(CCPixMWTester) : public CScriptBase
    {
    public:  // Constructors and destructor

        /**
        * Two-phased constructor.
        */
        static CCPixMWTester* NewL( CTestModuleIf& aTestModuleIf );

        /**
        * Destructor.
        */
        virtual ~CCPixMWTester();

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
        CCPixMWTester( CTestModuleIf& aTestModuleIf );

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
        * TestBlacklistPluginL test method.
        * @des Tests the Blacklist manager functionality.
        * This test adds details of a plugin and checks if the details exists in database
        * @param aItem Script line containing parameters.
        * @return Symbian OS error code.
        */        
        virtual TInt TestBlacklistPluginL( CStifItemParser& aItem );
        /**
        * TestBlacklistPluginVersionL test method.
        * @des Tests the Blacklist manager functionality.
        * This test checks the functionality of blacklist manager if there is a new version
        * of plugin is available
        * @param aItem Script line containing parameters.
        * @return Symbian OS error code.
        */ 
        virtual TInt TestBlacklistPluginVersionL( CStifItemParser& aItem );
        /**
        * TestWatchdogL test method.
        * @des Tests the Watchdog functionality.
        * This test checks the functionality of watchdog.
        * @param aItem Script line containing parameters.
        * @return Symbian OS error code.
        */ 
        virtual TInt TestWatchdogL( CStifItemParser& aItem );
        
        virtual TInt TestDeleteContentInfoDBL( CStifItemParser& aItem );
        
        virtual TInt TestAddContentL( CStifItemParser& aItem );
        
        virtual TInt TestRemoveContentL( CStifItemParser& aItem );
        
        virtual TInt TestResetContentL( CStifItemParser& aItem );
        
        virtual TInt TestUpdateBLStatusL( CStifItemParser& aItem );
        
        virtual TInt TestUpdateINStatusL( CStifItemParser& aItem );
        
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

#endif      // CPIXMWTESTER_H

// End of File
