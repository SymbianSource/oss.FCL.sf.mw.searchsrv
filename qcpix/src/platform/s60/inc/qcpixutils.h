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
* Description: 
*
*/

#ifndef QCPIXUTILS_H_
#define QCPIXUTILS_H_

/**
 * Helper to convert symbian descriptor to QString.
 * @param aDesc the descriptor to be converted to QString.
 * @return QString representation of the descriptor.
 */
QString QStringFromDescriptor( const TDesC& aDesc );

/**
 * Gets a QCPixDocument from CSearchDocument
 * @param aDoc CSearchDocument to be converted to QCPixDocument. Takes ownership of aDoc.
 * @return QCPixDocument representation of aDoc. Ownership transferred to caller.
 */
QCPixDocument* QCPixDocFromCPixDoc( CSearchDocument* aDoc );

#endif /* QCPIXUTILS_H_ */
