ECHO OFF
CLS
if "%1" =="debug" GOTO DEBUG
if "%1" =="release" GOTO RELEASE
if "%1" =="clean_debug" GOTO CLEANDBG
if "%1" =="clean_release" GOTO CLEANRELEASE
:DEBUG
	cd ../../terralibx 
	qmake MAKEFILE=Makefile.Debug TE_PROJECT_TYPE=DEBUG
	mingw32-make -f Makefile.Debug
	
	cd ../terraView/terraViewCore
	qmake -spec win32-g++ MAKEFILE=Makefile.Debug TE_PROJECT_TYPE=DEBUG
	mingw32-make -f Makefile.Debug

	cd ../
	qmake -spec win32-g++ MAKEFILE=Makefile.Debug TE_PROJECT_TYPE=DEBUG
	mingw32-make -f Makefile.Debug
	GOTO CPDBGDEPEPENCIES

:RELEASE
	cd ../../terralibx 
	qmake -spec win32-g++ MAKEFILE=Makefile.Release
	mingw32-make -f Makefile.Release
	
	cd ../terraView/terraViewCore
	qmake -spec win32-g++ MAKEFILE=Makefile.Release
	mingw32-make -f Makefile.Release

	cd ../
	qmake -spec win32-g++ MAKEFILE=Makefile.Release
	mingw32-make -f Makefile.Release
	GOTO CPDEPEPENCIES

:CLEANDBG
	ECHO Cleaning terralib debug
	cd ../../terralibx 
	mingw32-make -f Makefile.Debug clean

	cd ../terraView/terraViewCore
	mingw32-make -f Makefile.Debug clean

	cd ../
	mingw32-make -f Makefile.Debug clean
	GOTO END

:CLEANRELEASE
	ECHO Cleaning terralib release
	cd ../../terralibx 
	mingw32-make -f Makefile.Release clean

	cd ../terraView/terraViewCore
	mingw32-make -f Makefile.Release clean

	cd ../
	mingw32-make -f Makefile.Release clean
	GOTO CPDEPENDENCIES
	
:CPDBGDEPEPENCIES	
	ECHO Copying debug win32 dependencies ...
	cd ../Debug/win32-g++
	GOTO CPDEPS
:CPDEPEPENCIES	
	ECHO Copying release win32 dependencies ...
	cd ../Release/win32-g++
:CPDEPS
	copy ..\..\terraView\windows\*.dll .
	copy %MINGWDIR%\bin\*.dll .
	copy %QTDIR%\bin\*.dll .
:END	
