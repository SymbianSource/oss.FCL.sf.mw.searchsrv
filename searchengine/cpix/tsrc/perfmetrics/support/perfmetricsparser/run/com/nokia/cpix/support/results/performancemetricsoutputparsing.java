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

package com.nokia.cpix.support.results;

import java.io.File;


public class PerformanceMetricsOutputParsing {
	
	public static void main(String[] args) {
		new PerformanceMetricsOutputParser(
			new File("runio/perfmetrics/input"), 
			new File("runio/perfmetrics/output")).run();
		System.out.println("done.");
	}


}
