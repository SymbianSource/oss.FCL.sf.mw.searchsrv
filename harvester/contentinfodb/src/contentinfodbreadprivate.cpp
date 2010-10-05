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
 * Description:  read api implementation 
 *
 */
#include "contentinfodbreadprivate.h"
#include "contentinfodb.h"
#include "cpixcontentinfodbread.h"


// ---------------------------------------------------------------------------
// ContentInfoDbReadPrivate::ContentInfoDbReadPrivate()
// ---------------------------------------------------------------------------
//
ContentInfoDbReadPrivate::ContentInfoDbReadPrivate(QObject* aParent)
    {
    iReadPtr = reinterpret_cast<ContentInfoDbRead*> (aParent);
    mdb = ContentInfoDb::instance();
    connect(mdb, SIGNAL(dbUpdated()),this, SLOT(slotdataChanged()));

    }
// ---------------------------------------------------------------------------
// ContentInfoDbReadPrivate::ContentInfoDbReadPrivate()
// ---------------------------------------------------------------------------
//
ContentInfoDbReadPrivate::~ContentInfoDbReadPrivate()
    {
    ContentInfoDb::deleteinstance();
    }
// ---------------------------------------------------------------------------
// ContentInfoDbReadPrivate::ContentInfoDbReadPrivate()
// ---------------------------------------------------------------------------
//
QStringList ContentInfoDbReadPrivate::getPrimaryKeys()
    {
    return mdb->readData(QString(QUERYSORT));
    }
// ---------------------------------------------------------------------------
// ContentInfoDbReadPrivate::ContentInfoDbReadPrivate()
// ---------------------------------------------------------------------------
//
QString ContentInfoDbReadPrivate::getValues(QString Key, QString columnname)
    {
    return mdb->readStringData(
            QString(QUERYDATARETRIVAL).arg(columnname).arg(MAINTABLENAME).arg(
                    TAGPRIMARYKEY).arg(Key));
    }
// ---------------------------------------------------------------------------
// ContentInfoDbReadPrivate::ContentInfoDbReadPrivate()
// ---------------------------------------------------------------------------
//
QStringList ContentInfoDbReadPrivate::getActionuri(QString Key)
    {
    QString tablename = mdb->readStringData(QString(QUERYDATARETRIVAL).arg(
            TAGACTIONURI).arg(MAINTABLENAME).arg(TAGPRIMARYKEY).arg(Key));
    if (!tablename.length())
        return QStringList();
    return mdb->readData(
            QString(QUERYPRIMARYKEYS).arg(TAGACTIONURISTRING).arg(tablename));
    }
// ---------------------------------------------------------------------------
// ContentInfoDbReadPrivate::ContentInfoDbReadPrivate()
// ---------------------------------------------------------------------------
//
QString ContentInfoDbReadPrivate::getShortTaburi(QString Key)
    {
    QString tablename = mdb->readStringData(QString(QUERYDATARETRIVAL).arg(
            TAGACTIONURI).arg(MAINTABLENAME).arg(TAGPRIMARYKEY).arg(Key));
    if (!tablename.length())
        return NULL;
    return mdb->readStringData(QString(QUERYFILTERDATA).arg(
            TAGACTIONURISTRING).arg(tablename).arg(TAGACTION).arg(
            PRIMARYVALUE));
    }
// ---------------------------------------------------------------------------
// ContentInfoDbReadPrivate::ContentInfoDbReadPrivate()
// ---------------------------------------------------------------------------
//
QStringList ContentInfoDbReadPrivate::getLongTaburi(QString Key,
        QString FieldName)
    {
    QStringList longurilist;

    QString tablename = mdb->readStringData(QString(QUERYDATARETRIVAL).arg(
            TAGACTIONURI).arg(MAINTABLENAME).arg(TAGPRIMARYKEY).arg(Key));
    if (!tablename.length())
        return QStringList();
    QString fieldmaptable =
            mdb->readStringData(
                    QString(QUERYDATARETRIVAL).arg(TAGFIELDMAP).arg(
                            MAINTABLENAME).arg(TAGPRIMARYKEY).arg(Key));

    QString
            filedmapsubtable = mdb->readStringData(
                    QString(QUERYDATARETRIVAL).arg(TAGURILIST).arg(
                            fieldmaptable).arg(TAGFIELD).arg(FieldName));

    QStringList urireflist = mdb->readData(QString(QUERYPRIMARYKEYS).arg(
            TAGAURI).arg(filedmapsubtable));

    for (int i = 0; i < urireflist.count(); i++)
        {
        QString str;

        str = mdb->readStringData(QString(QUERYDATARETRIVAL).arg(
                TAGACTIONURISTRING).arg(tablename).arg(TAGURIREF).arg(
                urireflist.at(i)));

        if (str.length())
            longurilist.append(str);
        }
    return longurilist;
    }
// ---------------------------------------------------------------------------
// ContentInfoDbReadPrivate::ContentInfoDbReadPrivate()
// ---------------------------------------------------------------------------
//
bool ContentInfoDbReadPrivate::geturiDetails(QString key, QString uri,
        QString& iconname, QString& displayname)
    {
    QString tablename = mdb->readStringData(QString(QUERYDATARETRIVAL).arg(
            TAGACTIONURI).arg(MAINTABLENAME).arg(TAGPRIMARYKEY).arg(key));

    if (!tablename.length())
        return false;

    iconname = QString();
    displayname = QString();

    iconname = mdb->readStringData(
            QString(QUERYDATARETRIVAL).arg(TAGLNAME).arg(tablename).arg(
                    TAGACTIONURISTRING).arg(uri));

    displayname = mdb->readStringData(
            QString(QUERYDATARETRIVAL).arg(TAGLICON).arg(tablename).arg(
                    TAGACTIONURISTRING).arg(uri));
    if (iconname.length() && displayname.length())
        return true;
    return false;
    }
// ---------------------------------------------------------------------------
// ContentInfoDbReadPrivate::slotdataChanged()
// ---------------------------------------------------------------------------
//
void ContentInfoDbReadPrivate::slotdataChanged()
    {
    if(iReadPtr)
        emit iReadPtr->dataChanged();
    }
// ---------------------------------------------------------------------------
// ContentInfoDbReadPrivate::categoryCount()
// ---------------------------------------------------------------------------
//
int ContentInfoDbReadPrivate::categoryCount()
    {
    return (mdb->readStringData(QString(QUERYCOUNT))).toInt();
    }
