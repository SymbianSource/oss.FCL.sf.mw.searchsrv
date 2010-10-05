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
TARGET = cpixsearch 
QT        += core
QT        -= gui 

INCLUDEPATH += ../s60/searchclient/inc
INCLUDEPATH += ../cfg

HEADERS   += ../searchsrv_plat/cpix_search_api/inc/cpixcommon.h \
             ../searchsrv_plat/cpix_search_api/inc/cpixdocument.h \
             ../searchsrv_plat/cpix_search_api/inc/cpixdocumentfield.h \
             ../searchsrv_plat/cpix_search_api/inc/cpixsearcher.h 

SOURCES   += src/cpixdocument.cpp \ 
             src/cpixdocumentfield.cpp \
             src/cpixsearcher.cpp

DEFINES += BUILD_DLL

symbian{
    TARGET.CAPABILITY = CAP_GENERAL_DLL
    TARGET.EPOCALLOWDLLDATA = 1
    TARGET.UID3 = 0x20029B38
    TARGET.VID = VID_DEFAULT
    VERSION = 1.0.0
    LIBS += -leuser -lcpixsearchclient
    
    INCLUDEPATH += $$APP_LAYER_SYSTEMINCLUDE
    
    HEADERS +=  src/platform/s60/inc/cpixdocumentprivate.h \
                src/platform/s60/inc/cpixdocumentfieldprivate.h \
                src/platform/s60/inc/cpixsearcherprivate.h \
                src/platform/s60/inc/cpixutils.h 
        
    SOURCES +=  src/platform/s60/src/cpixdocumentfieldprivate.cpp \
                src/platform/s60/src/cpixsearcherprivate.cpp \
                src/platform/s60/src/cpixutils.cpp
    
    qcpixsearchclient.sources += /epoc32/release/armv5/urel/cpixsearch.dll
    qcpixsearchclient.path = /sys/bin
    DEPLOYMENT += cpixsearch
    
    defFileBlock =                    \
    "$${LITERAL_HASH}if defined(WINSCW)"    \
    "DEFFILE bwins/"                        \
    "$${LITERAL_HASH}else"                  \
    "DEFFILE eabi/"                         \
    "$${LITERAL_HASH}endif"                 \
    
    MMP_RULES += defFileBlock

}