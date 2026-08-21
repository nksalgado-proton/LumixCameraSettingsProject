# Lumix G9 MkI Firmware 2.7 - Menu Atlas and Programming Runbook

Status: Approved firmware-2.7 navigation and programming authority. Macro targets are acceptance-tested; final common-setting and mode closure work follows `G9MkI-Final-Implementation-and-Backup-Checklist.md`.

Camera: Panasonic Lumix DC-G9 (G9 MkI)

Camera menu language: English

Installed firmware target: 2.7

Primary authority: Panasonic operating instructions `DC-G9_DVQP1406ZE_full_eng.pdf`. This revision contains the cumulative menu changes through firmware 2.5. Panasonic's firmware notes describe 2.6 and 2.7 as corrective releases rather than menu-structure releases; firmware 2.7 corrects restoration of the shooting mode when a Custom Mode is restored.

Firmware release authority: `https://av.jpn.support.panasonic.com/support/global/cs/dsc/download/fts/dl/g9.html`.

Project targets: `Camera-Mode-Redesign-US-Parks-2026.md`, `Camera-Implementation-Checklist-US-Parks-2026.md`, and `field-cards.json`.

## 1. Purpose

This is the navigation authority for programming the G9 MkI. The Field Cards explain when and why to use a mode. This atlas explains exactly how to build it in the camera.

During implementation, only one table will be presented at a time. After completing that screen, the operator reports `Done` or describes the discrepancy. We do not continue until the visible result agrees with the table.

## 2. Stop rules

Stop immediately and do not choose the closest-looking option if:

- the menu item is absent;
- the English name differs from the atlas;
- the available values differ from the atlas;
- an item is greyed out;
- changing one item forces another item to change;
- the camera is not in the required physical state;
- the screen order differs from the manual.

Record the complete screen or photograph it. The manual and the actual firmware-2.7 camera screen then become the evidence for revising the atlas.

## 3. Recovery checkpoint

The pre-change state has already been saved and copied to the project backup directory.

| Camera file | Size | SHA-256 |
|---|---:|---|
| `backup/G9/G9PRE.DAT` | 10,957 bytes | `0BD17FC0C01BB6F7C15D76783DF73C1EBB4AADDCDD69FA50AA03D3812C70EEF5` |

Do not format or overwrite the card copy used for recovery until the new configuration has passed the final capture tests. Keep the project backup private and outside any publication or commit.

## 4. What the G9 MkI actually stores

This storage model determines the programming order. Panasonic manual pages 85-86 are authoritative.

| Setting class | Stored in C1/C2/C3-n? | Consequence |
|---|---:|---|
| Recording mode and `Rec` menu settings | Yes | Set them before each `Cust.Set Mem.` save. |
| Almost all `Custom` menu settings | Yes | They must be verified in every custom mode, even when the intended value is common. |
| `Custom > Menu Guide` | No | Global behavior. |
| `Custom > Face Recog.` registered data | No | Global data. Not part of this project. |
| `Custom > Profile Setup` | No | Global data. Not part of this project. |
| `Setup` menu | No, except `Night Mode` | Program once. Verify `Night Mode` before every custom-mode save. |
| `Playback > Rotate Disp.` | No | Program once. |
| `Playback > Picture Sort` | No | Preserve. |
| `Playback > Delete Confirmation` | No | Program once. |
| Mode dial, focus mode lever, drive mode dial, Function Lever | Physical controls | A custom-mode recall cannot move them. Check them before use and before verification. |
| Godox X3/MF12 mode and power | External equipment | Set and verify on the trigger/flashes; the camera cannot save these values. |

Important correction: the earlier checklist's instruction to set the `Custom` menu once globally is unsafe on this camera. We will create a verified common template, but we will inspect it again immediately before saving each custom mode.

## 5. Manual reconciliation - corrections to earlier project language

These are evidence-based corrections, not changes of photographic intent.

| Earlier project wording or assumption | Firmware-2.7-compatible G9 MkI instruction | Reason / manual source |
|---|---|---|
| `Double Card Slot Function` | `Double Slot Function` | Exact Setup name, pp. 220, 229. |
| `Picture Quality` | `Quality` | Exact Rec name, pp. 185, 187. |
| `Multi` metering | Select the Multiple-metering icon | The manual labels it `Multiple`, p. 192. |
| `Full Area` | `225-Area` | Exact G9 MkI AF mode, p. 93. |
| `Full Area + Face/Eye Detection` | Select `Human Detect AF` as the AF mode | Detection is an AF mode on this body, not a separate Full-Area switch; firmware 2.4, pp. F-32 to F-33. |
| `Face/Eye Detection Off` in C1 | Select `225-Area` | Choosing a non-detection AF mode is the unambiguous way to disable detection. |
| `Photo Grid Line = 9-SECTION` | `Guide Line` and select the 3-by-3 grid icon | Exact item name; values are displayed as icons, p. 213. |
| `Blinking Highlights` | `Highlight = ON` | Exact item name, p. 214. |
| `Focus Peaking` | `Peaking` | Exact item name, p. 213. |
| `Lens Focus Resume` | `Lens Position Resume` | Exact item name, p. 216. |
| `Focus/Shutter Priority` | `Focus/Release Priority` | Exact item name, p. 209. |
| `Looped Focus Frame` | `Loop Movement Focus Frame` | Exact item name, p. 209. |
| `Half-Press Shutter` | `Half Press Release` | Exact item name; target is `OFF`, p. 207. |
| `Diffraction Compensation = ON/OFF` | `AUTO/OFF`; target `OFF` | Actual available values, p. 196. |
| `Burst I = L at 2 fps` | `Burst Shot 1 Setting = L` | The selectable value is `L`; fps is expected performance, not another setting, p. 115. |
| `Burst II = M at 7 fps` | `Burst Shot 2 Setting = M` | The selectable value is `M`; fps is expected performance, not another setting, p. 115. |
| Q.Menu has 12 or possibly 13 positions | Q.Menu accepts up to 15 items | Manual p. 59. Availability of each candidate must be checked in the camera's add-item screen. |
| My Menu supports 16 items | My Menu accepts up to 23 menus | Manual p. 232. |
| `Copyright Information`, `Artist`, `Copyright Holder` | Not available on the G9 MkI; do not program | No such Setup item exists in the full G9 manual or its firmware appendices. |
| `Thermal Management` / `Recording Max Temperature` | Not available on the G9 MkI; do not program | No such item exists in the full G9 manual or its firmware appendices. |
| `System Frequency` | Not available as a G9 MkI Setup item; do not program | No such Setup item appears in the manual. Region-dependent video rates are not changed in this still-photo project. |
| `Battery Use Priority = BODY` | Do not program without a battery grip | The menu is documented only after attaching the optional DMW-BGG9. The user does not use a grip; p. 309. |
| `Focal Length display = ON` | No persistent setting by this name | `Focal Length Set` under `Rec > Stabilizer` applies to lenses whose focal length is not communicated automatically. Do not use it for the planned electronic lenses. |
| `Level Gauge = ON` | Display it with `DISP.` or the assigned `Level Gauge` function | Setup contains only `Level Gauge Adjust.`, not an on/off preference; pp. 61, 232. |
| `Aperture Ring Increment = 1/3 EV` as a general requirement | Verify only if the attached lens exposes it | It applies to supported clickless aperture-ring lenses; firmware 1.3 appendix, p. F-9. It is not required for the planned 12-35mm II, 35-100mm II, or Olympus 60mm Macro workflow. |

## 6. Programming session protocol

Every live session follows this loop:

1. Confirm battery above 60%, both cards inserted, camera menu in English, and firmware 2.7.
2. State the required lens and all physical-control positions.
3. Navigate from a known starting point. Press `MENU/SET`; do not rely on the menu opening at the previous location.
4. Complete only the current screen table, from top to bottom.
5. Report `Done` or report the exact mismatch.
6. Re-open the screen and read back every changed row.
7. Only after verification, proceed to the next screen.
8. Save a custom mode only when its complete worksheet has passed.
9. Power-cycle and recall the saved mode before considering it complete.

Use these action words consistently:

| Action | Meaning |
|---|---|
| `SET` | Change to the target value. |
| `VERIFY` | Read the displayed value; change only if it differs. |
| `PRESERVE` | Do not change. Report the current value if requested. |
| `SKIP` | Do not open or alter. |
| `UNAVAILABLE` | The camera/lens/accessory state does not expose this item. Do not improvise. |

## 7. Stage A - known starting state

Use this state before the global Setup pass. It minimizes unavailable items and prevents flash or burst restrictions from hiding settings.

| Physical control | Required position | Verification |
|---|---|---|
| Mode dial | `A` | `A` appears on the recording screen. |
| Focus mode lever | `AFS/AFF` | Recording screen shows `AFS` after `Rec > AFS/AFF = AFS`. |
| Drive mode dial | Single Shot | Single-frame icon appears. |
| Function Lever | `MODE1` | Assigned override is inactive. |
| Lens | Lumix 12-35mm II | Lens communicates aperture and focal length. |
| Flash trigger | Removed or switched off | No flash workflow is active. |

## 8. Stage B - Setup and Playback settings that are truly global

The G9 MkI `Setup` menu is a flat menu. The headings below are work batches, not camera subfolders. Navigate by the exact item name shown.

### B1. Firmware confirmation

Navigation: `MENU/SET > Setup > Version Disp.`

| Screen row | Target | Action | Manual |
|---|---|---|---|
| Body firmware | `2.7` | VERIFY | p. 227 plus Panasonic firmware release record |
| Lens firmware | Record displayed version | PRESERVE | p. 227 |

Gate: stop if the body does not display 2.7.

Observed at the start of implementation:

| Attached lens | Observed firmware | Official current target | Decision |
|---|---:|---:|---|
| Lumix G X Vario 12–35mm f/2.8 II (`H-HSA12035`) | 1.3 | 1.3 | Updated and verified. |
| Lumix G X Vario 35–100mm f/2.8 II (`H-HSA35100`) | 1.3 | 1.3 | Updated and verified. |
| M.Zuiko Digital ED 60mm f/2.8 Macro | 1.2 | 1.2 | Current and verified. |
| Leica 100–400mm II (`H-RSA100400`) | Not yet received | 1.1 | Audit after collection in Denver; the outgoing MkI lens is excluded. |

### B2. Double-card backup recording

Navigation: `MENU/SET > Setup > Double Slot Function`

| Screen row | Target | Action | Manual |
|---|---|---|---|
| `Recording Method` | `Backup Rec` | SET | p. 229 |

The two inserted cards must be the matched Lexar 256 GB 1667x UHS-II V60 pair. Do not select `Relay Rec` or `Allocation Rec`.

Verification is a capture test, not just a menu check: take one RAW file and confirm it is present on both cards.

### B3. File-name prefix

Navigation: `MENU/SET > Setup > Folder / File Settings > File Name Setting`

| Screen row | Target | Action | Manual |
|---|---|---|---|
| `Folder Number Link` | Not selected | PRESERVE | p. 228 |
| `User Setting` | Select | SET | p. 228 |
| Three-character segment | `G1_` | SET | p. 228 |

Expected sRGB filename construction: `PG1_0001.RW2`. The leading `P` is the camera's sRGB marker; `G1_` is the user segment. Do not run `No.Reset` during configuration testing.

### B4. Bluetooth clock and location

Navigation: `MENU/SET > Setup > Bluetooth`

First-time pairing is not a simple ON/OFF toggle. Use `Setup > Bluetooth > Bluetooth > SET > Pairing`, then in Panasonic Image App select `Bluetooth`, turn it on, and select the camera under `Camera enable to be registered`. On iPhone, enable Wi-Fi, select the SSID/device name shown by the camera, and return to Image App. Registration is complete only after the initial Wi-Fi connection succeeds (manual pp. 252-255).

Bluetooth pairing data is not included in `Save/Restore Camera Setting` (manual p. 230), so treat the phone connection as a separate recovery domain. Only after pairing, set the following rows:

| Screen row | Target | Action | Manual |
|---|---|---|---|
| `Bluetooth` | `OFF` during the day | SET | pp. 222, 254-255 |
| `Location Logging` | `OFF` | SET | pp. 222, 268 |
| `Remote Wakeup` | `OFF` | SET | pp. 222, 260 |
| `Auto Clock Set` | Preference `ON`; displayed as forced `OFF` while Bluetooth is off | VERIFY | pp. 222, 269; observed on firmware 2.7 |
| `Auto Transfer` | `OFF` | SET | pp. 222, 264 |
| `Wi-Fi network settings` | Do not change | SKIP | pp. 222, 265 |
| `Returning from Sleep Mode` | Preserve; it may be unavailable while `Remote Wakeup` is OFF | PRESERVE/UNAVAILABLE | firmware 1.1, p. F-8 |

Approved travel workflow: enable Bluetooth each morning, connect Panasonic Image App and visually verify clock synchronization; then turn Bluetooth off for the day. The camera was observed to restore the saved `Auto Clock Set = ON` preference when Bluetooth reconnects. Direct camera geolocation is not used; an iPhone reference photograph supplies location during post-processing.

### B5. Beep

Navigation: `MENU/SET > Setup > Beep`

| Screen row | Target | Action | Manual |
|---|---|---|---|
| `Beep Volume` | OFF/speaker-muted icon | SET | p. 222 |
| `E-Shutter Vol` | OFF/speaker-muted icon | SET | p. 222 |
| `E-Shutter Tone` | Preserve | PRESERVE | p. 222 |

The tone choice is irrelevant while `E-Shutter Vol` is off. Do not use `Silent Mode` to achieve this global result because `Silent Mode` also disables flash and forces `ESHTR`.

### B6. Economy

Navigation: `MENU/SET > Setup > Economy`

| Screen row | Target | Action | Manual |
|---|---|---|---|
| `Sleep Mode` | `10MIN.` | SET | p. 223 |
| `Sleep Mode(Wi-Fi)` | Preserve current value | PRESERVE | p. 223 |
| `Auto LVF/Monitor Off` | `2MIN.` | SET | p. 223 |
| `Power Save LVF Shooting > Time` | Preserve current value | PRESERVE | p. 223 |
| `Power Save LVF Shooting > Display` | Preserve current value | PRESERVE | p. 223 |

### B7. Display speeds and luminance

These are separate flat Setup items.

| Exact navigation | Target | Action | Manual |
|---|---|---|---|
| `Setup > Monitor Display Speed` | `60fps` | SET | p. 224 |
| `Setup > LVF Display Speed` | `60fps` | SET | p. 224 |
| `Setup > Monitor Luminance` | `[A*]` automatic-luminance icon | SET | p. 225 |

Do not alter `Monitor Display` or `Viewfinder` color calibration unless a separate calibration session is planned.

### B8. Eye Sensor

Navigation: `MENU/SET > Setup > Eye Sensor`

| Screen row | Target | Action | Manual |
|---|---|---|---|
| `Sensitivity` | `LOW` | SET | p. 226 |
| `LVF/Monitor Switch` | `LVF/MON AUTO` | VERIFY | p. 226 |

### B9. USB

These are separate flat Setup items.

| Exact navigation | Target | Action | Manual |
|---|---|---|---|
| `Setup > USB Mode` | `Select on connection` | SET | p. 226 |
| `Setup > USB Power Supply` | `ON` | SET | p. 226 |

### B10. Items deliberately not changed

| Exact Setup item | Action | Reason |
|---|---|---|
| `Clock Set` | PRESERVE after Auto Clock Set verification | Phone synchronization is the target workflow. |
| `World Time` | Configure only when the travel-time-zone workflow begins | It is not saved by the camera-settings backup. |
| `Travel Date` | SKIP | No project need. |
| `Wi-Fi` | SKIP outside active transfer/remote use | Avoid an unnecessary active Wi-Fi session. |
| `Monitor Display` / `Viewfinder` | PRESERVE | Calibration is outside this migration. |
| `Status-LCD Backlight` | PRESERVE | No approved change. |
| `Battery Use Priority` | UNAVAILABLE without grip | User does not use DMW-BGG9. |
| `TV Connection` | PRESERVE | Atomos/video setup is outside the G9 MkI still-photo migration. |
| `Language` | VERIFY `English` | Do not change. |
| `No.Reset` | SKIP | Defer until all testing is finished. |
| `Reset` / `Reset Network Settings` | SKIP | Destructive to the configuration. |
| `Pixel Refresh` / `Sensor Cleaning` / `Level Gauge Adjust.` | Separate maintenance only | Not configuration preferences. |
| `Format` | SKIP | Cards were just formatted and now contain the recovery copy. |

### B11. Playback safety

| Exact navigation | Target | Action | Manual |
|---|---|---|---|
| `Playback > Rotate Disp.` | `ON` | SET | p. 248 |
| `Playback > Picture Sort` | Preserve current value | PRESERVE | p. 249 |
| `Playback > Delete Confirmation` | `“No” first` | SET | p. 249 |

## 9. Stage C - common saved template for every G9 MkI custom mode

The following settings are stored in C1/C2/C3-n. They must be set in the working state and verified before every `Cust.Set Mem.` operation.

### C1. Rec menu - common output baseline

Navigation: `MENU/SET > Rec`. The G9 MkI Rec menu is flat; follow the order shown.

| Order | Exact item | Common target | Action | Manual |
|---:|---|---|---|---|
| 1 | `Aspect Ratio` | `4:3` | SET | p. 185 |
| 2 | `Picture Size` | `L` | SET | p. 186 |
| 3 | `Quality` | RAW-only icon | SET | p. 187 |
| 4 | `AFS/AFF` | `AFS` when the physical lever is at AFS/AFF | VERIFY | p. 89 |
| 5 | `AF Custom Setting(Photo)` | Preserve | PRESERVE | p. 90 |
| 6 | `Photo Style` | Mode-specific | SET in worksheet | pp. 188-190 |
| 7 | `Filter Settings` | `OFF` | VERIFY | p. 190 |
| 8 | `Color Space` | `sRGB` | SET | p. 191 |
| 9 | `Metering Mode` | Multiple icon | SET | p. 192 |
| 10 | `Highlight Shadow` | Standard | VERIFY | p. 192 |
| 11 | `i.Dynamic` | `OFF` | SET | p. 193 |
| 12 | `i.Resolution` | `OFF` | SET | p. 193 |
| 13 | `Flash` | Mode-specific | SET in worksheet | pp. 158-160 |
| 14 | `Red-Eye Removal` | `OFF` | VERIFY | p. 194 |
| 15 | `ISO Sensitivity (photo) > ISO Auto Lower Limit Setting` | `200` | VERIFY | p. 194 |
| 16 | `ISO Sensitivity (photo) > ISO Auto Upper Limit Setting` | `AUTO` | SET | p. 194 |
| 17 | `Min. Shtr Speed` | Mode-specific | SET in worksheet | p. 194 |
| 18 | `Long Shtr NR` | `OFF` | SET | p. 195 |
| 19 | `Shading Comp.` | `OFF` | SET | p. 195 |
| 20 | `Diffraction Compensation` | `OFF` | SET | p. 196 |
| 21 | `Stabilizer > Operation Mode` | Mode-specific | SET in worksheet | pp. 145-146 |
| 22 | `Stabilizer > E-Stabilization (Video)` | Preserve | PRESERVE | p. 146 |
| 23 | `Stabilizer > I.S. Lock (Video)` | Preserve | PRESERVE | p. 147 |
| 24 | `Stabilizer > Focal Length Set` | Do not use with electronic planned lenses | SKIP | p. 147 |
| 25 | `Ex. Tele Conv.` | `OFF` | VERIFY | p. 150 |
| 26 | `Digital Zoom` | `OFF` | VERIFY | p. 152 |
| 27 | `Burst Shot 1 Setting` | `L` | SET | p. 115 |
| 28 | `Burst Shot 2 Setting` | `M` | SET | p. 115 |
| 29 | `6K/4K PHOTO` | Preserve/off state | VERIFY | p. 119 |
| 30 | `Post Focus` | Preserve/off state | VERIFY | p. 130 |
| 31 | `Self Timer` | Preserve/off state | VERIFY | p. 135 |
| 32 | `High Resolution Mode` | Not active | VERIFY | pp. 196-198 |
| 33 | `Time Lapse/Animation` | Not active | VERIFY | pp. 137-140 |
| 34 | `Silent Mode` | `OFF` | SET | p. 198 |
| 35 | `Shutter Type` | Mode-specific | SET in worksheet | p. 199 |
| 36 | `Shutter Delay` | Mode-specific | SET in worksheet | p. 200 |
| 37 | `Bracket > Bracket Type` | Mode-specific; otherwise `OFF` | SET in worksheet | pp. 141-144 |
| 38 | `HDR` | `OFF` | VERIFY | p. 201 |
| 39 | `Multi Exp.` | Not active | VERIFY | p. 202 |
| 40 | `Time Stamp Rec` | `OFF` | VERIFY | p. 170 |

### C2. Custom > Exposure

Navigation: `MENU/SET > Custom > Exposure`

| Screen row | Target | Action | Manual |
|---|---|---|---|
| `ISO Increments` | `1/3 EV` | SET | p. 207 |
| `Extended ISO` | `OFF` | SET | p. 207 |
| `Exposure Comp. Reset` | `ON` | SET | p. 207 |
| `Exposure Offset Adjust.` | All metering methods at `±0` | VERIFY | firmware 2.0, p. F-30 |

### C3. Custom > Focus / Release Shutter

Navigation: `MENU/SET > Custom > Focus / Release Shutter`

| Order | Exact item | Target | Action | Manual |
|---:|---|---|---|---|
| 1 | `AF/AE Lock` | Preserve current assignment | PRESERVE | pp. 106, F-12 |
| 2 | `AF/AE Lock Hold` | Preserve current value | PRESERVE | p. 207 |
| 3 | `Shutter AF` | `OFF` | SET | p. 207 |
| 4 | `Half Press Release` | `OFF` | SET | p. 207 |
| 5 | `Quick AF` | `OFF` | SET | p. 208 |
| 6 | `Eye Sensor AF` | `OFF` | SET | p. 208 |
| 7 | `Pinpoint AF Setting` | Preserve | PRESERVE | p. 208 |
| 8 | `AF-Point Scope Setting` | Preserve | PRESERVE | p. 94 |
| 9 | `AF Assist Lamp` | Preserve | PRESERVE | p. 208 |
| 10 | `Focus/Release Priority > AFS/AFF` | `FOCUS` | SET | p. 209 |
| 11 | `Focus/Release Priority > AFC` | Preserve | PRESERVE | p. 209 |
| 12 | `Focus Switching for Vert / Hor` | `OFF` | SET | p. 97 |
| 13 | `Loop Movement Focus Frame` | `OFF` | SET | p. 209 |
| 14 | `AF Area Display` | Preserve | PRESERVE | p. 209 |
| 15 | `AF+MF` | `ON` | SET | p. 210 |
| 16 | `MF Assist` | Focus-ring activation icon | SET | p. 210 |
| 17 | `MF Assist Display` | PIP/window icon | SET | p. 210 |

The camera uses icons for `MF Assist` and `MF Assist Display`. Match the icon to the manual illustration; do not substitute a translated text label.

### C4. Custom > Operation

Navigation: `MENU/SET > Custom > Operation`

| Exact item | Target | Action | Manual |
|---|---|---|---|
| `Fn Button Set` | See Stage D | DEFER | pp. 60-62 |
| `Fn Lever Setting` | See Stage D | DEFER | p. 63 |
| `Q.MENU` | `CUSTOM` | SET | pp. 58-59, 211 |
| `Dial Set.` | Preserve | PRESERVE | p. 46 |
| `Joystick Setting` | `D.FOCUS Movement` | SET | p. 48 |
| `Operation Lock Setting` | Preserve | PRESERVE | p. 211 and firmware F-6 |
| `Video Button` | `ON` | VERIFY | p. 211 |
| `Touch Settings` | Preserve current values | PRESERVE | p. 211 |
| `Focus Ring Lock` | `OFF` | VERIFY | firmware F-6 |
| `WB/ISO/Expo. Button` | Preserve current value | PRESERVE | firmware F-6 |
| `Video Rec. Button (Remote)` | Preserve | PRESERVE | firmware F-9 |

### C5. Custom > Monitor / Display

Navigation: `MENU/SET > Custom > Monitor / Display`

| Order | Exact item | Target | Action | Manual |
|---:|---|---|---|---|
| 1 | `Auto Review > Duration Time (photo)` | `OFF` | SET | pp. 212, F-30 |
| 2 | `Auto Review > Playback Operation Priority` | Preserve | PRESERVE | p. 212 |
| 3 | `Monochrome Live View` | `OFF` | VERIFY | p. 212 |
| 4 | `Constant Preview` | `ON` for C1/C2; `OFF` for all macro modes | SET in worksheet | p. 212 |
| 5 | `Live View Boost` | `OFF` | SET | firmware F-6/F-30 |
| 6 | `Peaking` | `ON` | SET | p. 213 |
| 7 | `Peaking > SET > Detect Level` | `LOW` | SET | p. 213 |
| 8 | `Peaking > SET > Display Color` | Red icon | SET | p. 213 |
| 9 | `Peaking > SET > Display While AFS` | Preserve | PRESERVE | firmware F-12 |
| 10 | `Histogram` | `OFF` | SET | p. 213 |
| 11 | `Guide Line` | 3-by-3 grid icon | SET | p. 213 |
| 12 | `Center Marker` | `OFF` | VERIFY | p. 214 |
| 13 | `Highlight` | `ON` | SET | p. 214 |
| 14 | `Zebra Pattern` | `OFF` | SET | p. 214 |
| 15 | `Expo.Meter` | `OFF` | SET | p. 215 |
| 16 | `MF Guide` | `ON` | SET | p. 215 |
| 17 | `LVF/Monitor Disp. Set` | Preserve | PRESERVE | p. 49 |
| 18 | `Monitor Info. Disp.` | Preserve | PRESERVE | p. 215 |
| 19 | `Rec Area` | Still-picture icon | VERIFY | p. 215 |
| 20 | `Remaining Disp.` | Remaining-pictures icon | VERIFY | p. 215 |
| 21 | `Menu Guide` | Preserve | PRESERVE | p. 215; not stored in custom modes |
| 22 | `Red REC Frame Indicator` | Preserve | PRESERVE | firmware F-34 |
| 23 | `Frame Marker` | `OFF` | VERIFY | firmware F-35 |

`Night Mode` is physically located in Setup but is the one Setup item stored in a custom mode. Verify `Setup > Night Mode` has both monitor and LVF red display disabled before every save.

### C6. Custom > Lens / Others

Navigation: `MENU/SET > Custom > Lens / Others`

| Exact item | Target | Action | Manual |
|---|---|---|---|
| `Lens Position Resume` | `ON` | SET | p. 216 |
| `Power Zoom Lens` | Preserve | PRESERVE | p. 153 |
| `Lens Fn Button Setting` | `AF-ON` when exposed by the attached lens | SET/UNAVAILABLE | p. 216 |
| `Aperture Ring Increment` | `1/3EV` only when exposed by a supported lens | VERIFY/UNAVAILABLE | firmware F-9 |
| `Vertical Position Info (Video)` | Preserve | PRESERVE | firmware F-34 |
| `Focus Ring Control` | `NON-LINEAR` when exposed by a supported lens | SET/UNAVAILABLE | firmware F-36 |
| `Face Recog.` | Do not change | SKIP | pp. 217-218; data is not stored in custom modes |
| `Profile Setup` | Do not change | SKIP | p. 219; data is not stored in custom modes |

## 10. Stage D - camera-specific controls

### D1. Physical buttons

Navigation: `MENU/SET > Custom > Operation > Fn Button Set > Setting in REC mode`

| Button shown by camera | Target | Action | Manual |
|---|---|---|---|
| `Fn1` | `AF-ON` | VERIFY/SET | pp. 60-61 |
| `Fn2` | `Q.MENU` | VERIFY/SET | pp. 60-61 |
| `Fn3` | `LVF/Monitor Switch` | VERIFY/SET | pp. 60-61 |
| `Fn4` | `AF-Point Scope` | VERIFY/SET | pp. 60-61 |
| `Fn5` | `Preview` | VERIFY/SET | pp. 60-61 |

Do not alter the dedicated `ISO`, `WB`, exposure-compensation, or video-record controls. Do not infer a numbered Fn mapping from an older JSON file; match the physical button shown on the camera's assignment diagram.

### D2. Function Lever

Navigation: `MENU/SET > Custom > Operation > Fn Lever Setting`

| Screen row | Target | Action | Manual |
|---|---|---|---|
| `Function of Fn Lever` | `Stabilizer` | SET | p. 63 |
| `MODE 2 Setting` | `OFF` | SET | p. 63 |

Meaning after programming:

- `MODE1`: use the stabilizer value stored in the recalled custom mode.
- `MODE2`: force `Stabilizer = OFF`.

Use `MODE1` for C1, C2, C3-1, and C3-2. Use `MODE2` for supported C3-3. The physical lever is an override and must always appear on the Field Card.

### D3. Q.Menu

Navigation: `MENU/SET > Custom > Operation > Q.MENU > CUSTOM`. Return to the recording screen, press `Fn2` (assigned to `Q.MENU` on this camera), press down to select the edit icon, and press `MENU/SET` (manual p. 59). The manual's generic `[Q.MENU]` notation is a function label, not a dedicated G9 MkI button.

The manual confirms a maximum of 15 items but does not publish a complete firmware-2.7 add-item catalogue. The camera's actual add-item screen subsequently confirmed all 12 approved entries below. `Shutter Delay` was not promoted and remains in My Menu.

| Desired order | Confirmed exact name | Manual evidence | Implementation result |
|---:|---|---|---|
| 1 | `Flash Mode` | Assignable-function list, p. 61 | Confirmed on camera. |
| 2 | `Flash Adjust.` | p. 61 | Confirmed on camera. |
| 3 | `Stabilizer` | p. 61 | Confirmed on camera. |
| 4 | `Metering Mode` | p. 61 | Confirmed on camera. |
| 5 | `Quality` | p. 61 | Confirmed on camera. |
| 6 | `Shutter Type` | p. 61 | Confirmed on camera. |
| 7 | `Bracket` | p. 61 | Confirmed on camera. |
| 8 | `Burst Shot Setting` | p. 61 | Confirmed on camera. |
| 9 | `Min. Shtr Speed` | p. 61 | Confirmed on camera. |
| 10 | `Photo Style` | p. 61 | Confirmed on camera. |
| 11 | `Aspect Ratio` | p. 61 | Confirmed on camera. |
| 12 | `Peaking` | p. 61 | Confirmed on camera. |
| Deferred | `Shutter Delay` | Not established by the manual's assignable list | Kept in My Menu. |

This corrects the earlier unsupported assumption that `Shutter Delay` must occupy a Q.Menu slot. Leave unused positions empty unless a later, evidence-based revision is approved.

### D4. My Menu

Navigation: `MENU/SET > My Menu > My Menu Setting`

Use `Add` to register an item, `Sorting` to arrange it, and `Delete Item` to remove a wrong entry. The camera supports up to 23 items (manual p. 232).

| Order | Exact item to add | Source menu |
|---:|---|---|
| 1 | `Cust.Set Mem.` | Setup |
| 2 | `Bracket` | Rec |
| 3 | `Burst Shot 1 Setting` | Rec |
| 4 | `Burst Shot 2 Setting` | Rec |
| 5 | `Shutter Delay` | Rec |
| 6 | `ISO Sensitivity (photo)` | Rec |
| 7 | `Long Shtr NR` | Rec |
| 8 | `Peaking` | Custom > Monitor / Display |
| 9 | `Sensor Cleaning` | Setup |
| 10 | `Format` | Setup |
| 11 | `Fn Button Set` | Custom > Operation |
| 12 | `Time Lapse/Animation` | Rec |
| 13 | `Zebra Pattern` | Custom > Monitor / Display |

`Burst Shot Setting` is not the name of a Rec menu item on this camera; the Rec menu contains two exact entries, so both are included. Remove `Ex. Tele Conv.` and any duplicate items only after confirming the desired replacement list is complete.

## 11. Stage E - custom-mode construction order

There is no spare custom slot for a neutral template: C1, C2, C3-1, C3-2, and C3-3 are all final destinations. Build the complete common working template in `A`, add the C1-specific values, and save that state as `C1 - General / Street`. C1 then becomes the clean stored anchor for the remaining modes. The pre-migration DAT remains the rollback source for every old configuration.

| Build order | Source working state | Destination | Reason |
|---:|---|---|---|
| 1 | Common template built in `A`, plus C1-specific values | C1 | Creates the final C1 and the stored anchor in one verified save. |
| 2 | Recall completed C1 | C2 | Portrait differs from C1 in relatively few settings. |
| 3 | Recall completed C1 | C3-1 | Creates the clean TTL macro base without importing an old custom-mode state. |
| 4 | Recall completed C3-1 | C3-3 | Supported focus bracket shares the macro base and must not inherit burst/manual-flash changes. |
| 5 | Recall completed C3-1 again | C3-2 | Program the manual-flash burst last so it cannot contaminate the supported-bracket source. |

Before every `Cust.Set Mem.` save, verify all common-template sections as well as the destination worksheet. Never use an old custom slot as a construction source merely because it has a similar name.

## 12. Stage F - exact custom-mode worksheets

### F1. C1 - General / Street

Required physical state before editing and before recall verification:

| Control | Position |
|---|---|
| Mode dial | `C1` when recalling; use the recalled state as the working source |
| Focus mode lever | `AFS/AFF` |
| Drive mode dial | Single Shot |
| Function Lever | `MODE1` |
| Lens | 12-35mm II |
| Godox X3 | Attach and switch on only while setting the saved flash state; remove after the save |

Recording-screen and menu targets:

| Exact control/path | Target | Action | Manual |
|---|---|---|---|
| Recording mode | `A` | VERIFY | mode dial/custom working state |
| Aperture | `F5.6` | SET with dial | recording screen |
| ISO | `AUTO` | SET with ISO button/dial | pp. 109-111 |
| White Balance | `AWB` | SET with WB button/dial | pp. 112-114 |
| AF mode button | `225-Area` | SET | p. 93 |
| `Rec > Min. Shtr Speed` | `1/125` | SET | p. 194 |
| `Rec > Photo Style` | `Standard` | SET | p. 188 |
| `Rec > Stabilizer > Operation Mode` | Normal icon | SET | p. 146 |
| `Rec > Shutter Type` | `EFC` | SET | p. 199 |
| `Rec > Flash > Flash Mode` | Forced flash off icon | SET with the X3 attached and on | pp. 155, 158 |
| `Rec > Bracket > Bracket Type` | `OFF` | SET | p. 141 |
| `Rec > Shutter Delay` | `OFF` | SET | p. 200 |
| `Custom > Monitor / Display > Constant Preview` | `ON` | SET | p. 212 |
| Common template, Sections C1-C6 | All common targets | VERIFY | cited above |
| `Setup > Night Mode` | Monitor OFF and LVF OFF | VERIFY | p. 224 |

Save: `MENU/SET > Setup > Cust.Set Mem. > C1`.

Recall test: power off, power on, set Mode dial to C1, confirm `A`, F5.6, AUTO ISO, AFS, Single Shot, 225-Area, RAW, 4:3, EFC, and Stabilizer normal.

### F2. C2 - Portrait

Required physical state:

| Control | Position |
|---|---|
| Mode dial | `C2` when recalling |
| Focus mode lever | `AFS/AFF` |
| Drive mode dial | Single Shot |
| Function Lever | `MODE1` |
| Lens | 35-100mm II; 12-35mm II is an approved field alternative |
| Godox X3 | Attach and switch on only while setting the saved flash state; remove after the save |

Recording-screen and menu targets:

| Exact control/path | Target | Action | Manual |
|---|---|---|---|
| Recording mode | `A` | VERIFY | recording screen |
| Aperture | `F2.8` | SET with dial | recording screen |
| ISO | `AUTO` | SET with ISO button/dial | pp. 109-111 |
| White Balance | `AWB` | SET with WB button/dial | pp. 112-114 |
| AF mode button | `Human Detect AF` | SET | firmware F-32/F-33 |
| Automatic detection within this AF mode | Human detection ON; animal detection OFF | VERIFY | firmware F-32/F-33 |
| `Rec > Min. Shtr Speed` | `1/125` | SET | p. 194 |
| `Rec > Photo Style` | `Portrait` | SET | p. 188 |
| `Rec > Stabilizer > Operation Mode` | Normal icon | SET | p. 146 |
| `Rec > Shutter Type` | `EFC` | SET | p. 199 |
| `Rec > Flash > Flash Mode` | Forced flash off icon | SET with the X3 attached and on | pp. 155, 158 |
| `Rec > Bracket > Bracket Type` | `OFF` | SET | p. 141 |
| `Rec > Shutter Delay` | `OFF` | SET | p. 200 |
| `Custom > Monitor / Display > Constant Preview` | `ON` | SET | p. 212 |
| Common template, Sections C1-C6 | All common targets | VERIFY | cited above |
| `Setup > Night Mode` | Monitor OFF and LVF OFF | VERIFY | p. 224 |

Save: `MENU/SET > Setup > Cust.Set Mem. > C2`.

Recall test: photograph a person, verify the yellow detection frame selects a face/eye, and confirm `A`, F2.8, AUTO ISO, AFS, Single Shot, RAW, 4:3, Portrait, EFC, and Stabilizer normal.

### F3. C3-1 - Single Macro - TTL

Required physical state:

| Control/accessory | Position |
|---|---|
| Mode dial | `C3`, then select `C3-1` after it is saved |
| Focus mode lever | `MF` |
| Drive mode dial | Single Shot |
| Function Lever | `MODE1` |
| Lens | Olympus 60mm Macro |
| Godox X3 | TTL, flash exposure compensation 0 |
| MF12 | Both units in Group A, equal output, diffusers fitted; Group B OFF |

Camera targets:

| Exact control/path | Target | Action | Manual |
|---|---|---|---|
| Recording mode | `M` | SET in working state | recording screen |
| Aperture / shutter | `F16` / `1/200` | SET with dials | recording screen |
| ISO | `200` | SET | pp. 109-111 |
| White Balance | `AWB` | SET | pp. 112-114 |
| `Rec > Photo Style` | `Natural` | SET | p. 188 |
| `Rec > Flash > Flash Mode` | Forced flash on icon | SET | p. 158 |
| `Rec > Flash > Firing Mode` | `TTL` if exposed with the trigger attached | VERIFY | pp. 158-160 |
| `Rec > Flash > Flash Adjust.` | `±0` | SET | p. 160 |
| `Rec > Stabilizer > Operation Mode` | Normal icon | SET | p. 146 |
| `Rec > Shutter Type` | `MSHTR` | SET | p. 199 |
| `Rec > Bracket > Bracket Type` | `OFF` | SET | p. 141 |
| `Rec > Shutter Delay` | `OFF` | SET | p. 200 |
| `Custom > Monitor / Display > Constant Preview` | `OFF` | SET | p. 212 |
| Common template, Sections C1-C6 | All common targets | VERIFY | cited above |
| `Setup > Night Mode` | Monitor OFF and LVF OFF | VERIFY | p. 224 |

Save: `MENU/SET > Setup > Cust.Set Mem. > C3-1`.

The camera stores its flash menu state; it does not store the X3 or MF12 external state. Verify both domains after recall.

### F4. C3-3 - Supported Macro Focus Bracket

Create this by recalling the completed C3-1 anchor, not C3-2.

Required physical state:

| Control/accessory | Position |
|---|---|
| Mode dial | `C3`; recall the completed C3-1 source before editing |
| Focus mode lever | `MF` |
| Drive mode dial | Single Shot |
| Function Lever | `MODE2` during use; `MODE1` is acceptable while editing if `Stabilizer` is set OFF in the menu |
| Lens | Olympus 60mm Macro |
| Support | Tripod or firm surface |
| Godox X3/MF12 | X3 Group A TTL at +0.0, Group B OFF; both MF12 units in Group A with diffusers |

Camera targets:

| Exact control/path | Target | Action | Manual |
|---|---|---|---|
| Recording mode | `M` | SET | recording screen |
| Aperture / shutter | `F8` / `1/200` | SET | recording screen |
| ISO | `400` | SET | pp. 109-111 |
| White Balance | `AWB` | SET | pp. 112-114 |
| `Rec > Photo Style` | `Natural` | SET | p. 188 |
| `Rec > Flash > Flash Mode` | Forced flash on icon | SET | p. 158 |
| `Rec > Stabilizer > Operation Mode` | `OFF` | SET | p. 146 |
| `Rec > Shutter Type` | `MSHTR` | SET | p. 199 |
| `Rec > Bracket > Bracket Type` | Focus Bracket icon | SET | p. 141 |
| `Rec > Bracket > More Settings > Step` | `2` | SET | p. 143 |
| `Rec > Bracket > More Settings > Image Count` | `40` | SET | p. 143 |
| `Rec > Bracket > More Settings > Sequence` | `0/+` icon | SET | p. 143 |
| `Rec > Shutter Delay` | `OFF` | SET; 2SEC was rejected because it delayed every bracket frame | p. 200 |
| `Custom > Monitor / Display > Constant Preview` | `OFF` | SET | p. 212 |
| Common template, Sections C1-C6 | All common targets | VERIFY | cited above |
| `Setup > Night Mode` | Monitor OFF and LVF OFF | VERIFY | p. 224 |

Save: `MENU/SET > Setup > Cust.Set Mem. > C3-3`.

Validated result: TTL completed 40 fully illuminated frames in 12 seconds, covered approximately 4 cm from near to far, and merged perfectly in Helicon Focus. `Shutter Delay = 2SEC` was rejected after producing roughly one frame every two seconds because the delay applied to every bracket image.

### F5. C3-2 - Macro Burst - Manual Flash

Build this last from a fresh recall of the newly completed C3-1. Never enter the old C3-2 destination as the working source.

Required physical state:

| Control/accessory | Position |
|---|---|
| Mode dial | `C3`; recall the newly completed C3-1 before editing, then save the result to C3-2 |
| Focus mode lever | `MF` |
| Drive mode dial | Burst II |
| Function Lever | `MODE1` |
| Lens | Olympus 60mm Macro |
| Godox X3 | Manual |
| MF12 | Both units in Group A at 1/32, diffusers fitted; Group B OFF |

Camera targets:

| Exact control/path | Target | Action | Manual |
|---|---|---|---|
| Recording mode | `M` | SET | recording screen |
| Aperture / shutter | `F16` / `1/200` | SET | recording screen |
| ISO | `400` | SET | pp. 109-111 |
| White Balance | `AWB` | SET | pp. 112-114 |
| `Rec > Photo Style` | `Natural` | SET | p. 188 |
| `Rec > Burst Shot 1 Setting` | `L` | VERIFY; slower alternative | p. 115 |
| `Rec > Burst Shot 2 Setting` | `M` | SET; tested baseline | p. 115 |
| `Rec > Flash > Flash Mode` | Forced flash on icon | SET | p. 158 |
| `Rec > Stabilizer > Operation Mode` | Normal icon | SET | p. 146 |
| `Rec > Shutter Type` | `MSHTR` | SET | p. 199 |
| `Rec > Bracket > Bracket Type` | `OFF` | SET | p. 141 |
| `Rec > Shutter Delay` | `OFF` | SET | p. 200 |
| `Custom > Monitor / Display > Constant Preview` | `OFF` | SET | p. 212 |
| Common template, Sections C1-C6 | All common targets | VERIFY | cited above |
| `Setup > Night Mode` | Monitor OFF and LVF OFF | VERIFY | p. 224 |

Save: `MENU/SET > Setup > Cust.Set Mem. > C3-2`.

Tested cadence with `Burst II / M`, ISO 400, and both Group A MF12 units at 1/32 was 26 frames in 10 seconds (approximately 2.6 fps), with every frame illuminated consistently. Use three to five frames per burst. The slower quality-priority alternative is `Burst I / L`, ISO 200, and 1/32, measured at 15 frames in 10 seconds.

TTL was tested under the C3-2 conditions and rejected because it produced many black frames. Keep Group A manual at 1/32 for this mode.

## 13. Final verification matrix

Power-cycle between modes so temporary changes cannot masquerade as saved values.

| Mode | Physical controls | Camera recall essentials | Capture test |
|---|---|---|---|
| C1 | AFS/AFF, Single Shot, MODE1 | A, F5.6, AUTO ISO, 1/125 minimum, 225-Area, Standard, EFC, IS normal | General scene; AF-ON focuses; shutter half-press does not focus. |
| C2 | AFS/AFF, Single Shot, MODE1 | A, F2.8, AUTO ISO, Human Detect AF, Portrait, EFC, IS normal | Human face/eye receives the yellow focus frame. |
| C3-1 | MF, Single Shot, MODE1; X3 TTL | M, F16, 1/200, ISO 200, MSHTR, Forced Flash On, IS normal, Constant Preview OFF | Several individual macro frames; consistent TTL exposure and usable viewfinder. |
| C3-2 | MF, Burst II, MODE1; X3 manual, both MF12 in Group A at 1/32 | M, F16, 1/200, ISO 400, Burst 2 M, MSHTR, IS normal | Repeated 3-5-frame bursts; measured 2.6 fps and every frame illuminated. |
| C3-3 | MF, Single Shot, MODE2; supported camera; X3 TTL Group A | M, F8, 1/200, ISO 400, Focus Bracket 2/40/0+, delay OFF, IS OFF | 40 frames in 12s, approximately 4 cm coverage, successful Helicon Focus merge. |

For every mode also verify: RAW, 4:3, Multiple metering, AWB, common Custom template, correct Function Lever behavior, and both-card recording.

## 14. Backup and final closure snapshot

Only after all capture tests pass:

1. Insert the designated primary card in Slot 1.
2. Navigate to `MENU/SET > Setup > Save/Restore Camera Setting > Save`.
3. Select `New File`.
4. Confirm `G9PARKS.DAT` does not already exist on the camera, in `backup/G9/`, or on the dedicated recovery card. Stop rather than overwrite any existing copy.
5. Save the new closure snapshot as `G9PARKS.DAT`.
6. Copy it without force/overwrite to `backup/G9/` and the dedicated recovery card; do not use a primary shooting card as the durable second copy because those cards are later formatted.
7. While both cards are computer-mounted, compare the primary-card source, project copy and recovery-card copy. Require identical size and SHA-256 before ejecting either card.
8. Reinsert the recovery card, confirm `G9PARKS.DAT` appears under Load without executing it, then lock and label the recovery card.
9. Keep `G9PRE.DAT` and the existing `G9FINAL.DAT` intermediate snapshot as rollback files.

Existing post-macro backup:

| Camera file | Size | SHA-256 |
|---|---:|---|
| `backup/G9/G9FINAL.DAT` | 10,957 bytes | `83354A31B04A119AE1016AA1E9E21DD307BBFDBE9CE250DF71AE853CF248ADB1` |

This existing file is the intermediate post-macro snapshot historically called `G9POST.DAT` in the implementation checklist. Preserve it. The post-runbook closure backup uses the new name `G9PARKS.DAT` so no recovery point is overwritten.

## 15. Review gate before implementation

Before touching the camera, approve or revise these evidence-based changes:

- C1 uses `225-Area`, not `Full Area`.
- C2 uses `Human Detect AF`, not `Full Area + Face/Eye Detection`.
- Custom menu settings are verified per custom mode because the G9 MkI stores them in each slot.
- Unsupported G9 MkI targets are removed: Copyright Information, Thermal Management, System Frequency, and battery-priority programming without a grip.
- `Shutter Delay` remains in My Menu unless the firmware-2.7 Q.Menu add screen explicitly offers it.
- Q.Menu can hold up to 15 items and My Menu up to 23.
- Burst rates are validation expectations; the menu values are only `L` and `M`.

Review gate approved. Final camera programming must still pass the power-cycle, capture and backup closure gates in `G9MkI-Final-Implementation-and-Backup-Checklist.md`.
