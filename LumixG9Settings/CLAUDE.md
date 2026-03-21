# CLAUDE.md

This file provides guidance to Claude Code (claude.ai/code) when working with code in this repository.

## Project Overview

XLumix G9 Settings — a desktop tool to edit all Panasonic Lumix G9 camera settings (G9 Mark II and G9 Mark I where possible) without using the camera menu, then generate a CamSet DAT file to load onto the camera via SD card.

Two-pronged approach:
1. **WiFi API** (`xlumix.wifi`): Probe and control camera settings over HTTP via `cam.cgi` at `192.168.54.1`. Used for discovery and automated reverse-engineering of the settings format.
2. **CamSet DAT files** (`xlumix.camset`): Read, diff, and write the binary `.DAT` settings files stored in `AD_LUMIX/CAMSET/` on the SD card. This is the primary delivery mechanism — the end goal is a full settings editor that produces ready-to-load DAT files.

## Build & Run

```bash
# Install in editable mode (from project root)
pip install -e ".[dev]"

# Run CLI
xlumix probe --ip 192.168.54.1 --output probe_results.json
xlumix dump --ip 192.168.54.1
xlumix camset read path/to/file.DAT
xlumix camset diff baseline.DAT changed.DAT

# Run tests
pytest
pytest tests/test_camset_differ.py              # single file
pytest tests/test_camset_differ.py::test_group_diffs_single  # single test

# Lint
ruff check src/ tests/
ruff format src/ tests/
```

Requires Python ≥ 3.12. Dependencies: `httpx` (HTTP client), `rich` (terminal UI). Dev deps: `pytest`, `ruff`.

## Architecture

- `src/xlumix/cli.py` — CLI entry point (argparse), dispatches to subcommands. Uses **lazy imports** inside command branches to keep startup fast and avoid importing `httpx`/`rich` when not needed.
- `src/xlumix/wifi/client.py` — `LumixClient` dataclass wrapping `httpx.Client` for all `cam.cgi` requests. Used as a context manager. All camera WiFi communication goes through this class.
- `src/xlumix/wifi/probe.py` — Discovery tool (`run_probe`) that systematically queries camera endpoints. Also contains `run_dump` for quick settings dump.
- `src/xlumix/camset/reader.py` — Binary reader for CamSet DAT files (hex dump, string extraction).
- `src/xlumix/camset/differ.py` — Byte-level diff of two DAT files. `diff_camset()` returns a list of `(offset, byte_a, byte_b)` tuples and prints a Rich table. `_group_diffs()` clusters consecutive changes into regions.
- `data/` — Sample CamSet DAT files for G9 (`data/g9/MKISET_FW2.5.DAT`) and G9 II (`data/g9ii/MKIISET_FW2.7.DAT`), useful for testing binary readers and diffs.

## Lumix WiFi API

All commands: `GET http://192.168.54.1/cam.cgi?mode=X&type=Y&value=Z`
Responses are XML. Key modes: `getinfo`, `getsetting`, `setsetting`, `camcmd`, `getstate`, `accctrl`.

## CamSet DAT Files

Binary files on SD card at `AD_LUMIX/CAMSET/`. Naming: `G9M2Vxx.DAT` where xx relates to firmware version. Requires firmware ≥ V2.3. Loading a CamSet overwrites all camera settings.

The reverse-engineering workflow: change one setting on camera → save CamSet → `xlumix camset diff baseline.DAT changed.DAT` → identify byte offset for that setting.
