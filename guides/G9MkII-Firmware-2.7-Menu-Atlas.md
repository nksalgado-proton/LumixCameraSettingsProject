# Lumix G9 MkII Firmware 2.7 — Menu Atlas and Programming Runbook

Status: Active implementation authority. Program only one presented screen at a time.

Camera: Panasonic Lumix DC-G9M2 (G9 MkII)

Camera menu language: English

Installed firmware: 2.7

Primary authority: Panasonic complete operating instructions `DC-G9M2_DVQP3010ZG_full_eng.pdf`, document revision `DVQP3010ZG` (`F0923KN6046`).

Firmware authority: Panasonic G9M2 firmware release record. Firmware 2.7, released 1 June 2026, is corrective and adds no new menu items. The `ZG` manual includes the menu additions from earlier firmware releases through 2.5 that remain present in 2.7.

Project targets: `Camera-Mode-Redesign-US-Parks-2026.md`, `Camera-Implementation-Checklist-US-Parks-2026.md`, and `field-cards.json`.

## 1. Purpose

This document is the navigation and implementation authority for the G9 MkII. Field Cards explain when and why to use each mode. This atlas states the exact English menu path and value used to program it.

During live implementation, present only one camera screen or one tightly bounded submenu at a time. The operator reports `Done` or the exact discrepancy. Do not continue until the visible result agrees with the table.

## 2. Stop rules

Stop immediately and do not select the closest-looking choice if:

- the item is absent;
- the English item name differs from this atlas;
- the displayed choices differ;
- the item is greyed out;
- one change forces an unexpected change elsewhere;
- the required physical-control position is different;
- a lens, drive mode, recording mode, card, flash, HDMI device, USB device, or Bluetooth state changes availability.

Record the complete screen or photograph it. Reconcile the actual firmware-2.7 camera against the `ZG` manual before proceeding.

## 3. Verified source and recovery checkpoint

| Authority or recovery item | Verified value |
|---|---|
| Manual file | `reference/manuals/DC-G9M2_DVQP3010ZG_full_eng.pdf` |
| Manual pages | 931 |
| Manual SHA-256 | `83E305911AC1A328A859F36E69A56BC4BE4567FCFDC85D7D8C10426AE099547B` |
| Body firmware | `2.7` |
| Pre-change backup | `backup/G9II/G2PRE.DAT` |
| Backup size | 8,959,329 bytes |
| Backup SHA-256 | `39C5CE389E4875C138D0A6172DEE8CC72B433A4B0CCCC6AEC92A2EC094A8A287` |
| Final programmed backup | `backup/G9II/G2FINAL.DAT` |
| Final backup size | 8,959,329 bytes |
| Final backup SHA-256 | `F97CA4B01BE2C78424DFB6F4EAF70EB786EEC08F1B94AA9D4D79B22C43C16B0E` |

Keep the backup directory private and excluded from publication and commits.

## 4. Lens firmware ledger

| Lens | Verified firmware | Decision |
|---|---:|---|
| Lumix G X Vario 12–35mm f/2.8 II | `1.3` | Current and verified. |
| Lumix G X Vario 35–100mm f/2.8 II | `1.3` | Current and verified. |
| M.Zuiko Digital ED 60mm f/2.8 Macro | `1.2` | Current and verified. |
| Leica DG Vario-Elmar 100–400mm II | Not yet received | Audit after collection in Denver on 1 September. The outgoing MkI lens is excluded. |

## 5. What the G9 MkII actually stores

Panasonic manual pages 553–556 and 781–802 are authoritative. In the manual tables, the `C` column explicitly identifies settings saved by `[Save to Custom Mode]`.

| Setting class | Stored in C1/C2/C3-n? | Programming consequence |
|---|---:|---|
| Recording mode, exposure baseline, white balance, ISO, AF mode and drive-related recording choices | Yes | Set and verify before saving every custom mode. |
| `[Photo]` menu | Yes | Build a common template, but verify all relevant exceptions before every save. |
| `[Video]` menu | Yes | Isolate it from stills and verify it explicitly for C3-10. |
| `[Custom]` menu | Yes | Common values, Fn assignments and Q.Menu layout must exist before the first custom-mode save. |
| `[Setup]` menu | No | Program once as the camera-wide Set and Forget layer. |
| `[Playback]` menu | No | Program once. |
| `My Menu` registrations and order | No | Program once. |
| Mode dial, drive dial and focus-mode lever | No | Field Cards must always state their physical positions. |
| Lens switches, filter, tripod, release cable and MIOPS state | No | Set in the field; the camera cannot recall them. |

Important consequence: the common template is not a separate hidden camera profile. It is a fully verified starting state held in a completed custom mode and copied with `[Load Custom Mode]`. Every target slot is saved only after its complete worksheet is checked.

## 6. Safe implementation order

1. Verify the recovery checkpoint, firmware and cards.
2. Program camera-wide `[Setup]` settings.
3. Program camera-wide `[Playback]` safety settings.
4. Program camera-wide `My Menu` registrations.
5. Set `[Setup] > [Setting] > [Custom Mode Settings]` behavior.
6. Build the common still-photo template: common `[Photo]` and `[Custom]` values, controls, Fn buttons and Q.Menu.
7. Save and power-cycle-test C1. C1 becomes the first verified construction anchor.
8. Derive C2 from C1 and verify all differences.
9. Derive C3-1, then use the closest completed mode for each later C3 slot.
10. Test each mode after power cycling, including physical controls and accessories.
11. Save a final camera-settings backup and synchronize the Field Cards with tested behavior.

No target slot is overwritten until its source state, destination slot and difference table have all been read back.

Implementation decision: do not load any pre-existing C3 slot as a construction source. Its stored values may have drifted since the earlier project inventory. Use only the newly programmed, power-cycle-verified C1 or another newly completed and verified target mode. The pre-change DAT file is the recovery path for the old configuration.

## 7. Programming-session protocol

1. Confirm battery above 60%, both cards inserted, menus in English and firmware 2.7.
2. State the required physical-control positions and attached equipment.
3. Present one exact screen table.
4. Change rows from top to bottom.
5. Report `Done` or the exact mismatch.
6. Re-open and read back every changed row when requested.
7. Save only after the complete worksheet passes.
8. Power-cycle and recall the saved mode before marking it complete.

| Action | Meaning |
|---|---|
| `SET` | Change to the target. |
| `VERIFY` | Read the displayed value; change it only if different. |
| `PRESERVE` | Do not alter it. |
| `SKIP` | Do not open or change it. |
| `UNAVAILABLE` | Stop; do not improvise. |

## 8. Stage A — known starting state

Use this neutral state before the camera-wide pass. It minimizes unavailable recording choices; Setup settings themselves do not depend on saving this state.

| Physical control or equipment | Required state |
|---|---|
| Mode dial | `A` |
| Drive dial | Single Shot |
| Focus-mode lever | `S` |
| Lens | 12–35mm II preferred |
| Flash trigger | Removed or switched off |
| HDMI, USB and remote cables | Disconnected |
| Card Slot 1 | Lexar 256 GB 2000x UHS-II V90 |
| Card Slot 2 | Matching Lexar 256 GB 2000x UHS-II V90 |

## 9. Stage B — camera-wide Setup settings

Manual paths and English names in this section are from pages 596–617 and the Setup menu index on pages 630–631.

### B1. Card backup recording

Navigation: `MENU/SET > Setup > Card/File > Double Card Slot Function`

| Screen row | Target | Action | Manual |
|---|---|---|---|
| `Recording Method` | `Backup Rec` | VERIFY | p. 598 |

The two matched 256 GB cards provide one effective 256 GB capacity with immediate duplication. Daily verified downloads make this acceptable.

### B2. USB-SSD

Navigation: `MENU/SET > Setup > Card/File > USB-SSD`

| Screen row | Target | Action | Manual |
|---|---|---|---|
| `USB-SSD` | `OFF` | SET | p. 598 |

### B3. Folder and file-name prefix

Navigation: `MENU/SET > Setup > Card/File > Folder / File Settings > File Name Setting`

| Screen row | Target | Action | Manual |
|---|---|---|---|
| `Folder Number Link` | Do not select | PRESERVE | pp. 599–600 |
| `User Setting` | Select | SET | p. 600 |
| Three-character user segment | `G2_` | SET | p. 600 |

Expected filename example: `PG2_0001.RW2`. The leading `P` is added by the camera and cannot be removed.

Do not use `[File Number Reset]` during programming. Perform it once immediately before the trip, after all capture tests are complete.

### B4. Copyright Information

Navigation: `MENU/SET > Setup > Card/File > Copyright Information`

| Screen row | Target | Action | Manual |
|---|---|---|---|
| `Artist` | `ON` | SET | p. 601 |
| `Artist > SET` | `NKS` | SET | p. 601 |
| `Copyright Holder` | `ON` | SET | p. 601 |
| `Copyright Holder > SET` | `Nelson Krahenbuhl Salgado` | SET | p. 601 |

### B5. Power Save Mode

Navigation: `MENU/SET > Setup > Monitor / Display > Power Save Mode`

| Screen row | Approved target | Action | Manual |
|---|---|---|---|
| `Sleep Mode` | `10MIN.` | SET | p. 602 |
| `Sleep Mode(Wi-Fi)` | `ON` | SET | p. 602 |
| `Auto LVF/Monitor Off` | `2MIN.` | SET | p. 602 |
| `Power Save LVF Shooting` | Preserve current state | PRESERVE | p. 602 |

These timings now match the tested G9 MkI preference. The user explicitly transferred the 10-minute Sleep and 2-minute display-off behavior to the G9 MkII during implementation.

### B6. Thermal Management

Navigation: `MENU/SET > Setup > Monitor / Display > Thermal Management`

| Screen row | Target | Action | Manual |
|---|---|---|---|
| `Recording Max Temperature` | `STANDARD` | SET | p. 603 |

`HIGH` permits a hotter body and is intended for extended recording with a tripod. It is not the normal handheld wildlife-video baseline.

### B7. Monitor and LVF frame rates

Navigation: `MENU/SET > Setup > Monitor / Display`

| Screen item | Target | Action | Manual |
|---|---|---|---|
| `Monitor Frame Rate` | `60fps` | SET | p. 603 |
| `LVF Frame Rate` | `120fps` | SET | p. 604 |

The 120 fps LVF target prioritizes wildlife response and consumes more battery.

### B8. Brightness and Eye Sensor

Navigation: `MENU/SET > Setup > Monitor / Display`

| Screen or submenu | Row | Target | Action | Manual |
|---|---|---|---|---|
| `Monitor Backlight` | displayed value | `AUTO` | SET | p. 605 |
| `LVF Luminance` | displayed value | `AUTO` | SET | p. 605 |
| `Eye Sensor` | `Sensitivity` | `LOW` | SET | p. 606 |
| `Eye Sensor` | `LVF/Monitor Switch` | `LVF/MON AUTO` | SET | p. 606 |

Do not run `[Level Gauge Adjust.]` unless an actual calibration test shows an error.

### B9. Beep

Navigation: `MENU/SET > Setup > IN/OUT > Beep`

| Screen row | Target | Action | Manual |
|---|---|---|---|
| `Beep Volume` | Off icon | SET | p. 607 |
| `AF Beep Volume` | Off icon | SET | p. 607 |
| `AF Beep Tone` | PRESERVE | PRESERVE | p. 607 |
| `E-Shutter Vol` | Off icon | SET | p. 607 |
| `E-Shutter Tone` | PRESERVE | PRESERVE | p. 607 |

The tone rows are irrelevant when their corresponding volumes are off.

### B10. Bluetooth operating target

Navigation: `MENU/SET > Setup > IN/OUT > Bluetooth`

| Screen item | Target | Action | Manual |
|---|---|---|---|
| `Bluetooth Function` | Paired; `OFF` during the day | VERIFY | pp. 608, 638–643; firmware 2.3 menu revision p. F-84 |
| `Remote Wakeup` | `OFF` | SET | pp. 608, 665 |
| `Auto Transfer` | `OFF` | SET | pp. 608, 661 |
| `Location Logging` | `OFF` | SET | pp. 608, 663 |
| `LUMIX Sync Settings > Auto Clock Set` | Preference `ON`; displayed as forced `OFF` while Bluetooth Function is off | VERIFY | pp. 667, 880; observed on firmware 2.7 |

The iPhone connection is managed with LUMIX Sync. Approved travel workflow: enable Bluetooth Function each morning, connect LUMIX Sync and visually verify clock synchronization; then turn Bluetooth Function off for the day. The camera was observed to restore the saved `Auto Clock Set = ON` preference when Bluetooth reconnects. Direct camera geolocation is not used; an iPhone reference photograph supplies location during post-processing. Wi-Fi remains inactive unless deliberately required.

### B11. USB and unused grip setting

Navigation: `MENU/SET > Setup > IN/OUT > USB`

| Screen row | Target | Action | Manual |
|---|---|---|---|
| `USB Mode` | `Select on connection` | SET | p. 609 |
| `USB Power Supply` | `ON` | SET | p. 609 |

`Battery Use Priority` is a separate `IN/OUT` item. The user has no battery grip; do not program it.

### B12. Network indicator

Navigation: `MENU/SET > Setup > IN/OUT > Network Connection Light`

| Screen row | Target | Action | Manual |
|---|---|---|---|
| `Network Connection Light` | `OFF` | SET | p. 611 |

### B13. Custom-mode behavior

Navigation: `MENU/SET > Setup > Setting > Custom Mode Settings`

| Screen row | Target | Action | Manual |
|---|---|---|---|
| `Limit No. of Custom Mode` | `10` | SET | pp. 554, 612 |
| `How to Reload Custom Mode` | `Turn the Power ON` | SET | p. 554 |
| `Select Loading Details > F / SS / ISO Sensitivity` | `ON` | SET | p. 554 |
| `Select Loading Details > White Balance` | `ON` | SET | p. 554 |

`Edit Title` is completed slot by slot. Titles accept at most 22 characters.

### B14. Save/Restore behavior

Navigation: `MENU/SET > Setup > Setting > Save/Restore Camera Setting`

| Screen row | Target | Action | Manual |
|---|---|---|---|
| `Keep Settings While Format` | `ON` | SET | p. 613 |

Do not select `Load`, `Delete`, or `Reset`. A new post-implementation `Save` is created only after all modes pass.

### B15. System Frequency

Navigation: `MENU/SET > Setup > Others > System Frequency`

| Screen row | Target | Action | Manual |
|---|---|---|---|
| `System Frequency` | `59.94Hz (NTSC)` | VERIFY | p. 615 |

Do not run `[Pixel Refresh]`, `[Sensor Cleaning]`, or `[Reset]` as part of programming.

## 10. Stage C — camera-wide Playback safety

### C1. Display rotation and sorting

Navigation: `MENU/SET > Playback > Playback Mode`

| Screen item | Target | Action | Manual |
|---|---|---|---|
| `Rotate Disp.` | `ON` | SET | p. 517 |
| `Picture Sort` | `DATE/TIME` | SET | p. 517 |
| `Magnify from AF Point` | `ON` | SET | p. 517 |

### C2. Deletion confirmation

Navigation: `MENU/SET > Playback > Others > Delete Confirmation`

| Screen row | Target | Action | Manual |
|---|---|---|---|
| `Delete Confirmation` | `“No” first` | SET | p. 525 |

## 11. Stage D — common saved template boundary

### D1. Camera-wide My Menu order

Register these exact items in order. `Rec Quality` is deliberately excluded, and the combined MkII `Burst Shot Setting` produces a final total of 17 entries.

| Slot | Exact item | Source menu |
|---:|---|---|
| 1 | `Save to Custom Mode` | `Setup > Setting` |
| 2 | `Bracketing` | `Photo > Others (Photo)` |
| 3 | `Burst Shot Setting` | `Photo > Others (Photo)` |
| 4 | `Shutter Delay` | `Photo > Others (Photo)` |
| 5 | `ISO Sensitivity (photo)` | `Photo > Image Quality` |
| 6 | `Long Exposure NR` | `Photo > Image Quality` |
| 7 | `Focus Peaking` | `Photo > Focus` |
| 8 | `Sensor Cleaning` | `Setup > Others` |
| 9 | `Card Format` | `Setup > Card/File` |
| 10 | `Fn Button Set` | `Custom > Operation` |
| 11 | `Time Lapse/Animation` | `Photo > Others (Photo)` |
| 12 | `Zebra Pattern` | `Custom > Monitor / Display (Video)` |
| 13 | `AF Custom Setting(Photo)` | `Photo > Focus` |
| 14 | `Focus Limiter` | `Photo > Focus` |
| 15 | `High Resolution Mode Setting` | `Photo > Image Quality` |
| 16 | `Live View Composite` | `Photo > Others (Photo)` |
| 17 | `Custom Mode Settings` | `Setup > Setting` |

After registration, use `My Menu > Edit My Menu > Sorting` only if the visible order differs from this table. Set `Display from My Menu` to `ON`.

### D2. Saved-template boundary

The following groups are not camera-wide even when their intended value is common. They must be programmed before C1 is saved and then inherited or rechecked in every custom mode:

- `[Photo] > [Image Quality]`
- `[Photo] > [Focus]`
- `[Photo] > [Others (Photo)]`
- `[Custom] > [Image Quality]`
- `[Custom] > [Focus/Shutter]`
- `[Custom] > [Operation]`, including Fn buttons and Q.Menu
- `[Custom] > [Monitor / Display (Photo)]`
- `[Custom] > [Lens / Others]`

The verified firmware-2.7 Photo Q.Menu order is:

| Position | Exact item |
|---:|---|
| 1 | `AF Detection Setting` |
| 2 | `Detecting Subject` |
| 3 | `Image Stabilizer` |
| 4 | `Metering Mode` |
| 5 | `Picture Quality` |
| 6 | `Shutter Type` |
| 7 | `Bracketing` |
| 8 | `AF Custom Setting(Photo)` |
| 9 | `Focus Peaking` |
| 10 | `Min. Shutter Speed` |
| 11 | `Photo Style` |
| 12 | `Aspect Ratio` |

`Burst Shot Setting` and `Shutter Delay` are not offered by the Q.Menu add-item screen and do not appear in the manual's exhaustive registration list on pp. 547–550. Both remain in My Menu.

### D3. Verified common Focus Peaking details

Navigation: `Photo > Focus > Focus Peaking > SET`

| Screen row | Target | Evidence |
|---|---|---|
| `Focus Peaking Sensitivity` | `-2` | Observed on the firmware-2.7 camera; most selective approved baseline. |
| `Display Color` | `RED` | Approved high-visibility color. |
| `Display During AFS` | `ON` | Observed and retained. |
| `Display During MF > While In Live View` | `ON` | Observed and retained. |
| `Display During MF > While Live View Is Enlarged` | `ON` | Observed and retained. |
| `Display During MF > When Shutter Is Pressed` | `ON` | Observed and retained. |

The common template and all custom modes were subsequently programmed and verified using this atlas.

## 12. Stage E — verified C3-10 video behavior

The following behavior was observed while building and capture-testing `C3-10 — Wildlife Video` on firmware 2.7:

| Menu or control | Verified behavior |
|---|---|
| Mode source | Build directly from Creative Video; a still custom mode cannot be loaded into Creative Video. |
| `Rec. File Format` | `MP4` |
| `Rec Quality` | `4K/10bit/100M/60p` |
| `Luminance Level` | `64-940` |
| `Sound Rec Quality` | Fixed to `48kHz/16bit` with MP4. |
| `Time Code` | Greyed out/unavailable with MP4. |
| `XLR Mic Adaptor Setting` | Not displayed when no XLR adaptor is attached; built-in microphone remains active. |
| `Zebra Pattern > SET` | Zebra 1 and Zebra 2 both offered a maximum of 95% with the active 4K 10-bit / 64-940 configuration. Save both at 95%; leave `Zebra Pattern = OFF` by default. |
| Recall test | `C3-10 — Wildlife Video` recalled Creative Video M, 1/125, f/6.3, Auto ISO, AFC Animal Eye/Body Tracking and the approved 4K60 quality. |
| Capture test | Short clip recorded and played correctly with audible internal-microphone sound, active audio meters, continuous AF and the red recording frame. |
