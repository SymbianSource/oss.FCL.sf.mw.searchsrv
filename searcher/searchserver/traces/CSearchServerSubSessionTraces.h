// Created by TraceCompiler 2.2.3
// DO NOT EDIT, CHANGES WILL BE LOST

#ifndef __CSEARCHSERVERSUBSESSIONTRACES_H__
#define __CSEARCHSERVERSUBSESSIONTRACES_H__

#define KOstTraceComponentID 0x2001f6f7

#define CSEARCHSERVERSUBSESSION_CSEARCHSERVERSUBSESSION 0x86000b
#define DUP1_CSEARCHSERVERSUBSESSION_CSEARCHSERVERSUBSESSION 0x86000c
#define CSEARCHSERVERSUBSESSION_LIMITEXCERPTTOMAXLENGTHL 0x86000d
#define DUP1_CSEARCHSERVERSUBSESSION_LIMITEXCERPTTOMAXLENGTHL 0x86000e
#define CSEARCHSERVERSUBSESSION_SEARCHL_ENTRY 0x8a000c
#define CSEARCHSERVERSUBSESSION_SEARCHL_EXIT 0x8a000d
#define DUP1_CSEARCHSERVERSUBSESSION_SEARCHL_EXIT 0x8a000e
#define CSEARCHSERVERSUBSESSION_SEARCHCOMPLETEL_ENTRY 0x8a000f
#define CSEARCHSERVERSUBSESSION_SEARCHCOMPLETEL_EXIT 0x8a0010
#define CSEARCHSERVERSUBSESSION_GETDOCUMENTL_ENTRY 0x8a0011
#define CSEARCHSERVERSUBSESSION_GETDOCUMENTL_EXIT 0x8a0012
#define DUP1_CSEARCHSERVERSUBSESSION_GETDOCUMENTL_EXIT 0x8a0013
#define CSEARCHSERVERSUBSESSION_GETDOCUMENTCOMPLETEL_ENTRY 0x8a0014
#define CSEARCHSERVERSUBSESSION_GETDOCUMENTCOMPLETEL_EXIT 0x8a0015
#define CSEARCHSERVERSUBSESSION_GETDOCUMENTOBJECTL_ENTRY 0x8a0016
#define DUP1_CSEARCHSERVERSUBSESSION_GETDOCUMENTOBJECTL_EXIT 0x8a0018
#define CSEARCHSERVERSUBSESSION_GETDOCUMENTOBJECTL_EXIT 0x8a0019


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


#endif

// End of file

