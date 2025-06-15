@echo off
:: uninstall.bat - Professional uninstaller for file-utils-i

title file-utils-i Uninstaller

echo.
echo ╔══════════════════════════════════════════════════════════════╗
echo ║                    file-utils-i v0.3.0                        ║
echo ║              Quantum-Enhanced File Security                  ║
echo ║                     UNINSTALLER                              ║
echo ║                   by whispr.dev                              ║
echo ╚══════════════════════════════════════════════════════════════╝
echo.

:: Check for admin privileges
net session >nul 2>&1
if %errorLevel% == 0 (
    echo [✓] Running with administrator privileges
) else (
    echo [!] Administrator privileges recommended for complete removal
    echo     Some system-wide changes may require elevation.
    echo.
)

:: Confirmation prompt
echo This will completely remove file-utils-i from your system.
echo.
set /p "confirm=Are you sure you want to uninstall file-utils-i? (Y/N): "
if /i not "%confirm%"=="Y" (
    echo.
    echo [→] Uninstallation cancelled by user.
    goto :end
)

echo.
echo [→] Starting file-utils-i removal...

:: Define installation directory
set INSTALL_DIR=%ProgramFiles%\whispr.dev\file-utils-i

:: Remove desktop shortcut
echo [→] Removing desktop shortcuts...
if exist "%PUBLIC%\Desktop\file-utils-i.lnk" (
    del "%PUBLIC%\Desktop\file-utils-i.lnk" >nul 2>&1
    echo [✓] Removed public desktop shortcut
)
if exist "%USERPROFILE%\Desktop\file-utils-i.lnk" (
    del "%USERPROFILE%\Desktop\file-utils-i.lnk" >nul 2>&1
    echo [✓] Removed user desktop shortcut
)

:: Remove Start Menu shortcuts
echo [→] Removing Start Menu entries...
set STARTMENU=%ProgramData%\Microsoft\Windows\Start Menu\Programs\whispr.dev
if exist "%STARTMENU%" (
    rmdir /s /q "%STARTMENU%" >nul 2>&1
    echo [✓] Removed Start Menu folder
)

:: Remove user-specific Start Menu entries
set USER_STARTMENU=%APPDATA%\Microsoft\Windows\Start Menu\Programs\whispr.dev
if exist "%USER_STARTMENU%" (
    rmdir /s /q "%USER_STARTMENU%" >nul 2>&1
    echo [✓] Removed user Start Menu entries
)

:: Remove from PATH (system-wide)
echo [→] Removing from system PATH...
for /f "usebackq tokens=2*" %%A in (`reg query HKLM\SYSTEM\CurrentControlSet\Control\Session" Manager\Environment" /v PATH 2^>nul`) do set "SYSPATH=%%B"
if defined SYSPATH (
    setlocal enabledelayedexpansion
    set "NEWPATH=!SYSPATH:%INSTALL_DIR%;=!"
    set "NEWPATH=!NEWPATH:;%INSTALL_DIR%=!"
    set "NEWPATH=!NEWPATH:%INSTALL_DIR%=!"
    if not "!NEWPATH!"=="!SYSPATH!" (
        reg add "HKLM\SYSTEM\CurrentControlSet\Control\Session Manager\Environment" /v PATH /t REG_EXPAND_SZ /d "!NEWPATH!" /f >nul 2>&1
        echo [✓] Removed from system PATH
    ) else (
        echo [→] Not found in system PATH
    )
    endlocal
)

:: Remove from user PATH
echo [→] Checking user PATH...
for /f "usebackq tokens=2*" %%A in (`reg query HKCU\Environment /v PATH 2^>nul`) do set "USERPATH=%%B"
if defined USERPATH (
    setlocal enabledelayedexpansion
    set "NEWUSERPATH=!USERPATH:%INSTALL_DIR%;=!"
    set "NEWUSERPATH=!NEWUSERPATH:;%INSTALL_DIR%=!"
    set "NEWUSERPATH=!NEWUSERPATH:%INSTALL_DIR%=!"
    if not "!NEWUSERPATH!"=="!USERPATH!" (
        reg add HKCU\Environment /v PATH /t REG_EXPAND_SZ /d "!NEWUSERPATH!" /f >nul 2>&1
        echo [✓] Removed from user PATH
    ) else (
        echo [→] Not found in user PATH
    )
    endlocal
)

:: Remove registry entries (if any were created)
echo [→] Cleaning registry entries...
reg delete "HKLM\SOFTWARE\Microsoft\Windows\CurrentVersion\Uninstall\file-utils-i" /f >nul 2>&1
reg delete "HKCU\SOFTWARE\whispr.dev\file-utils-i" /f >nul 2>&1
echo [✓] Registry entries cleaned

:: Secure delete installation files
echo [→] Removing installation files...
if exist "%INSTALL_DIR%" (
    echo [→] Found installation directory: %INSTALL_DIR%
    
    :: First try to remove normally
    rmdir /s /q "%INSTALL_DIR%" >nul 2>&1
    
    :: If it still exists, try to force remove with file-utils-i itself (if available)
    if exist "%INSTALL_DIR%" (
        echo [→] Some files are stubborn, attempting secure deletion...
        if exist "%INSTALL_DIR%\file-utils.exe" (
            "%INSTALL_DIR%\file-utils-i.exe" -s "%INSTALL_DIR%\*" >nul 2>&1
        )
        :: Try removing directory again
        rmdir /s /q "%INSTALL_DIR%" >nul 2>&1
    )
    
    if exist "%INSTALL_DIR%" (
        echo [!] Warning: Some files could not be removed automatically
        echo     Directory: %INSTALL_DIR%
        echo     Please remove manually or reboot and try again.
    ) else (
        echo [✓] Installation directory removed
    )
)

:: Remove parent whispr.dev directory if empty
set PARENT_DIR=%ProgramFiles%\whispr.dev
if exist "%PARENT_DIR%" (
    rmdir "%PARENT_DIR%" >nul 2>&1
    if not exist "%PARENT_DIR%" (
        echo [✓] Removed empty whispr.dev directory
    )
)

:: Clean up temporary files
echo [→] Cleaning temporary files...
if exist "%TEMP%\file-utils-i*" (
    rmdir /s /q "%TEMP%\file-utils*" >nul 2>&1
    echo [✓] Cleaned temporary files
)

:: Remove application data (user config files, if any)
echo [→] Cleaning user data...
if exist "%APPDATA%\file-utils-i" (
    set /p "remove_data=Remove user configuration files? (Y/N): "
    if /i "!remove_data!"=="Y" (
        rmdir /s /q "%APPDATA%\file-utils-i" >nul 2>&1
        echo [✓] Removed user configuration files
    ) else (
        echo [→] Kept user configuration files
    )
)

:: Final cleanup - refresh environment
echo [→] Refreshing environment variables...
:: Broadcast WM_SETTINGCHANGE to update PATH for all processes
powershell -Command "Add-Type -TypeDefinition 'using System; using System.Runtime.InteropServices; public class Win32 { [DllImport(\"user32.dll\", SetLastError = true, CharSet = CharSet.Auto)] public static extern IntPtr SendMessageTimeout(IntPtr hWnd, uint Msg, UIntPtr wParam, string lParam, uint fuFlags, uint uTimeout, out UIntPtr lpdwResult); }'; $HWND_BROADCAST = [IntPtr]0xffff; $WM_SETTINGCHANGE = 0x1a; $result = [UIntPtr]::Zero; [Win32]::SendMessageTimeout($HWND_BROADCAST, $WM_SETTINGCHANGE, [UIntPtr]::Zero, 'Environment', 2, 5000, [ref]$result)" >nul 2>&1

echo.
echo ╔══════════════════════════════════════════════════════════════╗
echo ║                 Uninstallation Complete!                    ║
echo ╚══════════════════════════════════════════════════════════════╝
echo.
echo [✓] file-utils-i has been removed from your system
echo [✓] Shortcuts and PATH entries cleaned
echo [✓] Registry entries removed
echo [✓] Installation files deleted
echo.
echo Thank you for using file-utils-i by whispr.dev!
echo.

:end
echo Press any key to exit...
pause >nul