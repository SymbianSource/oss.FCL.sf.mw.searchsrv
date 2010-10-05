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

#ifndef __MESSAGEHARVESTERDEFS_H__
#define __MESSAGEHARVESTERDEFS_H__


/**
 * @file
 * @ingroup Common
 * @brief Message harvester definitions
 */


// ***** MESSAGING ***** 

#define SMS_QBASEAPPCLASS  "@0:root msg phone sms"
// nasty hack to make the expression _L(SMS_QBASEAPPCLASS) compile in symbian
#define LSMS_QBASEAPPCLASS  L"@0:root msg phone sms"
#define ALLMSG_QBASEAPPCLASS  "@0:root msg"
#define LALLMSG_QBASEAPPCLASS  L"@0:root msg"

#define SMSAPPCLASS       "root msg phone sms"
#define LSMSAPPCLASS       L"root msg phone sms"
#define MMSAPPCLASS       "root msg phone mms"
#define LMMSAPPCLASS       L"root msg phone mms"
#define EMAILAPPCLASS       "root msg phone email"
#define LEMAILAPPCLASS       L"root msg phone email"

#define TO_FIELD          "To"
#define CC_FIELD          "Cc"
#define BCC_FIELD         "Bcc"
#define FROM_FIELD        "From"
#define SUBJECT_FIELD     "Subject"
#define FOLDER_FIELD      "Folder"
#define BODY_FIELD        "Body"
#define ATTACHMENT_FIELD  "Attachment"
#define LTO_FIELD          L"To"
#define LCC_FIELD          L"Cc"
#define LBCC_FIELD         L"Bcc"
#define LFROM_FIELD        L"From"
#define LSUBJECT_FIELD     L"Subject"
#define LFOLDER_FIELD      L"Folder"
#define LBODY_FIELD        L"Body"
#define LATTACHMENT_FIELD  L"Attachment"

#endif /*__MESSAGEHARVESTERDEFS_H__*/
