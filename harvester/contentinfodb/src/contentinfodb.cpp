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
 * Description:  Implementation of contentinfodb 
 *
 */
#include "contentinfodb.h"
#include <qdir.h>
#include <qvariant.h>
#include <qsqlquery.h>
#include <qstringlist.h>


ContentInfoDb *ContentInfoDb::m_instance = 0;
int ContentInfoDb::m_instanceCounter = 0;

// ---------------------------------------------------------------------------
// ContentInfoDb::ContentInfoDb()
// ---------------------------------------------------------------------------
//
ContentInfoDb::ContentInfoDb()
    {
    m_db = QSqlDatabase::addDatabase(DATABASETYPE, CONNECTIONNAME);
    m_db.setDatabaseName(DBNAME);
    }
// ---------------------------------------------------------------------------
// ContentInfoDb::~ContentInfoDb()
// ---------------------------------------------------------------------------
//
ContentInfoDb::~ContentInfoDb()
    {
    Close();
    }
// ---------------------------------------------------------------------------
// ContentInfoDb::writeData()
// ---------------------------------------------------------------------------
//
bool ContentInfoDb::writeData(QString query)
    {
    if (Open())
        {
        QSqlQuery q(QSqlDatabase::database(CONNECTIONNAME));
        q.prepare(query);
        if (q.exec())
            {
            emit dbUpdated();
            return true;
            }
        return false;
        }
    return false;
    }
// ---------------------------------------------------------------------------
// ContentInfoDb::readData()
// ---------------------------------------------------------------------------
//
QStringList ContentInfoDb::readData(QString query)
    {
    QStringList list;
    if (Open())
        {        
        QSqlQuery q(QSqlDatabase::database(CONNECTIONNAME));
        q.prepare(query);
        q.exec();
        while (q.next())
            {
            list.append(q.value(0).toString());  
            }
        }
    return list;
    }
// ---------------------------------------------------------------------------
// ContentInfoDb::readStringData()
// ---------------------------------------------------------------------------
//
QString ContentInfoDb::readStringData(QString query)
    {
    if (Open())
        {
        QSqlQuery q(QSqlDatabase::database(CONNECTIONNAME));
        q.prepare(query);
        q.exec();
        q.next();
        return q.value(0).toString();
        }
    return QString();
    }
// ---------------------------------------------------------------------------
// ContentInfoDb::createTable()
// ---------------------------------------------------------------------------
//
void ContentInfoDb::createTable(QString tablename, QString query)
    {
    if (!m_db.tables().contains(tablename))
        {
        QSqlQuery q(query, m_db);
        }
    }
// ---------------------------------------------------------------------------
// ContentInfoDb::open()
// ---------------------------------------------------------------------------
//    
bool ContentInfoDb::Open()
    {
    if (!m_db.isValid())
        {
        Close();
        return false;
        }
    if (!m_db.isOpen())
        {
        if (!m_db.open())
            {
            Close();
            return false;
            }
        }
    if (!m_db.tables().contains(MAINTABLENAME))
        {
        QSqlQuery q(QUERYCREATEMAINTABLE, m_db);
        }
    return true;
    }
// ---------------------------------------------------------------------------
// ContentInfoDb::Close()
// ---------------------------------------------------------------------------
//
void ContentInfoDb::Close()
    {
    m_db.close();
    }
