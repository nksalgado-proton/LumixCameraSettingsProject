# XLumix G9 Settings — Product Vision

## The Problem

Configuring a Panasonic Lumix G9 (Mark I or Mark II) for a specific shooting scenario requires navigating dozens of nested camera menus. Switching between scenarios (e.g. wildlife → macro → portrait) means either memorizing settings, using the limited C1/C2/C3 slots, or spending minutes re-configuring in the field. There is no way to see all settings at a glance, validate them for consistency, or prepare configurations offline.

## The Solution

A desktop application that replaces the camera menu as the primary way to manage settings. The app produces CamSet DAT files that load onto the camera via SD card, instantly applying a full configuration.

---

## Feature Layers

### Layer 1 — Settings Browser (GUI)

A clear graphical interface for navigating and editing all camera settings. Organized to mirror the camera's menu structure but with major usability improvements:

- All settings visible at once in a searchable, filterable tree
- Current values shown inline — no drilling into submenus
- Side-by-side comparison of two configurations
- Far faster than the camera's menu system for reviewing what is set

### Layer 2 — Shooting Scenarios

Named, reusable setting profiles tied to real-world shooting situations.

**Creation methods:**
- **Manual** — user sets each value through the GUI
- **Descriptive** — user describes the scenario in natural language (e.g. "Birds in Flight with Leica 100-400mm, overcast conditions") and the app generates recommended settings
- **Guided Q&A** — the app asks targeted questions ("Subject movement speed?", "Tripod or handheld?", "Flash?") to build the configuration step by step

**Examples:**
- Birds in Flight (BIF)
- Insects Macro with 60mm + Raynox DCR-250
- Street Photography — silent mode
- Portraits without flash — outdoor
- Focus Bracketing — 90mm at 1:1

### Layer 3 — Scenario Library

A persistent, editable database of all shooting scenarios.

- Tag and categorize scenarios (wildlife, macro, portrait, landscape, etc.)
- Version history — track changes to a scenario over time
- Scenario inheritance — "BIF Overcast" inherits from "BIF" but overrides WB and ISO range
- Import/export as JSON or YAML for sharing between users or backing up

### Layer 4 — Scenario-to-Slot Mapping

Assign scenarios to the camera's configuration slots:

- **C1, C2, C3** (and sub-modes C3-1, C3-2, etc. on G9 II)
- **Fn button assignments** — map physical and touch Fn buttons
- **Quick Menu layout** — choose which settings appear in the quick menu
- **My Menu** — build custom menu pages

Visual drag-and-drop interface for slot assignment.

### Layer 5 — Equipment Profiles

Define camera + lens(es) + flash combinations that represent real-world kits:

| Profile Name | Camera | Lenses | Flash |
|---|---|---|---|
| Wildlife | G9 II | Leica 100-400mm | — |
| Macro Field | G9 II | OM 90mm f/3.5, OM 60mm f/2.8 | Godox TT350 |
| General Walk-around | G9 II | Leica 12-60mm | — |
| Legacy Wildlife | G9 (MkI) | Leica 100-400mm | — |

Equipment profiles:
- Constrain which scenarios are relevant (no flash scenarios for a flashless kit)
- Enable lens-aware defaults (IS mode depends on lens OIS, min shutter relates to focal length)
- Group the scenarios you want available for a specific outing

### Layer 6 — Camera Configuration Export

The final output: select an equipment profile → pick scenarios → assign to slots → generate a CamSet DAT file ready to copy to SD card.

- Preview exactly what will be written before exporting
- Support both G9 Mark I and G9 Mark II DAT formats (where possible)
- Re-export after firmware updates (automated byte-map re-generation)

### Layer 7 — Consistency Checklist

Two-part conflict detection to prevent settings that look correct individually but fail together in practice.

**Automated checks (software settings):**
- Electronic shutter + flash enabled → flash won't fire
- Focus bracketing + MF mode without AF override → bracketing won't step
- Silent mode + flash enabled → contradiction
- Pre-burst + mechanical shutter → incompatible
- AF-C mode but Fn button mapped to MF toggle → user intent ambiguity
- IS mode mismatch with lens OIS capability

**Hardware checklist (physical switches the app cannot detect):**
- "Is the 90mm focus clutch ring pushed forward for AF?" — required for focus bracketing
- "Is the lens OIS switch ON?" — if camera IS is set to use lens OIS
- "Flash power switch position?" — for manual flash scenarios
- "Lens aperture ring locked to A?" — for Leica lenses with aperture rings

The checklist is presented before export and can be customized per scenario.

### Layer 8 — XmacroSettings Integration

Integrate with the existing [XmacroSettings](file:///D:/Projetos_Nelson/XmacroSettings) focus bracketing calculator to automatically compute:

- Focus bracket step size based on lens, magnification, and aperture
- Number of frames needed for full depth coverage
- Estimated total shooting time (using camera-specific fps data)
- Diffraction warnings when aperture exceeds sensor-format limits

For macro shooting scenarios, the user selects a lens + accessory (e.g. 90mm + MC-20 teleconverter, or 60mm + Raynox DCR-250), sets the target magnification range, and the integration fills in the bracketing parameters automatically.

**Shared equipment data:** Both tools use the same camera, lens, and accessory definitions (G9, G9 II, 60mm, 90mm, MC-20, Raynox DCR-250). The data model should be unified so equipment is defined once.

---

## Additional Capabilities

### Scenario Diffing
Compare two scenarios side-by-side with human-readable setting names: "What changed between my BIF v1 and BIF v2?" — shown as named settings, not byte offsets.

### Import from Camera
Read a CamSet DAT file and decompose it into its constituent settings, creating a new scenario from the camera's current state. Essential for bootstrapping — capture your current setup as a starting point.

### Multi-Camera Support
Maintain a scenario once; export it for each camera's DAT format. Where settings differ between G9 MkI and MkII, flag the differences.

### Session Planner
"Tomorrow I'm going to a park with the 100-400 and the 60mm macro" → the app suggests which scenarios to load and how to map them to C1/C2/C3 based on the equipment profile.

### Community Sharing
Export/import scenarios as portable files so photographers can share configurations: "Here's my BIF setup for the G9 II + 100-400mm."

---

## Development Phases

| Phase | Focus | Deliverable |
|---|---|---|
| **1** | Automated DAT reverse-engineering | Byte map of settings for G9 II FW 2.7 (and G9 MkI FW 2.5 where possible). Automated pipeline that can be re-run on firmware updates. |
| **2** | Settings data model + GUI browser | Python data model for all mapped settings. GUI to browse/edit settings and validate the byte map is correct. |
| **3** | Scenario engine + library | Create, store, edit, and diff named shooting scenarios. Persistent database. |
| **4** | Equipment profiles + slot mapping | Define camera/lens/flash kits. Map scenarios to C1/C2/C3, Fn buttons, quick menu, my menu. |
| **5** | Consistency checklist | Automated conflict detection + hardware checklist per scenario. |
| **6** | AI-assisted scenario creation | Natural language and guided Q&A scenario generation. |
| **7** | XmacroSettings integration | Unified equipment data, automatic bracketing parameter computation for macro scenarios. |
| **8** | Import, sharing, multi-camera, session planner | Community features, DAT import, cross-camera sync. |

---

## Technical Notes

- **Python ≥ 3.12**, using `httpx` for WiFi API, `rich` for CLI output
- GUI framework TBD (candidates: Qt/PySide6, or web-based with FastAPI + frontend)
- CamSet DAT files are binary; byte layout varies by firmware version
- WiFi API at `192.168.54.1/cam.cgi` is used for Phase 1 reverse-engineering (toggle settings remotely, then diff the resulting DAT files)
- Equipment data model should be shared with XmacroSettings (currently in `gear_data.py` and `equipament.json`)
