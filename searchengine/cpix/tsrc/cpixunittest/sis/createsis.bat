@rem
@rem Copyright (c) 2010 Nokia Corporation and/or its subsidiary(-ies).
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

set TARGET=%1
set PLATFORM=%3
echo %TARGET%
echo %PLATFORM%

makesis cpixunittest.pkg
signsis cpixunittest.sis cpixunittest.sisx ..\..\..\s60\sis\rd.crt ..\..\..\s60\sis\rd.key

makesis cpixunittestcorpus.pkg
signsis cpixunittestcorpus.sis cpixunittestcorpus.sisx ..\..\..\s60\sis\rd.crt ..\..\..\s60\sis\rd.key

makesis CPiXUnitTestInstaller.pkg
signsis CPiXUnitTestInstaller.sis CPiXUnitTestInstaller.sisx ..\..\..\s60\sis\rd.crt ..\..\..\s60\sis\rd.key

set EPOCROOT=%FOOBAR%
