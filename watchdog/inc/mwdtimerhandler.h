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
* Description:  This application is to monitor Harvester and Search Server
*
*/

#ifndef MWDTIMERHANDLER_H
#define MWDTIMERHANDLER_H


class MWDTimerHandler
    {
public :
    /*
    * HandleWDTimerL signal when the watchdog timer expires 
    */
    virtual void HandleWDTimerL() = 0;
    };


#endif /* MWDTIMERHANDLER_H */
