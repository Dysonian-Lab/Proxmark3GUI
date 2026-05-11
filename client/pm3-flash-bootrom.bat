@echo off
REM Flash bootloader only — pass COM port as argument
REM Usage: pm3-flash-bootrom COM3 [--force]
setlocal enabledelayedexpansion

set "CLIENT=%~dp0proxmark3.exe"
set "BOOT=%~dp0bootrom.elf"

if not exist "%CLIENT%" (
    echo Error: proxmark3.exe not found in %~dp0
    exit /b 1
)
if not exist "%BOOT%" (
    echo Error: bootrom.elf not found in %~dp0
    exit /b 1
)

if "%~1"=="" (
    echo.
    echo Usage: pm3-flash-bootrom COM3 [--force]
    echo.
    echo Tip: Check Device Manager ^> Ports ^(COM ^& LPT^) to find your COM port.
    exit /b 0
)

set "PORT=%~1"
shift

echo.
echo =============================================
echo  WARNING: This will flash the BOOTLOADER.
echo  Only do this if you know what you're doing!
echo =============================================
echo.
echo COM Port    : %PORT%
echo Bootloader  : %BOOT%
echo.

set "ARGS=--flash --unlock-bootloader --port %PORT% --image "%BOOT%""
if /i "%~1"=="--force" set "ARGS=%ARGS% --force"

echo Running: "%CLIENT%" %ARGS%
echo.
"%CLIENT%" %ARGS%
endlocal