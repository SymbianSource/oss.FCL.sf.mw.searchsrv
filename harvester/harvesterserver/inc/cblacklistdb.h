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
* Description:  This component is to store the details of blacklisted plugins
*
*/


#ifndef CBLACKLISTDB_H
#define CBLACKLISTDB_H

#include <e32base.h>
#include <f32file.h>
#include <D32DBMS.H>

class CBlacklistDb : public CBase
    {
public :
    /*
    * Construction
    * @return instance of Blacklist database
    */
    static CBlacklistDb* NewL();
    
    /*
    * Construction
    * @return instance of Blacklist database
    */
    static CBlacklistDb* NewLC();
    //Destructor
    ~CBlacklistDb();
    
private :
    /*
    * Default constructor
    */
    CBlacklistDb();
    
    /*
     * Second phase constructor
     */
    void ConstructL();
    
public :
    /*
     * @description Adds the given details of a plugin to the Blacklist DB.
     * @param aPluginUid Uid of the plugin
     * @param aVersion version of the plugin
	 * @return sysmbian error code
     * Leaves in case of errors.
     */    
    TInt  AddL( TInt32 aPluginUid , TInt aVersion );
    
    /*
    * @description Removes the given details of a plugin from the Blacklist DB.
    * @param aPluginUid Uid of the plugin
    * Leaves in case of errors.
    */ 
    void  Remove( TInt32 aPluginUid );
	
	/*
    * @description Updates the details of a plugin in the Blacklist DB.
    * @param aPluginUid Uid of the plugin
	* @param aVersion version of the plugin
	* @return sysmbian error code
    * Leaves in case of errors.
    */
	TInt UpdateL( TInt32 aPluginUid , TInt aVersion );
    
    /*
     * @description Checks wether the plugin details are exists in the database or not.
     * If the plugin Uid is found in the database and the version number passed is higher 
     * than the version number stored in database, then this method will return EFalse.
     * Assuming always a higher version is passed if there is any change in the 
     * version number of plugin.
     * @param aPluginUid Uid of the plugin
     * @param aVersion version of the plugin
     * @return ETrue if details exists else returns EFalse
     * Leaves in case of errors.
     */	
    TBool FindWithVersionL(TInt32 aPluginUid , TInt aVersion);
	
	/*
    * @description Checks if the plugin uid is present in the database.
    * @param aPluginUid Uid of the plugin
    * Leaves in case of errors.
    */
	TBool FindL(TInt32 aPluginUid);
	
	/*
     * @description Adds the given uid of a plugin to the dontloadlist table.
     * @param aPluginUid Uid of the plugin
     * @return symbian error code
     * Leaves in case of errors.
     */  
	TInt AddtoDontloadListL( TInt32 aPluginUid );
    
	/*
     * @description remove the given uid of a plugin to the dontloadlist table.
     * @param aPluginUid Uid of the plugin
     * Leaves in case of errors.
     */  
    void RemoveFromDontloadListL( TInt32 aPluginUid );
    
    /*
     * @description Find the given uid of a plugin to the dontloadlist table.
     * @param aPluginUid Uid of the plugin
     * @return ETrue if exists else returns EFalse
     * Leaves in case of errors.
     */  
    TBool FindInDontloadListL( TInt32 aPluginUid );
        
private :
    /*
    * @description Creates the Blacklist database.
    * Leaves in case of errors.
    */
    void CreateDBL();
    
	/*
    * @description Creates Column set for database.
	* @return CDbColSet database column set
    * Leaves in case of errors.
    */
    CDbColSet* CreateBlacklistColumnSetLC();
    
    /*
    * @description Creates Column set for dontload table.
    * @return CDbColSet database column set
    * Leaves in case of errors.
    */
    CDbColSet* CreateDontloadColumnSetLC();
private:    
    /*
     * A handle to a file server session.Owned
     */
    RFs iFs;
    /*
     * Database to maintain the details of Blacklisted plugin.Owned
     */
    RDbNamedDatabase  iDatabase;
    /**
     * Flag for, whether database has been opened
     */
    TBool iOpened;
    };

#endif /* CBLACKLISTDB_H */

