@echo off
REM Build Burst Culler as a standalone Windows executable with Nuitka.
REM
REM Prerequisites:
REM   pip install nuitka
REM   pip install rawpy Pillow
REM
REM Output: dist\burst_culler.exe (single file, ~80-100MB)

setlocal

set OUTPUT_DIR=dist
if not exist %OUTPUT_DIR% mkdir %OUTPUT_DIR%

python -m nuitka ^
    --onefile ^
    --windows-console-mode=force ^
    --enable-plugin=tk-inter ^
    --include-package=rawpy ^
    --include-package=PIL ^
    --include-data-dir=bin=bin ^
    --output-dir=%OUTPUT_DIR% ^
    --output-filename=burst_culler.exe ^
    --product-name="Burst Culler" ^
    --file-description="Visual culling tool for travel photography" ^
    --file-version=1.0.0 ^
    --assume-yes-for-downloads ^
    burst_culler.py

if errorlevel 1 (
    echo.
    echo BUILD FAILED
    exit /b 1
)

echo.
echo ===============================================
echo Build complete: %OUTPUT_DIR%\burst_culler.exe
echo ===============================================
echo.
echo The executable is self-contained. Copy it to any
echo Windows machine and run from the command line:
echo.
echo   burst_culler.exe "C:\path\to\photos"
echo.
endlocal
