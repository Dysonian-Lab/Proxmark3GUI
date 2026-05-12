@echo off
REM update_client.bat
REM Downloads the latest RRG/Iceman Proxmark3 Windows client and updates
REM the local "client" folder.
REM
REM Usage:  double-click or run from a cmd prompt:
REM           update_client.bat
REM
REM Requires: PowerShell 5+ (ships with Windows 10/11)
REM           bsdtar (shipped with Git for Windows) or 7-Zip for extraction

setlocal EnableExtensions DisableDelayedExpansion

REM ---------- configuration ----------
set "CLIENT_DIR=%~dp0client"
set "GITHUB_API=https://api.github.com/repos/RfidResearchGroup/proxmark3/releases/latest"
set "GITHUB_BASE=https://github.com/RfidResearchGroup/proxmark3/releases/download"

REM ---------- helpers ----------
:check_admin
net session >nul 2>&1
if %errorlevel% neq 0 (
    echo [WARN] Not running as administrator. Extraction may fail if UAC is on.
    echo        Right-click this file and choose "Run as administrator" if you
    echo        encounter permission errors.
)

:check_client_dir
if not exist "%CLIENT_DIR%" (
    echo [INFO] Client directory does not exist. Creating it...
    mkdir "%CLIENT_DIR%"
    if !errorlevel! neq 0 (
        echo [ERROR] Could not create "%CLIENT_DIR%". Check permissions.
        exit /b 1
    )
)

REM ---------- step 1: query GitHub API for latest release ----------
echo [INFO] Querying GitHub API for latest RRG/Iceman release...

for /f "tokens=*" %%A in ('powershell -NoLogo -NoProfile -Command ^
    "$json = (Invoke-RestMethod -Uri '%GITHUB_API%' -UseBasicParsing); ^
     $json.tag_name" 2^>^nul') do set "TAG=%%A"

if "%TAG%"=="" (
    echo [ERROR] Could not reach GitHub API. Check your internet connection.
    echo        Falling back to direct download attempt...
    goto :fallback_download
)

echo [INFO] Latest release tag: %TAG%

REM ---------- step 2: find the Windows client archive URL ----------
REM GitHub release assets for the RRG repo use names like:
REM   proxmark3-v4.21611.zip   (generic / non-RDV4 client)
REM   proxmark3-v4.21611.7z
REM We prefer the .zip since bsdtar (Git for Windows) can extract it.

set "ARCHIVE_URL="

for /f "tokens=*" %%A in ('powershell -NoLogo -NoProfile -Command ^
    "$json = (Invoke-RestMethod -Uri '%GITHUB_API%' -UseBasicParsing); ^
     $asset = $json.assets | Where-Object { $_.name -match 'proxmark3.*\.zip$' -and $_.name -notmatch 'rdv4' -and $_.name -notmatch 'blueshark' } | ^
     Select-Object -First 1; ^
     if ($asset) { $asset.browser_download_url } else { '' }" 2^>^nul') do set "ARCHIVE_URL=%%A"

if "%ARCHIVE_URL%"=="" (
    echo [WARN] No generic client .zip asset found on GitHub.
    echo        Trying any .zip asset...
    for /f "tokens=*" %%A in ('powershell -NoLogo -NoProfile -Command ^
        "$json = (Invoke-RestMethod -Uri '%GITHUB_API%' -UseBasicParsing); ^
         $asset = $json.assets | Where-Object { $_.name -match '\.zip$' } | ^
         Select-Object -First 1; ^
         if ($asset) { $browser_download_url = $asset.browser_download_url; ^
           if ($browser_download_url) { $browser_download_url } else { '' } }" 2^>^nul) do set "ARCHIVE_URL=%%A"
)

if "%ARCHIVE_URL%"=="" (
    echo [WARN] No .zip assets found on GitHub release page.
    echo        Falling back to proxmarkbuilds.org...
    goto :fallback_download
)

echo [INFO] Download URL: %ARCHIVE_URL%

REM ---------- step 3: download the archive ----------
set "TMPFILE=%TEMP%\pm3_client_update.zip"
if exist "%TMPFILE%" del /q "%TMPFILE%" 2>nul

echo [INFO] Downloading archive to %TMPFILE% ...
powershell -NoLogo -NoProfile -Command ^
    "try { ^
        [Net.ServicePointManager]::SecurityProtocol = [Net.SecurityProtocolType]::Tls12; ^
        Invoke-WebRequest -Uri '%ARCHIVE_URL%' -OutFile '%TMPFILE%' -UseBasicParsing; ^
        exit 0 ^
     } catch { ^
        Write-Host 'DOWNLOAD_FAILED:' $_.Exception.Message; ^
        exit 1 ^
     }"

if !errorlevel! neq 0 (
    echo [ERROR] Download failed.
    goto :fallback_download
)

if not exist "%TMPFILE%" (
    echo [ERROR] Downloaded file not found. Aborting.
    goto :cleanup
)

REM ---------- step 4: extract ----------
echo [INFO] Extracting archive to "%CLIENT_DIR%" ...

REM Try bsdtar first (ships with Git for Windows)
where bsdtar >nul 2>&1
if !errorlevel! equ 0 (
    bsdtar -xvf "%TMPFILE%" -C "%CLIENT_DIR%" --strip-components=0
    if !errorlevel! neq 0 (
        echo [ERROR] bsdtar extraction failed. Trying 7-Zip...
        goto :try_7z
    )
    goto :extraction_done
)

:try_7z
where 7z >nul 2>&1
if !errorlevel! equ 0 (
    7z x "%TMPFILE%" -o"%CLIENT_DIR%" -y >nul
    if !errorlevel! neq 0 (
        echo [ERROR] 7-Zip extraction failed.
        goto :cleanup
    )
    goto :extraction_done
)

REM Fallback: use PowerShell's Expand-Archive (works for .zip only)
powershell -NoLogo -NoProfile -Command ^
    "try { ^
        Expand-Archive -Path '%TMPFILE%' -DestinationPath '%CLIENT_DIR%' -Force; ^
        exit 0 ^
     } catch { ^
        Write-Host 'EXTRACT_FAILED:' $_.Exception.Message; ^
        exit 1 ^
     }"

if !errorlevel! neq 0 (
    echo [ERROR] All extraction methods failed.
    echo        Install 7-Zip or Git for Windows (which includes bsdtar).
    goto :cleanup
)

:extraction_done
echo [INFO] Extraction complete.

REM ---------- step 5: clean up temp files ----------
:cleanup
if exist "%TMPFILE%" del /q "%TMPFILE%" 2>nul

REM ---------- step 6: verify ----------
echo [INFO] Verifying client folder contents...
dir /b "%CLIENT_DIR%" 2>&1 | findstr /i "proxmark3" >nul
if !errorlevel! equ 0 (
    echo [OK] Client update completed successfully.
    echo.
    echo The following executables are now in "%CLIENT_DIR%":
    dir /b "%CLIENT_DIR%\proxmark3*.exe" 2>nul
    if !errorlevel! neq 0 (
        echo [NOTE] No proxmark3*.exe found directly. Check the client folder structure.
    )
) else (
    echo [WARN] No proxmark3 executables detected after extraction.
    echo        The archive may have placed files in a subfolder.
    echo        Check "%CLIENT_DIR%" manually.
)

goto :end

REM ---------- fallback: try proxmarkbuilds.org ----------
:fallback_download
echo [INFO] Attempting download from proxmarkbuilds.org...

REM proxmarkbuilds.org latest generic RRG client redirect
REM The site serves .7z files; we try to fetch via PowerShell
powershell -NoLogo -NoProfile -Command ^
    "try { ^
        $wc = New-Object System.Net.WebClient; ^
        [Net.ServicePointManager]::SecurityProtocol = [Net.SecurityProtocolType]::Tls12; ^
        $page = $wc.DownloadString('https://www.proxmarkbuilds.org/'); ^
        # Try to extract a download link for rrg_other or generic build ^
        $match = [regex]::Match($page, 'href=\"(/files/[^\"]*rrg[^\"]*\.(?:zip|7z))\"'); ^
        if ($match.Success) { ^
            $url = 'https://www.proxmarkbuilds.org' + $match.Groups[1].Value; ^
            Write-Host 'FOUND_URL:' $url; ^
        } else { ^
            # Fallback: try the generic/latest download pattern ^
            Write-Host 'NO_MATCH'; ^
        } ^
     } catch { ^
        Write-Host 'ERROR:' $_.Exception.Message; ^
     }" 2>&1

echo [INFO] Fallback download attempt finished. Check output above for any usable URL.
echo [INFO] You may need to visit https://www.proxmarkbuilds.org manually.

goto :end

:end
echo.
echo Done. Press any key to exit...
pause >nul
endlocal