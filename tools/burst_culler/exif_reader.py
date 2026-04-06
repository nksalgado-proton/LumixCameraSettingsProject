"""
EXIF extraction via bundled exiftool.
Reads all tags needed for mode classification in a single batch call.
"""

import json
import subprocess
from dataclasses import dataclass, field
from datetime import datetime
from pathlib import Path

EXIFTOOL_PATH = Path(__file__).parent / 'bin' / 'exiftool.exe'

# Tags we extract for classification (exiftool short names)
TAGS = [
    'DateTimeOriginal', 'Model', 'LensModel', 'FocalLength',
    'FNumber', 'ExposureTime', 'ISO',
    'Flash', 'FocusMode', 'AFAreaMode', 'AFSubjectDetection',
    'ShootingMode', 'BurstMode', 'Bracketing',
    'PhotoStyle', 'ImageStabilization', 'ShutterType',
    'ExtTeleConv', 'MacroMode', 'SilentMode', 'ColorEffect',
    'ExposureMode', 'WhiteBalance', 'Orientation',
]


@dataclass
class PhotoExif:
    path: Path
    timestamp: datetime | None = None
    model: str = ''
    lens: str = ''
    focal_length: float = 0.0
    aperture: float = 0.0
    shutter_speed: float = 0.0  # in seconds
    iso: int = 0
    flash_fired: bool = False
    focus_mode: str = ''
    af_area_mode: str = ''
    af_subject: str = ''
    shooting_mode: str = ''
    burst_mode: bool = False
    bracketing: str = ''
    photo_style: str = ''
    shutter_type: str = ''
    ext_tele_conv: str = ''
    silent_mode: bool = False
    raw: dict = field(default_factory=dict)

    @property
    def focal_35mm(self) -> float:
        """MFT crop factor ~2x."""
        return self.focal_length * 2

    @property
    def is_flash_shot(self) -> bool:
        return self.flash_fired

    @property
    def is_focus_bracket(self) -> bool:
        return 'focus' in (self.bracketing or '').lower()

    @property
    def is_long_exposure(self) -> bool:
        return self.shutter_speed >= 1.0


def _parse_float(s: str) -> float:
    """Parse values like '6.3', '1/2000', '0.005'."""
    if not s:
        return 0.0
    s = str(s).strip()
    if '/' in s:
        try:
            n, d = s.split('/')
            return float(n) / float(d)
        except (ValueError, ZeroDivisionError):
            return 0.0
    try:
        return float(s)
    except ValueError:
        return 0.0


def _parse_int(s: str) -> int:
    try:
        return int(str(s).strip())
    except (ValueError, AttributeError):
        return 0


def _parse_timestamp(s: str) -> datetime | None:
    if not s:
        return None
    # Formats: "2026:03:30 08:58:12" or with fractions
    s = str(s).strip().split('.')[0].split('+')[0].split('-07:00')[0]
    for fmt in ('%Y:%m:%d %H:%M:%S', '%Y-%m-%d %H:%M:%S'):
        try:
            return datetime.strptime(s, fmt)
        except ValueError:
            continue
    return None


def _extract_focal(raw_val: str) -> float:
    """Focal length may come as '400.0 mm' or '400'."""
    if not raw_val:
        return 0.0
    s = str(raw_val).replace('mm', '').strip()
    return _parse_float(s)


def _extract_aperture(raw_val: str) -> float:
    """FNumber may come as '6.3' or '63/10'."""
    return _parse_float(str(raw_val).replace('f/', '').strip())


def _extract_shutter(raw_val: str) -> float:
    """ExposureTime in seconds. '1/2000' = 0.0005."""
    return _parse_float(raw_val)


def read_exif_batch(files: list[Path]) -> list[PhotoExif]:
    """Read EXIF from many files using an exiftool argfile (fast, no
    command-line length limits)."""
    if not files:
        return []

    import tempfile
    # Write file list to temp argfile (avoids Windows cmd-line length limit)
    with tempfile.NamedTemporaryFile(mode='w', encoding='utf-8',
                                     suffix='.txt', delete=False) as tf:
        argfile_path = tf.name
        for f in files:
            tf.write(f'{f}\n')

    cmd = [
        str(EXIFTOOL_PATH),
        '-json',
        '-charset', 'filename=UTF8',
        '-charset', 'UTF8',
        '-@', argfile_path,
    ]
    for tag in TAGS:
        cmd.append(f'-{tag}')

    try:
        result = subprocess.run(
            cmd, capture_output=True, text=True, encoding='utf-8',
            check=False
        )
        if result.returncode != 0 and not result.stdout:
            print(f'ExifTool error: {result.stderr[:500]}')
            return []
        data = json.loads(result.stdout or '[]')
    except (subprocess.SubprocessError, json.JSONDecodeError) as e:
        print(f'ExifTool failed: {e}')
        return []
    finally:
        try:
            Path(argfile_path).unlink()
        except Exception:
            pass

    photos = []
    for entry in data:
        source = Path(entry.get('SourceFile', ''))
        photo = PhotoExif(
            path=source,
            timestamp=_parse_timestamp(entry.get('DateTimeOriginal', '')),
            model=str(entry.get('Model', '')),
            lens=str(entry.get('LensModel', '')).strip(),
            focal_length=_extract_focal(entry.get('FocalLength', '')),
            aperture=_extract_aperture(entry.get('FNumber', '')),
            shutter_speed=_extract_shutter(entry.get('ExposureTime', '')),
            iso=_parse_int(entry.get('ISO', 0)),
            flash_fired='did not fire' not in str(entry.get('Flash', '')).lower()
                        and 'off' not in str(entry.get('Flash', '')).lower(),
            focus_mode=str(entry.get('FocusMode', '')),
            af_area_mode=str(entry.get('AFAreaMode', '')),
            af_subject=str(entry.get('AFSubjectDetection', '')),
            shooting_mode=str(entry.get('ShootingMode', '')),
            burst_mode=str(entry.get('BurstMode', '')).lower() not in
                      ('', 'off', 'none', '0'),
            bracketing=str(entry.get('BurstMode', '')),
            photo_style=str(entry.get('PhotoStyle', '')),
            shutter_type=str(entry.get('ShutterType', '')),
            ext_tele_conv=str(entry.get('ExtTeleConv', '')),
            silent_mode='on' in str(entry.get('SilentMode', '')).lower(),
            raw=entry,
        )
        photos.append(photo)
    return photos


def read_exif_single(path: Path) -> PhotoExif | None:
    result = read_exif_batch([path])
    return result[0] if result else None
