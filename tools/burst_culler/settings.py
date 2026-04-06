"""
Persistent settings for Burst Culler.
Stored as JSON in %LOCALAPPDATA%/BurstCuller/settings.json.
"""

import json
import os
import platform
from pathlib import Path


def _settings_path() -> Path:
    if platform.system() == 'Windows':
        base = os.environ.get('LOCALAPPDATA', os.path.expanduser('~'))
        d = Path(base) / 'BurstCuller'
    else:
        d = Path.home() / '.local' / 'share' / 'burst_culler'
    d.mkdir(parents=True, exist_ok=True)
    return d / 'settings.json'


DEFAULT_SETTINGS = {
    'peaking_on': False,
    'peaking_color': 'Red',
    'peaking_sensitivity': 0.4,  # 0=strict, 1=loose
    'font_scale': 1.0,           # 0.8, 0.9, 1.0, 1.1, 1.2
    'stack_anim_speed': 5,       # fps: 1=slow, 3=med, 5=fast, 10=max
    'last_source': '',
    'last_destination': '',
}


def load_settings() -> dict:
    path = _settings_path()
    settings = dict(DEFAULT_SETTINGS)
    if path.exists():
        try:
            with open(path, 'r', encoding='utf-8') as f:
                saved = json.load(f)
            settings.update(saved)
        except (json.JSONDecodeError, TypeError):
            pass
    return settings


def save_settings(settings: dict) -> None:
    path = _settings_path()
    with open(path, 'w', encoding='utf-8') as f:
        json.dump(settings, f, indent=2, ensure_ascii=False)
