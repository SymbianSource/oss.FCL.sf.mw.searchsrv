// Created by TraceCompiler 2.2.3
// DO NOT EDIT, CHANGES WILL BE LOST

#ifndef __CCPIXSEARCHERTRACES_H__
#define __CCPIXSEARCHERTRACES_H__

#define KOstTraceComponentID 0x2001f6fe

#define CCPIXSEARCHER_SEARCHL_ENTRY 0x8a0001
#define DUP1_CCPIXSEARCHER_SEARCHL_ENTRY 0x8a0002
#define CCPIXSEARCHER_SEARCHL_EXIT 0x8a0003
#define CCPIXSEARCHER_GETDOCUMENTL_ENTRY 0x8a0004
#define DUP1_CCPIXSEARCHER_GETDOCUMENTL_ENTRY 0x8a0005
#define CCPIXSEARCHER_GETDOCUMENTL_EXIT 0x8a0006
#define CCPIXSEARCHER_OPENDATABASEL_ENTRY 0x8a0010
#define CCPIXSEARCHER_OPENDATABASEL_EXIT 0x8a0011
#define DUP1_CCPIXSEARCHER_OPENDATABASEL_ENTRY 0x8a0012
#define DUP1_CCPIXSEARCHER_OPENDATABASEL_EXIT 0x8a0013
#define CCPIXSEARCHER_SETANALYZERL_ENTRY 0x8a0014
#define CCPIXSEARCHER_SETANALYZERL_EXIT 0x8a0015
#define DUP1_CCPIXSEARCHER_SETANALYZERL_ENTRY 0x8a0016
#define DUP1_CCPIXSEARCHER_SETANALYZERL_EXIT 0x8a0017
#define CCPIXSEARCHER_SEARCHL 0x860003
#define DUP1_CCPIXSEARCHER_SEARCHL 0x860004
#define CCPIXSEARCHER_GETDOCUMENTL 0x860005
#define DUP1_CCPIXSEARCHER_GETDOCUMENTL 0x860006
#define CCPIXSEARCHER_GETBATCHDOCUMENTL 0x860007
#define DUP1_CCPIXSEARCHER_GETBATCHDOCUMENTL 0x860008
#define CCPIXSEARCHER_RUNL 0x860009


#ifndef __KERNEL_MODE__
#ifndef __OSTTRACEGEN2_TUINT32_CONST_TDESC16REF_CONST_TDESC16REF__
#define __OSTTRACEGEN2_TUINT32_CONST_TDESC16REF_CONST_TDESC16REF__

inline TBool OstTraceGen2( TUint32 aTraceID, const TDesC16& aParam1, const TDesC16& aParam2 )
    {
    TBool retval = BTraceFiltered8( EXTRACT_GROUP_ID(aTraceID), EOstTraceActivationQuery, KOstTraceComponentID, aTraceID );
    if ( retval )
        {
        TInt length = 0;
        // Check that parameter lenght is not too long
        TInt length1 = aParam1.Size();
        if ((length + length1 + sizeof ( TUint32 )) > KOstMaxDataLength)
            {
            length1 = KOstMaxDataLength - (length + sizeof ( TUint32 ));
            }
        TInt lengthAligned1 = ( length1 + 3 ) & ~3;
        if (lengthAligned1 > 0)
            {
            length = length + sizeof ( TUint32 ) + lengthAligned1;
            }
        // Check that parameter lenght is not too long
        TInt length2 = aParam2.Size();
        if ((length + length2 + sizeof ( TUint32 )) > KOstMaxDataLength)
            {
            length2 = KOstMaxDataLength - (length + sizeof ( TUint32 ));
            }
        TInt lengthAligned2 = ( length2 + 3 ) & ~3;
        if (lengthAligned2 > 0)
            {
            length = length + sizeof ( TUint32 ) + lengthAligned2;
            }
        TUint8 data[ KOstMaxDataLength ];
        TUint8* ptr = data;
        // Set length to zero and calculate it againg
        // when adding parameters
        length = 0;
        if (length1 > 0)
            {
            // Number of elements is written before data
            // In case of Unicode string, number of elements is half of length
            *( ( TUint32* )ptr ) = length1 / (aParam1.Size() / aParam1.Length());
            ptr += sizeof ( TUint32 );
            memcpy( ptr, aParam1.Ptr(), length1 );
            ptr += length1;
            // Fillers are written to get 32-bit alignment
            while ( length1++ < lengthAligned1 )
                {
                *ptr++ = 0;
                }
            length += sizeof ( TUint32 ) + lengthAligned1;
            }
        else if (length + sizeof ( TUint32 ) <= KOstMaxDataLength)
            {
            *( ( TUint32* )ptr ) = 0;
            ptr += sizeof ( TUint32 );
            length += sizeof ( TUint32 );
            }
        if (length2 > 0)
            {
            // Number of elements is written before data
            // In case of Unicode string, number of elements is half of length
            *( ( TUint32* )ptr ) = length2 / (aParam2.Size() / aParam2.Length());
            ptr += sizeof ( TUint32 );
            memcpy( ptr, aParam2.Ptr(), length2 );
            ptr += length2;
            // Fillers are written to get 32-bit alignment
            while ( length2++ < lengthAligned2 )
                {
                *ptr++ = 0;
                }
            length += sizeof ( TUint32 ) + lengthAligned2;
            }
        else if (length + sizeof ( TUint32 ) <= KOstMaxDataLength)
            {
            *( ( TUint32* )ptr ) = 0;
            ptr += sizeof ( TUint32 );
            length += sizeof ( TUint32 );
            }
        ptr -= length;
        retval = OstSendNBytes( EXTRACT_GROUP_ID(aTraceID), EOstTrace, KOstTraceComponentID, aTraceID, ptr, length );
        }
    return retval;
    }

#endif // __OSTTRACEGEN2_TUINT32_CONST_TDESC16REF_CONST_TDESC16REF__

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

