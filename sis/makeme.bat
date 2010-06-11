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

