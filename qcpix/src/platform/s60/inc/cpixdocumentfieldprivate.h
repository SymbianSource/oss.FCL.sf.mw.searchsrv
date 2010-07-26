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

#ifndef _CPIXDOCUMENTFIELDPVTIMPL_H
#define _CPIXDOCUMENTFIELDPVTIMPL_H

#include <QObject>

class CpixDocumentFieldPrivate: public QObject
{
public:
    CpixDocumentFieldPrivate(const QString aName,const QString aValue,const int aConfig);
    ~CpixDocumentFieldPrivate();
    QString iName;
    QString iValue;
    int iConfig;
};

#endif // _CPIXDOCUMENTFIELDPVTIMPL_H
