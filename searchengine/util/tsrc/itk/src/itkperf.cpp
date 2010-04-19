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
/*
 * itkperf.cpp
 *
 *  Created on: Jun 3, 2009
 *      Author: admin
 */

#include "itkperf.h"

namespace Itk {

	
	Timestamper::Timestamper(const char    * name,
				Itk::TestMgr  * testMgr)
		: name_(name),
		  testMgr_(testMgr)
	{
		Itk::getTimestamp(&begin_);
	}


	Timestamper::~Timestamper()
	{
		Itk::Timestamp end;
		Itk::getTimestamp(&end);

		long
			millis = Itk::getElapsedMs(&end,
									   &begin_);

		ITK_REPORT(testMgr_,
				   name_.c_str(),
				   "%d ms",
				   millis);
	}


}
