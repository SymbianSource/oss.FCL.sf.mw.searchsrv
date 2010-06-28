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

include(../qttestutil/qttestutil.pri)

TEMPLATE = app
TARGET = qtcpixunittests 
CONFIG += qtestlib

QT        += core
QT        -= gui 

HEADERS   += inc/ut_qtcpixsearcher.h \
             inc/ut_qtcpixdocument.h \
             inc/ut_qtcpixdocumentfield.h

SOURCES   += src/ut_qtcpixsearcher.cpp \
             src/ut_qtcpixdocument.cpp \
             src/ut_qtcpixdocumentfield.cpp \
             ../qttestutil/SimpleChecker.cpp \
             qtcpixunittests_reg.rss

symbian{
    BLD_INF_RULES.prj_exports += "data/_0.cfs      \epoc32\winscw\c\private\2001f6f7\indexing\indexdb\root\contact\_0\_0.cfs" \
                                 "data/deletable   \epoc32\winscw\c\private\2001f6f7\indexing\indexdb\root\contact\_0\deletable" \
                                 "data/segments    \epoc32\winscw\c\private\2001f6f7\indexing\indexdb\root\contact\_0\segments" \
                                 "data/cpixreg.txt \epoc32\winscw\c\private\2001f6f7\cpixreg.txt" \
                                 "data/config.ini  \epoc32\winscw\c\system\data\config.ini"
	
    INCLUDEPATH += $$APP_LAYER_SYSTEMINCLUDE
	
    TARGET.CAPABILITY = ALL -TCB -DRM
    TARGET.UID3 = 0xE76C2AE7
    LIBS += -lqcpixsearchclient
}
