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
TARGET = cpixharvesterserver 

QT += core
QT -= gui

symbian:TARGET.UID3 = 0x2001f6fb
symbian:TARGET.CAPABILITY = ALL -TCB -DRM -AllFiles

symbian{
    INCLUDEPATH += $$MW_LAYER_SYSTEMINCLUDE
    
    INCLUDEPATH +=   ../../searchengine/cpix/cpix/inc/public  
    
    INCLUDEPATH +=   ../contentinfodb/inc  
	HEADERS   +=   inc/CHarvesterServer.h
	HEADERS   +=   inc/CHarvesterServerSession.h
	HEADERS   +=   inc/CIndexingManager.h
	HEADERS   +=   inc/CBlacklistMgr.h
	HEADERS   +=   inc/CBlacklistDb.h
    	HEADERS   +=   inc/cactivitymanager.h
	HEADERS   +=   inc/harvesterserver.pan
	HEADERS   +=   inc/harvesterservercommons.h
	HEADERS   +=   inc/qtmythread.h
	HEADERS   +=   inc/cgaurdtimer.h
	HEADERS   +=   inc/cpixstateobserver.h
	HEADERS   +=   traces/CBlacklistDbTraces.h
	HEADERS   +=   traces/CBlacklistMgrTraces.h
	HEADERS   +=   traces/CIndexingManagerTraces.h
	HEADERS   +=   traces/OstTraceDefinitions.h
	HEADERS   +=   traces/fixed_id.definitions
	HEADERS   +=   traces/cactivitymanagerTraces.h
	 
	SOURCES   +=   src/main.cpp
	SOURCES   +=   src/CHarvesterServer.cpp
	SOURCES   +=   src/CHarvesterServerSession.cpp
	SOURCES   +=   src/CIndexingManager.cpp 
	SOURCES   +=   src/CBlacklistMgr.cpp 
	SOURCES   +=   src/CBlacklistDb.cpp 
	SOURCES   +=   src/cactivitymanager.cpp
	SOURCES   +=   src/cgaurdtimer.cpp
	SOURCES   +=   src/cpixstateobserver.cpp
	        
    resourceTargetBlock = 						\
    "SOURCEPATH        data" 				\
	"START RESOURCE    2001F6FB.rss" 			\
	"TARGETPATH        /private/101f875a/import"\
	"END"										\
    
    MMP_RULES += resourceTargetBlock
    
    MMP_RULES += "EPOCSTACKSIZE     0x5000"
    MMP_RULES += "EPOCHEAPSIZE      0x50000 0x300000" #From about 65KB to about 3 MB

    LIBS += -leuser -lestor -lCPixSearchClient -lCPixHarvesterPluginInterface -ledbms -lsqldb -lcentralrepository -lflogger -lefsrv 
    LIBS += -lcpixcontentinfodb -lmdccommon -lmdeclient -lharvesterclient -lmpxcollectionhelper -lmpxcollectionutility -lmpxcommon
    
}
