import math
from .gear_data import SENSOR_FORMATS, STEP_TABLES

# Aperture at which diffraction becomes significant per sensor format
DIFFRACTION_LIMIT_F = {
    "MFT":        8.0,
    "APS-C":      11.0,
    "Full Frame":  16.0,
}


def effective_aperture(base_f: float, accessory: dict) -> float:
    """Effective f-number after applying accessory.
    Teleconverters reduce light; diopters do not affect aperture.
    """
    if accessory.get("type") == "teleconverter":
        stops = accessory["aperture_stops"]
        return base_f * (2 ** (stops / 2))
    return base_f


def effective_magnification(base_mag: float, lens: dict, accessory: dict) -> float:
    """Effective magnification after applying accessory.

    Teleconverter: multiplies by factor.
    Diopter (Raynox): adds focal_length / diopter_focal_length.
    """
    if accessory.get("type") == "teleconverter":
        return base_mag * accessory["factor"]
    elif accessory.get("type") == "diopter":
        diopter_focal_mm = 1000.0 / accessory["power_diopters"]
        return base_mag + (lens["focal_length_mm"] / diopter_focal_mm)
    return base_mag


def depth_of_field(N: float, coc: float, m: float) -> float:
    """Macro DoF formula: DoF = (2 * N * C * (m + 1)) / m²

    N   : effective f-number
    coc : circle of confusion in mm
    m   : magnification
    """
    if m <= 0:
        return float("inf")
    return (2.0 * N * coc * (m + 1.0)) / (m ** 2)


def lumix_step_for_overlap(desired_step_mm: float, step_table: dict) -> int:
    """Returns the Lumix step number whose physical step size (mm) is closest
    to desired_step_mm.  Works correctly at any magnification or with any
    dumb optical accessory (diopter, extension ring) the camera cannot detect.
    """
    best_step, best_diff = 1, float("inf")
    for step, step_mm in step_table.items():
        diff = abs(step_mm - desired_step_mm)
        if diff < best_diff:
            best_diff = diff
            best_step = step
    return best_step


def calculate_all(
    camera: dict,
    lens: dict,
    accessory: dict,
    camera_name: str,
    lens_name: str,
    acc_name: str,
    dial_mag: float,
    dial_f: float,
    subject_depth_mm: float,
    overlap: float,
    flash_recycle_s: float | None,
) -> dict:
    """Master calculation. Returns all bracketing parameters.

    overlap         : fraction of consecutive frames that overlap (0–1).
                      0.5 = 50% overlap; 0.0 = no overlap.
    flash_recycle_s : seconds between shots when using flash.
                      None means ambient — sequence time derived from FPS.
    """
    sensor = SENSOR_FORMATS[camera["sensor_format"]]
    coc = sensor["coc_mm"]

    eff_mag = effective_magnification(dial_mag, lens, accessory)
    eff_f   = effective_aperture(dial_f, accessory)
    dof     = depth_of_field(eff_f, coc, eff_mag)
    step    = dof * (1.0 - overlap)
    num_shots = math.ceil(subject_depth_mm / step) if step > 0 else 0

    # Sequence time
    fps = camera.get("fps_bracket", 10)
    if flash_recycle_s and flash_recycle_s > 0:
        seq_time_s = num_shots * flash_recycle_s
    else:
        seq_time_s = num_shots / fps

    # Diffraction warning
    diff_limit = DIFFRACTION_LIMIT_F.get(camera["sensor_format"], 8.0)
    diffraction_warning = eff_f >= diff_limit

    # Look up step table: prefer accessory-specific entry, fall back to bare lens.
    step_table = (
        STEP_TABLES.get((camera_name, lens_name, acc_name))
        or STEP_TABLES.get((camera_name, lens_name, "None"), {})
    )
    lumix_step      = lumix_step_for_overlap(step, step_table) if step_table else None
    lumix_step_mm   = step_table.get(lumix_step) if lumix_step is not None else None
    lumix_frac      = (lumix_step_mm / dof) if (lumix_step_mm is not None and dof > 0) else None

    return {
        "effective_magnification": round(eff_mag, 3),
        "effective_aperture":      round(eff_f, 1),
        "dof_mm":                  round(dof, 4),
        "step_mm":                 round(step, 4),
        "num_shots":               num_shots,
        "lumix_step":              lumix_step,
        "lumix_step_fraction":     lumix_frac,
        "rail_step_mm":            round(step, 4),
        "sequence_time_s":         round(seq_time_s, 1),
        "diffraction_warning":     diffraction_warning,
        "diff_limit_f":            diff_limit,
    }
