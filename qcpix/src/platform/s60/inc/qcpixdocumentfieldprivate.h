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

#ifndef _QCPIXDOCUMENTFIELDPVTIMPL_H
#define _QCPIXDOCUMENTFIELDPVTIMPL_H

#include <QObject>

class QCPixDocumentFieldPrivate: public QObject
{
public:
    QCPixDocumentFieldPrivate(const QString aName,const QString aValue,const int aConfig);
    ~QCPixDocumentFieldPrivate();
    QString iName;
    QString iValue;
    int iConfig;
};

#endif // _QCPIXDOCUMENTFIELDPVTIMPL_H
