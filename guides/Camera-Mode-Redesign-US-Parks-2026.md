# Camera Mode Redesign — US National Parks 2026

Date started: 2026-08-07

Status: Approved decision and migration ledger. G9 MkI macro tests are complete and its final target JSON is updated; final camera programming, recall verification and `G9PARKS.DAT` backup remain pending. G9 MkII work remains independent.

## Operating Model

- Both cameras will be carried.
- C1 and C2 remain identical on both bodies to preserve muscle memory.
- Camera-specific specialization begins at C3.
- The G9 MkI normally carries the 12–35mm or 35–100mm in the parks and switches to the Olympus 60mm Macro when a macro opportunity appears.
- The G9 MkII normally carries the Leica 100–400mm II so unexpected wildlife can be photographed without a lens change.
- Landscape photography is concentrated on the G9 MkII because it allows preparation and a deliberate lens change.

## Confirmed Target Mode Map

| Slot | G9 MkI | G9 MkII |
|---|---|---|
| C1 | General / Street | General / Street |
| C2 | Portrait | Portrait |
| C3-1 | Single Macro — TTL | Stationary / Slow Wildlife |
| C3-2 | Macro Burst — Manual Flash | Wildlife Action |
| C3-3 | Supported Macro Focus Bracket | Fast / Erratic Wildlife |
| C3-4 | — | Standard Handheld Landscape |
| C3-5 | — | Tripod HDR Landscape |
| C3-6 | — | Focus-Bracketed Landscape |
| C3-7 | — | Long Exposure Landscape with ND |
| C3-8 | — | Lightning — MIOPS Smart+ |
| C3-9 | — | Tripod High Resolution Landscape |
| C3-10 | — | Wildlife Video |

### In-Camera Display Titles

The approved names above remain the conceptual names used by this project. The G9 MkII `Edit Title` field accepts a maximum of 22 characters, so implementation uses the following concise English display titles:

| Slot | In-camera title |
|---|---|
| C1 | `General / Street` |
| C2 | `Portrait` |
| C3-1 | `Wildlife Still` |
| C3-2 | `Wildlife Action` |
| C3-3 | `Wildlife Fast` |
| C3-4 | `Landscape Handheld` |
| C3-5 | `Landscape T HDR` |
| C3-6 | `Landscape T Focus` |
| C3-7 | `Landscape T ND` |
| C3-8 | `Lightning T MIOPS` |
| C3-9 | `Landscape T HiRes` |
| C3-10 | `Wildlife Video` |

The G9 MkI mode labels remain conceptual labels in the project documentation unless the camera presents an editable title field. Do not infer or force a naming function that is not shown by the camera.

## Implementation Rule

For every target mode:

1. Identify the current mode used as its baseline.
2. Preserve every existing setting unless a change is explicitly approved.
3. Record each approved change as `current → target` with its reason.
4. Change the JSON configuration only after the full migration ledger has been reviewed.

## Global Set-and-Forget Review

Status: global targets reviewed and approved. The G9 MkI target record is updated; physical implementation remains governed by its final runbook.

### Storage and Double-Card Recording

Status: approved.

Approved changes:

| Camera | Current | Target | Main cards | Reason |
|---|---|---|---|---|
| G9 MkI | Relay Recording | Backup Recording | 2x Lexar 256 GB 1667x UHS-II V60 | Record every file simultaneously to two matched cards. The planned macro, portrait and general-use workload does not require the faster 64 GB V90 pair, while 256 GB provides substantially more daily capacity. |
| G9 MkII | Relay Recording | Backup Recording | 2x Lexar 256 GB 2000x UHS-II V90, up to 300 MB/s read and 260 MB/s write | Record every file simultaneously to two matched high-speed cards, including wildlife bursts and 4K60 100 Mbps video. |

Accepted tradeoff:

- Effective capacity is 256 GB per camera rather than the combined 512 GB available with Relay Recording.
- This is accepted because both cards will be emptied every day, making it unlikely that either camera will record 256 GB in one day.
- Immediate in-camera redundancy is more valuable for the trip than the additional capacity provided by Relay Recording.

Card operating procedure:

- Carry the 2x Lexar 64 GB 2000x UHS-II V90 cards as formatted emergency reserves.
- Format cards in their respective camera before the trip and after the daily files have been copied and verified.
- Before leaving each morning, confirm that both cards are present, recognized, empty and that Backup Recording remains active.
- During use, monitor both card-status indicators; a full, slow or failed card can interrupt simultaneous recording.
- Relay Recording remains an emergency field option only if available space becomes unexpectedly insufficient.

### File Identification, Numbering, Clock and Copyright

Status: approved.

Approved configuration:

| Setting | G9 MkI | G9 MkII | Reason |
|---|---|---|---|
| User-defined file-name characters | `G1_` | `G2_` | Distinguish files produced by the two cameras and avoid name collisions during import. |
| Example RAW file name | `PG1_0001.RW2` | `PG2_0001.RW2` | Panasonic adds the color-space character before the three user-defined characters. |
| File numbering | Continuous | Continuous | Preserve chronological continuity; do not restart numbering during daily downloads. |
| File Number Reset | Perform once immediately before the trip, after testing is complete | Perform once immediately before the trip, after testing is complete | Begin the trip with a clean sequence while minimizing duplicate names. |
| Folder / File behavior | Preserve standard folder-number-linked behavior | Preserve standard folder-number-linked behavior | Avoid unnecessary manual folder management during the trip. |
| Copyright Information | Not available on G9 MkI firmware 2.7 | Verify on G9 MkII with its firmware-2.7 Menu Atlas | The G9 MkI manual contains no Copyright Information, Artist or Copyright Holder menu items. |
| Artist | `NKS` | `NKS` | Exact approved text. |
| Copyright Holder | `Nelson Krahenbuhl Salgado` | `Nelson Krahenbuhl Salgado` | Exact approved text; no fixed year so the persistent setting does not become obsolete. |

Clock and time-zone procedure:

- Use the smartphone as the normal clock and time-zone reference through Bluetooth Auto Clock Set on both cameras.
- Use local time at the photographic location and verify that the smartphone has selected the correct local time zone, especially when cellular service is unavailable.
- At the start of each day and after crossing a time-zone boundary, power both cameras on, allow the Bluetooth connection to establish, and verify that their displayed times agree.
- If either camera does not connect, synchronize it manually before photographing. Do not knowingly leave only one camera on a different time reference, because synchronized capture times are required to sort the combined trip photographs correctly.

### Global Photo Quality and In-Camera Processing

Status: approved.

Preserve on both cameras:

- RAW picture quality.
- 4:3 aspect ratio and large picture size.
- Standard Photo Style as the global baseline; custom modes retain their approved overrides.
- Multi metering.
- sRGB color space metadata.
- i.Resolution off.
- Long Exposure Noise Reduction off.
- Extended ISO off.
- ISO increments at 1/3 EV.
- Auto ISO with the current no-upper-limit behavior. Fixed-ISO macro and landscape modes retain their explicit overrides.
- Electronic Front Curtain as the global shutter baseline; custom modes retain their approved Mechanical or Electronic overrides.

Approved G9 MkI changes:

| Setting | Current | Target | Reason |
|---|---|---|---|
| i.Dynamic | Auto | Off | Avoid automatic contrast and exposure intervention in the RAW-centered workflow and align the two cameras. |
| Shading Compensation | On | Off | Avoid automatic corner brightening and possible increased corner noise; apply lens correction during RAW processing when desired. |
| Diffraction Compensation | AUTO | OFF | The G9 MkI exposes `AUTO/OFF`, not `ON/OFF`; disable automatic processing and possible increased high-ISO noise. |

Clarification:

- Turning Diffraction Compensation off does not change the approved f/16 aperture in the G9 MkI macro modes.
- The accepted f/16 depth-of-field versus diffraction tradeoff remains. In-camera compensation cannot restore all optical detail lost to diffraction in the RAW capture.
- The G9 MkII already has i.Dynamic Range, Vignetting Compensation and Diffraction Compensation off for still photography, so no equivalent change is required.

### Global Autofocus and Shutter Behavior

Status: approved.

Preserve on both cameras:

- Back-button focus: Shutter AF off and AF-ON assigned to the rear control.
- G9 MkI `Half Press Release` off; verify the G9 MkII exact equivalent in its Menu Atlas.
- G9 MkI `Focus/Release Priority > AFS/AFF` set to Focus; verify the G9 MkII exact equivalent in its Menu Atlas.
- AF+MF on.
- MF Assist enabled with PIP display and MF Guide on.
- G9 MkI `Peaking` on, red and Low `Detect Level`.
- G9 MkII `Focus Peaking` on with sensitivity `-2`, red display, `Display During AFS` on, and all three `Display During MF` conditions on. These exact firmware-2.7 values were observed during implementation.
- Quick AF and Eye Sensor AF off.
- Global Focus Limiter off.
- AF Micro Adjustment off.
- AF Custom Setting (Photo) Set 1.
- Focus Switching for Vertical/Horizontal off.
- G9 MkI `Loop Movement Focus Frame` off; verify the G9 MkII exact equivalent in its Menu Atlas.

Approved changes:

| Camera | Setting | Current | Target | Reason |
|---|---|---|---|---|
| G9 MkII | AFC Focus/Shutter Priority | Focus | Balance | Reduce burst interruptions caused by strict focus confirmation while retaining more focus discipline than Release priority. |
| G9 MkII | AF Assist Light | On | Off | Its short effective range does not help normal 100–400mm wildlife distances and the light may attract or disturb animals. |
| G9 MkI | Lens Position Resume | Off | On | Exact firmware-2.7 name; preserve the last focus distance across power cycles. |
| G9 MkII | Lens-position-resume equivalent | Off | On | Exact name and path must come from the G9 MkII firmware-2.7 Menu Atlas. |
| G9 MkI | Burst Shot 1 Setting | H | L | Retain as the ISO 200/slower alternative; measured 15 frames in 10 seconds with flash. |
| G9 MkI | Burst Shot 2 Setting | `SH75` (invalid copied setting) | M | Tested C3-2 baseline; measured 26 consistently illuminated frames in 10 seconds with Manual 1/32. SH75 does not exist on the G9 MkI. |
| G9 MkII | Burst Shot 1 | H | H | Preserve the normal wildlife burst used by C3-1 through C3-3. |
| G9 MkII | Burst Shot 2 | SH75 | SH20 PRE | Provide an AFC-compatible pre-capture option for sudden wildlife action. |
| G9 MkII | SH Pre-Burst Recording Time | Not recorded | 0.5 second | Save ten frames before the full shutter press at SH20, providing reaction-time protection without the volume of a one-second pre-burst. |

Operational clarification:

- G9 MkII Burst II / SH20 PRE is an emergency field alternative for takeoffs, jumps and other unpredictable instants. It does not replace Burst I / H in the approved wildlife custom modes.
- SH20 PRE forces the Electronic shutter and creates substantial image volume, so it should be used selectively.

### Global Monitor, Viewfinder and Exposure Aids

Status: approved.

Preserve on both cameras:

- Auto Review off, avoiding interruptions after normal frames and bursts.
- 9-section / 3x3 photo grid on.
- Show the level gauge with `DISP.` when desired; there is no persistent On/Off preference.
- Do not use `Focal Length Set` with the planned communicating electronic lenses.
- Live View Boost off as the normal baseline; activate only when required for a dark night scene.
- Night Mode off.
- Exposure Meter display off.
- Sheer Overlay off.
- Constant Preview on as the normal baseline.
- Zebra display off for still photography; it remains available from My Menu for a special case and will be reviewed separately for video.

Approved changes:

| Camera | Setting | Current | Target | Reason |
|---|---|---|---|---|
| G9 MkII | Histogram | Off | On | Provide an approximate live exposure guide on the camera that will handle most landscapes. Position it in an unobtrusive corner and use DISP. when a clean view is required. |
| G9 MkI | Highlight | Off | On | Exact firmware-2.7 name; identify clipped areas during playback, particularly useful for macro flash and portraits. |
| G9 MkII | Blinking Highlights | Off | On | Identify clipped areas during playback for landscapes, wildlife and other subjects. |

Exceptions and clarification:

- Histogram remains off on the G9 MkI because a live histogram does not represent the final flash-lit macro exposure reliably.
- Constant Preview remains off in all three approved G9 MkI macro custom modes.
- With Auto Review off, the G9 MkI `Highlight` warning and the G9 MkII equivalent are seen when an image is deliberately opened in playback; they will not interrupt capture automatically.

### Global Stabilization and Lens Behavior

Status: reviewed; no additional changes beyond previously approved settings.

Preserve:

- Stabilization on with normal handheld behavior as the global baseline on both cameras.
- Explicit stabilization overrides in every approved custom mode: tripod modes off, G9 MkI handheld macro Mode 1, normal G9 MkII wildlife Mode 1, and fast/erratic wildlife Mode 2.
- Focus Ring Control Non-Linear only when the attached lens exposes the setting; otherwise record it as unavailable.
- Lens Fn Button set to AF-ON only when the attached lens exposes the setting.
- Aperture Ring Increment at 1/3 EV only when a supported clickless aperture-ring lens exposes it; none of the planned lenses requires it.
- AF Micro Adjustment off unless a repeatable, tested focus offset is later demonstrated.
- G9 MkII Lens Information Confirmation on.

Previously approved global change retained:

- G9 MkI `Lens Position Resume` on; verify the G9 MkII exact equivalent in its Menu Atlas.

### Global Power, Monitor and Viewfinder Setup

Status: approved.

Power-save timing:

- Both cameras: Sleep Mode at 10 minutes; Auto LVF/Monitor Off at 2 minutes.
- The G9 MkII originally had a 2-minute/1-minute target. During implementation, the user explicitly transferred the less intrusive G9 MkI timing after testing it in practice.

Preserve on both cameras:
- Monitor Frame Rate at 60 fps.
- Monitor Backlight / LVF Luminance on Auto.
- Eye Sensor sensitivity Low.
- G9 MkI: no thermal-management menu exists in firmware 2.7. Verify the G9 MkII exact thermal setting in its Menu Atlas.
- All beeps off.
- USB connection selection on connection and USB power on.
- G9 MkI: no `System Frequency` Setup item exists in firmware 2.7. Verify the G9 MkII video-system behavior in its Menu Atlas rather than inferring a common menu.

Approved changes:

| Camera | Setting | Current | Target | Reason |
|---|---|---|---|---|
| G9 MkII | LVF Frame Rate | 60 fps | 120 fps | Provide smoother viewfinder motion for tracking fast wildlife. Increased battery use is accepted and mitigated by the approved power-save timings. |
| G9 MkI | Battery Use Priority | Unavailable without DMW-BGG9 | Do not program | The user has no grip; the manual documents this menu only after attaching it. |
| G9 MkII | Battery Use Priority | Unknown until firmware-2.7 atlas verification | Do not infer | Verify whether the item is exposed with no grip before attempting a change. |

Preserve G9 MkI LVF Frame Rate at 60 fps because its main macro, portrait, general and occasional-landscape roles do not justify the additional battery consumption of a high-rate wildlife viewfinder.

### Global Video and Audio Setup

Status: approved.

Operating scope:

- No general-purpose video custom mode will be created.
- G9 MkII C3-10 Wildlife Video remains the only planned specialized video mode.
- G9 MkI video settings remain unchanged because video is outside its assigned trip role.

Approved G9 MkII changes:

| Setting | Current | Target | Reason |
|---|---|---|---|
| Creative Video Combined Set | All Combined | Separate all five groups | Prevent Creative Video settings from changing still-photo settings and prevent still-photo settings from contaminating C3-10. |
| Exposure / ISO / Exposure Compensation group | Combined | Separate video value | Isolate video exposure controls. |
| White Balance group | Combined | Separate video value | Isolate video white balance. |
| Photo Style group | Combined | Separate video value | Isolate the approved Standard video rendering. |
| Metering Mode group | Combined | Separate video value | Isolate video metering behavior. |
| AF Mode group | Combined | Separate video value | Isolate AFC Animal Tracking from still-photo modes. |
| Global 8-bit Luminance Level | 16-255 | 16-235 | Use the standard video range for broad compatibility. |
| C3-10 10-bit Luminance Level | Not explicitly recorded | 64-940 | Use the corresponding standard 10-bit range with Standard Photo Style. |
| Sound Rec Level Display | Off | On, Small | Confirm that internal-microphone audio is present and monitor excessive levels without a large overlay. |
| XLR Mic Adaptor Setting | Not displayed without adaptor | Effective Off | The firmware-2.7 camera uses the built-in microphone and hides this control when no XLR adaptor is attached. |

Preserve on the G9 MkII:

- MP4 format and Full image area.
- 4K30 as the global fallback quality outside C3-10.
- C3-10 4K60, 10-bit, HEVC 100 Mbps quality and all previously approved C3-10 settings.
- Internal microphone, Standard recording gain, 0 dB level adjustment, limiter on, Standard wind-noise cancellation and Lens Noise Cut on.
- Continuous AF Mode 1.
- Zebra 1 and Zebra 2 at 95%, the maximum offered by the camera in the active 4K 10-bit / 64-940 configuration; Zebra display off by default.
- Red Recording Frame on.
- Time Code unavailable with MP4.
- Auto Exposure in P/A/S/M on.
- Current HDMI output settings. The Shinobi II remains outside the trip workflow.

Preserve the G9 MkI video baseline without changes, including its audio-level display off.

### Global Wireless Connectivity, Clock Synchronization and Geolocation

Status: approved subject to a two-camera field test.

Approved target on both cameras:

| Setting | Target | Reason |
|---|---|---|
| Bluetooth | Off during the photographic day | Save camera and phone battery; enable only for deliberate morning clock synchronization. |
| Auto Clock Set | Preference On | When Bluetooth reconnects, both cameras restore this preference and synchronize clock/time-zone information; while Bluetooth is Off, the row is greyed and displays forced Off. |
| Location Logging | Off | Direct Bluetooth geotagging is outside the final workflow. |
| Wi-Fi | Off unless actively required | Clock and location use Bluetooth; avoid unnecessary higher-power Wi-Fi operation. |
| Auto Transfer | Off | Avoid large RAW transfers, Wi-Fi activation, battery use and background transfer delays. |
| Remote Wakeup | Off | It is not required for clock or location logging and can continue draining the camera battery while the power switch is off. |

Applications:

- G9 MkI: use Panasonic Image App briefly for morning Auto Clock Set synchronization.
- G9 MkII: use LUMIX Sync briefly for morning Auto Clock Set synchronization.
- G9 MkII Network Connection Light remains off.
- G9 MkII USB-SSD remains off.
- USB Mode remains Select on Connection and USB Power remains on.

Required two-camera test before travel:

1. Each morning, verify that the iPhone has the correct local time and time zone.
2. Enable Bluetooth on the G9 MkI, connect Panasonic Image App and verify the camera clock; then disable Bluetooth.
3. Enable Bluetooth Function on the G9 MkII, connect LUMIX Sync and verify the camera clock; then disable Bluetooth Function.
4. Repeat after every time-zone change or extended period without a camera battery.
5. At each new photographic location, take an iPhone reference photograph with precise location enabled.
6. During post-processing, copy the iPhone photograph's GPS metadata to the corresponding Lumix photographs.

No continuous background camera connection or GPX track is required in the final workflow.

Privacy note:

- Geotagged files reveal the capture location. Remove location metadata before public sharing when the location is private or sensitive, particularly for vulnerable wildlife.

### G9 MkII Custom-Mode Reload Behavior

Status: approved.

Approved configuration:

| Setting | Target | Reason |
|---|---|---|
| Limit No. of Custom Mode | 10 | Make all approved C3-1 through C3-10 slots available. |
| How to Reload Custom Mode | Turn the Power ON | Preserve temporary field changes through normal Sleep cycles but restore the registered baseline after the camera is switched off and on. |
| Select Loading Details — F / SS / ISO Sensitivity | On | Ensure each mode recalls its approved exposure baseline. |
| Select Loading Details — White Balance | On | Ensure each mode recalls its approved white-balance baseline. |
| Edit Title | Use all approved mode names | Make every C3 selection immediately identifiable. |

Operational rule:

- Temporary changes made while using a custom mode are not saved permanently unless Save to Custom Mode is used explicitly.
- Power-cycling the camera restores the registered custom-mode baseline.
- Switching away from a custom mode and recalling it also restores its registered settings.
- Do not use Save to Custom Mode in the field unless intentionally replacing the approved baseline.

### Global Playback Safety

Status: approved.

Approved target on both cameras:

- Playback Mode: Normal Play.
- Rotate Display: On.
- Delete Confirmation: No First, requiring a deliberate move to Yes before deletion.
- Auto Review remains off as previously approved.
- Avoid extensive in-camera deletion or culling during the day; perform the main selection only after the daily files from both backup cards have been copied and verified.

### Remaining Global Controls

Status: approved.

Approved change on both cameras:

| Setting | Current | Target | Reason |
|---|---|---|---|
| Exposure Comp Reset | Off | On | Clear forgotten exposure compensation after a power cycle and reduce the risk of an unintentionally biased session. The loss of intentional persistence across power cycles is accepted. |

Preserve on both cameras where available:

- Touch Screen on; Touch Tab and Touch Pad AF off.
- Joystick set to direct focus-area movement.
- Current default dial behavior.
- WB/ISO/Exposure button behavior After Pressing 2.
- Operation Lock with all normal controls unlocked.
- AWB Lock through the approved Fn behavior.
- Silent Mode off as the global baseline.
- Flash off as the global baseline; approved G9 MkI macro modes supply their explicit TTL or Manual flash overrides.
- Flash synchronization First Curtain, global flash compensation ±0 and Wireless Flash off.
- Shutter Delay, Bracketing and in-camera HDR off as global baselines; approved custom modes supply explicit overrides.
- Current interface language.

### Firmware and Pre-Programming Maintenance

Status: approved. Perform before programming the redesigned global settings and custom modes.

Current official target versions as checked on 2026-08-07:

| Equipment | Project/current record | Official target |
|---|---|---|
| G9 MkI body | `Latest`, exact installed version to verify | 2.7 |
| G9 MkII body | 2.5 | 2.7 |
| Lumix G 12–35mm f/2.8 II, H-HSA12035 | Installed version to verify | 1.3 |
| Lumix G 35–100mm f/2.8 II, H-HSA35100 | Installed version to verify | 1.3 |
| Leica 100–400mm II, H-RSA100400 | Installed version to verify | 1.1 |

Firmware rationale and order:

- G9 MkII firmware 2.7 is required because it fixes cases in which AF-ON did not function with Shutter AF off, directly affecting the approved back-button-focus system.
- Photograph or otherwise record critical current settings before any update.
- Save the current G9 MkII state to a card and to LUMIX Sync before updating.
- Update bodies first and lenses afterward, using only one firmware file at a time.
- Do not perform a general Reset after updating.
- Verify AF-ON, Shutter AF off, Bluetooth, card recording and basic capture after each relevant update.
- Program the redesigned globals and custom modes only after firmware verification succeeds.

Pre-programming maintenance:

- Run Sensor Cleaning on both cameras.
- Make an f/16 photograph of an evenly illuminated plain surface and inspect it for sensor dust.
- Perform physical sensor cleaning only if the test demonstrates a need and use an appropriate procedure.
- Calibrate the Level Gauge on both cameras using a genuinely level reference surface.
- Use Pixel Refresh only if repeatable bright or stuck pixels are visible; do not use it as routine maintenance.
- Test all four primary 256 GB cards and the two 64 GB reserve cards; format each card in its intended camera after its contents have been backed up and verified.
- Test MIOPS cables and triggering, Godox X3/MF12 flash operation, and the physical contacts and operation of lenses and teleconverters.
- Complete the approved morning two-camera clock-synchronization test and iPhone reference-photo geotagging test.

Final backup after programming and validation:

- Create the G9 MkII settings backup `USPK2026` on both primary cards, preserve a computer copy, enable Keep Settings While Format and save a LUMIX Sync settings copy.
- Preserve the G9 MkI through its final JSON, complete programming checklist and photographs of critical menu pages.

## Global Review Completion

Status: all conceptual global set-and-forget categories have been reviewed and approved. The G9 MkI target JSON is updated after its macro tests; its final physical closure pass remains pending. Do not infer G9 MkII closure from that state.

## G9 MkII Migration Ledger

Implementation override: the historical baselines named below describe conceptual ancestry only. Do not load an old C3 slot during programming because its stored settings may have changed since the inventory was written. Build C3-1 from the newly programmed and power-cycle-verified C1 common template, then derive every later target only from C1 or the closest newly completed and verified target. The pre-change DAT backup preserves the old camera state if recovery is needed.

### C3-1 — Stationary / Slow Wildlife

Status: settings confirmed.

Baseline: current G9 MkII C3-3 `Wildlife / Action`.

Preserve:

- Recording mode: Manual
- Aperture: f/6.3 at 400mm
- ISO: Auto, current upper-limit behavior
- Focus mode: AFC
- Subject detection: Animal
- AF area: Zone
- Drive: Burst I
- Burst Shot 1: H
- Shutter type: Electronic
- Stabilizer: Mode 1
- RAW, Standard Photo Style, AWB, Multi metering

Approved changes:

| Setting | Current | Target | Reason |
|---|---|---|---|
| Slot | C3-3 | C3-1 | Begin the progressive wildlife sequence with slow subjects. |
| Name | Wildlife / Action | Stationary / Slow Wildlife | Describe resting, grazing and slowly walking animals. |
| Shutter speed | 1/1000s | 1/500s | Gain one stop of ISO while retaining protection against modest animal movement. |

Teleconverter field variants, without separate slots:

- 1.4×: C3-1 is the normal basis without a saved-mode change; C3-2 is allowed for action only in strong light.
- 2×: C3-1 is the normal basis without a saved-mode change, normally with tripod and a stationary or slow subject.
- C3-3 is normally avoided with either teleconverter because 1/4000s combined with the reduced aperture imposes a severe ISO cost.

### C3-2 — Wildlife Action

Status: migration confirmed; settings approved.

Baseline: current G9 MkII C3-3 `Wildlife / Action`.

Approved changes:

- Move C3-3 to C3-2.
- Rename to `Wildlife Action`.
- No technical setting changes.

Preserve:

- Recording mode: Manual
- Aperture: f/6.3 at 400mm
- Shutter speed: 1/1000s
- ISO: Auto, current upper-limit behavior
- Focus mode: AFC
- Subject detection: Animal
- AF area: Zone
- Drive: Burst I
- Burst Shot 1: H
- Shutter type: Electronic
- Stabilizer: Mode 1
- RAW, Standard Photo Style, AWB, Multi metering

### C3-3 — Fast / Erratic Wildlife

Status: migration confirmed; settings approved.

Baseline: current G9 MkII C3-8 `Birds in Flight (BIF)`.

Approved changes:

- Move C3-8 to C3-3.
- Rename to `Fast / Erratic Wildlife` so it also covers running mammals and other unpredictable action.
- No technical setting changes.

Preserve:

- Recording mode: Manual
- Aperture: f/6.3 at 400mm
- Shutter speed: 1/4000s
- ISO: Auto, current upper-limit behavior
- Focus mode: AFC
- Subject detection: Animal
- AF area: Tracking
- Drive: Burst I
- Burst Shot 1: H
- Shutter type: Electronic
- Stabilizer: Mode 2 (panning left/right)
- RAW, Standard Photo Style, AWB, Multi metering

### C3-4 — Standard Handheld Landscape

Status: migration confirmed; settings approved.

Baseline: current G9 MkII C3-1 `Landscape`.

Approved changes:

- Move C3-1 to C3-4.
- Rename to `Standard Handheld Landscape`.
- No technical setting changes.

Preserve:

- Recording mode: Aperture Priority
- Aperture: f/8
- Focus mode: AFS
- Drive: Single Shot
- AF Detection: Off
- Photo Style: Scenery/Landscape
- White balance: AWB
- Current inherited global settings, including normal stabilization for handheld use

### C3-5 — Tripod HDR Landscape

Status: migration and settings confirmed.

Baseline: current G9 MkII C3-1 `Landscape`.

Preserve:

- Recording mode: Aperture Priority
- Aperture: f/8
- Focus mode: AFS
- AF Detection: Off
- Photo Style: Scenery/Landscape
- White balance: AWB
- Metering: Multi
- RAW
- Shutter type: Electronic Front Curtain

Approved changes:

| Setting | Current | Target | Reason |
|---|---|---|---|
| Slot | C3-1 | C3-5 | Place tripod HDR after the standard handheld landscape mode. |
| Name | Landscape | Tripod HDR Landscape | Identify the mode as an HDR-merge workflow. |
| ISO | Auto | ISO 100 fixed | Maximize image quality and ensure the bracket varies exposure by shutter speed. |
| Stabilizer | On / normal handheld behavior | Off | The camera is supported on a tripod. |
| Exposure bracketing | Off | Five RAW frames at 1 EV: −2, −1, 0, +1, +2 EV | Capture the approved exposure range for HDR merging. |
| Bracket capture | One frame per shutter press | Complete bracket from one shutter press | Avoid touching the camera between bracketed exposures. |
| Shutter delay | Off | 2 seconds | Allow vibration from pressing the shutter to settle before the sequence. |

Operational intent:

- Always merge the bracketed set rather than treating the mode as a single-exposure selector.
- Aperture Priority keeps f/8 fixed while shutter speed provides the exposure variation.

### C3-6 — Focus-Bracketed Landscape

Status: migration and initial settings confirmed; field calibration still required.

Baseline: current G9 MkII C3-1 `Landscape`.

Preserve:

- Recording mode: Aperture Priority
- Aperture: f/8
- Focus mode: AFS
- AF Detection: Off
- Photo Style: Scenery/Landscape
- White balance: AWB
- Metering: Multi
- RAW
- Shutter type: Electronic Front Curtain

Approved changes:

| Setting | Current | Target | Reason |
|---|---|---|---|
| Slot | C3-1 | C3-6 | Place focus-stacked landscape after tripod HDR. |
| Name | Landscape | Focus-Bracketed Landscape | Identify the focus-stacking workflow. |
| ISO | Auto | ISO 100 fixed | Maximize image quality and keep exposure consistent through the stack. |
| AF area | Current / inherited | 1-Area | Select the nearest intended point precisely. |
| Stabilizer | On / normal handheld behavior | Off | The camera is supported on a tripod. |
| Bracketing | Off | Focus Bracket | Record the images needed for focus stacking. |
| Focus bracket sequence | Not applicable | 0/+ | Start at the selected near point and move focus only toward the far distance. |
| Focus bracket step | Not applicable | 3 | Favor conservative overlap between focus planes. |
| Focus bracket image count | Not applicable | 30 | Provide a practical safety margin across the 12–35mm zoom range. |
| Shutter delay | Off | 2 seconds | Allow vibration from pressing the shutter to settle before the sequence. |

Operational intent and calibration:

- Tripod workflow for landscapes with a close foreground and distant background.
- Primary lens: 12–35mm actual focal length (24–70mm full-frame equivalent).
- Set the 1-Area AF point on the nearest element that must be sharp, then capture the 0/+ sequence.
- Preserve AFS rather than copying MF from the current Tripod Macro: AFS simplifies selecting the near starting point, while Focus Bracket performs the subsequent focus shifts.
- Step 3 and 30 images are the approved initial settings, not assumed final calibration values.
- Before travel, test at 12mm, 25mm and 35mm and merge the results to confirm that there are no focus gaps.

### C3-7 — Long Exposure Landscape with ND

Status: migration and initial settings confirmed.

Baseline: current G9 MkII C3-1 `Landscape`.

Preserve:

- Aperture: f/8
- Focus mode: AFS
- AF Detection: Off
- Photo Style: Scenery/Landscape
- White balance: AWB
- Metering: Multi
- RAW
- Shutter type: Electronic Front Curtain
- Long Exposure Noise Reduction: Off

Approved changes:

| Setting | Current | Target | Reason |
|---|---|---|---|
| Slot | C3-1 | C3-7 | Place the long-exposure workflow after the two bracketed landscape modes. |
| Name | Landscape | Long Exposure Landscape with ND | Identify the tripod and ND-filter workflow. |
| Recording mode | Aperture Priority | Manual | Prevent automatic exposure changes while preparing and fitting the filter. |
| ISO | Auto | ISO 100 fixed | Maximize quality and facilitate longer exposures. |
| Initial shutter speed | Automatic | 1 second | Provide a practical starting point for moving water with ND64. |
| AF area | Current / inherited | 1-Area | Select the intended focus region precisely. |
| Stabilizer | On / normal handheld behavior | Off | The camera is supported on a tripod. |
| Shutter delay | Off | 2 seconds | Allow vibration from pressing the shutter to settle. |

Operational intent:

- Recall baseline: Manual, f/8, ISO 100 and 1 second.
- Primary use: waterfalls and rivers.
- Shutter speed is the normal field adjustment according to ambient light, filter strength and desired water rendering.
- Keep Long Exposure Noise Reduction off to avoid a same-duration dark exposure after every attempt; enable it manually only for unusually long exposures when appropriate.
- K&F Concept magnetic 72mm kit: GND8, ND8, ND64 and ND1000, plus 58→72mm and 55→72mm direct step-up rings.
- ND64 is the initial default; a detailed filter field guide will be created later.
- Leica 9mm f/1.7 waterfall alternative: use the direct 55→72mm step-up ring with the threaded K&F Nano-X PRO VND2-32/CPL 72mm, tripod, ISO 100, f/8 and approximately 1/2 second as the initial exposure. Compare roughly 1/4, 1/2 and 1 second; adjust CPL conservatively while watching the full ultra-wide frame. Do not stack filters.

### C3-8 — Lightning — MIOPS Smart+

Status: migration and settings confirmed; device and cable test still required.

Baseline: current G9 MkII C3-6 `Landscape Tripod (Lightning) — Live View Composite`.

Preserve:

- Recording mode: Manual
- Focus mode: MF
- Drive: Single Shot
- Aperture: f/8
- AF Detection: Off
- Photo Style: Standard
- Flash: Forced Off
- Stabilizer: Off
- Tripod workflow
- White balance: Daylight

Approved changes:

| Setting | Current | Target | Reason |
|---|---|---|---|
| Slot | C3-6 | C3-8 | Place lightning after the main landscape modes. |
| Name | Landscape Tripod (Lightning) — Live View Composite | Lightning — MIOPS Smart+ | Make the external lightning trigger the primary workflow. |
| Aspect ratio | 16:9 | 4:3 | Use the full sensor and leave cropping decisions for post-processing. |
| Shutter speed | 4 seconds | 1/125s | Provide the initial response-oriented exposure for a MIOPS-triggered lightning event. |
| ISO | 400 | 200 | Reduce the risk of clipping the bright lightning channels. |
| Live View Composite | Configured / primary | Off / night variation | MIOPS Lightning Mode becomes the principal method. |
| Shutter type | Mechanical, forced by Live View Composite | Electronic Front Curtain | Remove the LVC restriction and reduce camera-induced vibration. |
| Shutter delay | 2 seconds | Off | Respond immediately to the MIOPS trigger signal. |
| Long Exposure Noise Reduction | On, forced by Live View Composite | Off | It is unnecessary at 1/125s and would interfere with continuous readiness. |

Recall baseline:

- Manual, f/8, 1/125s, ISO 200, MF, RAW, 4:3, Electronic Front Curtain and stabilizer off.

MIOPS Smart+ companion setup:

- Lightning Mode.
- Prefocus enabled.
- Calibrate sensitivity for ambient light and false-trigger behavior.
- Correct camera cable is owned but has never been tested.
- Before travel, verify cable release, Prefocus, Lightning Mode, sensitivity and false-trigger behavior.

Field variation:

- Live View Composite remains available as a manually activated night-lightning variation; it is not the saved C3-8 baseline.
- MIOPS Capsule Pro is not the lightning trigger; it remains optional for panorama/timelapse work.

### C3-9 — Tripod High Resolution Landscape

Status: migration and settings confirmed.

Baseline: current G9 MkII C3-1 `Landscape`.

Preserve:

- Recording mode: Aperture Priority
- Aperture: f/8
- Focus mode: AFS
- AF Detection: Off
- Photo Style: Scenery/Landscape
- White balance: AWB
- Metering: Multi
- Aspect ratio: 4:3
- RAW workflow

Approved changes:

| Setting | Current | Target | Reason |
|---|---|---|---|
| Slot | C3-1 | C3-9 | Place High Resolution after the lightning workflow. |
| Name | Landscape | Tripod High Resolution Landscape | Identify the static-scene tripod workflow. |
| Drive mode | Single Shot | High Resolution | Activate the multi-image sensor-shift capture and merge. |
| ISO | Auto | ISO 100 fixed | Maximize detail and dynamic range. |
| AF area | Current / inherited | 1-Area | Position focus precisely. |
| Handheld High-Res | Not applicable / Off | Off | This custom mode is designed for tripod use. |
| High Resolution Picture Quality | Not applicable | RAW | Preserve maximum processing flexibility. |
| High Resolution Picture Size | Not applicable | Camera-forced XL, 100 MP at 4:3 (11552×8672) when Picture Quality is RAW | The item is unavailable because RAW fixes the maximum dimensions. |
| Simul Record Normal Shot | Off | On | Save a normal approximately 25 MP safety frame as well as the merged image. |
| Motion Blur Processing | Not applicable | Mode 1 | Prioritize the full High Resolution effect. |
| Shutter delay | Off | 2 seconds | Allow vibration from pressing the shutter to settle. |

Camera-enforced behavior:

- With Handheld High-Res off, stabilization is unavailable; tripod support is required.
- Shutter type is fixed to Electronic in High Resolution Mode.
- Available shutter-speed range is 1 second to 1/32000s.

Operational intent:

- Recall baseline: Aperture Priority, f/8, ISO 100, AFS 1-Area, 100 MP RAW, simultaneous normal RAW, Motion Blur Processing Mode 1 and 2-second delay.
- Use only for substantially static scenes. Mode 1 can render movement in leaves, water, animals or people as afterimages.
- The simultaneously recorded normal frame is the safety image when movement compromises the merged result.
- If the required exposure is longer than 1 second, use C3-4, C3-5 or C3-7 instead.

### C3-10 — Wildlife Video

Status: implemented, recalled and capture-tested on firmware 2.7.

C3-10 was built directly in Creative Video. The previous custom mode was not recalled because stored camera settings could have drifted from the earlier project record, and firmware 2.7 does not permit loading a still custom mode into Creative Video.

Implemented baseline:

- Recording mode: Creative Video
- Exposure sub-mode: Manual
- Aperture: f/6.3
- ISO: Auto, current upper-limit behavior
- Focus mode: AFC
- Subject detection: Animal
- AF area: Tracking
- Photo Style: Standard
- White balance: AWB
- Optical stabilization: Normal
- E-Stabilization Video: High
- Boost I.S. Video: Off
- Electronic shutter / video behavior
- Flash: Forced Off
- Internal recording
- Internal microphone, Standard gain, 0dB, limiter On, Standard wind-noise cancellation and Lens Noise Cut On

Approved changes:

| Setting | Current | Target | Reason |
|---|---|---|---|
| Name | Video — Wildlife in Motion | Wildlife Video | Simplify the label and align it with the target mode map. |
| Recording quality | 4K 30p | MP4 4K 60p, 10-bit, 100 Mbps, HEVC | Provide smoother motion and allow 2× slow motion on a 30p timeline. |
| Shutter speed | 1/500s | 1/125s | Approximate a 180-degree shutter at 60p, render motion naturally and reduce ISO demand. |

Operational intent:

- Recall baseline: 4K60, 10-bit, HEVC 100 Mbps, Manual exposure, 1/125s, f/6.3, Auto ISO, AFC Animal Tracking, optical stabilization Normal and E-Stabilization High.
- Handheld-first; tripod remains a field variation.
- 100–400mm II is the primary lens.
- Raise shutter speed manually to 1/250s or 1/500s for very fast animals, frame analysis or possible frame extraction, accepting higher ISO and less natural motion rendering.
- Standard Photo Style only; no V-Log/HLG workflow.
- Atomos Shinobi II is excluded from the trip workflow.

Verified firmware-2.7 behavior:

- MP4 fixes `Sound Rec Quality` to 48kHz/16bit.
- `Time Code` is unavailable with MP4.
- `XLR Mic Adaptor Setting` is not displayed when no adaptor is attached; the built-in microphone is active.
- Zebra 1 and Zebra 2 offered a maximum of 95% with the active 4K 10-bit / 64-940 configuration, so both were saved at 95% and the Zebra display remains Off by default.
- The recalled mode recorded and played back a 4K60 10-bit HEVC clip successfully, with moving audio meters, audible internal-microphone sound, continuous AF and the red recording frame.

## G9 MkI Migration Ledger

### C1 — General / Street

Status: settings confirmed for both G9 MkI and G9 MkII.

Baseline: current G9 MkI and G9 MkII C1 `Street / General`. The G9 MkI behavior is the focus-system reference because it already explicitly disables detection.

Preserve on both cameras:

- Recording mode: Aperture Priority
- Aperture: f/5.6
- Focus mode: AFS
- Drive: Single Shot
- ISO: Auto, current upper-limit behavior
- Photo Style: Standard
- RAW
- White balance: AWB
- Metering: Multi
- Normal handheld stabilization
- Shutter type: Electronic Front Curtain

Approved changes:

| Setting | Current | Target | Reason |
|---|---|---|---|
| G9 MkII AF Detection | May inherit global Human Detection because C1 has no explicit override | Explicitly Off | Make C1 behavior predictable and consistent across both bodies. |
| G9 MkI AF mode | Not uniformly explicit | `225-Area` | Exact broad-area mode without automatic detection on the G9 MkI. |
| G9 MkII AF area/detection | Not uniformly explicit | Verify exact equivalent in G9 MkII Menu Atlas | Do not transfer the G9 MkI terminology to the newer body. |
| Minimum shutter speed | Auto | 1/125s | Protect street photographs from subject movement and modest camera motion. |

Accepted tradeoff:

- Auto ISO will rise earlier in low light because of the 1/125s minimum.
- This is accepted because C1 prioritizes General/Street responsiveness, while deliberate landscapes are assigned primarily to the G9 MkII landscape modes.

Operational intent:

- Primary lens: 12–35mm.
- 35–100mm is an alternate for tighter travel details.
- G9 MkI recall baseline: Aperture Priority, f/5.6, AFS `225-Area`, Single Shot, Auto ISO with 1/125s minimum, RAW, AWB, stabilization on and Electronic Front Curtain.

### C2 — Portrait

Status: settings confirmed for both G9 MkI and G9 MkII.

Baseline: current G9 MkI and G9 MkII C2 `Portrait / People`.

Preserve on both cameras:

- Recording mode: Aperture Priority
- Aperture: f/2.8
- Focus mode: AFS
- Drive: Single Shot
- Photo Style: Portrait
- Human / Face / Eye Detection
- ISO: Auto, current upper-limit behavior
- Minimum shutter speed: 1/125s
- White balance: AWB
- Metering: Multi
- RAW
- Normal handheld stabilization
- Shutter type: Electronic Front Curtain

Approved clarification / change:

| Setting | Current | Target | Reason |
|---|---|---|---|
| G9 MkII AF Detection and area | Inherited from global baseline | Explicit Human Detection On and Full Area | Prevent a later global change from altering C2 behavior. |

G9 MkI behavior:

- G9 MkI: replace the imprecise Full Area/detection wording with the exact AF mode `Human Detect AF`.
- The focus-system wording and capability differ by camera generation, but the operational intent is the same.

Operational intent:

- Primary lens: 35–100mm f/2.8.
- Environmental alternative: 12–35mm f/2.8.
- Olympus 60mm Macro is an alternate for tighter, deliberate portraits.
- G9 MkI recall baseline: Aperture Priority, f/2.8, AFS `Human Detect AF`, Single Shot, Auto ISO with 1/125s minimum, Portrait Photo Style, RAW, AWB, stabilization on and Electronic Front Curtain.

### C3-1 — Single Macro — TTL

Status: macro acceptance-tested and target approved; final clean-source programming and closure verification remain pending.

Historical ancestry: old G9 MkI C3-2 `Macro Handheld (Single Shot)`. Executable source: newly completed C1, then save the fully specified macro state to C3-1.

Preserve:

- Recording mode: Manual
- Focus mode: MF
- Drive: Single Shot
- Aperture: f/16
- Shutter speed: 1/200s
- ISO: 200 fixed
- Photo Style: Natural
- Flash: Forced On / TTL, compensation ±0
- Shutter type: Mechanical
- Stabilizer: On, Mode 1
- White balance: AWB
- RAW
- G9 MkI `Peaking`: On, `Detect Level = LOW`, red `Display Color`
- MF Assist and MF Guide: On

Approved changes:

| Setting | Current | Target | Reason |
|---|---|---|---|
| Slot | C3-2 | C3-1 | Make TTL single-frame macro the first specialized mode. |
| Name | Macro Handheld (Single Shot) | Single Macro — TTL | Identify the capture and flash-control method immediately. |
| Primary lighting equipment | Godox AD100Pro, X3 and ML-CD15 diffuser | Two Godox MF12 units on the lens ring, controlled by Godox X3 | Match the principal travel macro kit. |
| Constant Preview | Globally On / no current mode override | Off in C3-1 | Keep the live view usable at Manual f/16, 1/200s before the flash fires. |

Recall baseline:

- Manual, MF, Single Shot, f/16, 1/200s, ISO 200, TTL ±0, Mechanical shutter, stabilizer Mode 1 and Constant Preview off.

Operational intent and calibration:

- Olympus 60mm Macro.
- Manual-focus rocking technique.
- Preserve f/16 and ISO 200. Several individual frames produced consistent TTL exposure and a usable viewfinder; this is intentionally not a burst mode.
- The Panasonic Wireless menu stays off because radio control is handled by the Godox X3/MF12 system.

### C3-2 — Macro Burst — Manual Flash

Status: macro acceptance-tested and target approved; Burst II / M is the final target, with clean-source programming and closure verification pending.

Historical ancestry: old G9 MkI C3-2 `Macro Handheld (Single Shot)`. Executable source: recall the newly completed C3-1 again, then save the burst target to C3-2.

Preserve:

- Recording mode: Manual
- Focus mode: MF
- Aperture: f/16
- Shutter speed: 1/200s
- Photo Style: Natural
- RAW
- Shutter type: Mechanical
- Stabilizer: On, Mode 1
- White balance: AWB
- Constant Preview: Off
- G9 MkI Peaking, MF Assist and MF Guide: On

Approved changes from target C3-1:

| Setting | C3-1 | C3-2 | Reason |
|---|---|---|---|
| Name | Single Macro — TTL | Macro Burst — Manual Flash | Identify the short-burst and manual-lighting workflow. |
| Drive | Single Shot | Physical Burst II; `Burst Shot 2 Setting = M` | The validated combination recorded 26 fully illuminated frames in 10 seconds; use only 3–5 frames per field burst. |
| ISO | 200 | 400 | Save one stop of flash output and shorten recycling. |
| Flash control | TTL | Manual | Keep exposure and recycling consistent through the burst. |
| Initial MF12 power | TTL-determined | Group A at 1/32 for both units | Provide consistent low-power output and rapid recycling. |
| MF12 grouping | Both units in Group A; Group B Off | Both units in Group A; Group B Off | The X3 controls the pair together; do not document a nonexistent A/B left-right balance. |

Recall baseline:

- Manual, MF, physical Burst II with `Burst Shot 2 Setting = M`, f/16, 1/200s, ISO 400, both Group A MF12 units at Manual 1/32, Mechanical shutter and stabilizer Mode 1.

Operational intent:

- Olympus 60mm Macro with manual-focus rocking.
- Hold the shutter for approximately 1.2–1.9 seconds to produce 3–5 frames at the measured 2.6 fps cadence.
- The objective is not a guaranteed stack; it is to increase the probability that at least one frame places the critical plane in focus.
- If body movement is sufficiently smooth, the sequence may occasionally support a small software stack, but this is not the mode's success criterion.
- The physical drive dial must be moved to Burst II; a recalled custom mode cannot physically move the dial.

Validated calibration and field adjustment order:

1. Burst II / M, ISO 400 and Group A Manual 1/32 recorded 26 fully illuminated frames in 10 seconds and is the final baseline.
2. TTL was rejected after producing many black frames under C3-2 conditions.
3. Physical Burst I / L, ISO 200 and Group A Manual 1/32 recorded 15 illuminated frames in 10 seconds and remains the quality-priority alternative.
4. If the final baseline is underexposed in a new situation, raise ISO to 640 or 800 before raising flash power.
5. If any frame misses flash, stop and allow recycling; do not increase cadence.

### C3-3 — Supported Macro Focus Bracket

Status: macro acceptance-tested and target approved; TTL completed the full capture and merge workflow, with clean-source programming and closure verification pending.

Historical ancestry: old G9 MkI C3-2 `Macro Handheld (Single Shot)`; old C3-3 `Wildlife / Action` is retired because wildlife specialization moves to the G9 MkII. Executable source: newly completed C3-1, then save the supported-bracket target to C3-3.

Preserve:

- Recording mode: Manual
- Focus mode: MF
- Olympus 60mm Macro
- Shutter speed: 1/200s
- Photo Style: Natural
- RAW
- White balance: AWB
- Shutter type: Mechanical
- Constant Preview: Off
- G9 MkI Peaking, MF Assist and MF Guide: On

Approved changes:

| Setting | Current Macro Single | Target | Reason |
|---|---|---|---|
| Slot | C3-2 | C3-3 | Reserve the third specialized slot for the low-priority supported stack. |
| Name | Macro Handheld (Single Shot) | Supported Macro Focus Bracket | Identify the supported automatic focus-stack workflow. |
| Camera support | Handheld | Tripod or firm surface | Keep framing and perspective fixed through the stack. |
| Aperture | f/16 | f/8 | Let stacking provide depth, reduce diffraction and require four times less flash energy than f/16. |
| ISO | 200 | 400 | Save one additional stop of flash energy. |
| Flash control | TTL | Group A TTL +0.0; Group B Off | Validated with both diffused MF12 units in Group A. |
| Stabilizer | On, Mode 1 | Off | The camera is supported. |
| Bracketing | Off | Focus Bracket | Record the automatic focus sequence. |
| Focus bracket step | Not applicable | 2 | Favor conservative overlap between focus planes. |
| Focus bracket image count | Not applicable | 40 | Provide a practical initial range for the 60mm Macro. |
| Focus bracket sequence | Not applicable | 0/+ | Start at the nearest required detail and move toward the background. |
| Shutter delay | Off | Off | The 2-second setting delayed every bracket frame and would stretch 40 images to roughly 80 seconds. |
| Physical drive dial | Single Shot | Single Shot | Allow the camera to execute the programmed image count automatically. |

Recall baseline:

- Manual, MF, Single Shot, f/8, 1/200s, ISO 400, Group A TTL +0.0, Mechanical shutter, stabilizer off, Focus Bracket Step 2, 40 images, sequence 0/+ and shutter delay Off.

Operational intent:

- Supported camera and static subject; no handheld focus-stacking expectation in this mode.
- Not emphasized as the principal macro workflow.
- Compared with C3-1, f/8 and ISO 400 reduce theoretical flash-energy demand by three stops.
- MF12 pair controlled by X3 is the initial light; AD100Pro remains an optional off-camera variation.

Validated result:

1. A supported camera and static subject produced 40 fully illuminated TTL frames in 12 seconds.
2. Step 2 / 40 / 0+ covered approximately 4 cm from near to far.
3. The sequence merged perfectly in Helicon Focus.
4. Manual 1/64 and the MF12 modeling lights remain fallbacks only; the approved TTL baseline did not require them.

## Equipment and Scope Decisions

Travel macro gear excluded:

- OM System 90mm f/3.5 Macro IS PRO
- Raynox DCR-250
- OM MC-20

Travel macro gear included:

- Olympus 60mm f/2.8 Macro
- Pair of Godox MF12 flashes on the lens ring
- Godox X3 transmitter
- Godox AD100Pro as optional handheld/off-camera light

Wildlife gear:

- Leica 100–400mm II
- Panasonic 1.4× and 2× teleconverters
- Tripod normally available

Landscape filter gear:

- K&F Concept magnetic 72mm kit: fixed GND8, ND8, ND64 and ND1000
- Direct 58→72mm and 55→72mm step-up rings
- K&F Concept KF01.2001 Nano-X PRO 72mm VND2-32 + CPL 2-in-1, 1–5 stops, 36-layer coating
- K&F Concept 72mm VND2-400, 1–9 stops
- K&F Concept NANO-K 58mm HMC CPL, standalone circular polarizer
- K&F Concept NANO-K 72mm HMC CPL, standalone circular polarizer
- Never stack the two variable filters; the VND2-32/CPL is the normal Leica 9mm waterfall choice, with VND2-400 reserved for light too strong for 5 stops
- Keep the standalone HMC CPL filters distinct from the KF01.2001 combined VND/CPL when selecting gear in the field

### Filter selection quick guide

| Filter | Use it when | Avoid or replace it when |
|---|---|---|
| NANO-K 58mm HMC CPL | Direct fit on the 12–35mm II for glare on water, wet foliage, rocks or glass; rotate while watching the subject in the EVF. | Light is scarce, reflections are not a problem, or the ultra-wide sky becomes uneven. |
| NANO-K 72mm HMC CPL | The lens or direct step-up ends at 72mm and the goal is controlling reflections or improving color separation, not creating a long exposure. | Another polarizer or VND is already mounted; never stack polarizers. |
| Nano-X PRO VND2-32 + CPL 2-in-1 | Waterfalls, streams and wet landscapes where both 1–5 stops of exposure control and independent reflection control are useful; normal Leica 9mm choice. | More than 5 stops are required, only simple polarization is needed, or the ultra-wide frame becomes uneven. |
| VND2-400, 1–9 stops | Strong or changing light where the needed density is not known in advance; increase only until the target shutter speed is reached. | A fixed ND already matches the required density, reflection control is the main goal, or the high setting creates an X pattern or color/exposure irregularity. |
| Fixed ND8, 3 stops | Mild motion blur, shade, dawn/dusk, or when ND64 would make the exposure unnecessarily long. | It cannot slow the shutter enough in bright conditions. |
| Fixed ND64, 6 stops | Normal daylight waterfall and river starting point for smooth water without exposures of many minutes. | Light is already low or the desired texture needs only a small reduction. |
| Fixed ND1000, 10 stops | Deliberate multi-second/minute effects in strong light: very smooth water, moving clouds or reduction of moving people. | Vegetation moves, spray reaches the lens, or a shorter exposure preserves better water texture. |
| GND8, up to 3 stops graduated | A bright sky and darker foreground are divided by a simple horizon that can align with the fixed transition. | Trees, mountains or buildings cross the transition visibly; use HDR bracket or a normal RAW instead. |

Selection rule: choose the effect first. CPL controls reflections; ND controls time; GND controls a brightness difference across the frame. Use only the least complex filter that solves the actual problem.

Travel video and observation gear:

- GoPro HERO12 Black, firmware 2.40, four batteries and one 256 GB V30 microSD card
- Premium suction mount, PGYTECH backpack-strap clip with articulating adapter, chest/head mounts and floating grip
- Zeiss Victory SF 8x32 binocular
- GoPro modes and daily clock synchronization are defined in `GoPro-HERO12-Travel-Setup.md` and `data/camera-config-gopro-hero12.json`

## G9 MkII Teleconverter Field Variants

No dedicated custom-mode slots and no pre-emptive changes to the saved wildlife modes.

### Optical ranges and light loss

| Configuration | Actual focal-length range | Full-frame equivalent | Maximum aperture at native 400mm |
|---|---:|---:|---:|
| 100–400mm II alone | 100–400mm | 200–800mm | f/6.3 |
| DMW-TC14 1.4× | 294–560mm | 588–1120mm | approximately f/9 |
| DMW-TC20 2× | 420–800mm | 840–1600mm | approximately f/13 |

With either teleconverter attached, the lens's usable native zoom range is restricted to 210–400mm.

### DMW-TC14 1.4×

Default basis: C3-1 `Stationary / Slow Wildlife`.

- Recall C3-1 without a camera-setting change: Manual, 1/500s, maximum available aperture, Auto ISO, AFC Animal, Zone, Electronic shutter and stabilizer Mode 1.
- The camera automatically changes the available maximum aperture; no manual aperture compensation is required.
- Use C3-2 `Wildlife Action` only when the subject is genuinely active, light is strong and the resulting ISO remains acceptable.
- Avoid C3-3 as the routine choice because 1/4000s at approximately f/9 drives ISO sharply upward.

### DMW-TC20 2×

Default basis: C3-1 `Stationary / Slow Wildlife`.

- Recall C3-1 without a camera-setting change: Manual, 1/500s, approximately f/13 at the long end, Auto ISO, AFC Animal, Zone and Electronic shutter.
- Normal conditions: stationary or slow subject, good light, tripod available and relatively clean air without severe haze or heat distortion.
- Do not use C3-2 or C3-3 routinely with the 2×.

### Required physical setup

Before attaching either teleconverter:

1. Rotate the lens zoom to at least 250mm toward the telephoto end.
2. Set the lens `ZOOM LIMIT` switch to `ON`.
3. Attach the teleconverter.
4. Do not force the zoom below the restricted native 210mm position.

Field controls:

- Locked tripod head and fixed composition: lens O.I.S. switch Off.
- Loose head, gimbal or active subject tracking: lens O.I.S. On and camera Mode 1.
- Distant subject beyond 5m: lens focus-range selector at `5m–∞` to improve AF speed.
- Branches or foreground clutter confusing Zone AF: temporarily change to 1-Area.

Other decisions:

- Teleconverters are field variants of wildlife modes, not separate custom modes.
- 1.4× loses 1 stop; 2× loses 2 stops.
- Moon photography is a variation of C3-1, not a dedicated mode.
- Astro/Night and General Travel Video do not receive custom slots.
- Atomos Shinobi II stays home.

## Confirmed Q.Menu Redesign

Status: G9 MkI layout corrected against the firmware-2.7-compatible manual; G9 MkII remains subject to its own firmware-2.7 Menu Atlas.

Design rule:

- Slots 1–8 retain the approved cross-camera muscle-memory pattern.
- G9 MkI slots 9–12 use only items established by the G9 manual. `Shutter Delay` remains in My Menu unless the firmware-2.7 add-item screen explicitly offers it.
- G9 MkII slots 9–12 remain unchanged until its Menu Atlas verifies their exact names and availability.

| Slot | G9 MkI | G9 MkII |
|---:|---|---|
| 1 | Flash Mode | AF Detection Setting |
| 2 | Flash Adjust. | Detecting Subject |
| 3 | Stabilizer | Image Stabilizer |
| 4 | Metering Mode | Metering Mode |
| 5 | Quality | Picture Quality |
| 6 | Shutter Type | Shutter Type |
| 7 | Bracket | Bracketing |
| 8 | Burst Shot Setting | AF Custom Setting(Photo) |
| 9 | Min. Shtr Speed | Focus Peaking |
| 10 | Photo Style | Minimum Shutter Speed |
| 11 | Aspect Ratio | Photo Style |
| 12 | Peaking | Aspect Ratio |
| 13–15 | Empty | Not applicable |

G9 MkI removals:

- Remove `i.Dynamic`: low priority for the RAW-centered workflow.
- Remove `Ex. Tele Conv.`: the G9 MkI is no longer the principal wildlife body.

G9 MkI additions:

- `Burst Shot Setting`: select the active Burst I/Burst II preset; exact Rec entries remain `Burst Shot 1 Setting` and `Burst Shot 2 Setting`.
- `Shutter Delay`: keep in My Menu unless it appears explicitly in the firmware-2.7 Q.Menu add-item screen.
- `Peaking`: exact G9 MkI name; quick MF display toggle for macro.

G9 MkII removals:

- Remove `Flash Mode` and `Flash Adjust.` because no planned G9 MkII mode uses flash.

G9 MkII implementation correction:

- The firmware-2.7-compatible manual's complete Q.Menu registration list on pp. 547–550 does not include `Burst Shot Setting` or `Shutter Delay`; the camera add-item screen confirmed the omission.
- Slot 8 is therefore `AF Custom Setting(Photo)` for immediate wildlife AFC behavior changes.
- Slot 9 is `Focus Peaking` for immediate landscape/manual-focus display control.
- `Burst Shot Setting` and `Shutter Delay` remain in My Menu for detailed access.

Constant Preview decision:

- Keep Constant Preview Off in G9 MkI C3-1, C3-2 and C3-3.
- When a flash is recognized, the camera normally ignores Constant Preview; keeping it Off is still the safer fallback if the X3 is powered off, disconnected or not recognized.
- Constant Preview does not receive a Q.Menu slot.

## Confirmed My Menu Redesign

Status: G9 MkI layout corrected against the firmware-2.7-compatible manual; G9 MkII remains subject to its Menu Atlas.

Design rule:

- Q.Menu is for immediate field changes.
- My Menu is for detailed configuration, calibration and maintenance.
- Preserve the same functional priorities where possible, but do not force identical names or slot counts across cameras.

### G9 MkI exact slots

| Slot | Exact item | Purpose |
|---:|---|---|
| 1 | Cust.Set Mem. | Re-save a mode after an approved calibration. |
| 2 | Bracket | Change Type, Step, Count and Sequence. |
| 3 | Burst Shot 1 Setting | Configure the physical Burst I preset. |
| 4 | Burst Shot 2 Setting | Configure the physical Burst II preset. |
| 5 | Shutter Delay | Adjust supported-mode delay. |
| 6 | ISO Sensitivity (photo) | Change the Auto ISO upper limit. |
| 7 | Long Shtr NR | Exceptional use for very long exposures. |
| 8 | Peaking | Change peaking color and sensitivity. |
| 9 | Sensor Cleaning | Field maintenance. |
| 10 | Format | Deliberate formatting, kept below photographic controls to reduce accidental access. |
| 11 | Fn Button Set | Maintain button assignments. |
| 12 | Time Lapse/Animation | Occasional access, including possible Capsule Pro experiments. |
| 13 | Zebra Pattern | Change the zebra threshold. |

### G9 MkI decisions

- Stop at slot 13; the camera supports up to 23 My Menu entries, but no further G9 MkI entries are approved. `Rec Quality` was removed because general video is outside the approved scope.
- Remove `AF Custom Setting (Photo)`: C1/C2 use AFS and the C3 modes use MF.
- Remove `Focus Limiter`: the planned macro modes use MF.
- Remove `Ex. Tele Conv.`: wildlife has moved to the G9 MkII.

### G9 MkII exact slots

The G9 MkII uses one combined `Burst Shot Setting` item rather than the G9 MkI's two separate burst presets. `Rec Quality` remains deliberately excluded because general video is outside the project scope. The resulting final list contains 17 entries.

| Slot | Exact item | Purpose |
|---:|---|---|
| 1 | Save to Custom Mode | Re-save a mode after an approved calibration. |
| 2 | Bracketing | Change bracket type and detailed parameters. |
| 3 | Burst Shot Setting | Configure Burst I, Burst II and pre-burst behavior. |
| 4 | Shutter Delay | Adjust supported-mode delay. |
| 5 | ISO Sensitivity (photo) | Change Auto ISO limits. |
| 6 | Long Exposure NR | Exceptional use for very long exposures. |
| 7 | Focus Peaking | Change peaking display details. |
| 8 | Sensor Cleaning | Field maintenance. |
| 9 | Card Format | Deliberate formatting below photographic controls. |
| 10 | Fn Button Set | Maintain button assignments. |
| 11 | Time Lapse/Animation | Occasional interval and animation access. |
| 12 | Zebra Pattern | Change zebra thresholds. |
| 13 | AF Custom Setting(Photo) | Change wildlife AFC response Set 1–4. |
| 14 | Focus Limiter | Apply a precise AF range when the lens selector is insufficient. |
| 15 | High Resolution Mode Setting | Adjust RAW, XL, motion blur handling and simultaneous normal frame. |
| 16 | Live View Composite | Access the night/lightning variation. |
| 17 | Custom Mode Settings | Maintain titles, mode count and reload behavior. |

G9 MkII removals:

- Remove `Ex. Tele Conv.` because the workflow uses optical teleconverters rather than a digital crop.
- Remove `Sound Rec Level Adj.` because C3-10 uses the internal microphone at default levels; the adjustment remains available in the normal menu.
- Remove redundant entries already available directly in Q.Menu.

## Confirmed Fn Button Redesign

Status: assignments confirmed for both cameras. The G9 MkI target record is updated and its final physical implementation is pending; the G9 MkII record remains unchanged until its own validation completes.

### G9 MkI

Preserve:

- Fn1: AF-ON
- Fn2: Q.Menu
- Fn3: LVF/Monitor Switch
- Fn4: AF-Point Scope
- Fn5: Preview
- ISO button: Sensitivity
- WB button: White Balance
- `+/-` button: Exposure Compensation
- Joystick: D.FOCUS Movement
- Video record button: Video Record

Approved change:

| Control | Current | Target | Reason |
|---|---|---|---|
| Function Lever | Silent Mode | Stabilizer; Mode 2 Setting = Off | Silent Mode forces flash off and can cause confusing macro failure; Stabilizer Off directly supports the handheld-versus-supported macro split. |

Function Lever operating rule:

- Mode 1: use the stabilizer state saved in the recalled custom mode.
- Mode 2: force stabilization Off.
- C1, C2, C3-1 and C3-2 handheld: leave the lever in Mode 1.
- C3-3 supported: Mode 2 is appropriate, although the custom mode already saves stabilization Off.
- Mark or memorize Mode 2 as `IS OFF`; leaving it engaged accidentally is less harmful than silently disabling macro flash, but it can still remove stabilization from handheld modes.

### G9 MkII

No changes:

- Fn1: AF-ON Near Shift
- Fn2: AF-Point Scope
- Fn3: Preview
- Dedicated AF-ON: AF-ON
- Dedicated Q: Q.Menu
- Dedicated AF Mode: AF Mode
- Lens Fn on 100–400mm II: AF-ON
- ISO, WB, Exposure Compensation, LVF and Video Record remain on their dedicated controls.

Rationale:

- Near Shift directly addresses wildlife AF grabbing the background.
- Lens AF-ON remains useful with tripod or monopod support.
- The existing G9 MkII physical-control layout already matches its wildlife specialization.

## Pending Detailed Work

Decisions completed:

- Reviewed every target mode against its exact current baseline.
- Confirmed the complete slot map for both cameras without conflicts.
- Confirmed all G9 MkII specialized wildlife, landscape, lightning, High Resolution and wildlife-video modes.
- Confirmed all G9 MkI specialized macro modes.
- Unified C1 General/Street and C2 Portrait across both bodies.
- Established teleconverter conditions, base modes and field adjustments.

Physical calibration and preparation still required:

- G9 MkI macro calibration is complete. Apply the final common settings and C1/C2 changes, re-save all five modes, pass the power-cycle recall/capture checks, and create the verified `G9PARKS.DAT` backup using `G9MkI-Final-Implementation-and-Backup-Checklist.md`.
- Calibrate G9 MkII C3-6 landscape Focus Bracket Step 3 and 30 frames at 12mm, 25mm and 35mm, including actual software merges.
- Test the MIOPS Smart+, correct camera cable, Prefocus, Lightning Mode sensitivity and false-trigger behavior.
- Write the K&F filter field guide.
- G9 MkI Q.Menu and My Menu are implemented; its final controls and mode closure pass remain pending. G9 MkII implementation work remains tracked separately.

Implementation gate:

- The G9 MkI approved target JSON and camera-in-hand runbook are ready. Mark that body implemented-and-validated only after its runbook closure gate and `G9PARKS.DAT` verification pass.
- Keep G9 MkII configuration status independent; do not close it from G9 MkI evidence.
