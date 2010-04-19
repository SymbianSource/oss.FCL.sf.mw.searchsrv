#
# Copyright (c) 2010 Nokia Corporation and/or its subsidiary(-ies).
# All rights reserved.
# This component and the accompanying materials are made available
# under the terms of "Eclipse Public License v1.0"
# which accompanies this distribution, and is available
# at the URL "http://www.eclipse.org/legal/epl-v10.html".
#
# Initial Contributors:
# Nokia Corporation - initial contribution.
#
# Contributors:
#
# Description: 
#

TEMPLATE = lib
TARGET = qcpixsearchclient 
QT        += core
QT        -= gui 

INCLUDEPATH += ../s60/searchclient/inc
INCLUDEPATH += ../cfg

HEADERS   += ../searchsrv_plat/cpix_search_api/inc/qcpixcommon.h \
             ../searchsrv_plat/cpix_search_api/inc/qcpixdocument.h \
             ../searchsrv_plat/cpix_search_api/inc/qcpixdocumentfield.h \
             ../searchsrv_plat/cpix_search_api/inc/qcpixsearcher.h 

SOURCES   += src/qcpixdocument.cpp \ 
             src/qcpixdocumentfield.cpp \
             src/qcpixsearcher.cpp

DEFINES += BUILD_DLL

symbian{
    TARGET.CAPABILITY = CAP_GENERAL_DLL -DRM
    TARGET.EPOCALLOWDLLDATA = 1
    TARGET.UID3 = 0xE3B89364
    TARGET.VID = VID_DEFAULT
    VERSION = 1.0.0
    LIBS += -leuser -lcpixsearchclient
    
    HEADERS +=  src/platform/s60/inc/qcpixdocumentprivate.h \
                src/platform/s60/inc/qcpixdocumentfieldprivate.h \
                src/platform/s60/inc/qcpixsearcherprivate.h \
                src/platform/s60/inc/qcpixutils.h 
        
    SOURCES +=  src/platform/s60/src/qcpixdocumentfieldprivate.cpp \
                src/platform/s60/src/qcpixsearcherprivate.cpp \
                src/platform/s60/src/qcpixutils.cpp
    
    qcpixsearchclient.sources += /epoc32/release/armv5/urel/qcpixsearchclient.dll
    qcpixsearchclient.path = /sys/bin
    DEPLOYMENT += qcpixsearchclient
    
    defFileBlock =                    \
    "$${LITERAL_HASH}if defined(WINSCW)"    \
    "DEFFILE bwins/"                        \
    "$${LITERAL_HASH}else"                  \
    "DEFFILE eabi/"                         \
    "$${LITERAL_HASH}endif"                 \
    
    MMP_RULES += defFileBlock

}