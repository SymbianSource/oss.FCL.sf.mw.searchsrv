/*
* Copyright (c) 2010 Nokia Corporation and/or its subsidiary(-ies).
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
* Description:  Qt search document field APIs
*
*/

#ifndef _CPIXDOCUMENTFIELD_H
#define _CPIXDOCUMENTFIELD_H

#ifdef BUILD_DLL
#define DLL_EXPORT Q_DECL_EXPORT
#else
#define DLL_EXPORT Q_DECL_IMPORT
#endif

/**
 * @file
 * @ingroup Qt Search ClientAPI
 * @brief Contains CCPixDocument field APIs
 */

#include <QObject>
#include <QStringList>

//Forward decelaration
class CpixDocumentFieldPrivate;

/**
 * @brief Represents the fields of documents returned as a result of a search query.
 * @ingroup ClientAPI
 * 
 * Link against: cpixsearch.lib 
 */
class DLL_EXPORT CpixDocumentField: public QObject
    {
    Q_OBJECT
public:
    /**
     * Constructor.
     * Creates a CpixDocumentField object and return a pointer to the created object.
     * @param aName Name of the field
     * @param aValue Value of the field
     * @param aConfig Config for the field
     * @return A pointer to the created instance of CpixDocumentField.
     * 
     * @note After using this constructor, the client has to mandatorily call 
     * SetDatabase() before invoking any search.
     */
    static CpixDocumentField* newInstance( const QString aName, const QString aValue, const int aConfig );

    /**
     * Destructor.
     */
    virtual ~CpixDocumentField();

    /**
     * Getter: Gets the name of the field.
     * @return The name of the field
     */
    QString name() const;

    /**
     * Getter: Gets the Value of the field.
     * @return The value of the field
     */
    QString value() const;

    /**
     * Getter: Gets the Config of the field.
     * @return The config of the field
     */
    int config() const;

    /**
     * Setter: Sets the Name of the field.
     * @param aName The name of the field
     */
    void setName(const QString aName); 

    /**
     * Setter: Sets the Value of the field.
     * @param aValue The value of the field
     */
    void setValue(const QString aValue);

    /**
     * Setter: Sets the Name of the field.
     * @param aConfig The config of the field
     */
    void setConfig(const int aConfig);
    
    /**
     * Getter: Gets the Value of the field.
     * @return The value of the field in list
     */
    QStringList values() const;

private:
	/**
     * Constructor.
     * Creates a CpixDocumentField object and return a pointer to the created object.
     * @param aName Name of the field
     * @param aValue Value of the field
     * @param aConfig Config for the field
     * @return A pointer to the created instance of CpixDocumentField.
     */
    CpixDocumentField(const QString aName, const QString aValue, const int aConfig);

private:
    CpixDocumentFieldPrivate* const iPvtImpl;
	Q_DECLARE_PRIVATE_D( iPvtImpl, CpixDocumentField )
    };		

#endif //_CPIXDOCUMENTFIELD_H
