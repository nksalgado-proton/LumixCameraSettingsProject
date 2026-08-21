import re
import unittest
import os
import json
from pathlib import Path

from core.recommendations import (
    ALTERNATIVE_MAGNIFICATION_RECOMMENDATIONS,
    CHARACTER_SIZE_RECOMMENDATIONS,
    DEFAULT_CHARACTER_SIZE_MM,
    TESTED_LUMIX_STEP,
    alternative_shots_for_character_size,
    shots_for_character_size,
    supported_alternative_magnifications,
    supported_coverage_sizes,
)


_REPO_ROOT = Path(__file__).resolve().parents[4]
_CARDS = json.loads((_REPO_ROOT / "data/field-cards.json").read_text(encoding="utf-8"))
_G9 = next(camera for camera in _CARDS["cameras"] if camera["camera_short"] == "G9")
_C3_3 = next(mode for mode in _G9["modes"] if mode["code"] == "C3-3")
_ROWS = _C3_3["coverage_guide"]["rows"]

EXPECTED_RECOMMENDATIONS = {row["target_mm"]: row["tested_1_2"] for row in _ROWS}
EXPECTED_ALTERNATIVES = {
    magnification: {row["target_mm"]: row[field_name] for row in _ROWS}
    for magnification, field_name in (
        ("1:4", "alternative_1_4"),
        ("1:1.3", "alternative_1_1_3"),
        ("1:1", "alternative_1_1"),
    )
}


class RecommendationTests(unittest.TestCase):
    @classmethod
    def setUpClass(cls):
        cls.repo_root = Path(__file__).resolve().parents[4]
        cls.source_pwa = (
            cls.repo_root / "XmacroSettings/python/source/pwa/index.html"
        ).read_text(encoding="utf-8")
        cls.published_pwa = (
            cls.repo_root / "docs/macro-bracket/index.html"
        ).read_text(encoding="utf-8")

    def test_python_recommendations_and_default(self):
        self.assertEqual(CHARACTER_SIZE_RECOMMENDATIONS, EXPECTED_RECOMMENDATIONS)
        self.assertEqual(DEFAULT_CHARACTER_SIZE_MM, 20)
        self.assertEqual(TESTED_LUMIX_STEP, 2)
        self.assertEqual(supported_coverage_sizes(), tuple(EXPECTED_RECOMMENDATIONS))
        for size_mm, expected_shots in EXPECTED_RECOMMENDATIONS.items():
            with self.subTest(size_mm=size_mm):
                self.assertEqual(shots_for_character_size(size_mm), expected_shots)

    def test_python_alternative_magnification_recommendations(self):
        self.assertEqual(
            ALTERNATIVE_MAGNIFICATION_RECOMMENDATIONS,
            EXPECTED_ALTERNATIVES,
        )
        self.assertEqual(
            supported_alternative_magnifications(),
            tuple(EXPECTED_ALTERNATIVES),
        )
        for magnification, recommendations in EXPECTED_ALTERNATIVES.items():
            for size_mm, expected_shots in recommendations.items():
                with self.subTest(magnification=magnification, size_mm=size_mm):
                    self.assertEqual(
                        alternative_shots_for_character_size(size_mm, magnification),
                        expected_shots,
                    )

    def test_unknown_alternative_uses_tested_default(self):
        self.assertEqual(alternative_shots_for_character_size(999, "1:4"), 6)
        self.assertEqual(alternative_shots_for_character_size(20, "unknown"), 20)

    def test_pwa_recommendations_match_python(self):
        block = re.search(
            r"const CHARACTER_SIZE_RECOMMENDATIONS = \{(?P<body>.*?)\};",
            self.source_pwa,
            re.DOTALL,
        )
        self.assertIsNotNone(block)
        pwa_recommendations = {
            int(size): int(shots)
            for size, shots in re.findall(r"(\d+):\s*(\d+)", block.group("body"))
        }
        self.assertEqual(pwa_recommendations, EXPECTED_RECOMMENDATIONS)
        self.assertRegex(
            self.source_pwa,
            r'<option value="20" selected>20 mm</option>',
        )
        self.assertRegex(
            self.source_pwa,
            r'id="testedShots">20</span>',
        )
        self.assertIn("<h4>Alternatives</h4>", self.source_pwa)
        self.assertIn('data-alternative-mag="1:4">6</span>', self.source_pwa)
        self.assertIn('data-alternative-mag="1:1.3">40</span>', self.source_pwa)
        self.assertIn('data-alternative-mag="1:1">60</span>', self.source_pwa)
        self.assertIn("40 shots ≈ 40 mm coverage", self.source_pwa)
        self.assertIn("estimated from depth of field", self.source_pwa)

        alternatives_block = re.search(
            r"const ALTERNATIVE_MAGNIFICATION_RECOMMENDATIONS = "
            r"\{(?P<body>.*?)\n\};",
            self.source_pwa,
            re.DOTALL,
        )
        self.assertIsNotNone(alternatives_block)
        for magnification, expected in EXPECTED_ALTERNATIVES.items():
            row = re.search(
                rf'"{re.escape(magnification)}":\s*\{{(?P<body>[^}}]+)\}}',
                alternatives_block.group("body"),
            )
            self.assertIsNotNone(row)
            actual = {
                int(size): int(shots)
                for size, shots in re.findall(r"(\d+):\s*(\d+)", row.group("body"))
            }
            self.assertEqual(actual, expected)

    def test_published_pwa_and_cache_match_source(self):
        self.assertEqual(self.published_pwa, self.source_pwa)
        source_sw = (
            self.repo_root / "XmacroSettings/python/source/pwa/sw.js"
        ).read_text(encoding="utf-8")
        published_sw = (
            self.repo_root / "docs/macro-bracket/sw.js"
        ).read_text(encoding="utf-8")
        self.assertEqual(published_sw, source_sw)
        self.assertEqual(
            source_sw.splitlines()[0],
            "const CACHE_NAME = 'macro-bracket-v4-final';",
        )

    def test_field_app_cache_refreshes_final_shell_and_data(self):
        field_sw = (self.repo_root / "docs/sw.js").read_text(encoding="utf-8")
        self.assertEqual(
            field_sw.splitlines()[0],
            "const CACHE_NAME = 'travel-cards-v11-waterfall';",
        )
        for asset in (
            "'./'",
            "'./index.html'",
            "'./field-cards.json'",
            "'./manifest.json'",
            "'./icon-192.png'",
            "'./icon-512.png'",
        ):
            with self.subTest(asset=asset):
                self.assertIn(asset, field_sw)

    def test_field_app_contains_final_focus_coverage_guide(self):
        source_cards = (self.repo_root / "data/field-cards.json").read_text(
            encoding="utf-8"
        )
        published_cards = (
            self.repo_root / "docs/field-cards.json"
        ).read_text(encoding="utf-8")
        field_app = (self.repo_root / "docs/index.html").read_text(encoding="utf-8")

        self.assertEqual(published_cards, source_cards)
        self.assertIn('"version": "2.5"', source_cards)
        self.assertIn('"status": "FINAL CAMERA CONFIGURATION"', source_cards)
        self.assertIn('"coverage_guide"', source_cards)
        self.assertIn('"baseline": "Step 2 / 40 images / 0/+"', source_cards)
        self.assertIn('"tested_1_2": 20', source_cards)
        self.assertIn('"alternative_1_4": 6', source_cards)
        self.assertIn('"alternative_1_1_3": 40', source_cards)
        self.assertIn('"alternative_1_1": 60', source_cards)
        self.assertIn("coverage-guide", field_app)
        self.assertIn("Focus Bracket Coverage", field_app)

        cards_data = json.loads(source_cards)
        hero12 = next(
            camera for camera in cards_data["cameras"] if camera["camera_short"] == "H12"
        )
        self.assertEqual(
            [mode["code"] for mode in hero12["modes"]],
            ["TW", "VID", "PHOTO", "BURST", "NIGHT"],
        )
        video = next(mode for mode in hero12["modes"] if mode["code"] == "VID")
        self.assertTrue(
            any(alternative["name"].startswith("Snorkel") for alternative in video["alternatives"])
        )
        g9ii = next(
            camera for camera in cards_data["cameras"] if camera["camera_short"] == "G9II"
        )
        long_exposure = next(mode for mode in g9ii["modes"] if mode["code"] == "C3-7")
        waterfall = next(
            alternative
            for alternative in long_exposure["alternatives"]
            if alternative["name"].startswith("Leica 9mm")
        )
        self.assertIn("VND2-32/CPL", waterfall["name"])
        self.assertTrue(
            any(change["value"] == "55→72mm direct" for change in waterfall["changes"])
        )
        self.assertTrue(
            any(
                alternative["name"].startswith("VND2-400")
                for alternative in long_exposure["alternatives"]
            )
        )
        inventory = (
            self.repo_root / "guides/Camera-Mode-Redesign-US-Parks-2026.md"
        ).read_text(encoding="utf-8")
        self.assertIn("KF01.2001", inventory)
        self.assertIn("NANO-K 58mm HMC CPL", inventory)
        self.assertIn("NANO-K 72mm HMC CPL", inventory)
        self.assertIn("Filter selection quick guide", inventory)
        g9 = next(camera for camera in cards_data["cameras"] if camera["camera_short"] == "G9")
        c3_3 = next(mode for mode in g9["modes"] if mode["code"] == "C3-3")
        guide = c3_3["coverage_guide"]
        self.assertTrue(
            any(
                "Se a célula selecionada indicar mais de 40" in note
                and "restaure o preset salvo" in note
                for note in guide["notes"]
            )
        )
        self.assertEqual(
            {row["target_mm"]: row["tested_1_2"] for row in guide["rows"]},
            EXPECTED_RECOMMENDATIONS,
        )
        field_alternatives = {
            "1:4": "alternative_1_4",
            "1:1.3": "alternative_1_1_3",
            "1:1": "alternative_1_1",
        }
        for magnification, key in field_alternatives.items():
            self.assertEqual(
                {row["target_mm"]: row[key] for row in guide["rows"]},
                EXPECTED_ALTERNATIVES[magnification],
            )


os.environ.setdefault("QT_QPA_PLATFORM", "offscreen")
try:
    from PyQt6.QtWidgets import QApplication
    from ui.main_window import MainWindow
except ModuleNotFoundError as exc:
    if exc.name != "PyQt6" and not (exc.name or "").startswith("PyQt6."):
        raise
    QApplication = None
    MainWindow = None


@unittest.skipUnless(QApplication is not None, "PyQt6 is not installed")
class DesktopRecommendationTests(unittest.TestCase):
    @classmethod
    def setUpClass(cls):
        cls.app = QApplication.instance() or QApplication([])

    def test_depth_selection_updates_tested_and_alternative_counts(self):
        window = MainWindow()
        try:
            for size_mm, tested_shots in EXPECTED_RECOMMENDATIONS.items():
                with self.subTest(size_mm=size_mm):
                    index = window.character_size_combo.findData(size_mm)
                    window.character_size_combo.setCurrentIndex(index)
                    self.assertEqual(window.tested_shots_label.text(), str(tested_shots))
                    rendered = window.alternatives_label.text()
                    for magnification, recommendations in EXPECTED_ALTERNATIVES.items():
                        self.assertIn(f"<b>{magnification}</b>", rendered)
                        self.assertIn(f">{recommendations[size_mm]} shots</td>", rendered)
        finally:
            window.close()


if __name__ == "__main__":
    unittest.main()
