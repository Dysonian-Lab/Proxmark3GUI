@echo off
REM Flash bootloader + full firmware — pass COM port as argument
REM Usage: pm3-flash-all COM3 [--force]
setlocal enabledelayedexpansion

set "CLIENT=%~dp0proxmark3.exe"
set "BOOT=%~dp0bootrom.elf"
set "FULL=%~dp0fullimage.elf"

if not exist "%CLIENT%" (
    echo Error: proxmark3.exe not found in %~dp0
    exit /b 1
)
if not exist "%BOOT%" (
    echo Error: bootrom.elf not found in %~dp0
    exit /b 1
)
if not exist "%FULL%" (
    echo Error: fullimage.elf not found in %~dp0
    exit /b 1
)

if "%~1"=="" (
    echo.
    echo Usage: pm3-flash-all COM3 [--force]
    echo.
    echo This flashes BOTH bootloader and full firmware.
    echo Warning: ensure your device supports this firmware version!
    echo.
    echo Tip: Check Device Manager ^> Ports ^(COM ^& LPT^) to find your COM port.
    exit /b 0
)

set "PORT=%~1"
shift

echo.
echo =============================================
echo  WARNING: This will flash BOTH bootloader
echo  and full firmware. Ensure your device
echo  supports v4.21611 firmware!
echo =============================================
echo.
echo COM Port    : %PORT%
echo Bootloader  : %BOOT%
echo Firmware    : %FULL%
echo.

set "ARGS=--flash --unlock-bootloader --port %PORT% --image "%BOOT%" --image "%FULL%""
if /i "%~1"=="--force" set "ARGS=%ARGS% --force"

echo Running: "%CLIENT%" %ARGS%
echo.
"%CLIENT%" %ARGS%
endlocal