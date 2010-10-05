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
 * Description:  ContentInfoDbReadPrivate implementation of read api's
 *
 */

#ifndef CONTENTINFODBREADPRIVATE_H
#define CONTENTINFODBREADPRIVATE_H

/**
 * @file
 * @ingroup contentinfodb read api's
 * @brief Contains ContentInfoDbReadPrivate used for reading the DB 
 */

#include <qobject.h>
#include <qstringlist.h>
#include "contentinfodbcommon.h"

class ContentInfoDbRead;
class ContentInfoDb;

CONTENTINFODB_CLASS(ContentinfodbTest)
class ContentInfoDbReadPrivate : public QObject
    {
Q_OBJECT
public:

    /**
     * Constructor.
     * @since S60 ?S60_version.
     */
    ContentInfoDbReadPrivate(QObject*);

    /**
     * Destructor.
     * @since S60 ?S60_version.
     */
    ~ContentInfoDbReadPrivate();

public:
    
    /**
     * returns primary keys alias category names    
     */
    QStringList getPrimaryKeys();

    /**
     * return db value for the mentioned key and fieldname     
     */
    QString getValues(QString Key, QString columnname);
    
    /**
     * returns the list of activity uris for the mentioned category     
     */
    QStringList getActionuri(QString Key);

    /**
     * returns primary activityuri     
     */
    QString getShortTaburi(QString Key);

    /**
     * returns list of uri's mapped for particular field     
     */
    QStringList getLongTaburi(QString Key, QString FieldName);

    /**
     * returns the logical name/icon for a particular uri     
     */
    bool geturiDetails(QString key, QString uri, QString& iconname,
            QString& displayname);
    
    /**
     * returns total number of categories available in contentinfoDb     
     */
    int categoryCount();

public slots:
    /**
     * slot to get notify when changes happen in Db     
     */
    void slotdataChanged();

private:
    /**
     * handler to emit datachanged signal
     * Not Own.
     */
    ContentInfoDbRead* iReadPtr;

    /**
     * handler to manage database operations
     * Own.
     */
    ContentInfoDb* mdb;

CONTENTINFODB_FRIEND_CLASS    (ContentinfodbTest)
    };

#endif //ContentInfoDbReadPrivate
