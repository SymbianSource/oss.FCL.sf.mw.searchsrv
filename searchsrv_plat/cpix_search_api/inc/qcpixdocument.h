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
* Description:  Qt search document APIs
*
*/

#ifndef _QCPIXDOCUMENT_H
#define _QCPIXDOCUMENT_H

#ifdef BUILD_DLL
#define DLL_EXPORT Q_DECL_EXPORT
#else
#define DLL_EXPORT Q_DECL_IMPORT
#endif

/**
 * @file
 * @ingroup Qt Search ClientAPI
 * @brief Contains CCPixDocument APIs
 */

#include <QObject>

//Forward  Declaration
class QCPixDocumentPrivate;
class QCPixDocumentField;

/**
 * @brief Represents the document returned as a result of a search query.
 * @ingroup ClientAPI
 * 
 * Link against: QCPixSearchClient.lib 
 */
class DLL_EXPORT QCPixDocument: public QObject
    {
    Q_OBJECT
public:
    //@TODO: The following enums are copied over from S60. They need to 
    // be moved to a common file so that they can be shared by s60, qt and openc.
    /**
     * TStored defines wheter the value is stored to database.
     * If value is stored to database, it can be retrieved from search result. 
     */
    enum TStored
        {
        EStoreYes = 1,
        EStoreNo = 2
        };

    /**
     * TIndexed defines how the value of the field is indexed. 
     * If value is indexed, it can be searched. 
     */
    enum TIndexed
        {
        EIndexNo = 16,
        EIndexTokenized = 32,
        EIndexUnTokenized = 64
        };

    /**
     * TAggregated defines how the value of the field are exposed 
     * for aggregation. Aggregated field can be found with generic searches. 
     * If aggregation is not specified, all indexed field are aggeregated by
     * default. Note: that also non-indexed fields can be aggregated.
     */
    enum TAggregated
        {
        EAggregateNo = 1<<30,
        EAggregateYes = 1<<31,
        EAggregateDefault = 0
        };
    /**
     * By default, field value is stored to database and it's indexed as tokenized strings.
     */
    static const TInt KDefaultConfig = EStoreYes | EIndexTokenized | EAggregateDefault;

    static const TReal32 KDefaultBoost; 

public:
    /**
       * Constructor.
       * Creates a QCPixDocument object and return a pointer to the created object.
       * @return A pointer to the created instance of CCPixDocument.
       */
    static QCPixDocument* newInstance();

    /**
     * Destructor
     */
    virtual ~QCPixDocument();
    
    /**
     * Getter: Gets the document identifier.
     * @return document identifier.
     */
    QString docId() const;
    
    /**
     * Getter: Gets the document excert.
     * @return document excerpt.
     */
    QString excerpt() const;
    
    /**
     * Getter: Gets the document's base app class.
     * @return document's base app class.
     */
    QString baseAppClass() const;

    /**
     * Getter: Gets the field at aIndex in the document.
     * @param aIndex index of the field that is to be retrieved.
     * @return QCPixDocumentField that was at aIndex.
     */
    const QCPixDocumentField& field( const int aIndex ) const;
    
    /**
     * Getter: Gets the field count.
     * @return The number of the fields in the document.
     */
    int fieldCount() const;
    
    /**
     * Setter: Sets the document identifier for the document.
     * @param aDocId the document identifier to be set.
     */
    void setDocId(const QString aDocId);
    
    /**
     * Setter: Sets the excerpt for the document.
     * @param aExcerpt the excerpt to be set.
     */
    void setExcerpt(const QString aExcerpt);
    
    /**
     * Setter: Sets the base app class for the document.
     * @param aBaseAppClass the base app class to be set.
     */
    void setBaseAppClass(const QString aBaseAppClass);

    /**
     * Setter: Adds a field with aName, aValue and aConfig to the document.
     * @param aName The name of the field
     * @param aValue The value of the field
     * @param aConfig The config of the field
     * @return the QCPixDocumentField that was added to the document.
     */
    void addField(const QString aName, const QString aValue, const int aConfig = QCPixDocument::KDefaultConfig);

private:
    /**
     * Default constructor
     */
    QCPixDocument();

private:
    QCPixDocumentPrivate* const iPvtImpl;
	Q_DECLARE_PRIVATE_D( iPvtImpl, QCPixDocument )
    };

#endif //_QCPIXDOCUMENT_H
