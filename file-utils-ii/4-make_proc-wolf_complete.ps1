# Build-ProcWolfComplete.ps1
# Master build script for complete proc-wolf self-extracting installer
# Orchestrates the entire build pipeline from source files to final executable

param(
    [string]$SourceDir = ".",
    [string]$OutputDir = ".\dist",
    [string]$IconPath = ".\proc-wolf.ico",
    [string]$Version = "3.0.0.0",
    [string]$Company = "RYO Modular",
    [string]$Product = "proc-wolf Process Monitor",
    [switch]$SkipArchive,
    [switch]$SkipInstaller,
    [switch]$SkipExe,
    [switch]$Force,
    [switch]$Verbose,
    [switch]$NoConsole,
    [switch]$CleanFirst
)

# Color output functions
function Write-Success { param($Message) Write-Host "✓ $Message" -ForegroundColor Green }
function Write-Error { param($Message) Write-Host "✗ $Message" -ForegroundColor Red }
function Write-Warning { param($Message) Write-Host "⚠ $Message" -ForegroundColor Yellow }
function Write-Info { param($Message) Write-Host "ℹ $Message" -ForegroundColor Cyan }
function Write-Step { param($Message) Write-Host "➤ $Message" -ForegroundColor Magenta }
function Write-Phase { param($Message) Write-Host "`n$('='*60)" -ForegroundColor Blue; Write-Host "PHASE: $Message" -ForegroundColor Blue; Write-Host "$('='*60)" -ForegroundColor Blue }

# Build configuration
$ErrorActionPreference = "Stop"
$BuildConfig = @{
    Name = "proc-wolf Complete Build Pipeline"
    Version = "3.0"
    BuildDate = Get-Date -Format "yyyy-MM-dd HH:mm:ss"
    StartTime = Get-Date
    Steps = @{
        Archive = @{ Name = "Archive Creation"; Script = "Build-ProcWolfArchive.ps1"; Status = "Pending" }
        Installer = @{ Name = "Installer Generation"; Script = "Build-ProcWolfInstaller.ps1"; Status = "Pending" }
        Executable = @{ Name = "EXE Conversion"; Script = "Convert-ProcWolfToExe.ps1"; Status = "Pending" }
    }
    Files = @{
        Archive = "proc-wolf-files.zip"
        Installer = "proc-wolf-installer.ps1"
        Executable = "proc-wolf-installer.exe"
        Checksums = "proc-wolf-installer.checksums.txt"
        Guide = "INSTALLATION-GUIDE.md"
    }
}

function Test-BuildEnvironment {
    Write-Phase "ENVIRONMENT VALIDATION"
    
    $issues = @()
    $warnings = @()
    
    # Check PowerShell version
    if ($PSVersionTable.PSVersion.Major -lt 5) {
        $issues += "PowerShell 5.0 or higher required (current: $($PSVersionTable.PSVersion))"
    } else {
        Write-Success "PowerShell version: $($PSVersionTable.PSVersion)"
    }
    
    # Check for required build scripts
    $requiredScripts = @(
        "Build-ProcWolfArchive.ps1",
        "Build-ProcWolfInstaller.ps1", 
        "Convert-ProcWolfToExe.ps1"
    )
    
    foreach ($script in $requiredScripts) {
        if (Test-Path $script) {
            Write-Success "Build script found: $script"
        } else {
            $issues += "Required build script not found: $script"
        }
    }
    
    # Check source directory
    if (-not (Test-Path $SourceDir)) {
        $issues += "Source directory not found: $SourceDir"
    } else {
        Write-Success "Source directory: $SourceDir"
        
        # Check for proc-wolf files
        $procWolfFiles = @(
            "ProcWolf.exe",
            "ProcWolfCLI.exe",
            "ProcWolfService.exe",
            "install.bat",
            "uninstall.bat",
            "README.txt"
        )
        
        $foundFiles = 0
        $missingFiles = @()
        
        foreach ($file in $procWolfFiles) {
            $filePath = Join-Path $SourceDir $file
            if (Test-Path $filePath) {
                $foundFiles++
                if ($Verbose) { Write-Success "  Found: $file" }
            } else {
                $missingFiles += $file
            }
        }
        
        Write-Info "Source files: $foundFiles/$($procWolfFiles.Count) required files found"
        
        if ($missingFiles.Count -gt 0) {
            Write-Warning "Missing files will prevent build:"
            $missingFiles | ForEach-Object { Write-Warning "  - $_" }
        }
    }
    
    # Check icon file
    if ($IconPath) {
        if (Test-Path $IconPath) {
            $iconInfo = Get-Item $IconPath
            if ($iconInfo.Extension -eq '.ico') {
                Write-Success "Icon file: $IconPath ($([math]::Round($iconInfo.Length/1KB, 1)) KB)"
            } else {
                $warnings += "Icon file is not .ico format: $($iconInfo.Extension)"
            }
        } else {
            $warnings += "Icon file not found: $IconPath"
        }
    }
    
    # Check for ps2exe module (needed for final step)
    $ps2exeModule = Get-Module -ListAvailable -Name "ps2exe"
    if ($ps2exeModule) {
        Write-Success "ps2exe module: v$($ps2exeModule.Version)"
    } else {
        $warnings += "ps2exe module not found - will be needed for EXE conversion"
    }
    
    # Check disk space (estimate needed space)
    try {
        $drive = Split-Path $OutputDir -Qualifier
        if (-not $drive) { $drive = (Get-Location).Drive.Root }
        
        $driveInfo = Get-WmiObject -Class Win32_LogicalDisk | Where-Object { $_.DeviceID -eq $drive.TrimEnd('\') }
        if ($driveInfo) {
            $freeSpaceGB = [math]::Round($driveInfo.FreeSpace / 1GB, 2)
            if ($freeSpaceGB -gt 1) {
                Write-Success "Available disk space: $freeSpaceGB GB"
            } else {
                $warnings += "Low disk space: $freeSpaceGB GB available"
            }
        }
    } catch {
        $warnings += "Could not check disk space"
    }
    
    # Display warnings
    if ($warnings.Count -gt 0) {
        Write-Warning "Build warnings:"
        $warnings | ForEach-Object { Write-Warning "  - $_" }
    }
    
    # Check for issues
    if ($issues.Count -gt 0) {
        Write-Error "Build environment issues:"
        $issues | ForEach-Object { Write-Error "  - $_" }
        return $false
    }
    
    Write-Success "Build environment validation passed"
    return $true
}

function Initialize-BuildEnvironment {
    Write-Step "Initializing build environment..."
    
    # Create output directory
    if (-not (Test-Path $OutputDir)) {
        New-Item -ItemType Directory -Path $OutputDir -Force | Out-Null
        Write-Success "Created output directory: $OutputDir"
    } else {
        Write-Info "Output directory exists: $OutputDir"
        
        # Clean if requested
        if ($CleanFirst) {
            Write-Step "Cleaning output directory..."
            Get-ChildItem $OutputDir -File | Remove-Item -Force
            Write-Success "Output directory cleaned"
        }
    }
    
    # Check for existing files
    $existingFiles = @()
    foreach ($file in $BuildConfig.Files.Values) {
        $filePath = Join-Path $OutputDir $file
        if (Test-Path $filePath) {
            $existingFiles += $file
        }
    }
    
    if ($existingFiles.Count -gt 0 -and -not $Force) {
        Write-Warning "Existing build files found:"
        $existingFiles | ForEach-Object { Write-Warning "  - $_" }
        Write-Info "Use -Force to overwrite or -CleanFirst to clean before building"
        
        $continue = Read-Host "Continue build anyway? (y/N)"
        if ($continue -ne 'y' -and $continue -ne 'Y') {
            Write-Info "Build cancelled by user"
            return $false
        }
    }
    
    return $true
}

function Invoke-BuildStep {
    param(
        [string]$StepName,
        [string]$ScriptPath,
        [hashtable]$Parameters = @{}
    )
    
    Write-Phase $BuildConfig.Steps[$StepName].Name
    
    try {
        $BuildConfig.Steps[$StepName].Status = "Running"
        $BuildConfig.Steps[$StepName].StartTime = Get-Date
        
        # Build parameter string for script execution
        $paramString = ""
        foreach ($param in $Parameters.GetEnumerator()) {
            if ($param.Value -is [switch] -and $param.Value) {
                $paramString += " -$($param.Key)"
            } elseif ($param.Value -is [string] -and $param.Value) {
                $paramString += " -$($param.Key) '$($param.Value)'"
            }
        }
        
        Write-Step "Executing: $ScriptPath $paramString"
        
        # Execute the build script
        $result = & $ScriptPath @Parameters
        $exitCode = $LASTEXITCODE
        
        if ($exitCode -eq 0) {
            $BuildConfig.Steps[$StepName].Status = "Success"
            $BuildConfig.Steps[$StepName].EndTime = Get-Date
            $duration = $BuildConfig.Steps[$StepName].EndTime - $BuildConfig.Steps[$StepName].StartTime
            Write-Success "$($BuildConfig.Steps[$StepName].Name) completed in $([math]::Round($duration.TotalSeconds, 1)) seconds"
            return $true
        } else {
            $BuildConfig.Steps[$StepName].Status = "Failed"
            $BuildConfig.Steps[$StepName].EndTime = Get-Date
            Write-Error "$($BuildConfig.Steps[$StepName].Name) failed with exit code: $exitCode"
            return $false
        }
    } catch {
        $BuildConfig.Steps[$StepName].Status = "Error"
        $BuildConfig.Steps[$StepName].EndTime = Get-Date
        $BuildConfig.Steps[$StepName].Error = $_.Exception.Message
        Write-Error "$($BuildConfig.Steps[$StepName].Name) error: $($_.Exception.Message)"
        return $false
    }
}

function Invoke-ArchiveStep {
    if ($SkipArchive) {
        Write-Phase "ARCHIVE CREATION (SKIPPED)"
        $BuildConfig.Steps.Archive.Status = "Skipped"
        
        # Verify archive exists
        $archivePath = Join-Path $OutputDir $BuildConfig.Files.Archive
        if (Test-Path $archivePath) {
            Write-Info "Using existing archive: $archivePath"
            return $true
        } else {
            Write-Error "Archive not found and step skipped: $archivePath"
            return $false
        }
    }
    
    $params = @{
        SourceDir = $SourceDir
        OutputDir = $OutputDir
        Verbose = $Verbose
        Force = $Force
    }
    
    return Invoke-BuildStep -StepName "Archive" -ScriptPath ".\Build-ProcWolfArchive.ps1" -Parameters $params
}

function Invoke-InstallerStep {
    if ($SkipInstaller) {
        Write-Phase "INSTALLER GENERATION (SKIPPED)"
        $BuildConfig.Steps.Installer.Status = "Skipped"
        
        # Verify installer exists
        $installerPath = Join-Path $OutputDir $BuildConfig.Files.Installer
        if (Test-Path $installerPath) {
            Write-Info "Using existing installer: $installerPath"
            return $true
        } else {
            Write-Error "Installer not found and step skipped: $installerPath"
            return $false
        }
    }
    
    $archivePath = Join-Path $OutputDir $BuildConfig.Files.Archive
    $outputName = $BuildConfig.Files.Installer
    
    $params = @{
        ArchivePath = $archivePath
        OutputDir = $OutputDir
        OutputName = $outputName
        Verbose = $Verbose
        Force = $Force
    }
    
    return Invoke-BuildStep -StepName "Installer" -ScriptPath ".\Build-ProcWolfInstaller.ps1" -Parameters $params
}

function Invoke-ExecutableStep {
    if ($SkipExe) {
        Write-Phase "EXE CONVERSION (SKIPPED)"
        $BuildConfig.Steps.Executable.Status = "Skipped"
        
        # Verify executable exists
        $exePath = Join-Path $OutputDir $BuildConfig.Files.Executable
        if (Test-Path $exePath) {
            Write-Info "Using existing executable: $exePath"
            return $true
        } else {
            Write-Error "Executable not found and step skipped: $exePath"
            return $false
        }
    }
    
    $inputScript = Join-Path $OutputDir $BuildConfig.Files.Installer
    $outputExe = Join-Path $OutputDir $BuildConfig.Files.Executable
    
    $params = @{
        InputScript = $inputScript
        OutputExe = $outputExe
        Version = $Version
        Company = $Company
        Product = $Product
        Verbose = $Verbose
        Force = $Force
        NoConsole = $NoConsole
    }
    
    # Add icon if available
    if ($IconPath -and (Test-Path $IconPath)) {
        $params.IconPath = $IconPath
    }
    
    return Invoke-BuildStep -StepName "Executable" -ScriptPath ".\Convert-ProcWolfToExe.ps1" -Parameters $params
}

function Test-BuildResults {
    Write-Phase "BUILD VERIFICATION"
    
    $success = $true
    $results = @{}
    
    # Check each expected output file
    foreach ($fileType in $BuildConfig.Files.GetEnumerator()) {
        $filePath = Join-Path $OutputDir $fileType.Value
        
        if (Test-Path $filePath) {
            $fileInfo = Get-Item $filePath
            $results[$fileType.Key] = @{
                Path = $filePath
                Size = $fileInfo.Length
                Created = $fileInfo.CreationTime
                Modified = $fileInfo.LastWriteTime
                Hash = (Get-FileHash $filePath -Algorithm SHA256).Hash
            }
            Write-Success "$($fileType.Key): $($fileType.Value) ($([math]::Round($fileInfo.Length/1MB, 2)) MB)"
        } else {
            Write-Error "$($fileType.Key): $($fileType.Value) - NOT FOUND"
            $success = $false
        }
    }
    
    # Test executable if it exists
    $exePath = Join-Path $OutputDir $BuildConfig.Files.Executable
    if (Test-Path $exePath) {
        Write-Step "Testing executable..."
        try {
            # Test basic execution (version check)
            $testOutput = & $exePath -Extract -InstallPath "$env:TEMP\proc-wolf-test" -Quiet 2>&1
            
            if (Test-Path "$env:TEMP\proc-wolf-test") {
                $extractedFiles = Get-ChildItem "$env:TEMP\proc-wolf-test" -File
                Write-Success "Executable test passed - extracted $($extractedFiles.Count) files"
                
                # Clean up test extraction
                Remove-Item "$env:TEMP\proc-wolf-test" -Recurse -Force -ErrorAction SilentlyContinue
            } else {
                Write-Warning "Executable test: extraction location not found"
            }
        } catch {
            Write-Warning "Executable test failed: $($_.Exception.Message)"
        }
    }
    
    return $success, $results
}

function Write-BuildSummary {
    param($BuildResults, $Success)
    
    $buildDuration = (Get-Date) - $BuildConfig.StartTime
    
    Write-Host "`n" + "="*80 -ForegroundColor Cyan
    Write-Host "PROC-WOLF COMPLETE BUILD SUMMARY" -ForegroundColor Cyan
    Write-Host "="*80 -ForegroundColor Cyan
    
    Write-Host "`nBuild Information:" -ForegroundColor Yellow
    Write-Host "  Project: $($BuildConfig.Name)"
    Write-Host "  Version: $Version"
    Write-Host "  Build Date: $($BuildConfig.BuildDate)"
    Write-Host "  Build Duration: $([math]::Round($buildDuration.TotalMinutes, 1)) minutes"
    Write-Host "  Output Directory: $OutputDir"
    
    Write-Host "`nBuild Steps:" -ForegroundColor Yellow
    foreach ($step in $BuildConfig.Steps.GetEnumerator()) {
        $stepInfo = $step.Value
        $statusColor = switch ($stepInfo.Status) {
            "Success" { "Green" }
            "Failed" { "Red" }
            "Error" { "Red" }
            "Skipped" { "Yellow" }
            default { "Gray" }
        }
        
        $duration = if ($stepInfo.EndTime -and $stepInfo.StartTime) {
            " ($([math]::Round(($stepInfo.EndTime - $stepInfo.StartTime).TotalSeconds, 1))s)"
        } else { "" }
        
        Write-Host "  $($stepInfo.Name): " -NoNewline
        Write-Host "$($stepInfo.Status)$duration" -ForegroundColor $statusColor
        
        if ($stepInfo.Error) {
            Write-Host "    Error: $($stepInfo.Error)" -ForegroundColor Red
        }
    }
    
    if ($BuildResults) {
        Write-Host "`nGenerated Files:" -ForegroundColor Yellow
        foreach ($result in $BuildResults.GetEnumerator()) {
            Write-Host "  $($result.Key):"
            Write-Host "    Path: $($result.Value.Path)"
            Write-Host "    Size: $([math]::Round($result.Value.Size/1MB, 2)) MB"
            Write-Host "    SHA256: $($result.Value.Hash)"
        }
    }
    
    Write-Host "`nBuild Status: " -NoNewline
    if ($Success) {
        Write-Host "SUCCESS" -ForegroundColor Green
        
        Write-Host "`nReady for Distribution:" -ForegroundColor Green
        Write-Host "  - Self-extracting installer: $($BuildConfig.Files.Executable)"
        Write-Host "  - Checksums file: $($BuildConfig.Files.Checksums)"
        Write-Host "  - Installation guide: $($BuildConfig.Files.Guide)"
        
        Write-Host "`nNext Steps:" -ForegroundColor Green
        Write-Host "  1. Test installation: .\$($BuildConfig.Files.Executable) -Extract"
        Write-Host "  2. Verify checksums"
        Write-Host "  3. Test on clean Windows system"
        Write-Host "  4. Sign executable (optional)"
        Write-Host "  5. Create distribution package"
    } else {
        Write-Host "FAILED" -ForegroundColor Red
        Write-Host "  Check error messages above and resolve issues" -ForegroundColor Red
    }
    
    Write-Host "`n" + "="*80 -ForegroundColor Cyan
}

# Main execution
try {
    Write-Host "PROC-WOLF COMPLETE BUILD PIPELINE v$($BuildConfig.Version)" -ForegroundColor Cyan
    Write-Host "="*60 -ForegroundColor Cyan
    Write-Host "Building self-extracting installer from source files" -ForegroundColor Cyan
    Write-Host "Started: $($BuildConfig.BuildDate)" -ForegroundColor Cyan
    
    # Validate build environment
    if (-not (Test-BuildEnvironment)) {
        exit 1
    }
    
    # Initialize build environment
    if (-not (Initialize-BuildEnvironment)) {
        exit 1
    }
    
    # Execute build steps
    $allStepsSuccessful = $true
    
    # Step 1: Create archive
    if (-not (Invoke-ArchiveStep)) {
        $allStepsSuccessful = $false
        if (-not $Force) {
            Write-Error "Archive step failed. Use -Force to continue with subsequent steps."
            exit 1
        }
    }
    
    # Step 2: Generate installer
    if (-not (Invoke-InstallerStep)) {
        $allStepsSuccessful = $false
        if (-not $Force) {
            Write-Error "Installer step failed. Use -Force to continue with subsequent steps."
            exit 1
        }
    }
    
    # Step 3: Convert to executable
    if (-not (Invoke-ExecutableStep)) {
        $allStepsSuccessful = $false
        if (-not $Force) {
            Write-Error "Executable step failed."
            exit 1
        }
    }
    
    # Verify build results
    $buildSuccess, $buildResults = Test-BuildResults
    $finalSuccess = $allStepsSuccessful -and $buildSuccess
    
    # Display summary
    Write-BuildSummary -BuildResults $buildResults -Success $finalSuccess
    
    if ($finalSuccess) {
        Write-Success "`nComplete build pipeline finished successfully!"
        Write-Info "Distribution ready: $(Join-Path $OutputDir $BuildConfig.Files.Executable)"
        exit 0
    } else {
        Write-Error "`nBuild pipeline completed with errors!"
        exit 1
    }
    
} catch {
    Write-Error "Build pipeline failed: $($_.Exception.Message)"
    if ($Verbose) {
        Write-Error "Full error: $($_.Exception)"
        Write-Error "Stack trace: $($_.ScriptStackTrace)"
    }
    exit 1
}