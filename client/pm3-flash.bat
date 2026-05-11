@echo off
REM Flash Proxmark3 — pass COM port as argument
REM Usage: pm3-flash COM3 [-b] image.elf [...]
REM   -b = unlock bootloader for writing
setlocal enabledelayedexpansion

set "CLIENT=%~dp0proxmark3.exe"
if not exist "%CLIENT%" (
    echo Error: proxmark3.exe not found in %~dp0
    exit /b 1
)

if "%~1"=="" (
    echo.
    echo Usage: pm3-flash COM3 [-b] image.elf [...]
    echo.
    echo Examples:
    echo   pm3-flash COM3 -b bootrom.elf
    echo   pm3-flash COM3 fullimage.elf
    echo.
    echo Tip: Check Device Manager ^> Ports ^(COM ^& LPT^) to find your COM port.
    exit /b 0
)

set "PORT=%~1"
shift
set "ARGS=--flash --port %PORT%"

:parse
if "%~1"=="" goto done
if /i "%~1"=="-b" (
    set "ARGS=%ARGS% --unlock-bootloader"
    shift
    goto parse
)
if /i "%~1"=="--force" (
    set "ARGS=%ARGS% --force"
    shift
    goto parse
)
set "ARGS=%ARGS% --image %~1"
shift
goto parse
:done

echo.
echo Running: "%CLIENT%" %ARGS%
echo.
"%CLIENT%" %ARGS%
endlocal