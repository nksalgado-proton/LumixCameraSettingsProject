"""
Burst Culler v3 — Universal visual culling tool for photography.

Layout: Left panel | Center workspace | Right panel
Each panel has 4 logical blocks: Dirs, Counts, Photo Info, Actions.
All decisions deferred — files copied only on Commit.
Session auto-saved to %LOCALAPPDATA%/BurstCuller/.

Usage:
    python burst_culler.py               # interactive
    python burst_culler.py <src> --out <dst>
    python burst_culler.py <src> --classify-only --dry-run
"""

import argparse
import io
import random
import shutil
import sys
import tkinter as tk
from collections import Counter, defaultdict
from datetime import datetime
from pathlib import Path
from tkinter import filedialog, messagebox

import rawpy
from PIL import Image, ImageOps, ImageTk

from classifier import SCENARIOS, SCENARIO_ORDER, classify
from exif_reader import PhotoExif, read_exif_batch
from focus_tools import apply_peaking, load_preview, sharpness_score
from session import (BurstRecord, PhotoRecord, Session,
                     load_session, save_session)
from settings import load_settings, save_settings

# ── File types ───────────────────────────────────────────────────────────
RAW_EXT = {'.rw2', '.raf', '.arw', '.cr2', '.cr3', '.nef', '.orf',
           '.dng', '.raw', '.srw', '.pef'}
JPEG_EXT = {'.jpg', '.jpeg'}
PHOTO_EXT = RAW_EXT | JPEG_EXT

# ── Colors — Modern Flat ────────────────────────────────────────────────
BG = '#0f0f14'
BG_PANEL = '#16161e'
BG_CELL = '#1c1c26'
BG_SIDE = '#131319'
FG = '#e8e8f0'
FG_DIM = '#9a9aaa'
FG_SUBTLE = '#5a5a6a'
ACCENT = '#7c3aed'
ACCENT_HOVER = '#a78bfa'
SELECTED = '#3b1e70'
SELECTED_BORDER = '#a78bfa'
DANGER = '#ef4444'
SUCCESS = '#10b981'
WARNING = '#f59e0b'
INFO = '#3b82f6'
BTN_BG = '#252530'
BTN_HOVER = '#35354a'
DIVIDER = '#2a2a38'

ASSETS_DIR = Path(__file__).parent / 'assets'


# ── Scanning ─────────────────────────────────────────────────────────────

def scan_folder(folder: Path) -> list[PhotoExif]:
    print(f'Scanning {folder} (recursive)...')
    files = [p for p in folder.rglob('*')
             if p.is_file() and p.suffix.lower() in PHOTO_EXT
             and 'preview' not in p.name.lower()
             and not any(x.lower() == 'keepers' for x in p.parts)]
    print(f'Found {len(files)} photo files. Reading EXIF...')
    photos = read_exif_batch(files)
    photos = [p for p in photos if p.timestamp]
    photos.sort(key=lambda p: p.timestamp)
    print(f'Read metadata from {len(photos)} photos.')
    return photos


def pair_raw_jpeg(photos: list[PhotoExif]) -> list[tuple[PhotoExif, list[Path]]]:
    by_stem: dict[str, list[PhotoExif]] = defaultdict(list)
    for p in photos:
        by_stem[p.path.stem.lower()].append(p)
    pairs = []
    for files in by_stem.values():
        raw = [f for f in files if f.path.suffix.lower() in RAW_EXT]
        primary = raw[0] if raw else files[0]
        companions = [f.path for f in files if f is not primary]
        pairs.append((primary, companions))
    pairs.sort(key=lambda x: x[0].timestamp)
    paired = sum(1 for _, c in pairs if c)
    if paired:
        print(f'  Paired {paired} RAW+JPEG sets')
    return pairs


def build_session(photos: list[PhotoExif], source: str,
                  destination: str, gap: float) -> Session:
    pairs = pair_raw_jpeg(photos)
    session = Session(source=source, destination=destination,
                      created=datetime.now().isoformat(), gap=gap)
    for primary, companions in pairs:
        c = classify(primary)
        session.photos.append(PhotoRecord(
            path=str(primary.path),
            companions=[str(p) for p in companions],
            timestamp=primary.timestamp.isoformat(),
            classification=c.scenario,
            confidence=c.confidence.value))

    # Group into bursts
    # Focus bracket frames are spaced 1-2s apart (mechanical shutter),
    # so use a wider gap (10s) for consecutive focus-bracket photos.
    # Regular bursts use the tight gap (0.5s default).
    STACK_GAP = 10.0  # seconds between focus bracket frames

    burst_id = 0
    current = [0] if session.photos else []
    for i in range(1, len(session.photos)):
        curr_p = _find_exif(session.photos[i], photos)
        prev_p = _find_exif(session.photos[current[-1]], photos)
        if curr_p and prev_p:
            delta = (curr_p.timestamp - prev_p.timestamp).total_seconds()
            both_bracket = curr_p.is_focus_bracket and prev_p.is_focus_bracket
            same_lens = curr_p.lens == prev_p.lens
            if both_bracket:
                # Focus bracket: wider gap, lens must match
                ok = delta <= STACK_GAP and same_lens
            else:
                # Regular burst: tight gap, both must be burst mode
                ok = (delta <= gap and same_lens
                      and curr_p.burst_mode and prev_p.burst_mode)
        else:
            ok = False
        if ok:
            current.append(i)
        else:
            _add_burst(session, burst_id, current)
            burst_id += 1
            current = [i]
    if current:
        _add_burst(session, burst_id, current)

    # Mark stacks with stack_group but don't auto-complete
    for b in session.bursts:
        p0 = session.photos[b.photo_indices[0]]
        if p0.classification == 'stacks':
            b.stack_group = session.next_stack_id()
            b.mode = 'stack'

    for b in session.bursts:
        for idx in b.photo_indices:
            session.photos[idx].burst_id = b.id

    return session


def _find_exif(rec: PhotoRecord, photos: list[PhotoExif]):
    for p in photos:
        if str(p.path) == rec.path:
            return p
    return None


def _add_burst(session, burst_id, indices):
    n = len(indices)
    mode = 'solo' if n == 1 else ('grid' if n <= 6 else 'tournament')
    session.bursts.append(BurstRecord(id=burst_id,
                                      photo_indices=indices, mode=mode))


# ── Thumbnails ───────────────────────────────────────────────────────────

def extract_thumbnail(path: Path, size: int = 600) -> Image.Image | None:
    if size < 50:
        size = 200
    try:
        if path.suffix.lower() in JPEG_EXT:
            img = Image.open(path)
        else:
            with rawpy.imread(str(path)) as raw:
                thumb = raw.extract_thumb()
                img = (Image.open(io.BytesIO(thumb.data))
                       if thumb.format == rawpy.ThumbFormat.JPEG
                       else Image.fromarray(thumb.data))
        img = ImageOps.exif_transpose(img)
        img.thumbnail((size, size), Image.LANCZOS)
        return img
    except Exception as e:
        print(f'  Thumb error for {path.name}: {e}')
        return None


def format_exif_short(p: PhotoRecord, cache: dict) -> str:
    exif = cache.get(p.path)
    if not exif:
        return ''
    parts = []
    if exif.focal_length:
        parts.append(f'{exif.focal_length:.0f}mm')
    if exif.aperture:
        parts.append(f'f/{exif.aperture:.1f}')
    if exif.shutter_speed:
        if exif.shutter_speed >= 1:
            parts.append(f'{exif.shutter_speed:.1f}s')
        else:
            parts.append(f'1/{1/exif.shutter_speed:.0f}s')
    if exif.iso:
        parts.append(f'ISO{exif.iso}')
    return ' · '.join(parts)


def format_exif_detail(p: PhotoRecord, cache: dict) -> str:
    """Multi-line EXIF for left panel display."""
    exif = cache.get(p.path)
    lines = [Path(p.path).name, '']
    if not exif:
        return '\n'.join(lines)
    if exif.lens:
        lines.append(f'Lens:  {exif.lens}')
    if exif.focal_length:
        lines.append(f'Focal:  {exif.focal_length:.0f}mm'
                     f'  ({exif.focal_length * 2:.0f}mm equiv)')
    lines.append('')
    lines.append('─── Exposure ───')
    if exif.aperture:
        lines.append(f'Aperture:  f/{exif.aperture:.1f}')
    if exif.shutter_speed:
        if exif.shutter_speed >= 1:
            lines.append(f'Shutter:  {exif.shutter_speed:.1f}s')
        else:
            lines.append(f'Shutter:  1/{1/exif.shutter_speed:.0f}s')
    if exif.iso:
        lines.append(f'ISO:  {exif.iso}')
    lines.append('')
    lines.append('─── Focus ───')
    if exif.focus_mode:
        lines.append(f'Mode:  {exif.focus_mode}')
    if exif.af_area_mode:
        lines.append(f'AF Area:  {exif.af_area_mode}')
    if exif.af_subject and exif.af_subject != 'n/a':
        lines.append(f'Subject:  {exif.af_subject}')
    lines.append('')
    lines.append('─── Other ───')
    if exif.flash_fired:
        lines.append('Flash:  Fired')
    if exif.burst_mode:
        lines.append('Drive:  Burst')
    if exif.shutter_type:
        lines.append(f'Shutter:  {exif.shutter_type}')
    if exif.model:
        lines.append(f'Camera:  {exif.model}')
    return '\n'.join(lines)


# ── Main Application ────────────────────────────────────────────────────

class CullerApp:
    def __init__(self):
        self.session = None
        self.photos_cache: dict[str, PhotoExif] = {}
        self.thumb_cache: dict[tuple, ImageTk.PhotoImage] = {}
        self.photo_refs: list = []
        self.tournament_state = None
        self._save_counter = 0
        self._anim_playing = False
        self._anim_frames = []
        self._anim_idx = 0
        self._anim_after_id = None
        self._sharpness_cache: dict[str, float] = {}

        # Load persistent settings
        self._settings = load_settings()
        self._peaking_on = self._settings['peaking_on']
        self._peak_color_name = self._settings['peaking_color']
        self._peak_threshold = int(80 - self._settings['peaking_sensitivity'] * 65)
        self._font_scale = self._settings['font_scale']
        self._anim_speed = self._settings['stack_anim_speed']

        self.root = tk.Tk()
        self.root.title('Burst Culler')
        self.root.geometry('1600x960')
        self.root.configure(bg=BG)
        self.root.state('zoomed')

        self._build_shell()
        # Delay startup screen until window is fully rendered at maximized size
        self.root.update_idletasks()
        self.root.update()
        self.root.after(200, self._show_startup)
        self.root.mainloop()

    # ── Shell (always visible structure) ─────────────────────────────

    def _build_shell(self):
        # Progress bar
        self.progress_canvas = tk.Canvas(
            self.root, bg=BG_PANEL, height=3, highlightthickness=0)
        self.progress_canvas.pack(fill='x', side='top')

        # Main 3 columns
        self.main = tk.Frame(self.root, bg=BG)
        self.main.pack(fill='both', expand=True)

        self.left = tk.Frame(self.main, bg=BG_SIDE, width=220)
        self.left.pack(side='left', fill='y')
        self.left.pack_propagate(False)

        self.right = tk.Frame(self.main, bg=BG_SIDE, width=240)
        self.right.pack(side='right', fill='y')
        self.right.pack_propagate(False)

        self.center = tk.Frame(self.main, bg=BG)
        self.center.pack(side='left', fill='both', expand=True)

        self.root.bind('<Key>', self._on_key)

    # ── Startup screen ───────────────────────────────────────────────

    def _show_startup(self):
        self._clear_all_panels()

        # Left: source selection
        self._section(self.left, 'SOURCE')
        self.src_startup_lbl = tk.Label(
            self.left, text='No folder selected', bg=BG_SIDE, fg=FG_DIM,
            font=('Segoe UI', 10), padx=14, wraplength=200,
            justify='left', anchor='nw')
        self.src_startup_lbl.pack(fill='x', pady=(0, 6))
        self._mkbtn(self.left, '📂  Select Source…',
                     self._startup_pick_source,
                     bg=ACCENT, fg='white', bold=True)

        # Right: destination selection
        self._section(self.right, 'DESTINATION')
        self.dst_startup_lbl = tk.Label(
            self.right, text='No folder selected', bg=BG_SIDE, fg=FG_DIM,
            font=('Segoe UI', 10), padx=14, wraplength=210,
            justify='left', anchor='nw')
        self.dst_startup_lbl.pack(fill='x', pady=(0, 6))
        self.dst_startup_btn = self._mkbtn(
            self.right, '📂  Select Destination…',
            self._startup_pick_dest, bg=ACCENT, fg='white', bold=True)

        # Settings + Quit at bottom of right
        tk.Frame(self.right, bg=BG_SIDE).pack(fill='both', expand=True)
        self._mkbtn(self.right, '⚙  Settings…', self._show_settings,
                     fg=FG_DIM)
        self._mkbtn(self.right, 'Quit', self._quit, fg=DANGER)

        # Center: splash image
        self._show_splash()

        self._startup_source = None
        self._startup_dest = None
        self._startup_source_files = None

        # Pre-fill from last session
        if self._settings.get('last_source'):
            self._startup_source = self._settings['last_source']
            self.src_startup_lbl.config(
                text=self._settings['last_source'], fg=FG)
        if self._settings.get('last_destination'):
            self._startup_dest = self._settings['last_destination']
            self.dst_startup_lbl.config(
                text=self._settings['last_destination'], fg=FG)

    def _show_splash(self):
        for w in self.center.winfo_children():
            w.destroy()
        splash_files = list(ASSETS_DIR.glob('*_splash.*'))
        if splash_files:
            try:
                chosen = random.choice(splash_files)
                img = Image.open(chosen)
                img = ImageOps.exif_transpose(img)
                # Fit to center
                self.root.update_idletasks()
                cw = self.center.winfo_width() or 1100
                ch = self.center.winfo_height() or 800
                img.thumbnail((cw - 40, ch - 120), Image.LANCZOS)
                tk_img = ImageTk.PhotoImage(img)
                lbl = tk.Label(self.center, image=tk_img, bg=BG)
                lbl.image = tk_img
                lbl.pack(expand=True, pady=20)
            except Exception:
                pass
        tk.Label(self.center, text='Burst Culler', bg=BG, fg=FG_SUBTLE,
                 font=('Segoe UI', 28, 'bold')).pack(pady=(10, 4))
        tk.Label(self.center,
                 text='Select source and destination to begin',
                 bg=BG, fg=FG_DIM,
                 font=('Segoe UI', 12)).pack()

    def _startup_pick_source(self):
        """Pick source: right-click menu with Folder or Files options."""
        menu = tk.Menu(self.root, tearoff=0, bg=BG_CELL, fg=FG,
                       activebackground=ACCENT, activeforeground='white',
                       font=('Segoe UI', 11))
        menu.add_command(label='  Select Folder…',
                         command=self._pick_source_folder)
        menu.add_command(label='  Select Files (Ctrl/Shift)…',
                         command=self._pick_source_files)
        try:
            btn = self.left.winfo_children()[1]  # the source button
            x = btn.winfo_rootx()
            y = btn.winfo_rooty() + btn.winfo_height()
            menu.tk_popup(x, y)
        except Exception:
            menu.tk_popup(200, 200)
        finally:
            menu.grab_release()

    def _pick_source_folder(self):
        path = filedialog.askdirectory(title='Select source folder')
        if not path:
            return
        self._startup_source = path
        self._startup_source_files = None
        self.src_startup_lbl.config(text=path, fg=FG)
        self._check_startup_ready()

    def _pick_source_files(self):
        filetypes = [
            ('Photo files', '*.rw2 *.arw *.cr2 *.cr3 *.nef *.orf *.dng '
                            '*.raf *.raw *.jpg *.jpeg'),
            ('All files', '*.*'),
        ]
        files = filedialog.askopenfilenames(
            title='Select photos (Ctrl/Shift for multi-select)',
            filetypes=filetypes)
        if not files:
            return
        # Use parent folder of first file as "source"
        source_dir = str(Path(files[0]).parent)
        self._startup_source = source_dir
        self._startup_source_files = [str(f) for f in files]
        self.src_startup_lbl.config(
            text=f'{len(files)} files from {source_dir}', fg=FG)
        self._check_startup_ready()

    def _startup_pick_dest(self):
        path = filedialog.askdirectory(title='Select destination folder')
        if not path:
            return
        self._startup_dest = path
        self.dst_startup_lbl.config(text=path, fg=FG)
        self._check_startup_ready()

    def _check_startup_ready(self):
        if self._startup_source and self._startup_dest:
            src = self._startup_source
            dst = self._startup_dest
            selected_files = getattr(self, '_startup_source_files', None)

            # Check for existing session (only for folder mode)
            if not selected_files:
                existing = load_session(src, dst)
                if existing:
                    if messagebox.askyesno(
                            'Resume Session',
                            f'Found previous session:\n'
                            f'  {existing.keep_count} kept, '
                            f'{existing.discard_count} discarded, '
                            f'{existing.pending_count} pending\n\n'
                            f'Resume?'):
                        self.session = existing
                        self._set_busy(True)
                        self.root.update()
                        photos = scan_folder(Path(src))
                        self.photos_cache = {str(p.path): p
                                             for p in photos}
                        self._set_busy(False)
                        self._start_culling()
                        return

            self._set_busy(True)
            self.root.update()

            if selected_files:
                # Read EXIF only for selected files
                file_paths = [Path(f) for f in selected_files]
                print(f'Reading EXIF for {len(file_paths)} selected files...')
                photos = read_exif_batch(file_paths)
                photos = [p for p in photos if p.timestamp]
                photos.sort(key=lambda p: p.timestamp)
            else:
                photos = scan_folder(Path(src))

            self._set_busy(False)
            if not photos:
                messagebox.showinfo('Empty', 'No photos found.')
                return
            self.photos_cache = {str(p.path): p for p in photos}
            self.session = build_session(photos, src, dst, 0.5)
            save_session(self.session)
            self._start_culling()

    # ── Main culling UI ──────────────────────────────────────────────

    def _start_culling(self):
        # Save last dirs to settings
        self._settings['last_source'] = self.session.source
        self._settings['last_destination'] = self.session.destination
        save_settings(self._settings)

        self._clear_all_panels()
        self._build_left()
        self._build_right()
        self._build_center()
        self.root.update_idletasks()
        self.root.update()
        self.root.after(100, lambda: self._navigate_to(
            self.session.current_burst_idx))

    def _build_left(self):
        """Left panel: Block 1 (dir), Block 2 (counts), Block 3 (EXIF), Block 4 (actions)."""
        # Block 1 — Directory
        self._section(self.left, 'SOURCE')
        self.src_lbl = tk.Label(
            self.left, text=self.session.source, bg=BG_SIDE, fg=FG,
            font=('Consolas', 9), anchor='nw', padx=14,
            wraplength=200, justify='left')
        self.src_lbl.pack(fill='x')
        self._mkbtn(self.left, 'Change Source…',
                     self._change_source, fg=FG_DIM)
        self._divider(self.left)

        # Block 2 — Counts
        self._section(self.left, 'SOURCE COUNTS')
        self.l_photos = self._info_label(self.left)
        self.l_bursts = self._info_label(self.left)
        self.l_singles = self._info_label(self.left)
        self.l_stacks = self._info_label(self.left)
        self._divider(self.left)

        # Block 3 — Photo Info (EXIF)
        self._section(self.left, 'PHOTO INFO')
        self.exif_lbl = tk.Label(
            self.left, text='Click a photo to see details',
            bg=BG_SIDE, fg=FG_DIM,
            font=('Segoe UI', 9), anchor='nw', padx=14,
            wraplength=200, justify='left')
        self.exif_lbl.pack(fill='x')

        # Block 5 — Previous burst preview
        tk.Frame(self.left, bg=BG_SIDE).pack(fill='both', expand=True)
        self._divider(self.left)
        tk.Label(self.left, text='← PREVIOUS', bg=BG_SIDE, fg=FG_SUBTLE,
                 font=('Segoe UI', 8, 'bold'), padx=14).pack(fill='x')
        self.prev_thumb_lbl = tk.Label(
            self.left, bg=BG_SIDE, cursor='hand2')
        self.prev_thumb_lbl.pack(padx=14, pady=4)
        self.prev_thumb_lbl.bind('<Button-1>', lambda e: self._back())

        # Block 4 — Actions (at bottom)
        self._divider(self.left)
        self._mkbtn(self.left, '🗑  Clear Source…',
                     self._clear_source, fg=DANGER)

    def _build_right(self):
        """Right panel: Block 1 (dir), Block 2 (dest counts), Block 3 (scenario), Block 4 (actions)."""
        # Block 1 — Directory
        self._section(self.right, 'DESTINATION')
        self.dst_lbl = tk.Label(
            self.right, text=self.session.destination, bg=BG_SIDE, fg=FG,
            font=('Consolas', 9), anchor='nw', padx=14,
            wraplength=210, justify='left')
        self.dst_lbl.pack(fill='x')
        self._mkbtn(self.right, 'Change Destination…',
                     self._change_destination, fg=FG_DIM)
        self._divider(self.right)

        # Block 2 — Destination counts
        self._section(self.right, 'DECISIONS')
        self.r_keep = tk.Label(
            self.right, text='', bg=BG_SIDE, fg=SUCCESS,
            font=('Segoe UI', 11, 'bold'), anchor='w', padx=14)
        self.r_keep.pack(fill='x')
        self.r_discard = tk.Label(
            self.right, text='', bg=BG_SIDE, fg=DANGER,
            font=('Segoe UI', 11, 'bold'), anchor='w', padx=14)
        self.r_discard.pack(fill='x')
        self.r_pending = tk.Label(
            self.right, text='', bg=BG_SIDE, fg=FG,
            font=('Segoe UI', 11), anchor='w', padx=14)
        self.r_pending.pack(fill='x')
        self._divider(self.right)

        # Block 3 — Scenario (classification)
        self._section(self.right, 'CLASSIFICATION')
        self.scenario_list = tk.Frame(self.right, bg=BG_SIDE)
        self.scenario_list.pack(fill='x', padx=10)
        self._scenario_labels = {}
        for key in SCENARIO_ORDER:
            desc = SCENARIOS[key]
            row = tk.Frame(self.scenario_list, bg=BG_SIDE)
            row.pack(fill='x', pady=1)
            lbl = tk.Label(row, text=f'  {desc}', bg=BG_SIDE, fg=FG_SUBTLE,
                           font=('Segoe UI', 9), anchor='w', padx=4,
                           cursor='hand2')
            lbl.pack(fill='x')
            lbl.bind('<Button-1>',
                     lambda e, k=key: self._set_scenario(k))
            self._scenario_labels[key] = lbl
        self._divider(self.right)

        # Block 4 — Actions (at bottom)
        # Block 5 — Next burst preview
        tk.Frame(self.right, bg=BG_SIDE).pack(fill='both', expand=True)
        self._divider(self.right)
        tk.Label(self.right, text='NEXT →', bg=BG_SIDE, fg=FG_SUBTLE,
                 font=('Segoe UI', 8, 'bold'), padx=14).pack(fill='x')
        self.next_thumb_lbl = tk.Label(
            self.right, bg=BG_SIDE, cursor='hand2')
        self.next_thumb_lbl.pack(padx=14, pady=4)
        self.next_thumb_lbl.bind('<Button-1>', lambda e: self._skip_burst())

        # Block 4 — Actions at bottom
        self._divider(self.right)
        self._mkbtn(self.right, 'Accept All Remaining',
                     self._accept_all, fg=SUCCESS)
        self._mkbtn(self.right, 'Reject All Remaining',
                     self._reject_all, fg=DANGER)
        self._mkbtn(self.right, 'Reset All',
                     self._reset_all, fg=WARNING)
        self._divider(self.right)
        self._mkbtn(self.right, '✓  Commit & Copy',
                     self._commit, bg=SUCCESS, fg='white', bold=True)
        self._mkbtn(self.right, '⚙  Settings…', self._show_settings,
                     fg=FG_DIM)
        self._mkbtn(self.right, 'Quit', self._quit, fg=DANGER)

    def _build_center(self):
        """Center: mode banner, burst info, workspace, action bar."""
        self.mode_banner = tk.Label(
            self.center, text='', bg=INFO, fg='white',
            font=('Segoe UI', 18, 'bold'), pady=8)
        self.mode_banner.pack(fill='x')

        # Info + tools row
        info_row = tk.Frame(self.center, bg=BG)
        info_row.pack(fill='x')

        self.burst_info = tk.Label(
            info_row, text='', bg=BG, fg=FG_DIM,
            font=('Segoe UI', 11), pady=4)
        self.burst_info.pack(side='left', padx=16)

        # Peaking controls (right side of info row)
        peak_frame = tk.Frame(info_row, bg=BG)
        peak_frame.pack(side='right', padx=16)

        # Color selector
        self._peak_colors = {
            'Red': (255, 50, 50),
            'Yellow': (255, 230, 0),
            'Green': (100, 255, 80),
            'Cyan': (0, 220, 255),
            'White': (255, 255, 255),
        }
        self._peak_color_name = 'Red'
        self._peak_threshold = 50  # stricter default

        # Peaking toggle
        self.peak_btn = tk.Button(
            peak_frame, text='🔍 Peaking: OFF', command=self._toggle_peaking,
            bg=BTN_BG, fg=FG_DIM,
            activebackground=BTN_HOVER, activeforeground=FG,
            font=('Segoe UI', 10, 'bold'), bd=0, padx=14, pady=6,
            cursor='hand2')
        self.peak_btn.pack(side='left', padx=4)

        # Color selector
        self.peak_color_btn = tk.Button(
            peak_frame, text='Red ▾', command=self._cycle_peak_color,
            bg=BTN_BG, fg='#ff3232',
            activebackground=BTN_HOVER, activeforeground=FG,
            font=('Segoe UI', 9, 'bold'), bd=0, padx=10, pady=6,
            cursor='hand2')
        self.peak_color_btn.pack(side='left', padx=4)

        # Sensitivity slider: 0 (strict) to 1 (loose)
        # Maps to threshold: 0→80 (very strict), 1→15 (very loose)
        tk.Label(peak_frame, text='Strict', bg=BG, fg=FG_SUBTLE,
                 font=('Segoe UI', 8)).pack(side='left', padx=(8, 2))
        self._peak_sens_var = tk.DoubleVar(value=0.4)
        self._peak_slider = tk.Scale(
            peak_frame, from_=0.0, to=1.0, resolution=0.05,
            orient='horizontal', length=120,
            variable=self._peak_sens_var,
            command=self._on_peak_slider,
            bg=BG, fg=FG, troughcolor=BG_CELL,
            activebackground=ACCENT,
            highlightthickness=0, bd=0,
            font=('Segoe UI', 8), showvalue=False)
        self._peak_slider.pack(side='left', padx=2)
        tk.Label(peak_frame, text='Loose', bg=BG, fg=FG_SUBTLE,
                 font=('Segoe UI', 8)).pack(side='left', padx=(2, 4))

        # Action bar packed BEFORE workspace (bottom) so buttons always visible
        self.action_bar = tk.Frame(self.center, bg=BG_PANEL, height=64)
        self.action_bar.pack(fill='x', side='bottom')
        self.action_bar.pack_propagate(False)

        # Workspace fills remaining space
        # Solo: [Discard] [Back] [Next] [Keep]
        self.solo_bar = tk.Frame(self.action_bar, bg=BG_PANEL)
        self._action_btn(self.solo_bar, '✗  Discard', self._solo_discard,
                         bg=DANGER, fg='white')
        self._action_btn(self.solo_bar, '←  Back', self._back,
                         bg=BTN_BG, fg=FG_DIM)
        self._action_btn(self.solo_bar, 'Skip  →', self._skip_burst,
                         bg=BTN_BG, fg=FG_DIM)
        self._action_btn(self.solo_bar, '✓  Keep', self._solo_keep,
                         bg=SUCCESS, fg='white')

        # Grid: [Discard All] [Back] [Next] [Keep All]
        self.grid_bar = tk.Frame(self.action_bar, bg=BG_PANEL)
        self._action_btn(self.grid_bar, '✗  Discard All',
                         self._burst_discard_all, bg=DANGER, fg='white')
        self._action_btn(self.grid_bar, '←  Back', self._back,
                         bg=BTN_BG, fg=FG_DIM)
        self._action_btn(self.grid_bar, 'Skip  →', self._skip_burst,
                         bg=BTN_BG, fg=FG_DIM)
        self._action_btn(self.grid_bar, '✓  Keep All',
                         self._burst_keep_all, bg=SUCCESS, fg='white')

        # Tournament: [Discard Both] [Back to Grid] [Keep Both]
        self.tour_bar = tk.Frame(self.action_bar, bg=BG_PANEL)
        self._action_btn(self.tour_bar, '✗  Discard Both',
                         self._tour_discard, bg=DANGER, fg='white')
        self._action_btn(self.tour_bar, '←  Grid View',
                         self._tour_to_grid, bg=BTN_BG, fg=FG_DIM)
        self._action_btn(self.tour_bar, '✓  Keep Both',
                         self._tour_both, bg=SUCCESS, fg='white')

        # Winner review: [Reject All] [Reject] [Back] [Accept] [Keep All]
        self.winner_bar = tk.Frame(self.action_bar, bg=BG_PANEL)
        self._action_btn(self.winner_bar, '✗  Reject All',
                         self._burst_discard_all, bg='#7a2020', fg='white')
        self._action_btn(self.winner_bar, '✗  Reject Winner',
                         self._winner_reject, bg=DANGER, fg='white')
        self._action_btn(self.winner_bar, '←  Back',
                         self._back, bg=BTN_BG, fg=FG_DIM)
        self._action_btn(self.winner_bar, '✓  Accept Winner',
                         self._winner_accept, bg=SUCCESS, fg='white')
        self._action_btn(self.winner_bar, '✓  Keep All',
                         self._burst_keep_all, bg='#0a7a5a', fg='white')

        # Stack review: [Discard Stack] [Back] [Keep Stack]
        self.stack_bar = tk.Frame(self.action_bar, bg=BG_PANEL)
        self._action_btn(self.stack_bar, '✗  Discard Stack',
                         self._stack_discard, bg=DANGER, fg='white')
        self._action_btn(self.stack_bar, '←  Back',
                         self._back, bg=BTN_BG, fg=FG_DIM)
        self._action_btn(self.stack_bar, '✓  Keep Stack',
                         self._stack_keep, bg=SUCCESS, fg='white')

    # ── Widget helpers ───────────────────────────────────────────────

    def _font(self, size: int, bold: bool = False) -> tuple:
        """Return a scaled font tuple."""
        scaled = max(7, int(size * self._font_scale))
        return ('Segoe UI', scaled, 'bold') if bold else ('Segoe UI', scaled)

    def _mono(self, size: int, bold: bool = False) -> tuple:
        scaled = max(7, int(size * self._font_scale))
        return ('Consolas', scaled, 'bold') if bold else ('Consolas', scaled)

    def _section(self, parent, title):
        tk.Label(parent, text=title, bg=parent['bg'], fg=FG_SUBTLE,
                 font=self._font(9, bold=True), anchor='w',
                 padx=14, pady=6).pack(fill='x')

    def _divider(self, parent):
        tk.Frame(parent, bg=DIVIDER, height=1).pack(
            fill='x', padx=12, pady=6)

    def _info_label(self, parent) -> tk.Label:
        lbl = tk.Label(parent, text='', bg=parent['bg'], fg=FG,
                       font=('Segoe UI', 10), anchor='w', padx=14)
        lbl.pack(fill='x')
        return lbl

    def _mkbtn(self, parent, text, cmd, bg=None, fg=None, bold=False):
        font = ('Segoe UI', 11, 'bold') if bold else ('Segoe UI', 11)
        btn = tk.Button(parent, text=text, command=cmd,
                        bg=bg or BTN_BG, fg=fg or FG,
                        activebackground=BTN_HOVER, activeforeground=FG,
                        font=font, bd=0, padx=16, pady=10,
                        cursor='hand2', relief='flat')
        btn.pack(fill='x', padx=12, pady=3)
        return btn

    def _action_btn(self, parent, text, cmd, bg=None, fg=None):
        """Center action bar button — expands horizontally."""
        btn = tk.Button(parent, text=text, command=cmd,
                        bg=bg or BTN_BG, fg=fg or FG,
                        activebackground=BTN_HOVER, activeforeground=FG,
                        font=('Segoe UI', 12, 'bold'), bd=0,
                        padx=16, pady=12, cursor='hand2', relief='flat')
        btn.pack(side='left', fill='both', expand=True, padx=4, pady=8)
        return btn

    def _toggle_peaking(self):
        self._peaking_on = not self._peaking_on
        if self._peaking_on:
            self.peak_btn.config(text='🔍 Peaking: ON', bg=DANGER, fg='white')
        else:
            self.peak_btn.config(text='🔍 Peaking: OFF', bg=BTN_BG, fg=FG_DIM)
        self.thumb_cache.clear()
        self.photo_refs.clear()
        if self.session:
            self._render_burst()

    def _on_peak_slider(self, _value=None):
        """Update peaking threshold from slider position."""
        # Map 0.0 (strict) → threshold 80, 1.0 (loose) → threshold 15
        sens = self._peak_sens_var.get()
        self._peak_threshold = int(80 - sens * 65)
        if self._peaking_on:
            self.thumb_cache.clear()
            self.photo_refs.clear()
            if self.session:
                self._render_burst()

    def _cycle_peak_color(self):
        """Cycle through peaking highlight colors."""
        names = list(self._peak_colors.keys())
        idx = names.index(self._peak_color_name)
        self._peak_color_name = names[(idx + 1) % len(names)]
        rgb = self._peak_colors[self._peak_color_name]
        hex_color = f'#{rgb[0]:02x}{rgb[1]:02x}{rgb[2]:02x}'
        self.peak_color_btn.config(
            text=f'{self._peak_color_name} ▾', fg=hex_color)
        # Re-render if peaking is on
        if self._peaking_on:
            self.thumb_cache.clear()
            self.photo_refs.clear()
            if self.session:
                self._render_burst()

    def _get_sharpness(self, path: Path) -> float:
        """Get cached sharpness score for a photo."""
        key = str(path)
        if key not in self._sharpness_cache:
            img = load_preview(path, 800)
            self._sharpness_cache[key] = sharpness_score(img) if img else 0
        return self._sharpness_cache[key]

    def _set_busy(self, busy: bool):
        """Toggle busy/normal cursor for the entire app."""
        cursor = 'wait' if busy else ''
        self.root.config(cursor=cursor)
        self.root.update_idletasks()

    def _clear_all_panels(self):
        for panel in (self.left, self.right, self.center):
            for w in panel.winfo_children():
                w.destroy()

    # ── Navigation ───────────────────────────────────────────────────

    def _navigate_to(self, idx: int):
        while (idx < self.session.total_bursts
               and self.session.bursts[idx].status == 'completed'):
            idx += 1
        if idx >= self.session.total_bursts:
            self._show_done()
            return
        self.session.current_burst_idx = idx
        self.session.bursts[idx].status = 'in_progress'
        self._update_panels()
        self._render_burst()
        self._auto_save()

    def _render_burst(self):
        # Stop any running animation
        if self._anim_after_id:
            self.root.after_cancel(self._anim_after_id)
            self._anim_after_id = None
        self._anim_playing = False

        for w in self.workspace.winfo_children():
            w.destroy()
        self.photo_refs.clear()
        self.tournament_state = None

        idx = self.session.current_burst_idx
        burst = self.session.bursts[idx]
        n = len(burst.photo_indices)

        if burst.mode == 'stack':
            pass  # keep as stack
        elif n == 1:
            burst.mode = 'solo'
        elif n <= 6:
            burst.mode = burst.mode if burst.mode in ('grid', 'tournament') else 'grid'
        else:
            burst.mode = burst.mode if burst.mode in ('grid', 'tournament') else 'tournament'

        # Mode banner
        banners = {
            'solo': (INFO, 'SINGLE PHOTO'),
            'grid': (ACCENT, f'GRID  —  {n} photos'),
            'tournament': (WARNING, f'TOURNAMENT  —  {n} photos'),
            'stack': (SUCCESS, f'FOCUS STACK  —  {n} frames'),
        }
        color, text = banners.get(burst.mode, (INFO, burst.mode))
        self.mode_banner.config(text=text, bg=color)
        self.burst_info.config(
            text=f'Burst {idx+1} of {self.session.total_bursts}')

        # Swap action bars
        for bar in (self.solo_bar, self.grid_bar, self.tour_bar,
                    self.winner_bar, self.stack_bar):
            bar.pack_forget()

        if burst.mode == 'stack':
            self.stack_bar.pack(fill='x')
            self._render_stack(burst)
        elif burst.mode == 'solo':
            self.solo_bar.pack(fill='x')
            self._render_solo(burst)
        elif burst.mode == 'grid':
            self.grid_bar.pack(fill='x')
            self._render_grid(burst)
        else:
            self.tour_bar.pack(fill='x')
            self._render_tournament(burst)

    # ── Stack ────────────────────────────────────────────────────────

    def _render_stack(self, burst):
        """Show stack as animated player — focus moves across the subject."""
        p0 = self.session.photos[burst.photo_indices[0]]
        self._show_exif(p0)

        indices = burst.photo_indices
        n = len(indices)

        # Header
        tk.Label(self.workspace,
                 text=f'Focus Stack — {n} frames',
                 bg=BG, fg=FG_DIM,
                 font=('Segoe UI', 11)).pack(pady=(4, 4))

        # Animation frame
        anim_frame = tk.Frame(self.workspace, bg=BG_CELL, padx=10, pady=10,
                              highlightbackground=SUCCESS,
                              highlightcolor=SUCCESS,
                              highlightthickness=2)
        anim_frame.pack(expand=True, padx=30, pady=6)

        # Frame counter
        self._anim_counter = tk.Label(
            anim_frame, text='', bg=BG_CELL, fg=FG_DIM,
            font=('Consolas', 10))
        self._anim_counter.pack(pady=(0, 4))

        # Image display
        self._anim_label = tk.Label(anim_frame, bg=BG_CELL)
        self._anim_label.pack()

        # Controls row
        ctrl = tk.Frame(self.workspace, bg=BG)
        ctrl.pack(pady=6)

        self._anim_play_btn = tk.Button(
            ctrl, text='▶  Play', command=self._anim_toggle,
            bg=SUCCESS, fg='white', activebackground='#0da373',
            font=('Segoe UI', 11, 'bold'), bd=0, padx=20, pady=8,
            cursor='hand2')
        self._anim_play_btn.pack(side='left', padx=6)

        tk.Button(ctrl, text='◀', command=self._anim_prev,
                  bg=BTN_BG, fg=FG, font=('Segoe UI', 12, 'bold'),
                  bd=0, padx=14, pady=8, cursor='hand2').pack(
            side='left', padx=2)
        tk.Button(ctrl, text='▶', command=self._anim_next_frame,
                  bg=BTN_BG, fg=FG, font=('Segoe UI', 12, 'bold'),
                  bd=0, padx=14, pady=8, cursor='hand2').pack(
            side='left', padx=2)

        # Speed control
        tk.Label(ctrl, text='Speed:', bg=BG, fg=FG_DIM,
                 font=('Segoe UI', 10)).pack(side='left', padx=(16, 4))
        self._speed_var = tk.IntVar(value=5)
        speeds = [(1, 'Slow'), (3, 'Med'), (5, 'Fast'), (10, 'Max')]
        for val, label in speeds:
            tk.Radiobutton(
                ctrl, text=label, variable=self._speed_var, value=val,
                bg=BG, fg=FG_DIM, selectcolor=BG_CELL,
                activebackground=BG, activeforeground=FG,
                font=('Segoe UI', 9),
                command=self._anim_speed_changed).pack(side='left', padx=2)

        # Load all frames
        aw = self.workspace.winfo_width() or 1200
        ah = self.workspace.winfo_height() or 700
        thumb_size = max(300, min(aw - 100, ah - 200, 900))

        self._anim_frames = []
        self._anim_tk_frames = []
        for pi in indices:
            photo = self.session.photos[pi]
            img = load_preview(Path(photo.path), thumb_size)
            if img:
                if self._peaking_on:
                    color = self._peak_colors[self._peak_color_name]
                    img = apply_peaking(img, color=color,
                                        threshold=self._peak_threshold,
                                        opacity=0.7)
                self._anim_frames.append(img)
                tk_img = ImageTk.PhotoImage(img)
                self._anim_tk_frames.append(tk_img)
                self.photo_refs.append(tk_img)

        self._anim_idx = 0
        self._anim_playing = False
        self._anim_show_frame()

    def _anim_show_frame(self):
        """Display current animation frame."""
        if not self._anim_tk_frames:
            return
        idx = self._anim_idx % len(self._anim_tk_frames)
        self._anim_label.config(image=self._anim_tk_frames[idx])
        n = len(self._anim_tk_frames)
        self._anim_counter.config(text=f'Frame {idx + 1} / {n}')

    def _anim_toggle(self):
        """Play/pause animation."""
        if self._anim_playing:
            self._anim_playing = False
            self._anim_play_btn.config(text='▶  Play', bg=SUCCESS)
            if self._anim_after_id:
                self.root.after_cancel(self._anim_after_id)
                self._anim_after_id = None
        else:
            self._anim_playing = True
            self._anim_play_btn.config(text='⏸  Pause', bg=WARNING)
            self._anim_advance()

    def _anim_advance(self):
        """Advance to next frame (called by timer)."""
        if not self._anim_playing or not self._anim_tk_frames:
            return
        self._anim_idx = (self._anim_idx + 1) % len(self._anim_tk_frames)
        self._anim_show_frame()
        speed_fps = self._speed_var.get()
        delay = max(50, 1000 // speed_fps)
        self._anim_after_id = self.root.after(delay, self._anim_advance)

    def _anim_next_frame(self):
        """Step forward one frame."""
        if self._anim_tk_frames:
            self._anim_idx = (self._anim_idx + 1) % len(self._anim_tk_frames)
            self._anim_show_frame()

    def _anim_prev(self):
        """Step backward one frame."""
        if self._anim_tk_frames:
            self._anim_idx = (self._anim_idx - 1) % len(self._anim_tk_frames)
            self._anim_show_frame()

    def _anim_speed_changed(self):
        """Update animation speed without restart."""
        pass  # next _anim_advance call will use new speed

    def _stack_keep(self):
        idx = self.session.current_burst_idx
        burst = self.session.bursts[idx]
        all_sel = set(range(len(burst.photo_indices)))
        self.session.mark_burst_photos(idx, all_sel, 'stacks')
        self._navigate_to(idx + 1)

    def _stack_discard(self):
        idx = self.session.current_burst_idx
        self.session.mark_burst_photos(idx, set(), 'stacks')
        self._navigate_to(idx + 1)

    # ── Solo ─────────────────────────────────────────────────────────

    def _render_solo(self, burst):
        photo = self.session.photos[burst.photo_indices[0]]
        self._show_exif(photo)

        frame = tk.Frame(self.workspace, bg=BG_CELL, padx=20, pady=16,
                         highlightbackground=FG_SUBTLE,
                         highlightcolor=FG_SUBTLE, highlightthickness=1)
        frame.pack(expand=True, padx=20, pady=10)

        tk.Label(frame, text=Path(photo.path).name, bg=BG_CELL,
                 fg=ACCENT, font=('Consolas', 11, 'bold')).pack(
            pady=(0, 4))
        tk.Label(frame, text=format_exif_short(photo, self.photos_cache),
                 bg=BG_CELL, fg=FG_DIM,
                 font=('Segoe UI', 10)).pack(pady=(0, 8))

        h = self.workspace.winfo_height() - 100
        w = self.workspace.winfo_width() - 80
        size = max(200, min(w, h, 1000))
        tk_img = self._thumb(Path(photo.path), size)
        if tk_img:
            lbl = tk.Label(frame, image=tk_img, bg=BG_CELL)
            lbl.pack()
            lbl.bind('<Button-3>',
                     lambda e: self._zoom(Path(photo.path), photo))

    def _solo_keep(self):
        idx = self.session.current_burst_idx
        self.session.mark_burst_photos(idx, {0}, self._cur_scenario())
        self._navigate_to(idx + 1)

    def _solo_discard(self):
        idx = self.session.current_burst_idx
        self.session.mark_burst_photos(idx, set(), self._cur_scenario())
        self._navigate_to(idx + 1)

    # ── Grid ─────────────────────────────────────────────────────────

    def _render_grid(self, burst):
        n = len(burst.photo_indices)
        cols = n if n <= 3 else (2 if n == 4 else 3)
        rows = (n + cols - 1) // cols
        aw = self.workspace.winfo_width() or 1200
        ah = self.workspace.winfo_height() or 700
        thumb = max(200, min((aw // cols) - 40, (ah // rows) - 60))

        for li, pi in enumerate(burst.photo_indices):
            photo = self.session.photos[pi]
            r, c = li // cols, li % cols
            cell = tk.Frame(self.workspace, bg=BG_CELL,
                            highlightbackground=FG_SUBTLE,
                            highlightcolor=FG_SUBTLE,
                            highlightthickness=2, padx=8, pady=6)
            cell.grid(row=r, column=c, padx=6, pady=6, sticky='nsew')

            # Sharpness score
            score = self._get_sharpness(Path(photo.path))
            score_color = SUCCESS if score > 400 else (WARNING if score > 200 else DANGER)

            header = tk.Frame(cell, bg=BG_CELL)
            header.pack(fill='x', pady=(0, 2))
            tk.Label(header, text=f'[{li+1}]  {Path(photo.path).name}',
                     bg=BG_CELL, fg=ACCENT,
                     font=('Consolas', 9, 'bold'), anchor='w').pack(
                side='left')
            tk.Label(header, text=f'S:{score:.0f}',
                     bg=BG_CELL, fg=score_color,
                     font=('Consolas', 9, 'bold'), anchor='e').pack(
                side='right')

            tk.Label(cell,
                     text=format_exif_short(photo, self.photos_cache),
                     bg=BG_CELL, fg=FG_DIM,
                     font=('Segoe UI', 8), anchor='w').pack(
                fill='x', pady=(0, 4))

            tk_img = self._thumb(Path(photo.path), thumb)
            if tk_img:
                il = tk.Label(cell, image=tk_img, bg=BG_CELL,
                              cursor='hand2')
                il.pack()
                il.bind('<Button-1>',
                        lambda e, p=photo: self._show_exif(p))
                il.bind('<Double-Button-1>',
                        lambda e, i=li: self._grid_pick(i))
                il.bind('<Button-3>',
                        lambda e, p=photo: self._zoom(
                            Path(p.path), p))

        for c in range(cols):
            self.workspace.columnconfigure(c, weight=1)

    def _grid_pick(self, local_idx):
        idx = self.session.current_burst_idx
        self.session.mark_burst_photos(
            idx, {local_idx}, self._cur_scenario())
        self._navigate_to(idx + 1)

    def _burst_keep_all(self):
        idx = self.session.current_burst_idx
        burst = self.session.bursts[idx]
        self.session.mark_burst_photos(
            idx, set(range(len(burst.photo_indices))),
            self._cur_scenario())
        self._navigate_to(idx + 1)

    def _burst_discard_all(self):
        idx = self.session.current_burst_idx
        self.session.mark_burst_photos(
            idx, set(), self._cur_scenario())
        self._navigate_to(idx + 1)

    # ── Tournament ───────────────────────────────────────────────────

    def _render_tournament(self, burst):
        if (not self.tournament_state
                or self.tournament_state['burst_idx']
                != self.session.current_burst_idx):
            self.tournament_state = {
                'burst_idx': self.session.current_burst_idx,
                'queue': list(range(len(burst.photo_indices))),
                'next_round': [], 'match_num': 1,
                'total': len(burst.photo_indices),
                'winner': None, 'runner_up': None}
        self._tour_next()

    def _tour_next(self):
        s = self.tournament_state
        burst = self.session.bursts[s['burst_idx']]

        while len(s['queue']) < 2:
            if s['next_round']:
                if len(s['next_round']) == 1 and not s['queue']:
                    s['winner'] = s['next_round'][0]
                    self._tour_show_winner()
                    return
                s['queue'] = s['next_round'] + s['queue']
                s['next_round'] = []
            elif len(s['queue']) == 1:
                s['winner'] = s['queue'][0]
                self._tour_show_winner()
                return
            else:
                self._burst_discard_all()
                return

        for w in self.workspace.winfo_children():
            w.destroy()
        self.photo_refs.clear()

        left = s['queue'].pop(0)
        right = s['queue'].pop(0)
        s['_pair'] = (left, right)

        alive = 2 + len(s['queue']) + len(s['next_round'])
        done = s['total'] - alive
        tk.Label(self.workspace,
                 text=f'Match {done+1} of ~{s["total"]-1}'
                      f'  ·  {alive} photos remain',
                 bg=BG, fg=FG_DIM,
                 font=('Segoe UI', 11)).pack(pady=(4, 8))

        pair = tk.Frame(self.workspace, bg=BG)
        pair.pack(expand=True, fill='both', padx=10)
        pair.columnconfigure(0, weight=1)
        pair.columnconfigure(1, weight=1)

        for col, li in enumerate([left, right]):
            pi = burst.photo_indices[li]
            photo = self.session.photos[pi]
            cell = tk.Frame(pair, bg=BG_CELL, padx=16, pady=12,
                            highlightbackground=FG_SUBTLE,
                            highlightcolor=FG_SUBTLE,
                            highlightthickness=1)
            cell.grid(row=0, column=col, padx=10, pady=8, sticky='nsew')

            score = self._get_sharpness(Path(photo.path))
            score_color = SUCCESS if score > 400 else (WARNING if score > 200 else DANGER)

            th = tk.Frame(cell, bg=BG_CELL)
            th.pack(fill='x', pady=(0, 2))
            tk.Label(th, text=f'[{li+1}]  {Path(photo.path).name}',
                     bg=BG_CELL, fg=ACCENT,
                     font=('Consolas', 10, 'bold'), anchor='w').pack(
                side='left')
            tk.Label(th, text=f'Sharpness: {score:.0f}',
                     bg=BG_CELL, fg=score_color,
                     font=('Consolas', 10, 'bold'), anchor='e').pack(
                side='right')

            tk.Label(cell,
                     text=format_exif_short(photo, self.photos_cache),
                     bg=BG_CELL, fg=FG_DIM,
                     font=('Segoe UI', 9)).pack(pady=(0, 8))

            tk_img = self._thumb(Path(photo.path), 680)
            if tk_img:
                il = tk.Label(cell, image=tk_img, bg=BG_CELL,
                              cursor='hand2')
                il.pack()
                il.bind('<Button-1>',
                        lambda e, p=photo: self._show_exif(p))
                il.bind('<Double-Button-1>',
                        lambda e, i=li: self._tour_pick(i))
                il.bind('<Button-3>',
                        lambda e, p=photo: self._zoom(
                            Path(p.path), p))

            tk.Button(cell, text='↑  Winner',
                      command=lambda i=li: self._tour_pick(i),
                      bg=ACCENT, fg='white',
                      activebackground=ACCENT_HOVER,
                      font=('Segoe UI', 11, 'bold'), bd=0,
                      pady=10, padx=24, cursor='hand2').pack(pady=(10, 0))

    def _tour_pick(self, winner):
        s = self.tournament_state
        left, right = s['_pair']
        s['next_round'].append(winner)
        s['runner_up'] = right if winner == left else left
        s['match_num'] += 1
        self._tour_next()

    def _tour_both(self):
        s = self.tournament_state
        if not s or '_pair' not in s:
            return
        l, r = s['_pair']
        s['next_round'].extend([l, r])
        s['match_num'] += 1
        self._tour_next()

    def _tour_discard(self):
        s = self.tournament_state
        if s:
            s['match_num'] += 1
        self._tour_next()

    def _tour_to_grid(self):
        burst = self.session.bursts[self.session.current_burst_idx]
        burst.mode = 'grid'
        self.tournament_state = None
        self._render_burst()

    def _tour_show_winner(self):
        """Show winner for acceptance/rejection before committing."""
        s = self.tournament_state
        burst = self.session.bursts[s['burst_idx']]
        winner = s['winner']
        runner = s['runner_up']

        for w in self.workspace.winfo_children():
            w.destroy()
        self.photo_refs.clear()

        # Swap to winner bar
        for bar in (self.solo_bar, self.grid_bar, self.tour_bar,
                    self.winner_bar, self.stack_bar):
            bar.pack_forget()
        self.winner_bar.pack(fill='x')

        self.mode_banner.config(text='TOURNAMENT WINNER', bg=SUCCESS)

        # Store for accept/reject
        self._winner_idx = winner
        self._runner_idx = runner

        pi = burst.photo_indices[winner]
        photo = self.session.photos[pi]
        self._show_exif(photo)

        frame = tk.Frame(self.workspace, bg=BG_CELL, padx=20, pady=16,
                         highlightbackground=SUCCESS,
                         highlightcolor=SUCCESS, highlightthickness=2)
        frame.pack(expand=True, padx=30, pady=10)

        tk.Label(frame, text=f'Winner: [{winner+1}]  '
                              f'{Path(photo.path).name}',
                 bg=BG_CELL, fg=SUCCESS,
                 font=('Consolas', 12, 'bold')).pack(pady=(0, 4))
        if runner is not None:
            rp = self.session.photos[burst.photo_indices[runner]]
            tk.Label(frame,
                     text=f'Runner-up: [{runner+1}]  '
                          f'{Path(rp.path).name}',
                     bg=BG_CELL, fg=FG_DIM,
                     font=('Segoe UI', 9)).pack(pady=(0, 8))

        h = self.workspace.winfo_height() - 140
        w = self.workspace.winfo_width() - 120
        size = max(200, min(w, h, 900))
        tk_img = self._thumb(Path(photo.path), size)
        if tk_img:
            lbl = tk.Label(frame, image=tk_img, bg=BG_CELL)
            lbl.pack()

    def _winner_accept(self):
        s = self.tournament_state
        idx = s['burst_idx']
        selected = {self._winner_idx}
        # Ask about runner-up
        if self._runner_idx is not None:
            if messagebox.askyesno(
                    'Runner-up',
                    f'Also keep runner-up [{self._runner_idx+1}]?'):
                selected.add(self._runner_idx)
        self.session.mark_burst_photos(idx, selected, self._cur_scenario())
        self.tournament_state = None
        self._navigate_to(idx + 1)

    def _winner_reject(self):
        s = self.tournament_state
        idx = s['burst_idx']
        self.session.mark_burst_photos(idx, set(), self._cur_scenario())
        self.tournament_state = None
        self._navigate_to(idx + 1)

    # ── Shared actions ───────────────────────────────────────────────

    def _skip_burst(self):
        """Skip to next burst without making a decision (stays pending)."""
        idx = self.session.current_burst_idx
        burst = self.session.bursts[idx]
        burst.status = 'pending'
        # Find next non-completed burst after this one
        next_idx = idx + 1
        while (next_idx < self.session.total_bursts
               and self.session.bursts[next_idx].status == 'completed'):
            next_idx += 1
        if next_idx < self.session.total_bursts:
            self.session.current_burst_idx = next_idx
            self.session.bursts[next_idx].status = 'in_progress'
            self._update_panels()
            self._render_burst()
            self._auto_save()

    def _back(self):
        idx = self.session.current_burst_idx
        if idx == 0:
            return
        prev = idx - 1
        while prev >= 0:
            if self.session.bursts[prev].status == 'completed':
                self.session.reset_burst(prev)
                self._navigate_to(prev)
                return
            prev -= 1

    def _cur_scenario(self) -> str:
        burst = self.session.bursts[self.session.current_burst_idx]
        return self.session.photos[burst.photo_indices[0]].effective_scenario

    def _show_exif(self, photo):
        self.exif_lbl.config(text=format_exif_detail(photo,
                                                      self.photos_cache))

    def _zoom(self, path, photo):
        try:
            if path.suffix.lower() in JPEG_EXT:
                img = Image.open(path)
            else:
                with rawpy.imread(str(path)) as raw:
                    t = raw.extract_thumb()
                    img = (Image.open(io.BytesIO(t.data))
                           if t.format == rawpy.ThumbFormat.JPEG
                           else Image.fromarray(t.data))
            img = ImageOps.exif_transpose(img)
            win = tk.Toplevel(self.root)
            win.title(f'Zoom — {path.name}')
            win.configure(bg=BG)
            win.state('zoomed')
            img.thumbnail((1800, 1000), Image.LANCZOS)
            tk_img = ImageTk.PhotoImage(img)
            lbl = tk.Label(win, image=tk_img, bg=BG)
            lbl.image = tk_img
            lbl.pack(expand=True, padx=10, pady=10)
            tk.Label(win, text=f'{path.name}  ·  '
                               f'{format_exif_short(photo, self.photos_cache)}',
                     bg=BG, fg=FG_DIM,
                     font=('Segoe UI', 10)).pack()
            tk.Button(win, text='Close  (Esc)', command=win.destroy,
                      bg=BTN_BG, fg=FG, bd=0, pady=8, padx=24,
                      cursor='hand2',
                      font=('Segoe UI', 11)).pack(pady=10)
            win.bind('<Escape>', lambda e: win.destroy())
            win.bind('<Button-1>', lambda e: win.destroy())
        except Exception as e:
            messagebox.showerror('Error', str(e))

    # ── Settings dialog ────────────────────────────────────────────

    def _show_settings(self):
        win = tk.Toplevel(self.root)
        win.title('Settings')
        win.configure(bg=BG_PANEL)
        win.geometry('420x500')
        win.resizable(False, False)
        # Center on screen
        win.update_idletasks()
        x = (win.winfo_screenwidth() - 420) // 2
        y = (win.winfo_screenheight() - 500) // 2
        win.geometry(f'+{x}+{y}')

        tk.Label(win, text='Settings', bg=BG_PANEL, fg=FG,
                 font=('Segoe UI', 16, 'bold'), pady=12).pack(fill='x')

        # ── Peaking ──
        self._divider(win)
        tk.Label(win, text='FOCUS PEAKING', bg=BG_PANEL, fg=FG_SUBTLE,
                 font=('Segoe UI', 9, 'bold'), padx=16).pack(
            fill='x', anchor='w')

        row1 = tk.Frame(win, bg=BG_PANEL)
        row1.pack(fill='x', padx=16, pady=4)
        tk.Label(row1, text='Default state:', bg=BG_PANEL, fg=FG,
                 font=('Segoe UI', 10)).pack(side='left')
        peak_var = tk.BooleanVar(value=self._settings['peaking_on'])
        tk.Checkbutton(row1, text='ON at startup', variable=peak_var,
                       bg=BG_PANEL, fg=FG, selectcolor=BG_CELL,
                       activebackground=BG_PANEL, activeforeground=FG,
                       font=('Segoe UI', 10)).pack(side='left', padx=10)

        row2 = tk.Frame(win, bg=BG_PANEL)
        row2.pack(fill='x', padx=16, pady=4)
        tk.Label(row2, text='Color:', bg=BG_PANEL, fg=FG,
                 font=('Segoe UI', 10)).pack(side='left')
        color_var = tk.StringVar(value=self._settings['peaking_color'])
        colors = list(self._peak_colors.keys())
        for c in colors:
            rgb = self._peak_colors[c]
            hex_c = f'#{rgb[0]:02x}{rgb[1]:02x}{rgb[2]:02x}'
            tk.Radiobutton(row2, text=c, variable=color_var, value=c,
                           bg=BG_PANEL, fg=hex_c, selectcolor=BG_CELL,
                           activebackground=BG_PANEL,
                           font=('Segoe UI', 9)).pack(side='left', padx=4)

        row3 = tk.Frame(win, bg=BG_PANEL)
        row3.pack(fill='x', padx=16, pady=4)
        tk.Label(row3, text='Sensitivity:', bg=BG_PANEL, fg=FG,
                 font=('Segoe UI', 10)).pack(side='left')
        tk.Label(row3, text='Strict', bg=BG_PANEL, fg=FG_SUBTLE,
                 font=('Segoe UI', 8)).pack(side='left', padx=(8, 2))
        sens_var = tk.DoubleVar(
            value=self._settings['peaking_sensitivity'])
        tk.Scale(row3, from_=0.0, to=1.0, resolution=0.05,
                 orient='horizontal', length=150,
                 variable=sens_var, bg=BG_PANEL, fg=FG,
                 troughcolor=BG_CELL, activebackground=ACCENT,
                 highlightthickness=0, bd=0, showvalue=False,
                 font=('Segoe UI', 8)).pack(side='left', padx=2)
        tk.Label(row3, text='Loose', bg=BG_PANEL, fg=FG_SUBTLE,
                 font=('Segoe UI', 8)).pack(side='left', padx=(2, 0))

        # ── Stack Animation ──
        self._divider(win)
        tk.Label(win, text='STACK ANIMATION', bg=BG_PANEL, fg=FG_SUBTLE,
                 font=('Segoe UI', 9, 'bold'), padx=16).pack(
            fill='x', anchor='w')

        row4 = tk.Frame(win, bg=BG_PANEL)
        row4.pack(fill='x', padx=16, pady=4)
        tk.Label(row4, text='Default speed:', bg=BG_PANEL, fg=FG,
                 font=('Segoe UI', 10)).pack(side='left')
        speed_var = tk.IntVar(value=self._settings['stack_anim_speed'])
        for val, label in [(1, 'Slow'), (3, 'Med'), (5, 'Fast'), (10, 'Max')]:
            tk.Radiobutton(row4, text=label, variable=speed_var, value=val,
                           bg=BG_PANEL, fg=FG, selectcolor=BG_CELL,
                           activebackground=BG_PANEL,
                           font=('Segoe UI', 9)).pack(side='left', padx=4)

        # ── Font Scale ──
        self._divider(win)
        tk.Label(win, text='DISPLAY', bg=BG_PANEL, fg=FG_SUBTLE,
                 font=('Segoe UI', 9, 'bold'), padx=16).pack(
            fill='x', anchor='w')

        row5 = tk.Frame(win, bg=BG_PANEL)
        row5.pack(fill='x', padx=16, pady=4)
        tk.Label(row5, text='Font size:', bg=BG_PANEL, fg=FG,
                 font=('Segoe UI', 10)).pack(side='left')
        tk.Label(row5, text='Small', bg=BG_PANEL, fg=FG_SUBTLE,
                 font=('Segoe UI', 8)).pack(side='left', padx=(8, 2))
        font_var = tk.DoubleVar(value=self._settings['font_scale'])
        tk.Scale(row5, from_=0.8, to=1.3, resolution=0.05,
                 orient='horizontal', length=150,
                 variable=font_var, bg=BG_PANEL, fg=FG,
                 troughcolor=BG_CELL, activebackground=ACCENT,
                 highlightthickness=0, bd=0, showvalue=True,
                 font=('Segoe UI', 8)).pack(side='left', padx=2)
        tk.Label(row5, text='Large', bg=BG_PANEL, fg=FG_SUBTLE,
                 font=('Segoe UI', 8)).pack(side='left', padx=(2, 0))

        # ── Buttons ──
        tk.Frame(win, bg=BG_PANEL).pack(fill='both', expand=True)
        btn_row = tk.Frame(win, bg=BG_PANEL)
        btn_row.pack(fill='x', padx=16, pady=12)

        def _apply():
            self._settings['peaking_on'] = peak_var.get()
            self._settings['peaking_color'] = color_var.get()
            self._settings['peaking_sensitivity'] = sens_var.get()
            self._settings['stack_anim_speed'] = speed_var.get()
            self._settings['font_scale'] = font_var.get()
            save_settings(self._settings)

            # Apply immediately
            self._peaking_on = peak_var.get()
            self._peak_color_name = color_var.get()
            self._peak_threshold = int(80 - sens_var.get() * 65)
            self._font_scale = font_var.get()
            self._anim_speed = speed_var.get()

            # Update peaking button state
            if hasattr(self, 'peak_btn'):
                if self._peaking_on:
                    self.peak_btn.config(text='🔍 Peaking: ON',
                                         bg=DANGER, fg='white')
                else:
                    self.peak_btn.config(text='🔍 Peaking: OFF',
                                         bg=BTN_BG, fg=FG_DIM)
                rgb = self._peak_colors[self._peak_color_name]
                hex_c = f'#{rgb[0]:02x}{rgb[1]:02x}{rgb[2]:02x}'
                self.peak_color_btn.config(
                    text=f'{self._peak_color_name} ▾', fg=hex_c)
                self._peak_sens_var.set(sens_var.get())

            # Clear caches to apply new peaking/font
            self.thumb_cache.clear()
            self.photo_refs.clear()

            win.destroy()

            # Rebuild UI if font changed (requires full rebuild)
            if self._font_scale != self._settings.get('_prev_scale', 1.0):
                self._settings['_prev_scale'] = self._font_scale
                if self.session:
                    self._render_burst()

        def _cancel():
            win.destroy()

        tk.Button(btn_row, text='Cancel', command=_cancel,
                  bg=BTN_BG, fg=FG_DIM, activebackground=BTN_HOVER,
                  font=('Segoe UI', 11), bd=0, padx=20, pady=8,
                  cursor='hand2').pack(side='left', expand=True)
        tk.Button(btn_row, text='Apply & Close', command=_apply,
                  bg=ACCENT, fg='white', activebackground=ACCENT_HOVER,
                  font=('Segoe UI', 11, 'bold'), bd=0, padx=20, pady=8,
                  cursor='hand2').pack(side='right', expand=True)

        win.grab_set()
        win.focus_force()

    # ── Scenario ─────────────────────────────────────────────────────

    def _set_scenario(self, key):
        burst = self.session.bursts[self.session.current_burst_idx]
        for pi in burst.photo_indices:
            self.session.photos[pi].scenario_override = key
        self._update_scenario_list()

    def _update_scenario_list(self):
        current = self._cur_scenario()
        for key, lbl in self._scenario_labels.items():
            if key == current:
                lbl.config(bg=ACCENT, fg='white',
                           font=('Segoe UI', 10, 'bold'))
            else:
                lbl.config(bg=BG_SIDE, fg=FG_SUBTLE,
                           font=('Segoe UI', 9))

    # ── Source/dest actions ──────────────────────────────────────────

    def _change_source(self):
        if self.session.pending_count > 0:
            if not messagebox.askyesno(
                    'Warning',
                    f'{self.session.pending_count} photos still pending.\n'
                    f'Change source? Current decisions will be lost.'):
                return
        path = filedialog.askdirectory(title='Select new source folder')
        if not path:
            return
        photos = scan_folder(Path(path))
        if not photos:
            messagebox.showinfo('Empty', 'No photos found.')
            return
        self.session = build_session(
            photos, path, self.session.destination, 0.5)
        self.photos_cache = {str(p.path): p for p in photos}
        self.thumb_cache.clear()
        save_session(self.session)
        self._clear_all_panels()
        self._build_left()
        self._build_right()
        self._build_center()
        self.root.update_idletasks()
        self._navigate_to(0)

    def _change_destination(self):
        path = filedialog.askdirectory(title='Select new destination')
        if not path:
            return
        self.session.destination = path
        self.dst_lbl.config(text=path)
        save_session(self.session)

    def _clear_source(self):
        if not self.session.committed:
            messagebox.showwarning('Not committed',
                                   'Commit files first before clearing.')
            return
        files = []
        for p in self.session.photos:
            files.append(Path(p.path))
            files.extend(Path(c) for c in p.companions)
        if not messagebox.askyesno(
                'Clear Source',
                f'Delete {len(files)} files from source?\n'
                f'This cannot be undone.', icon='warning'):
            return
        if not messagebox.askyesno(
                'Confirm', 'LAST CHANCE. Really delete?',
                icon='warning'):
            return
        d = sum(1 for f in files if f.exists() and not f.unlink())
        messagebox.showinfo('Done', f'Deleted {d} files.')

    def _accept_all(self):
        n = self.session.pending_count
        if n == 0:
            return
        if messagebox.askyesno('Accept All',
                               f'Accept {n} remaining photos?'):
            self.session.accept_all_remaining()
            self._show_done()

    def _reject_all(self):
        n = self.session.pending_count
        if n == 0:
            return
        if messagebox.askyesno('Reject All',
                               f'Reject {n} remaining photos?'):
            self.session.reject_all_remaining()
            self._show_done()

    def _reset_all(self):
        if messagebox.askyesno('Reset', 'Reset ALL decisions?'):
            self.session.reset_all()
            self.tournament_state = None
            self._navigate_to(0)

    def _commit(self):
        files = self.session.files_to_copy()
        if not files:
            messagebox.showinfo('Nothing', 'No files to copy.')
            return
        by_s = Counter(s for _, s in files)
        summary = '\n'.join(f'  {s}: {n}' for s, n in by_s.most_common())
        if not messagebox.askyesno(
                'Commit',
                f'Copy {len(files)} files to:\n'
                f'{self.session.destination}\n\n{summary}\n\nProceed?'):
            return
        dest = Path(self.session.destination)
        copied, errors = 0, []
        for src, scenario in files:
            try:
                d = dest / scenario
                d.mkdir(parents=True, exist_ok=True)
                t = d / Path(src).name
                if t.exists():
                    stem, suf = t.stem, t.suffix
                    i = 1
                    while t.exists():
                        t = d / f'{stem}_dup{i}{suf}'
                        i += 1
                shutil.copy2(src, t)
                copied += 1
            except Exception as e:
                errors.append(str(e))
        self.session.committed = True
        save_session(self.session)
        msg = f'Copied {copied} files.'
        if errors:
            msg += f'\n{len(errors)} errors.'
        messagebox.showinfo('Done', msg)
        self._update_panels()

    def _quit(self):
        if self.session:
            save_session(self.session)
            print(f'\nSession saved. Keep={self.session.keep_count}, '
                  f'Discard={self.session.discard_count}, '
                  f'Pending={self.session.pending_count}')
        self.root.destroy()

    # ── Done screen ──────────────────────────────────────────────────

    def _show_done(self):
        for w in self.workspace.winfo_children():
            w.destroy()
        for bar in (self.solo_bar, self.grid_bar, self.tour_bar,
                    self.winner_bar, self.stack_bar):
            bar.pack_forget()
        self.mode_banner.config(text='REVIEW COMPLETE', bg=SUCCESS)
        self.burst_info.config(text='All bursts reviewed')
        tk.Label(self.workspace, text='All bursts reviewed',
                 bg=BG, fg=FG, font=('Segoe UI', 20, 'bold')).pack(
            expand=True, pady=(60, 10))
        tk.Label(self.workspace,
                 text=f'{self.session.keep_count} to keep  ·  '
                      f'{self.session.discard_count} discarded',
                 bg=BG, fg=FG_DIM, font=('Segoe UI', 13)).pack(
            pady=(0, 20))
        tk.Button(self.workspace, text='Commit & Copy Files',
                  command=self._commit, bg=SUCCESS, fg='white',
                  activebackground='#0da373',
                  font=('Segoe UI', 14, 'bold'), bd=0,
                  padx=40, pady=14, cursor='hand2').pack(pady=10)
        self._update_panels()

    # ── Panel updates ────────────────────────────────────────────────

    def _update_panels(self):
        s = self.session
        self.l_photos.config(text=f'{s.total_photos} photos')
        multi = sum(1 for b in s.bursts if len(b.photo_indices) > 1
                    and s.photos[b.photo_indices[0]].classification != 'stacks')
        singles = sum(1 for b in s.bursts if len(b.photo_indices) == 1)
        stacks = sum(1 for b in s.bursts
                     if s.photos[b.photo_indices[0]].classification == 'stacks')
        self.l_bursts.config(text=f'{multi} bursts')
        self.l_singles.config(text=f'{singles} individual')
        self.l_stacks.config(text=f'{stacks} stacks')
        self.r_keep.config(text=f'✓  Keep: {s.keep_count}')
        self.r_discard.config(text=f'✗  Discard: {s.discard_count}')
        self.r_pending.config(text=f'…  Pending: {s.pending_count}')
        self._update_scenario_list()
        self._update_progress()
        self._update_prev_next_thumbs()

    def _update_prev_next_thumbs(self):
        """Show small thumbnails of previous and next bursts."""
        idx = self.session.current_burst_idx

        # Previous
        prev_idx = idx - 1
        if prev_idx >= 0:
            burst = self.session.bursts[prev_idx]
            photo = self.session.photos[burst.photo_indices[0]]
            tk_img = self._thumb(Path(photo.path), 160)
            if tk_img:
                self.prev_thumb_lbl.config(image=tk_img)
                self.prev_thumb_lbl.image = tk_img
            else:
                self.prev_thumb_lbl.config(image='', text='(no prev)')
        else:
            self.prev_thumb_lbl.config(image='', text='')

        # Next
        next_idx = idx + 1
        while (next_idx < self.session.total_bursts
               and self.session.bursts[next_idx].status == 'completed'):
            next_idx += 1
        if next_idx < self.session.total_bursts:
            burst = self.session.bursts[next_idx]
            photo = self.session.photos[burst.photo_indices[0]]
            tk_img = self._thumb(Path(photo.path), 160)
            if tk_img:
                self.next_thumb_lbl.config(image=tk_img)
                self.next_thumb_lbl.image = tk_img
            else:
                self.next_thumb_lbl.config(image='', text='(no next)')
        else:
            self.next_thumb_lbl.config(image='', text='(end)')

    def _update_progress(self):
        self.progress_canvas.delete('all')
        w = self.progress_canvas.winfo_width()
        if w < 10:
            w = 1600
        total = self.session.total_bursts
        done = self.session.completed_bursts
        pct = done / total if total > 0 else 0
        bw = int(w * pct)
        if bw > 0:
            self.progress_canvas.create_rectangle(
                0, 0, bw, 3, fill=ACCENT, outline='')

    def _thumb(self, path, size) -> ImageTk.PhotoImage | None:
        peak_key = (self._peak_color_name if self._peaking_on else 'off')
        key = (str(path), size, peak_key)
        if key in self.thumb_cache:
            return self.thumb_cache[key]
        img = extract_thumbnail(path, size)
        if not img:
            return None
        if self._peaking_on:
            color = self._peak_colors[self._peak_color_name]
            img = apply_peaking(img, color=color,
                                threshold=self._peak_threshold,
                                opacity=0.7)
        tk_img = ImageTk.PhotoImage(img)
        self.photo_refs.append(tk_img)
        self.thumb_cache[key] = tk_img
        return tk_img

    def _auto_save(self):
        self._save_counter += 1
        if self._save_counter % 5 == 0:
            save_session(self.session)

    def _on_key(self, event):
        if not self.session:
            return
        key = event.keysym.lower()
        idx = self.session.current_burst_idx
        if idx >= self.session.total_bursts:
            return
        burst = self.session.bursts[idx]
        if burst.mode == 'solo':
            if key in ('return', 'space'):
                self._solo_keep()
            elif key in ('delete', 'x'):
                self._solo_discard()
        elif burst.mode == 'grid':
            if key in '123456':
                n = int(key) - 1
                if n < len(burst.photo_indices):
                    self._grid_pick(n)
            elif key == 'a':
                self._burst_keep_all()
        elif burst.mode == 'tournament':
            s = self.tournament_state
            if s and '_pair' in s:
                if key in ('left', 'l'):
                    self._tour_pick(s['_pair'][0])
                elif key in ('right', 'r'):
                    self._tour_pick(s['_pair'][1])
                elif key == 'space':
                    self._tour_both()
                elif key == 'g':
                    self._tour_to_grid()
        if key == 'backspace':
            self._back()
        elif key == 'q':
            self._quit()


# ── CLI classify-only ────────────────────────────────────────────────────

def run_classify_only(photos, dest, dry_run=False):
    print(f'\nClassify-only: {len(photos)} photos\n')
    if not dry_run:
        dest.mkdir(parents=True, exist_ok=True)
    counts = Counter()
    low = []
    for p in photos:
        c = classify(p)
        counts[c.scenario] += 1
        if c.confidence.value == 'LOW':
            low.append((p, c))
        if not dry_run:
            d = dest / c.scenario
            d.mkdir(parents=True, exist_ok=True)
            t = d / p.path.name
            if t.exists():
                stem, suf = t.stem, t.suffix
                i = 1
                while t.exists():
                    t = d / f'{stem}_dup{i}{suf}'
                    i += 1
            shutil.copy2(p.path, t)
    print('=== SUMMARY ===')
    for s, n in counts.most_common():
        print(f'  {s}: {n}')
    if low:
        print(f'\n{len(low)} LOW confidence:')
        for p, c in low[:10]:
            print(f'  {p.path.name} -> {c.scenario} ({"; ".join(c.reasons)})')
        if len(low) > 10:
            print(f'  ...and {len(low)-10} more')
    if dry_run:
        print('\n(DRY RUN)')
    else:
        print(f'\nCopied to: {dest}')


# ── Entry point ──────────────────────────────────────────────────────────

def main():
    parser = argparse.ArgumentParser(description='Burst Culler v3')
    parser.add_argument('folder', nargs='?', default=None)
    parser.add_argument('--gap', type=float, default=0.5)
    parser.add_argument('--out', default=None)
    parser.add_argument('--classify-only', action='store_true')
    parser.add_argument('--dry-run', action='store_true')
    args = parser.parse_args()

    if args.classify_only:
        if not args.folder:
            print('Error: --classify-only requires a folder argument')
            sys.exit(1)
        src = Path(args.folder).resolve()
        dest = Path(args.out).resolve() if args.out else src / 'classified'
        photos = scan_folder(src)
        if photos:
            run_classify_only(photos, dest, args.dry_run)
        return

    # Interactive mode — app handles folder selection
    CullerApp()


if __name__ == '__main__':
    main()
