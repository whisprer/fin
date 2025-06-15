# make_installer.ps1 - Shows what's happening!

param(
    [switch]$Verbose,
    [switch]$Debug
)

# Enable verbose output if requested
if ($Verbose) {
    $VerbosePreference = 'Continue'
}

if ($Debug) {
    $DebugPreference = 'Continue'
    $VerbosePreference = 'Continue'
}

# ALWAYS show basic progress
$ErrorActionPreference = 'Continue'  # Don't stop on errors, show them

Write-Host "=== make_installer.ps1 Starting ===" -ForegroundColor Cyan
Write-Host "Current directory: $(Get-Location)" -ForegroundColor Gray
Write-Host "PowerShell version: $($PSVersionTable.PSVersion)" -ForegroundColor Gray
Write-Host ""

####################
# STEP 0: Check Prerequisites
####################

Write-Host "STEP 0: Checking prerequisites..." -ForegroundColor Yellow

$requiredFiles = @(
    "7za.exe",
    "file-utils-i.exe", 
    "install.bat",
    "UNWISE.bat",
    "README",
    "LICENSE"
    "docs\"
)

Write-Host "Required files check:" -ForegroundColor Gray
$missingFiles = @()
foreach ($file in $requiredFiles) {
    if (Test-Path $file) {
        $size = [math]::Round((Get-Item $file).Length / 1024, 1)
        Write-Host "  Found: $file ($size KB)" -ForegroundColor Green
    } else {
        Write-Host "  Missing: $file" -ForegroundColor Red
        $missingFiles += $file
    }
}

if ($missingFiles.Count -gt 0) {
    Write-Host ""
    Write-Host "ERROR: Missing required files:" -ForegroundColor Red
    $missingFiles | ForEach-Object { Write-Host "  - $_" -ForegroundColor Red }
    Write-Host ""
    Write-Host "Current directory contents:" -ForegroundColor Yellow
    Get-ChildItem | ForEach-Object { Write-Host "  $($_.Name)" -ForegroundColor White }
    Read-Host "Press Enter to exit"
    exit 1
}

Write-Host "All required files found!" -ForegroundColor Green

####################
# STEP 1: Create Archive
####################

Write-Host ""
Write-Host "STEP 1: Creating 7z archive..." -ForegroundColor Yellow

# Clean up old archive
if (Test-Path "archive.7z") {
    Write-Host "Removing existing archive.7z..." -ForegroundColor Gray
    Remove-Item "archive.7z" -Force
}

try {
    Write-Host "Running 7za.exe command..." -ForegroundColor Gray
    Write-Host "Command: .\7za.exe a -t7z archive.7z file-utils-i.exe README LICENSE docs\ install.bat UNWISE.bat" -ForegroundColor Gray
    
    # Run 7za and capture output
    $process = Start-Process -FilePath ".\7za.exe" `
        -ArgumentList "a", "-t7z", "archive.7z", "file-utils-i.exe", "README", "LICENSE", "docs\", "install.bat", "UNWISE.bat" `
        -Wait -NoNewWindow -PassThru -RedirectStandardOutput "7za_output.txt" -RedirectStandardError "7za_error.txt"
    
    # Show output
    if (Test-Path "7za_output.txt") {
        Get-Content "7za_output.txt" | ForEach-Object { Write-Host $_ -ForegroundColor Gray }
        Remove-Item "7za_output.txt" -Force
    }
    
    if (Test-Path "7za_error.txt") {
        $errorContent = Get-Content "7za_error.txt"
        if ($errorContent) {
            $errorContent | ForEach-Object { Write-Host $_ -ForegroundColor Red }
        }
        Remove-Item "7za_error.txt" -Force
    }
    
    Write-Host "7za.exe exit code: $($process.ExitCode)" -ForegroundColor Gray
    
    if ($process.ExitCode -eq 0) {
        if (Test-Path "archive.7z") {
            $archiveSize = [math]::Round((Get-Item "archive.7z").Length / 1024, 1)
            Write-Host "Created archive.7z ($archiveSize KB)" -ForegroundColor Green
        } else {
            throw "archive.7z was not created despite exit code 0"
        }
    } else {
        throw "7za.exe failed with exit code $($process.ExitCode)"
    }
} catch {
    Write-Host "Failed to create archive: $($_.Exception.Message)" -ForegroundColor Red
    Write-Host ""
    Write-Host "Debugging info:" -ForegroundColor Yellow
    Write-Host "  Working directory: $(Get-Location)"
    Write-Host "  7za.exe exists: $(Test-Path '.\7za.exe')"
    if (Test-Path ".\7za.exe") {
        Write-Host "  7za.exe size: $([math]::Round((Get-Item '.\7za.exe').Length / 1024, 1)) KB"
    }
    Read-Host "Press Enter to exit"
    exit 1
}

####################
# STEP 2: Check/Install ps2exe
####################

Write-Host ""
Write-Host "STEP 2: Checking ps2exe..." -ForegroundColor Yellow

try {
    $ps2exeModule = Get-Module -ListAvailable -Name ps2exe
    if ($ps2exeModule) {
        Write-Host "ps2exe already available (version: $($ps2exeModule.Version))" -ForegroundColor Green
    } else {
        Write-Host "Installing ps2exe module..." -ForegroundColor Yellow
        
        # For PowerShell 7, we need to handle NuGet provider differently
        try {
            Write-Host "  Installing NuGet provider..." -ForegroundColor Gray
            Install-PackageProvider -Name NuGet -MinimumVersion 2.8.5.201 -Force -Scope CurrentUser | Out-Null
        } catch {
            Write-Host "  NuGet provider already available or using PowerShell 7+" -ForegroundColor Gray
        }
        
        Write-Host "  Setting PSGallery as trusted..." -ForegroundColor Gray
        Set-PSRepository -Name 'PSGallery' -InstallationPolicy Trusted
        
        Write-Host "  Installing ps2exe module..." -ForegroundColor Gray
        Install-Module ps2exe -Scope CurrentUser -Force -AllowClobber
        
        Write-Host "ps2exe installed successfully" -ForegroundColor Green
    }
} catch {
    Write-Host "Failed to install ps2exe: $($_.Exception.Message)" -ForegroundColor Red
    Write-Host "  You can manually install with: Install-Module ps2exe -Scope CurrentUser -Force" -ForegroundColor Yellow
    Read-Host "Press Enter to exit"
    exit 1
}

####################
# STEP 3: Create PowerShell Installer
####################

Write-Host ""
Write-Host "STEP 3: Creating PowerShell installer..." -ForegroundColor Yellow

try {
    # Use full paths to avoid working directory issues
    $archivePath = Join-Path -Path (Get-Location) -ChildPath "archive.7z"
    $7zaPath = Join-Path -Path (Get-Location) -ChildPath "7za.exe"
    
    Write-Host "Reading archive.7z from: $archivePath" -ForegroundColor Gray
    if (-not (Test-Path $archivePath)) {
        throw "archive.7z not found at: $archivePath"
    }
    
    $archiveBytes = [System.IO.File]::ReadAllBytes($archivePath)
    $base64Archive = [Convert]::ToBase64String($archiveBytes)
    
    Write-Host "Reading 7za.exe from: $7zaPath" -ForegroundColor Gray
    if (-not (Test-Path $7zaPath)) {
        throw "7za.exe not found at: $7zaPath"
    }
    
    $7zaBytes = [System.IO.File]::ReadAllBytes($7zaPath)
    $base647za = [Convert]::ToBase64String($7zaBytes)
    
    $archiveKB = [math]::Round($archiveBytes.Length / 1024, 1)
    $7zaKB = [math]::Round($7zaBytes.Length / 1024, 1)
    $base64SizeKB = [math]::Round(($base64Archive.Length + $base647za.Length) / 1024, 1)
    
    Write-Host "  Archive: $archiveKB KB -> Base64: $([math]::Round($base64Archive.Length / 1024, 1)) KB" -ForegroundColor Gray
    Write-Host "  7za.exe: $7zaKB KB -> Base64: $([math]::Round($base647za.Length / 1024, 1)) KB" -ForegroundColor Gray
    Write-Host "  Total embedded data: $base64SizeKB KB" -ForegroundColor Cyan
    
    # Create installer content

$installerContent = @"
# file-utils-i Self-Extracting Installer v0.4.0
# Generated by make_installer.ps1 - Quantum-Enhanced File Security

`$ProgressPreference = 'SilentlyContinue'
`$ErrorActionPreference = 'Stop'

Write-Host ""
Write-Host "=================================================================" -ForegroundColor Cyan
Write-Host "                    file-utils-i v0.4.0                         " -ForegroundColor Cyan
Write-Host "              Quantum-Enhanced File Security                    " -ForegroundColor Cyan
Write-Host "                   by whispr.dev                               " -ForegroundColor Cyan
Write-Host "=================================================================" -ForegroundColor Cyan
Write-Host ""
Write-Host "PROCWOLF-Enhanced Secure File Operations" -ForegroundColor Yellow
Write-Host "   - Secure deletion with stubborn file handling" -ForegroundColor White
Write-Host "   - Process lock detection and termination" -ForegroundColor White
Write-Host "   - Advanced Windows file attribute management" -ForegroundColor White
Write-Host "   - Multi-pass cryptographic overwriting" -ForegroundColor White
Write-Host ""

# Check for admin privileges
`$currentPrincipal = New-Object Security.Principal.WindowsPrincipal([Security.Principal.WindowsIdentity]::GetCurrent())
`$isAdmin = `$currentPrincipal.IsInRole([Security.Principal.WindowsBuiltInRole]::Administrator)

if (-not `$isAdmin) {
    Write-Host "WARNING: Not running as Administrator" -ForegroundColor Yellow
    Write-Host "   Some PROCWOLF features require admin privileges" -ForegroundColor White
    Write-Host ""
}

# Installation confirmation
`$response = Read-Host "Install file-utils-i? [Y/n]"
if (`$response -match "^[Nn]") {
    Write-Host "Installation cancelled by user." -ForegroundColor Yellow
    Read-Host "Press Enter to exit"
    exit 0
}

Write-Host ""
Write-Host "Starting installation..." -ForegroundColor Green

# Create secure temporary directory
`$tempDir = "`$env:TEMP\file-utils-i_install_`$(Get-Random -Minimum 10000 -Maximum 99999)"
Write-Host "Creating temporary directory: `$tempDir" -ForegroundColor Gray

try {
    New-Item -ItemType Directory -Path `$tempDir -Force | Out-Null
    
    Write-Host "Extracting embedded files..." -ForegroundColor Yellow
    
    # Extract 7za.exe (embedded as base64)
    Write-Host "  Extracting 7za.exe ($7zaKB KB)..." -ForegroundColor Gray
    `$7zaData = "$base647za"
    `$7zaBytes = [Convert]::FromBase64String(`$7zaData)
    [System.IO.File]::WriteAllBytes("`$tempDir\7za.exe", `$7zaBytes)
    
    # Verify 7za.exe
    if (-not (Test-Path "`$tempDir\7za.exe")) {
        throw "Failed to extract 7za.exe"
    }
    
    # Extract main archive (embedded as base64)
    Write-Host "  Extracting package archive ($archiveKB KB)..." -ForegroundColor Gray
    `$archiveData = "$base64Archive"
    `$archiveBytes = [Convert]::FromBase64String(`$archiveData)
    [System.IO.File]::WriteAllBytes("`$tempDir\package.7z", `$archiveBytes)
    
    # Verify archive
    if (-not (Test-Path "`$tempDir\package.7z")) {
        throw "Failed to extract package archive"
    }
    
    Write-Host "Embedded files extracted successfully" -ForegroundColor Green
    
    # Extract the main package
    Write-Host "Unpacking installation files..." -ForegroundColor Yellow
    Push-Location `$tempDir
    
    try {
        `$extractResult = & ".\7za.exe" x "package.7z" -y 2>&1
        
        if (`$LASTEXITCODE -ne 0) {
            Write-Host "7za.exe output:" -ForegroundColor Red
            `$extractResult | ForEach-Object { Write-Host "  `$_" -ForegroundColor Red }
            throw "Failed to extract package (exit code: `$LASTEXITCODE)"
        }
        
        Write-Host "Package extracted successfully" -ForegroundColor Green
        
        # List extracted files for verification
        Write-Host "  Extracted files:" -ForegroundColor Gray
        Get-ChildItem | Where-Object { `$_.Name -ne "7za.exe" -and `$_.Name -ne "package.7z" } | 
            ForEach-Object { 
                `$size = if (`$_.Length -gt 1KB) { "[`$([math]::Round(`$_.Length / 1KB, 1)) KB]" } else { "[`$(`$_.Length) B]" }
                Write-Host "    `$(`$_.Name) `$size" -ForegroundColor White 
            }
        
    } finally {
        Pop-Location
    }
    
    # Run the installation batch file
    if (Test-Path "`$tempDir\install.bat") {
        Write-Host ""
        Write-Host "Running installation script..." -ForegroundColor Yellow
        
        Push-Location `$tempDir
        try {
            # Run install.bat and capture output
            `$installOutput = & cmd.exe /c "install.bat" 2>&1
            `$installExitCode = `$LASTEXITCODE
            
            # Show install output
            if (`$installOutput) {
                Write-Host "Installation output:" -ForegroundColor Gray
                `$installOutput | ForEach-Object { Write-Host "  `$_" -ForegroundColor White }
            }
            
            if (`$installExitCode -eq 0) {
                Write-Host "Installation script completed successfully" -ForegroundColor Green
            } else {
                Write-Host "Installation script exited with code: `$installExitCode" -ForegroundColor Yellow
                Write-Host " Installation may have completed with warnings" -ForegroundColor Gray
            }
            
        } finally {
            Pop-Location
        }
    } else {
        Write-Host " install.bat not found in package" -ForegroundColor Red
        Write-Host " Manual installation may be required" -ForegroundColor Yellow
    }
    
    # Verify installation
    Write-Host ""
    Write-Host "Verifying installation..." -ForegroundColor Yellow
    
    # Check if file-utils-i.exe is in PATH or standard locations
    `$fileUtilsPaths = @(
    `$$exePath = (Get-Command "file-utils-i.exe" -ErrorAction SilentlyContinue | Select-Object -ExpandProperty Source -ErrorAction SilentlyContinue)
    `$$paths = @(
        `$$exePath
    # "`$env:whispr-dev-x64\file-utils-i\file-utils-i.exe",
        "`$env:whispr-dev-x86\file-utils-i\file-utils-i.exe",
        "`$env:LOCALAPPDATA\whispr-dev\file-utils-i\file-utils-i.exe",
        "C:\tools\file-utils-i.exe"
    ) | Where-Object { `$_ -and (Test-Path `$_) }
    
    if (`$fileUtilsiPaths) {
        Write-Host "file-utils-i.exe found at: `$(`$fileUtilsiPaths[0])" -ForegroundColor Green
        
        # Try to get version info
        try {
            `$versionInfo = & `$fileUtilsiPaths[0] --version 2>&1
            if (`$versionInfo) {
                Write-Host "  Version: `$versionInfo" -ForegroundColor Cyan
            }
        } catch {
            Write-Host "  Version check failed, but executable exists" -ForegroundColor Gray
        }
        
    } else {
        Write-Host "file-utils-i.exe not found in standard locations" -ForegroundColor Yellow
        Write-Host "   You may need to add it to your PATH manually" -ForegroundColor Gray
    }
    
    Write-Host ""
    Write-Host "Installation completed successfully!" -ForegroundColor Green
    Write-Host ""
    Write-Host "Next steps:" -ForegroundColor Cyan
    Write-Host "  1. Open a new command prompt or PowerShell window" -ForegroundColor White
    Write-Host "  2. Try: file-utils-i --help" -ForegroundColor White
    Write-Host "  3. For PROCWOLF features, run as Administrator" -ForegroundColor White
    Write-Host ""
    Write-Host "PROCWOLF ready for deployment!" -ForegroundColor Yellow
    
} catch {
    Write-Host ""
    Write-Host "Installation failed: `$(`$_.Exception.Message)" -ForegroundColor Red
    Write-Host ""
    Write-Host "Troubleshooting:" -ForegroundColor Yellow
    Write-Host "  - Ensure you're running as Administrator for full features" -ForegroundColor White
    Write-Host "  - Check antivirus isn't blocking the installation" -ForegroundColor White
    Write-Host "  - Verify you have write permissions to installation directory" -ForegroundColor White
    Write-Host ""
    Write-Host "For support, visit: https://whispr.dev/support" -ForegroundColor Cyan
    Write-Host ""
    
    exit 1
    
} finally {
    # Cleanup temporary directory
    Write-Host "Cleaning up temporary files..." -ForegroundColor Gray
    
    if (Test-Path `$tempDir) {
        try {
            # Sometimes files are still locked, so retry cleanup
            for (`$i = 1; `$i -le 3; `$i++) {
                try {
                    Remove-Item `$tempDir -Recurse -Force -ErrorAction Stop
                    Write-Host "Temporary files cleaned up" -ForegroundColor Green
                    break
                } catch {
                    if (`$i -eq 3) {
                        Write-Host "Could not remove temporary directory: `$tempDir" -ForegroundColor Yellow
                        Write-Host "   You may need to delete it manually" -ForegroundColor Gray
                    } else {
                        Start-Sleep -Milliseconds 500
                    }
                }
            }
        } catch {
            Write-Host "Cleanup warning: `$(`$_.Exception.Message)" -ForegroundColor Yellow
        }
    }
}

Write-Host ""
Write-Host "=================================================================" -ForegroundColor Cyan
Write-Host "file-utils-i installation complete! Thank you for choosing" -ForegroundColor Cyan
Write-Host "whispr.dev quantum-enhanced file security solutions!" -ForegroundColor Cyan
Write-Host "=================================================================" -ForegroundColor Cyan

Read-Host "Press Enter to exit installer"
"@

    Write-Host "Writing file-utils-i_installer.ps1..." -ForegroundColor Gray
    $installerPath = Join-Path -Path (Get-Location) -ChildPath "file-utils-i_installer.ps1"
    $installerContent | Out-File -FilePath $installerPath -Encoding UTF8
    
    if (Test-Path $installerPath) {
        $ps1Size = [math]::Round((Get-Item $installerPath).Length / 1024, 1)
        Write-Host "Created file-utils-i_installer.ps1 ($ps1Size KB)" -ForegroundColor Green
    } else {
        throw "file-utils-i_installer.ps1 was not created"
    }
    
} catch {
    Write-Host "Failed to create PowerShell installer: $($_.Exception.Message)" -ForegroundColor Red
    Read-Host "Press Enter to exit"
    exit 1
}

####################
# STEP 4: Convert to EXE
####################

Write-Host ""
Write-Host "STEP 4: Converting PowerShell to EXE..." -ForegroundColor Yellow

try {
    Write-Host "Importing ps2exe module..." -ForegroundColor Gray
    Import-Module ps2exe -Force
    
    # Check for icon
    $iconPath = Join-Path -Path (Get-Location) -ChildPath "file-utils-i.ico"
    $iconExists = Test-Path $iconPath
    Write-Host "Icon file (file-utils-i.ico) exists: $iconExists" -ForegroundColor Gray
    
    if ($iconExists) {
        $iconSize = [math]::Round((Get-Item $iconPath).Length / 1024, 1)
        Write-Host "Icon size: $iconSize KB" -ForegroundColor Gray
    }
    
    # Clean up old EXE
    $exePath = Join-Path -Path (Get-Location) -ChildPath "file-utils-i_installer.exe"
    if (Test-Path $exePath) {
        Write-Host "Removing existing EXE..." -ForegroundColor Gray
        Remove-Item $exePath -Force -ErrorAction SilentlyContinue
        Start-Sleep -Seconds 1
    }
    
    Write-Host "Running ps2exe conversion..." -ForegroundColor Gray
    
    $ps1Path = Join-Path -Path (Get-Location) -ChildPath "file-utils-i_installer.ps1"
    
    if ($iconExists) {
        Write-Host "ps2exe command: ps2exe -inputFile file-utils-i_installer.ps1 -outputFile file-utils-i_installer.exe -iconFile file-utils-i.ico -title 'file-utils-i Installer' ..." -ForegroundColor Gray
        
        ps2exe `
            -inputFile $ps1Path `
            -outputFile $exePath `
            -iconFile $iconPath `
            -title "file-utils-i Installer" `
            -description "Quantum-Enhanced File Security Tool Installer" `
            -company "whispr.dev" `
            -version "0.4.0" `
            -copyright "Copyright 2025 whispr.dev" `
            -product "file-utils-i" `
            -noConsole `
            -requireAdmin `
            -verbose
    } else {
        Write-Host "ps2exe command: ps2exe -inputFile file-utils-i_installer.ps1 -outputFile file-utils-i_installer.exe -title 'file-utils-i Installer' ..." -ForegroundColor Gray
        
        ps2exe `
            -inputFile $ps1Path `
            -outputFile $exePath `
            -title "file-utils-i Installer" `
            -description "Quantum-Enhanced File Security Tool Installer" `
            -company "whispr.dev" `
            -version "0.3.1" `
            -copyright "Copyright 2025 whispr.dev" `
            -product "file-utils-i" `
            -noConsole `
            -requireAdmin `
            -verbose
    }
    
    Start-Sleep -Seconds 2
    
    if (Test-Path $exePath) {
        $exeSize = [math]::Round((Get-Item $exePath).Length / 1024 / 1024, 2)
        Write-Host "Created file-utils-i_installer.exe ($exeSize MB)" -ForegroundColor Green
    } else {
        Write-Host "ps2exe completed but EXE not found" -ForegroundColor Yellow
    }
    
} catch {
    Write-Host "ps2exe conversion failed: $($_.Exception.Message)" -ForegroundColor Red
    
    $exePath = Join-Path -Path (Get-Location) -ChildPath "file-utils-i_installer.exe"
    if (Test-Path $exePath) {
        $exeSize = [math]::Round((Get-Item $exePath).Length / 1024 / 1024, 2)
        Write-Host "BUT EXE was created anyway ($exeSize MB)" -ForegroundColor Green
    } else {
        Write-Host "Creating batch wrapper as fallback..." -ForegroundColor Yellow
        
        $batchWrapper = @"
@echo off
title file-utils-i Installer
cd /d "%~dp0"
echo Starting file-utils-i installer...
powershell -ExecutionPolicy Bypass -WindowStyle Normal -File "file-utils-i_installer.ps1"
pause
"@
        
        $batchPath = Join-Path -Path (Get-Location) -ChildPath "file-utils-i_installer.bat"
        $batchWrapper | Out-File -FilePath $batchPath -Encoding ASCII
        Write-Host "Created batch wrapper: file-utils-i_installer.bat" -ForegroundColor Green
    }
}

####################
# GENERATE CHECKSUMS
####################

Write-Host ""
Write-Host "STEP 5: Generating checksums..." -ForegroundColor Yellow

$exePath = Join-Path -Path (Get-Location) -ChildPath "file-utils-i_installer.exe"
if (Test-Path $exePath) {
    $hash = Get-FileHash $exePath -Algorithm SHA256
    $size = [math]::Round((Get-Item $exePath).Length / 1024 / 1024, 2)
    
    $checksumContent = @"
file-utils-i v0.4.0 - SHA256 Checksum
=====================================
Generated: $(Get-Date -Format "yyyy-MM-dd HH:mm:ss UTC")
Publisher: whispr.dev

Verify with: Get-FileHash file-utils-i_installer.exe -Algorithm SHA256

$($hash.Hash.ToLower())  file-utils-i_installer.exe  ($size MB)
"@
    
    $checksumPath = Join-Path -Path (Get-Location) -ChildPath "checksums.md"
    $checksumContent | Out-File $checksumPath -Encoding UTF8
    
    Write-Host "Checksums saved to checksums.md" -ForegroundColor Green
} else {
    Write-Host "No EXE found to generate checksums" -ForegroundColor Yellow
}

####################
# FINAL RESULTS
####################

Write-Host ""
Write-Host "=== FINAL RESULTS ===" -ForegroundColor Cyan
Write-Host ""

$outputFiles = @("file-utils-i_installer.exe", "file-utils-i_installer.ps1", "file-utils-i_installer.bat", "archive.7z", "checksums.md")
foreach ($file in $outputFiles) {
    $filePath = Join-Path -Path (Get-Location) -ChildPath $file
    if (Test-Path $filePath) {
        $size = Get-Item $filePath | ForEach-Object { 
            if ($_.Length -gt 1MB) { 
                "$([math]::Round($_.Length / 1MB, 2)) MB" 
            } else { 
                "$([math]::Round($_.Length / 1KB, 1)) KB" 
            }
        }
        Write-Host "$file ($size)" -ForegroundColor Green
    } else {
        Write-Host "$file (not found)" -ForegroundColor Red
    }
}

Write-Host ""
Write-Host "Build process complete!" -ForegroundColor Green
Write-Host ""
Write-Host "To test: .\file-utils-i_installer.exe" -ForegroundColor Yellow

##########################
# CONGRATZ. YOU MADE IT! #
##########################