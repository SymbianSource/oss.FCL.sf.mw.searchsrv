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
#ifndef CCONTENTINFO_H_
#define CCONTENTINFO_H_

#include <e32base.h>

class CContentInfo : public CBase
    {
public :
    /*
    * Construction
    * @return instance of CContentInfo
    */
    static CContentInfo* NewL();
    
    /*
    * Construction
    * @return instance of CContentInfo
    */
    static CContentInfo* NewLC();
    //Destructor
    ~CContentInfo();
    
private :
    /*
    * Default constructor
    */
    CContentInfo();  
    
public:
    /*
    * @description Returns the name of the content( This is the display
    * name given in the corresponding plugin RSS file).
    * @return the descriptor containing the name of the content and it
    * is the responsibility of the caller to free the allocated memory
    * Leaves in case of errors.
    */
    HBufC* GetNameL();
    
    /*
     * @description Returns the Indexing status of the content.
     * @return the Indexing status of the content
     */
    TInt GetIndexStatus();
    
    /*
     * @description Returns the Blacklist status of the content.
     * @return ETrue if the plugin is blacklisted else returns EFalse
     */
    TBool GetBlacklistStatus();
    
    /*
     * @description Sets the name of the content.
     * @Param aName is the name of the content
     * Leaves in case of errors.
     */
    void SetNameL( const TDesC& aName );
    
    /*
     * @description Sets the Indexing status of the content.
     * @Param aIndexStatus is the Indexing status of the content
     */
    void SetIndexStatus( const TInt aIndexStatus );
    
    /*
     * @description Sets the Blacklisting status of the content.
     * @Param aBlacklistStatus is the Blacklisting status of the content
     */
    void SetBlacklistStatus( const TBool aBlacklistStatus );
       
private:    
    /*
     * Name of the content.owned
     */
    HBufC* iContentName;
    /*
     * Indexing status
     */
    TInt iIndexStatus;
    /**
     * Blacklist status
     */
    TBool iBlacklistStatus;
    };

#endif /* CCONTENTINFO_H_ */
