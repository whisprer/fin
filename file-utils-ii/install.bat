@echo off
echo ===================================
echo Proc-Wolf Installation
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

echo Installing Proc-Wolf Service...
echo.

:: Stop the service if it's already running
echo Checking if service already exists...
sc query "ProcWolfService" >nul 2>&1
if %errorLevel% equ 0 (
    echo Found existing service, stopping it first...
    net stop "ProcWolfService" >nul 2>&1
    timeout /t 2 /nobreak >nul
)

:: Remove existing service if it exists
sc query "ProcWolfService" >nul 2>&1
if %errorLevel% equ 0 (
    echo Removing existing service...
    "%~dp0ProcWolfService.exe" remove
    timeout /t 2 /nobreak >nul
)

:: Install the service
echo Installing service...
"%~dp0ProcWolfService.exe" --startup auto install

:: Start the service
echo Starting Proc-Wolf Service...
net start "ProcWolfService"

if %errorLevel% equ 0 (
    echo.
    echo ===================================
    echo Installation successful!
    echo ===================================
    echo.
    echo The Proc-Wolf service is now running in the background.
    echo.
    echo To use the system tray monitor: Run ProcWolf.exe
    echo To use the CLI commands: Run ProcWolfCLI.exe
    echo.
    echo Log files are stored in:
    echo - Service logs: C:\ProgramData\proc-wolf    echo - Client logs: Same folder as the executable
    echo.
) else (
    echo.
    echo ===================================
    echo Installation had issues
    echo ===================================
    echo.
    echo The service might not have started correctly.
    echo Check the logs for more information.
    echo.
)

pause
