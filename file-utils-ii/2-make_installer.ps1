# Build-ProcWolfInstaller.ps1
# Creates self-extracting PowerShell installer with embedded archive
# Part 2 of the proc-wolf self-extracting installer build process

param(
    [string]$ArchivePath = ".\dist\proc-wolf-files.zip",
    [string]$OutputDir = ".\dist",
    [string]$OutputName = "proc-wolf-installer.ps1",
    [switch]$Verbose,
    [switch]$Force
)

# Color output functions
function Write-Success { param($Message) Write-Host "✓ $Message" -ForegroundColor Green }
function Write-Error { param($Message) Write-Host "✗ $Message" -ForegroundColor Red }
function Write-Warning { param($Message) Write-Host "⚠ $Message" -ForegroundColor Yellow }
function Write-Info { param($Message) Write-Host "ℹ $Message" -ForegroundColor Cyan }
function Write-Step { param($Message) Write-Host "➤ $Message" -ForegroundColor Magenta }

# Configuration
$ErrorActionPreference = "Stop"
$InstallerConfig = @{
    Name = "proc-wolf Self-Extracting Installer"
    Version = "3.0"
    BuildDate = Get-Date -Format "yyyy-MM-dd HH:mm:ss"
    DefaultInstallPath = "C:\Program Files\proc-wolf"
    ServiceName = "ProcWolfService"
}

function Test-Prerequisites {
    Write-Step "Checking prerequisites..."
    
    # Check if archive exists
    if (-not (Test-Path $ArchivePath)) {
        Write-Error "Archive not found: $ArchivePath"
        Write-Info "Run Build-ProcWolfArchive.ps1 first to create the archive"
        return $false
    }
    
    # Check archive size (should be reasonable)
    $archiveInfo = Get-Item $ArchivePath
    if ($archiveInfo.Length -gt 50MB) {
        Write-Warning "Archive is quite large: $([math]::Round($archiveInfo.Length/1MB, 2)) MB"
    }
    
    Write-Success "Archive found: $([math]::Round($archiveInfo.Length/1MB, 2)) MB"
    return $true
}

function Get-Base64Archive {
    param($ArchivePath)
    
    Write-Step "Encoding archive to Base64..."
    
    try {
        $bytes = [System.IO.File]::ReadAllBytes($ArchivePath)
        $base64 = [System.Convert]::ToBase64String($bytes)
        
        Write-Success "Archive encoded successfully"
        Write-Info "  Original size: $([math]::Round($bytes.Length/1MB, 2)) MB"
        Write-Info "  Encoded size: $([math]::Round($base64.Length/1MB, 2)) MB"
        Write-Info "  Size increase: $([math]::Round((($base64.Length - $bytes.Length) / $bytes.Length) * 100, 1))%"
        
        return $base64
    } catch {
        throw "Failed to encode archive: $($_.Exception.Message)"
    }
}

function New-InstallerScript {
    param($Base64Archive, $OutputPath)
    
    Write-Step "Creating installer script..."
    
    $installerScript = @"
# proc-wolf Self-Extracting Installer v$($InstallerConfig.Version)
# Generated: $($InstallerConfig.BuildDate)
# This script contains an embedded ZIP archive with all proc-wolf files

#Requires -RunAsAdministrator

param(
    [string]`$InstallPath = "$($InstallerConfig.DefaultInstallPath)",
    [switch]`$Quiet,
    [switch]`$NoService,
    [switch]`$Force,
    [switch]`$Extract
)

# Color output functions
function Write-Success { param(`$Message) if (-not `$Quiet) { Write-Host "✓ `$Message" -ForegroundColor Green } }
function Write-Error { param(`$Message) Write-Host "✗ `$Message" -ForegroundColor Red }
function Write-Warning { param(`$Message) if (-not `$Quiet) { Write-Host "⚠ `$Message" -ForegroundColor Yellow } }
function Write-Info { param(`$Message) if (-not `$Quiet) { Write-Host "ℹ `$Message" -ForegroundColor Cyan } }
function Write-Step { param(`$Message) if (-not `$Quiet) { Write-Host "➤ `$Message" -ForegroundColor Magenta } }

function Test-Administrator {
    `$currentUser = [Security.Principal.WindowsIdentity]::GetCurrent()
    `$principal = New-Object Security.Principal.WindowsPrincipal(`$currentUser)
    return `$principal.IsInRole([Security.Principal.WindowsBuiltInRole]::Administrator)
}

function Stop-ExistingService {
    try {
        `$service = Get-Service -Name "$($InstallerConfig.ServiceName)" -ErrorAction SilentlyContinue
        if (`$service) {
            if (`$service.Status -eq "Running") {
                Write-Step "Stopping existing proc-wolf service..."
                Stop-Service -Name "$($InstallerConfig.ServiceName)" -Force -ErrorAction Stop
                Write-Success "Service stopped"
            }
            
            Write-Step "Removing existing service..."
            `$exePath = Join-Path `$InstallPath "ProcWolfService.exe"
            if (Test-Path `$exePath) {
                & "`$exePath" remove
                Start-Sleep -Seconds 2
            }
            Write-Success "Existing service removed"
        }
    } catch {
        Write-Warning "Could not stop/remove existing service: `$(`$_.Exception.Message)"
    }
}

function Expand-EmbeddedArchive {
    param(`$DestinationPath)
    
    Write-Step "Extracting embedded archive..."
    
    # Embedded archive data (Base64 encoded)
    `$archiveData = @"
$Base64Archive
"@
    
    try {
        # Decode Base64 and save to temp file
        `$tempZip = [System.IO.Path]::GetTempFileName() + ".zip"
        `$bytes = [System.Convert]::FromBase64String(`$archiveData)
        [System.IO.File]::WriteAllBytes(`$tempZip, `$bytes)
        
        Write-Info "Temporary archive: `$tempZip ($([math]::Round(`$bytes.Length/1MB, 2)) MB)"
        
        # Extract archive
        if (-not (Test-Path `$DestinationPath)) {
            New-Item -ItemType Directory -Path `$DestinationPath -Force | Out-Null
        }
        
        Expand-Archive -Path `$tempZip -DestinationPath `$DestinationPath -Force
        
        # Clean up temp file
        Remove-Item `$tempZip -Force
        
        # Verify extraction
        `$extractedFiles = Get-ChildItem `$DestinationPath -File | Measure-Object
        Write-Success "Extracted `$(`$extractedFiles.Count) files to `$DestinationPath"
        
        return `$true
    } catch {
        Write-Error "Failed to extract archive: `$(`$_.Exception.Message)"
        if (Test-Path `$tempZip) {
            Remove-Item `$tempZip -Force -ErrorAction SilentlyContinue
        }
        return `$false
    }
}

function Install-ProcWolfService {
    param(`$InstallPath)
    
    if (`$NoService) {
        Write-Info "Service installation skipped (--NoService specified)"
        return `$true
    }
    
    Write-Step "Installing proc-wolf service..."
    
    `$serviceExe = Join-Path `$InstallPath "ProcWolfService.exe"
    if (-not (Test-Path `$serviceExe)) {
        Write-Error "Service executable not found: `$serviceExe"
        return `$false
    }
    
    try {
        # Install service
        & "`$serviceExe" --startup auto install
        
        # Start service
        Start-Service -Name "$($InstallerConfig.ServiceName)" -ErrorAction Stop
        
        `$service = Get-Service -Name "$($InstallerConfig.ServiceName)"
        if (`$service.Status -eq "Running") {
            Write-Success "Service installed and started successfully"
            return `$true
        } else {
            Write-Error "Service installed but failed to start"
            return `$false
        }
    } catch {
        Write-Error "Failed to install service: `$(`$_.Exception.Message)"
        return `$false
    }
}

function Set-FilePermissions {
    param(`$InstallPath)
    
    Write-Step "Setting file permissions..."
    
    try {
        # Set directory permissions
        `$acl = Get-Acl `$InstallPath
        `$accessRule = New-Object System.Security.AccessControl.FileSystemAccessRule("Users", "ReadAndExecute", "ContainerInherit,ObjectInherit", "None", "Allow")
        `$acl.SetAccessRule(`$accessRule)
        Set-Acl -Path `$InstallPath -AclObject `$acl
        
        Write-Success "File permissions set"
    } catch {
        Write-Warning "Could not set file permissions: `$(`$_.Exception.Message)"
    }
}

function Write-InstallationSummary {
    param(`$InstallPath, `$ServiceInstalled)
    
    Write-Host "`n" + "="*60 -ForegroundColor Cyan
    Write-Host "PROC-WOLF INSTALLATION COMPLETE" -ForegroundColor Cyan
    Write-Host "="*60 -ForegroundColor Cyan
    
    Write-Host "`nInstallation Details:" -ForegroundColor Yellow
    Write-Host "  Install Path: `$InstallPath"
    Write-Host "  Service Installed: `$(if (`$ServiceInstalled) { 'Yes' } else { 'No' })"
    
    if (`$ServiceInstalled) {
        try {
            `$service = Get-Service -Name "$($InstallerConfig.ServiceName)"
            Write-Host "  Service Status: `$(`$service.Status)"
        } catch {
            Write-Host "  Service Status: Unknown"
        }
    }
    
    Write-Host "`nInstalled Files:" -ForegroundColor Yellow
    try {
        `$files = Get-ChildItem `$InstallPath -File | Sort-Object Name
        foreach (`$file in `$files) {
            Write-Host "  - `$(`$file.Name) ($([math]::Round(`$file.Length/1KB, 1)) KB)"
        }
    } catch {
        Write-Host "  Could not enumerate files"
    }
    
    Write-Host "`nUsage:" -ForegroundColor Green
    Write-Host "  Background Monitor: `"`$InstallPath\ProcWolf.exe`""
    Write-Host "  Command Line: `"`$InstallPath\ProcWolfCLI.exe`""
    Write-Host "  Service Control: services.msc (look for '$($InstallerConfig.ServiceName)')"
    
    Write-Host "`nLog Locations:" -ForegroundColor Green
    Write-Host "  Service Logs: C:\ProgramData\proc-wolf\"
    Write-Host "  Client Logs: %LOCALAPPDATA%\proc-wolf\"
    
    Write-Host "`n" + "="*60 -ForegroundColor Cyan
}

# Main installation logic
try {
    if (-not `$Quiet) {
        Write-Host "PROC-WOLF INSTALLER v$($InstallerConfig.Version)" -ForegroundColor Cyan
        Write-Host "="*40 -ForegroundColor Cyan
    }
    
    # Check administrator privileges
    if (-not (Test-Administrator)) {
        Write-Error "This installer must be run as Administrator"
        Write-Info "Right-click on the installer and select 'Run as administrator'"
        exit 1
    }
    
    # Handle extract-only mode
    if (`$Extract) {
        Write-Info "Extract-only mode specified"
        `$extractPath = if (`$InstallPath -eq "$($InstallerConfig.DefaultInstallPath)") { ".\proc-wolf-extracted" } else { `$InstallPath }
        
        if (Expand-EmbeddedArchive -DestinationPath `$extractPath) {
            Write-Success "Files extracted to: `$extractPath"
        } else {
            exit 1
        }
        exit 0
    }
    
    # Validate install path
    if (-not `$InstallPath) {
        Write-Error "Install path cannot be empty"
        exit 1
    }
    
    # Check if installation already exists
    if ((Test-Path `$InstallPath) -and -not `$Force) {
        `$existingFiles = Get-ChildItem `$InstallPath -ErrorAction SilentlyContinue
        if (`$existingFiles) {
            Write-Warning "Installation directory already exists: `$InstallPath"
            Write-Info "Use -Force to overwrite existing installation"
            exit 1
        }
    }
    
    # Stop existing service if present
    Stop-ExistingService
    
    # Extract files
    if (-not (Expand-EmbeddedArchive -DestinationPath `$InstallPath)) {
        exit 1
    }
    
    # Set file permissions
    Set-FilePermissions -InstallPath `$InstallPath
    
    # Install service
    `$serviceInstalled = Install-ProcWolfService -InstallPath `$InstallPath
    
    # Display summary
    Write-InstallationSummary -InstallPath `$InstallPath -ServiceInstalled `$serviceInstalled
    
    if (-not `$Quiet) {
        Write-Success "`nInstallation completed successfully!"
    }
    
} catch {
    Write-Error "Installation failed: `$(`$_.Exception.Message)"
    exit 1
}
"@

    # Save installer script
    $installerScript | Set-Content -Path $OutputPath -Encoding UTF8
    Write-Success "Installer script created: $OutputPath"
    
    return $OutputPath
}

function Test-InstallerScript {
    param($InstallerPath)
    
    Write-Step "Validating installer script..."
    
    try {
        # Check syntax by parsing
        $null = [System.Management.Automation.PSParser]::Tokenize((Get-Content $InstallerPath -Raw), [ref]$null)
        Write-Success "PowerShell syntax validation passed"
        
        # Check file size
        $installerInfo = Get-Item $InstallerPath
        Write-Info "Installer size: $([math]::Round($installerInfo.Length/1MB, 2)) MB"
        
        # Verify embedded data is present
        $content = Get-Content $InstallerPath -Raw
        if ($content -match '\$archiveData = @"') {
            Write-Success "Embedded archive data found"
        } else {
            Write-Error "Embedded archive data not found"
            return $false
        }
        
        return $true
    } catch {
        Write-Error "Installer validation failed: $($_.Exception.Message)"
        return $false
    }
}

function Write-BuildSummary {
    param($ArchivePath, $InstallerPath)
    
    $archiveInfo = Get-Item $ArchivePath
    $installerInfo = Get-Item $InstallerPath
    
    Write-Host "`n" + "="*80 -ForegroundColor Cyan
    Write-Host "PROC-WOLF INSTALLER BUILD SUMMARY" -ForegroundColor Cyan
    Write-Host "="*80 -ForegroundColor Cyan
    
    Write-Host "`nInput:" -ForegroundColor Yellow
    Write-Host "  Archive: $ArchivePath ($([math]::Round($archiveInfo.Length/1MB, 2)) MB)"
    
    Write-Host "`nOutput:" -ForegroundColor Yellow
    Write-Host "  Installer: $InstallerPath ($([math]::Round($installerInfo.Length/1MB, 2)) MB)"
    Write-Host "  Size Ratio: $([math]::Round(($installerInfo.Length / $archiveInfo.Length), 2))x"
    
    Write-Host "`nInstaller Features:" -ForegroundColor Yellow
    Write-Host "  - Self-extracting PowerShell script"
    Write-Host "  - Embedded ZIP archive (Base64 encoded)"
    Write-Host "  - Administrator privilege checking"
    Write-Host "  - Automatic service installation/management"
    Write-Host "  - Extract-only mode support"
    Write-Host "  - Installation path customization"
    Write-Host "  - Force overwrite capability"
    
    Write-Host "`nUsage Examples:" -ForegroundColor Green
    Write-Host "  Default install: .\proc-wolf-installer.ps1"
    Write-Host "  Custom path: .\proc-wolf-installer.ps1 -InstallPath 'C:\MyApps\proc-wolf'"
    Write-Host "  Extract only: .\proc-wolf-installer.ps1 -Extract"
    Write-Host "  Quiet install: .\proc-wolf-installer.ps1 -Quiet"
    Write-Host "  No service: .\proc-wolf-installer.ps1 -NoService"
    
    Write-Host "`nNext Steps:" -ForegroundColor Green
    Write-Host "  1. Test the installer: .\proc-wolf-installer.ps1 -Extract"
    Write-Host "  2. Run Convert-ProcWolfToExe.ps1 to create final .exe"
    Write-Host "  3. Generate checksums for distribution"
    
    Write-Host "`n" + "="*80 -ForegroundColor Cyan
}

# Main execution
try {
    Write-Host "PROC-WOLF INSTALLER BUILDER v$($InstallerConfig.Version)" -ForegroundColor Cyan
    Write-Host "="*50 -ForegroundColor Cyan
    
    # Test prerequisites
    if (-not (Test-Prerequisites)) {
        exit 1
    }
    
    # Create output directory
    if (-not (Test-Path $OutputDir)) {
        New-Item -ItemType Directory -Path $OutputDir -Force | Out-Null
        Write-Success "Created output directory: $OutputDir"
    }
    
    $outputPath = Join-Path $OutputDir $OutputName
    
    # Check if output exists
    if ((Test-Path $outputPath) -and -not $Force) {
        Write-Error "Output file already exists: $outputPath (use -Force to overwrite)"
        exit 1
    }
    
    # Encode archive to Base64
    $base64Archive = Get-Base64Archive -ArchivePath $ArchivePath
    
    # Create installer script
    $installerPath = New-InstallerScript -Base64Archive $base64Archive -OutputPath $outputPath
    
    # Validate installer
    if (-not (Test-InstallerScript -InstallerPath $installerPath)) {
        exit 1
    }
    
    # Display summary
    Write-BuildSummary -ArchivePath $ArchivePath -InstallerPath $installerPath
    
    Write-Success "`nInstaller build completed successfully!"
    
} catch {
    Write-Error "Build failed: $($_.Exception.Message)"
    if ($Verbose) {
        Write-Error "Full error: $($_.Exception)"
        Write-Error "Stack trace: $($_.ScriptStackTrace)"
    }
    exit 1
}