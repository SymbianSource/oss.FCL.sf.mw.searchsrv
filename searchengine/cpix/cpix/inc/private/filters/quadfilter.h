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

#ifndef CPIX_QUADFILTER_H_
#define CPIX_QUADFILTER_H_


namespace Cpix
{

    class QuadFilter : public IFieldFilter
    {
    public:
        //
        // public operations
        //
        
        virtual bool process(Cpix::Document * doc);
        

        virtual ~QuadFilter();

    private:
        //
        // implementation details
        //
        void emitWarning(Cpix::Document * doc,
                         const char                 * msg);

        void process(Cpix::Document * doc,
                     double           gpsLat,
                     double           gpsLong);
        
    };

}

#endif // CPIX_QUADFILTER_H_
