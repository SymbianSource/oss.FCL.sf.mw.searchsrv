/*
 * Copyright (c) 2009 Nokia Corporation and/or its subsidiary(-ies).
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
 * Description:  Implementation of read api for contentinfodb 
 *
 */
#include "cpixcontentinfodbread.h"
#include "contentinfodbreadprivate.h"
#include <qstringlist.h>

// ---------------------------------------------------------------------------
// ContentInfoDbRead::ContentInfoDbRead()
// ---------------------------------------------------------------------------
//
ContentInfoDbRead::ContentInfoDbRead():iPvtImpl( new ContentInfoDbReadPrivate( this ) )
    {
    }
// ---------------------------------------------------------------------------
// ContentInfoDbRead::~ContentInfoDbRead()
// ---------------------------------------------------------------------------
//
ContentInfoDbRead::~ContentInfoDbRead()
    {

    }
// ---------------------------------------------------------------------------
// ContentInfoDbRead::getPrimaryKeys()
// ---------------------------------------------------------------------------
//
QStringList ContentInfoDbRead::getPrimaryKeys()
    {
    return iPvtImpl->getPrimaryKeys();
    }
// ---------------------------------------------------------------------------
// ContentInfoDbRead::getValues()
// ---------------------------------------------------------------------------
//
QString ContentInfoDbRead::getValues(QString Key, QString columnname)
    {
    return iPvtImpl->getValues(Key,columnname);
    }
// ---------------------------------------------------------------------------
// ContentInfoDbRead::getActionuri()
// ---------------------------------------------------------------------------
//
QStringList ContentInfoDbRead::getActionuri(QString Key)
    {
    return iPvtImpl->getActionuri(Key);
    }
// ---------------------------------------------------------------------------
// ContentInfoDbRead::getShortTaburi()
// ---------------------------------------------------------------------------
//
QString ContentInfoDbRead::getShortTaburi(QString Key)
    {
    return iPvtImpl->getShortTaburi(Key);
    }
// ---------------------------------------------------------------------------
// ContentInfoDbRead::getLongTaburi()
// ---------------------------------------------------------------------------
//
QStringList ContentInfoDbRead::getLongTaburi(QString Key, QString FieldName)
    {
    return iPvtImpl->getLongTaburi(Key,FieldName);
    }
// ---------------------------------------------------------------------------
// ContentInfoDbRead::geturiDetails()
// ---------------------------------------------------------------------------
//
bool ContentInfoDbRead::geturiDetails(QString key, QString uri, QString& iconname,
        QString& displayname)
    {
    return iPvtImpl->geturiDetails(key,uri,iconname,displayname);
    }
int ContentInfoDbRead::categoryCount()
    {
    return iPvtImpl->categoryCount();
    }
