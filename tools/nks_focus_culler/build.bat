@echo off
REM Build NKS Focus Culler as a standalone onefile Windows executable.
REM
REM Prerequisites:
REM   pip install nuitka rawpy Pillow numpy
REM   python setup.py  (downloads exiftool to bin/)

setlocal

set OUTPUT_DIR=dist
if not exist %OUTPUT_DIR% mkdir %OUTPUT_DIR%

REM Clean previous build
if exist %OUTPUT_DIR%\nks_focus_culler.exe del %OUTPUT_DIR%\nks_focus_culler.exe

python -m nuitka ^
    --standalone ^
    --onefile ^
    --windows-console-mode=force ^
    --enable-plugin=tk-inter ^
    --follow-imports ^
    --include-package=rawpy ^
    --include-package=PIL ^
    --include-package=numpy ^
    --include-data-dir=bin=bin ^
    --include-data-files=bin\exiftool.exe=bin\exiftool.exe ^
    --include-data-dir=assets=assets ^
    --output-dir=%OUTPUT_DIR% ^
    --output-filename=nks_focus_culler.exe ^
    --product-name="NKS Focus Culler" ^
    --file-description="Visual culling tool for photography" ^
    --file-version=1.0.0 ^
    --jobs=8 ^
    --assume-yes-for-downloads ^
    nks_focus_culler.py

if errorlevel 1 (
    echo.
    echo BUILD FAILED
    exit /b 1
)

echo.
echo ===============================================
echo Build complete: %OUTPUT_DIR%\nks_focus_culler.exe
echo ===============================================
echo.
endlocal
