from PyQt6.QtWidgets import (
    QMainWindow, QWidget, QVBoxLayout, QHBoxLayout, QFormLayout,
    QGroupBox, QComboBox, QDoubleSpinBox, QLabel, QRadioButton,
    QButtonGroup, QSizePolicy,
)
from PyQt6.QtCore import Qt
from PyQt6.QtGui import QFont, QAction

from core.gear_data import CAMERAS, LENSES, ACCESSORIES, FLASH_UNITS
from core.calculations import calculate_all

# Shooting conditions: name → (overlap, description)
CONDITIONS = {
    "Studio":  (0.55, "55% overlap — max quality, controlled environment"),
    "Field":   (0.40, "40% overlap — balanced, light breeze"),
    "Windy":   (0.20, "20% overlap — faster sequence, less overlap"),
}


class MainWindow(QMainWindow):
    def __init__(self):
        super().__init__()
        self.setWindowTitle("Macro Focus Bracketing Calculator")
        self.setMinimumWidth(620)

        self._build_menu()

        central = QWidget()
        self.setCentralWidget(central)
        root = QVBoxLayout(central)
        root.setSpacing(10)
        root.setContentsMargins(14, 14, 14, 14)

        root.addWidget(self._build_gear_group())
        root.addWidget(self._build_params_group())
        root.addWidget(self._build_shooting_mode_group())
        root.addWidget(self._build_results_group())
        root.addStretch()

        self._connect_signals()
        self._on_lens_changed()   # initialise accessory list
        self._recalculate()       # initial results

    # ------------------------------------------------------------------
    # UI builders
    # ------------------------------------------------------------------

    def _build_menu(self):
        tools_menu = self.menuBar().addMenu("Tools")
        calib_action = QAction("Calibrate Lumix Steps…", self)
        calib_action.triggered.connect(self._open_calibration)
        tools_menu.addAction(calib_action)

    def _open_calibration(self):
        from ui.calibration_dialog import CalibrationDialog
        dlg = CalibrationDialog(self)
        dlg.exec()

    def _build_gear_group(self) -> QGroupBox:
        box = QGroupBox("Gear")
        form = QFormLayout(box)

        self.cam_combo = QComboBox()
        self.cam_combo.addItems(CAMERAS.keys())
        form.addRow("Camera:", self.cam_combo)

        self.lens_combo = QComboBox()
        self.lens_combo.addItems(LENSES.keys())
        form.addRow("Lens:", self.lens_combo)

        self.acc_combo = QComboBox()
        form.addRow("Accessory:", self.acc_combo)

        return box

    def _build_params_group(self) -> QGroupBox:
        box = QGroupBox("Parameters")
        form = QFormLayout(box)

        # Magnification
        mag_row = QHBoxLayout()
        self.mag_spin = QDoubleSpinBox()
        self.mag_spin.setRange(0.1, 5.0)
        self.mag_spin.setSingleStep(0.1)
        self.mag_spin.setDecimals(2)
        self.mag_spin.setValue(1.0)
        self.mag_eff_label = QLabel("→ effective: 1.00×")
        self.mag_eff_label.setStyleSheet("color: #555;")
        mag_row.addWidget(self.mag_spin)
        mag_row.addWidget(self.mag_eff_label)
        mag_row.addStretch()
        form.addRow("Magnification (dial):", mag_row)

        # Aperture
        f_row = QHBoxLayout()
        self.f_spin = QDoubleSpinBox()
        self.f_spin.setRange(1.0, 22.0)
        self.f_spin.setSingleStep(1.0)
        self.f_spin.setDecimals(1)
        self.f_spin.setValue(5.6)
        self.f_spin.setPrefix("f/")
        self.f_eff_label = QLabel("→ effective: f/5.6")
        self.f_eff_label.setStyleSheet("color: #555;")
        f_row.addWidget(self.f_spin)
        f_row.addWidget(self.f_eff_label)
        f_row.addStretch()
        form.addRow("Aperture:", f_row)

        # Subject depth
        self.depth_spin = QDoubleSpinBox()
        self.depth_spin.setRange(0.1, 200.0)
        self.depth_spin.setSingleStep(0.5)
        self.depth_spin.setDecimals(1)
        self.depth_spin.setValue(5.0)
        self.depth_spin.setSuffix(" mm")
        form.addRow("Subject Depth:", self.depth_spin)

        # Condition
        cond_row = QHBoxLayout()
        self.cond_btn_group = QButtonGroup()
        for i, (name, (_, desc)) in enumerate(CONDITIONS.items()):
            rb = QRadioButton(name)
            rb.setToolTip(desc)
            self.cond_btn_group.addButton(rb, i)
            cond_row.addWidget(rb)
            if name == "Field":
                rb.setChecked(True)
        cond_row.addStretch()
        form.addRow("Condition:", cond_row)

        return box

    def _build_shooting_mode_group(self) -> QGroupBox:
        box = QGroupBox("Shooting Mode")
        form = QFormLayout(box)

        # Ambient / Flash radio buttons
        mode_row = QHBoxLayout()
        self.mode_btn_group = QButtonGroup()
        self.rb_ambient = QRadioButton("Ambient")
        self.rb_flash   = QRadioButton("Flash")
        self.rb_ambient.setChecked(True)
        self.mode_btn_group.addButton(self.rb_ambient, 0)
        self.mode_btn_group.addButton(self.rb_flash, 1)
        mode_row.addWidget(self.rb_ambient)
        mode_row.addWidget(self.rb_flash)
        mode_row.addStretch()
        form.addRow("Mode:", mode_row)

        # Flash unit
        self.flash_unit_label = QLabel("Flash unit:")
        self.flash_combo = QComboBox()
        flash_names = [k for k in FLASH_UNITS if k != "Ambient / No Flash"]
        self.flash_combo.addItems(flash_names)
        form.addRow(self.flash_unit_label, self.flash_combo)

        # Recycle time
        self.recycle_label = QLabel("Recycle time:")
        self.recycle_spin = QDoubleSpinBox()
        self.recycle_spin.setRange(0.1, 10.0)
        self.recycle_spin.setSingleStep(0.1)
        self.recycle_spin.setDecimals(1)
        self.recycle_spin.setValue(1.5)
        self.recycle_spin.setSuffix(" s")
        form.addRow(self.recycle_label, self.recycle_spin)

        self._set_flash_widgets_visible(False)
        return box

    def _build_results_group(self) -> QGroupBox:
        box = QGroupBox("Results")
        layout = QVBoxLayout(box)

        # Optical summary
        summary_row = QHBoxLayout()
        self.dof_label  = QLabel()
        self.step_label = QLabel()
        bold = QFont()
        bold.setBold(True)
        self.dof_label.setFont(bold)
        self.step_label.setFont(bold)
        summary_row.addWidget(self.dof_label)
        summary_row.addSpacing(20)
        summary_row.addWidget(self.step_label)
        summary_row.addStretch()
        layout.addLayout(summary_row)

        # Diffraction warning
        self.diff_warn_label = QLabel()
        self.diff_warn_label.setStyleSheet("color: #c0392b; font-weight: bold;")
        self.diff_warn_label.hide()
        layout.addWidget(self.diff_warn_label)

        # Two result panels side by side
        panels = QHBoxLayout()

        # In-camera panel
        cam_panel = QGroupBox("In-Camera (Lumix)")
        cam_layout = QVBoxLayout(cam_panel)
        cam_form   = QFormLayout()
        self.lumix_step_label  = QLabel()
        self.lumix_shots_label = QLabel()
        self.lumix_time_label  = QLabel()
        cam_form.addRow("Step Setting:",    self.lumix_step_label)
        cam_form.addRow("Number of Shots:", self.lumix_shots_label)
        cam_form.addRow("Sequence Time:",   self.lumix_time_label)
        cam_layout.addLayout(cam_form)
        self.gap_warn_label = QLabel()
        self.gap_warn_label.setStyleSheet("color: #c0392b; font-weight: bold;")
        self.gap_warn_label.setWordWrap(True)
        self.gap_warn_label.hide()
        cam_layout.addWidget(self.gap_warn_label)

        # Rail panel
        rail_panel = QGroupBox("Rail (Fotopro FT100 Pro)")
        rail_form  = QFormLayout(rail_panel)
        self.rail_step_label  = QLabel()
        self.rail_shots_label = QLabel()
        rail_form.addRow("Step Size:",      self.rail_step_label)
        rail_form.addRow("Number of Shots:", self.rail_shots_label)

        panels.addWidget(cam_panel)
        panels.addWidget(rail_panel)
        layout.addLayout(panels)

        return box

    # ------------------------------------------------------------------
    # Signal wiring
    # ------------------------------------------------------------------

    def _connect_signals(self):
        self.cam_combo.currentTextChanged.connect(self._recalculate)
        self.lens_combo.currentTextChanged.connect(self._on_lens_changed)
        self.acc_combo.currentTextChanged.connect(self._recalculate)
        self.mag_spin.valueChanged.connect(self._recalculate)
        self.f_spin.valueChanged.connect(self._recalculate)
        self.depth_spin.valueChanged.connect(self._recalculate)
        self.cond_btn_group.buttonToggled.connect(self._recalculate)
        self.mode_btn_group.buttonToggled.connect(self._on_mode_changed)
        self.flash_combo.currentTextChanged.connect(self._on_flash_unit_changed)
        self.recycle_spin.valueChanged.connect(self._recalculate)

    # ------------------------------------------------------------------
    # Slots
    # ------------------------------------------------------------------

    def _on_lens_changed(self):
        lens_name = self.lens_combo.currentText()
        lens = LENSES.get(lens_name, {})
        compatible = lens.get("compatible_accessories", list(ACCESSORIES.keys()))

        self.acc_combo.blockSignals(True)
        self.acc_combo.clear()
        self.acc_combo.addItems(compatible)
        self.acc_combo.blockSignals(False)

        # Clamp magnification to lens native range
        mag_min, mag_max = lens.get("mag_range", (0.1, 5.0))
        self.mag_spin.setRange(mag_min, mag_max)

        # Clamp aperture minimum to lens max aperture
        self.f_spin.setMinimum(lens.get("max_aperture", 1.0))

        self._recalculate()

    def _on_mode_changed(self):
        is_flash = self.rb_flash.isChecked()
        self._set_flash_widgets_visible(is_flash)
        self._recalculate()

    def _on_flash_unit_changed(self):
        unit_name = self.flash_combo.currentText()
        unit = FLASH_UNITS.get(unit_name, {})
        recycle = unit.get("recycle_s", 1.5) or 1.5
        self.recycle_spin.blockSignals(True)
        self.recycle_spin.setValue(recycle)
        self.recycle_spin.blockSignals(False)
        self._recalculate()

    def _recalculate(self, *_):
        camera_name = self.cam_combo.currentText()
        lens_name   = self.lens_combo.currentText()
        acc_name    = self.acc_combo.currentText()

        if not all([camera_name, lens_name, acc_name]):
            return

        camera    = CAMERAS[camera_name]
        lens      = LENSES[lens_name]
        accessory = ACCESSORIES.get(acc_name, ACCESSORIES["None"])

        dial_mag  = self.mag_spin.value()
        dial_f    = self.f_spin.value()
        depth_mm  = self.depth_spin.value()

        cond_names = list(CONDITIONS.keys())
        cid = self.cond_btn_group.checkedId()
        overlap, _ = CONDITIONS[cond_names[max(cid, 0)]]

        flash_recycle = None
        if self.rb_flash.isChecked():
            flash_recycle = self.recycle_spin.value()

        r = calculate_all(
            camera, lens, accessory,
            camera_name, lens_name, acc_name,
            dial_mag, dial_f, depth_mm,
            overlap, flash_recycle,
        )

        # Effective labels
        self.mag_eff_label.setText(f"→ effective: {r['effective_magnification']:.2f}×")
        self.f_eff_label.setText(f"→ effective: f/{r['effective_aperture']:.1f}")

        # Summary
        self.dof_label.setText(f"DoF / frame:  {r['dof_mm']:.4f} mm")
        self.step_label.setText(f"Focus step:  {r['step_mm']:.4f} mm")

        # Diffraction warning
        if r["diffraction_warning"]:
            self.diff_warn_label.setText(
                f"⚠  Diffraction significant above f/{r['diff_limit_f']:.0f} on this sensor"
            )
            self.diff_warn_label.show()
        else:
            self.diff_warn_label.hide()

        # In-camera panel
        lumix_step = r["lumix_step"]
        self.lumix_step_label.setText(str(lumix_step) if lumix_step is not None else "N/A")

        frac = r["lumix_step_fraction"]
        if frac is not None and frac > 1.0:
            self.gap_warn_label.setText(
                f"⚠  Step {lumix_step} skips {frac:.2f}× DoF — focus gaps. "
                "Use rail or reduce step."
            )
            self.gap_warn_label.show()
        else:
            self.gap_warn_label.hide()
        self.lumix_shots_label.setText(str(r["num_shots"]))
        time_s = r["sequence_time_s"]
        self.lumix_time_label.setText(
            f"{time_s:.1f} s" if time_s < 60 else f"{time_s/60:.1f} min"
        )

        # Rail panel
        self.rail_step_label.setText(f"{r['rail_step_mm']:.4f} mm")
        self.rail_shots_label.setText(str(r["num_shots"]))

    # ------------------------------------------------------------------
    # Helpers
    # ------------------------------------------------------------------

    def _set_flash_widgets_visible(self, visible: bool):
        for w in (self.flash_unit_label, self.flash_combo,
                  self.recycle_label, self.recycle_spin):
            w.setVisible(visible)
