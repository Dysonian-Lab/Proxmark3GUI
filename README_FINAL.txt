================================================================================
                      Proxmark3GUI FINAL DELIVERABLE
                          RRG/Iceman v4.21611 Client
================================================================================

PACKAGE CONTENTS
----------------
This deliverable contains the complete Proxmark3GUI distribution with the
updated RRG/Iceman v4.21611 client firmware and all supporting files.

The following directories are included:

1. GUI/
   - Proxmark3GUI.exe (Windows GUI launcher)
   - All required Qt5 runtime DLLs (Qt5Core.dll, Qt5Gui.dll, Qt5Widgets.dll, etc.)
   - Additional runtime libraries (libgcc_s_seh-1.dll, libstdc++-6.dll, libwinpthread-1.dll)
   - plugins/ directory with Qt platform plugins
   - GUIsettings.ini (configuration file)

2. client/
   - proxmark3.exe (RRG/Iceman v4.21611 client binary)
   - Firmware files:
     * bootrom.elf
     * fullimage.elf
   - Flash utility scripts:
     * pm3, pm3-flash, pm3-flash-all, pm3-flash-bootrom, pm3-flash-fullimage
   - All required runtime DLLs (Qt5Core.dll, Qt5Gui.dll, Qt5Network.dll, etc.)
   - Lua runtime (lua55.dll) and Tcl/Tk libraries (tcl86.dll, tk86.dll)
   - Python 3.14 runtime (libpython3.14.dll, libpython3.dll)
   - Standard C/C++ runtime libraries (libgcc, libstdc++, libwinpthread)
   - Third-party library dependencies (libssl, libcrypto, libcrypto-3-x64.dll, etc.)
   - Subdirectories:
     * cmdscripts/    - Command scripts
     * dictionaries/   - Dictionary files
     * libs/          - Library files
     * lualibs/       - Lua libraries
     * luascripts/    - Lua scripts
     * resources/     - Resource files
     * tmp/           - Temporary working directory

3. SourceCode/
   - Complete Proxmark3GUI source code
   - Configuration files in config/ (including config_rrgv4.16717.json, config_rrgv4.15864.json, etc.)
   - Internationalization files in i18n/
   - UI source files in src/ui/
   - Documentation in doc/
   - Deployment tools in deploy/


HOW TO RUN THE GUI
------------------
IMPORTANT: Before launching Proxmark3GUI.exe for the first time, run setup.bat
from the client folder. This configures the Qt plugin paths required for the
GUI to load properly.

1. Ensure your Proxmark3 device is connected via USB.

2. Open a terminal in the client folder and run setup.bat:
   > cd "D:\kilocode\PM3\Proxmark3_Final_Updated\client"
   > setup.bat

3. Navigate to the GUI directory: D:\kilocode\PM3\Proxmark3_Final_Updated\GUI\

4. Launch the GUI:
   Double-click: Proxmark3GUI.exe
   OR from command line:
   > cd "D:\kilocode\PM3\Proxmark3_Final_Updated\GUI"
   > Proxmark3GUI.exe

5. The GUI will start and attempt to connect to the Proxmark3 device using
   the client binary from the client/ directory.

6. If prompted for client path, browse to:
   D:\kilocode\PM3\Proxmark3_Final_Updated\client\proxmark3.exe

7. Configure the COM port if automatic detection fails (typically COM3 or COM4).


HOW TO USE UPDATE SCRIPTS
--------------------------
The client directory includes flash utility scripts for updating firmware:

Flash scripts (.bat) are for use from Windows CMD or PowerShell.
Flash scripts (.sh) are for use from MSYS2/WSL/Linux bash only.

The .bat scripts require the COM port to be passed as an argument (e.g., `pm3-flash-all.bat COM3`). Check Device Manager > Ports (COM & LPT) to find your COM port.

1. Flash all firmware (bootrom + fullimage):
   > cd "D:\kilocode\PM3\Proxmark3_Final_Updated\client"
   > pm3-flash-all.bat COM3

2. Flash bootrom only:
   > pm3-flash-bootrom.bat COM3

3. Flash full image only:
   > pm3-flash-fullimage.bat COM3

4. Manual flash using pm3 utility:
   > pm3 flash -b bootrom.elf
   > pm3 flash -f fullimage.elf

Note: For bootrom flashing, hold the PM3 button while plugging in USB to enter bootloader mode.


VERSION INFORMATION
-------------------
Client Firmware Version: RRG/Iceman v4.21611
- Based on Iceman fork (iceman1001)
- Includes RRG (Real Russian Guild) modifications
- Changelog: [BREAKMEIFYOUCAN!] - 2026-04-14 release
  * Fixed hf mf wrbl and hf mfp wrbl ACL RO checks on 16-block sectors
  * Added hf mfp acl command
  * Added hf mfdes brutedamslot and getdelegateappinfo commands
  * Added hf secc info command
  * Added hf secc simulation support
  * Added lf relay command for device-to-device communication
  * Many improvements to hf iclass legbrute performance
  * And many more fixes and enhancements

GUI Version: Proxmark3GUI (Qt-based)
- Qt 5.15.2 runtime
- qdarkstyle dark theme support
- Multi-language support (English, Chinese)


BUILD DATE
----------
Package assembled: 2026-05-10
Delivered from: D:\kilocode\PM3\Proxmark3GUI_Updated\
Client binary timestamp: 2026-05-10 09:33:39 AM


REQUIREMENTS
------------
For a fresh Windows 10 install:

- Windows 10/11 (64-bit, version 1903 or later)
- Proxmark3 RDV4.0 or compatible hardware (e.g., Proxmark3 Easy, Proxmark3 RDV4)
- USB-A to Micro-B cable (for device connection)
- USB device drivers (usually auto-install on first connect; if not, use Zadig tool to install WinUSB driver)
- NO additional software installation required — all runtime DLLs (Qt5, MSVC, Python, Lua, OpenSSL, libusb) are pre-bundled in the client/ folder
- For flashing via batch files: Windows PowerShell 5+ or CMD (both are already included in Windows 10)


TROUBLESHOOTING
---------------
1. "Cannot find proxmark3.exe" - Ensure client/proxmark3.exe exists and has
   not been blocked by Windows Defender (right-click -> Properties -> Unblock).

2. "DLL load failed" - Ensure all DLL files in client/ are present and not
   blocked by security software.

3. Device not detected - Check USB cable, ensure drivers installed, verify
   COM port in Device Manager.

4. Flash failures - Put device in bootloader mode (hold button while plugging
   USB) before running flash scripts.

5. Windows Defender may quarantine proxmark3.exe — right-click -> Properties -> Unblock or add exception.

6. If GUI shows 'Cannot find client', ensure setup.bat was run and that client\proxmark3.exe exists.

7. Flash script says 'COM port not specified' — Pass your COM port as an argument.
   Example: `pm3-flash-all.bat COM3`. Find your COM port in Device Manager >
   Ports (COM & LPT).


LICENSE
-------
Proxmark3GUI and client firmware are open-source projects.
See SourceCode/LICENSE for details.


================================================================================
                          END OF DELIVERABLE PACKAGE
================================================================================