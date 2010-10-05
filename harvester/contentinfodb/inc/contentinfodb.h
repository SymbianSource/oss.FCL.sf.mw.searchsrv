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
 * Description:  contentinfodb .
 *
 */
#ifndef CONTENTINFODB_H
#define CONTENTINFODB_H
#include <qobject.h>
#include <QSqlDatabase>
#include <qmutex.h>
#include "contentinfodbcommon.h"
CONTENTINFODB_CLASS(ContentinfodbTest)

class ContentInfoDb : public QObject
    {
Q_OBJECT
public:
    /**
     * Singleton construtor
     * @since S60 ?S60_version.
     */
    static ContentInfoDb* instance()
        {
        static QMutex mutex;
        if (!m_instance)
            {
            mutex.lock();
            m_instanceCounter++;
            if (!m_instance)
                {
                m_instance = new ContentInfoDb;
                }
            mutex.unlock();
            }

        return m_instance;
        }

    /**
     * Singleton destructor
     * @since S60 ?S60_version.
     */
    static void deleteinstance()
        {
        m_instanceCounter--;
        if ((m_instanceCounter <= 0) && (m_instance))
            {
            delete m_instance;
            m_instance = 0;
            }
        }
private:
    /**
     * Constructor.
     * @since S60 ?S60_version.
     */
    ContentInfoDb();

    /**
     * Destructor.
     * @since S60 ?S60_version.
     */
    ~ContentInfoDb();
public:

    /**
     * returns success/failure status.
     * @since S60 ?S60_version.
     * @param query
     */
    bool writeData(QString query);

    /**
     * returns success/failure status.
     * @since S60 ?S60_version.
     * @param query input
     */
    QStringList readData(QString query);

    /**
     * returns value at mentioned position
     * @param query input     
     */
    QString readStringData(QString query);

    /**
     * creates the table with the mentioned tablename.
     * @param tablename 
     * @param sql query
     */
    void createTable(QString tablename, QString query);

    /**
     * returns success/failure status.
     * @since S60 ?S60_version.
     */
    bool Open();

    /**     
     * close the database connection
     * @since S60 ?S60_version.     
     */
    void Close();

signals:

    /**     
     * signal to notify when updation happens in the contentinfodb
     */
    void dbUpdated();

private:

    /**
     * static instance to make single instance
     * Own.
     */
    static ContentInfoDb *m_instance;

    /**
     * counter for number for instance
     * Own.
     */
    static int m_instanceCounter;

    /**
     * database handle
     * Own.
     */
    QSqlDatabase m_db;

private:
    CONTENTINFODB_FRIEND_CLASS(ContentinfodbTest)
    };
#endif //CONTENTINFODB_H
