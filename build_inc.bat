@echo off
setlocal enabledelayedexpansion
cd /D "%~dp0"

set CONF=%1
set REL="Release"
set "BUILD=1000"
set "TOTAL=1000"

(set /p BUILD=<build_num.txt)2>nul
(set /p TOTAL=<build_num_total.txt)2>nul

set /A "BUILD+=1"
set /A "TOTAL+=1"

>build_num_total.txt echo %TOTAL%

echo Build: %TOTAL%

if "%CONF%"=="%REL%" (
	>build_num.txt echo %TOTAL%
	powershell "$(Get-Item Core/Tera/Core.h).lastwritetime=$(Get-Date)"
)
exit 0