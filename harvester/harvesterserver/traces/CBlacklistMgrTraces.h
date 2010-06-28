// Created by TraceCompiler 2.2.2
// DO NOT EDIT, CHANGES WILL BE LOST

#ifndef __CBLACKLISTMGRTRACES_H__
#define __CBLACKLISTMGRTRACES_H__

#define KOstTraceComponentID 0x2001f6fb

#define CBLACKLISTMGR_NEWL_ENTRY 0x8a000f
#define CBLACKLISTMGR_NEWL_EXIT 0x8a0010
#define CBLACKLISTMGR_CONSTRUCTL_ENTRY 0x8a0011
#define CBLACKLISTMGR_CONSTRUCTL_EXIT 0x8a0012
#define CBLACKLISTMGR_ADDL_ENTRY 0x8a0013
#define CBLACKLISTMGR_ADDL_EXIT 0x8a0014
#define CBLACKLISTMGR_REMOVE_ENTRY 0x8a0015
#define CBLACKLISTMGR_REMOVE_EXIT 0x8a0016
#define CBLACKLISTMGR_ADDTOUNLOADLISTL_ENTRY 0x8a0043
#define CBLACKLISTMGR_REMOVEFROMUNLOADLISTL_ENTRY 0x8a0045
#define CBLACKLISTMGR_ADDTODONTLOADLISTL_EXIT 0x8a0055
#define CBLACKLISTMGR_REMOVEFROMDONTLOADLISTL_EXIT 0x8a0056
#define CBLACKLISTMGR_ADDL 0x86000b
#define CBLACKLISTMGR_REMOVE 0x86000c
#define CBLACKLISTMGR_FINDL 0x86000d
#define DUP1_CBLACKLISTMGR_FINDL 0x86000e
#define DUP2_CBLACKLISTMGR_FINDL 0x86000f
#define CBLACKLISTMGR_REMOVEFROMUNLOADLISTL 0x86002b
#define CBLACKLISTMGR_ADDTODONTLOADLISTL 0x86002f
#define CBLACKLISTMGR_FINDINDONTLOADLISTL 0x860030


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


#endif

// End of file

