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
// Created by TraceCompiler 2.3.0
// DO NOT EDIT, CHANGES WILL BE LOST

#ifndef __CBLACKLISTMGRTRACES_H__
#define __CBLACKLISTMGRTRACES_H__

#define KOstTraceComponentID 0x2001f6fb

#define CBLACKLISTMGR_NEWL_ENTRY 0x8a0017
#define CBLACKLISTMGR_NEWL_EXIT 0x8a0018
#define CBLACKLISTMGR_CONSTRUCTL_ENTRY 0x8a0019
#define CBLACKLISTMGR_CONSTRUCTL_EXIT 0x8a001a
#define CBLACKLISTMGR_ADDL_ENTRY 0x8a001b
#define CBLACKLISTMGR_ADDL_EXIT 0x8a001c
#define CBLACKLISTMGR_REMOVE_ENTRY 0x8a001d
#define CBLACKLISTMGR_REMOVE_EXIT 0x8a001e
#define CBLACKLISTMGR_ADDTOUNLOADLISTL_ENTRY 0x8a001f
#define CBLACKLISTMGR_ADDTODONTLOADLISTL_EXIT 0x8a0020
#define CBLACKLISTMGR_REMOVEFROMUNLOADLISTL_ENTRY 0x8a0021
#define CBLACKLISTMGR_REMOVEFROMDONTLOADLISTL_EXIT 0x8a0022
#define CBLACKLISTMGR_ADDL 0x86000d
#define CBLACKLISTMGR_REMOVE 0x86000e
#define CBLACKLISTMGR_FINDL 0x86000f
#define DUP1_CBLACKLISTMGR_FINDL 0x860010
#define DUP2_CBLACKLISTMGR_FINDL 0x860011
#define CBLACKLISTMGR_ADDTODONTLOADLISTL 0x860012
#define CBLACKLISTMGR_REMOVEFROMUNLOADLISTL 0x860013
#define CBLACKLISTMGR_FINDINDONTLOADLISTL 0x860014


#ifndef __OSTTRACEGEN2_TUINT32_TUINT_TINT__
#define __OSTTRACEGEN2_TUINT32_TUINT_TINT__

inline TBool OstTraceGen2( TUint32 aTraceID, TUint aParam1, TInt aParam2 )
    {
    TBool retval = BTraceFiltered8( EXTRACT_GROUP_ID(aTraceID), EOstTraceActivationQuery, KOstTraceComponentID, aTraceID );
    if ( retval )
        {
        TUint8 data[ 8 ];
        TUint8* ptr = data;
        *( ( TUint* )ptr ) = aParam1;
        ptr += sizeof ( TUint );
        *( ( TInt* )ptr ) = aParam2;
        ptr += sizeof ( TInt );
        ptr -= 8;
        retval = OstSendNBytes( EXTRACT_GROUP_ID(aTraceID), EOstTrace, KOstTraceComponentID, aTraceID, ptr, 8 );
        }
    return retval;
    }

#endif // __OSTTRACEGEN2_TUINT32_TUINT_TINT__


#ifndef __OSTTRACEGEN2_TUINT32_TUINT32_TINT32__
#define __OSTTRACEGEN2_TUINT32_TUINT32_TINT32__

inline TBool OstTraceGen2( TUint32 aTraceID, TUint32 aParam1, TInt32 aParam2 )
    {
    TBool retval = BTraceFiltered8( EXTRACT_GROUP_ID(aTraceID), EOstTraceActivationQuery, KOstTraceComponentID, aTraceID );
    if ( retval )
        {
        TUint8 data[ 8 ];
        TUint8* ptr = data;
        *( ( TUint* )ptr ) = aParam1;
        ptr += sizeof ( TUint );
        *( ( TInt* )ptr ) = aParam2;
        ptr += sizeof ( TInt );
        ptr -= 8;
        retval = OstSendNBytes( EXTRACT_GROUP_ID(aTraceID), EOstTrace, KOstTraceComponentID, aTraceID, ptr, 8 );
        }
    return retval;
    }

#endif // __OSTTRACEGEN2_TUINT32_TUINT32_TINT32__



#endif

// End of file

