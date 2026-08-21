# G9 MkI — Final Mode Implementation and Backup Checklist

Date prepared: 2026-08-16  
Camera: Panasonic Lumix G9 MkI, firmware 2.7  
Authority: `G9MkI-Firmware-2.7-Menu-Atlas.md` and the validated macro tests  
Target record: `../data/camera-config-g9mki.json`

Use this runbook with the camera in hand. The G9 MkI is closed only after every item in the **Closure gate** is checked. Do not overwrite `G9PRE.DAT`, the existing repository `G9FINAL.DAT`, or the recovery-card copy historically documented as `G9POST.DAT`; they preserve earlier recovery points.

## 1. Preflight and rollback protection

- [ ] Charge the G9 battery and both MF12 units; fit the Olympus 60mm Macro when checking C3 modes.
- [ ] Insert the two intended G9 primary SD cards and confirm that their existing photographs have been imported and verified.
- [ ] Confirm firmware `2.7` on the camera.
- [ ] Confirm the existing computer recovery files and record their known hashes:
  - `G9PRE.DAT`: `0BD17FC0C01BB6F7C15D76783DF73C1EBB4AADDCDD69FA50AA03D3812C70EEF5`
  - Existing `G9FINAL.DAT` (the same intermediate payload historically documented as `G9POST.DAT`): `83354A31B04A119AE1016AA1E9E21DD307BBFDBE9CE250DF71AE853CF248ADB1`
- [ ] Recompute both rollback hashes and confirm both files are exactly 10,957 bytes. **Stop if either file is missing or any size/hash differs; do not begin camera programming with an unverified rollback.**

```powershell
Get-Item 'backup/G9/G9PRE.DAT', 'backup/G9/G9FINAL.DAT' |
  Select-Object Name, Length, LastWriteTime
Get-FileHash -Algorithm SHA256 'backup/G9/G9PRE.DAT', 'backup/G9/G9FINAL.DAT'
```

- [ ] Do not load either rollback file unless recovery is actually required.
- [ ] Photograph the current C1, C2 and C3-1 through C3-3 recall screens before editing.
- [ ] Remember that the DAT backup does **not** include Bluetooth pairing. Panasonic Image App pairing remains a separate recovery item.

## 2. Apply the final common camera settings

Set these before the final per-mode saves. Items stored inside a Custom Mode must also be checked again while programming each slot.

### Cards, files and output

- [ ] Set `Double Slot Function > Recording Method = Backup Rec`.
- [ ] Set the three user-defined filename characters to `G1_`.
- [ ] Set `Aspect Ratio = 4:3`, `Picture Size = L`, `Quality = RAW`, `Photo Style = Standard`, `Metering Mode = Multi` and `Color Space = sRGB`.
- [ ] Set `i.Dynamic = Off`, `i.Resolution = Off`, `Shading Comp. = Off`, `Diffraction Compensation = Off` and `Long Shtr NR = Off`.
- [ ] Set `Extended ISO = Off`, `ISO Increments = 1/3 EV`, unrestricted Auto ISO upper limit and `Exposure Comp. Reset = On`.
- [ ] Keep the general baseline at `Shutter Type = EFC`, `Bracket = Off`, `HDR = Off`, `Shutter Delay = Off` and `Silent Mode = Off`.

### Focus, display and lens behavior

- [ ] Set `Shutter AF = Off`, `Half Press Release = Off` and `Focus/Release Priority > AFS/AFF = FOCUS`.
- [ ] Set `AF+MF = On`, focus-ring `MF Assist = On`, `MF Assist Display = PIP` and `MF Guide = On`.
- [ ] Set `Peaking = On`, `Detect Level = LOW` and the red `Display Color`.
- [ ] Set `Quick AF = Off`, `Eye Sensor AF = Off`, `Focus Switching for Vert/Hor = Off`, `Loop Movement Focus Frame = Off` and `AF Micro Adjustment = Off`.
- [ ] Set `Lens Position Resume = On`. If the attached lens exposes them, set `Focus Ring Control = NON-LINEAR` and `Lens Fn Button Setting = AF-ON`; otherwise record them as unavailable. Verify `Aperture Ring Increment = 1/3 EV` only if a supported clickless aperture-ring lens exposes it; none of the planned lenses requires it.
- [ ] Set `Auto Review = Off`, `Photo Grid Line = 9-SECTION`, `Live View Boost = Off`, `Night Mode = Off`, `Expo.Meter = Off` and `Sheer Overlay = Off`.
- [ ] Do not use `Focal Length Set` with the planned communicating electronic lenses. Display the level gauge with `DISP.` when desired; there is no persistent `Level Gauge = On` preference.
- [ ] Set global `Constant Preview = On`, `Highlight = On`, `Histogram = Off` and `Zebra Pattern = Off`.

### Power and system

- [ ] Confirm `Sleep Mode = 10 MIN`, `Auto LVF/Monitor Off = 2 MIN`, monitor and LVF frame rates `60fps`, luminance `AUTO` and eye-sensor sensitivity `LOW`.
- [ ] Turn all beeps and electronic-shutter sounds Off.
- [ ] Set `USB Mode = Select on connection` and `USB Power Supply = On`.
- [ ] Do not look for Copyright Information, Thermal Management or System Frequency; firmware 2.7 does not expose those G9 MkI targets.
- [ ] Do not configure Battery Use Priority without a DMW-BGG9 grip.

## 3. Apply the final controls and menus

### Physical-control assignments

- [ ] Preserve `Fn1 = AF-ON`, `Fn2 = Q.MENU`, `Fn3 = LVF/Monitor Switch`, `Fn4 = AF-Point Scope` and `Fn5 = Preview`.
- [ ] Preserve the dedicated ISO, WB, exposure-compensation and Video Record functions.
- [ ] Preserve `Joystick Setting = D.FOCUS Movement`.
- [ ] Change `Function Lever` from Silent Mode to `Stabilizer`, with `Mode 2 Setting = Off`.
- [ ] Confirm MODE1 preserves the saved stabilizer state and MODE2 forces stabilization Off.

### Q.Menu — exact order

- [ ] 1 `Flash Mode`; 2 `Flash Adjust.`; 3 `Stabilizer`; 4 `Metering Mode`.
- [ ] 5 `Quality`; 6 `Shutter Type`; 7 `Bracket`; 8 `Burst Shot Setting`.
- [ ] 9 `Min. Shtr Speed`; 10 `Photo Style`; 11 `Aspect Ratio`; 12 `Peaking`.
- [ ] Leave positions 13–15 empty and keep `Shutter Delay` in My Menu.

### My Menu — exact order

- [ ] 1 `Cust.Set Mem.`; 2 `Bracket`; 3 `Burst Shot 1 Setting`; 4 `Burst Shot 2 Setting`.
- [ ] 5 `Shutter Delay`; 6 `ISO Sensitivity (photo)`; 7 `Long Shtr NR`; 8 `Peaking`.
- [ ] 9 `Sensor Cleaning`; 10 `Format`; 11 `Fn Button Set`; 12 `Time Lapse/Animation`; 13 `Zebra Pattern`.
- [ ] Set `Burst Shot 1 Setting = L` and `Burst Shot 2 Setting = M`.

## 4. Program and save the five Custom Modes

Physical focus, drive and Function Lever positions are not moved by recalling a Custom Mode. Set them as listed before every capture test.

### C1 — General / Street

- [ ] Build the complete common template in normal `A` mode; do not recall an old Custom Mode as the source.
- [ ] Set physical controls to AFS, Single Shot and Function Lever MODE1.
- [ ] Set `A`, f/5.6, Auto ISO with `Min. Shtr Speed = 1/125s`, `225-Area`, Standard, Multi, RAW, 4:3, AWB, EFC and Stabilizer Normal.
- [ ] Attach and switch on the X3 while setting/verifying `Forced Flash Off`; confirm bracket and shutter delay Off and Constant Preview On. Remove the X3 after saving if desired.
- [ ] Immediately before saving, verify every stored common-template section and confirm both monitor and LVF Night Mode are Off.
- [ ] Save with `Setup > Cust.Set Mem. > C1`.

### C2 — Portrait / People

- [ ] Recall the newly completed C1 as the clean working source; never enter old C2 as the construction source.
- [ ] Set physical controls to AFS, Single Shot and Function Lever MODE1.
- [ ] Set `A`, f/2.8, Auto ISO with `Min. Shtr Speed = 1/125s`, `Human Detect AF`, Portrait, Multi, RAW, 4:3, AWB, EFC and Stabilizer Normal.
- [ ] Attach and switch on the X3 while setting/verifying `Forced Flash Off`; confirm bracket and shutter delay Off and Constant Preview On. Remove the X3 after saving if desired.
- [ ] Immediately before saving, verify every stored common-template section and confirm both monitor and LVF Night Mode are Off.
- [ ] Save with `Setup > Cust.Set Mem. > C2`.

### C3-1 — Single Macro — TTL

- [ ] Recall the newly completed C1 as the clean working source; never enter old C3-1 as the construction source.
- [ ] Set physical controls to MF, Single Shot and Function Lever MODE1.
- [ ] Set the X3 to Group A TTL ±0, Group B Off; place both diffused MF12 units in Group A.
- [ ] Set `M`, f/16, 1/200s, ISO 200, Natural, Multi, RAW, 4:3, AWB, `MSHTR`, Forced Flash On and Stabilizer Normal.
- [ ] Confirm bracket and shutter delay Off; Constant Preview Off; Peaking On/LOW/RED.
- [ ] Immediately before saving, verify every stored common-template section and confirm both monitor and LVF Night Mode are Off.
- [ ] Save with `Setup > Cust.Set Mem. > C3-1`.

### C3-3 — Supported Macro Focus Bracket

- [ ] Start from the completed C3-1 state; set physical controls to MF, Single Shot and Function Lever MODE2.
- [ ] Support the camera; keep X3 Group A TTL +0.0, Group B Off and both diffused MF12 units in Group A.
- [ ] Set `M`, f/8, 1/200s, ISO 400, Natural, Multi, RAW, 4:3, AWB, `MSHTR` and Stabilizer Off.
- [ ] Set Focus Bracket `Step 2 / 40 images / 0/+`; keep `Shutter Delay = Off` and Constant Preview Off.
- [ ] Immediately before saving, verify every stored common-template section and confirm both monitor and LVF Night Mode are Off.
- [ ] Save with `Setup > Cust.Set Mem. > C3-3`.

### C3-2 — Macro Burst — Manual Flash

- [ ] Recall the newly completed C3-1 again as the clean working source; never enter old C3-2 as the construction source.
- [ ] Program this last. Set physical controls to MF, Burst II and Function Lever MODE1.
- [ ] Set the X3 to Manual, Group A 1/32 and Group B Off; place both diffused MF12 units in Group A.
- [ ] Set `M`, f/16, 1/200s, ISO 400, Natural, Multi, RAW, 4:3, AWB, `MSHTR`, Forced Flash On and Stabilizer Normal.
- [ ] Confirm `Burst Shot 2 Setting = M`, bracket and shutter delay Off, Constant Preview Off and Peaking On/LOW/RED.
- [ ] Immediately before saving, verify every stored common-template section and confirm both monitor and LVF Night Mode are Off.
- [ ] Save with `Setup > Cust.Set Mem. > C3-2`.

## 5. Power-cycle and capture verification

- [ ] Power the camera Off and On before recall verification.
- [ ] C1: recall it with AFS/Single/MODE1; confirm the saved values, AF-ON focus, no shutter-half-press AF and a RAW written to both cards.
- [ ] C2: recall it with AFS/Single/MODE1; confirm a human face/eye receives the yellow detection frame and a RAW is written to both cards.
- [ ] C3-1: recall it with MF/Single/MODE1 and X3 TTL; make several individual frames and confirm consistent TTL exposure and a usable viewfinder.
- [ ] C3-2: recall it with MF/Burst II/MODE1 and Group A manual 1/32; make repeated 3–5-frame bursts and confirm every frame is illuminated. Expected measured cadence is approximately 2.6 fps.
- [ ] C3-3: recall it with MF/Single/MODE2 on support and X3 TTL; confirm 40 illuminated frames, usable near-to-far coverage and a successful Helicon Focus merge.
- [ ] For all five modes confirm RAW, 4:3, Multi, AWB, correct Stabilizer behavior and Backup Rec to both cards.
- [ ] If any saved value is wrong, correct and re-save only that mode, power-cycle, and repeat its recall/capture test.

## 6. Create and verify the final backup

Do this only after Section 5 passes.

- [ ] Insert the designated primary card in Slot 1.
- [ ] Check the camera's Save/Load file list and the project destination. If `G9PARKS.DAT` already exists in either place, **stop and preserve it; do not accept an overwrite prompt or use a forced copy**. Resolve the earlier run before choosing a separately documented versioned name; check the recovery-card destination when it is mounted below.
- [ ] Navigate to `Setup > Save/Restore Camera Setting > Save > New File`.
- [x] Save the final post-runbook state as a new file. Actual camera filename: `G9MK1SET.DAT`; no recovery file was overwritten.
- [ ] Mount the primary card and dedicated recovery card on the computer. Confirm again that neither `backup/G9/G9PARKS.DAT` nor a recovery-card `G9PARKS.DAT` exists.
- [x] Copy the card `G9MK1SET.DAT` to `backup/G9/G9MK1SET.DAT` without a force/overwrite option.
- [x] Record the final file size, modification time and SHA-256 below.

| File | Size | Modified | SHA-256 |
|---|---:|---|---|
| `backup/G9/G9MK1SET.DAT` | 10,957 bytes | 2026-08-20 10:18:06 | `EA7F23E8C96C12334E65620DC4B7AF40524B03742BB807E5A8E4AFAA2039EABE` |

The mounted card copy and project copy were verified identical by size and SHA-256 on 2026-08-20. If a separate second recovery card is added later, repeat the comparison with all three copies.

```powershell
$g9CardCopy = 'I:\AD_LUMIX\CAMSET\G9MK1SET.DAT'
$g9ProjectCopy = 'backup/G9/G9MK1SET.DAT'

Get-Item $g9CardCopy, $g9ProjectCopy |
  Select-Object FullName, Length, LastWriteTime
Get-FileHash -Algorithm SHA256 $g9CardCopy, $g9ProjectCopy
```

- [x] Stop on any missing copy, size difference or hash difference. The mounted card and project copy match.
- [ ] Safely eject both cards only after the three-way comparison passes.
- [ ] Reinsert the recovery card and confirm the preserved recovery entries plus `G9PARKS.DAT` appear under `Save/Restore Camera Setting > Load`; **do not execute Load**.
- [ ] Lock the recovery card's write-protect tab and label it `CAMERA RECOVERY — DO NOT FORMAT`.
- [ ] Photograph the final C1, C2, C3-1 through C3-3 summaries, Q.Menu, My Menu, Fn Button Set and Function Lever settings.
- [ ] Verify Panasonic Image App pairing separately because the DAT file does not preserve it; then leave camera Bluetooth Off for normal daytime use. After any future DAT restore, verify Image App pairing and `Auto Clock Set`; if either is absent, rebuild pairing using Menu Atlas §B4 before returning Bluetooth to Off.

## 7. Closure gate

- [ ] Sections 1–6 are complete with no unresolved failure.
- [ ] All five Custom Modes pass recall and capture verification after a power cycle.
- [ ] Both-card Backup Rec is proven with real RAW files.
- [x] `G9MK1SET.DAT` exists in the computer backup and on the mounted camera-settings card with matching SHA-256.
- [ ] The final hash and photographs are recorded.
- [x] Update `data/camera-config-g9mki.json` status from implementation-pending to implemented-and-validated.
- [ ] Mark the G9 MkI closure items complete in `Camera-Implementation-Checklist-US-Parks-2026.md`.

When every closure item is checked, the G9 MkI is complete. Remaining G9 MkII calibration or trip-preparation work does not reopen the G9 MkI configuration.
