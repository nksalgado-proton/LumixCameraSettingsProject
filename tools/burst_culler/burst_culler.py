"""
Burst Culler — Visual culling tool for travel photography.

Groups photos by timestamp into bursts, detects the scenario automatically
using EXIF + Panasonic maker notes, and lets you quickly pick keepers from
each burst with mouse (or keyboard) shortcuts.

Usage:
    python burst_culler.py <source_folder> [--gap SECONDS] [--out FOLDER]

Output: copies keepers to <source>/keepers/<scenario>/ subfolders.
Originals are never touched.
"""

import argparse
import io
import shutil
import sys
import time
import tkinter as tk
from dataclasses import dataclass, field
from pathlib import Path
from tkinter import messagebox

import rawpy
from PIL import Image, ImageTk

from classifier import SCENARIOS, Classification, classify
from exif_reader import PhotoExif, read_exif_batch

RAW_EXT = {'.rw2', '.raf', '.arw', '.cr2', '.cr3', '.nef', '.orf',
           '.dng', '.raw', '.srw', '.pef'}
JPEG_EXT = {'.jpg', '.jpeg'}
PHOTO_EXT = RAW_EXT | JPEG_EXT

# Colors (dark theme)
BG = '#1a1a1a'
BG_PANEL = '#2a2a2a'
FG = '#e0e0e0'
FG_DIM = '#888888'
ACCENT = '#4a9eff'
ACCENT_HOT = '#ff8844'
SELECTED = '#2a5a8f'
WIN_GLOW = '#2a7a3a'


@dataclass
class Burst:
    photos: list[PhotoExif]
    classification: Classification | None = None
    selected: set[int] = field(default_factory=set)  # indices of keepers
    scenario: str = ''  # user-overridable scenario folder
    committed: bool = False


def scan_folder(folder: Path) -> list[PhotoExif]:
    """Recursively scan folder for photo files (excluding keepers/)."""
    print(f'Scanning {folder} (recursive)...')
    files = []
    for p in folder.rglob('*'):
        if not p.is_file():
            continue
        if p.suffix.lower() not in PHOTO_EXT:
            continue
        if 'preview' in p.name.lower():
            continue
        # Exclude anything under a "keepers" folder
        if any(part.lower() == 'keepers' for part in p.parts):
            continue
        files.append(p)
    print(f'Found {len(files)} photo files. Reading EXIF...')
    photos = read_exif_batch(files)
    # Drop photos without timestamps
    photos = [p for p in photos if p.timestamp]
    photos.sort(key=lambda p: p.timestamp)
    print(f'Read metadata from {len(photos)} photos.')
    return photos


def group_bursts(photos: list[PhotoExif], gap_seconds: float) -> list[Burst]:
    """Group consecutive photos within gap_seconds. Also splits on lens change."""
    if not photos:
        return []
    bursts = []
    current = [photos[0]]
    for p in photos[1:]:
        prev = current[-1]
        delta = (p.timestamp - prev.timestamp).total_seconds()
        same_lens = p.lens == prev.lens
        if delta <= gap_seconds and same_lens:
            current.append(p)
        else:
            bursts.append(Burst(photos=current))
            current = [p]
    bursts.append(Burst(photos=current))
    return bursts


def classify_bursts(bursts: list[Burst]) -> None:
    """Use the first photo of each burst to classify it."""
    for b in bursts:
        c = classify(b.photos[0])
        b.classification = c
        b.scenario = c.scenario


def extract_thumbnail(path: Path, target_size: int = 600) -> Image.Image | None:
    if target_size < 50:
        target_size = 200  # guard against bad sizing
    try:
        if path.suffix.lower() in JPEG_EXT:
            img = Image.open(path)
        else:
            with rawpy.imread(str(path)) as raw:
                thumb = raw.extract_thumb()
                if thumb.format == rawpy.ThumbFormat.JPEG:
                    img = Image.open(io.BytesIO(thumb.data))
                else:
                    img = Image.fromarray(thumb.data)
        # Handle EXIF orientation
        from PIL import ImageOps
        img = ImageOps.exif_transpose(img)
        img.thumbnail((target_size, target_size), Image.LANCZOS)
        return img
    except Exception as e:
        print(f'  Thumb error for {path.name}: {e}')
        return None


def format_exposure(p: PhotoExif) -> str:
    parts = []
    if p.focal_length:
        parts.append(f'{p.focal_length:.0f}mm')
    if p.aperture:
        parts.append(f'f/{p.aperture:.1f}')
    if p.shutter_speed:
        if p.shutter_speed >= 1:
            parts.append(f'{p.shutter_speed:.0f}s')
        else:
            parts.append(f'1/{1 / p.shutter_speed:.0f}s')
    if p.iso:
        parts.append(f'ISO{p.iso}')
    return ' · '.join(parts)


class CullerApp:
    def __init__(self, bursts: list[Burst], source: Path, keepers: Path):
        self.bursts = bursts
        self.source = source
        self.keepers = keepers
        self.idx = 0
        self.mode = 'grid'  # or 'tournament'
        self.tournament_state = None  # current tournament state
        self.stats = {'files_copied': 0, 'bursts_processed': 0}
        self.photo_refs = []  # prevent PIL garbage collection
        self.thumb_cache = {}

        self.root = tk.Tk()
        self.root.title('Burst Culler')
        self.root.geometry('1600x960')
        self.root.configure(bg=BG)
        self._build_ui()

        # Auto-select initial mode per burst
        self._auto_mode_for_current()
        self._show_burst()
        self.root.mainloop()

    def _build_ui(self):
        # Top header bar
        self.header_frame = tk.Frame(self.root, bg=BG_PANEL, height=60)
        self.header_frame.pack(fill='x', side='top')
        self.header_frame.pack_propagate(False)

        self.header_lbl = tk.Label(
            self.header_frame, bg=BG_PANEL, fg=FG,
            font=('Segoe UI', 11, 'bold'), anchor='w', padx=15)
        self.header_lbl.pack(side='left', fill='y')

        # Scenario override button
        self.scenario_btn = tk.Button(
            self.header_frame, text='Change ▾', command=self._change_scenario,
            bg='#3a3a3a', fg=FG, activebackground='#555',
            font=('Segoe UI', 9), bd=0, padx=12, pady=4)
        self.scenario_btn.pack(side='left', padx=10, pady=16)

        # Mode toggle
        mode_frame = tk.Frame(self.header_frame, bg=BG_PANEL)
        mode_frame.pack(side='right', padx=15)
        self.mode_var = tk.StringVar(value='grid')
        for text, val in [('Grid', 'grid'), ('Tournament', 'tournament')]:
            rb = tk.Radiobutton(
                mode_frame, text=text, variable=self.mode_var, value=val,
                bg=BG_PANEL, fg=FG, selectcolor=BG, activebackground=BG_PANEL,
                activeforeground=FG, font=('Segoe UI', 10),
                command=self._on_mode_change)
            rb.pack(side='left', padx=8)

        # Main content area
        self.content = tk.Frame(self.root, bg=BG)
        self.content.pack(fill='both', expand=True, padx=10, pady=8)

        # Bottom action bar
        self.action_frame = tk.Frame(self.root, bg=BG_PANEL, height=60)
        self.action_frame.pack(fill='x', side='bottom')
        self.action_frame.pack_propagate(False)

        # Grid mode buttons (will be shown/hidden based on mode)
        self.grid_actions = tk.Frame(self.action_frame, bg=BG_PANEL)
        self._make_button(self.grid_actions, '✓ Commit Selection',
                          self._commit_grid, ACCENT, width=20)
        self._make_button(self.grid_actions, 'Keep All',
                          self._keep_all, None)
        self._make_button(self.grid_actions, 'Skip (keep none)',
                          self._skip_burst, None)
        self._make_button(self.grid_actions, '← Back',
                          self._back, None)

        # Tournament actions
        self.tour_actions = tk.Frame(self.action_frame, bg=BG_PANEL)
        self._make_button(self.tour_actions, 'Keep Both',
                          self._tour_both, ACCENT)
        self._make_button(self.tour_actions, 'Skip this round',
                          self._tour_skip, None)
        self._make_button(self.tour_actions, '← Back to grid',
                          self._tour_to_grid, None)

        # Quit button always visible
        quit_btn = tk.Button(
            self.action_frame, text='Quit', command=self._quit,
            bg='#552222', fg=FG, activebackground='#772222',
            font=('Segoe UI', 10), width=8, bd=0, pady=4)
        quit_btn.pack(side='right', padx=10, pady=12)

        # Clear source button (destructive, always visible)
        clear_btn = tk.Button(
            self.action_frame, text='🗑  Clear Source…',
            command=self._clear_source,
            bg='#4a2a2a', fg='#ffb0b0', activebackground='#6a3030',
            font=('Segoe UI', 10), bd=0, pady=4, padx=12)
        clear_btn.pack(side='right', padx=5, pady=12)

        self.root.bind('<Key>', self._on_key)
        self._last_width = 0
        self._resize_after = None
        self.root.bind('<Configure>', self._on_resize)

    def _on_resize(self, event):
        """Re-render grid when window width changes significantly."""
        if event.widget is not self.root:
            return
        if abs(event.width - self._last_width) < 50:
            return
        self._last_width = event.width
        # Debounce: wait 300ms after user stops resizing
        if self._resize_after:
            self.root.after_cancel(self._resize_after)
        self._resize_after = self.root.after(300, self._do_resize)

    def _do_resize(self):
        self._resize_after = None
        self.thumb_cache.clear()  # different sizes now
        if self.mode == 'grid':
            self._show_burst()

    def _make_button(self, parent, text, cmd, color, width=16):
        btn = tk.Button(
            parent, text=text, command=cmd,
            bg=color or BG, fg=FG,
            activebackground='#444', activeforeground=FG,
            font=('Segoe UI', 10), width=width, bd=0, pady=6, padx=8)
        btn.pack(side='left', padx=5, pady=12)
        return btn

    def _on_mode_change(self):
        new_mode = self.mode_var.get()
        if new_mode != self.mode:
            self.mode = new_mode
            self._show_burst()

    def _auto_mode_for_current(self):
        if self.idx >= len(self.bursts):
            return
        b = self.bursts[self.idx]
        # Auto-route special cases
        if b.scenario == 'stacks':
            # Focus bracket: auto-keep all, no UI needed
            return
        if len(b.photos) <= 1:
            return
        new_mode = 'tournament' if len(b.photos) >= 6 else 'grid'
        self.mode = new_mode
        self.mode_var.set(new_mode)

    def _current_burst(self) -> Burst | None:
        if 0 <= self.idx < len(self.bursts):
            return self.bursts[self.idx]
        return None

    def _update_header(self):
        b = self._current_burst()
        if not b:
            return
        n = len(b.photos)
        ts = b.photos[0].timestamp.strftime('%Y-%m-%d %H:%M:%S')
        cls = b.classification
        conf = cls.confidence.value if cls else '?'
        text = (f'Burst {self.idx + 1}/{len(self.bursts)}  ·  '
                f'{n} photo{"s" if n > 1 else ""}  ·  '
                f'{ts}  ·  Scenario: {b.scenario} [{conf}]  ·  '
                f'Copied: {self.stats["files_copied"]}')
        self.header_lbl.config(text=text)

    def _show_burst(self):
        for w in self.content.winfo_children():
            w.destroy()
        self.photo_refs.clear()

        if self.idx >= len(self.bursts):
            self._finish()
            return

        b = self._current_burst()

        # Auto-handle special bursts
        if b.scenario == 'stacks' and not b.committed:
            # Focus bracket: copy all frames silently
            for p in b.photos:
                self._copy_photo(p, 'stacks')
            b.committed = True
            self.idx += 1
            self._auto_mode_for_current()
            self.root.after(1, self._show_burst)
            return
        if len(b.photos) == 1 and not b.committed:
            # Single photo: auto-copy to its scenario
            self._copy_photo(b.photos[0], b.scenario)
            b.committed = True
            b.selected = {0}
            self.idx += 1
            self._auto_mode_for_current()
            self.root.after(1, self._show_burst)
            return

        self._update_header()
        # Swap action bars
        self.grid_actions.pack_forget()
        self.tour_actions.pack_forget()
        if self.mode == 'grid':
            self.grid_actions.pack(side='left')
            self._show_grid(b)
        else:
            self.tour_actions.pack(side='left')
            self._show_tournament(b)

    def _show_grid(self, b: Burst):
        n = len(b.photos)
        cols = min(n, 5) if n <= 5 else 4
        self._grid_cells = {}  # photo_idx -> cell widget for in-place updates
        # Calculate thumbnail width (guard against tiny initial window)
        avail_w = self.content.winfo_width()
        if avail_w < 400:  # not yet rendered, use default
            avail_w = 1580
        thumb_w = max(200, min(500, (avail_w // cols) - 25))

        for i, photo in enumerate(b.photos):
            row = i // cols
            col = i % cols
            is_selected = i in b.selected
            bg_color = SELECTED if is_selected else BG
            cell = tk.Frame(self.content, bg=bg_color, bd=3,
                            relief='solid' if is_selected else 'flat',
                            highlightbackground=ACCENT if is_selected else BG,
                            highlightthickness=3, padx=4, pady=4)
            cell.grid(row=row, column=col, padx=4, pady=4, sticky='nsew')
            self._grid_cells[i] = cell

            # Filename + number badge
            num_text = f'[{i + 1}]  {photo.path.name}'
            if is_selected:
                num_text = '✓ ' + num_text
            num_lbl = tk.Label(
                cell, text=num_text, bg=bg_color, fg=ACCENT,
                font=('Consolas', 9, 'bold'), anchor='w')
            num_lbl.pack(fill='x')
            cell._num_lbl = num_lbl  # keep reference for in-place updates

            # EXIF info
            exif_lbl = tk.Label(
                cell, text=format_exposure(photo), bg=bg_color, fg=FG_DIM,
                font=('Segoe UI', 8), anchor='w')
            exif_lbl.pack(fill='x')
            cell._exif_lbl = exif_lbl
            cell._photo_ref = photo

            # Thumbnail
            tk_img = self._get_thumb(photo.path, thumb_w)
            if tk_img:
                img_lbl = tk.Label(cell, image=tk_img, bg=bg_color,
                                   cursor='hand2')
                img_lbl.pack()
                # Single-click = toggle select
                img_lbl.bind('<Button-1>',
                             lambda e, idx=i: self._toggle_select(idx))
                # Double-click = select this one and commit
                img_lbl.bind('<Double-Button-1>',
                             lambda e, idx=i: self._double_click_commit(idx))
                # Right-click = zoom
                img_lbl.bind('<Button-3>',
                             lambda e, p=photo: self._zoom_photo(p))
            else:
                tk.Label(cell, text='(no preview)', bg='#333', fg=FG_DIM,
                         width=30, height=12).pack()

        # Make columns expandable
        for c in range(cols):
            self.content.columnconfigure(c, weight=1)

    def _show_tournament(self, b: Burst):
        if not self.tournament_state or self.tournament_state['burst'] is not b:
            # Initialize tournament for this burst
            photos_idx = list(range(len(b.photos)))
            self.tournament_state = {
                'burst': b,
                'queue': photos_idx,  # winners of current round
                'next_round': [],
                'round_num': 1,
                'winner': None,
                'winner_beat': None,  # runner-up candidate
            }
        self._tour_next_match()

    def _tour_next_match(self):
        state = self.tournament_state
        if state is None:
            return
        b = state['burst']

        if state['winner'] is not None:
            self._tour_finish()
            return

        # Keep merging rounds until we have 2+ photos or a winner
        while len(state['queue']) < 2:
            if state['next_round']:
                # End of round: merge next_round winners into queue
                state['queue'] = state['next_round'] + state['queue']
                state['next_round'] = []
                state['round_num'] += 1
            elif len(state['queue']) == 1:
                # Only one photo left overall → winner
                state['winner'] = state['queue'][0]
                self._tour_finish()
                return
            else:
                # Empty — no winner (user skipped everything)
                state['winner'] = None
                self._tour_finish()
                return

        # Show current pair
        for w in self.content.winfo_children():
            w.destroy()
        self.photo_refs.clear()

        left_idx = state['queue'].pop(0)
        right_idx = state['queue'].pop(0)
        state['_current_pair'] = (left_idx, right_idx)

        left_photo = b.photos[left_idx]
        right_photo = b.photos[right_idx]

        # Layout: two big thumbnails side by side
        for col, (idx, photo) in enumerate([(left_idx, left_photo),
                                            (right_idx, right_photo)]):
            cell = tk.Frame(self.content, bg=BG, padx=10, pady=10)
            cell.grid(row=0, column=col, padx=10, pady=10, sticky='nsew')
            self.content.columnconfigure(col, weight=1)

            tk.Label(cell, text=f'[{idx + 1}] {photo.path.name}',
                     bg=BG, fg=ACCENT, font=('Consolas', 10, 'bold')).pack()
            tk.Label(cell, text=format_exposure(photo), bg=BG, fg=FG_DIM,
                     font=('Segoe UI', 9)).pack()

            tk_img = self._get_thumb(photo.path, 750)
            if tk_img:
                img_lbl = tk.Label(cell, image=tk_img, bg=BG, cursor='hand2')
                img_lbl.pack()
                img_lbl.bind('<Button-1>',
                             lambda e, i=idx: self._tour_pick(i))
                img_lbl.bind('<Button-3>',
                             lambda e, p=photo: self._zoom_photo(p))

            btn = tk.Button(
                cell, text='▲  Pick this one',
                command=lambda i=idx: self._tour_pick(i),
                bg=ACCENT, fg='white', activebackground='#2a7ecf',
                font=('Segoe UI', 11, 'bold'), bd=0, pady=10, padx=20)
            btn.pack(pady=8)

        # Update header
        rnd = self.tournament_state['round_num']
        remain = len(self.tournament_state['queue']) + len(
            self.tournament_state['next_round']) + 2
        self.header_lbl.config(
            text=f'Tournament · Round {rnd} · '
                 f'{remain} photos remaining · '
                 f'Click winner or press L/R')

    def _tour_pick(self, winner_idx: int):
        state = self.tournament_state
        left, right = state['_current_pair']
        loser = right if winner_idx == left else left
        state['next_round'].append(winner_idx)
        state['winner_beat'] = loser  # track runner-up candidate
        self._tour_next_match()

    def _tour_both(self):
        state = self.tournament_state
        if not state:
            return
        left, right = state['_current_pair']
        state['next_round'].append(left)
        state['next_round'].append(right)
        self._tour_next_match()

    def _tour_skip(self):
        # Skip current match without advancing anyone
        self._tour_next_match()

    def _tour_to_grid(self):
        self.mode = 'grid'
        self.mode_var.set('grid')
        self.tournament_state = None
        self._show_burst()

    def _tour_finish(self):
        state = self.tournament_state
        b = state['burst']
        winner = state['winner']
        runner_up = state['winner_beat']

        if winner is None:
            # No winner (all skipped) — commit empty
            b.selected = set()
        elif runner_up is not None and runner_up != winner:
            keep_runner = messagebox.askyesno(
                'Tournament winner',
                f'Winner: photo [{winner + 1}]\n'
                f'Runner-up: photo [{runner_up + 1}]\n\n'
                f'Keep the runner-up too?')
            b.selected = {winner, runner_up} if keep_runner else {winner}
        else:
            b.selected = {winner}

        self._commit_burst(b)
        self.tournament_state = None
        self.idx += 1
        self._auto_mode_for_current()
        self._show_burst()

    def _change_scenario(self):
        """Open a menu to change the current burst's scenario."""
        b = self._current_burst()
        if not b:
            return
        menu = tk.Menu(self.root, tearoff=0, bg=BG_PANEL, fg=FG,
                       activebackground=ACCENT, activeforeground='white')
        for key, desc in SCENARIOS.items():
            label = f'{key} — {desc}'
            if key == b.scenario:
                label = '✓ ' + label
            menu.add_command(
                label=label,
                command=lambda k=key: self._set_scenario(k))
        try:
            x = self.scenario_btn.winfo_rootx()
            y = self.scenario_btn.winfo_rooty() + self.scenario_btn.winfo_height()
            menu.tk_popup(x, y)
        finally:
            menu.grab_release()

    def _set_scenario(self, scenario: str):
        b = self._current_burst()
        if b:
            b.scenario = scenario
            self._update_header()

    def _double_click_commit(self, idx: int):
        """Double-click: select only this one and commit immediately."""
        b = self._current_burst()
        if not b:
            return
        b.selected = {idx}
        self._commit_grid()

    def _toggle_select(self, idx: int):
        b = self._current_burst()
        if idx in b.selected:
            b.selected.remove(idx)
        else:
            b.selected.add(idx)
        # Update cell in place (don't rebuild — preserves double-click events)
        self._refresh_cell(idx)

    def _refresh_cell(self, idx: int):
        """Update one cell's selection state visually without rebuilding."""
        cell = self._grid_cells.get(idx)
        if cell is None:
            return
        b = self._current_burst()
        is_selected = idx in b.selected
        bg_color = SELECTED if is_selected else BG
        cell.config(bg=bg_color,
                    relief='solid' if is_selected else 'flat',
                    highlightbackground=ACCENT if is_selected else BG)
        # Update children backgrounds
        for child in cell.winfo_children():
            try:
                child.config(bg=bg_color)
            except tk.TclError:
                pass
        # Update filename label text with checkmark
        photo = cell._photo_ref
        num_text = f'[{idx + 1}]  {photo.path.name}'
        if is_selected:
            num_text = '✓ ' + num_text
        cell._num_lbl.config(text=num_text)

    def _zoom_photo(self, photo: PhotoExif):
        """Open a fullscreen preview of one photo."""
        try:
            if photo.path.suffix.lower() in JPEG_EXT:
                img = Image.open(photo.path)
            else:
                with rawpy.imread(str(photo.path)) as raw:
                    thumb = raw.extract_thumb()
                    if thumb.format == rawpy.ThumbFormat.JPEG:
                        img = Image.open(io.BytesIO(thumb.data))
                    else:
                        img = Image.fromarray(thumb.data)
            from PIL import ImageOps
            img = ImageOps.exif_transpose(img)

            win = tk.Toplevel(self.root)
            win.title(f'Zoom — {photo.path.name}')
            win.configure(bg=BG)
            win.geometry('1400x900')

            # Scale to fit window
            img.thumbnail((1380, 820), Image.LANCZOS)
            tk_img = ImageTk.PhotoImage(img)
            lbl = tk.Label(win, image=tk_img, bg=BG)
            lbl.image = tk_img
            lbl.pack(padx=10, pady=10)

            info = tk.Label(
                win,
                text=f'{photo.path.name}  ·  {format_exposure(photo)}',
                bg=BG, fg=FG, font=('Segoe UI', 10))
            info.pack()

            tk.Button(win, text='Close (Esc)', command=win.destroy,
                      bg=BG_PANEL, fg=FG, bd=0, pady=6, padx=20).pack(pady=8)
            win.bind('<Escape>', lambda e: win.destroy())
            win.bind('<Button-1>', lambda e: win.destroy())
        except Exception as e:
            messagebox.showerror('Error', f'Could not zoom: {e}')

    def _commit_grid(self):
        b = self._current_burst()
        if not b:
            return
        if not b.selected:
            if not messagebox.askyesno(
                    'Empty selection',
                    'No photos selected. Skip this burst?'):
                return
        self._commit_burst(b)
        self.idx += 1
        self._auto_mode_for_current()
        self._show_burst()

    def _keep_all(self):
        b = self._current_burst()
        if not b:
            return
        b.selected = set(range(len(b.photos)))
        self._commit_burst(b)
        self.idx += 1
        self._auto_mode_for_current()
        self._show_burst()

    def _skip_burst(self):
        b = self._current_burst()
        if b:
            b.selected = set()
            b.committed = True
        self.idx += 1
        self._auto_mode_for_current()
        self._show_burst()

    def _back(self):
        if self.idx == 0:
            messagebox.showinfo('Back', 'Already at first burst.')
            return
        # Find previous committed burst
        for i in range(self.idx - 1, -1, -1):
            b = self.bursts[i]
            if b.committed:
                # Undo: delete copied files, uncommit
                if messagebox.askyesno(
                        'Undo previous burst',
                        f'Undo burst {i + 1}? Copied files will be deleted.'):
                    self._undo_burst(b)
                    self.idx = i
                    self._auto_mode_for_current()
                    self._show_burst()
                return
        messagebox.showinfo('Back', 'No previous committed burst.')

    def _undo_burst(self, b: Burst):
        """Remove copies of this burst's keepers."""
        dest = self.keepers / b.scenario
        for idx in b.selected:
            photo = b.photos[idx]
            candidate = dest / photo.path.name
            if candidate.exists():
                candidate.unlink()
                self.stats['files_copied'] -= 1
        b.committed = False
        b.selected = set()

    def _commit_burst(self, b: Burst):
        for idx in b.selected:
            self._copy_photo(b.photos[idx], b.scenario)
        b.committed = True
        self.stats['bursts_processed'] += 1

    def _copy_photo(self, photo: PhotoExif, scenario: str):
        dest_dir = self.keepers / scenario
        dest_dir.mkdir(parents=True, exist_ok=True)
        dest = dest_dir / photo.path.name
        if dest.exists():
            stem, suffix = dest.stem, dest.suffix
            i = 1
            while dest.exists():
                dest = dest_dir / f'{stem}_dup{i}{suffix}'
                i += 1
        shutil.copy2(photo.path, dest)
        self.stats['files_copied'] += 1

    def _get_thumb(self, path: Path, size: int) -> ImageTk.PhotoImage | None:
        cache_key = (str(path), size)
        if cache_key in self.thumb_cache:
            return self.thumb_cache[cache_key]
        img = extract_thumbnail(path, size)
        if img is None:
            return None
        tk_img = ImageTk.PhotoImage(img)
        self.photo_refs.append(tk_img)
        self.thumb_cache[cache_key] = tk_img
        return tk_img

    def _on_key(self, event):
        key = event.keysym.lower()
        if key == 'q':
            self._quit()
            return
        b = self._current_burst()
        if not b:
            return
        if self.mode == 'grid':
            if key in '123456789':
                n = int(key) - 1
                if n < len(b.photos):
                    self._toggle_select(n)
            elif key == 'return':
                self._commit_grid()
            elif key == 'a':
                self._keep_all()
            elif key == 's':
                self._skip_burst()
            elif key == 't':
                self.mode = 'tournament'
                self.mode_var.set('tournament')
                self._show_burst()
        elif self.mode == 'tournament':
            if key in ('left', 'l'):
                state = self.tournament_state
                if state and '_current_pair' in state:
                    self._tour_pick(state['_current_pair'][0])
            elif key in ('right', 'r'):
                state = self.tournament_state
                if state and '_current_pair' in state:
                    self._tour_pick(state['_current_pair'][1])
            elif key == 'space':
                self._tour_both()
            elif key == 'g':
                self._tour_to_grid()

    def _clear_source(self):
        """Delete all photo files from source folder. Multiple confirmations."""
        # Count source files (excluding keepers/)
        keepers_resolved = self.keepers.resolve()
        src_files = []
        for p in self.source.rglob('*'):
            if (p.is_file() and p.suffix.lower() in PHOTO_EXT
                    and keepers_resolved not in p.resolve().parents):
                src_files.append(p)

        # Count keepers
        kept_files = []
        if self.keepers.exists():
            for p in self.keepers.rglob('*'):
                if p.is_file() and p.suffix.lower() in PHOTO_EXT:
                    kept_files.append(p)

        if not src_files:
            messagebox.showinfo(
                'Clear Source',
                'No photo files to delete in source folder.')
            return

        # Warn if fewer keepers than expected
        if len(kept_files) == 0:
            if not messagebox.askyesno(
                    'No keepers saved',
                    'WARNING: The keepers folder is empty.\n\n'
                    'Are you sure you want to delete all '
                    f'{len(src_files)} source files?\n\n'
                    'This cannot be undone.',
                    icon='warning'):
                return

        # First confirmation with counts
        msg1 = (
            f'DELETE SOURCE FILES\n\n'
            f'Will delete: {len(src_files)} files from\n'
            f'  {self.source}\n\n'
            f'Safely copied: {len(kept_files)} files in\n'
            f'  {self.keepers}\n\n'
            f'This cannot be undone. Proceed?'
        )
        if not messagebox.askyesno('Clear Source — Step 1/2', msg1,
                                   icon='warning'):
            return

        # Second confirmation
        if not messagebox.askyesno(
                'Clear Source — Step 2/2',
                f'Really delete {len(src_files)} files?\n\n'
                f'Last chance to cancel.',
                icon='warning'):
            return

        # Delete
        deleted = 0
        failed = []
        for f in src_files:
            try:
                f.unlink()
                deleted += 1
            except Exception as e:
                failed.append(f'{f.name}: {e}')

        result = f'Deleted {deleted} files.'
        if failed:
            result += f'\n\n{len(failed)} failures:\n' + '\n'.join(failed[:5])
        messagebox.showinfo('Clear Source — Done', result)

    def _quit(self):
        self._finish()

    def _finish(self):
        total_bursts = len(self.bursts)
        committed = sum(1 for b in self.bursts if b.committed)
        print('\n=== SUMMARY ===')
        print(f'Bursts processed: {committed}/{total_bursts}')
        print(f'Files copied: {self.stats["files_copied"]}')
        # Folder breakdown
        if self.keepers.exists():
            print(f'\nKeepers folder: {self.keepers}')
            for sub in sorted(self.keepers.iterdir()):
                if sub.is_dir():
                    count = len(list(sub.iterdir()))
                    print(f'  {sub.name}/: {count} files')
        self.root.destroy()


def run_classify_only(photos: list[PhotoExif], keepers: Path,
                      dry_run: bool = False) -> None:
    """Classify each photo individually and copy to scenario folder.

    No GUI, no burst grouping, no user interaction. Useful when culling
    is already done and you just want to organize by scenario.
    """
    from collections import Counter
    print(f'\nClassify-only mode: {len(photos)} photos\n')
    if not dry_run:
        keepers.mkdir(parents=True, exist_ok=True)

    counts = Counter()
    low_conf = []
    for p in photos:
        c = classify(p)
        counts[c.scenario] += 1
        if c.confidence.value == 'LOW':
            low_conf.append((p, c))
        if not dry_run:
            dest_dir = keepers / c.scenario
            dest_dir.mkdir(parents=True, exist_ok=True)
            dest = dest_dir / p.path.name
            if dest.exists():
                stem, suffix = dest.stem, dest.suffix
                i = 1
                while dest.exists():
                    dest = dest_dir / f'{stem}_dup{i}{suffix}'
                    i += 1
            shutil.copy2(p.path, dest)

    print('\n=== CLASSIFICATION SUMMARY ===')
    for scen, n in counts.most_common():
        print(f'  {scen}: {n}')
    if low_conf:
        print(f'\n{len(low_conf)} photos with LOW confidence:')
        for p, c in low_conf[:10]:
            print(f'  {p.path.name} -> {c.scenario} ({"; ".join(c.reasons)})')
        if len(low_conf) > 10:
            print(f'  ... and {len(low_conf) - 10} more')
    if not dry_run:
        print(f'\nCopied to: {keepers}')
    else:
        print('\n(DRY RUN — no files copied)')


def main():
    parser = argparse.ArgumentParser(description='Burst Culler')
    parser.add_argument('folder', help='Folder containing photos')
    parser.add_argument('--gap', type=float, default=3.0,
                        help='Max seconds between burst members (default 3)')
    parser.add_argument('--out', default=None,
                        help='Output folder (default: <folder>/keepers)')
    parser.add_argument('--classify-only', action='store_true',
                        help='Skip GUI — just classify + copy all photos '
                             'to scenario folders (no burst grouping)')
    parser.add_argument('--dry-run', action='store_true',
                        help='With --classify-only: show counts without '
                             'copying files')
    args = parser.parse_args()

    src = Path(args.folder).resolve()
    if not src.is_dir():
        print(f'Error: {src} is not a directory')
        sys.exit(1)

    keepers = Path(args.out).resolve() if args.out else (src / 'keepers')

    start = time.time()
    photos = scan_folder(src)
    if not photos:
        print('No photos with timestamps found.')
        sys.exit(0)

    if args.classify_only:
        run_classify_only(photos, keepers, dry_run=args.dry_run)
        return

    bursts = group_bursts(photos, args.gap)
    classify_bursts(bursts)
    multi = sum(1 for b in bursts if len(b.photos) > 1)
    single = len(bursts) - multi
    print(f'Grouped into {len(bursts)} bursts '
          f'({multi} multi-photo, {single} singles)')
    # Scenario breakdown
    from collections import Counter
    counts = Counter(b.scenario for b in bursts)
    print('By scenario:')
    for scen, n in counts.most_common():
        print(f'  {scen}: {n}')
    print(f'Scan took {time.time() - start:.1f}s')
    print(f'Keepers folder: {keepers}\n')
    print('Launching GUI...')

    CullerApp(bursts, src, keepers)


if __name__ == '__main__':
    main()
