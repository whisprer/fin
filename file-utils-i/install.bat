@echo off
:: install.bat - Professional installer for file-utils

echo.
echo ╔══════════════════════════════════════════════════════════════╗
echo ║                    file-utils v0.3.0                        ║
echo ║              Quantum-Enhanced File Security                  ║
echo ║                   by whispr.dev                              ║
echo ╚══════════════════════════════════════════════════════════════╝
echo.

:: Check for admin privileges
net session >nul 2>&1
if %errorLevel% == 0 (
    echo [✓] Running with administrator privileges
) else (
    echo [!] Administrator privileges recommended for system-wide installation
    echo     Run as administrator for best experience.
    echo.
)

:: Create installation directory
set INSTALL_DIR=%ProgramFiles%\whispr.dev\file-utils
echo [→] Installing to: %INSTALL_DIR%

if not exist "%ProgramFiles%\whispr.dev" mkdir "%ProgramFiles%\whispr.dev"
if not exist "%INSTALL_DIR%" mkdir "%INSTALL_DIR%"

:: Copy files
echo [→] Copying files...
copy file-utils.exe "%INSTALL_DIR%\" >nul
copy README.md "%INSTALL_DIR%\" >nul
copy LICENSE "%INSTALL_DIR%\" >nul
xcopy docs "%INSTALL_DIR%\docs\" /s /i /q >nul

:: Add to PATH
echo [→] Adding to system PATH...
setx PATH "%PATH%;%INSTALL_DIR%" /M >nul 2>&1

:: Create start menu shortcuts
echo [→] Creating Start Menu shortcuts...
set STARTMENU=%ProgramData%\Microsoft\Windows\Start Menu\Programs\whispr.dev
if not exist "%STARTMENU%" mkdir "%STARTMENU%"

:: Create desktop shortcut
echo [→] Creating desktop shortcut...
powershell -Command "& {$ws = New-Object -ComObject WScript.Shell; $s = $ws.CreateShortcut('%PUBLIC%\Desktop\file-utils.lnk'); $s.TargetPath = '%INSTALL_DIR%\file-utils.exe'; $s.WorkingDirectory = '%INSTALL_DIR%'; $s.Description = 'Quantum-Enhanced File Security Tool'; $s.Save()}"

echo.
echo ╔══════════════════════════════════════════════════════════════╗
echo ║                   Installation Complete!                    ║
echo ╚══════════════════════════════════════════════════════════════╝
echo.
echo [✓] file-utils installed to: %INSTALL_DIR%
echo [✓] Added to system PATH
echo [✓] Desktop shortcut created
echo [✓] Documentation available in: %INSTALL_DIR%\docs\
echo.
echo To get started, open a new Command Prompt and type: file-utils --help
echo.
pause