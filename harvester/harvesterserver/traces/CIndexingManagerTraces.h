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

#ifndef __CINDEXINGMANAGERTRACES_H__
#define __CINDEXINGMANAGERTRACES_H__

#define KOstTraceComponentID 0x2001f6fb

#define CINDEXINGMANAGER_LOADPLUGINSL_ENTRY 0x8a0023
#define CINDEXINGMANAGER_LOADPLUGINSL_EXIT 0x8a0024
#define CINDEXINGMANAGER_UPDATECONTENTINFODBL_ENTRY 0x8a0025
#define CINDEXINGMANAGER_UPDATECONTENTINFODBL_EXIT 0x8a0026
#define CINDEXINGMANAGER_UPDATEDONTLOADLISTL_ENTRY 0x8a0027
#define CINDEXINGMANAGER_UPDATEDONTLOADLISTL_EXIT 0x8a0028
#define DUP1_CINDEXINGMANAGER_GETPLUGINLOADSTATUSL_ENTRY 0x8a0029
#define CINDEXINGMANAGER_LOADHARVESTERPLUGINL_ENTRY 0x8a002a
#define CINDEXINGMANAGER_LOADHARVESTERPLUGINL_EXIT 0x8a002b
#define CINDEXINGMANAGER_HANDLEGAURDTIMERL_ENTRY 0x8a002c
#define CINDEXINGMANAGER_HANDLEGAURDTIMERL_EXIT 0x8a002d
#define CINDEXINGMANAGER_RUNL 0x860015
#define DUP1_CINDEXINGMANAGER_RUNL 0x860016
#define DUP2_CINDEXINGMANAGER_RUNL 0x860017
#define DUP3_CINDEXINGMANAGER_RUNL 0x860018
#define DUP4_CINDEXINGMANAGER_RUNL 0x860019
#define DUP5_CINDEXINGMANAGER_RUNL 0x86001a
#define DUP6_CINDEXINGMANAGER_RUNL 0x86001b
#define DUP7_CINDEXINGMANAGER_RUNL 0x86001c
#define DUP8_CINDEXINGMANAGER_RUNL 0x86001d
#define DUP9_CINDEXINGMANAGER_RUNL 0x86001e
#define CINDEXINGMANAGER_ADDHARVESTINGQUEUE 0x86001f
#define DUP1_CINDEXINGMANAGER_ADDHARVESTINGQUEUE 0x860020
#define DUP2_CINDEXINGMANAGER_ADDHARVESTINGQUEUE 0x860021
#define DUP3_CINDEXINGMANAGER_ADDHARVESTINGQUEUE 0x860022
#define CINDEXINGMANAGER_REMOVEHARVESTINGQUEUE 0x860023
#define DUP1_CINDEXINGMANAGER_REMOVEHARVESTINGQUEUE 0x860024
#define DUP2_CINDEXINGMANAGER_REMOVEHARVESTINGQUEUE 0x860025
#define CINDEXINGMANAGER_HARVESTINGCOMPLETED 0x860026
#define DUP1_CINDEXINGMANAGER_HARVESTINGCOMPLETED 0x860027
#define DUP2_CINDEXINGMANAGER_HARVESTINGCOMPLETED 0x860028
#define DUP1_CINDEXINGMANAGER_LOADHARVESTERPLUGINL 0x860029
#define CINDEXINGMANAGER_LOADHARVESTERPLUGINL 0x86002a
#define DUP2_CINDEXINGMANAGER_LOADHARVESTERPLUGINL 0x86002b
#define CINDEXINGMANAGER_ACTIVITYCHANGED 0x86002c
#define DUP1_CINDEXINGMANAGER_ACTIVITYCHANGED 0x86002d
#define CINDEXINGMANAGER_HANDLEGAURDTIMERL 0x86002e


#ifndef __KERNEL_MODE__
#ifndef __OSTTRACEGEN1_TUINT32_CONST_TDESC16REF__
#define __OSTTRACEGEN1_TUINT32_CONST_TDESC16REF__

inline TBool OstTraceGen1( TUint32 aTraceID, const TDesC16& aParam1 )
    {
    TBool retval;
    TInt size = aParam1.Size();
    // BTrace assumes that parameter size is atleast 4 bytes
    if (size % 4 == 0)
        {
        TUint8* ptr = ( TUint8* )aParam1.Ptr();
        // Data is written directly and length is determined from trace message length
        retval = OstSendNBytes( EXTRACT_GROUP_ID(aTraceID), EOstTrace, KOstTraceComponentID, aTraceID, ptr, size );
        }
    else
        {
        TUint8 data[ KOstMaxDataLength ];
        TUint8* ptr = data;
        if (size > KOstMaxDataLength)
            {
            size = KOstMaxDataLength;
            }
        TInt sizeAligned = ( size + 3 ) & ~3;
        memcpy( ptr, aParam1.Ptr(), size );
        ptr += size;
        // Fillers are written to get 32-bit alignment
        while ( size++ < sizeAligned )
            {
            *ptr++ = 0;
            }
        ptr -= sizeAligned;
        size = sizeAligned;
        // Data is written directly and length is determined from trace message length
        retval = OstSendNBytes( EXTRACT_GROUP_ID(aTraceID), EOstTrace, KOstTraceComponentID, aTraceID, ptr, size );
        }
    return retval;
    }

#endif // __OSTTRACEGEN1_TUINT32_CONST_TDESC16REF__

#endif


#ifndef __OSTTRACEGEN2_TUINT32_TINT_TINT__
#define __OSTTRACEGEN2_TUINT32_TINT_TINT__

inline TBool OstTraceGen2( TUint32 aTraceID, TInt aParam1, TInt aParam2 )
    {
    TBool retval = BTraceFiltered8( EXTRACT_GROUP_ID(aTraceID), EOstTraceActivationQuery, KOstTraceComponentID, aTraceID );
    if ( retval )
        {
        TUint8 data[ 8 ];
        TUint8* ptr = data;
        *( ( TInt* )ptr ) = aParam1;
        ptr += sizeof ( TInt );
        *( ( TInt* )ptr ) = aParam2;
        ptr += sizeof ( TInt );
        ptr -= 8;
        retval = OstSendNBytes( EXTRACT_GROUP_ID(aTraceID), EOstTrace, KOstTraceComponentID, aTraceID, ptr, 8 );
        }
    return retval;
    }

#endif // __OSTTRACEGEN2_TUINT32_TINT_TINT__


#ifndef __OSTTRACEGEN2_TUINT32_TINT32_TINT32__
#define __OSTTRACEGEN2_TUINT32_TINT32_TINT32__

inline TBool OstTraceGen2( TUint32 aTraceID, TInt32 aParam1, TInt32 aParam2 )
    {
    TBool retval = BTraceFiltered8( EXTRACT_GROUP_ID(aTraceID), EOstTraceActivationQuery, KOstTraceComponentID, aTraceID );
    if ( retval )
        {
        TUint8 data[ 8 ];
        TUint8* ptr = data;
        *( ( TInt* )ptr ) = aParam1;
        ptr += sizeof ( TInt );
        *( ( TInt* )ptr ) = aParam2;
        ptr += sizeof ( TInt );
        ptr -= 8;
        retval = OstSendNBytes( EXTRACT_GROUP_ID(aTraceID), EOstTrace, KOstTraceComponentID, aTraceID, ptr, 8 );
        }
    return retval;
    }

#endif // __OSTTRACEGEN2_TUINT32_TINT32_TINT32__



#endif

// End of file

