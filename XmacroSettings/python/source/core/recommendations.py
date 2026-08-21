"""Empirical recommendations for the tested Lumix G9 C3-3 macro preset."""

import json
from pathlib import Path

TESTED_LUMIX_STEP = 2
DEFAULT_CHARACTER_SIZE_MM = 20

def _coverage_rows() -> list[dict[str, int]]:
    """Load the published field-card source of truth for focus coverage."""
    repo_root = Path(__file__).resolve().parents[4]
    cards = json.loads((repo_root / "data/field-cards.json").read_text(encoding="utf-8"))
    g9 = next(camera for camera in cards["cameras"] if camera["camera_short"] == "G9")
    c3_3 = next(mode for mode in g9["modes"] if mode["code"] == "C3-3")
    return c3_3["coverage_guide"]["rows"]


_ROWS = _coverage_rows()
CHARACTER_SIZE_RECOMMENDATIONS = {
    row["target_mm"]: row["tested_1_2"] for row in _ROWS
}
ALTERNATIVE_MAGNIFICATION_RECOMMENDATIONS = {
    magnification: {
        row["target_mm"]: row[field_name] for row in _ROWS
    }
    for magnification, field_name in (
        ("1:4", "alternative_1_4"),
        ("1:1.3", "alternative_1_1_3"),
        ("1:1", "alternative_1_1"),
    )
}


def supported_coverage_sizes() -> tuple[int, ...]:
    """Return supported target coverage values in display order."""
    return tuple(CHARACTER_SIZE_RECOMMENDATIONS)


def supported_alternative_magnifications() -> tuple[str, ...]:
    """Return alternative magnifications in display order."""
    return tuple(ALTERNATIVE_MAGNIFICATION_RECOMMENDATIONS)


def shots_for_character_size(size_mm: int) -> int:
    """Return the tested shot count, defaulting to the 20 mm recommendation."""
    return CHARACTER_SIZE_RECOMMENDATIONS.get(
        size_mm,
        CHARACTER_SIZE_RECOMMENDATIONS[DEFAULT_CHARACTER_SIZE_MM],
    )


def alternative_shots_for_character_size(size_mm: int, magnification: str) -> int:
    """Return an estimated shot count for an alternative magnification."""
    recommendations = ALTERNATIVE_MAGNIFICATION_RECOMMENDATIONS.get(magnification)
    if recommendations is None:
        return shots_for_character_size(size_mm)
    return recommendations.get(
        size_mm,
        recommendations[DEFAULT_CHARACTER_SIZE_MM],
    )
