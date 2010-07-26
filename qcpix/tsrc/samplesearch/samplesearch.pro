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
TARGET = SampleSearch 

QT += core
QT += gui 
CONFIG += qtestlib
LIBS += -lcpixsearch

HEADERS   +=    SampleSearch.h \
                searchHandler.h
SOURCES   +=    SampleSearch_reg.rss \
                main.cpp \
                SampleSearch.cpp \
                searchHandler.cpp

symbian:TARGET.UID3 = 0xE11D1021
symbian:TARGET.CAPABILITY = ALL -TCB -DRM

symbian{
    LIBS +=  -lxqservice
}
