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
 * Description:  Implementation of xml parser for contentinfodb 
 *
 */
#include "contentinfodbparser.h"
#include <qdom.h>
#include <qfile.h> 
// ---------------------------------------------------------------------------
// ContentInfoDbParser::ContentInfoDbParser()
// ---------------------------------------------------------------------------
//
ContentInfoDbParser::ContentInfoDbParser()
    {
    }
// ---------------------------------------------------------------------------
// ContentInfoDbParser::~ContentInfoDbParser()
// ---------------------------------------------------------------------------
//
ContentInfoDbParser::~ContentInfoDbParser()
    {

    }
// ---------------------------------------------------------------------------
// ContentInfoDbParser::parse()
// ---------------------------------------------------------------------------
//
bool ContentInfoDbParser::parse(QString path)
    {
    QDomDocument doc;
    
    QFile file(path);    
    QMap<QString, QString> catDetails;
    QMap<QString, QString> uriDetails;

    if (!file.open(QFile::ReadOnly))
        return false;
    
    QByteArray array = file.readAll();
    
    if (!doc.setContent(array, true))
        {
        file.close();
        return false;
        }
    file.close();
    
    QDomElement docElem = doc.documentElement();

    QDomNode n = docElem.firstChild();

    /*
     * Sending the category details before the other details to prepare the table,
     * update of other details are not possible because the 'PRIMARYKEY' has to be there in db 
     */
    while (!n.isNull())
        {
        QDomElement e = n.toElement();
        if (!e.isNull())
            {
            catDetails.insert(e.tagName(), e.text());
            }
        n = n.nextSibling();
        }
    emit categoryDetails(catDetails);

    n = docElem.firstChild();// repositioning the node to get the other details

    while (!n.isNull())
        {
        QDomElement e = n.toElement(); // try to convert the node to an element.
        if (!e.isNull())
            {
            if (e.tagName() == TAGACTIONURI) // details of activity uri list
                {
                QDomNodeList uriNodeList = n.toElement().childNodes();
                for (int i = 0; i < uriNodeList.count(); i++)
                    {
                    QDomNode uriNode = uriNodeList.item(i);
                    QDomNodeList uriItemsList =
                            uriNode.toElement().childNodes();
                    for (int i = 0; i < uriItemsList.count(); i++)
                        {
                        QDomNode uriitems = uriItemsList.item(i);
                        e = uriitems.toElement();
                        uriDetails.insert(e.tagName(), e.text());
                        }
                    emit actionUri(uriDetails);
                    }
                }
            else if (e.tagName() == TAGFIELDMAP)// details of activity uri mapped with particular field
                {
                QDomNodeList uriNodeList = n.toElement().childNodes();
                QString tagfield;
                QStringList urilist;
                for (int i = 0; i < uriNodeList.count(); i++)
                    {
                    QDomNode uriNode = uriNodeList.item(i);
                    QDomNodeList uriItemsList =
                            uriNode.toElement().childNodes();
                    tagfield = QString();
                    urilist.clear();
                    for (int i = 0; i < uriItemsList.count(); i++)
                        {
                        QDomNode uriitems = uriItemsList.item(i);
                        e = uriitems.toElement();
                        if (e.tagName() == TAGFIELD)
                            {
                            tagfield = e.text();
                            }
                        else if (e.tagName() == TAGAURI)
                            {
                            urilist.append(e.text());
                            }
                        }
                    emit filedMapping(tagfield, urilist);
                    }
                }
            else if (e.tagName() == TAGRESULTFIRSTLINE) // details of what to show on firstline of result screen in search ui 
                {
                QDomNodeList uriNodeList = n.toElement().childNodes();
                QStringList urilist;                
                for (int i = 0; i < uriNodeList.count(); i++)
                    {
                    QDomNode uriNode = uriNodeList.item(i);
                    QDomNodeList uriItemsList =
                            uriNode.toElement().childNodes();
                    urilist.clear();
                    for (int i = 0; i < uriItemsList.count(); i++)
                        {
                        QDomNode uriitems = uriItemsList.item(i);
                        e = uriitems.toElement();
                        if (e.tagName() == TAGFIELD)
                            {
                            urilist.append(e.text());
                            }
                        }
                    emit firstLineMapping(urilist);
                    }
                }
            else if (e.tagName() == TAGRELEVANCY) // boost factor details,   
                {
                QDomNodeList uriNodeList = n.toElement().childNodes();
                QString field;
                QString boostvalue;
                for (int i = 0; i < uriNodeList.count(); i++)
                    {
                    QDomNode uriNode = uriNodeList.item(i);
                    QDomNodeList uriItemsList =
                            uriNode.toElement().childNodes();
                    field = QString();
                    boostvalue = QString();
                    for (int i = 0; i < uriItemsList.count(); i++)
                        {
                        QDomNode uriitems = uriItemsList.item(i);
                        e = uriitems.toElement();
                        if (e.tagName() == TAGFIELD)
                            {
                            field = e.text();
                            }
                        else if (e.tagName() == TAGBOOSTVALUE)
                            {
                            boostvalue = e.text();
                            }
                        }
                    emit relevancyMapping(field, boostvalue);
                    }
                }
            }
        n = n.nextSibling();
        }
    return true;
    }
