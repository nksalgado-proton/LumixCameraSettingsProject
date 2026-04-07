# Build NKS Focus Culler as standalone onefile Windows executable.
#
# Prerequisites:
#   pip install nuitka rawpy Pillow numpy
#   python setup.py  (downloads exiftool to bin/)

$SOURCE_DIR = Split-Path -Parent $MyInvocation.MyCommand.Path
Set-Location $SOURCE_DIR

$OUTPUT_DIR = "$SOURCE_DIR\dist"

# Clean previous build
if (Test-Path $OUTPUT_DIR) {
    Write-Host "Cleaning previous build..." -ForegroundColor Yellow
    Remove-Item -Recurse -Force $OUTPUT_DIR
}

# Collect all .exe and .dll files in bin/ that need explicit inclusion
# (Nuitka's --include-data-dir skips executables and DLLs)
$binaries = Get-ChildItem -Path "bin" -Recurse -Include "*.exe","*.dll" | ForEach-Object {
    $rel = $_.FullName.Substring($SOURCE_DIR.Length + 1).Replace("\", "/")
    "--include-data-files=$rel=$rel"
}

Write-Host "Found $($binaries.Count) binary files to include explicitly" -ForegroundColor Cyan

# Build
Write-Host "Starting Nuitka build..." -ForegroundColor Green
$args = @(
    "--standalone"
    "--onefile"
    "--windows-console-mode=force"
    "--enable-plugin=tk-inter"
    "--follow-imports"
    "--include-package=rawpy"
    "--include-package=PIL"
    "--include-package=numpy"
    "--include-data-dir=bin=bin"
    "--include-data-dir=assets=assets"
) + $binaries + @(
    "--output-dir=$OUTPUT_DIR"
    "--output-filename=nks_focus_culler.exe"
    "--product-name=NKS Focus Culler"
    "--file-description=Visual culling tool for photography"
    "--file-version=1.0.0"
    "--jobs=8"
    "--assume-yes-for-downloads"
    "nks_focus_culler.py"
)

& python -m nuitka @args

if ($LASTEXITCODE -eq 0) {
    $exe = Get-Item "$OUTPUT_DIR\nks_focus_culler.exe"
    $sizeMB = [math]::Round($exe.Length / 1MB, 1)
    Write-Host ""
    Write-Host "===============================================" -ForegroundColor Green
    Write-Host "Build complete: $($exe.FullName)" -ForegroundColor Green
    Write-Host "Size: $sizeMB MB" -ForegroundColor Green
    Write-Host "===============================================" -ForegroundColor Green
} else {
    Write-Host ""
    Write-Host "BUILD FAILED" -ForegroundColor Red
    exit 1
}
