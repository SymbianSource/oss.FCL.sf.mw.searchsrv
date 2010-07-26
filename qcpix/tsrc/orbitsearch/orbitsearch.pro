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

TEMPLATE = app
TARGET = OrbitSearch 

QT += core
QT += gui
QT += sql 
CONFIG += qtestlib
LIBS += -lcpixsearch -lxqservice

HEADERS   +=    uicontrols.h searchhelper.h testsearch.h testci.h
SOURCES   +=    OrbitSearch_reg.rss \
                main.cpp searchhelper.cpp testsearch.cpp testci.cpp

symbian:TARGET.UID3 = 0xE11D1102
symbian:TARGET.CAPABILITY = ALL -TCB -DRM

symbian{
    INCLUDEPATH += /epoc32/include/mw/hb
    INCLUDEPATH += /epoc32/include/mw/hb/hbcore
    INCLUDEPATH += /epoc32/include/mw/hb/hbwidgets
    
    LIBS += -lhbcore -lhbwidgets
}
