@echo off
REM Flash firmware image only (no bootloader) — pass COM port as argument
REM Usage: pm3-flash-fullimage COM3 [--force]
setlocal enabledelayedexpansion

set "CLIENT=%~dp0proxmark3.exe"
set "FULL=%~dp0fullimage.elf"

if not exist "%CLIENT%" (
    echo Error: proxmark3.exe not found in %~dp0
    exit /b 1
)
if not exist "%FULL%" (
    echo Error: fullimage.elf not found in %~dp0
    exit /b 1
)

if "%~1"=="" (
    echo.
    echo Usage: pm3-flash-fullimage COM3 [--force]
    echo.
    echo Tip: Check Device Manager ^> Ports ^(COM ^& LPT^) to find your COM port.
    exit /b 0
)

set "PORT=%~1"
shift

echo.
echo COM Port    : %PORT%
echo Firmware    : %FULL%
echo.

set "ARGS=--flash --port %PORT% --image "%FULL%""
if /i "%~1"=="--force" set "ARGS=%ARGS% --force"

echo Running: "%CLIENT%" %ARGS%
echo.
"%CLIENT%" %ARGS%
endlocal