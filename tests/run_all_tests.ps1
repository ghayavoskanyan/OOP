$ErrorActionPreference = "Stop"
$here = $PSScriptRoot
$exe = Join-Path $here "..\main\Compiler\calculator.exe"

if (-not (Test-Path $exe)) {
    Write-Host "ERROR: calculator.exe not found at: $exe"
    Write-Host "Build it first:  cd main\Compiler  &&  mingw32-make"
    exit 1
}

$tests = Get-ChildItem -Path $here -Filter "test_*.txt" | Sort-Object Name
if ($tests.Count -eq 0) {
    Write-Host "No test_*.txt files in $here"
    exit 1
}

$failed = 0
foreach ($t in $tests) {
    Write-Host ""
    Write-Host "========================================"
    Write-Host "  $($t.Name)"
    Write-Host "========================================"
    & $exe $t.FullName
    if ($LASTEXITCODE -ne 0) {
        Write-Host "^^^ EXIT CODE: $LASTEXITCODE"
        $failed++
    }
}

Write-Host ""
if ($failed -eq 0) {
    Write-Host "All $($tests.Count) tests finished with exit code 0."
} else {
    Write-Host "$failed test(s) reported non-zero exit."
    exit 1
}
