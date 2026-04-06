"""
Universal photo classifier — camera-agnostic.
Classifies photos into 7 categories based solely on EXIF data.
No camera-specific mode names (C1, C2, etc.) — pure photography signals.
"""

from dataclasses import dataclass
from enum import Enum

from exif_reader import PhotoExif


class Confidence(Enum):
    HIGH = 'HIGH'
    MEDIUM = 'MEDIUM'
    LOW = 'LOW'


@dataclass
class Classification:
    scenario: str
    confidence: Confidence
    reasons: list[str]
    score: int


# 7 universal categories — ordered for display
SCENARIOS = {
    'macro':            'Macro / Close-up',
    'stacks':           'Focus Stacks',
    'wildlife-action':  'Wildlife in Action',
    'wildlife-static':  'Wildlife Static / Perched',
    'people':           'People / Portraits',
    'night':            'Night / Long Exposure',
    'general-scene':    'General Scene',
}

SCENARIO_ORDER = list(SCENARIOS.keys())


def _is_macro_lens(p: PhotoExif) -> bool:
    """Check if the lens is a known macro lens."""
    lens = p.lens.lower()
    if 'macro' in lens:
        return True
    # Olympus/OM System 60mm f/2.8 and 90mm f/3.5 are always macro
    if '60mm f2.8' in lens or '60mm f/2.8' in lens:
        return True
    if '90mm f3.5' in lens or '90mm f/3.5' in lens:
        return True
    if '105mm' in lens:  # Sigma 105 Macro, etc.
        return True
    # Fallback: if lens field is empty, infer from focal length
    if not lens and p.focal_length in (60.0, 90.0, 105.0):
        return True
    return False


def _is_continuous_af(p: PhotoExif) -> bool:
    """AF-C / Continuous / Tracking."""
    fm = p.focus_mode.lower()
    return ('continuous' in fm or 'af-c' in fm or 'afc' in fm)


def _is_single_af(p: PhotoExif) -> bool:
    """AF-S / Single / One-shot. Also 'Auto' without 'Continuous'."""
    fm = p.focus_mode.lower()
    return ('single' in fm or 'af-s' in fm or 'afs' in fm
            or fm.strip() == 'auto')


def _has_animal_detect(p: PhotoExif) -> bool:
    return 'animal' in p.af_subject.lower()


def _has_human_detect(p: PhotoExif) -> bool:
    s = p.af_subject.lower()
    return 'human' in s or 'face' in s


def _ss_reciprocal(p: PhotoExif) -> float:
    """Shutter speed as reciprocal (e.g., 1/1000 returns 1000)."""
    if p.shutter_speed > 0:
        return 1.0 / p.shutter_speed
    return 0


def classify(p: PhotoExif) -> Classification:
    """Classify a photo into one of 7 universal categories."""

    # ── RULE 1: Focus Bracket → Stacks (highest priority, unambiguous)
    if p.is_focus_bracket:
        return Classification('stacks', Confidence.HIGH,
                              ['Focus bracket detected'], 100)

    # ── RULE 2: Long Exposure → Night
    if p.shutter_speed >= 1.0 and p.iso <= 800:
        return Classification('night', Confidence.HIGH,
                              [f'Long exposure {p.shutter_speed:.1f}s',
                               f'Low ISO {p.iso}'], 95)

    # ── RULE 3: Flash + Macro lens → Macro
    is_macro = _is_macro_lens(p)
    if p.flash_fired and is_macro:
        return Classification('macro', Confidence.HIGH,
                              ['Flash + Macro lens'], 95)

    # ── RULE 3b: Flash + MF + narrow aperture → Macro
    if (p.flash_fired
            and p.focus_mode.lower().startswith('manual')
            and p.aperture >= 8.0):
        return Classification('macro', Confidence.HIGH,
                              ['Flash', 'MF', f'f/{p.aperture}'], 93)

    # ── RULE 3c: Macro lens + narrow aperture (no flash) → Macro
    if is_macro and p.aperture >= 8.0:
        return Classification('macro', Confidence.MEDIUM,
                              ['Macro lens', f'f/{p.aperture}'], 80)

    # ── RULE 4: Animal detection → Wildlife family
    if _has_animal_detect(p):
        reasons = [f'Animal detect: {p.af_subject}']
        if _is_continuous_af(p) and p.burst_mode:
            reasons.append('AF-C + Burst')
            return Classification('wildlife-action', Confidence.HIGH,
                                  reasons, 90)
        if _is_continuous_af(p):
            reasons.append('AF-C')
            return Classification('wildlife-action', Confidence.MEDIUM,
                                  reasons, 82)
        reasons.append('Single AF / static')
        return Classification('wildlife-static', Confidence.HIGH,
                              reasons, 85)

    # ── RULE 5: Human/face detection → People
    if _has_human_detect(p):
        return Classification('people', Confidence.HIGH,
                              [f'Human detect: {p.af_subject}'], 85)

    # ── RULE 6: Telephoto + continuous AF + burst + fast SS → Wildlife Action
    ss = _ss_reciprocal(p)
    if (not p.flash_fired and p.focal_length >= 100
            and _is_continuous_af(p) and p.burst_mode
            and ss >= 500):
        reasons = [f'Tele {p.focal_length:.0f}mm', 'AF-C', 'Burst',
                   f'1/{ss:.0f}s']
        return Classification('wildlife-action', Confidence.MEDIUM,
                              reasons, 75)

    # ── RULE 7: Telephoto + fast SS (any AF) → Wildlife
    if (not p.flash_fired and p.focal_length >= 100 and ss >= 1000):
        reasons = [f'Tele {p.focal_length:.0f}mm', f'1/{ss:.0f}s']
        if _is_continuous_af(p):
            reasons.append('AF-C')
            return Classification('wildlife-action', Confidence.LOW,
                                  reasons, 60)
        if 'tracking' in p.af_area_mode.lower():
            reasons.append('Tracking AF')
            return Classification('wildlife-action', Confidence.LOW,
                                  reasons, 58)
        return Classification('wildlife-static', Confidence.LOW,
                              reasons, 55)

    # ── RULE 8: Telephoto + single AF → Wildlife Static
    if (not p.flash_fired and p.focal_length >= 100
            and _is_single_af(p)):
        return Classification('wildlife-static', Confidence.LOW,
                              [f'Tele {p.focal_length:.0f}mm', 'AF-S'],
                              50)

    # ── RULE 9: Continuous AF + telephoto → Wildlife (fallback)
    if _is_continuous_af(p) and p.focal_length >= 100:
        return Classification('wildlife-action', Confidence.LOW,
                              ['AF-C', f'Tele {p.focal_length:.0f}mm'],
                              45)

    # ── DEFAULT: General Scene
    return Classification('general-scene', Confidence.LOW,
                          ['No strong signals'], 0)


def format_reasons(c: Classification) -> str:
    return '; '.join(c.reasons)
