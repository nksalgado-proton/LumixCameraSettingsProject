# ---------------------------------------------------------------------------
# Gear database
# All data lives here. To add new gear, just extend the relevant dict.
# ---------------------------------------------------------------------------

SENSOR_FORMATS = {
    "MFT":        {"coc_mm": 0.015, "crop_factor": 2.0},
    "APS-C":      {"coc_mm": 0.019, "crop_factor": 1.5},
    "Full Frame":  {"coc_mm": 0.029, "crop_factor": 1.0},
}

CAMERAS = {
    "Lumix G9": {
        "sensor_format": "MFT",
        "fps_bracket": 2.9,   # measured with 60mm; 90mm gives ~3.0 fps (close enough)
    },
    "Lumix G9 II": {
        "sensor_format": "MFT",
        "fps_bracket": 3.3,   # measured with 60mm; 90mm gives ~3.0 fps (close enough)
    },
}

LENSES = {
    "Panasonic 30mm f/2.8 Macro": {
        "focal_length_mm": 30,
        "max_aperture": 2.8,
        "mag_range": (0.1, 1.0),
        "compatible_accessories": ["None", "Raynox DCR-250"],
    },
    "OM System 60mm f/2.8 Macro": {
        "focal_length_mm": 60,
        "max_aperture": 2.8,
        "mag_range": (0.1, 1.0),
        "compatible_accessories": ["None", "Raynox DCR-250"],
    },
    "OM System 90mm f/3.5 Macro": {
        "focal_length_mm": 90,
        "max_aperture": 3.5,
        "mag_range": (0.1, 2.0),
        "compatible_accessories": ["None", "OM MC-20"],
    },
}

ACCESSORIES = {
    "None": {
        "type": None,
    },
    "OM MC-20": {
        "type": "teleconverter",
        "factor": 2.0,
        "aperture_stops": 2,
    },
    "Raynox DCR-250": {
        "type": "diopter",
        "power_diopters": 8.0,
    },
}

FLASH_UNITS = {
    "Ambient / No Flash": {
        "recycle_s": None,
        "diffuser": None,
    },
    "Godox V-860 III": {
        "recycle_s": 1.5,
        "diffuser": "AK diffuser",
    },
    "Godox AD100Pro": {
        "recycle_s": 1.5,
        "diffuser": "Godox ML-CD15",
    },
}

# ---------------------------------------------------------------------------
# Focus-step calibration tables
# Keyed by (camera_name, lens_name, accessory_name).
# Values are PHYSICAL STEP SIZE in mm — valid at any magnification.
#
# Lookup fallback in calculations.py:
#   1. Try (camera, lens, accessory)
#   2. Fall back to (camera, lens, "None")
#
# Dumb accessories (Raynox DCR-250): no electronic contact → same physical
# steps as bare lens → no entry needed, fallback handles it automatically.
#
# Electronic accessories (OM MC-20): camera halves physical step when TC
# is attached (compensates for doubled magnification) → own entry required.
#
# step_mm = depth / (N − 1)  where N = frame where far edge becomes sharp.
# Entries marked "extrapolated" are scaled from bare-lens table at ratio
# confirmed by 3 measured data points.
# ---------------------------------------------------------------------------

STEP_TABLES = {
    ("Lumix G9", "OM System 60mm f/2.8 Macro", "None"): {
        # Full 10-step calibration. No plateau.
        # Measured: f/5.6, 1:1, 26mm depth (DoF = 0.336mm)
        1:  0.0502,  # N=519
        2:  0.1008,  # N=259
        3:  0.1529,  # N=171
        4:  0.2031,  # N=129
        5:  0.2407,  # N=109
        6:  0.2889,  # N=91
        7:  0.3421,  # N=77
        8:  0.3939,  # N=67
        9:  0.4333,  # N=61
        10: 0.4906,  # N=54   — no plateau
    },
    ("Lumix G9 II", "OM System 60mm f/2.8 Macro", "None"): {
        # Full 10-step calibration. Steps 9–10 plateau (same N=59).
        # Measured: f/5.6, 1:1, 26mm depth (DoF = 0.336mm)
        1:  0.0502,  # N=519
        2:  0.0970,  # N=269
        3:  0.1520,  # N=172
        4:  0.1757,  # N=149
        5:  0.2407,  # N=109
        6:  0.3133,  # N=84
        7:  0.3611,  # N=73
        8:  0.4194,  # N=63
        9:  0.4483,  # N=59  — plateau
        10: 0.4483,  # N=59  — same as 9; avoid for stacking
    },
    ("Lumix G9 II", "OM System 90mm f/3.5 Macro", "None"): {
        # Full 10-step calibration. No plateau. Steps never exceed 1 DoF.
        # Measured: f/5.6, 1:1, 26mm depth (DoF = 0.336mm)
        1:  0.0325,  # N=800
        2:  0.0652,  # N=400
        3:  0.0967,  # N=270
        4:  0.1307,  # N=200
        5:  0.1595,  # N=164
        6:  0.2016,  # N=130
        7:  0.2281,  # N=115
        8:  0.2626,  # N=100
        9:  0.2889,  # N=91
        10: 0.3250,  # N=81
    },
    ("Lumix G9 II", "OM System 90mm f/3.5 Macro", "OM MC-20"): {
        # Camera halves physical step when MC-20 attached (~0.52× bare values).
        # Confirmed on 3 points (steps 2, 4, 8): ratio 0.524 / 0.522 / 0.514.
        # Measured: f/10 effective, 17mm depth.
        # Remaining steps extrapolated from bare table × 0.520.
        1:  0.0169,  # extrapolated
        2:  0.0341,  # N=499  — measured
        3:  0.0503,  # extrapolated
        4:  0.0683,  # N=250  — measured
        5:  0.0829,  # extrapolated
        6:  0.1048,  # extrapolated
        7:  0.1186,  # extrapolated
        8:  0.1349,  # N=127  — measured
        9:  0.1502,  # extrapolated
        10: 0.1690,  # extrapolated
    },
    ("Lumix G9", "OM System 90mm f/3.5 Macro", "None"): {
        # G9 produces ~65% of G9 II step size — firmware difference with this lens.
        # Max step 10 = 0.2063 mm; overlap never below ~39% on this body.
        # Step 1 extrapolated (N > 999 at 26mm depth — camera limit reached).
        1:  0.0217,  # extrapolated
        2:  0.0434,  # N=600
        3:  0.0637,  # N=409
        4:  0.0844,  # N=309
        5:  0.1048,  # N=249
        6:  0.1262,  # N=207
        7:  0.1477,  # N=177
        8:  0.1677,  # N=156
        9:  0.1884,  # N=139
        10: 0.2063,  # N=127  — max useful step; never creates gaps
    },
    ("Lumix G9 II", "Panasonic 30mm f/2.8 Macro", "None"): {
        # ~5× coarser than 60mm. Only step 1 avoids gaps. Plateau at steps 9–10.
        # For gap-free in-camera stacking: step 1 only. Otherwise use rail.
        1:  0.2407,  # N=109  — only gap-free step
        2:  0.5098,  # N=52   — gaps
        3:  0.6667,  # N=40
        4:  1.0400,  # N=26
        5:  1.1818,  # N=23
        6:  1.3684,  # N=20
        7:  1.4444,  # N=19
        8:  1.7333,  # N=16
        9:  2.0000,  # N=14   — plateau
        10: 2.0000,  # N=14   — plateau
    },
    ("Lumix G9", "Panasonic 30mm f/2.8 Macro", "None"): {
        # ~69% of G9 II step size. Steps 1–2 avoid gaps (step 2 borderline ~0% overlap).
        # Steps 3–10 create gaps. No plateau at step 10.
        1:  0.1667,  # N=157
        2:  0.3333,  # N=79   — borderline, ~0% overlap
        3:  0.4815,  # N=55   — gaps
        4:  0.6667,  # N=40
        5:  0.7879,  # N=34
        6:  0.9286,  # N=29
        7:  1.1304,  # N=24
        8:  1.2381,  # N=22
        9:  1.3684,  # N=20
        10: 1.5294,  # N=18   — no plateau
    },
    # G9 + 90mm + MC-20: not yet measured. Likely ~0.52× G9 bare 90mm values.
    # G9 + 30mm + Raynox: falls back to G9 + 30mm + "None" automatically.
    # G9 II + 60mm + Raynox: falls back to G9 II + 60mm + "None" automatically.
}