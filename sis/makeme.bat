@rem
@rem Copyright (c) 2009 Nokia Corporation and/or its subsidiary(-ies).
@rem All rights reserved.
@rem This component and the accompanying materials are made available
@rem under the terms of "Eclipse Public License v1.0"
@rem which accompanies this distribution, and is available
@rem at the URL "http://www.eclipse.org/legal/epl-v10.html".
@rem
@rem Initial Contributors:
@rem Nokia Corporation - initial contribution.
@rem
@rem Contributors:
@rem
@rem Description: 
@rem
REM echo %EPOCROOT%
set FOOBAR=%EPOCROOT%
REM echo %FOOBAR%
del centrep.sis
del centrep.sisx
makesis centrep.pkg centrep.sis
signsis centrep.sis centrep.sisx rd.der rd.key

REM Generate WatchDog.sisx
cd ..\WatchDog\sis\
call makeWatchDog.bat %1 %EPOCROOT%
cd ..\..\sis\

REM Generate CPiXSearch.sisx
del CPiXSearch.pkg
perl makeMe.pl %1 %EPOCROOT%
del CPiXSearch.sis
del CPiXSearch.sisx
makesis CPiXSearch.pkg CPiXSearch.sis
signsis CPiXSearch.sis CPiXSearch.sisx rd.der rd.key
REM echo %EPOCROOT%
set EPOCROOT=%FOOBAR%
REM echo %EPOCROOT%

