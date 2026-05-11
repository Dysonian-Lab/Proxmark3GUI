@echo off
set "HOME=%~dp0"
echo HOME = %HOME%
set "QT_PLUGIN_PATH=%HOME%\libs\"
echo QT_PLUGIN_PATH = %QT_PLUGIN_PATH%
set "QT_QPA_PLATFORM_PLUGIN_PATH=%QT_PLUGIN_PATH%"
echo QT_QPA_PLATFORM_PLUGIN_PATH = %QT_QPA_PLATFORM_PLUGIN_PATH%
set "PATH=%QT_PLUGIN_PATH%;%QT_PLUGIN_PATH%shell\;%PATH%"
echo PATH updated to include libs and shell dirs
set MSYSTEM=MINGW64
echo MSYSTEM = %MSYSTEM%
echo Environment setup complete for Proxmark3GUI
