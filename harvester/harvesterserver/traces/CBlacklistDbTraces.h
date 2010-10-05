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

#ifndef __CBLACKLISTDBTRACES_H__
#define __CBLACKLISTDBTRACES_H__

#define KOstTraceComponentID 0x2001f6fb

#define CBLACKLISTDB_NEWL_ENTRY 0x8a0001
#define CBLACKLISTDB_NEWL_EXIT 0x8a0002
#define CBLACKLISTDB_CONSTRUCTL_ENTRY 0x8a0003
#define CBLACKLISTDB_CONSTRUCTL_EXIT 0x8a0004
#define CBLACKLISTDB_ADDL_ENTRY 0x8a0005
#define CBLACKLISTDB_ADDL_EXIT 0x8a0006
#define CBLACKLISTDB_REMOVE_ENTRY 0x8a0007
#define CBLACKLISTDB_REMOVE_EXIT 0x8a0008
#define CBLACKLISTDB_UPDATEL_ENTRY 0x8a0009
#define CBLACKLISTDB_UPDATEL_EXIT 0x8a000a
#define CBLACKLISTDB_CREATEDBL_ENTRY 0x8a000b
#define CBLACKLISTDB_CREATEDBL_EXIT 0x8a000c
#define CBLACKLISTDB_CREATEBLACKLISTCOLUMNSETLC_ENTRY 0x8a000d
#define CBLACKLISTDB_CREATEBLACKLISTCOLUMNSETLC_EXIT 0x8a000e
#define CBLACKLISTDB_CREATEDONTLOADCOLUMNSETLC_ENTRY 0x8a000f
#define CBLACKLISTDB_CREATEDONTLOADCOLUMNSETLC_EXIT 0x8a0010
#define CBLACKLISTDB_ADDTODONTLOADLISTL_ENTRY 0x8a0011
#define CBLACKLISTDB_ADDTODONTLOADLISTL_EXIT 0x8a0012
#define CBLACKLISTDB_REMOVEFROMDONTLOADLISTL_ENTRY 0x8a0013
#define CBLACKLISTDB_REMOVEFROMDONTLOADLISTL_EXIT 0x8a0014
#define CBLACKLISTDB_FINDINDONTLOADLISTL_ENTRY 0x8a0015
#define CBLACKLISTDB_FINDINDONTLOADLISTL_EXIT 0x8a0016
#define CBLACKLISTDB_CONSTRUCTL 0x860001
#define CBLACKLISTDB_ADDL 0x860002
#define CBLACKLISTDB_REMOVE 0x860003
#define DUP1_CBLACKLISTDB_REMOVE 0x860004
#define DUP2_CBLACKLISTDB_REMOVE 0x860005
#define CBLACKLISTDB_FINDWITHVERSIONL 0x860006
#define DUP1_CBLACKLISTDB_FINDWITHVERSIONL 0x860007
#define CBLACKLISTDB_UPDATEL 0x860008
#define CBLACKLISTDB_FINDL 0x860009
#define DUP1_CBLACKLISTDB_FINDL 0x86000a
#define DUP3_CBLACKLISTDB_REMOVEFROMDONTLOADLISTL 0x86000b
#define CBLACKLISTDB_FINDFROMDONTLOADLISTL 0x86000c


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

