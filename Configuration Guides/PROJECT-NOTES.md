# Lumix G9 MkI + MkII — Unified Configuration Project

## Goal
Completely reconfigure both cameras with a **simplified, unified system** that prioritizes consistency over per-mode customization. The previous guides (Nov 2025) were thorough but too complex — different Q.Menu layouts per mode meant Nelson could never remember where things were.

## Core Principles

1. **Same Q.Menu layout for ALL modes** — no per-mode Q.Menu differences. One layout, always in the same position.
2. **Same My Menu for ALL modes** — one toolbox, always the same.
3. **"Set and forget" settings applied globally** — things like Focus Peaking ON, Zebra Pattern at Zebra 1, etc. are always on, not buried in a Q.Menu slot.
4. **Standardize C1, C2, C3-1, C3-2, C3-3 across BOTH cameras** — pick up either body and it feels the same.
5. **C3-4 through C3-10 are MkII only** — bonus capacity, not shared.

## Cameras
- Panasonic Lumix G9 Mark I (5 custom mode slots: C1, C2, C3-1, C3-2, C3-3)
- Panasonic Lumix G9 Mark II (12 custom mode slots: C1, C2, C3-1 through C3-10)

## Previous Configuration (Nov 2025)

### G9 MkI (9-page guide, 5 modes)
- C1: Landscape (A, AFS, f/8) — 12-slot Q.Menu hybrid (6 global + 6 specific)
- C2: Portrait (A, AFS, Face/Eye)
- C3-1: Wildlife/Action (M, AFC, 9fps burst)
- C3-2: Tripod/MF (M, MF, long exposure)
- C3-3: Cinema Video (Creative Video, MF)
- Custom Menu: Save to Custom Mode, Format Card, SS/Gain Operation, Fn Button Set, Sensor Cleaning, Shutter AF
- Set & Forget: Shutter AF OFF, Constant Preview ON, Auto Review OFF, AF+MF ON, MF Assist Lens/FocusRing, MF Assist Display PIP, Burst Shot H=9fps/M, Beep OFF

### G9 MkII (27-page guide, 12 modes)
- C1: Landscape (A, AFS, f/8)
- C2: Low Light Handheld (M, AFC, wide open, 1/60s, ISO AUTO 12800)
- C3-1: Birds Static (M, AFC, 14fps, Animal Detect)
- C3-2: Birds in Flight (M, AFC, 14fps, Animal Detect, AF Custom Set 3, IS Mode 2)
- C3-3: Birds Pre-Burst (M, AFC, SH-PRE 60fps, ES, Animal Detect)
- C3-4: Portrait (A, AFC, Human Detect)
- C3-5: Low Light Tripod (M, MF, Self-Timer, IS OFF, f/8, ISO 100, 10s)
- C3-6: Macro Stills (A, MF, f/9, ISO 400)
- C3-7: Serious Cinema (Creative Video, MF, C4K24p, V-Log, 180deg)
- C3-8: Slow Motion (S&Q, AFC, 120fps, E-Stab ON)
- C3-9: 4K Vlog (Creative Video, AFC, 4K30p, P mode, Natural, E-Stab ON)
- C3-10: Helicopter Flight (Creative Video, MF, 4K60p, SEC/ISO, 1/250-1/500s)
- Q.Menu: 4-row hybrid (rows 1-2 global template, rows 3-4 per-mode bespoke)
- Two global templates: "AF-Stills" and "Video"
- Set & Forget: Constant Preview ON, Auto Review OFF, Focus/Release Priority AFS:FOCUS/AFC:FOCUS, AF+MF ON, MF Assist FocusRing, MF Assist Display PIP, Burst Shot I:H(14fps MS)/II:SH-60pre, Auto Exposure in P/A/S/M ON, Live View Boost Stills OFF / Video AUTO, Beep OFF, Fn buttons FnX:AF-ON Near Shift / FnY:AF-ON Far Shift

### Macro Field Guides (in Portuguese)
- Dedicated guides for OM 90mm PRO, MC-20, 60mm OM, 30mm Lumix
- Per-lens/per-scenario tables (handheld vs tripod)
- Focus Bracketing settings: Step, shot count, ISO by magnification
- Godox V860III flash guide: power settings, recycling tips
- C3 mode breakdown: C3-1 Insects, C3-2 Reptiles, C3-3 No Flash

## What Was Wrong With the Old Setup
- Per-mode Q.Menu rows 3-4 meant items moved position depending on which mode was active
- Muscle memory failed because the same function (e.g., Zebra Pattern) was in different slots per mode
- Too many modes with subtle differences (e.g., C3-1 and C3-2 for birds had identical Q.Menu but different core settings — could have been one mode)
- The guides were reference documents, not workflow documents

## Related Projects
- `XLumixG9Settings/` — ABANDONED. Python tool for reading/writing CamSet DAT files (binary settings on SD card) and WiFi API probing. No documentation exists on the binary file structure, and reverse engineering proved impossible due to too many variables.
- `XmacroSettings/` — COMPLETED. "Macro Bracketing Calculator" — PyQt6 desktop app (also has a PWA version) that calculates focus bracketing parameters. Contains empirically calibrated step tables for every camera/lens/accessory combo:
  - G9 + 60mm bare, G9 + 90mm bare, G9 + 30mm bare
  - G9 II + 60mm bare, G9 II + 90mm bare, G9 II + 90mm + MC-20, G9 II + 30mm bare
  - Includes Godox V-860 III and AD100Pro flash recycle times
  - Calculates: effective magnification, effective aperture, DoF, optimal step, shot count, sequence time, diffraction warnings
  - This data is directly relevant for configuring the macro C-modes

---

## NEW CONFIGURATION (Work in Progress)

### Trip Context: Costa Rica — Mid-April 2026
- 2-week trip, wildlife + landscape + travel photography
- Test trip planned beforehand to validate the full setup
- Traveling with a group of people

### Camera Roles

**G9 Mark I — PRIMARY: Macro (Nelson) + General (Wife)**
- Main lens: Olympus M.Zuiko 60mm f/2.8 Macro
- Accessory: Raynox DCR-250 (diopter, for extra magnification)
- Flash: Godox AD100Pro + Godox X3 trigger + ML-CD15 diffuser (Nelson needs to learn X3 config)
- Secondary use: general photos, landscapes, group shots (with shared lenses)
- NOTE: Nelson's wife will carry this camera — modes must be simple and intuitive for a non-expert user

**G9 Mark II — PRIMARY: Wildlife / Birds**
- Main lens: Leica DG Vario-Elmar 100-400mm f/4-6.3
- External monitor: Atomos Shinobi II (for wildlife shooting)
- Secondary use: general photos, landscapes, group shots (with shared lenses)

### Shared Lenses (used on either body)
- Lumix G Vario 12-35mm f/2.8 Power OIS (wide-to-normal zoom)
- Lumix G Vario 35-100mm f/2.8 Power OIS (portrait/tele zoom)

### Support Gear
- Tripod (for macro stacking on G9, long exposure, etc.)
- Monopod (for 100-400mm wildlife on G9 II)

### Gear NOT on This Trip (but owned and used for other work)
- Olympus M.Zuiko 90mm f/3.5 Macro (used with G9 II for tripod macro / focus bracketing)
- OM MC-20 teleconverter (used with 90mm for 4:1 macro)
- Godox V860III + AK diffuser (used for tripod macro stacking)
- Panasonic 30mm f/2.8 Macro (status unknown)

### Additional Gear for Costa Rica (updated)
- Godox ML-CD15 diffuser (for the AD100Pro)

### Decisions Made So Far

#### Shared Modes (BOTH cameras: C1, C2, C3-1, C3-2, C3-3)
| Mode | Purpose | Notes |
|------|---------|-------|
| C1 | Street / General | Walk-around, everyday photos |
| C2 | Portrait / People | Group shots, posed/candid people |
| C3-1 | Landscape | Scenic, travel landscapes |
| C3-2 | Macro Handheld (Single Shot) | No stacking, no bracketing — quick single macro shots |
| C3-3 | Wildlife / Action | G9: AFC + burst for any moving subject (no animal detect). G9II: Birds/Wildlife with Animal Detect |

#### G9 MkII Only Modes (C3-4 through C3-10)
| Mode | Purpose | Gear / Notes |
|------|---------|-------------|
| C3-4 | Tripod Macro (Focus Bracketing) | 90mm + MC-20 + Godox V860III +/- AK diffuser. Focus bracketing ON. NOT for Costa Rica trip. |
| C3-5 | Birds with Crop Zoom | 100-400mm + Crop Zoom (fw 2.3). Extra reach without quality loss. |
| C3-6 | Landscape Tripod (Lightning) | Tripod + MIOPS trigger. Long exposure, possibly Bulb. For lightning storms. |
| C3-7 | Low Light Handheld | Wide open, high ISO, stabilizer ON. Indoor, night, dim forest canopy. |
| C3-8 | Birds in Flight (BIF) | AFC + high burst + fast shutter + Animal Detect. Panning IS mode. |
| C3-9 | Video — Travel / Documentary | Simple travel scenes, documenting the trip. GoPro 13 also available. |
| C3-10 | Video — Wildlife in Motion | Wildlife behavior, birds, animals moving. Longer focal lengths. |

#### Additional Gear (updated)
- MIOPS trigger (for lightning/long exposure)
- GoPro Hero 13 (also being carried for video)

**MODE MAP STATUS: CONFIRMED ✓**

### Unified Q.Menu Layout (12 slots) ✓ CONFIRMED

Philosophy: Only items with NO dedicated button. AF Mode, ISO, WB, and Exposure Comp have dedicated buttons — they stay OUT. G9 MkI has 15 slots available but we use only 12 for consistency with G9 II.

**G9 MkII (12 slots):**
| Slot | Item | Rationale |
|------|------|-----------|
| 1 | AF Detection Setting (ON/OFF) | Master switch for subject detection. Turn OFF for landscape/macro, ON for wildlife/portraits. |
| 2 | Detecting Subject | When detection is ON: select Human/Animal/Car/Airplane/Train. |
| 3 | Image Stabilizer | Toggle ON/OFF (tripod), Mode 1/2 (panning). No dedicated button. |
| 4 | Metering Mode | Switch Multi/Spot/Center. No dedicated button. |
| 5 | Picture Quality | RAW/RAW+JPEG/JPEG. No dedicated button. |
| 6 | Shutter Type | Mech/EFC/Electronic. No dedicated button. |
| 7 | Bracketing | Focus/Exposure bracket ON/OFF + configure. No dedicated button. |
| 8 | Flash Mode | ON/OFF, Forced, Slow Sync. No dedicated button. |
| 9 | Flash Adjust. | Flash exposure compensation. Critical for macro. |
| 10 | Min. Shutter Speed | Safety net for Auto ISO. No dedicated button. |
| 11 | Photo Style | Switch look (Standard/Vivid/Natural/Portrait). No dedicated button. |
| 12 | Aspect Ratio | Switch 4:3/3:2/16:9/1:1. No dedicated button. |

**G9 MkI (12 of 15 slots used):**
| Slot | Item | Rationale |
|------|------|-----------|
| 1 | Photo Style | No subject detection on G9. Wife can switch looks (Standard/Vivid/Portrait). |
| 2 | Image Stabilizer | Toggle ON/OFF (tripod), Mode 1/2 (panning). No dedicated button. |
| 3 | Metering Mode | Switch Multi/Spot/Center. No dedicated button. |
| 4 | Quality | RAW/RAW+JPEG/JPEG. No dedicated button. |
| 5 | Shutter Type | Mech/EFC/Electronic. No dedicated button. |
| 6 | Bracket | Focus/Exposure bracket ON/OFF + configure. No dedicated button. |
| 7 | Flash Mode | ON/OFF, Forced, Slow Sync. No dedicated button. |
| 8 | Flash Adjust. | Flash exposure compensation. Critical for macro. |
| 9 | Min. Shtr Speed | Safety net for Auto ISO. No dedicated button. |
| 10 | i.Dynamic | Shadow/highlight recovery for JPEGs. G9 keeps this since no detection slots needed. |
| 11 | Ex. Tele Conv. | Digital zoom 1.4x/2.0x for extra reach — practical for wife with 60mm. |
| 12 | Aspect Ratio | Switch 4:3/3:2/16:9/1:1. No dedicated button. |

**How to set Q.Menu on G9 MkI:**
1. [Custom] > [Operation] > [Q.MENU] → set to [CUSTOM]
2. Press [Q.MENU] button → press ▼ to select edit icon → [MENU/SET]
3. Select item from top row → [MENU/SET] → select slot in bottom row → [MENU/SET]
4. Repeat for all 12 slots. Leave remaining 3 slots empty.
5. Press [Return] when done.

### Unified My Menu ✓ CONFIRMED

Philosophy: Settings you need occasionally but don't want to dig through deep menus. Unlike Q.Menu (field controls changed between shots), My Menu is your maintenance toolbox and deep-configuration access.

Note: External SSD recording was considered for the trip but Samsung T9 4TB is too large for field use. Recording to SD cards only. USB-SSD stays OFF.

**G9 MkII (16 items):**
| # | Item | Why |
|---|------|-----|
| 1 | Save to Custom Mode | Re-save modes after tweaks. Most-used during configuration, occasional in field. |
| 2 | Card Format | Quick format between shooting days. |
| 3 | AF Custom Setting (Photo) | Switch tracking behavior Set 1-4. Critical when switching between bird types or erratic vs. predictable subjects. |
| 4 | Focus Limiter | Prevent lens hunting to infinity during macro. Set near/far range for 60mm or 90mm. |
| 5 | Burst Shot Setting | Change burst speed H/M/L or SH burst/pre-burst settings. |
| 6 | Bracket (detailed settings) | Configure Step size, Number of Photos, Type for Focus Bracketing. Q.Menu only gives ON/OFF. |
| 7 | Ex. Tele Conv. | Digital crop 1.4x/2.0x. |
| 8 | Long Exposure NR | Toggle ON/OFF. ON for single long shots, OFF for stacking. Buried deep in menus. |
| 9 | Fn Button Set | Reassign function buttons without deep menu diving. |
| 10 | Shutter Delay | Add 1-8s delay for tripod vibration avoidance. |
| 11 | Sensor Cleaning | Quick field maintenance. Costa Rica = dusty trails + lens changes = dust risk. |
| 12 | Focus Peaking SET | Adjust sensitivity/color. Switch RED to BLUE for red subjects, LOW to HIGH for video. |
| 13 | Zebra Pattern SET | Adjust threshold. Tighten to 95% for critical work, loosen to 109% for bright scenes. |
| 14 | Rec Quality | Change video resolution/framerate. Buried in Video > Image Format. |
| 15 | Sound Rec Level Adj. | Adjust mic input levels for video. Buried in Audio menu. |
| 16 | Time Lapse/Animation | Configure time lapse and stop motion settings. |
| 17 | ISO Sensitivity (photo) | Access Auto ISO Upper Limit setting. Buried 4 levels deep in menus. Different modes need different limits (6400 street, 12800 wildlife, 3200 landscape). Quick field adjustment. |
| 18 | Custom Mode Settings | Access Edit Title (rename modes), Limit No. of Custom Mode (3 vs 10), How to Reload Custom Mode. Essential for setup and maintenance. |

**G9 MkI (16 items):**
| # | Item | Why |
|---|------|-----|
| 1 | Cust.Set Mem. | Re-save modes after tweaks. |
| 2 | Format | Quick format between shooting days. |
| 3 | AF Custom Setting (Photo) | Switch tracking behavior Set 1-4. |
| 4 | Focus Limiter | Prevent lens hunting to infinity during macro. |
| 5 | Burst Shot Setting | Change burst speed H/M/L or SH settings. |
| 6 | Bracket (detailed settings) | Configure Step, Count, Type for Focus Bracketing. |
| 7 | Ex. Tele Conv. | Digital crop 1.4x/2.0x. |
| 8 | Long Shtr NR | Toggle ON/OFF for long exposures. |
| 9 | Fn Button Set | Reassign function buttons. |
| 10 | Shutter Delay | Tripod vibration avoidance delay. |
| 11 | Sensor Cleaning | Quick field maintenance. |
| 12 | Focus Peaking SET | Adjust peaking sensitivity/color. |
| 13 | Zebra Pattern SET | Adjust zebra threshold. |
| 14 | Rec Quality | Change motion picture quality. |
| 15 | Time Lapse/Animation | Configure time lapse and stop motion. |
| 16 | ISO Sensitivity (photo) | Access Auto ISO Upper Limit. Different modes need different limits. Quick field adjustment. |

**Note on Crop Zoom (G9 II only):** Not in My Menu — controlled via Fn4 button (toggle on/off) + rear dial (wheel up = zoom in, wheel down = zoom out).

### Fn Button Assignments — G9 MkII ✓ CONFIRMED

**Physical button locations (from manual diagram):**
- **(27) Fn1** = bottom-right of back panel (also Cancel/Delete button) — near your right thumb at rest
- **(44) Fn2** = front of camera, upper-right near grip (also AF-Point Scope button)
- **(46) Fn3** = front of camera, lower-right near grip (also Preview button)
- **(6) WB** = top panel, left cluster
- **(7) ISO** = top panel, left cluster
- **(8) +/-** = top panel, left cluster (Exposure Comp.)
- **(10) Video rec** = top panel, right side
- **(15) LVF** = back panel, top-left
- **(18) AF Mode** = back panel, top-right
- **(20) AF ON** = back panel, right side (thumb position)
- **(22) Q** = back panel, below joystick
- **(23) Fn9-Fn12** = cursor buttons around MENU/SET (▲=Fn9, ►=Fn10, ▼=Fn11, ◄=Fn12)
- **(21) Fn13-Fn17** = joystick directions + center
- **Fn4-Fn8** = touch icons on screen (tap [Fn] on screen to reveal)

**NOTE: Touch Tab is OFF** — Fn4 through Fn8 (touch icons) are not accessible. This prevents accidental tab touches in the field. Functions originally planned for Fn4-Fn8 are accessible via Q.Menu, My Menu, or per-mode settings.

**Button we are CHANGING (1 button only):**

| Button | Location | Assignment | Rationale |
|--------|----------|-----------|-----------|
| **Fn1** | Back panel, bottom-right (Cancel/Delete) | **AF-ON: Near Shift** | Tells AF to prioritize closer subjects. When camera grabs background instead of bird, press Fn1 and it locks onto the closer subject. Invaluable when shooting through foliage with the 100-400mm. Also useful for macro when AF can't decide between subject and background. Located at natural right-thumb rest position for quick access without moving hand. |

**NOTE on AF-ON: Far Shift:** Camera only allows one AF-ON shift variant assigned at a time — assigning Near Shift removes Far Shift from available pool. Near Shift kept as it's more commonly needed (foreground distractions). For shooting through fences/grass, use tight AF area (1-Area/Pinpoint) aimed at distant subject instead.

**Buttons we are KEEPING at defaults:**

| Button | Location | Default Assignment | Why keep |
|--------|----------|-------------------|----------|
| **Fn2** | Front, upper-right near grip | **AF-Point Scope** | Zooms into AF point for precise focus check. Press to magnify exactly where the camera is focusing. Essential for confirming eye-focus on birds and macro subjects. Quick press-and-release gives you instant confirmation without leaving the viewfinder. |
| **Fn3** | Front, lower-right near grip | **Preview** | Depth-of-field preview. Press and hold to stop down the lens to your set aperture — shows you exactly how much blur the background will have. Critical for portraits (is f/2.8 enough separation?) and landscape (is f/8 giving enough depth?). Release to return to full-brightness view. |
| **WB** | Top panel, left cluster | **White Balance** | Dedicated WB button. Press to enter WB adjustment, use dials to change. No reason to reassign. |
| **ISO** | Top panel, left cluster | **Sensitivity** | Dedicated ISO button. Press to enter ISO adjustment, use dials to change. No reason to reassign. |
| **+/-** | Top panel, left cluster | **Exposure Comp.** | Dedicated exposure compensation button. No reason to reassign. |
| **Video rec** | Top panel, right side | **Video Record** | Starts/stops video recording. Dedicated purpose. |
| **LVF** | Back panel, top-left | **LVF/Monitor Switch** | Toggles between viewfinder and rear monitor. Dedicated purpose. |
| **AF Mode** | Back panel, top-right | **AF Mode** | Switches AF mode (1-Area, Zone, Tracking, Full Area, Pinpoint). Dedicated purpose. |
| **AF ON** | Back panel, right thumb position | **AF-ON** | The backbone of back-button focus. Press to focus, release to lock. Dedicated purpose — never reassign. |
| **Q** | Back panel, below joystick | **Q.MENU** | Opens the Quick Menu. Dedicated purpose. |
| **Fn9-Fn12** | Cursor buttons (▲►▼◄) | **Auto-assigned by Crop Zoom** | When Crop Zoom is active, these automatically become zoom controls (▲=in, ▼=out, ◄►=step zoom). When Crop Zoom is off, they function as normal cursor navigation. No manual assignment needed. |
| **Fn13-Fn17** | Joystick directions + center | **D.FOCUS Movement** | Joystick moves the AF point — intuitive directional control. Keep as-is. |

### Fn Button Assignments — G9 MkI ✓ CONFIRMED

**Physical button locations (from manual diagram):**
- **(42) Fn1** = back panel, next to AF Mode button — also serves as AF Mode button
- **(52) Fn2** = back panel, near Delete/Cancel — also serves as Q.MENU button
- **(36) Fn3** = back panel, top-left — also serves as LVF button
- **(11) Fn4** = front of camera, right side near grip
- **(12) Fn5** = front of camera, right side — also serves as Preview button
- **(29) Function Lever** = back panel, left side (MODE1/MODE2 toggle)
- **(4) +/-** = top panel (Exposure Comp.)
- **(5) ISO** = top panel
- **(6) WB** = top panel
- **(2) Video rec** = top panel
- **(41) Fn11-Fn15** = joystick directions + center
- **(46) Fn16-Fn19** = cursor buttons
- **Fn6-Fn10** = touch icons on screen (require Touch Tab ON — we have it OFF, so not accessible)

**NOTE: Touch Tab is OFF** — Fn6 through Fn10 (touch icons) are not accessible. Same as G9 II.
**NOTE: G9 MkI has NO dedicated AF-ON button** — must assign AF-ON to an Fn button for back-button focus to work.
**NOTE: G9 MkI has NO dedicated Q button** — Fn2 serves as Q.MENU and must stay.

**Button we are CHANGING (1 button only):**

| Button | Location | Assignment | Rationale |
|--------|----------|-----------|-----------|
| **Fn1** | Back panel, next to AF Mode button (item 42) | **AF-ON** | **MANDATORY.** The G9 MkI has no dedicated AF-ON button. With Shutter AF set to OFF, the camera has no way to trigger autofocus without this assignment. This is the backbone of back-button focus on this body. Fn1 sits at natural thumb position next to AF Mode button. Wife tutorial: "the button next to the AF Mode button focuses." |

**Buttons we are KEEPING at defaults:**

| Button | Location | Default Assignment | Why keep |
|--------|----------|-------------------|----------|
| **Fn2** | Back panel (item 52) | **Q.MENU** | G9 MkI has NO dedicated Q button — Fn2 is the only way to access Quick Menu. Must keep. |
| **Fn3** | Back panel, top-left (item 36) | **LVF/Monitor Switch** | Toggles between viewfinder and rear monitor. Dedicated purpose. |
| **Fn4** | Front of camera, near grip (item 11) | **AF-Point Scope** | Zooms into AF point to check precise focus. Useful for confirming eye-focus on wildlife and macro. |
| **Fn5** | Front of camera (item 12) | **Preview** | Depth-of-field preview. Press and hold to see actual blur at your set aperture. |
| **Fn Lever** | Back panel, left side (item 29) | **Silent Mode** | Flip lever to MODE2 to toggle Silent Mode — disables all sounds, switches to electronic shutter. Great for wildlife approach. Physical lever is intuitive for wife. |
| **+/-** | Top panel (item 4) | **Exposure Comp.** | Dedicated purpose. |
| **ISO** | Top panel (item 5) | **Sensitivity** | Dedicated purpose. |
| **WB** | Top panel (item 6) | **White Balance** | Dedicated purpose. |
| **Video rec** | Top panel (item 2) | **Video Record** | Dedicated purpose. |
| **Fn11-Fn15** | Joystick (item 41) | **Off** (default) | Not assigned. Joystick used for AF point movement via D.FOCUS Movement. |
| **Fn16-Fn19** | Cursor buttons (item 46) | **Off** (default) | Not assigned. Cursor buttons used for menu navigation. |

### Set and Forget Settings — CONFIRMED

#### Batch 1: [Custom] > [Focus/Shutter] — AF Foundation ✓ (Applied to BOTH cameras)

| # | Setting | Value | Rationale |
|---|---------|-------|-----------|
| 1 | Shutter AF | **OFF** | Backbone of back-button focus. Shutter button only takes the photo, never focuses. Focus exclusively with AF-ON. Enables pre-focus and recompose without camera refocusing on shutter press. Essential for wildlife and macro. Wife needs quick tutorial: "back button focuses, top button shoots." |
| 2 | Focus/Shutter Priority | **AFS: FOCUS, AFC: FOCUS** | Camera refuses to fire until subject is sharp — no blurry shots from half-focused frames. AFC set to FOCUS prioritizes sharp frames over firing speed during bursts. For wildlife and macro, a sharp shot matters more than filling the buffer. If C3-8 (BIF) misses too many frames, can switch AFC to BALANCE for that one mode only. |
| 3 | AF+MF | **ON** | After camera locks focus with AF-ON, immediately fine-tune by turning focus ring — no need to flip physical switch to MF. Critical for macro (AF gets close, then nudge ring for precision) and wildlife (camera grabs branch instead of bird). |
| 4 | MF Assist | **Focus Ring: ON, Display: PIP** | Turning focus ring in MF auto-magnifies a portion of the frame. PIP shows magnified view as inset while keeping full composition visible — much better than FULL which takes over entire screen and loses framing. |
| 5 | MF Guide | **ON** | Small visual indicator showing which direction to turn focus ring. Helpful, never intrusive. No reason to disable. |
| 6 | Quick AF | **OFF** | Would make camera continuously adjust focus without pressing AF-ON — drains battery and fights back-button focus. Camera should focus only when commanded. |
| 7 | Eye Sensor AF | **OFF** | Would trigger AF when eye reaches viewfinder — fights back-button focus by focusing without command. |
| 8 | Half-Press Shutter | **OFF** | Would assign focus to half-press shutter — the exact opposite of back-button focus. Must stay OFF. |
| 9 | Looped Focus Frame | **OFF** | AF point wrapping from edge to edge is disorienting in practice — overshoot and get lost. Better to stop at edges. |

#### Batch 2: [Custom] > [Monitor / Display (Photo)] — What You See While Shooting ✓ (Applied to BOTH cameras)

| # | Setting | Value | Rationale |
|---|---------|-------|-----------|
| 10 | Constant Preview | **ON** | Shows actual exposure on screen in real time — what you see is what you get. Without it, Manual mode is a guessing game. Critical for M-mode (wildlife, lightning, macro). Fw 2.5 now works with flash attached. |
| 11 | Auto Review | **OFF** | Camera does NOT pop up the image after taking a shot. In burst/wildlife, review image blocks view of live subject. Press Play to review when you choose. Keeps you in the moment. |
| 12 | Live View Boost | **OFF** | Brightens screen artificially in dark scenes, but disables Constant Preview — lose real exposure feedback. Must stay OFF so Constant Preview works in all M-mode scenarios. (G9 II has separate video Live View Boost that can be AUTO for V-Log.) |
| 13 | Histogram | **OFF** | Clutters screen during shooting. Zebra Pattern provides real-time overexposure warnings directly on image — much more useful. Check histogram during playback if needed. |
| 14 | Photo Grid Line | **9-SECTION** | Classic rule-of-thirds grid. Subtle overlay for composition without distraction. Useful for landscapes, portraits, street — everything. Wife benefits too. |
| 15 | Level Gauge | **ON** | Small level indicator to prevent crooked horizons, especially in landscape mode. No downside. |
| 16 | Expo.Meter | **OFF** | With Constant Preview ON and Zebra Pattern available, exposure meter is redundant clutter. You can see exposure by looking at the screen. |
| 17 | Focal Length | **ON** | Shows current focal length on screen. Useful with zoom lenses (12-35mm, 35-100mm, 100-400mm). No clutter cost. |
| 18 | Blinking Highlights | **OFF** | Flashes blown-out areas during playback review. Auto Review is OFF so you won't see it. Zebra Pattern handles overexposure live, which is better. |
| 19 | I.S. Status Scope | **OFF** | Shows stabilizer working range. Mostly for troubleshooting IS issues — not needed day-to-day. |
| 20 | Luminance Spot Meter | **OFF** | Pro exposure tool showing exact luminance value of a point. Useful for cinema, overkill for stills. Can add to Q.Menu or My Menu for specific modes if wanted. |
| 21 | Framing Outline | **OFF** | Shows frame outline for visualizing crop — not needed for current workflow. |

#### Batch 3: [Custom] > [Monitor / Display (Video)] + Focus Peaking & Zebra — Always-On Tools ✓ (Applied to BOTH cameras unless noted)

| # | Setting | Value | Camera | Rationale |
|---|---------|-------|--------|-----------|
| 22 | Focus Peaking | **ON, LOW, RED, PEAKING & IMAGE** | Both | Always on. LOW sensitivity avoids false peaking on noisy textures. RED most visible against natural subjects (foliage, bark, sky). PEAKING & IMAGE overlays highlights on normal image so you keep composition. Primary focus confirmation for macro and MF fine-tuning. |
| 23 | Zebra Pattern | **ZEBRA1, ~100-105%** | Both | Always on. Diagonal stripes on blown-out areas. Threshold at 100-105% so it only triggers on truly clipped highlights — not just bright areas. Live overexposure alarm for blown skies, white feathers, flash hotspots in macro. |
| 24 | V-Log View Assist | **OFF** | G9 II only | Not shooting V-Log for this trip. Enable per-mode if cinema mode with V-Log is set up later. |
| 25 | HLG View Assist | **MODE2 / AUTO** (default) | G9 II only | Only relevant for HLG video, not in current plan. Leave at defaults. |
| 26 | Anamorphic Desqueeze | **OFF** | G9 II only | No anamorphic lenses owned. |
| 27 | Monochrome Live View | **OFF** | Both | Shows live view in B&W. Only for dedicated B&W video work. Off. |
| 28 | Center Marker | **OFF** | Both | Crosshair in center. With grid line already on, redundant clutter. |
| 29 | Safety Zone Marker | **OFF** | G9 II only | Broadcast-safe area markers. Only for broadcast production. |
| 30 | Frame Marker | **OFF** | G9 II only | Aspect ratio overlays for video. Not needed for travel/wildlife video. (G9 II fw 2.3 supports up to 3 simultaneous.) |
| 31 | WFM/Vector Scope | **OFF** | G9 II only | Waveform monitor and vectorscope — pro color grading tools. Overkill for current video use. |
| 32 | Video-Priority Display | **OFF** | G9 II only | Changes display to prioritize video info even in photo mode. Confusing. Off. |
| 33 | Red REC Frame Indicator | **ON** | Both | Red border around screen when recording video. Obvious "you're recording" confirmation. Prevents accidentally recording or forgetting to stop. |

#### Batch 4: [Custom] > [Image Quality] — Exposure & Color Behavior ✓ (Applied to BOTH cameras unless noted)

| # | Setting | Value | Camera | Rationale |
|---|---------|-------|--------|-----------|
| 34 | ISO Increments | **1/3 EV** | Both | Finer ISO control. 1 EV jumps (100→200→400) too coarse — overshoot. 1/3 EV steps (100→125→160→200) let you dial in exactly the right sensitivity. |
| 35 | Extended ISO | **OFF** | Both | ISO 50 is a "pull" — shoots at ISO 100 and overexposes by 1 stop, then pulls down. Loses highlight headroom. Not worth it. Keep native range. |
| 36 | Color Space | **sRGB** | Both | Universal standard for screens, phones, social media, printers. AdobeRGB gives wider gamut but JPEGs look dull on non-color-managed screens. RAW files contain full color data regardless. |
| 37 | Face Priority In Multi Metering | **ON** | Both | Camera biases exposure toward detected face in Multi metering. Prevents backlit portrait silhouettes. Helpful for Portrait and Street modes. No downside — meters normally if no face detected. |
| 38 | Exposure Comp. Reset | **OFF** | Both | Exposure compensation carries over when switching modes or powering off. Safer — if you dial in +1 EV for a bright scene, it stays until you change it. Resetting can cause forgotten corrections. |
| 39 | Auto Exposure in P/A/S/M | **ON** | Both | If you accidentally press red record button during stills (e.g., BIF), camera records video using auto P-mode exposure instead of your stills M-mode settings (which would give choppy dark video). Safety net. |
| 40 | AWB Lock Setting | **Sync Shutter: OFF, Fn Lock: ON** | Both | WB doesn't lock on shutter press — continuously adjusts (correct for back-button focus). Fn Button option ON allows manual AWB lock with function button when consistent color needed across a series. |
| 41 | CreativeVideo Combined Set. | **All sub-items set to Camera icon (combined/shared)** | G9 II only | F/SS/ISO/Expo, WB, Photo Style, Metering, AF Mode — each set to camera icon so video shares settings with photo. Since video modes (C3-9, C3-10) are saved as Custom Modes with their own settings, Combined keeps things predictable. Avoids hidden duplicate settings. |

#### Batch 5: [Custom] > [Operation] + [Setup] > [IN/OUT] — How the Camera Feels ✓ (Applied to BOTH cameras unless noted)

| # | Setting | Value | Camera | Rationale |
|---|---------|-------|--------|-----------|
| 42 | Q.MENU Layout Style | **MODE1 (12-item grid)** | Both | Fixed 12-slot grid — always the same positions. MODE2 is expandable list that changes size. MODE1 is the only option for "same layout, always, everywhere." |
| 43 | Touch Settings | **Screen: ON, Tab: OFF, AF: AF, Pad AF: OFF** | Both | Touch Screen ON for tap-to-set AF point. Touch Tab OFF prevents accidental menu tab changes (nose bumps). Touch AF "AF" (not "AF+AE") — sets focus point without shifting exposure. Touch Pad AF OFF prevents touchscreen moving AF point when eye at viewfinder (nose/cheek touch — extremely annoying in field). |
| 44 | Joystick Setting | **D.FOCUS Movement** | Both | Pressing joystick in a direction moves AF point that direction — intuitive and direct. Alternative "Direct Focus Movement" is less intuitive. |
| 45 | WB/ISO/Expo. Button | **AFTER PRESSING2** | Both | Press button once to enter adjustment, use dials to change, press again to confirm. Prevents accidental changes — deliberate action required. WHILE PRESSING requires holding button and turning dial simultaneously, awkward one-handed. |
| 46 | Beep | **All volumes OFF (0), E-Shutter Vol: OFF** | Both | Complete silence. No beep on focus confirmation, no fake shutter sound. Essential for wildlife (don't scare bird) and street (don't announce yourself). Visual confirmations (green AF box, focus peaking) are sufficient. |
| 47 | Assign REC to Shutter Button | **ON** | G9 II only | In Creative Video mode (C3-9, C3-10), shutter button starts/stops recording — same as red button. Two ways to start recording, handy for different grip positions. |
| 48 | Focus Switching for Vert/Hor | **OFF** | Both | Camera remembering separate AF points for horizontal/vertical is confusing — turn camera and AF point jumps unexpectedly. With back-button focus, AF point is placed deliberately each time. |
| 49 | AF/AE Lock Hold | **OFF** | Both | Lock only active while button is held (not toggle on/off). Release button, release lock. More intuitive, no surprises. |

#### Batch 6: [Custom] > [Lens / Others] + [Setup] — Camera Infrastructure ✓ (Applied to BOTH cameras unless noted)

| # | Setting | Value | Camera | Rationale |
|---|---------|-------|--------|-----------|
| 50 | Lens Focus Resume | **OFF** | Both | Camera remembering yesterday's focus position is unreliable with back-button focus. Want fresh start each time. Always focus fresh with AF-ON. |
| 51 | Focus Ring Control | **NON-LINEAR** | Both | Slow ring turn = fine adjustment, fast turn = big jump (like mouse acceleration). Natural and intuitive for general use. LINEAR preferred for cinema focus pulls, not a priority here. |
| 52 | Aperture Ring Increment | **1/3 EV** | Both | Matches ISO increments. Fine control. Consistent steps on lenses with aperture rings (e.g., Leica 100-400mm). |
| 53 | Lens Fn Button Setting | **AF-ON** | G9 II only | 100-400mm has barrel Fn button. AF-ON gives another way to trigger focus when thumb can't reach back AF-ON while supporting big lens on monopod. |
| 54 | Lens Info. Confirmation | **ON** | Both | Shows lens info on attach. Quick confirmation camera recognizes lens. No downside. |
| 55 | Vertical Position Info (Video) | **ON** | G9 II only | Records camera orientation during video for auto-rotate in playback software. No reason to disable. |
| 56 | Power Save Mode | **Sleep: 2MIN, Monitor Off: 1MIN** | Both | Battery life matters on 2-week trip. 1MIN monitor off saves battery without being annoying. 2MIN sleep balances recompose time vs. battery drain. |
| 57 | Thermal Management | **STANDARD** | G9 II only | HIGH allows camera to get hotter before shutdown but Costa Rica is already hot — risks overheating damage in tropical heat. STANDARD is safer. |
| 58 | Monitor Frame Rate | **60fps** | Both | Smooth live view. 30fps saves battery but looks choppy tracking moving subjects. 60fps worth the battery cost. |
| 59 | LVF Frame Rate | **120fps** | G9 II only | Viewfinder at 120fps noticeably smoother for tracking BIF and fast action. Battery cost worth it for wildlife-primary body. |
| 60 | Eye Sensor Sensitivity | **LOW** | Both | Less jumpy switching between monitor and viewfinder. Prevents camera switching to viewfinder when nose or hand gets close. HIGH causes annoying constant switching in field. |
| 61 | Network Connection Light | **OFF** | Both | Blinking LED for Wi-Fi/Bluetooth is distracting in field and can spook wildlife at close range. |

#### Batch 7: [Setup] + Remaining — Defaults & Infrastructure ✓ (Applied to BOTH cameras unless noted)

| # | Setting | Value | Camera | Rationale |
|---|---------|-------|--------|-----------|
| 62 | Card Format | **Action only** | Both | No persistent setting — format as needed. |
| 63 | Double Card Slot Function | **Relay** (default) | Both | Card 2 takes over when Card 1 fills. Relay is right for a trip (Backup would halve storage). |
| 64 | USB-SSD | **OFF** (default) | G9 II only | No external SSD recording planned. |
| 65 | Folder/File Settings | **Folder Number Link** (default) | Both | Standard file numbering. |
| 66 | File Number Reset | **Action only** | Both | No persistent setting. |
| 67 | Copyright Information | **ON — set Nelson's name** | Both | Free EXIF metadata — every photo has your name embedded. Costs nothing, helps if images shared without credit. |
| 68 | Monitor Settings/Viewfinder | **Factory defaults** | Both | Leave at factory calibration unless color issues noticed. |
| 69 | Monitor Backlight/LVF Luminance | **AUTO** (default) | Both | Auto-adjusts brightness. |
| 70 | Level Gauge Adjust. | **Action only** | Both | Calibrate if level gauge seems off. |
| 71 | USB Mode | **Select on connection** (default) | Both | Asks what to do each time USB plugged in. |
| 72 | USB Power Supply | **ON** (default) | Both | Camera charges via USB when plugged in. |
| 73 | Battery Use Priority | **BG** (default) | Both | Use grip battery first (if grip attached). |
| 74 | HDMI Connection | **Output Resolution: AUTO** (default) | Both | For Shinobi II, AUTO negotiates the right resolution. |
| 75 | Clock Set | **Already set** | Both | — |
| 76 | Time Zone | **Set for Costa Rica on arrival (UTC-6)** | Both | No DST in Costa Rica. |
| 77 | System Frequency | **59.94Hz NTSC** | Both | Correct for the Americas. |
| 78 | Language | **Already set** | Both | — |
| 79 | Pixel Refresh | **Periodic maintenance** | Both | Run occasionally. |
| 80 | Sensor Cleaning | **Periodic maintenance** | Both | Run on power-on or manually. |

**TOTAL "SET AND FORGET" SETTINGS: 80 — ALL CONFIRMED ✓**

### How to Use Back-Button Focus — Quick Tutorial

**IMPORTANT: On both cameras, the top shutter button does NOT focus. It only takes the picture. You must focus first with the back button, then shoot.**

#### G9 MkI (Wife's camera)

**The two buttons you need:**
1. **Fn1** (back of camera, next to the AF Mode button, pressed with your right thumb) = **FOCUS**
2. **Shutter button** (top of camera, pressed with your right index finger) = **TAKE PICTURE**

**Basic workflow — 3 steps:**
1. **Point** the camera at your subject
2. **Press and hold Fn1** with your thumb — the camera focuses (you'll see a green box when it locks)
3. **Press the shutter button** with your index finger — the picture is taken

**For a still subject (landscape, posed portrait, flower):**
1. Press Fn1 once to focus → green box appears → release Fn1
2. Recompose your shot if needed (focus stays locked)
3. Press shutter button whenever ready — focus won't change

**For a moving subject (bird, kids running, action):**
1. Press and HOLD Fn1 — the camera continuously tracks the subject as long as you hold it (in AFC mode)
2. While still holding Fn1, press the shutter button to take the picture
3. Keep holding Fn1 to keep tracking, press shutter again for more shots

**If the picture is blurry:** You forgot to press Fn1 first. The shutter button alone will NOT focus. Always focus first, then shoot.

#### G9 MkII (Nelson's camera)

**The two buttons you need:**
1. **AF-ON** (back of camera, right side where your thumb naturally rests) = **FOCUS**
2. **Shutter button** (top of camera) = **TAKE PICTURE**

**The workflow is identical to the G9 MkI above** — just use AF-ON instead of Fn1.

**Extra button — Fn1 (Cancel/Delete button, bottom-right of back panel):**
This is **AF-ON: Near Shift**. It works like AF-ON but tells the camera to prefer closer subjects. Use when the camera is focusing on the background instead of your subject (e.g., a bird behind branches). Press Fn1 instead of AF-ON and it will lock onto the closer thing.

#### Why is this better than the normal way (half-press shutter to focus)?

With a normal camera setup, you half-press the shutter button to focus, then fully press to take the picture. This seems simpler but causes real problems:

**Problem 1: "Focus-Recompose" ruins your shot**
You want to photograph a person standing on the right side of the frame. The AF point is in the center. With half-press, you aim at the person's eye, half-press to focus, then move the camera to recompose with the person on the right. But the moment you fully press the shutter, the camera refocuses — on the background behind where the person was. Your subject is now blurry.
*With back-button focus:* Press AF-ON/Fn1 to focus on the eye, release the button, recompose freely, press the shutter whenever you want. The camera does NOT refocus. The eye stays sharp.

**Problem 2: Burst shooting refocuses every frame**
You're photographing a bird on a branch. With half-press, every time the shutter fires during a burst, the camera refocuses. If the bird moves slightly, the camera might grab the branch behind it on some frames. You lose shots.
*With back-button focus:* Press AF-ON/Fn1 once to lock on the bird's eye, release, then fire a burst. Every frame uses the same focus point. Or hold AF-ON/Fn1 for continuous tracking — you choose.

**Problem 3: You can't switch between single and continuous focus quickly**
With half-press, you need to physically flip the focus switch between AFS and AFC depending on whether your subject is still or moving. Miss a moving bird because you were in AFS? Too late.
*With back-button focus:* Tap AF-ON/Fn1 briefly = single focus (like AFS). Hold AF-ON/Fn1 = continuous tracking (like AFC). Same button, same mode — your thumb decides, not a switch. No mode change needed.

**Problem 4: Macro focus hunting**
You're doing macro photography. With half-press, the camera hunts for focus every time you try to take the shot — the lens zooms in and out searching. If you barely miss focus, it starts hunting all over again.
*With back-button focus:* Press AF-ON/Fn1 to get close to focus, release, then fine-tune with the focus ring (AF+MF is ON). Press shutter when ready — the camera does not hunt again. You're in full control.

**Problem 5: Waiting for the right moment**
A bird is perched. You want to wait for it to spread its wings. With half-press, you have to keep the button half-pressed the entire time you're waiting — your finger cramps, and if you accidentally release or fully press, you lose the moment.
*With back-button focus:* Press AF-ON/Fn1 to focus on the bird, release. Now wait as long as you need with no buttons pressed. The moment the wings open, fully press the shutter. Clean, relaxed, no cramping.

**In short:** Back-button focus separates "thinking" (focusing) from "acting" (shooting). You always decide when each happens, independently. It takes one day to get used to, and then you'll never go back.

### Per-Mode Core Settings

**IMPORTANT WORKFLOW NOTE:** Q.Menu settings are saved per Custom Mode. We saved C1 (with correct Q.Menu + C1 settings) to ALL mode slots as a baseline. When configuring each subsequent mode, we only change the per-mode differences and re-save. The Q.Menu carries forward unchanged.

#### C1: Street/General ✓ SAVED TO BOTH CAMERAS

**Purpose:** Walk-around, everyday mode. Parks, markets, towns, beaches, sightseeing. As automatic and forgiving as possible. Wife's primary mode on G9 MkI.

**Target lenses:** Lumix 12-35mm f/2.8 (primary) or Lumix 35-100mm f/2.8

**Physical controls (set before saving):**
| Control | Setting | Rationale |
|---------|---------|-----------|
| Mode dial | **A** (when saving) | Aperture Priority saved into C1 |
| Focus mode lever | **AFS** | Focus once, lock. Precise for static subjects, saves battery. |
| Drive mode dial | **Single Shot (S)** | One press, one photo. Deliberate shooting. |

**Lens settings (not saved — set on lens):**
| Lens | O.I.S. switch | Setting |
|------|--------------|---------|
| Lumix 12-35mm f/2.8 | O.I.S. | **ON** |
| Lumix 35-100mm f/2.8 | O.I.S. | **ON** |

**Menu settings:**
| Setting | Value | Rationale |
|---------|-------|-----------|
| Exposure Mode | **A (Aperture Priority)** | Camera handles shutter speed. |
| Aperture | **f/5.6** | Sharp across MFT frame. Adjust with front dial. |
| ISO | **AUTO (Upper Limit: 6400)** | 6400 keeps noise acceptable. Bright Costa Rica = ISO 200-400 most of the time. |
| Min. Shutter Speed | **AUTO** | Camera auto-selects based on focal length. Safe with IS on. |
| AF Mode | **1-Area+** (G9 II) / **1-Area** (G9) | Medium focus box on subject. "+" adds supplementary points. |
| AF Detection Setting | **OFF** (G9 II only) | No face detection for general street. Toggle ON via Q.Menu when needed. |
| Metering Mode | **Multi (Matrix)** | Best all-around for unpredictable scenes. |
| White Balance | **AWB** | Adapts to mixed lighting — daylight, shade, indoor. |
| Photo Style | **Standard** | Neutral, balanced. Good for JPEG and RAW. |
| Picture Quality | **RAW+FINE** | RAW for post-processing + JPEG for quick sharing. |
| Aspect Ratio | **3:2** | Classic photography ratio. |
| Image Stabilizer | **ON, Mode 1 (Normal)** | Standard stabilization for handheld. |
| Shutter Type | **EFC** | Eliminates first curtain vibration. No rolling shutter at street speeds. |
| Flash Mode | **Forced Flash OFF** | No flash for street. Toggle via Q.Menu. |
| Bracketing | **OFF** | Not needed. |
| i.Dynamic Range | **AUTO** (G9) / **OFF** (G9 II) | G9: helps wife's JPEGs. G9 II: OFF, shoot RAW. |
| Silent Mode | **OFF** | EFC is quiet enough. Function Lever (G9) for full silence. |

### Deliverables
- [ ] Printable configuration guide (PDF or DOCX)
- [ ] Field reference card (compact, for the bag)
- [ ] Step-by-step setup walkthrough
