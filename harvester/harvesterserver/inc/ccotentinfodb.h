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
* Description:  This component is to store the details of harvester plugins
*
*/

#ifndef CCOTENTINFODB_H_
#define CCOTENTINFODB_H_

#include <e32base.h>
#include <f32file.h>
#include <sqldb.h>

class CContentInfo;

class CContentInfoDb : public CBase
    {
public :
    /*
    * Construction
    * @return instance of CContentInfo database
    */
    static CContentInfoDb* NewL();
    
    /*
    * Construction
    * @return instance of CContentInfo database
    */
    static CContentInfoDb* NewLC();
    //Destructor
    virtual ~CContentInfoDb();
    
private :
    /*
    * Default constructor
    */
    CContentInfoDb();
    
    /*
     * Second phase constructor
     */
    void ConstructL();
    
public :
    /*
     * @description Adds the given details of a plugin to the contentinfo DB.
     * @param aContentInfo name,Indexing status,blacklist status of the plugin
     * @return sysmbian error code
     * Leaves in case of errors.
     */    
    TInt  AddL( CContentInfo* aContentInfo );
    
    /*
     * @description Updates the blacklsit status of the plugin.
     * @param aContentName name of the plugin
     * @param aBlacklistStatus blacklist status of plugin
     * @return sysmbian error code
     * Leaves in case of errors.
     */    
    TInt  UpdateBlacklistStatusL( const TDesC& aContentName , TInt aBlacklistStatus );
    
    /*
     * @description Updates the enable status of the plugin.
     * @param aContentName name of the plugin
     * @param aIndexStatus Indexing status of plugin
     * @return sysmbian error code
     * Leaves in case of errors.
     */    
    TInt  UpdatePluginIndexStatusL( const TDesC& aContentName , TInt aIndexStatus );
    
    /*
    * @description Removes the given details of a plugin from the contentInfo DB.
    * @param aContentName name of the plugin
    * Leaves in case of errors.
    */ 
    void  RemoveL( const TDesC& aContentName );
    
   /*
    * @description Removes all the rows in the database.
    * Leaves in case of errors.    
    */ 
    void  ResetDatabaseL( );
    
    /*
     * @description Returns the count of the content in content info DB
     * @return content count   
     * Leaves in case of errors.
     */ 
    TInt  GetContentCountL( );    
    
    /*
     * @description Checks if the plugin details is present in the database.
     * @param aContentName name of the plugin
     * Leaves in case of errors.
     */
     TBool FindL(const TDesC& aContentName);
   
    
private :
   /*
    * @description Creates the ContentInfo sql database.
    * Leaves in case of errors.
    */
    void CreateDBL();   

private:    
    /*
     * Database to maintain the details of ContentInfo plugin.Owned
     */
    RSqlDatabase  iDatabase;
    /**
     * Flag for, whether database has been opened
     */
    TBool iOpened;
    };

#endif /* CCOTENTINFODB_H_ */
