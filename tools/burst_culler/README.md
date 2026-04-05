# Burst Culler

Fast visual culling tool for travel photography. Groups photos by timestamp into bursts, auto-classifies each burst by scenario (macro, wildlife, BIF, etc.) using EXIF + Panasonic maker notes, and copies your keepers into scenario subfolders.

Designed for the workflow: dump SD cards → run tool → walk away with organized keepers.

## Features

- **Auto burst detection** — groups photos within N seconds into bursts, splits on lens change
- **Scenario classification** — heuristic matches photos to your custom modes (macro, wildlife, bif, landscape, portrait, etc.) using flash state, AF mode, subject detection, shutter speed, aperture, and focal length
- **Focus bracket handling** — auto-keeps ALL frames, routes to `stacks/` subfolder (never culls a stack)
- **Grid mode** — bursts of 2-5 shown side-by-side, click to toggle keepers
- **Tournament mode** — bursts of 6+ shown as pair comparisons, pick winner, runner-up offered at end
- **Switch modes anytime** — Grid / Tournament radio buttons
- **Scenario override** — "Change ▾" button to correct any misclassification
- **Zoom** — right-click any thumbnail for full-screen preview
- **Non-destructive** — originals are never touched, only copies go to keepers folder

## Usage

```bash
python burst_culler.py <folder> [--gap SECONDS] [--out FOLDER]
```

**Arguments:**
- `folder` — folder containing RAW/JPEG photos to cull
- `--gap` — max seconds between photos in the same burst (default 3)
- `--out` — output folder (default: `<folder>/keepers`)

**Example:**
```bash
python burst_culler.py "D:/Photos/Costa-Rica/2026-04-15"
```

Opens a GUI. When you close it, keepers are in `D:/Photos/Costa-Rica/2026-04-15/keepers/<scenario>/`.

## Output structure

```
source_folder/
└── keepers/
    ├── macro/              handheld flash macro
    ├── tripod-macro/       tripod flash bracket macro
    ├── stacks/             focus brackets (all frames)
    ├── wildlife/           wildlife/action
    ├── birds-crop/         birds with crop zoom
    ├── bif/                birds in flight
    ├── landscape/          scenery
    ├── portrait/           people
    ├── street/             general walking-around
    ├── indoor/             low light / silent
    ├── lightning/          long exposure
    └── other/              unclassified
```

## Controls

### Grid mode (bursts of 2-5 photos)
- **Click photo** → toggle select/deselect
- **Right-click photo** → zoom to full-screen preview
- **Commit Selection** button → save selections, advance
- **Keep All** → save every photo in this burst
- **Skip** → save nothing, advance
- **Back** → undo the last committed burst (deletes copies)

Keyboard: **1-9** toggle select, **Enter** commit, **A** keep all, **S** skip, **T** switch to tournament, **Q** quit

### Tournament mode (bursts of 6+ photos)
- **Click photo** (or its "Pick this" button) → that one wins, advances
- **Keep Both** → both advance
- **Skip this round** → neither advances
- At the end: prompt to also keep the runner-up

Keyboard: **L/←** left wins, **R/→** right wins, **Space** keep both, **G** switch to grid, **Q** quit

### Universal
- **Change ▾** (header) → manually override the detected scenario for this burst
- **Quit** → save progress so far, exit

## Supported formats

- RAW: `.rw2`, `.raf`, `.arw`, `.cr2`, `.cr3`, `.nef`, `.orf`, `.dng`, `.raw`, `.srw`, `.pef`
- JPEG: `.jpg`, `.jpeg`

Optimized for Panasonic RW2 with Lumix maker notes. Classification is most accurate for Panasonic cameras (other brands fall back to generic rules).

## How classification works

The tool extracts ~20 EXIF tags including Panasonic maker notes (AF Subject Detection, AF Area Mode, Burst Mode, Bracketing, etc.), then runs rules in order of specificity:

1. **Focus Bracket ON** → `stacks/` (unambiguous)
2. **Long exposure + low fixed ISO** → `lightning/`
3. **Flash + MF + narrow aperture** → `macro/`
4. **Animal subject detect + fast SS** → `bif/` or `wildlife/` or `birds-crop/`
5. **Human subject detect + wide aperture** → `portrait/`
6. **Human + Silent mode** → `indoor/`
7. **AFS + narrow aperture** → `landscape/`
8. **AFS + medium aperture** → `street/`
9. **Telephoto + fast SS + no flash** → `wildlife/` (fallback)
10. **Default** → `other/`

Each classification has a confidence level (HIGH/MEDIUM/LOW). Shown in the header so you can sanity-check.

## Dependencies

```bash
pip install rawpy Pillow
python setup.py          # one-time: downloads ExifTool into bin/
```

ExifTool is downloaded automatically by `setup.py` on first run (~11 MB).

## Files

- `burst_culler.py` — main tool with GUI
- `exif_reader.py` — EXIF extraction via ExifTool
- `classifier.py` — heuristic mode classifier
- `bin/exiftool.exe` — bundled ExifTool binary (v13.54)
- `bin/exiftool_files/` — ExifTool support files

## Building a standalone executable (Nuitka)

To make a portable `.exe` for your laptop:

```bash
pip install nuitka
build.bat
```

Output: `dist\burst_culler.exe` (single file, ~80-100 MB — includes Python runtime, rawpy, Pillow, tkinter, and the bundled exiftool).

The exe is self-contained. Copy it anywhere and run:

```bash
burst_culler.exe "C:\path\to\photos"
```

No Python installation needed on the target machine. First run is slightly slower because Nuitka extracts bundled files to a temp folder.

## Troubleshooting

**No photos found**: Check the folder contains RAW or JPEG files. Preview files (with "preview" in name) are filtered out.

**Wrong scenario detected**: Click "Change ▾" in the header and pick the correct one. The heuristic handles common cases but can miss edge cases — user override always wins.

**Thumbnails missing**: RW2 files need an embedded JPEG preview. If rawpy can't extract one, the photo shows "(no preview)" but is still selectable.

**Back button grayed out**: "Back" only works after at least one burst has been committed.
