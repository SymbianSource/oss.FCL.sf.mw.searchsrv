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


#ifndef CBLACKLISTMGR_H
#define CBLACKLISTMGR_H

#include <e32base.h>

class CBlacklistDb;

class CBlacklistMgr : public CBase
    {
public :
    /*
    * Construction
    * @return instance of Blacklist manager
    */
    static CBlacklistMgr* NewL();
    
    /*
    * Construction
    * @return instance of Blacklist manager
    */
    static CBlacklistMgr* NewLC();
    
    //Destructor
    ~CBlacklistMgr();
    
private :
    /*
    * Default constructor
    */
    CBlacklistMgr();
    
    /*
     * Second phase constructor
     */    
    void ConstructL();
    
public :
    /*
     * @description Adds the given details of a plugin to the Blacklist DB.
     * If plugin uid is already present in the database and the version number 
     * passed is higher then the version stored in database, then this method will
     * update the same database record with higher version number.
     * Assuming always a higher version is passed if there is any change in the 
     * version number of plugin.
     * @param aPluginUid Uid of the plugin
     * @param aVersion version of the plugin
	 * @return sysmbian error code
     * Leaves in case of errors.
     */    
    TInt  AddL( TUid aPluginUid , TInt aVersion );
    
    /*
    * @description Removes the given details of a plugin from the Blacklist DB.
    * @param aPluginUid Uid of the plugin
    * Leaves in case of errors.
    */ 
    void  Remove( TUid aPluginUid );
    
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
    TBool FindL(TUid aPluginUid , TInt aVersion);
    
private:    
    /*
     * Interface to Blacklist database.Owned
     */
    CBlacklistDb* iBlacklistDb;
    };

#endif /* CBLACKLISTMGR_H */

