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
 * Description:  update api for contentinfodb 
 *
 */
#include "contentinfodbupdate.h"
#include "contentinfodbparser.h"
#include "contentinfodb.h"
#include "cpixcontentinfodbdef.h"

// ---------------------------------------------------------------------------
// ContentInfoDbUpdate::ContentInfoDbUpdate()
// ---------------------------------------------------------------------------
//
ContentInfoDbUpdate::ContentInfoDbUpdate() :
    iParser(NULL),mdb(NULL)
    {
    mdb = ContentInfoDb::instance();
    iParser = new ContentInfoDbParser();
    if (iParser)
        {
        connect(iParser, SIGNAL(categoryDetails(QMap<QString,QString>)),
                this, SLOT(slotupdateCategoryInfo(QMap<QString,QString>)));
        connect(iParser, SIGNAL(actionUri(QMap<QString,QString>)), this,
                SLOT(slotupdateUri(QMap<QString,QString>)));
        connect(iParser, SIGNAL(filedMapping(QString,QStringList)), this,
                SLOT(slotupdateFieldmapping(QString,QStringList)));
        connect(iParser, SIGNAL(firstLineMapping(QStringList)), this,
                SLOT(slotfirstLineMapping(QStringList)));
        connect(iParser, SIGNAL(relevancyMapping(QString,QString)), this,
                SLOT(slotrelevancyMapping(QString,QString)));
        }
    counter = 0;
    }
// ---------------------------------------------------------------------------
// ContentInfoDbUpdate::~ContentInfoDbUpdate()
// ---------------------------------------------------------------------------
//
ContentInfoDbUpdate::~ContentInfoDbUpdate()
    {
    ContentInfoDb::deleteinstance();
    }
// ---------------------------------------------------------------------------
// ContentInfoDbUpdate::UpdateDb()
// ---------------------------------------------------------------------------
//
bool ContentInfoDbUpdate::UpdateDb(QString xmlpath)
    {
    if (mdb)
       (void)mdb->Open();
    if (iParser)
        {
        primarykey = QString();
        return iParser->parse(xmlpath);
        }
    return false;
    }
// ---------------------------------------------------------------------------
// ContentInfoDbUpdate::UpdateBlacklisted()
// ---------------------------------------------------------------------------
//
bool ContentInfoDbUpdate::UpdateBlacklisted(QString primarykey, int value)
    {
    if (mdb)
        (void)mdb->Open();
    QString query = QString(QUERYUPDATEDATA).arg(MAINTABLENAME).arg(
            BLACKLISTSTATUS).arg(value).arg(TAGPRIMARYKEY).arg(primarykey);
    if (!mdb->writeData(query))
        return false;
    return true;
    }
// ---------------------------------------------------------------------------
// ContentInfoDbUpdate::UpdateIndexStatus()
// ---------------------------------------------------------------------------
//
bool ContentInfoDbUpdate::UpdateIndexStatus(QString primarykey, int value)
    {
    if (mdb)
        (void)mdb->Open();
    QString query = QString(QUERYUPDATEDATA).arg(MAINTABLENAME).arg(
            INDEXINGSTATUS).arg(value).arg(TAGPRIMARYKEY).arg(primarykey);
    if (!mdb->writeData(query))
        return false;
    return true;
    }
// ---------------------------------------------------------------------------
// ContentInfoDbUpdate::InsertPrimaryKey()
// ---------------------------------------------------------------------------
//
QString ContentInfoDbUpdate::InsertPrimaryKey(QString tableName,
        QString tagName, QMap<QString, QString> data)
    {
    QMapIterator<QString, QString> i(data);
    QString query;
    QString p_key;
    while (i.hasNext())
        {
        i.next();
        if (i.key() == tagName)
            {
            p_key = i.value();
            break;
            }
        }
    query = QString(QUERYINSERTROW).arg(tableName).arg(tagName).arg(p_key);
    mdb->writeData(query);
    return p_key;
    }
// ---------------------------------------------------------------------------
// ContentInfoDbUpdate::slotupdateCategoryInfo()
// ---------------------------------------------------------------------------
//
void ContentInfoDbUpdate::slotupdateCategoryInfo(QMap<QString, QString> map)
    {
    QString query;
    QString p_key = InsertPrimaryKey(MAINTABLENAME, TAGPRIMARYKEY, map);
    primarykey = p_key;
    if (p_key.length())
        {
        QMapIterator<QString, QString> i(map);
        while (i.hasNext())
            {
            i.next();
            if (i.key() != TAGPRIMARYKEY)
                {
                query = QString();
                if ((i.key() == TAGACTIONURI) || (i.key() == TAGFIELDMAP)
                        || (i.key() == TAGRELEVANCY) || (i.key()
                        == TAGRESULTFIRSTLINE))
                    {
                    counter++;
                    QString tablename = QString(DYNAMICTABLE).arg(counter);
                    query = QString(QUERYUPDATEDATA).arg(MAINTABLENAME).arg(
                            i.key()).arg(tablename).arg(TAGPRIMARYKEY).arg(
                            p_key);
                    }
                else
                    {
                    query = QString(QUERYUPDATEDATA).arg(MAINTABLENAME).arg(
                            i.key()).arg(i.value()).arg(TAGPRIMARYKEY).arg(
                            p_key);
                    }
                if (!mdb->writeData(query))
                    {
                    
                    }                
                }
            }
        }
    }
// ---------------------------------------------------------------------------
// ContentInfoDbUpdate::slotupdateUri()
// ---------------------------------------------------------------------------
//
void ContentInfoDbUpdate::slotupdateUri(QMap<QString, QString> map)
    {
    //get actionuri table name
    QString query = QString(QUERYDATARETRIVAL).arg(TAGACTIONURI).arg(
            MAINTABLENAME).arg(TAGPRIMARYKEY).arg(primarykey);
    QString actionuritablename = mdb->readStringData(query);

    if (actionuritablename.length())
        {
        query = QString();
        query = QString(QUERYCREATEACTIONURITABLE).arg(actionuritablename);
        mdb->createTable(actionuritablename, query);
        QString p_key = InsertPrimaryKey(actionuritablename,
                TAGACTIONURISTRING, map);
        if (p_key.length())
            {
            QMapIterator<QString, QString> i(map);
            while (i.hasNext())
                {
                i.next();
                if (i.key() != TAGACTIONURISTRING)
                    {
                    query = QString();
                    query
                            = QString(QUERYUPDATEDATA).arg(actionuritablename).arg(
                                    i.key()).arg(i.value()).arg(
                                    TAGACTIONURISTRING).arg(p_key);
                    if (!mdb->writeData(query))
                        {
                        
                        }   
                    }
                }
            }
        }
    }
// ---------------------------------------------------------------------------
// ContentInfoDbUpdate::slotupdateFieldmapping()
// ---------------------------------------------------------------------------
//
void ContentInfoDbUpdate::slotupdateFieldmapping(QString key,
        QStringList value)
    {
    //get fieldmap table name from maintable
    QString query = QString(QUERYDATARETRIVAL).arg(TAGFIELDMAP).arg(
            MAINTABLENAME).arg(TAGPRIMARYKEY).arg(primarykey);
    QString fieldtablename = mdb->readStringData(query);

    if (fieldtablename.length())
        {
        // create a table for fieldmap 
        query = QString();
        query = QString(QUERYCREATEFIELDMAPTABLE).arg(fieldtablename);
        mdb->createTable(fieldtablename, query);

        // insert the field in filedmap table
        query = QString();
        query
                = QString(QUERYINSERTROW).arg(fieldtablename).arg(TAGFIELD).arg(
                        key);
        mdb->writeData(query);
            {
            
            }

        counter++;
        query = QString();

        QString tablename = QString(DYNAMICTABLE).arg(counter);

        // enter the value as table name second column in field map will refer to another table
        query
                = QString(QUERYUPDATEDATA).arg(fieldtablename).arg(TAGURILIST).arg(
                        tablename).arg(TAGFIELD).arg(key);
        if (!mdb->writeData(query))
            {
            
            }

        query = QString();
        // creating a table for the field map values 
        query = QString(QUERYCREATEFIELDMAPSUBTABLE).arg(tablename);
        mdb->createTable(tablename, query);

        for (int i = 0; i < value.count(); i++)
            {
            query = QString();
            // inserting the uri list mapped for a particular field
            query = QString(QUERYINSERTROW).arg(tablename).arg(TAGAURI).arg(
                    value[i]);
            if (!mdb->writeData(query))
                {
            
                }
            }
        }
    }
// ---------------------------------------------------------------------------
// ContentInfoDbUpdate::slotfirstLineMapping()
// ---------------------------------------------------------------------------
//
void ContentInfoDbUpdate::slotfirstLineMapping(QStringList list)
    {
    //get actionuri table name
    QString query = QString(QUERYDATARETRIVAL).arg(TAGRESULTFIRSTLINE).arg(
            MAINTABLENAME).arg(TAGPRIMARYKEY).arg(primarykey);
    QString firstlinetablename = mdb->readStringData(query);

    if (firstlinetablename.length())
        {
        query = QString();
        query = QString(QUERYCREATEFIRSTLINETABLE).arg(firstlinetablename);
        mdb->createTable(firstlinetablename, query);

        for (int i = 0; i < list.count(); i++)
            {
            query = QString();
            // inserting the uri list mapped for a particular field
            query = QString(QUERYINSERTROW).arg(firstlinetablename).arg(
                    TAGFIELD).arg(list[i]);
            if (!mdb->writeData(query))
                {
                
                }
            }
        }
    }
// ---------------------------------------------------------------------------
// ContentInfoDbUpdate::slotrelevancyMapping()
// ---------------------------------------------------------------------------
//
void ContentInfoDbUpdate::slotrelevancyMapping(QString field,
        QString boostvalue)
    {
    QString query = QString(QUERYDATARETRIVAL).arg(TAGRELEVANCY).arg(
            MAINTABLENAME).arg(TAGPRIMARYKEY).arg(primarykey);
    QString relevancytablename = mdb->readStringData(query);
    if (relevancytablename.length())
        {
        query = QString();
        query = QString(QUERYCREATERELEVANCYTABLE).arg(relevancytablename);
        mdb->createTable(relevancytablename, query);

        query = QString();
        // inserting field name as a primary key
        query
                = QString(QUERYINSERTROW).arg(relevancytablename).arg(
                        TAGFIELD).arg(field);

        if (!mdb->writeData(query))
            {

            }
        query = QString();
        // updating the row with boostvalue
        query = QString(QUERYUPDATEDATA).arg(relevancytablename).arg(
                TAGBOOSTVALUE).arg(boostvalue).arg(TAGFIELD).arg(field);
        if (!mdb->writeData(query))
            {

            }
        }
    }
// ---------------------------------------------------------------------------
// ContentInfoDbUpdate::slotrelevancyMapping()
// ---------------------------------------------------------------------------
//
void ContentInfoDbUpdate::deleteCategory(QString Key)
    {
    if (mdb)
        (void)mdb->Open();
    mdb->writeData(QString(QUERYDELETE).arg(Key));
    }
