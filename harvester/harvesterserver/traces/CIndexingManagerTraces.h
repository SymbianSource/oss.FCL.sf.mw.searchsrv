// Created by TraceCompiler 2.2.2
// DO NOT EDIT, CHANGES WILL BE LOST

#ifndef __CINDEXINGMANAGERTRACES_H__
#define __CINDEXINGMANAGERTRACES_H__

#define KOstTraceComponentID 0x2001f6fb

#define CINDEXINGMANAGER_UPDATECONTENTINFODBL_ENTRY 0x8a0047
#define CINDEXINGMANAGER_UPDATECONTENTINFODBL_EXIT 0x8a0048
#define CINDEXINGMANAGER_UPDATEUNLOADLISTL_ENTRY 0x8a0049
#define CINDEXINGMANAGER_UPDATEUNLOADLISTL_EXIT 0x8a004a
#define CINDEXINGMANAGER_RUNL 0x860010
#define DUP1_CINDEXINGMANAGER_RUNL 0x860011
#define DUP2_CINDEXINGMANAGER_RUNL 0x860012
#define DUP3_CINDEXINGMANAGER_RUNL 0x860013
#define DUP4_CINDEXINGMANAGER_RUNL 0x860014
#define DUP5_CINDEXINGMANAGER_RUNL 0x860015
#define DUP6_CINDEXINGMANAGER_RUNL 0x860016
#define DUP7_CINDEXINGMANAGER_RUNL 0x860017
#define DUP8_CINDEXINGMANAGER_RUNL 0x860018
#define DUP9_CINDEXINGMANAGER_RUNL 0x860019
#define CINDEXINGMANAGER_LOADPLUGINSL 0x86001a
#define DUP1_CINDEXINGMANAGER_LOADPLUGINSL 0x86001b
#define DUP2_CINDEXINGMANAGER_LOADPLUGINSL 0x86001c
#define CINDEXINGMANAGER_ADDHARVESTINGQUEUE 0x86001d
#define DUP1_CINDEXINGMANAGER_ADDHARVESTINGQUEUE 0x86001e
#define DUP2_CINDEXINGMANAGER_ADDHARVESTINGQUEUE 0x86001f
#define DUP3_CINDEXINGMANAGER_ADDHARVESTINGQUEUE 0x860020
#define CINDEXINGMANAGER_REMOVEHARVESTINGQUEUE 0x860021
#define DUP1_CINDEXINGMANAGER_REMOVEHARVESTINGQUEUE 0x860022
#define DUP2_CINDEXINGMANAGER_REMOVEHARVESTINGQUEUE 0x860023
#define CINDEXINGMANAGER_HARVESTINGCOMPLETED 0x860024
#define DUP1_CINDEXINGMANAGER_HARVESTINGCOMPLETED 0x860025
#define DUP2_CINDEXINGMANAGER_HARVESTINGCOMPLETED 0x860026


#ifndef __KERNEL_MODE__
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
#endif


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


#endif

// End of file

