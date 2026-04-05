"""
Heuristic mode classifier.
Scores each photo against known scenarios using EXIF signals.
Returns the best-match scenario + confidence level.
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
    scenario: str  # folder name
    confidence: Confidence
    reasons: list[str]  # human-readable evidence
    score: int  # numeric score for ranking


# Scenario folder names (destination subfolders)
SCENARIOS = {
    'stacks': 'Focus stacks (keep all frames)',
    'macro': 'Handheld macro with flash',
    'tripod-macro': 'Tripod macro with flash bracket',
    'wildlife': 'Wildlife / Birds / Action',
    'birds-crop': 'Birds with Crop Zoom',
    'landscape': 'Landscape',
    'portrait': 'Portrait / People',
    'street': 'Street / General',
    'indoor': 'Indoor / Low Light',
    'lightning': 'Lightning / Long exposure',
    'video-travel': 'Video travel',
    'video-wildlife': 'Video wildlife',
    'other': 'Uncategorized',
}


def classify(p: PhotoExif) -> Classification:
    """Classify a photo into a scenario folder with confidence."""
    reasons = []

    # RULE 1: Focus bracket → stacks (highest priority, unambiguous)
    if p.is_focus_bracket:
        return Classification('stacks', Confidence.HIGH,
                              ['Focus bracketing detected'], 100)

    # RULE 2: Long exposure with fixed ISO → Lightning
    if p.shutter_speed >= 3.0 and p.iso <= 800:
        return Classification('lightning', Confidence.HIGH,
                              [f'Long exposure {p.shutter_speed:.0f}s',
                               f'low ISO {p.iso}'], 95)

    # RULE 3: Flash fired + Macro lens → Macro (Nelson only uses macro lens
    # for macro, so this is definitive)
    lens_lower = p.lens.lower()
    # Match known macro lens patterns across branding variations:
    # Olympus/OM: "60mm F2.8", "90mm F3.5" are always macro
    # "Macro" in name (Panasonic Leica, Sigma, Tamron)
    is_macro_lens = (
        'macro' in lens_lower
        or '60mm f2.8' in lens_lower
        or '60mm f/2.8' in lens_lower
        or '90mm f3.5' in lens_lower
        or '90mm f/3.5' in lens_lower
        or '105mm' in lens_lower  # Sigma 105 Macro, etc.
    )
    # Fallback: if lens field is empty (common on G9 MkI JPEGs),
    # infer from focal length — 60mm/90mm on MFT is almost always a macro lens
    if not lens_lower and p.focal_length in (60.0, 90.0, 105.0):
        is_macro_lens = True
    if p.flash_fired and is_macro_lens:
        return Classification('macro', Confidence.HIGH,
                              ['Flash + Macro lens'], 95)

    # RULE 3a: Macro lens + narrow aperture (no flash) → natural-light macro
    if is_macro_lens and p.aperture >= 8.0:
        return Classification('macro', Confidence.MEDIUM,
                              ['Macro lens', f'Narrow f/{p.aperture}'], 80)

    # RULE 3b: Flash + MF + narrow aperture = textbook macro
    # (catches cases where lens model is missing from EXIF)
    if (p.flash_fired
            and p.focus_mode.lower().startswith('manual')
            and p.aperture >= 8.0):
        return Classification('macro', Confidence.HIGH,
                              ['Flash', 'MF', f'Narrow f/{p.aperture}'], 95)

    # RULE 4: Animal detection → Wildlife family
    subject_lower = p.af_subject.lower()
    if 'animal' in subject_lower:
        reasons.append(f'Subject detect: {p.af_subject}')
        # Check if JPEG-only output on G9M2 (indicates Crop Zoom mode)
        if p.path.suffix.lower() in ('.jpg', '.jpeg') and p.model == 'DC-G9M2':
            reasons.append('JPEG-only (Crop Zoom)')
            return Classification('birds-crop', Confidence.MEDIUM, reasons, 80)
        return Classification('wildlife', Confidence.HIGH, reasons, 85)

    # RULE 5: Human detection → Portrait family
    if 'human' in subject_lower or 'face' in subject_lower:
        reasons.append(f'Subject detect: {p.af_subject}')
        if p.silent_mode:
            reasons.append('Silent mode ON')
            return Classification('indoor', Confidence.HIGH, reasons, 90)
        if p.aperture <= 3.0:
            reasons.append(f'Wide aperture f/{p.aperture}')
            return Classification('portrait', Confidence.HIGH, reasons, 85)
        if p.aperture >= 5.6:
            reasons.append(f'Medium aperture f/{p.aperture}')
            return Classification('street', Confidence.MEDIUM, reasons, 70)
        return Classification('portrait', Confidence.MEDIUM, reasons, 70)

    # RULE 6: No subject detect + narrow aperture + AFS → Landscape
    # Panasonic stores "Auto" for AFS, "Auto, Continuous" for AFC
    focus_mode_lower = p.focus_mode.lower()
    is_afs = ('single' in focus_mode_lower or 'afs' in focus_mode_lower
              or focus_mode_lower.strip() == 'auto')
    if is_afs:
        if p.aperture >= 7.0:
            reasons.append(f'AFS + narrow f/{p.aperture}')
            # Photo style Scenery = landscape
            if 'scenery' in p.photo_style.lower() or 'landscape' in p.photo_style.lower():
                reasons.append(f'Photo style: {p.photo_style}')
                return Classification('landscape', Confidence.HIGH, reasons, 85)
            return Classification('landscape', Confidence.MEDIUM, reasons, 70)
        # AFS with medium aperture → Street/General
        reasons.append(f'AFS + f/{p.aperture}')
        return Classification('street', Confidence.MEDIUM, reasons, 60)

    # RULE 6b: Tracking AF + fast SS + no flash → wildlife
    # (Tracking AF is only used for moving subjects)
    if (not p.flash_fired and 'tracking' in p.af_area_mode.lower()
            and p.shutter_speed > 0 and 1 / p.shutter_speed >= 500):
        ss_recip = 1 / p.shutter_speed
        reasons = [f'Tracking AF + 1/{ss_recip:.0f}s']
        return Classification('wildlife', Confidence.MEDIUM, reasons, 72)

    # RULE 7: Telephoto + fast shutter + no flash → likely wildlife
    # (covers MF-on-moving-subject case where subject detect is n/a)
    if (not p.flash_fired and p.focal_length >= 100
            and p.shutter_speed > 0):
        ss_recip = 1 / p.shutter_speed
        if ss_recip >= 500:  # 1/500s or faster
            reasons.append(f'Tele {p.focal_length:.0f}mm + 1/{ss_recip:.0f}s')
            if 'tracking' in p.af_area_mode.lower():
                reasons.append('Tracking AF')
                return Classification('wildlife', Confidence.MEDIUM,
                                      reasons, 72)
            return Classification('wildlife', Confidence.LOW, reasons, 55)

    # RULE 8: Manual focus, no flash, no long exposure → likely macro w/o flash
    # (user may have turned off flash, still macro setup)
    if p.focus_mode.lower().startswith('manual') and not p.flash_fired:
        if p.aperture >= 8.0:
            reasons = [f'MF + f/{p.aperture}', 'No flash']
            return Classification('macro', Confidence.LOW, reasons, 50)

    # RULE 9: AFC without subject detect → guess based on focal length
    # Panasonic uses "AF-C" or "Auto, Continuous" for continuous AF
    is_afc = ('continuous' in focus_mode_lower
              or 'afc' in focus_mode_lower
              or 'af-c' in focus_mode_lower)
    if is_afc:
        if p.focal_35mm >= 200:  # 100mm+ on MFT
            reasons.append(f'AFC + tele {p.focal_length:.0f}mm')
            return Classification('wildlife', Confidence.LOW, reasons, 45)

    # FALLBACK: unknown
    return Classification('other', Confidence.LOW,
                          ['No strong signals matched'], 0)


def format_reasons(c: Classification) -> str:
    return '; '.join(c.reasons)
