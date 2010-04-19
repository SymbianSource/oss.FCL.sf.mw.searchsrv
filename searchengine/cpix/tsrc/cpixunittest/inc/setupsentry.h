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

#ifndef SETUPSENTRY_H
#define SETUPSENTRY_H


namespace Itk
{
    class ITestContext;
}


/**
 * This utility class will help in Itk::ITestContext implementors to
 * implement their setup function in a decent manner.
 *
 *  (1) Initialize all pointers to NULL in the constructor
 *
 *  (2) define cleanup() that releases / deletes those pointers AND
 *      sets them to NULL
 *
 *  (3) call cleanup() from tearDown() and the destructor
 *
 *  (4) In the setup() function, create an instance of this class
 *      template-instantiated for that context class, like
 *
 *      SetupSentry
 *        ss(*this);
 *
 *  (5) At the end of the setup function, call
 *
 *        ss.setupComplete();
 *
 *  (6) Keep using ITK_PANIC macros as you will (they are implemented
 *      by means of C++ exceptions): the ss instance will call to the
 *      tearDown() (and thus the cleanup()) method, so you don't need
 *      to free / delete / reset to NULL in setup() functions. Also,
 *      one does not need temp auto_ptr-s anymore.
 *
 * One would use auto_ptr all around with C++ instances, but many of
 * the pointers are of the type cpix_XXX *, which do not have
 * destructors, hence this gadgetry.
 * 
 */
class SetupSentry
{
private:
    //
    // private members
    //
    Itk::ITestContext   & context_;
    bool                  isSetupComplete_;

public:
    SetupSentry(Itk::ITestContext & context);

    
    ~SetupSentry();


    void setupComplete();
};


#endif SETUPSENTRY_H
