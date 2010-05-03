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

#ifndef CONTENTINFOMGR_H_
#define CONTENTINFOMGR_H_

#include <e32base.h>

class CContentInfoDb;
class CContentInfo;

class CContentInfoMgr : public CBase
    {
public :
    /*
    * Construction
    * @return instance of Contentinfo manager
    */
    static CContentInfoMgr* NewL();
    
    /*
    * Construction
    * @return instance of content info manager
    */
    static CContentInfoMgr* NewLC();
    
    //Destructor
    ~CContentInfoMgr();
    
private :
    /*
    * Default constructor
    */
    CContentInfoMgr();
    
    /*
     * Second phase constructor
     */    
    void ConstructL();
    
public :
    /*
     * @description Adds the given details of a plugin to the contentinfo DB.
     * status will be set as 1(Not Blacklisted) 
     * @param aContentInfo name,indexing status,blacklist status of content
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
    * @description Finds whether the given plugin exists in the 
    * database or not.
    * @param aContentName name of the plugin
    * @retutn ETrue if exists else returns EFalse
    * Leaves in case of errors.
    */ 
    TBool  FindL( const TDesC& aContentName );
      
   /*
   * @description Removes the given details of a plugin from the contentInfo DB.
   * @param aContentName name of the plugin
   * Leaves in case of errors.
   */ 
   void  RemoveL( const TDesC& aContentName );
   
  /*
   * @description Removes all the rows in the database   
   * Leaves in case of errors.
   */ 
   void  ResetL( );
   
   /*
    * @description Returns the count of the content in content info DB
    * @return content count   
    * Leaves in case of errors.
    */ 
    TInt  GetContentCountL( );
    
private:    
    /*
     * Interface to contentinfo database.Owned
     */
    CContentInfoDb* iContentInfoDb;
    };

#endif /* CONTENTINFOMGR_H_ */
