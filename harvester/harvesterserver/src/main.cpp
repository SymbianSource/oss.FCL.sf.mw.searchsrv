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
#include <QtCore>
#include <QCoreApplication>
#include "charvesterserver.h"
#include "qtmythread.h"

int main(int argc, char *argv[])
    {
    QCoreApplication a( argc , argv);
    HarvesterThread mythread;    
    mythread.start();
    return a.exec();
    }

void HarvesterThread::run()
    {
    User::SetCritical(User::EProcessCritical);
    //Trapping the error is handled inside the ThreadFunction    
    CHarvesterServer::ThreadFunction();
    exec();
    }
