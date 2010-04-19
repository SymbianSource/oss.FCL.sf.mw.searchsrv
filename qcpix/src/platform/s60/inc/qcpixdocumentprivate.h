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

#ifndef _QCPIXDOCUMENTPVTIMPL_H
#define _QCPIXDOCUMENTPVTIMPL_H

#include <QString>
#include <QList>
#include "qcpixdocumentfield.h"

//Forward Declarations
class QCPixDocumentField;

class QCPixDocumentPrivate: public QObject
{
public:
    QString iDocId;
    QString iExcerpt;
    QString iBaseAppClass;
    QList< QCPixDocumentField* > iFields;
};

#endif //_QCPIXDOCUMENTPVTIMPL_H
