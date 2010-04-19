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

package com.nokia.cpix.util;

import java.io.File;

public class FileIntroducer {
	
	public static void introduce(File file, Visitor<File> visitor) {
		if (file.isDirectory()) {
			for (File f : file.listFiles()) {
				introduce(f, visitor); 
			}
		} else if (file.isFile()) {
			visitor.visit(file); 
		} else {
			throw new IllegalArgumentException(file.toString()); 
		}
	}
	
}
