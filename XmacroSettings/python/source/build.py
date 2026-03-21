"""
Build script for Macro Bracketing Calculator.

Usage:
    python build.py            — build single-file .exe (smaller, slower startup)
    python build.py --onedir   — build directory bundle (faster startup)

Output goes to dist/
"""

import subprocess
import sys

def main():
    onedir = "--onedir" in sys.argv

    cmd = [
        sys.executable, "-m", "PyInstaller",
        "--name", "MacroBracketCalc",
        "--windowed",
        "--noconfirm",
        "--clean",
    ]

    if onedir:
        cmd.append("--onedir")
    else:
        cmd.append("--onefile")

    # Hidden imports — PyInstaller sometimes misses sub-packages
    cmd += ["--hidden-import", "core"]
    cmd += ["--hidden-import", "core.calculations"]
    cmd += ["--hidden-import", "core.gear_data"]
    cmd += ["--hidden-import", "ui"]
    cmd += ["--hidden-import", "ui.main_window"]
    cmd += ["--hidden-import", "ui.calibration_dialog"]

    cmd.append("main.py")

    print("Running:", " ".join(cmd))
    subprocess.run(cmd, check=True)
    print("\nDone! Executable is in the dist/ folder.")


if __name__ == "__main__":
    main()