# US National Parks 2026 — Camera Configuration Implementation Checklist

Created: 2026-08-08

This is the execution document for the decisions recorded in `guides/Camera-Mode-Redesign-US-Parks-2026.md`. Final approved targets are recorded in the camera configuration JSON files only after supporting physical tests. It is intended to be followed with both cameras physically available.

For the final G9 MkI camera-in-hand sequence and backup procedure, use `guides/G9MkI-Final-Implementation-and-Backup-Checklist.md`.

All camera menu names, setting names, option values and custom-mode titles in this checklist are written in English. This does not require changing the camera's `Language` setting if Nelson prefers to preserve its current value.

## Working Rules

- [ ] Do not use `Reset` on either camera.
- [ ] Photograph or record every relevant current screen before changing it.
- [ ] Treat `Change` as an approved modification and `Preserve / Verify` as protection against accidental drift.
- [ ] If the camera shows a different English label after a firmware update, record the displayed label and confirm its function; do not select a merely similar item by guesswork.
- [ ] Do not overwrite a Custom Mode until its source has been copied according to the migration order in this checklist.
- [ ] After programming, verify recalled settings after a power cycle, not only immediately after saving.
- [ ] Do not edit `data/camera-config-g9mki.json` or `data/camera-config-g9mkii.json` until physical implementation and validation are complete.

## Phase 1 — Preserve the Current State

### 1.1 Equipment and power

- [ ] Fully charge at least two batteries for each camera.
- [ ] Remove any battery grip; both cameras are configured for body batteries.
- [ ] Prepare the two matched 256 GB cards for each body and keep the two 64 GB V90 cards aside as reserves.
- [ ] Have the 12–35mm II, 35–100mm II, Olympus 60mm Macro and Leica 100–400mm II available.
- [ ] Have the DMW-TC14, DMW-TC20, Godox X3, both MF12 units, diffusers, AD100Pro, MIOPS Smart+, correct camera cable and tripod available for validation.

### 1.2 Record the current cameras

- [ ] G9 MkI: photograph the firmware screen, every Custom Mode summary, `Q.MENU`, `My Menu`, `Fn Button Set`, `Function Lever Setting`, card settings and Bluetooth settings.
- [x] G9 MkII: use `Setup > Setting > Save/Restore Camera Setting > Save` and save the current configuration as `G2PRE.DAT` on a card.
- [x] G9 MkII: copy `G2PRE.DAT` to the computer before changing settings; verified SHA-256 `39C5CE389E4875C138D0A6172DEE8CC72B433A4B0CCCC6AEC92A2EC094A8A287`.
- [ ] G9 MkII: photograph every current Custom Mode summary and its title.
- [ ] Both cameras: record the current firmware version for each attached lens.
- [ ] Confirm that the existing project JSON files remain unchanged at this stage.

### Phase 1 gate

- [ ] Current settings can be reconstructed from photographs, the G9 MkII card backup and the existing project records.

## Phase 2 — Firmware and Physical Maintenance

Perform updates before programming the new modes because firmware can change menu behavior or saved settings.

### 2.1 Firmware targets

- [ ] G9 MkI body: verify or update to firmware `2.7`.
- [ ] G9 MkII body: update from the recorded current version to firmware `2.7`.
- [ ] Lumix G X Vario 12–35mm f/2.8 II (`H-HSA12035`): verify or update to firmware `1.3`.
- [ ] Lumix G X Vario 35–100mm f/2.8 II (`H-HSA35100`): verify or update to firmware `1.3`.
- [ ] Leica DG Vario-Elmar 100–400mm II (`H-RSA100400`): verify or update to firmware `1.1`.
- [ ] M.Zuiko Digital ED 60mm f/2.8 Macro: verify or update to firmware `1.2` using the OM SYSTEM package for a Panasonic camera body.
- [ ] Use one firmware file at a time on a freshly formatted card and a fully charged battery.
- [ ] Update bodies first, then lenses.

Official firmware ledger, verified 2026-08-10:

| Lens | Model | Observed | Official target | Status |
|---|---|---:|---:|---|
| Lumix G X Vario 12–35mm f/2.8 II | H-HSA12035 | 1.0 | 1.3 | Update required |
| Lumix G X Vario 35–100mm f/2.8 II | H-HSA35100 | Not checked | 1.3 | Audit required |
| Leica DG Vario-Elmar 100–400mm f/4–6.3 II | H-RSA100400 | Not checked | 1.1 | Audit required |
| M.Zuiko Digital ED 60mm f/2.8 Macro | OM SYSTEM / Olympus | Not checked | 1.2 | Audit required |

Firmware authorities:

- Panasonic Micro Four Thirds lens index: `https://av.jpn.support.panasonic.com/support/global/cs/dsc/download/index5.html`
- H-HSA12035: `https://av.jpn.support.panasonic.com/support/global/cs/dsc/download/fts/dl/hsa12035.html`
- H-HSA35100: `https://av.jpn.support.panasonic.com/support/global/cs/dsc/download/fts/dl/hsa35100.html`
- H-RSA100400: `https://av.jpn.support.panasonic.com/support/global/cs/dsc/download/fts/dl/rsa100400.html`
- M.Zuiko 60mm update for Panasonic bodies: `https://support.jp.omsystem.com/en/support/imsg/digicamera/download/software/pana/m_ed6028macro.html`

The DMW-TC14 and DMW-TC20 teleconverters have no separate firmware item in the approved travel-gear ledger; inspect and clean their electronic contacts and test lens recognition after the 100–400mm II update.
- [ ] Do not perform a general camera reset after updating.

### 2.2 Post-update checks

- [ ] G9 MkII: with `Shutter AF = Off`, verify that the dedicated `AF-ON` button still activates autofocus.
- [ ] Both cameras: verify that every card slot and every physical control works.
- [ ] Run `Sensor Cleaning` on both bodies.
- [ ] Photograph an evenly illuminated plain surface at approximately f/16 and inspect for persistent sensor spots.
- [ ] Clean physically only if the test shows a real need.
- [ ] Calibrate `Level Gauge Adjust.` on a known-level surface.
- [ ] Use `Pixel Refresh` only if the image test reveals persistent stuck pixels.
- [ ] Clean and inspect lens and teleconverter contacts.

### Phase 2 gate

- [ ] All firmware versions match the targets, both cameras operate normally and no reset has been performed.

## Phase 3 — Global Settings on Both Cameras

Set these once per body. Do not save them repeatedly into individual Custom Modes unless the camera stores that particular item per mode.

### 3.1 Cards, files and authorship

- [ ] G9 MkI: change `Double Slot Function > Recording Method`: `Relay Rec` → `Backup Rec`.
- [ ] G9 MkII: use its firmware-2.7 Menu Atlas for the exact double-card menu name and set backup recording.
- [ ] Insert two Lexar 256 GB 1667x UHS-II V60 cards in the G9 MkI.
- [ ] Insert two Lexar 256 GB 2000x UHS-II V90 cards in the G9 MkII.
- [ ] Verify that a test RAW file is written to both cards on each camera.
- [ ] Set the three user-defined file-name characters to `G1_` on the G9 MkI and `G2_` on the G9 MkII.
- [ ] Verify example RAW naming as `PG1_0001.RW2` and `PG2_0001.RW2` or the camera's equivalent prefix construction.
- [ ] Preserve continuous file numbering during testing.
- [ ] Preserve standard folder-number-linked behavior; do not create date folders or a manual daily folder workflow.
- [ ] G9 MkI: do not attempt copyright programming; firmware 2.7 has no `Copyright Information`, `Artist`, or `Copyright Holder` menu items.
- [ ] G9 MkII: program copyright fields only after their exact firmware-2.7 paths are verified in its Menu Atlas.
- [ ] Defer the single pre-trip `File Number Reset` until all testing is complete.

### 3.2 Morning clock synchronization and daytime Bluetooth state

- [x] Pair the G9 MkI with Panasonic Image App.
- [x] Pair the G9 MkII with LUMIX Sync.
- [x] Set `Location Logging = OFF`, `Auto Transfer = OFF` and `Remote Wakeup = OFF` on both cameras.
- [x] Confirm experimentally that each camera restores its saved `Auto Clock Set = ON` preference after Bluetooth is re-enabled and the phone reconnects.
- [x] Leave `Bluetooth = OFF` on the G9 MkI and `Bluetooth Function = OFF` on the G9 MkII during the photographic day.
- [ ] Each morning and after every time-zone change, enable and connect each camera separately, verify its clock against the iPhone, and disable Bluetooth again.
- [ ] At each new photographic location, take one iPhone reference photograph with precise location enabled; use its GPS metadata to geotag the corresponding Lumix photographs during post-processing.
- [x] Keep `Wi-Fi` `OFF` except during an active transfer or remote-control session.
- [ ] Check the phone's location permission, Bluetooth permission and background-operation permission for both apps.
- [ ] Test both cameras during the same outing and verify correct time, time zone and embedded location data in actual files.
- [ ] If the phone cannot maintain both connections reliably, give the G9 MkII priority and reconnect the G9 MkI deliberately when needed.

### 3.3 Photo output and exposure behavior

- [ ] Set `Aspect Ratio` to `4:3`.
- [ ] Set `Picture Size` to `L`.
- [ ] Set `Quality` / `Picture Quality` to `RAW`.
- [ ] Set the general `Photo Style` baseline to `Standard`.
- [ ] Set `Metering Mode` to `Multi`.
- [ ] Set `Color Space` to `sRGB`.
- [ ] Set `i.Resolution` to `Off` where present.
- [ ] Set `Long Shtr NR` / `Long Exposure NR` to `Off`.
- [ ] Set `Extended ISO` to `Off`.
- [ ] Set `ISO Increments` to `1/3 EV`.
- [ ] Preserve `ISO Auto Upper Limit` as unrestricted / `Auto` unless a mode specifies fixed ISO.
- [ ] Change `Exposure Comp. Reset`: `Off` → `On` on both cameras.
- [ ] Preserve `Shutter Type = EFC` as the general still-photo baseline.
- [ ] Preserve `Silent Mode = Off`.
- [ ] Keep `Bracket`, `HDR` and `Shutter Delay` `Off` globally; enable them only in the relevant saved mode.

### 3.4 Focus and shutter operation

- [ ] Set `Shutter AF` to `Off`.
- [ ] G9 MkI: set `Half Press Release` to `Off`.
- [ ] G9 MkII: set `Half-Press Shutter` to `Off` after exact-name verification in its Menu Atlas.
- [ ] G9 MkI: set `Focus/Release Priority > AFS/AFF` to `FOCUS`.
- [ ] G9 MkII: set `Focus/Shutter Priority > AFC` to `BALANCE`.
- [ ] Set `AF+MF` to `On`.
- [ ] Set `MF Assist Display` to `PIP` and preserve focus-ring activation.
- [ ] Set `MF Guide` to `On`.
- [ ] G9 MkI: set `Peaking` to `On`, `Detect Level = LOW`, and the red `Display Color` icon.
- [ ] G9 MkII: set `Focus Peaking = On`; under `SET`, use sensitivity `-2`, `Display Color = Red`, `Display During AFS = On`, and all three `Display During MF` conditions `On`.
- [ ] Set `Quick AF` to `Off`.
- [ ] Set `Eye Sensor AF` to `Off`.
- [ ] Set `Focus Switching for Vert/Hor` to `Off`.
- [ ] G9 MkI: set `Loop Movement Focus Frame` to `Off`.
- [ ] Keep `AF Micro Adjustment` `Off`.
- [ ] G9 MkII: change `AF Assist Light`: `On` → `Off`.
- [ ] G9 MkII: keep global `Focus Limiter` `Off`.
- [ ] G9 MkII: preserve `AF Custom Setting (Photo) = Set 1`.
- [ ] G9 MkI: set `Lens Position Resume` to `On`.
- [ ] G9 MkII: set its corresponding lens-position-resume function after exact-name verification in its Menu Atlas.

### 3.5 Displays

- [ ] Set `Auto Review` to `Off`.
- [ ] Set `Photo Grid Line` to `9-SECTION`.
- [ ] Use `DISP.` to show the level gauge when desired; there is no persistent `Level Gauge = On` preference.
- [ ] Do not use `Focal Length Set` with the planned communicating electronic lenses.
- [ ] Set `Live View Boost` to `Off` globally.
- [ ] Set `Night Mode` to `Off`.
- [ ] Set `Expo.Meter` to `Off`.
- [ ] Set `Sheer Overlay` to `Off`.
- [ ] Set `Constant Preview` to `On` globally; the three G9 MkI macro modes override it to `Off`.
- [ ] Set `Zebra Pattern` to `Off` for still photography.
- [ ] G9 MkI: set `Highlight` to `On`.
- [ ] G9 MkII: set its corresponding highlight-warning function after exact-name verification in its Menu Atlas.
- [ ] G9 MkI: set `Histogram` to `Off`.
- [ ] G9 MkII: set `Histogram` to `On`.

### 3.6 Stabilization and lens behavior

- [ ] Preserve stabilization `On` as the global handheld baseline.
- [ ] Set `Focus Ring Control` to `NON-LINEAR` only when the attached lens exposes it.
- [ ] Set `Lens Fn Button Setting` to `AF-ON` only when the attached lens exposes it.
- [ ] Verify `Aperture Ring Increment = 1/3 EV` only with a supported clickless aperture-ring lens; none of the planned lenses requires it.
- [ ] G9 MkII: set `Lens Info. Confirmation` to `On`.

### 3.7 Power, sound and system

- [x] G9 MkI: set `Sleep Mode` to `10 MIN`.
- [x] G9 MkI: set `Auto LVF/Monitor Off` to `2 MIN`.
- [ ] G9 MkII: set `Sleep Mode` to `10 MIN.`.
- [ ] G9 MkII: set `Auto LVF/Monitor Off` to `2 MIN.`.
- [ ] Set `Monitor Frame Rate` to `60fps`.
- [ ] G9 MkI: keep `LVF Frame Rate` at `60fps`.
- [ ] G9 MkII: change `LVF Frame Rate`: `60fps` → `120fps`.
- [ ] Set monitor/LVF luminance to `AUTO`.
- [ ] Set `Eye Sensor > Sensitivity` to `LOW`.
- [ ] G9 MkI: do not attempt thermal-menu programming; firmware 2.7 has no `Thermal Management` or `Recording Max Temperature` item.
- [ ] G9 MkII: verify the exact thermal-menu path and value in its firmware-2.7 Menu Atlas.
- [ ] Turn all camera beeps and electronic-shutter sounds off.
- [ ] Set `USB Mode` to `Select on connection`.
- [ ] Set `USB Power Supply` to `On`.
- [ ] G9 MkI: do not attempt `Battery Use Priority`; it is documented only with the optional DMW-BGG9 attached, and no grip is used.
- [ ] G9 MkII: do not attempt battery-priority programming without verifying that firmware 2.7 exposes it with no grip.
- [ ] G9 MkI: do not attempt `System Frequency`; firmware 2.7 has no Setup item by that name.
- [ ] G9 MkII: verify video-system-frequency behavior in its Menu Atlas; do not infer a G9 MkI menu.
- [ ] Preserve the current `Language` setting.

### 3.8 Playback and deletion safety

- [ ] Preserve normal playback as the default playback mode.
- [ ] Set `Rotate Disp.` / `Rotate Display` to `On`.
- [ ] Set `Delete Confirmation` to `No First`.
- [ ] Confirm `Auto Review = Off`.
- [ ] Avoid routine in-camera culling; review and delete after daily verified import.

### Phase 3 gate

- [ ] Capture one RAW on each body and verify file name, copyright, time, location, correct card duplication and normal AF-ON behavior.

## Phase 4 — Camera-Specific Global Controls

### 4.1 G9 MkI

- [ ] Change `i.Dynamic`: `AUTO` → `OFF`.
- [ ] Change `Shading Comp.`: `ON` → `OFF`.
- [ ] Change `Diffraction Compensation`: `ON` → `OFF`.
- [ ] Set `Burst Shot 1 Setting` to `L`; approximately 2 fps is expected performance, not a separate menu value.
- [ ] Set `Burst Shot 2 Setting` to `M`; approximately 7 fps is expected performance, not a separate menu value. Do not use any unsupported SH value from project notes.
- [ ] Preserve `Fn1 = AF-ON`.
- [ ] Preserve `Fn2 = Q.MENU`.
- [ ] Preserve `Fn3 = LVF/Monitor Switch`.
- [ ] Preserve `Fn4 = AF-Point Scope`.
- [ ] Preserve `Fn5 = Preview`.
- [ ] Preserve the dedicated `ISO`, `WB`, `+/-` and Video Record button functions.
- [ ] Preserve `Joystick Setting = D.FOCUS Movement`.
- [ ] Change `Function Lever`: `Silent Mode` → `Stabilizer`, with `Mode 2 Setting = Off`.
- [ ] Mark or memorize Function Lever position 2 as `IS OFF`.

Function Lever rule: use position 1 for C1, C2, C3-1 and C3-2. Position 2 is appropriate for the supported C3-3 workflow.

### 4.2 G9 MkII

- [ ] Preserve `Fn1 = AF-ON: Near Shift`.
- [ ] Preserve `Fn2 = AF-Point Scope`.
- [ ] Preserve `Fn3 = Preview`.
- [ ] Preserve dedicated `AF-ON = AF-ON`.
- [ ] Preserve dedicated `Q = Q.MENU`.
- [ ] Preserve dedicated AF Mode button assignment.
- [ ] Preserve `ISO`, `WB`, Exposure Compensation, LVF and Video Record controls.
- [ ] Preserve the 100–400mm II lens Fn assignment as `AF-ON`.
- [ ] Preserve `Touch Screen = On`, `Touch Tab = Off` and `Touch Pad AF = Off`.
- [ ] Preserve default dial operation and keep all `Operation Lock Setup` items unlocked.
- [ ] Preserve `WB/ISO/Expo. Button = AFTER PRESSING2`.

### 4.3 G9 MkII Custom Mode behavior

- [ ] Open `Setup > Setting > Custom Mode Settings`.
- [ ] Set `Limit No. of Custom Mode` to `10`.
- [ ] Set `How to Reload Custom Mode` to `Turn the Power ON`.
- [ ] In `Select Loading Details`, set `F / SS / ISO Sensitivity` to `On`.
- [ ] In `Select Loading Details`, set `White Balance` to `On`.
- [ ] Remember that temporary field changes persist across sleep and mode changes but return to the registered values after a power cycle.

## Phase 5 — Q.Menu, My Menu and Buttons

Configure Q.Menu in photo mode. Where the camera maintains a separate video Q.Menu, preserve it unless explicitly listed.

### 5.1 Q.Menu — G9 MkI

- [x] Slot 1: `Flash Mode`.
- [x] Slot 2: `Flash Adjust.`.
- [x] Slot 3: `Stabilizer`.
- [x] Slot 4: `Metering Mode`.
- [x] Slot 5: `Quality`.
- [x] Slot 6: `Shutter Type`.
- [x] Slot 7: `Bracket`.
- [x] Slot 8: `Burst Shot Setting`.
- [x] Slot 9: `Min. Shtr Speed`.
- [x] Slot 10: `Photo Style`.
- [x] Slot 11: `Aspect Ratio`.
- [x] Slot 12: `Peaking`.
- [x] Keep `Shutter Delay` in My Menu.
- [x] Leave positions 13-15 empty.
- [x] Remove `i.Dynamic` and `Ex. Tele Conv.` from Q.Menu.

### 5.2 Q.Menu — G9 MkII

- [ ] Slot 1: `AF Detection Setting`.
- [ ] Slot 2: `Detecting Subject`.
- [ ] Slot 3: `Image Stabilizer`.
- [ ] Slot 4: `Metering Mode`.
- [ ] Slot 5: `Picture Quality`.
- [ ] Slot 6: `Shutter Type`.
- [ ] Slot 7: `Bracketing`.
- [x] Slot 8: `AF Custom Setting(Photo)`.
- [x] Slot 9: `Focus Peaking`.
- [ ] Slot 10: `Min. Shutter Speed`.
- [ ] Slot 11: `Photo Style`.
- [ ] Slot 12: `Aspect Ratio`.
- [ ] Remove `Flash Mode` and `Flash Adjust.` from Q.Menu.
- [x] Keep unsupported Q.Menu candidates `Burst Shot Setting` and `Shutter Delay` in My Menu instead.

### 5.3 My Menu — G9 MkI exact order

- [x] 1. `Cust.Set Mem.`.
- [x] 2. `Bracket`.
- [x] 3. `Burst Shot 1 Setting`.
- [x] 4. `Burst Shot 2 Setting`.
- [x] 5. `Shutter Delay`.
- [x] 6. `ISO Sensitivity (photo)`.
- [x] 7. `Long Shtr NR`.
- [x] 8. `Peaking`.
- [x] 9. `Sensor Cleaning`.
- [x] 10. `Format`.
- [x] 11. `Fn Button Set`.
- [x] 12. `Time Lapse/Animation`.
- [x] 13. `Zebra Pattern`.

`Rec Quality` was removed because general video is outside the approved project scope.

### 5.4 My Menu — G9 MkII exact order

The final list has 17 entries. `Rec Quality` remains excluded, and the MkII's single combined `Burst Shot Setting` replaces the MkI's two burst-preset entries.

- [ ] 1. `Save to Custom Mode`.
- [ ] 2. `Bracketing`.
- [ ] 3. `Burst Shot Setting`.
- [ ] 4. `Shutter Delay`.
- [ ] 5. `ISO Sensitivity (photo)`.
- [ ] 6. `Long Exposure NR`.
- [ ] 7. `Focus Peaking`.
- [ ] 8. `Sensor Cleaning`.
- [ ] 9. `Card Format`.
- [ ] 10. `Fn Button Set`.
- [ ] 11. `Time Lapse/Animation`.
- [ ] 12. `Zebra Pattern`.
- [ ] 13. `AF Custom Setting(Photo)`.
- [ ] 14. `Focus Limiter`.
- [ ] 15. `High Resolution Mode Setting`.
- [ ] 16. `Live View Composite`.
- [ ] 17. `Custom Mode Settings`.
- [ ] Remove `Ex. Tele Conv.`, `Sound Rec Level Adj.` and entries duplicated in Q.Menu.

### Phase 5 gate

- [ ] Open Q.Menu and My Menu on both cameras and verify the order from left to right / top to bottom using the tables above.

## Phase 6 — G9 MkI Custom Modes

The G9 MkI target modes were historically derived from its old C3-2 macro setup, but final programming must not use that old slot as a source. Build the common template in normal `A`, save C1, then use newly completed C1/C3-1 anchors exactly as specified in the final G9 MkI runbook. The display labels below are documentation labels; only enter a title if the camera itself presents an editable title field.

The checked macro items below are historical acceptance-test evidence. They approve the targets but do not satisfy final closure after the common template is reprogrammed. Repeat all five mode saves and power-cycle capture checks with `G9MkI-Final-Implementation-and-Backup-Checklist.md`.

### 6.1 C1 — General / Street

Construction source: complete common template built in normal `A`; do not recall old C1.

- [ ] Build the complete common template in normal `A` mode.
- [ ] Set `A`, f/5.6, AFS, Single Shot, Auto ISO, Standard, RAW, AWB, Multi, stabilization On and EFC.
- [ ] Set the G9 MkI AF mode to `225-Area`; this is the approved broad-area mode without automatic detection.
- [ ] Change `Min. Shtr Speed`: `AUTO` → `1/125s`.
- [ ] Save with `Cust.Set Mem. > C1`.

### 6.2 C2 — Portrait

Construction source: newly completed C1; do not recall old C2.

- [ ] Recall the newly completed C1.
- [ ] Set `A`, f/2.8, AFS, Single Shot, Auto ISO, Portrait Photo Style, RAW, AWB, Multi, stabilization On and EFC.
- [ ] Set the G9 MkI AF mode to `Human Detect AF`; do not look for a separate `Full Area + detection` combination.
- [ ] Preserve `Min. Shtr Speed = 1/125s`.
- [ ] Save with `Cust.Set Mem. > C2`.

### 6.3 C3-1 — Single Macro — TTL

Baseline: completed C1 Set and Forget anchor.

- [x] Build `M`, MF, Single Shot, f/16, 1/200s, ISO 200, Natural, RAW, AWB, Mechanical shutter and stabilizer Mode 1.
- [x] Set `Flash Mode = Forced Flash On`, X3 Group A TTL at +0.0, Group B Off, and both MF12 units in Group A.
- [x] Verify `Peaking = On / LOW / RED`, focus-ring `MF Assist`, windowed/PIP `MF Assist Display`, and `MF Guide = On`.
- [x] Set `Constant Preview = Off`.
- [x] Save and verify C3-1 with `Cust.Set Mem.`.

### 6.4 C3-3 — Supported Macro Focus Bracket

Baseline: completed C3-1 anchor.

- [x] Build `M`, MF, Single Shot, f/8, 1/200s, ISO 400, Natural, RAW, AWB, `MSHTR`, Peaking, MF Assist, MF Guide and Constant Preview Off.
- [x] Use X3 Group A TTL at +0.0, Group B Off, and both MF12 units in Group A.
- [x] Set stabilization Off and use Function Lever MODE2.
- [x] Set Focus Bracket to Step 2, Image Count 40 and Sequence 0/+.
- [x] Reject `Shutter Delay = 2 SEC` after it delayed every frame; save `Shutter Delay = Off`.
- [x] Validate 40 fully illuminated frames in 12 seconds, approximately 4 cm of focus coverage, and a perfect Helicon Focus merge.
- [x] Save and verify C3-3 with `Cust.Set Mem.`.

### 6.5 C3-2 — Macro Burst — Manual Flash

Baseline: completed C3-1 anchor.

- [x] Build `M`, MF, f/16, 1/200s, ISO 400, Natural, RAW, AWB, Mechanical shutter, stabilizer Mode 1 and Constant Preview Off.
- [x] Promote the physical `Burst II` position with `Burst Shot 2 Setting = M` after the field test.
- [x] Set X3 Group A to Manual 1/32, Group B Off, with both MF12 units in Group A.
- [x] Validate 26 consistently illuminated frames in 10 seconds (approximately 2.6 fps).
- [x] Reject TTL after it produced many black frames under C3-2 conditions.
- [x] Save and verify C3-2 with `Cust.Set Mem.`.

### 6.6 G9 MkI recall verification

- [ ] Power-cycle the camera.
- [ ] Recall C1 and confirm General / Street settings.
- [ ] Recall C2 and confirm Portrait settings.
- [x] Recall C3-1 and confirm Single Shot, TTL, f/16, ISO 200 and IS Mode 1.
- [x] Recall C3-2 and confirm Burst II M, Manual 1/32 flash baseline, f/16, ISO 400 and IS Mode 1.
- [x] Recall C3-3 and confirm Focus Bracket 2 / 40 / 0+, f/8, ISO 400, delay Off and IS Off.
- [ ] Verify that Function Lever position 1 preserves handheld stabilization and position 2 forces stabilization Off without disabling flash.

## Phase 7 — G9 MkII Custom-Mode Migration

> **Archived instructions — do not execute Sections 7.1 through 7.4.** They preserve the rejected old-slot migration plan only as historical context. Old C3 slots may have drifted and are not valid construction sources. The actual G9 MkII build used newly programmed and power-cycle-verified targets; its completed evidence begins at Section 7.5, and the pre-change DAT remains the rollback source.

### 7.1 Archived old-slot preservation plan — do not execute

#### A. Preserve current Wildlife / Action as target C3-2

- [ ] Load current C3-3 `Wildlife / Action`.
- [ ] Preserve all technical settings: `M`, f/6.3 at 400mm, 1/1000s, Auto ISO, AFC, Animal Detection, Zone, Burst I / H, Electronic shutter, IS Mode 1, RAW, Standard, AWB and Multi.
- [ ] Save to C3-2.
- [ ] Set title to `Wildlife Action`.

#### B. Preserve current BIF as target C3-3

- [ ] Load current C3-8 `Birds in Flight (BIF)`.
- [ ] Preserve all technical settings: `M`, f/6.3 at 400mm, 1/4000s, Auto ISO, AFC, Animal Detection, Tracking, Burst I / H, Electronic shutter, IS Mode 2, RAW, Standard, AWB and Multi.
- [ ] Save to C3-3.
- [ ] Set title to `Wildlife Fast`.

#### C. Preserve current lightning as target C3-8

- [ ] Load current C3-6 `Landscape Tripod (Lightning) — Live View Composite`.
- [ ] Preserve `M`, MF, Single Shot, f/8, Detection Off, Standard, Flash Forced Off, Daylight WB, tripod use and IS Off.
- [ ] Change 16:9 → `4:3`.
- [ ] Change shutter speed: 4s → `1/125s`.
- [ ] Change ISO: 400 → `200`.
- [ ] Set `Live View Composite = Off` as the saved baseline.
- [ ] Change forced Mechanical shutter → `EFC`.
- [ ] Change `Shutter Delay`: 2 seconds → `Off`.
- [ ] Change `Long Exposure NR`: forced On → `Off`.
- [ ] Save to C3-8.
- [ ] Set title to `Lightning T MIOPS`.

#### D. Preserve current Landscape as temporary C3-9

- [ ] Load current C3-1 `Landscape`.
- [ ] Without changing its technical settings, save it temporarily to C3-9.
- [ ] Set temporary title to `LANDSCAPE TEMP`.
- [ ] Do not convert C3-9 to High Resolution until all landscape derivatives below have been saved.

### 7.2 Archived temporary-landscape plan — do not execute

#### C3-4 — Standard Handheld Landscape

- [ ] Load temporary C3-9 `LANDSCAPE TEMP`.
- [ ] Preserve `A`, f/8, AFS, Single Shot, Detection Off, Landscape Photo Style, AWB and normal handheld stabilization.
- [ ] Verify RAW, Multi metering and EFC.
- [ ] Save to C3-4.
- [ ] Set title to `Landscape Handheld`.

#### C3-5 — Tripod HDR Landscape

- [ ] Load temporary C3-9 `LANDSCAPE TEMP`.
- [ ] Preserve `A`, f/8, AFS, Detection Off, Landscape, AWB, Multi, RAW and EFC.
- [ ] Change ISO: Auto → `100`.
- [ ] Change stabilization: On → `Off`.
- [ ] Set Exposure Bracket to five frames at 1 EV: `-2, -1, 0, +1, +2 EV`.
- [ ] Set the complete bracket to run from one shutter press.
- [ ] Set `Shutter Delay = 2 SEC`.
- [ ] Save to C3-5.
- [ ] Set title to `Landscape T HDR`.

#### C3-6 — Focus-Bracketed Landscape

- [ ] Load temporary C3-9 `LANDSCAPE TEMP`.
- [ ] Preserve `A`, f/8, AFS, Detection Off, Landscape, AWB, Multi, RAW and EFC.
- [ ] Change ISO: Auto → `100`.
- [ ] Change AF area → `1-Area`.
- [ ] Change stabilization: On → `Off`.
- [ ] Set `Bracket Type = Focus Bracket`.
- [ ] Set `Step = 3`.
- [ ] Set `Image Count = 30`.
- [ ] Set `Sequence = 0/+`.
- [ ] Set `Shutter Delay = 2 SEC`.
- [ ] Save to C3-6.
- [ ] Set title to `Landscape T Focus`.

#### C3-7 — Long Exposure Landscape with ND

- [ ] Load temporary C3-9 `LANDSCAPE TEMP`.
- [ ] Preserve f/8, AFS, Detection Off, Landscape, AWB, Multi, RAW, EFC and Long Exposure NR Off.
- [ ] Change recording mode: A → `M`.
- [ ] Change ISO: Auto → `100`.
- [ ] Set initial shutter speed to `1s`.
- [ ] Change AF area → `1-Area`.
- [ ] Change stabilization: On → `Off`.
- [ ] Set `Shutter Delay = 2 SEC`.
- [ ] Save to C3-7.
- [ ] Set title to `Landscape T ND`.

#### C3-9 — Tripod High Resolution Landscape

- [ ] Load temporary C3-9 `LANDSCAPE TEMP` one last time.
- [ ] Preserve `A`, f/8, AFS, Detection Off, Landscape, AWB, Multi, 4:3 and RAW workflow.
- [ ] Change ISO: Auto → `100`.
- [ ] Change AF area → `1-Area`.
- [ ] Select High Resolution drive / mode.
- [ ] Set `Handheld High-Res = Off`.
- [ ] Set High Resolution `Picture Quality = RAW`.
- [ ] After setting High Resolution `Picture Quality = RAW`, verify that `Picture Size` becomes unavailable and is camera-forced to 4:3, 11552×8672 (100 MP / XL).
- [ ] Set `Simul Record Normal Shot = On`.
- [ ] Set `Motion Blur Processing = MODE1`.
- [ ] Set `Shutter Delay = 2 SEC`.
- [ ] Verify the camera has forced Electronic shutter and made stabilization unavailable.
- [ ] Save over C3-9.
- [ ] Set title to `Landscape T HiRes`.

### 7.3 Archived wildlife-derivation plan — do not execute

#### C3-1 — Stationary / Slow Wildlife

- [ ] Load target C3-2 `Wildlife Action`.
- [ ] Preserve every setting except shutter speed.
- [ ] Change shutter speed: 1/1000s → `1/500s`.
- [ ] Save to C3-1.
- [ ] Set title to `Wildlife Still`.

#### C3-2 — Wildlife Action

- [ ] Recall C3-2.
- [ ] Verify `M`, f/6.3 at 400mm, 1/1000s, Auto ISO, AFC Animal Zone, Burst I / H, Electronic shutter and IS Mode 1.
- [ ] Verify title `Wildlife Action`.

#### C3-3 — Fast / Erratic Wildlife

- [ ] Recall C3-3.
- [ ] Verify `M`, f/6.3 at 400mm, 1/4000s, Auto ISO, AFC Animal Tracking, Burst I / H, Electronic shutter and IS Mode 2.
- [ ] Verify title `Wildlife Fast`.

### 7.4 Archived C1/C2 plan — do not execute

#### C1 — General / Street

- [ ] Load current C1 `Street / General`.
- [ ] Preserve `A`, f/5.6, AFS, Single Shot, Auto ISO, Standard, RAW, AWB, Multi, stabilization On and EFC.
- [ ] Set `AF Detection Setting = Off` explicitly.
- [ ] Set AF area to `Full Area`.
- [ ] Change `Min. Shutter Speed`: Auto → `1/125s`.
- [ ] Save to C1 and set title `General / Street`.

#### C2 — Portrait

- [ ] Load current C2 `Portrait / People`.
- [ ] Preserve `A`, f/2.8, AFS, Single Shot, Auto ISO, 1/125s minimum, Portrait, RAW, AWB, Multi, stabilization On and EFC.
- [ ] Set `AF Detection Setting = On` explicitly.
- [ ] Set `Detecting Subject = HUMAN` and enable Face/Eye target behavior.
- [ ] Set AF area to `Full Area`.
- [ ] Save to C2 and set title `Portrait`.

### 7.5 C3-10 — Wildlife Video

Executable implementation record resumes here. Sections 7.1–7.4 above are not checklist work and must remain unchecked.

Firmware 2.7 does not permit loading a still-photo custom mode into Creative Video. C3-10 was therefore built directly from the Creative Video position, without recalling the previous C3-10.

- [x] Set Creative Video, Manual exposure, f/6.3, 1/125, Auto ISO, AFC Animal Eye/Body Tracking, Standard, AWB and Multiple Metering.
- [x] Set `Rec. File Format = MP4`, `Image Area of Video = FULL` and `Rec Quality = 4K/10bit/100M/60p`.
- [x] Set `Luminance Level = 64-940`.
- [x] Set optical stabilization Normal, `E-Stabilization (Video) = HIGH` and `Boost I.S. (Video) = OFF`.
- [x] Set `Continuous AF = MODE1` and `AF Custom Setting(Video) = OFF`.
- [x] Set `Sound Rec Level Disp. = ON`, `Displaying Size = SMALL`, internal mic gain `STANDARD` and level `0dB`.
- [x] Set `Sound Rec Level Limiter = ON`, `Wind Noise Canceller = STANDARD` and `Lens Noise Cut = ON`.
- [x] Verify that the XLR control is not displayed without an adaptor; the built-in microphone is active.
- [x] Verify that `Sound Rec Quality` is fixed to 48kHz/16bit with MP4.
- [x] Set Zebra 1 and Zebra 2 to the camera-available maximum of 95%, with `Zebra Pattern = OFF`.
- [x] Set `Red REC Frame Indicator = ON`.
- [x] Verify that `Time Code` is unavailable with MP4.
- [x] Preserve `Auto Exposure in P/A/S/M = ON` globally.
- [x] Save to C3-10 and set title `Wildlife Video`.

### 7.6 G9 MkII video separation

- [x] Open `Custom > Image Quality > CreativeVideo Combined Set.`.
- [x] Set `F / SS / ISO / Exposure Comp.` to the movie-camera icon (separate video value).
- [x] Set `White Balance` to the movie-camera icon (separate video value).
- [x] Set `Photo Style` to the movie-camera icon (separate video value).
- [x] Set `Metering Mode` to the movie-camera icon (separate video value).
- [x] Set `AF Mode` to the movie-camera icon (separate video value).

No general-purpose video baseline is part of the approved trip workflow; C3-10 is the only programmed video mode.

### 7.7 G9 MkII recall verification

- [x] Power-cycle the camera so all temporary changes return to their registered values.
- [x] Recall C1 through C3-10 one at a time and compare each to the approved summary below.
- [x] Confirm every in-camera title is 22 characters or fewer and matches the checklist exactly.
- [x] Confirm C3-5 runs the entire five-frame exposure bracket from one press.
- [x] Confirm C3-6 runs a 30-frame 0/+ focus bracket.
- [x] Confirm C3-8 fires immediately without a shutter delay.
- [x] Confirm C3-9 records both the 100 MP RAW and the normal safety RAW.
- [x] Confirm C3-10 records a short 4K60 10-bit HEVC clip with audio.

## Phase 8 — Physical Calibration and Acceptance Tests

These tests determine whether the approved starting values should be re-saved. Do not alter a baseline merely because one uncontrolled trial fails.

### 8.1 G9 MkI C3-1 — TTL recycle test

- [x] Fit the Olympus 60mm Macro, both MF12 units, both diffusers and the X3.
- [x] Use fully charged flash batteries.
- [x] Recall C3-1 and photograph the same static subject repeatedly at f/16, 1/200s and ISO 200.
- [x] Confirm consistent TTL exposure and a usable viewfinder across several individual frames.
- [x] Preserve the approved single-shot TTL baseline; do not expect TTL burst recycling.

### 8.2 G9 MkI C3-2 — short burst test

- [x] Test both MF12 units together in Group A at Manual 1/32, with Group B Off.
- [x] Validate the slower quality-priority alternative at physical Burst I / `L`, ISO 200: 15 illuminated frames in 10 seconds.
- [x] Test physical Burst II with `Burst Shot 2 Setting = M`, ISO 400 and Manual 1/32.
- [x] Validate 26 consistently illuminated frames in 10 seconds, approximately 2.6 fps.
- [x] Reject TTL for C3-2 after it produced many black frames.
- [x] Promote Burst II / `M`, ISO 400 and Group A Manual 1/32 to the saved baseline.
- [x] Use 3–5 frames per field burst; a stackable sequence remains optional.

### 8.3 G9 MkI C3-3 — supported focus bracket

- [x] Use a tripod or firm support and a completely static test subject.
- [x] Confirm all 40 TTL frames receive consistent exposure.
- [x] Record 40 frames in 12 seconds with approximately 4 cm of near-to-far focus coverage.
- [x] Merge the sequence in Helicon Focus and confirm a perfect merge.
- [x] Reject `Shutter Delay = 2 SEC` because it delayed every bracket frame; preserve delay Off.
- [x] Keep the manual 1/64 and modeling-light paths as fallbacks only; the validated TTL baseline did not require them.
- [x] Re-save and recall C3-3 after the validated settings passed both capture and merge tests.

### G9 MkI calibration gate

- [x] C3-1, C3-2 and C3-3 have written pass results, and every change arising from calibration is recorded in the Menu Atlas, field cards and approved G9 MkI target JSON.
- [ ] Complete `G9MkI-Final-Implementation-and-Backup-Checklist.md` before declaring the G9 MkI closed.

### 8.4 G9 MkII C3-6 — landscape focus-bracket calibration

- [ ] Fit the 12–35mm II and use a tripod.
- [ ] At 12mm, capture and merge Step 3 / 30 frames from a near foreground point to infinity.
- [ ] Repeat at 25mm.
- [ ] Repeat at 35mm.
- [ ] Inspect every merge for focus gaps and edge artifacts.
- [ ] Adjust Step or Image Count only when the same deficiency is repeatable; then re-save C3-6 and record the final values in the decision ledger.

### 8.5 G9 MkII C3-8 — MIOPS Smart+ test

- [ ] Connect the tested camera cable between MIOPS Smart+ and the G9 MkII.
- [ ] Recall C3-8 and manually pre-focus at the expected lightning distance.
- [ ] Set MIOPS to `Lightning Mode` with `Prefocus` enabled.
- [ ] Verify that the MIOPS release signal triggers the camera immediately.
- [ ] Calibrate sensitivity under at least two ambient-light conditions.
- [ ] Record false triggers and missed simulated triggers.
- [ ] Keep Live View Composite as a manual night-lightning variation, not the C3-8 saved baseline.
- [ ] Keep MIOPS Capsule Pro outside the lightning workflow; reserve it for optional panorama or timelapse experiments.

### 8.6 Teleconverter checks

- [ ] With the 100–400mm II zoomed to at least 250mm, set the lens `ZOOM LIMIT` switch to `ON` before attaching a teleconverter.
- [ ] Attach DMW-TC14 and verify the restricted native 210–400mm zoom range without forcing the ring.
- [ ] Recall C3-1 and verify normal operation at the automatically reduced maximum aperture.
- [ ] In strong light, test C3-2 with an active subject and review ISO and AF performance.
- [ ] Attach DMW-TC20 using the same physical procedure.
- [ ] Test C3-1 with a stationary subject, tripod and clean air; review ISO, AF reliability and atmospheric distortion.
- [ ] Do not adopt C3-3 as a routine teleconverter mode.
- [ ] For a locked tripod head and fixed composition, switch lens O.I.S. Off.
- [ ] For a loose head, gimbal or active tracking, keep lens O.I.S. On and camera IS Mode 1.
- [ ] For a subject beyond 5m, use the lens focus-range selector at `5m–∞`.
- [ ] If branches or foreground clutter confuse Zone AF, temporarily select `1-Area`.

### Phase 8 gate

- [ ] Every test has a written pass/fail result, and every setting change arising from calibration has been deliberately re-saved and recorded.

## Phase 9 — Final Trip Preparation and Backup

- [ ] Complete `Pre-Trip-Sensor-and-Lens-Cleaning-Checklist.md` for both Lumix bodies, all five travel lenses (Leica 9mm f/1.7, 12–35mm II, 35–100mm II, Olympus 60mm Macro and incoming Leica 100–400mm II), DMW-TC14, DMW-TC20, Raynox DCR-250, every travel filter, GoPro HERO12 and Zeiss Victory SF 8x32. The Olympus 90mm Macro remains home.
- [ ] Complete Stage A in Brazil with internal cleaning, Giottos blower and `AFTER-BRAZIL` evidence; do not improvise wet-cleaning chemistry.
- [ ] Photographic Solutions Digital Survival Kit, Type 2 / 17 mm with Aeroclipse, ordered 2026-08-20 for US delivery; confirm receipt and seals, then repeat the test and complete `AFTER-USA` acceptance before the trip begins.
- [ ] Preserve the before/after RAW sensor tests and the final test photographs for every lens in two locations before formatting any card.
- [ ] Confirm that no repeatable spot remains at the same image coordinates in either body's final `f/16` test; if blower cleaning does not remove it, stop and use professional service rather than wet-cleaning the sensor.
- [ ] Re-run the full global-settings checklist on both bodies.
- [ ] Power-cycle and verify every Custom Mode again.
- [x] G9 MkII: preserve the earlier `G2FINAL.DAT` recovery snapshot, then save the final camera state as the new `G9MK2SET.DAT` file without overwriting it.
- [x] G9 MkII: copy final `G9MK2SET.DAT` to `backup/G9II/`; verified size 8,959,329 bytes and SHA-256 `E19F602CEC7F9FBA7E7B23C4D6966447460BAEA3BCB5CD9DBCC10A58DF93061B`.
- [x] Copy the validated intermediate G9 repository file `backup/G9/G9FINAL.DAT` (historically named `G9POST.DAT` on the recovery card) and the interim G9 MkII file `G2FINAL.DAT` to a dedicated spare SD card that will remain with the camera kit.
- [x] Insert the recovery card in both bodies and confirm that `G9POST.DAT` and `G2FINAL.DAT` appear under `Save/Restore Camera Setting > Load`, without executing the load.
- [x] Save final G9 MkI state as `G9MK1SET.DAT`, copy it to `backup/G9/`, and verify 10,957 bytes and SHA-256 `EA7F23E8C96C12334E65620DC4B7AF40524B03742BB807E5A8E4AFAA2039EABE` without deleting any recovery file.
- [ ] Engage the recovery card's physical write-protect lock and label it `CAMERA RECOVERY — DO NOT FORMAT`.
- [x] G9 MkII: set `Keep Settings While Format = On`.
- [ ] G9 MkII: save or synchronize the final configuration through LUMIX Sync if the app exposes the supported function.
- [x] G9 MkI: preserve the final state through `G9MK1SET.DAT`, updated project JSON and the G9 MkI runbook; menu photographs remain part of the physical kit documentation.
- [x] Change `data/camera-config-g9mki.json` from implementation-pending to implemented-and-validated after verifying the final camera backup.
- [ ] Update `data/camera-config-g9mkii.json` only after its remaining physical settings have passed validation.
- [x] Preserve the configured HERO12 state in `data/camera-config-gopro-hero12.json`; the GoPro/Quik workflow has no Panasonic-style `.DAT` export.
- [ ] Test and format the two 64 GB reserve cards in their intended bodies.
- [ ] After every test file has been imported and verified, perform one `File Number Reset` on each camera immediately before the trip.
- [ ] Format all primary cards in their respective cameras after the reset and final backup verification.
- [ ] Confirm both cards are present, empty, recognized and set to Backup Recording on the morning of departure.

## Final Mode Summary

| Slot | G9 MkI operational baseline | G9 MkII operational baseline |
|---|---|---|
| C1 | A, f/5.6, AFS `225-Area`, Single, Auto ISO min 1/125 | G9 MkII equivalent to be named exactly by its firmware-2.7 Menu Atlas |
| C2 | A, f/2.8, AFS `Human Detect AF`, Single, Auto ISO min 1/125 | G9 MkII equivalent to be named exactly by its firmware-2.7 Menu Atlas |
| C3-1 | M, MF, Single, f/16, 1/200, ISO 200, TTL, IS Mode 1 | M, f/6.3, 1/500, Auto ISO, AFC Animal Zone, H, Electronic, IS Mode 1 |
| C3-2 | M, MF, Burst II / `M` (2.6 fps measured), f/16, 1/200, ISO 400, both MF12 in Group A at Manual 1/32, IS Mode 1 | M, f/6.3, 1/1000, Auto ISO, AFC Animal Zone, H, Electronic, IS Mode 1 |
| C3-3 | M, MF, Single, f/8, 1/200, ISO 400, Focus Bracket 2/40/0+, delay Off, TTL Group A, IS Off | M, f/6.3, 1/4000, Auto ISO, AFC Animal Tracking, H, Electronic, IS Mode 2 |
| C3-4 | — | A, f/8, AFS, Detection Off, Landscape, handheld IS |
| C3-5 | — | A, f/8, ISO 100, five-frame ±2 EV bracket, 2s delay, IS Off |
| C3-6 | — | A, f/8, ISO 100, AFS 1-Area, Focus Bracket 3/30/0+, 2s delay, IS Off |
| C3-7 | — | M, f/8, ISO 100, 1s start, AFS 1-Area, 2s delay, IS Off |
| C3-8 | — | M, f/8, 1/125, ISO 200, MF, Daylight, EFC, immediate MIOPS release, IS Off |
| C3-9 | — | A, f/8, ISO 100, AFS 1-Area, 100 MP RAW plus normal RAW, 2s delay |
| C3-10 | — | Creative Video M, 4K60 10-bit HEVC 100 Mbps, 1/125, f/6.3, Auto ISO, AFC Animal Tracking |

## Field Recommendations Without Reprogramming

- For ordinary travel with no preparation, start with C1 on either camera.
- For people, switch to C2; the 35–100mm f/2.8 is the normal portrait lens.
- For an unexpected macro opportunity, fit the 60mm Macro and use C3-1; move to C3-2 only when a short burst can improve the chance of a sharp plane.
- Keep C3-3 macro as a low-priority supported workflow for static subjects.
- Keep the G9 MkII in C3-1 when wildlife may appear but is not yet moving quickly.
- Move progressively to C3-2 for action and C3-3 for fast or erratic movement.
- Use DMW-TC14 mainly from C3-1 and use C3-2 only in strong light.
- Use DMW-TC20 mainly from C3-1 with tripod, slow subjects and good light.
- Use C3-4 for normal handheld landscape work.
- Use C3-5 only when the five RAW files will be merged.
- Use C3-6 only when a close foreground and distant background justify a focus stack.
- Use C3-7 as the starting point for ND64 long exposures; adjust shutter speed in the field without re-saving the mode.
- Use C3-8 with MIOPS Smart+ as the principal lightning workflow.
- Use C3-9 only for substantially static scenes; use the simultaneous normal RAW when movement damages the High Resolution result.
- Use C3-10 for wildlife video; change temporarily to 1/250s or 1/500s only for very fast motion or frame extraction.
