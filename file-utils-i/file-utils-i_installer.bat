@echo off
title file-utils-i Installer
cd /d "%~dp0"
echo Starting file-utils-i installer...
powershell -ExecutionPolicy Bypass -WindowStyle Normal -File "file-utils-i_installer.ps1"
pause
