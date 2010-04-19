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

#ifndef _QCPIXDOCUMENTFIELD_H
#define _QCPIXDOCUMENTFIELD_H

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

//Forward decelaration
class QCPixDocumentFieldPrivate;

/**
 * @brief Represents the fields of documents returned as a result of a search query.
 * @ingroup ClientAPI
 * 
 * Link against: QCPixSearchClient.lib 
 */
class DLL_EXPORT QCPixDocumentField: public QObject
    {
    Q_OBJECT
public:
    /**
     * Constructor.
     * Creates a QCPixDocumentField object and return a pointer to the created object.
     * @param aName Name of the field
     * @param aValue Value of the field
     * @param aConfig Config for the field
     * @return A pointer to the created instance of QCPixDocumentField.
     * 
     * @note After using this constructor, the client has to mandatorily call 
     * SetDatabase() before invoking any search.
     */
    static QCPixDocumentField* newInstance( const QString aName, const QString aValue, const int aConfig );

    /**
     * Destructor.
     */
    virtual ~QCPixDocumentField();

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

private:
	/**
     * Constructor.
     * Creates a QCPixDocumentField object and return a pointer to the created object.
     * @param aName Name of the field
     * @param aValue Value of the field
     * @param aConfig Config for the field
     * @return A pointer to the created instance of QCPixDocumentField.
     */
    QCPixDocumentField(const QString aName, const QString aValue, const int aConfig);

private:
    QCPixDocumentFieldPrivate* const iPvtImpl;
	Q_DECLARE_PRIVATE_D( iPvtImpl, QCPixDocumentField )
    };		

#endif //_QCPIXDOCUMENTFIELD_H
