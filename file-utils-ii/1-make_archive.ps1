# 1-build-archive.ps1
# Creates the initial archive collection of file-utils-ii files
# Part 1 of the file-utils-ii self-extracting installer build process

param(
    [string]$SourceDir = ".",
    [string]$OutputDir = ".\dist",
    [switch]$Verbose,
    [switch]$Force
)

# Color output functions
function Write-Success { param($Message) Write-Host ":D $Message" -ForegroundColor Green }
function Write-Error { param($Message) Write-Host "D: $Message" -ForegroundColor Red }
function Write-Warning { param($Message) Write-Host ":/ $Message" -ForegroundColor Yellow }
function Write-Info { param($Message) Write-Host ":) $Message" -ForegroundColor Cyan }
function Write-Step { param($Message) Write-Host ":| $Message" -ForegroundColor Magenta }

# Configuration
$ErrorActionPreference = "Stop"
$BuildInfo = @{
    Name = "file-utils-ii"
    Version = "3.0"
    BuildDate = Get-Date -Format "yyyy-MM-dd HH:mm:ss"
    RequiredFiles = @(
        "file-utils-ii.exe",
        "file-utils-ii-CLI.exe", 
        "file-utils-ii-svc.exe",
        "install.bat",
        "uninstall.bat",
        "README.txt",
        "file-utils-ii_full_3-0.py"
    )
    OptionalFiles = @(
        "file-utils-ii.ico",
        "config.ini",
        "whitelist.txt",
        "blacklist.txt"
    )
    OutputArchive = "file-utils-ii.7z"
}

function Test-Prerequisites {
    Write-Step "Checking prerequisites..."
    
    $missing = @()
    
    # Check PowerShell version
    if ($PSVersionTable.PSVersion.Major -lt 5) {
        $missing += "PowerShell 5.0 or higher required"
    }
    
    # Check for required modules/commands
    try {
        Get-Command "Compress-Archive" -ErrorAction Stop | Out-Null
        Write-Success "Compress-Archive available"
    } catch {
        $missing += "Compress-Archive cmdlet not available"
    }
    
    if ($missing.Count -gt 0) {
        Write-Error "Missing prerequisites:"
        $missing | ForEach-Object { Write-Error "  - $_" }
        return $false
    }
    
    Write-Success "All prerequisites met"
    return $true
}

function Test-SourceFiles {
    Write-Step "Validating source files..."
    
    $results = @{
        Found = @()
        Missing = @()
        Optional = @()
        Errors = @()
    }
    
    # Check required files
    foreach ($file in $BuildInfo.RequiredFiles) {
        $path = Join-Path $SourceDir $file
        if (Test-Path $path) {
            $fileInfo = Get-Item $path
            $results.Found += @{
                Name = $file
                Path = $path
                Size = $fileInfo.Length
                Modified = $fileInfo.LastWriteTime
                Hash = (Get-FileHash $path -Algorithm SHA256).Hash
            }
            Write-Success "Found required: $file ($([math]::Round($fileInfo.Length/1KB, 2)) KB)"
        } else {
            $results.Missing += $file
            Write-Error "Missing required: $file"
        }
    }
    
    # Check optional files
    foreach ($file in $BuildInfo.OptionalFiles) {
        $path = Join-Path $SourceDir $file
        if (Test-Path $path) {
            $fileInfo = Get-Item $path
            $results.Optional += @{
                Name = $file
                Path = $path
                Size = $fileInfo.Length
                Modified = $fileInfo.LastWriteTime
                Hash = (Get-FileHash $path -Algorithm SHA256).Hash
            }
            Write-Success "Found optional: $file ($([math]::Round($fileInfo.Length/1KB, 2)) KB)"
        } else {
            Write-Warning "Optional file not found: $file"
        }
    }
    
    # Validate specific file types
    foreach ($foundFile in $results.Found) {
        switch -Regex ($foundFile.Name) {
            '\.exe$' {
                try {
                    $versionInfo = [System.Diagnostics.FileVersionInfo]::GetVersionInfo($foundFile.Path)
                    if ($Verbose) {
                        Write-Info "  $($foundFile.Name): $($versionInfo.FileDescription)"
                        Write-Info "  Version: $($versionInfo.FileVersion)"
                    }
                } catch {
                    $results.Errors += "Could not read version info for $($foundFile.Name): $($_.Exception.Message)"
                }
            }
            '\.bat$' {
                $content = Get-Content $foundFile.Path -Raw
                if ($content -match 'echo off' -and $content.Length -gt 100) {
                    if ($Verbose) { Write-Info "  $($foundFile.Name): Valid batch script" }
                } else {
                    $results.Errors += "$($foundFile.Name) appears to be an invalid or empty batch file"
                }
            }
            '\.py$' {
                $content = Get-Content $foundFile.Path -Raw
                if ($content -match '#!/usr/bin/env python' -or $content -match 'import ') {
                    if ($Verbose) { Write-Info "  $($foundFile.Name): Valid Python script" }
                } else {
                    $results.Errors += "$($foundFile.Name) may not be a valid Python script"
                }
            }
        }
    }
    
    return $results
}

function New-BuildManifest {
    param($ValidationResults, $OutputPath)
    
    Write-Step "Creating build manifest..."
    
    $manifest = @{
        BuildInfo = $BuildInfo
        Files = @{
            Required = $ValidationResults.Found | Where-Object { $_.Name -in $BuildInfo.RequiredFiles }
            Optional = $ValidationResults.Optional
        }
        Validation = @{
            TotalFiles = $ValidationResults.Found.Count + $ValidationResults.Optional.Count
            RequiredFound = $ValidationResults.Found.Count
            RequiredMissing = $ValidationResults.Missing.Count
            OptionalFound = $ValidationResults.Optional.Count
            Errors = $ValidationResults.Errors
        }
        Checksums = @{}
    }
    
    # Generate checksums for all files
    foreach ($fileGroup in @($ValidationResults.Found, $ValidationResults.Optional)) {
        foreach ($file in $fileGroup) {
            $manifest.Checksums[$file.Name] = $file.Hash
        }
    }
    
    # Save manifest
    $manifestPath = Join-Path $OutputPath "build-manifest.json"
    $manifest | ConvertTo-Json -Depth 10 | Set-Content $manifestPath -Encoding UTF8
    Write-Success "Manifest saved: $manifestPath"
    
    return $manifest
}

function New-Archive {
    param($ValidationResults, $OutputPath)
    
    Write-Step "Creating archive..."
    
    $archivePath = Join-Path $OutputPath $BuildInfo.OutputArchive
    
    # Remove existing archive if Force is specified
    if ((Test-Path $archivePath) -and $Force) {
        Remove-Item $archivePath -Force
        Write-Warning "Removed existing archive"
    } elseif (Test-Path $archivePath) {
        throw "Archive already exists: $archivePath (use -Force to overwrite)"
    }
    
    # Collect all files to archive
    $filesToArchive = @()
    foreach ($file in $ValidationResults.Found) {
        $filesToArchive += $file.Path
    }
    foreach ($file in $ValidationResults.Optional) {
        $filesToArchive += $file.Path
    }
    
    if ($filesToArchive.Count -eq 0) {
        throw "No files to archive"
    }
    
    Write-Info "Archiving $($filesToArchive.Count) files..."
    
    try {
        # Create archive with maximum compression
        Compress-Archive -Path $filesToArchive -DestinationPath $archivePath -CompressionLevel Optimal
        
        $archiveInfo = Get-Item $archivePath
        $compressionRatio = [math]::Round((1 - ($archiveInfo.Length / ($ValidationResults.Found + $ValidationResults.Optional | Measure-Object Size -Sum).Sum)) * 100, 1)
        
        Write-Success "Archive created: $archivePath"
        Write-Info "  Size: $([math]::Round($archiveInfo.Length/1MB, 2)) MB"
        Write-Info "  Compression: $compressionRatio%"
        Write-Info "  Files: $($filesToArchive.Count)"
        
        return $archivePath
    } catch {
        throw "Failed to create archive: $($_.Exception.Message)"
    }
}

function Test-Archive {
    param($ArchivePath)
    
    Write-Step "Validating archive..."
    
    try {
        # Test archive integrity by attempting to list contents
        $shell = New-Object -ComObject Shell.Application
        $zip = $shell.Namespace($ArchivePath)
        
        if (-not $7z) {
            throw "Could not open archive as 7z file"
        }
        
        $items = $7z.Items()
        $fileCount = $items.Count
        
        Write-Success "Archive validation passed"
        Write-Info "  Files in archive: $fileCount"
        
        # List contents if verbose
        if ($Verbose) {
            Write-Info "Archive contents:"
            foreach ($item in $items) {
                Write-Info "  - $($item.Name) ($([math]::Round($item.Size/1KB, 1)) KB)"
            }
        }
        
        return $true
    } catch {
        Write-Error "Archive validation failed: $($_.Exception.Message)"
        return $false
    } finally {
        # Clean up COM object
        if ($shell) {
            [System.Runtime.Interopservices.Marshal]::ReleaseComObject($shell) | Out-Null
        }
    }
}

function Write-BuildSummary {
    param($ValidationResults, $ArchivePath, $Manifest)
    
    Write-Host "`n" + "="*80 -ForegroundColor Cyan
    Write-Host "file-utils-ii ARCHIVE BUILD SUMMARY" -ForegroundColor Cyan
    Write-Host "="*80 -ForegroundColor Cyan
    
    Write-Host "`nBuild Information:" -ForegroundColor Yellow
    Write-Host "  Name: $($BuildInfo.Name)"
    Write-Host "  Version: $($BuildInfo.Version)"
    Write-Host "  Build Date: $($BuildInfo.BuildDate)"
    
    Write-Host "`nFile Validation:" -ForegroundColor Yellow
    Write-Host "  Required files found: $($ValidationResults.Found.Count)/$($BuildInfo.RequiredFiles.Count)"
    Write-Host "  Optional files found: $($ValidationResults.Optional.Count)/$($BuildInfo.OptionalFiles.Count)"
    Write-Host "  Total files archived: $($ValidationResults.Found.Count + $ValidationResults.Optional.Count)"
    
    if ($ValidationResults.Missing.Count -gt 0) {
        Write-Host "`nMissing Required Files:" -ForegroundColor Red
        $ValidationResults.Missing | ForEach-Object { Write-Host "  - $_" -ForegroundColor Red }
    }
    
    if ($ValidationResults.Errors.Count -gt 0) {
        Write-Host "`nValidation Errors:" -ForegroundColor Red
        $ValidationResults.Errors | ForEach-Object { Write-Host "  - $_" -ForegroundColor Red }
    }
    
    Write-Host "`nOutput:" -ForegroundColor Yellow
    Write-Host "  Archive: $ArchivePath"
    Write-Host "  Manifest: $(Join-Path (Split-Path $ArchivePath) 'build-manifest.json')"
    
    Write-Host "`nNext Steps:" -ForegroundColor Green
    Write-Host "  1. Run Build-fileutilsiiInstaller.ps1 to create self-extracting installer"
    Write-Host "  2. Run Convert-fileutilsii2Exe.ps1 to create final executable"
    
    Write-Host "`n" + "="*80 -ForegroundColor Cyan
}

# Main execution
try {
    Write-Host "file-utils-ii ARCHIVE BUILDER v$($BuildInfo.Version)" -ForegroundColor Cyan
    Write-Host "="*50 -ForegroundColor Cyan
    
    # Test prerequisites
    if (-not (Test-Prerequisites)) {
        exit 1
    }
    
    # Validate source directory
    if (-not (Test-Path $SourceDir)) {
        throw "Source directory not found: $SourceDir"
    }
    
    # Create output directory
    if (-not (Test-Path $OutputDir)) {
        New-Item -ItemType Directory -Path $OutputDir -Force | Out-Null
        Write-Success "Created output directory: $OutputDir"
    }
    
    # Validate source files
    $validationResults = Test-SourceFiles
    
    # Check if we have minimum required files
    if ($validationResults.Missing.Count -gt 0) {
        Write-Error "Cannot proceed with missing required files"
        exit 1
    }
    
    if ($validationResults.Errors.Count -gt 0 -and -not $Force) {
        Write-Error "Validation errors found (use -Force to ignore):"
        $validationResults.Errors | ForEach-Object { Write-Error "  - $_" }
        exit 1
    }
    
    # Create build manifest
    $manifest = New-BuildManifest -ValidationResults $validationResults -OutputPath $OutputDir
    
    # Create archive
    $archivePath = New-Archive -ValidationResults $validationResults -OutputPath $OutputDir
    
    # Validate archive
    if (-not (Test-Archive -ArchivePath $archivePath)) {
        exit 1
    }
    
    # Display summary
    Write-BuildSummary -ValidationResults $validationResults -ArchivePath $archivePath -Manifest $manifest
    
    Write-Success "`nArchive build completed successfully!"
    
} catch {
    Write-Error "Build failed: $($_.Exception.Message)"
    if ($Verbose) {
        Write-Error "Full error: $($_.Exception)"
        Write-Error "Stack trace: $($_.ScriptStackTrace)"
    }
    exit 1
}