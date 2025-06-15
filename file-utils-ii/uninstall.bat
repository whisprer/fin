@echo off
echo ===================================
echo Proc-Wolf Uninstallation
echo ===================================
echo.

:: Check for administrator privileges
net session >nul 2>&1
if %errorLevel% neq 0 (
    echo Error: This script must be run as administrator!
    echo Please right-click and select "Run as administrator"
    pause
    exit /B 1
)

echo Stopping Proc-Wolf Service...
net stop "ProcWolfService" 2>nul
echo.
echo Removing Proc-Wolf Service...
"%~dp0ProcWolfService.exe" remove
echo.
echo Uninstallation complete!
echo.
pause
