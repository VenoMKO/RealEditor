@echo off
setlocal enableextensions
cd /D "%~dp0"

set PUBLISH_DIR=Build\Release\Published
set PDB_DIR=%PUBLISH_DIR%\PDB
set EXE_DIR=%PUBLISH_DIR%

set "BUILD=-1"
(set /p BUILD=<build_num.txt)2>nul
IF %BUILD% EQU -1 (
	echo Error. Build number is missing.
	pause
	exit 1
)

start /wait "" "Build\Release\Real Editor.exe" "-build"
(set /p TEST_BUILD=<app_build.txt)2>nul
del app_build.txt
if %TEST_BUILD% NEQ %BUILD% (
	echo Error. App build does not match. Re-build the solution.
	pause
	exit 1
)

set "VERSION=-1"
start /wait "" "Build\Release\Real Editor.exe" "-version"
(set /p VERSION=<app_ver.txt)2>nul
del app_ver.txt
IF %VERSION% EQU -1 (
	echo Error. Failed to get app version.
	pause
	exit 1
)

IF EXIST %PDB_DIR%\%BUILD% (
	echo Error. The build %BUILD% was published already. Can't publish the same build twice.
	pause
	exit 1
)

IF EXIST %EXE_DIR%\RE_%VERSION%.zip (
	echo Error. The version %VERSION% was published already. Can't publish the same version twice.
	pause
	exit 1
)

md "%EXE_DIR%\%VERSION%\Real Editor"

md "%PDB_DIR%\%BUILD%"
copy /b "Build\Release\Real Editor.pdb" "%PDB_DIR%\%BUILD%\Real Editor.pdb">nul 
copy /b "Build\Release\Real Editor.exe" "%EXE_DIR%\%VERSION%\Real Editor\Real Editor.exe">nul
powershell -ExecutionPolicy Bypass -Command "& {&'Compress-Archive' \"%EXE_DIR%\%VERSION%\Real Editor\" \"%EXE_DIR%\RE_%VERSION%.zip\"}";
rd /s /q "%EXE_DIR%\%VERSION%\"
echo Created RE_%VERSION%.zip(Build: %BUILD%)
pause