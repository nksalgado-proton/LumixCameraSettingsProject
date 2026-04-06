"""
One-time setup: download and extract ExifTool into bin/ folder.
Run this once after cloning the repo.
"""

import io
import os
import shutil
import sys
import urllib.request
import zipfile
from pathlib import Path

EXIFTOOL_VERSION = '13.54'
EXIFTOOL_URL = f'https://exiftool.org/exiftool-{EXIFTOOL_VERSION}_64.zip'

SCRIPT_DIR = Path(__file__).parent
BIN_DIR = SCRIPT_DIR / 'bin'
EXIFTOOL_EXE = BIN_DIR / 'exiftool.exe'


def main():
    if EXIFTOOL_EXE.exists():
        print(f'ExifTool already installed at {EXIFTOOL_EXE}')
        print('Delete the bin/ folder to force re-download.')
        return

    print(f'Downloading ExifTool {EXIFTOOL_VERSION}...')
    print(f'  From: {EXIFTOOL_URL}')

    req = urllib.request.Request(
        EXIFTOOL_URL, headers={'User-Agent': 'Mozilla/5.0'})
    try:
        data = urllib.request.urlopen(req, timeout=120).read()
    except Exception as e:
        print(f'Download failed: {e}')
        sys.exit(1)

    print(f'  Downloaded {len(data) // 1024} KB')
    print('Extracting...')

    BIN_DIR.mkdir(exist_ok=True)
    with zipfile.ZipFile(io.BytesIO(data)) as z:
        z.extractall(BIN_DIR)

    # Flatten nested folder (exiftool-X.X_64/)
    nested = BIN_DIR / f'exiftool-{EXIFTOOL_VERSION}_64'
    if nested.exists():
        for item in nested.iterdir():
            dst = BIN_DIR / item.name
            if dst.exists():
                if dst.is_dir():
                    shutil.rmtree(dst)
                else:
                    dst.unlink()
            shutil.move(str(item), str(dst))
        nested.rmdir()

    # Rename exiftool(-k).exe → exiftool.exe
    for f in BIN_DIR.iterdir():
        if (f.is_file() and f.suffix.lower() == '.exe'
                and f.name.startswith('exiftool') and '-k' in f.name):
            f.rename(BIN_DIR / 'exiftool.exe')
            print(f'  Renamed {f.name} to exiftool.exe')
            break

    # Verify
    if EXIFTOOL_EXE.exists():
        print(f'\nInstalled: {EXIFTOOL_EXE}')
        print(f'Size: {EXIFTOOL_EXE.stat().st_size // 1024} KB')
        print('\nReady. Run: python nks_focus_culler.py <folder>')
    else:
        print(f'\nERROR: exiftool.exe not found at {EXIFTOOL_EXE}')
        print(f'Contents of bin/: {list(BIN_DIR.iterdir())}')
        sys.exit(1)


if __name__ == '__main__':
    main()
