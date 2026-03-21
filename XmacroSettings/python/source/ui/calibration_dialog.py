from PyQt6.QtWidgets import (
    QDialog, QVBoxLayout, QHBoxLayout, QFormLayout, QGroupBox,
    QComboBox, QDoubleSpinBox, QSpinBox, QLabel, QTableWidget,
    QTableWidgetItem, QPushButton, QHeaderView, QApplication,
)
from PyQt6.QtCore import Qt
from PyQt6.QtGui import QFont, QColor

from core.gear_data import CAMERAS, LENSES, SENSOR_FORMATS, STEP_TABLES
from core.calculations import effective_magnification, effective_aperture, depth_of_field

# Steps to highlight as "recommended to test"
RECOMMENDED_STEPS = {2, 3, 5}

PROTOCOL_TEXT = (
    "GOAL\n"
    "Find how far (in mm) the Lumix moves the focus plane for each step setting.\n"
    "Two things must be measured: (A) the depth of your test subject, (B) how many\n"
    "shots the camera takes to cover it.\n\n"

    "RECOMMENDED SETUP  (keep identical across all step tests and both bodies)\n"
    "  Lens: 60mm f/2.8 · Aperture: f/5.6 · Magnification dial: 1.00× · No accessories\n"
    "  → Computed DoF ≈ 0.336 mm  (shown live above when these values are entered)\n\n"

    "MEASUREMENT A — subject depth  (do once; reuse for every step)\n"
    "  1. Place any small subject on the rail stage (a matchbox, coin, etc.).\n"
    "  2. Rack the rail until the NEAR edge of the subject is sharp.  Zero the rail.\n"
    "  3. Rack forward until the FAR edge is sharp.  Read the rail displacement.\n"
    "     That number, in mm, is your Known Depth.\n"
    "  4. Rack back to zero (near edge sharp again).\n\n"

    "MEASUREMENT B — how many steps to cross the subject  (repeat per step setting)\n"
    "  5. Set the in-camera bracket to a generous shot count (e.g., 50) so the\n"
    "     camera won't stop before reaching the far edge.  Set the step to test.\n"
    "  6. Start focused at the near edge (rail at zero).  Trigger the bracket.\n"
    "  7. Review the resulting images.  Find the frame where the FAR edge first\n"
    "     becomes sharp.  Its position in the sequence is N.\n"
    "     Example: if that frame is the 18th image, N = 18.\n"
    "  8. Enter Known Depth and N in the matching row of the table above.\n\n"
    "  NOTE: N is NOT the shot count you pre-set.  It is found from the images.\n\n"

    "WHAT THE APP CALCULATES\n"
    "  step_mm = Known Depth / (N − 1)   [frame 1 = near, frame N = far → N−1 steps]\n"
    "  DoF fraction = step_mm / DoF\n"
    "  Green = within 5% of current table · Orange = 5–15% off · Red = >15% off\n\n"

    "Larger subjects make focus transitions easier to identify and increase shot count,\n"
    "both of which improve accuracy.  Test steps 2, 3, 5.  Repeat on both bodies."
)


class CalibrationDialog(QDialog):
    def __init__(self, parent=None):
        super().__init__(parent)
        self.setWindowTitle("Lumix Focus-Step Calibration")
        self.setMinimumWidth(680)
        self._current_dof: float = 0.0

        layout = QVBoxLayout(self)
        layout.setSpacing(10)
        layout.setContentsMargins(14, 14, 14, 14)

        layout.addWidget(self._build_setup_group())
        layout.addWidget(self._build_table_group())
        layout.addWidget(self._build_protocol_group())

        btn_row = QHBoxLayout()
        self.copy_btn = QPushButton("Copy Results to Clipboard")
        self.copy_btn.clicked.connect(self._copy_to_clipboard)
        close_btn = QPushButton("Close")
        close_btn.clicked.connect(self.accept)
        btn_row.addWidget(self.copy_btn)
        btn_row.addStretch()
        btn_row.addWidget(close_btn)
        layout.addLayout(btn_row)

        self._connect_signals()
        self._update_dof()

    # ------------------------------------------------------------------
    # UI builders
    # ------------------------------------------------------------------

    def _build_setup_group(self) -> QGroupBox:
        box = QGroupBox("Optical Setup")
        form = QFormLayout(box)

        self.cam_combo = QComboBox()
        self.cam_combo.addItems(CAMERAS.keys())
        form.addRow("Camera:", self.cam_combo)

        self.lens_combo = QComboBox()
        self.lens_combo.addItems(LENSES.keys())
        # Default to 60mm for recommended setup
        idx = self.lens_combo.findText("OM System 60mm f/2.8 Macro")
        if idx >= 0:
            self.lens_combo.setCurrentIndex(idx)
        form.addRow("Lens:", self.lens_combo)

        self.f_spin = QDoubleSpinBox()
        self.f_spin.setRange(1.0, 22.0)
        self.f_spin.setSingleStep(1.0)
        self.f_spin.setDecimals(1)
        self.f_spin.setValue(5.6)
        self.f_spin.setPrefix("f/")
        form.addRow("Aperture:", self.f_spin)

        self.mag_spin = QDoubleSpinBox()
        self.mag_spin.setRange(0.1, 5.0)
        self.mag_spin.setSingleStep(0.1)
        self.mag_spin.setDecimals(2)
        self.mag_spin.setValue(1.0)
        form.addRow("Magnification (dial):", self.mag_spin)

        self.dof_label = QLabel()
        bold = QFont()
        bold.setBold(True)
        self.dof_label.setFont(bold)
        form.addRow("Computed DoF:", self.dof_label)

        return box

    def _build_table_group(self) -> QGroupBox:
        box = QGroupBox(
            "Measurements  —  highlighted rows (2, 3, 5) are recommended test points"
        )
        layout = QVBoxLayout(box)

        self.table = QTableWidget()
        self.table.setRowCount(10)
        self.table.setColumnCount(5)
        self.table.setHorizontalHeaderLabels([
            "Step #",
            "Known Depth (mm)",
            "Shot Count (N)",
            "Measured Fraction",
            "Current Table",
        ])
        hdr = self.table.horizontalHeader()
        hdr.setSectionResizeMode(0, QHeaderView.ResizeMode.ResizeToContents)
        hdr.setSectionResizeMode(1, QHeaderView.ResizeMode.Stretch)
        hdr.setSectionResizeMode(2, QHeaderView.ResizeMode.Stretch)
        hdr.setSectionResizeMode(3, QHeaderView.ResizeMode.Stretch)
        hdr.setSectionResizeMode(4, QHeaderView.ResizeMode.Stretch)
        self.table.verticalHeader().setVisible(False)

        self._depth_spins: dict[int, QDoubleSpinBox] = {}
        self._shot_spins:  dict[int, QSpinBox]        = {}

        highlight_bg = QColor("#fff9e6")

        for row, step in enumerate(range(1, 11)):
            # Column 0: step number
            num_item = QTableWidgetItem(str(step))
            num_item.setFlags(Qt.ItemFlag.ItemIsEnabled)
            num_item.setTextAlignment(Qt.AlignmentFlag.AlignCenter)
            if step in RECOMMENDED_STEPS:
                f = QFont()
                f.setBold(True)
                num_item.setFont(f)
            self.table.setItem(row, 0, num_item)

            # Column 1: known depth spin
            depth_spin = QDoubleSpinBox()
            depth_spin.setRange(0.0, 200.0)
            depth_spin.setSingleStep(0.5)
            depth_spin.setDecimals(2)
            depth_spin.setValue(0.0)
            depth_spin.setSuffix(" mm")
            depth_spin.setSpecialValueText("—")
            self.table.setCellWidget(row, 1, depth_spin)
            self._depth_spins[step] = depth_spin

            # Column 2: shot count spin
            shot_spin = QSpinBox()
            shot_spin.setRange(0, 9999)
            shot_spin.setValue(0)
            shot_spin.setSpecialValueText("—")
            self.table.setCellWidget(row, 2, shot_spin)
            self._shot_spins[step] = shot_spin

            # Column 3: measured fraction (computed, read-only)
            frac_item = QTableWidgetItem("—")
            frac_item.setFlags(Qt.ItemFlag.ItemIsEnabled)
            frac_item.setTextAlignment(Qt.AlignmentFlag.AlignCenter)
            self.table.setItem(row, 3, frac_item)

            # Column 4: current table value (populated later by _refresh_current_table_column)
            cur_item = QTableWidgetItem("—")
            cur_item.setFlags(Qt.ItemFlag.ItemIsEnabled)
            cur_item.setTextAlignment(Qt.AlignmentFlag.AlignCenter)
            self.table.setItem(row, 4, cur_item)

            # Highlight recommended rows
            if step in RECOMMENDED_STEPS:
                for col in (0, 3, 4):
                    self.table.item(row, col).setBackground(highlight_bg)
                depth_spin.setStyleSheet("background-color: #fff9e6;")
                shot_spin.setStyleSheet("background-color: #fff9e6;")

            depth_spin.valueChanged.connect(self._recompute_fractions)
            shot_spin.valueChanged.connect(self._recompute_fractions)

        layout.addWidget(self.table)
        return box

    def _build_protocol_group(self) -> QGroupBox:
        box = QGroupBox("Field Protocol")
        layout = QVBoxLayout(box)
        lbl = QLabel(PROTOCOL_TEXT)
        lbl.setWordWrap(True)
        lbl.setStyleSheet("font-family: monospace; font-size: 9pt;")
        layout.addWidget(lbl)
        return box

    # ------------------------------------------------------------------
    # Signals
    # ------------------------------------------------------------------

    def _connect_signals(self):
        self.cam_combo.currentTextChanged.connect(self._update_dof)
        self.lens_combo.currentTextChanged.connect(self._update_dof)
        self.f_spin.valueChanged.connect(self._update_dof)
        self.mag_spin.valueChanged.connect(self._update_dof)

    # ------------------------------------------------------------------
    # Logic
    # ------------------------------------------------------------------

    def _current_step_table(self) -> dict:
        camera_name = self.cam_combo.currentText()
        lens_name   = self.lens_combo.currentText()
        return (
            STEP_TABLES.get((camera_name, lens_name, "None"))
            or STEP_TABLES.get((camera_name, lens_name, "None"), {})
        )

    def _update_dof(self, *_):
        camera_name = self.cam_combo.currentText()
        lens_name   = self.lens_combo.currentText()
        camera = CAMERAS.get(camera_name, {})
        lens   = LENSES.get(lens_name, {})
        sensor = SENSOR_FORMATS[camera.get("sensor_format", "MFT")]
        coc    = sensor["coc_mm"]

        eff_f   = effective_aperture(self.f_spin.value(), {"type": None})
        eff_mag = effective_magnification(self.mag_spin.value(), lens, {"type": None})
        dof     = depth_of_field(eff_f, coc, eff_mag)

        self._current_dof = dof
        self.dof_label.setText(f"{dof:.4f} mm")
        self._refresh_current_table_column()
        self._recompute_fractions()

    def _refresh_current_table_column(self):
        """Update column 4 (current table) to match the selected camera.
        Shows the stored physical step in mm AND the equivalent DoF fraction
        at the current optical setup.
        """
        step_table = self._current_step_table()
        dof = self._current_dof
        for step in range(1, 11):
            row = step - 1
            step_mm = step_table.get(step)
            if step_mm is not None and dof > 0:
                frac = step_mm / dof
                text = f"{frac:.3f}"
            else:
                text = "—"
            self.table.item(row, 4).setText(text)

    def _recompute_fractions(self, *_):
        dof = self._current_dof
        if dof <= 0:
            return

        step_table = self._current_step_table()
        for step in range(1, 11):
            row = step - 1
            depth   = self._depth_spins[step].value()
            n_shots = self._shot_spins[step].value()
            frac_item = self.table.item(row, 3)

            if depth > 0 and n_shots > 1:
                measured_step_mm = depth / (n_shots - 1)
                fraction         = measured_step_mm / dof
                frac_item.setText(f"{fraction:.3f}")

                table_step_mm = step_table.get(step)
                if table_step_mm and table_step_mm > 0:
                    table_frac = table_step_mm / dof
                    rel_diff   = abs(fraction - table_frac) / table_frac
                else:
                    rel_diff = 1.0

                if rel_diff > 0.15:
                    frac_item.setForeground(QColor("#c0392b"))   # red
                elif rel_diff > 0.05:
                    frac_item.setForeground(QColor("#e67e22"))   # orange
                else:
                    frac_item.setForeground(QColor("#27ae60"))   # green
            else:
                frac_item.setText("—")
                frac_item.setForeground(QColor("#888888"))

    def _copy_to_clipboard(self):
        """Copy a Python-ready snippet of the measured fractions to clipboard."""
        dof        = self._current_dof
        step_table = self._current_step_table()
        cam_name   = self.cam_combo.currentText()
        lens_name  = self.lens_combo.currentText()
        lines      = [f'    ("{cam_name}", "{lens_name}"): {{']
        for step in range(1, 11):
            depth   = self._depth_spins[step].value()
            n_shots = self._shot_spins[step].value()
            if dof > 0 and depth > 0 and n_shots > 1:
                step_mm = depth / (n_shots - 1)
                note    = f"  # N={n_shots}"
            else:
                step_mm = step_table.get(step, 0.0)
                note    = "  # unchanged"
            lines.append(f"        {step:>2}: {step_mm:.4f},{note}")
        lines.append("}")
        QApplication.clipboard().setText("\n".join(lines))
        self.copy_btn.setText("Copied!")
        from PyQt6.QtCore import QTimer
        QTimer.singleShot(2000, lambda: self.copy_btn.setText("Copy Results to Clipboard"))