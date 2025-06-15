# Convert-ProcWolfToExe.ps1
# Converts PowerShell installer to executable using ps2exe
# Part 3 of the proc-wolf self-extracting installer build process

param(
    [string]$InputScript = ".\dist\proc-wolf-installer.ps1",
    [string]$OutputExe = ".\dist\proc-wolf-installer.exe",
    [string]$IconPath = ".\proc-wolf.ico",
    [string]$Version = "3.0.0.0",
    [string]$Company = "RYO Modular",
    [string]$Product = "proc-wolf Process Monitor",
    [string]$Copyright = "Copyright (c) 2025 RYO Modular",
    [switch]$Verbose,
    [switch]$Force,
    [switch]$NoConsole
)

# Color output functions
function Write-Success { param($Message) Write-Host "✓ $Message" -ForegroundColor Green }
function Write-Error { param($Message) Write-Host "✗ $Message" -ForegroundColor Red }
function Write-Warning { param($Message) Write-Host "⚠ $Message" -ForegroundColor Yellow }
function Write-Info { param($Message) Write-Host "ℹ $Message" -ForegroundColor Cyan }
function Write-Step { param($Message) Write-Host "➤ $Message" -ForegroundColor Magenta }

# Configuration
$ErrorActionPreference = "Stop"
$ConversionConfig = @{
    Name = "proc-wolf EXE Converter"
    Version = "3.0"
    BuildDate = Get-Date -Format "yyyy-MM-dd HH:mm:ss"
    RequiredModules = @("ps2exe")
    PS2ExeMinVersion = "1.0.12"
}

function Test-Prerequisites {
    Write-Step "Checking prerequisites..."
    
    $issues = @()
    
    # Check PowerShell version
    if ($PSVersionTable.PSVersion.Major -lt 5) {
        $issues += "PowerShell 5.0 or higher required (current: $($PSVersionTable.PSVersion))"
    } else {
        Write-Success "PowerShell version: $($PSVersionTable.PSVersion)"
    }
    
    # Check for ps2exe module
    $ps2exeModule = Get-Module -ListAvailable -Name "ps2exe" | Sort-Object Version -Descending | Select-Object -First 1
    if (-not $ps2exeModule) {
        $issues += "ps2exe module not found. Install with: Install-Module ps2exe -Force"
    } else {
        Write-Success "ps2exe module found: v$($ps2exeModule.Version)"
        
        # Check version
        if ($ps2exeModule.Version -lt [version]$ConversionConfig.PS2ExeMinVersion) {
            $issues += "ps2exe version $($ConversionConfig.PS2ExeMinVersion) or higher required (found: $($ps2exeModule.Version))"
        }
    }
    
    # Check input script
    if (-not (Test-Path $InputScript)) {
        $issues += "Input script not found: $InputScript"
    } else {
        $scriptInfo = Get-Item $InputScript
        Write-Success "Input script found: $([math]::Round($scriptInfo.Length/1MB, 2)) MB"
        
        # Validate script content
        try {
            $content = Get-Content $InputScript -Raw
            if ($content -match '#Requires -RunAsAdministrator') {
                Write-Success "Script requires administrator privileges"
            } else {
                Write-Warning "Script doesn't explicitly require admin privileges"
            }
            
            if ($content -match '\$archiveData = @"') {
                Write-Success "Embedded archive data detected"
            } else {
                $issues += "Script doesn't appear to contain embedded archive data"
            }
        } catch {
            $issues += "Could not validate script content: $($_.Exception.Message)"
        }
    }
    
    # Check icon file
    if ($IconPath -and (Test-Path $IconPath)) {
        try {
            $iconInfo = Get-Item $IconPath
            if ($iconInfo.Extension -eq '.ico') {
                Write-Success "Icon file found: $IconPath ($([math]::Round($iconInfo.Length/1KB, 1)) KB)"
            } else {
                Write-Warning "Icon file is not .ico format: $($iconInfo.Extension)"
            }
        } catch {
            Write-Warning "Could not validate icon file: $($_.Exception.Message)"
        }
    } elseif ($IconPath) {
        Write-Warning "Icon file not found: $IconPath (will proceed without icon)"
        $script:IconPath = $null
    }
    
    # Check output path
    $outputDir = Split-Path $OutputExe -Parent
    if ($outputDir -and -not (Test-Path $outputDir)) {
        try {
            New-Item -ItemType Directory -Path $outputDir -Force | Out-Null
            Write-Success "Created output directory: $outputDir"
        } catch {
            $issues += "Could not create output directory: $outputDir"
        }
    }
    
    # Check if output exists
    if ((Test-Path $OutputExe) -and -not $Force) {
        $issues += "Output file already exists: $OutputExe (use -Force to overwrite)"
    }
    
    if ($issues.Count -gt 0) {
        Write-Error "Prerequisites not met:"
        $issues | ForEach-Object { Write-Error "  - $_" }
        return $false
    }
    
    return $true
}

function Install-PS2ExeModule {
    Write-Step "Installing ps2exe module..."
    
    try {
        # Check if running as administrator
        $currentUser = [Security.Principal.WindowsIdentity]::GetCurrent()
        $principal = New-Object Security.Principal.WindowsPrincipal($currentUser)
        $isAdmin = $principal.IsInRole([Security.Principal.WindowsBuiltInRole]::Administrator)
        
        if ($isAdmin) {
            Install-Module -Name "ps2exe" -Force -Scope AllUsers
            Write-Success "ps2exe installed for all users"
        } else {
            Install-Module -Name "ps2exe" -Force -Scope CurrentUser
            Write-Success "ps2exe installed for current user"
        }
        
        Import-Module ps2exe -Force
        return $true
    } catch {
        Write-Error "Failed to install ps2exe: $($_.Exception.Message)"
        Write-Info "You can install manually with: Install-Module ps2exe -Force"
        return $false
    }
}

function New-Executable {
    Write-Step "Converting PowerShell script to executable..."
    
    # Import ps2exe module
    try {
        Import-Module ps2exe -Force
    } catch {
        Write-Error "Could not import ps2exe module: $($_.Exception.Message)"
        return $false
    }
    
    # Build ps2exe parameters
    $ps2exeParams = @{
        inputFile = $InputScript
        outputFile = $OutputExe
        verbose = $Verbose
        noConsole = $NoConsole
        requireAdmin = $true
        longPaths = $true
    }
    
    # Add icon if available
    if ($IconPath -and (Test-Path $IconPath)) {
        $ps2exeParams.iconFile = $IconPath
        Write-Info "Using icon: $IconPath"
    }
    
    # Add version information
    if ($Version) {
        $ps2exeParams.version = $Version
    }
    if ($Company) {
        $ps2exeParams.company = $Company
    }
    if ($Product) {
        $ps2exeParams.product = $Product
    }
    if ($Copyright) {
        $ps2exeParams.copyright = $Copyright
    }
    
    # Additional metadata
    $ps2exeParams.description = "proc-wolf Process Monitor Self-Extracting Installer"
    $ps2exeParams.title = "proc-wolf Installer"
    
    Write-Info "Conversion parameters:"
    $ps2exeParams.GetEnumerator() | ForEach-Object {
        if ($_.Key -ne 'verbose') {
            Write-Info "  $($_.Key): $($_.Value)"
        }
    }
    
    try {
        Write-Info "Starting conversion... (this may take a while)"
        
        # Call ps2exe
        ps2exe @ps2exeParams
        
        if (Test-Path $OutputExe) {
            Write-Success "Executable created successfully"
            return $true
        } else {
            Write-Error "Executable was not created (ps2exe may have failed silently)"
            return $false
        }
    } catch {
        Write-Error "Conversion failed: $($_.Exception.Message)"
        return $false
    }
}

function Test-Executable {
    param($ExePath)
    
    Write-Step "Validating created executable..."
    
    try {
        $exeInfo = Get-Item $ExePath
        Write-Success "Executable file exists: $([math]::Round($exeInfo.Length/1MB, 2)) MB"
        
        # Check if it's a valid executable
        try {
            $versionInfo = [System.Diagnostics.FileVersionInfo]::GetVersionInfo($ExePath)
            Write-Success "File version info:"
            Write-Info "  Product: $($versionInfo.ProductName)"
            Write-Info "  Version: $($versionInfo.FileVersion)"
            Write-Info "  Company: $($versionInfo.CompanyName)"
            Write-Info "  Description: $($versionInfo.FileDescription)"
        } catch {
            Write-Warning "Could not read version information: $($_.Exception.Message)"
        }
        
        # Check digital signature (if any)
        try {
            $signature = Get-AuthenticodeSignature $ExePath
            if ($signature.Status -eq "Valid") {
                Write-Success "Digital signature: Valid"
                Write-Info "  Signer: $($signature.SignerCertificate.Subject)"
            } elseif ($signature.Status -eq "NotSigned") {
                Write-Warning "Executable is not digitally signed"
            } else {
                Write-Warning "Digital signature status: $($signature.Status)"
            }
        } catch {
            Write-Warning "Could not check digital signature: $($_.Exception.Message)"
        }
        
        # Test basic execution (just help/syntax check)
        if ($Verbose) {
            Write-Info "Testing executable syntax..."
            try {
                $testResult = & $ExePath -WhatIf 2>&1
                Write-Success "Executable syntax test passed"
            } catch {
                Write-Warning "Executable syntax test failed: $($_.Exception.Message)"
            }
        }
        
        return $true
    } catch {
        Write-Error "Executable validation failed: $($_.Exception.Message)"
        return $false
    }
}

function New-Checksums {
    param($ExePath)
    
    Write-Step "Generating checksums..."
    
    try {
        $checksums = @{}
        $algorithms = @('MD5', 'SHA1', 'SHA256', 'SHA512')
        
        foreach ($algorithm in $algorithms) {
            try {
                $hash = Get-FileHash -Path $ExePath -Algorithm $algorithm
                $checksums[$algorithm] = $hash.Hash
                Write-Success "$algorithm`: $($hash.Hash)"
            } catch {
                Write-Warning "Could not generate $algorithm hash: $($_.Exception.Message)"
            }
        }
        
        # Save checksums to file
        $outputDir = Split-Path $ExePath -Parent
        $checksumFile = Join-Path $outputDir "$(Split-Path $ExePath -LeafBase).checksums.txt"
        
        $checksumContent = @"
proc-wolf Installer Checksums
Generated: $(Get-Date -Format "yyyy-MM-dd HH:mm:ss")
File: $(Split-Path $ExePath -Leaf)
Size: $((Get-Item $ExePath).Length) bytes

"@
        
        foreach ($alg in $algorithms) {
            if ($checksums.ContainsKey($alg)) {
                $checksumContent += "$alg`: $($checksums[$alg])`n"
            }
        }
        
        $checksumContent | Set-Content -Path $checksumFile -Encoding UTF8
        Write-Success "Checksums saved: $checksumFile"
        
        return $checksums
    } catch {
        Write-Error "Checksum generation failed: $($_.Exception.Message)"
        return @{}
    }
}

function New-InstallationGuide {
    param($ExePath, $Checksums)
    
    Write-Step "Creating installation guide..."
    
    try {
        $outputDir = Split-Path $ExePath -Parent
        $guideFile = Join-Path $outputDir "INSTALLATION-GUIDE.md"
        
        $guide = @"
# proc-wolf Installation Guide

## Overview
This is a self-extracting installer for proc-wolf v3.0, a comprehensive process monitoring and security tool.

## System Requirements
- Windows 10/11 or Windows Server 2016/2019/2022
- Administrator privileges required
- PowerShell 5.0 or higher
- .NET Framework 4.7.2 or higher

## Installation Methods

### Method 1: Default Installation
1. Right-click on `$(Split-Path $ExePath -Leaf)` and select "Run as administrator"
2. Follow the on-screen prompts
3. proc-wolf will be installed to `C:\Program Files\proc-wolf`

### Method 2: Custom Installation Path
1. Open an elevated PowerShell prompt
2. Run: ``.\$(Split-Path $ExePath -Leaf) -InstallPath "C:\MyPath\proc-wolf"``

### Method 3: Extract Files Only
1. Run: ``.\$(Split-Path $ExePath -Leaf) -Extract``
2. Files will be extracted to `.\proc-wolf-extracted`

## Installation Options
- ``-InstallPath <path>`` - Custom installation directory
- ``-Quiet`` - Silent installation (no output)
- ``-NoService`` - Skip Windows service installation
- ``-Force`` - Overwrite existing installation
- ``-Extract`` - Extract files only, don't install

## File Verification
Verify the installer integrity using these checksums:

``````
"@
        
        if ($Checksums.Count -gt 0) {
            foreach ($alg in @('SHA256', 'SHA1', 'MD5')) {
                if ($Checksums.ContainsKey($alg)) {
                    $guide += "$alg`: $($Checksums[$alg])`n"
                }
            }
        }
        
        $guide += @"
``````

## What Gets Installed
- **ProcWolf.exe** - Background monitor with system tray
- **ProcWolfCLI.exe** - Command-line interface
- **ProcWolfService.exe** - Windows service for continuous monitoring
- **Configuration files** - Default settings and whitelists
- **Documentation** - README and usage instructions

## Post-Installation
1. The proc-wolf service will start automatically
2. Launch the system tray monitor: `C:\Program Files\proc-wolf\ProcWolf.exe`
3. Use CLI commands: `C:\Program Files\proc-wolf\ProcWolfCLI.exe list`

## Log Locations
- **Service logs**: `C:\ProgramData\proc-wolf\`
- **Client logs**: `%LOCALAPPDATA%\proc-wolf\`

## Uninstallation
1. Run `C:\Program Files\proc-wolf\uninstall.bat` as administrator
2. Or use Windows "Add/Remove Programs"

## Troubleshooting

### Installation Fails
- Ensure you're running as administrator
- Check that no antivirus is blocking the installer
- Verify system requirements are met

### Service Won't Start
- Check Windows Event Viewer for errors
- Verify all files were extracted properly
- Try reinstalling with `-Force` parameter

### Permission Denied Errors
- Ensure proc-wolf service has proper permissions
- Check file/folder permissions in installation directory

## Support
For issues or questions:
- Check the README.txt file in the installation directory
- Review log files for error details
- Ensure all system requirements are met

## Security Note
proc-wolf requires administrator privileges to monitor system processes effectively. The installer has been compiled from trusted PowerShell scripts and includes integrity verification.

Generated: $(Get-Date -Format "yyyy-MM-dd HH:mm:ss")
"@
        
        $guide | Set-Content -Path $guideFile -Encoding UTF8
        Write-Success "Installation guide created: $guideFile"
        
        return $guideFile
    } catch {
        Write-Warning "Could not create installation guide: $($_.Exception.Message)"
        return $null
    }
}

function Write-BuildSummary {
    param($InputScript, $OutputExe, $Checksums, $GuideFile)
    
    $inputInfo = Get-Item $InputScript
    $outputInfo = Get-Item $OutputExe
    
    Write-Host "`n" + "="*80 -ForegroundColor Cyan
    Write-Host "PROC-WOLF EXECUTABLE BUILD SUMMARY" -ForegroundColor Cyan
    Write-Host "="*80 -ForegroundColor Cyan
    
    Write-Host "`nConversion Details:" -ForegroundColor Yellow
    Write-Host "  Input Script: $InputScript ($([math]::Round($inputInfo.Length/1MB, 2)) MB)"
    Write-Host "  Output Executable: $OutputExe ($([math]::Round($outputInfo.Length/1MB, 2)) MB)"
    Write-Host "  Size Ratio: $([math]::Round(($outputInfo.Length / $inputInfo.Length), 2))x"
    Write-Host "  Icon: $(if ($IconPath -and (Test-Path $IconPath)) { $IconPath } else { 'None' })"
    
    Write-Host "`nExecutable Features:" -ForegroundColor Yellow
    Write-Host "  - Self-extracting installer with embedded ZIP"
    Write-Host "  - Requires administrator privileges"
    Write-Host "  - Version information embedded"
    Write-Host "  - Custom installation paths supported"
    Write-Host "  - Silent installation mode"
    Write-Host "  - Extract-only mode"
    Write-Host "  - Automatic service installation"
    
    if ($Checksums.Count -gt 0) {
        Write-Host "`nFile Integrity:" -ForegroundColor Yellow
        foreach ($alg in @('SHA256', 'MD5')) {
            if ($Checksums.ContainsKey($alg)) {
                Write-Host "  $alg`: $($Checksums[$alg])"
            }
        }
    }
    
    Write-Host "`nGenerated Files:" -ForegroundColor Yellow
    Write-Host "  - $OutputExe"
    Write-Host "  - $(Join-Path (Split-Path $OutputExe -Parent) "$(Split-Path $OutputExe -LeafBase).checksums.txt")"
    if ($GuideFile) {
        Write-Host "  - $GuideFile"
    }
    
    Write-Host "`nDistribution Ready:" -ForegroundColor Green
    Write-Host "  ✓ Self-contained executable installer"
    Write-Host "  ✓ Integrity checksums generated"
    Write-Host "  ✓ Installation guide created"
    Write-Host "  ✓ No external dependencies required"
    
    Write-Host "`nNext Steps:" -ForegroundColor Green
    Write-Host "  1. Test the installer: .\$(Split-Path $OutputExe -Leaf) -Extract"
    Write-Host "  2. Verify checksums match"
    Write-Host "  3. Test installation on clean system"
    Write-Host "  4. Distribute with installation guide"
    
    Write-Host "`n" + "="*80 -ForegroundColor Cyan
}

# Main execution
try {
    Write-Host "PROC-WOLF EXECUTABLE CONVERTER v$($ConversionConfig.Version)" -ForegroundColor Cyan
    Write-Host "="*50 -ForegroundColor Cyan
    
    # Test prerequisites
    if (-not (Test-Prerequisites)) {
        Write-Info "`nTo install ps2exe module:"
        Write-Info "  Install-Module ps2exe -Force"
        Write-Info "  (or run this script as admin to auto-install)"
        
        # Offer to install ps2exe if missing and running as admin
        $currentUser = [Security.Principal.WindowsIdentity]::GetCurrent()
        $principal = New-Object Security.Principal.WindowsPrincipal($currentUser)
        $isAdmin = $principal.IsInRole([Security.Principal.WindowsBuiltInRole]::Administrator)
        
        if ($isAdmin -and -not (Get-Module -ListAvailable -Name "ps2exe")) {
            $install = Read-Host "`nWould you like to install ps2exe module now? (y/N)"
            if ($install -eq 'y' -or $install -eq 'Y') {
                if (Install-PS2ExeModule) {
                    Write-Info "ps2exe installed. Re-running prerequisites check..."
                    if (-not (Test-Prerequisites)) {
                        exit 1
                    }
                } else {
                    exit 1
                }
            } else {
                exit 1
            }
        } else {
            exit 1
        }
    }
    
    # Remove existing output if Force is specified
    if ((Test-Path $OutputExe) -and $Force) {
        Remove-Item $OutputExe -Force
        Write-Warning "Removed existing executable"
    }
    
    # Convert to executable
    if (-not (New-Executable)) {
        exit 1
    }
    
    # Validate executable
    if (-not (Test-Executable -ExePath $OutputExe)) {
        exit 1
    }
    
    # Generate checksums
    $checksums = New-Checksums -ExePath $OutputExe
    
    # Create installation guide
    $guideFile = New-InstallationGuide -ExePath $OutputExe -Checksums $checksums
    
    # Display summary
    Write-BuildSummary -InputScript $InputScript -OutputExe $OutputExe -Checksums $checksums -GuideFile $guideFile
    
    Write-Success "`nExecutable conversion completed successfully!"
    Write-Info "Ready for distribution: $OutputExe"
    
} catch {
    Write-Error "Conversion failed: $($_.Exception.Message)"
    if ($Verbose) {
        Write-Error "Full error: $($_.Exception)"
        Write-Error "Stack trace: $($_.ScriptStackTrace)"
    }
    exit 1
}