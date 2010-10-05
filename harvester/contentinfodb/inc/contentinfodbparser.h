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
 * Description:  contentinfodb parser.
 *
 */
#ifndef CONTENTINFODBPARSER_H
#define CONTENTINFODBPARSER_H
#include <qobject.h>
#include <qstringlist.h>
#include <qmap.h>
#include "contentinfodbcommon.h"
CONTENTINFODB_CLASS(ContentinfodbTest)

class ContentInfoDbParser : public QObject
    {
Q_OBJECT
public:

    /**
     * Constructor.
     * @since S60 ?S60_version.
     */
    ContentInfoDbParser();

    /**
     * Destructor.
     * @since S60 ?S60_version.
     */
    ~ContentInfoDbParser();

    /**
     * parse the xml file.
     * @param path is the path for xml file.
     */
    bool parse(QString path);

signals:

    /**
     * signal to notify the category details.
     * @param Qmap is the tagname  and value.
     */
    void categoryDetails(QMap<QString, QString> );

    /**
     * signal to notify the activity uri details.
     * @param Qmap is the tagname  and value.
     */
    void actionUri(QMap<QString, QString> );

    /**
     * signal to notify the field mapping with activity uri's.
     * @param Qmap is the tagname  and value.
     */
    void filedMapping(QString, QStringList);

    /**
     * signal to notfy the first line display information.
     * @param list will be having the field names.
     */
    void firstLineMapping(QStringList);

    /**
     * signal to notfy the first line relevancy information.
     * @param field name.
     * @param boostvalue
     */
    void relevancyMapping(QString, QString);

private:
    CONTENTINFODB_FRIEND_CLASS(ContentinfodbTest)
    };
#endif //CONTENTINFODBPARSER_H
