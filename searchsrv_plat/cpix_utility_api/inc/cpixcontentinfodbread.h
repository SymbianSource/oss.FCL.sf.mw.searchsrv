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
 * Description:  contentinfodb read api.
 *
 */

#ifndef CONTENTINFODBREAD_H
#define CONTENTINFODBREAD_H

/**
 * @file
 * @ingroup contentinfodb read api's
 * @brief Contains ContentInfoDbRead used for reading the DB 
 */

#include <qobject.h>

#ifdef BUILD_CONTENTINFODB
#define DECL_CONTENTINFODB Q_DECL_EXPORT
#else
#define DECL_CONTENTINFODB Q_DECL_IMPORT
#endif

class ContentInfoDbReadPrivate;

class DECL_CONTENTINFODB ContentInfoDbRead : public QObject
    {
Q_OBJECT
public:

    /**
     * Constructor.
     * @since S60 ?S60_version.
     */
    ContentInfoDbRead();

    /**
     * Destructor.
     * @since S60 ?S60_version.
     */
    ~ContentInfoDbRead();

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

signals:
    /*
     * signal to get notificcation when db gets updated
     */
    void dataChanged();
private:
    ContentInfoDbReadPrivate* const iPvtImpl;
    Q_DECLARE_PRIVATE_D( iPvtImpl, ContentInfoDbRead )
    };

#endif //CONTENTINFODBREAD_H
