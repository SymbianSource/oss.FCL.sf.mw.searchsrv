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
 * Description:  contentinfodb update api.
 *
 */
#ifndef CONTENTINFODBUPDATE_H
#define CONTENTINFODBUPDATE_H
#include "contentinfodbcommon.h"
#include <qobject.h>
#include <qmap.h>
#include <qstringlist.h>

CONTENTINFODB_CLASS(ContentinfodbTest)

class ContentInfoDbParser;
class ContentInfoDb;

class DECL_CONTENTINFODB ContentInfoDbUpdate : public QObject
    {
Q_OBJECT
public:
    /**
     * Constructor.
     * @since S60 ?S60_version.
     */
    ContentInfoDbUpdate();

    /**
     * Destructor.
     * @since S60 ?S60_version.
     */
    ~ContentInfoDbUpdate();

public:

    /**
     * returns success/failure status.
     * @since S60 ?S60_version.
     * @param xmlpath is the file path
     */
    bool UpdateDb(QString xmlpath);

    /**
     * returns success/failure status.
     * @since S60 ?S60_version.
     * @param primarykey   
     * @param value is the blacklisted status
     */
    bool UpdateBlacklisted(QString primarykey, int value);

    /**
     * returns success/failure status.
     * @since S60 ?S60_version.
     * @param primarykey   
     * @param value is the indexing status
     */
    bool UpdateIndexStatus(QString primarykey, int value);
    
    /**
     * deletes the row for the mentioned primarykey
     * @since S60 ?S60_version.
     * @param primarykey
     */    
    void deleteCategory(QString primarykey);

private:

    /**
     * funtion to create a row with primary key on mentioned table.
     * @param tablename    
     * @param tagname
     * @param data will have mapped data of tagname and value 
     */
    QString InsertPrimaryKey(QString tableName, QString tagName, QMap<
            QString, QString> data);

public slots:

    /**
     * slot to update the category details to db.     
     * @param map will have mapped data of tagname and value 
     */
    void slotupdateCategoryInfo(QMap<QString, QString> );

    /**
     * slot to update the activity uri details to db.     
     * @param map will have mapped data of tagname and value 
     */
    void slotupdateUri(QMap<QString, QString> );

    /**
     * slot to update the fieldmapping details to db.     
     * @param string will be having name of the field
     * @param list will be having the uri reference number list 
     */
    void slotupdateFieldmapping(QString, QStringList);

    /**
     * slot to update the first line details to db.     
     * @param list will be having the field name as values
     */
    void slotfirstLineMapping(QStringList);

    /**
     * slot to update the relevancy details to db.     
     * @param string as field name
     * @param string as boostfactor 
     */
    void slotrelevancyMapping(QString, QString);

private:

    /**
     * parser handler
     * Own.
     */
    ContentInfoDbParser* iParser;

    /**
     * handler to manage database operations
     * Own.
     */
    ContentInfoDb* mdb;

    /**
     * not to create a repeated table names for dynamic table creation  
     * Own.
     */
    int counter;

    /**
     * primary key  
     * Own.
     */
    QString primarykey;
private:
    CONTENTINFODB_FRIEND_CLASS(ContentinfodbTest)
    };
#endif //CONTENTINFODBUPDATE_H
