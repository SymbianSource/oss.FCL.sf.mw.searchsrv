// Created by TraceCompiler 2.2.2
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
#define CBLACKLISTDB_CREATECOLUMNSETLC_ENTRY 0x8a000d
#define CBLACKLISTDB_CREATECOLUMNSETLC_EXIT 0x8a000e
#define CBLACKLISTDB_CREATEUNLOADCOLUMNSETLC_ENTRY 0x8a003b
#define CBLACKLISTDB_CREATEUNLOADCOLUMNSETLC_EXIT 0x8a003c
#define CBLACKLISTDB_ADDTOUNLOADLISTL_ENTRY 0x8a003d
#define CBLACKLISTDB_ADDTOUNLOADLISTL_EXIT 0x8a003e
#define CBLACKLISTDB_REMOVEFROMUNLOADLISTL_ENTRY 0x8a003f
#define CBLACKLISTDB_REMOVEFROMUNLOADLISTL_EXIT 0x8a0040
#define CBLACKLISTDB_FINDFROMUNLOADLISTL_ENTRY 0x8a0041
#define CBLACKLISTDB_FINDFROMUNLOADLISTL_EXIT 0x8a0042
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
#define CBLACKLISTDB_REMOVEFROMUNLOADLISTL 0x860027
#define DUP1_CBLACKLISTDB_REMOVEFROMUNLOADLISTL 0x860028
#define CBLACKLISTDB_FINDFROMUNLOADLISTL 0x860029


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

