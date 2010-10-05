#
# Copyright (c) 2009 Nokia Corporation and/or its subsidiary(-ies).
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
# Description:  will parse the xml file and does sql write/read operations
#

TEMPLATE = lib
TARGET = cpixcontentinfodb

DEFINES += BUILD_CONTENTINFODB 

QT += xml \
      sql

INCLUDEPATH += ./inc \
              ./src 

symbian {
    TARGET.UID2 = 0x20004C45
    TARGET.UID3 = 0x2002C37E    
    TARGET.EPOCALLOWDLLDATA = 1
    TARGET.CAPABILITY = CAP_GENERAL_DLL
    INCLUDEPATH += $$MW_LAYER_SYSTEMINCLUDE 
    INCLUDEPATH += $$APP_LAYER_SYSTEMINCLUDE
    
    defBlock = \      
	"$${LITERAL_HASH}if defined(EABI)" 	\
	"DEFFILE  eabi/" 				\
    "$${LITERAL_HASH}else" 				\
    "DEFFILE  bwins/" 				\
    "$${LITERAL_HASH}endif"
	MMP_RULES += defBlock 
}
include(contentinfodb.pri)
