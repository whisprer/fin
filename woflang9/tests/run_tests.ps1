# requires: a Release build at .\build\bin\woflang.exe
$Root = Split-Path -Parent (Resolve-Path "$PSScriptRoot\..")
$Bin  = Join-Path $Root "build\bin\woflang.exe"

if (-not (Test-Path $Bin)) {
  Write-Error "Missing $Bin — build first."
  exit 1
}

$pass = 0
$fail = 0
Write-Host "== Woflang test runner =="

Get-ChildItem -Path (Join-Path $Root "tests") -Filter *.wofl | ForEach-Object {
  $t = $_.FullName
  Write-Host "--> $($_.Name)"
  try {
    Get-Content $t | & $Bin *> $null
    if ($LASTEXITCODE -eq 0) {
      Write-Host "    PASS"
      $pass++
    } else {
      Write-Host "    FAIL ($LASTEXITCODE)"
      $fail++
    }
  } catch {
    Write-Host "    FAIL ($_)" 
    $fail++
  }
}

Write-Host "== Summary =="
Write-Host "PASS: $pass"
Write-Host "FAIL: $fail"
exit $fail
