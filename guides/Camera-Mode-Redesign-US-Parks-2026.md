# Camera Mode Redesign — US National Parks 2026

Date started: 2026-08-07

Status: Decision and migration ledger. Do not change the camera configuration JSON files until the implementation phase.

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
| C3-1 | `Slow / Still Wildlife` |
| C3-2 | `Wildlife Action` |
| C3-3 | `Fast / Erratic Wild` |
| C3-4 | `Handheld Landscape` |
| C3-5 | `Tripod HDR Landscape` |
| C3-6 | `Focus Stack Landscape` |
| C3-7 | `Long Exposure + ND` |
| C3-8 | `Lightning - MIOPS` |
| C3-9 | `Tripod High Res` |
| C3-10 | `Wildlife Video` |

The G9 MkI mode labels remain conceptual labels in the project documentation unless the camera presents an editable title field. Do not infer or force a naming function that is not shown by the camera.

## Implementation Rule

For every target mode:

1. Identify the current mode used as its baseline.
2. Preserve every existing setting unless a change is explicitly approved.
3. Record each approved change as `current → target` with its reason.
4. Change the JSON configuration only after the full migration ledger has been reviewed.

## Global Set-and-Forget Review

Status: review in progress. Global settings will be reviewed and approved before the camera configuration JSON files are changed.

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
| Copyright Information | On | On | Embed authorship information in captured files. |
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
| Diffraction Compensation | On | Off | Avoid automatic processing and possible increased high-ISO noise; align with the G9 MkII RAW workflow. |

Clarification:

- Turning Diffraction Compensation off does not change the approved f/16 aperture in the G9 MkI macro modes.
- The accepted f/16 depth-of-field versus diffraction tradeoff remains. In-camera compensation cannot restore all optical detail lost to diffraction in the RAW capture.
- The G9 MkII already has i.Dynamic Range, Vignetting Compensation and Diffraction Compensation off for still photography, so no equivalent change is required.

### Global Autofocus and Shutter Behavior

Status: approved.

Preserve on both cameras:

- Back-button focus: Shutter AF off and AF-ON assigned to the rear control.
- Half-Press Shutter off.
- AFS Focus/Shutter Priority set to Focus.
- AF+MF on.
- MF Assist enabled with PIP display and MF Guide on.
- Focus Peaking on, red and Low sensitivity.
- Quick AF and Eye Sensor AF off.
- Global Focus Limiter off.
- AF Micro Adjustment off.
- AF Custom Setting (Photo) Set 1.
- Focus Switching for Vertical/Horizontal off.
- Looped Focus Frame off.

Approved changes:

| Camera | Setting | Current | Target | Reason |
|---|---|---|---|---|
| G9 MkII | AFC Focus/Shutter Priority | Focus | Balance | Reduce burst interruptions caused by strict focus confirmation while retaining more focus discipline than Release priority. |
| G9 MkII | AF Assist Light | On | Off | Its short effective range does not help normal 100–400mm wildlife distances and the light may attract or disturb animals. |
| Both | Lens Focus Resume | Off | On | Preserve the last focus distance across power cycles, particularly useful for manual-focus macro work and deliberate prefocusing. |
| G9 MkI | Burst Shot 1 | H | L, 2 fps | Make Burst I the approved normal low-recycle macro burst. |
| G9 MkI | Burst Shot 2 | `SH75` (invalid copied setting) | M, 7 fps | Provide the approved higher-cadence 3–5 frame manual-flash test option. SH75 does not exist on the G9 MkI. |
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
- Level Gauge on.
- Focal Length display on.
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
| G9 MkI | Blinking Highlights | Off | On | Identify clipped areas during playback, particularly useful for macro flash and portraits. |
| G9 MkII | Blinking Highlights | Off | On | Identify clipped areas during playback for landscapes, wildlife and other subjects. |

Exceptions and clarification:

- Histogram remains off on the G9 MkI because a live histogram does not represent the final flash-lit macro exposure reliably.
- Constant Preview remains off in all three approved G9 MkI macro custom modes.
- With Auto Review off, Blinking Highlights is seen when an image is deliberately opened in playback; it will not interrupt capture automatically.

### Global Stabilization and Lens Behavior

Status: reviewed; no additional changes beyond previously approved settings.

Preserve:

- Stabilization on with normal handheld behavior as the global baseline on both cameras.
- Explicit stabilization overrides in every approved custom mode: tripod modes off, G9 MkI handheld macro Mode 1, normal G9 MkII wildlife Mode 1, and fast/erratic wildlife Mode 2.
- Focus Ring Control Non-Linear. Linear control is lens-dependent and is not required for the body-movement macro technique.
- Lens Fn Button set to AF-ON.
- Aperture Ring Increment at 1/3 EV.
- AF Micro Adjustment off unless a repeatable, tested focus offset is later demonstrated.
- G9 MkII Lens Information Confirmation on.

Previously approved global change retained:

- Lens Focus Resume on for both cameras.

### Global Power, Monitor and Viewfinder Setup

Status: approved.

Preserve on both cameras:

- Sleep Mode at 2 minutes.
- Auto LVF/Monitor Off at 1 minute.
- Monitor Frame Rate at 60 fps.
- Monitor Backlight / LVF Luminance on Auto.
- Eye Sensor sensitivity Low.
- Thermal Management / Recording Max Temperature at Standard for handheld use.
- All beeps off.
- USB connection selection on connection and USB power on.
- System Frequency at 59.94 Hz NTSC, matching the United States and the approved 60p wildlife video mode.

Approved changes:

| Camera | Setting | Current | Target | Reason |
|---|---|---|---|---|
| G9 MkII | LVF Frame Rate | 60 fps | 120 fps | Provide smoother viewfinder motion for tracking fast wildlife. Increased battery use is accepted and mitigated by the approved power-save timings. |
| G9 MkI | Battery Use Priority | Battery Grip | Body | No battery grip is used; make the setting match the actual equipment. |
| G9 MkII | Battery Use Priority | Battery Grip | Body | No battery grip is used; make the setting match the actual equipment. |

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
| XLR Mic Adaptor Setting | On | Off | No XLR microphone adaptor will be used. |

Preserve on the G9 MkII:

- MP4 format and Full image area.
- 4K30 as the global fallback quality outside C3-10.
- C3-10 4K60, 10-bit, HEVC 100 Mbps quality and all previously approved C3-10 settings.
- Internal microphone, Standard recording gain, 0 dB level adjustment, limiter on, Standard wind-noise cancellation and Lens Noise Cut on.
- Continuous AF Mode 1.
- Zebra 1 at the current 100–105% range.
- Red Recording Frame on.
- Time Code off.
- Auto Exposure in P/A/S/M on.
- Current HDMI output settings. The Shinobi II remains outside the trip workflow.

Preserve the G9 MkI video baseline without changes, including its audio-level display off.

### Global Wireless Connectivity, Clock Synchronization and Geolocation

Status: approved subject to a two-camera field test.

Approved target on both cameras:

| Setting | Target | Reason |
|---|---|---|
| Bluetooth | On | Maintain a low-energy connection for clock synchronization and geolocation. |
| Auto Clock Set | On | Synchronize camera clock and time-zone information from the smartphone when the camera connects, including at power-on. |
| Location Logging | On | Write smartphone-provided GPS coordinates directly to captured-image metadata. |
| Wi-Fi | Off unless actively required | Clock and location use Bluetooth; avoid unnecessary higher-power Wi-Fi operation. |
| Auto Transfer | Off | Avoid large RAW transfers, Wi-Fi activation, battery use and background transfer delays. |
| Remote Wakeup | Off | It is not required for clock or location logging and can continue draining the camera battery while the power switch is off. |

Applications:

- G9 MkI: use Panasonic Image App for complete support, including Auto Clock Set and Location Logging.
- G9 MkII: use LUMIX Sync for the approved workflow. LUMIX Lab remains optional and should not compete for the active connection during the test.
- G9 MkII Network Connection Light remains off.
- G9 MkII USB-SSD remains off.
- USB Mode remains Select on Connection and USB Power remains on.

Required two-camera test before travel:

1. Pair the G9 MkI with Panasonic Image App and the G9 MkII with LUMIX Sync on the same smartphone.
2. Grant both applications always-allowed location access and background operation; disable aggressive battery optimization for them.
3. Enable smartphone GPS, camera Bluetooth, Auto Clock Set and Location Logging.
4. Power both cameras on and wait until each shows a solid location/GPS indicator rather than a translucent unavailable indicator.
5. Photograph the same displayed clock with both cameras.
6. Lock the smartphone screen, move to a meaningfully different location, and photograph again with both cameras.
7. Inspect the files on a computer and verify capture-time agreement, latitude/longitude presence and updated coordinates after movement.
8. Repeat after a camera power cycle and with the phone operating without cellular data.

Fallback if the smartphone cannot maintain both background connections:

- Give live Bluetooth geolocation priority to the G9 MkII because it will capture most landscapes and wildlife.
- Synchronize the G9 MkI clock manually each day and after time-zone changes.
- Preserve a smartphone GPS track so G9 MkI images can be geotagged later by matching capture times if desired.

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
- Complete the approved two-camera Bluetooth clock and geolocation test.

Final backup after programming and validation:

- Create the G9 MkII settings backup `USPK2026` on both primary cards, preserve a computer copy, enable Keep Settings While Format and save a LUMIX Sync settings copy.
- Preserve the G9 MkI through its final JSON, complete programming checklist and photographs of critical menu pages.

## Global Review Completion

Status: all conceptual global set-and-forget categories have been reviewed and approved. Configuration JSON files remain unchanged pending a final consistency audit and the explicit implementation phase.

## G9 MkII Migration Ledger

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
- K&F Concept magnetic 72mm kit: GND8, ND8, ND64 and ND1000, plus 58→72mm step-up ring.
- ND64 is the initial default; a detailed filter field guide will be created later.

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
| High Resolution Picture Size | Not applicable | XL, 100 MP at 4:3 (11552×8672) | Use the maximum available output resolution. |
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

Status: migration and settings confirmed.

Baseline: current G9 MkII C3-10 `Video — Wildlife in Motion`.

Preserve:

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
- Internal microphone with current default levels

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
| AF area | Not uniformly explicit | Full Area with Detection Off | Reproduce the existing G9 MkI C1 behavior on both cameras. |
| Minimum shutter speed | Auto | 1/125s | Protect street photographs from subject movement and modest camera motion. |

Accepted tradeoff:

- Auto ISO will rise earlier in low light because of the 1/125s minimum.
- This is accepted because C1 prioritizes General/Street responsiveness, while deliberate landscapes are assigned primarily to the G9 MkII landscape modes.

Operational intent:

- Primary lens: 12–35mm.
- 35–100mm is an alternate for tighter travel details.
- Recall baseline: Aperture Priority, f/5.6, AFS Full Area, Detection Off, Single Shot, Auto ISO with 1/125s minimum, RAW, AWB, stabilization on and Electronic Front Curtain.

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

- No technical change: preserve Full Area with Face/Eye Detection.
- The focus-system wording and capability differ by camera generation, but the operational intent is the same.

Operational intent:

- Primary lens: 35–100mm f/2.8.
- Environmental alternative: 12–35mm f/2.8.
- Olympus 60mm Macro is an alternate for tighter, deliberate portraits.
- Recall baseline: Aperture Priority, f/2.8, AFS, Human/Face/Eye Detection, Full Area, Single Shot, Auto ISO with 1/125s minimum, Portrait Photo Style, RAW, AWB, stabilization on and Electronic Front Curtain.

### C3-1 — Single Macro — TTL

Status: migration and settings confirmed; TTL recycle-time test still required.

Baseline: current G9 MkI C3-2 `Macro Handheld (Single Shot)`.

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
- Focus Peaking: On, Low sensitivity, red
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
- Preserve f/16 and ISO 200 initially; measure real MF12 TTL recycle time before changing either.
- The Panasonic Wireless menu stays off because radio control is handled by the Godox X3/MF12 system.

### C3-2 — Macro Burst — Manual Flash

Status: migration and initial settings confirmed; flash exposure and recycle calibration still required.

Baseline: current G9 MkI C3-2 `Macro Handheld (Single Shot)` and approved target C3-1 `Single Macro — TTL`.

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
- Focus Peaking, MF Assist and MF Guide: On

Approved changes from target C3-1:

| Setting | C3-1 | C3-2 | Reason |
|---|---|---|---|
| Name | Single Macro — TTL | Macro Burst — Manual Flash | Identify the short-burst and manual-lighting workflow. |
| Drive | Single Shot | Burst I, L speed (2 fps) | Record 3–5 frames at a cadence the flashes can plausibly sustain. |
| ISO | 200 | 400 | Save one stop of flash output and shorten recycling. |
| Flash control | TTL | Manual | Keep exposure and recycling consistent through the burst. |
| Initial MF12 power | TTL-determined | 1/32 on each unit | Provide a conservative low-power starting point for rapid recycling. |
| MF12 balance | TTL | Groups A and B at 1:1 | Start with neutral left/right lighting before creative adjustment. |

Recall baseline:

- Manual, MF, Burst I at L 2 fps, f/16, 1/200s, ISO 400, MF12 groups A/B at 1/32, Mechanical shutter and stabilizer Mode 1.

Operational intent:

- Olympus 60mm Macro with manual-focus rocking.
- Hold the shutter for approximately 1.5–2.5 seconds to produce 3–5 frames.
- The objective is not a guaranteed stack; it is to increase the probability that at least one frame places the critical plane in focus.
- If body movement is sufficiently smooth, the sequence may occasionally support a small software stack, but this is not the mode's success criterion.
- The physical drive dial must be moved to Burst I; a recalled custom mode cannot physically move the dial.

Calibration and field adjustment order:

1. Test with both MF12 diffusers fitted and fully charged batteries.
2. If underexposed, raise ISO from 400 to 640 or 800 first.
3. Only then raise MF12 power from 1/32 to 1/16.
4. If any frame misses a flash, lower flash power or pause; do not raise the burst rate.
5. Also test Burst M at 7 fps with MF12 power at 1/32 and 1/64. Promote M to the saved baseline only if every frame is illuminated consistently; otherwise retain the approved L 2 fps baseline.

### C3-3 — Supported Macro Focus Bracket

Status: migration and initial settings confirmed; X3/MF12 compatibility, recycle and merge calibration still required.

Baseline: current G9 MkI C3-2 `Macro Handheld (Single Shot)`, adapted for an automatic supported stack. The current C3-3 `Wildlife / Action` is replaced because wildlife specialization moves to the G9 MkII.

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
- Focus Peaking, MF Assist and MF Guide: On

Approved changes:

| Setting | Current Macro Single | Target | Reason |
|---|---|---|---|
| Slot | C3-2 | C3-3 | Reserve the third specialized slot for the low-priority supported stack. |
| Name | Macro Handheld (Single Shot) | Supported Macro Focus Bracket | Identify the supported automatic focus-stack workflow. |
| Camera support | Handheld | Tripod or firm surface | Keep framing and perspective fixed through the stack. |
| Aperture | f/16 | f/8 | Let stacking provide depth, reduce diffraction and require four times less flash energy than f/16. |
| ISO | 200 | 400 | Save one additional stop of flash energy. |
| Flash control | TTL | TTL initially | Retain automatic exposure and test whether the Godox system communicates recycle readiness reliably. |
| Stabilizer | On, Mode 1 | Off | The camera is supported. |
| Bracketing | Off | Focus Bracket | Record the automatic focus sequence. |
| Focus bracket step | Not applicable | 2 | Favor conservative overlap between focus planes. |
| Focus bracket image count | Not applicable | 40 | Provide a practical initial range for the 60mm Macro. |
| Focus bracket sequence | Not applicable | 0/+ | Start at the nearest required detail and move toward the background. |
| Shutter delay | Off | 2 seconds | Allow vibration from pressing the shutter to settle before the sequence. |
| Physical drive dial | Single Shot | Single Shot | Allow the camera to execute the programmed image count automatically. |

Recall baseline:

- Manual, MF, Single Shot, f/8, 1/200s, ISO 400, TTL, Mechanical shutter, stabilizer off, Focus Bracket Step 2, 40 images, sequence 0/+ and 2-second delay.

Operational intent:

- Supported camera and static subject; no handheld focus-stacking expectation in this mode.
- Not emphasized as the principal macro workflow.
- Compared with C3-1, f/8 and ISO 400 reduce theoretical flash-energy demand by three stops.
- MF12 pair controlled by X3 is the initial light; AD100Pro remains an optional off-camera variation.

Required validation before implementation:

1. Use a fully static test subject, supported camera, charged MF12 units and fitted diffusers.
2. Confirm that all 40 TTL frames receive consistent flash exposure.
3. Merge the sequence in Helicon Focus and inspect it for focus gaps and alignment artifacts.
4. If TTL frames are inconsistent or recycle cannot keep up, test both MF12 units manually at 1/64.
5. If flash synchronization still fails, test the MF12 modeling lights as continuous illumination with an appropriately slower shutter speed.

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

Status: layout confirmed for both cameras; configuration JSON files remain unchanged until implementation.

Design rule:

- Slots 3–12 have the same operational meaning on both bodies to reinforce muscle memory.
- Slots 1–2 hold the camera-specific priorities: macro flash on the G9 MkI and subject detection on the G9 MkII.

| Slot | G9 MkI | G9 MkII |
|---:|---|---|
| 1 | Flash Mode | AF Detection Setting |
| 2 | Flash Adjust. | Detecting Subject |
| 3 | Image Stabilizer | Image Stabilizer |
| 4 | Metering Mode | Metering Mode |
| 5 | Quality | Picture Quality |
| 6 | Shutter Type | Shutter Type |
| 7 | Bracketing | Bracketing |
| 8 | Burst Shot Setting | Burst Shot Setting |
| 9 | Shutter Delay | Shutter Delay |
| 10 | Minimum Shutter Speed | Minimum Shutter Speed |
| 11 | Photo Style | Photo Style |
| 12 | Aspect Ratio | Aspect Ratio |
| 13 | Focus Peaking | Not available in the G9 MkII 12-item grid |
| 14–15 | Empty | Not applicable |

G9 MkI removals:

- Remove `i.Dynamic`: low priority for the RAW-centered workflow.
- Remove `Ex. Tele Conv.`: the G9 MkI is no longer the principal wildlife body.

G9 MkI additions:

- `Burst Shot Setting`: switch and calibrate L/M for C3-2.
- `Shutter Delay`: supported C3-3 control.
- `Focus Peaking`: quick MF display toggle for macro.

G9 MkII removals:

- Remove `Flash Mode` and `Flash Adjust.` because no planned G9 MkII mode uses flash.

G9 MkII additions:

- `Burst Shot Setting`: wildlife H/M/L adjustment.
- `Shutter Delay`: tripod, remote and bracketing variation.

Constant Preview decision:

- Keep Constant Preview Off in G9 MkI C3-1, C3-2 and C3-3.
- When a flash is recognized, the camera normally ignores Constant Preview; keeping it Off is still the safer fallback if the X3 is powered off, disconnected or not recognized.
- Constant Preview does not receive a Q.Menu slot.

## Confirmed My Menu Redesign

Status: layout confirmed for both cameras; configuration JSON files remain unchanged until implementation.

Design rule:

- Q.Menu is for immediate field changes.
- My Menu is for detailed configuration, calibration and maintenance.
- Slots 1–13 have the same operational meaning on both bodies.

### Common slots

| Slot | Item | Purpose |
|---:|---|---|
| 1 | Save to Custom Mode / Cust.Set Mem. | Re-save a mode after an approved calibration. |
| 2 | Bracket — Detailed Settings | Change Type, Step, Count and Sequence. |
| 3 | Burst Shot Setting | Configure H/M/L in detail. |
| 4 | Shutter Delay | Adjust supported-mode delay. |
| 5 | ISO Sensitivity — Photo | Change the Auto ISO upper limit. |
| 6 | Long Shutter NR | Exceptional use for very long exposures. |
| 7 | Focus Peaking Set | Change peaking color and sensitivity. |
| 8 | Sensor Cleaning | Field maintenance. |
| 9 | Card Format | Deliberate formatting, kept below photographic controls to reduce accidental access. |
| 10 | Fn Button Set | Maintain button assignments. |
| 11 | Rec Quality | Occasional video-format change. |
| 12 | Time Lapse/Animation | Occasional access, including possible Capsule Pro experiments. |
| 13 | Zebra Pattern Set | Change the zebra threshold. |

### G9 MkI decisions

- Stop at slot 13; slots 14–16 may remain empty.
- Remove `AF Custom Setting (Photo)`: C1/C2 use AFS and the C3 modes use MF.
- Remove `Focus Limiter`: the planned macro modes use MF.
- Remove `Ex. Tele Conv.`: wildlife has moved to the G9 MkII.

### G9 MkII additional slots

| Slot | Item | Purpose |
|---:|---|---|
| 14 | AF Custom Setting — Photo | Change wildlife tracking behavior Set 1–4. |
| 15 | Focus Limiter | Apply a more precise AF range when the physical lens selector is insufficient. |
| 16 | High Resolution Mode Setting | Adjust RAW, XL, Motion Blur Mode and simultaneous normal frame. |
| 17 | Live View Composite | Access the night-lightning variation. |
| 18 | Custom Mode Settings | Maintain titles, mode count and reload behavior. |

G9 MkII removals:

- Remove `Ex. Tele Conv.` because the workflow uses optical teleconverters rather than a digital crop.
- Remove `Sound Rec Level Adj.` because C3-10 uses the internal microphone at default levels; the adjustment remains available in the normal menu.
- Remove redundant entries already available directly in Q.Menu.

## Confirmed Fn Button Redesign

Status: assignments confirmed for both cameras; configuration JSON files remain unchanged until implementation.

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

- Calibrate G9 MkI C3-1 MF12 TTL recycle time at f/16 and ISO 200.
- Calibrate G9 MkI C3-2 MF12 exposure and cadence: approved L 2 fps baseline, plus M 7 fps tests at 1/32 and 1/64.
- Validate G9 MkI C3-3 Focus Bracket with X3/MF12 TTL, then manual 1/64 or continuous modeling-light fallback if needed.
- Calibrate G9 MkII C3-6 landscape Focus Bracket Step 3 and 30 frames at 12mm, 25mm and 35mm, including actual software merges.
- Test the MIOPS Smart+, correct camera cable, Prefocus, Lightning Mode sensitivity and false-trigger behavior.
- Write the K&F filter field guide.
- Q.Menu, My Menu and Fn-button assignments have been reviewed and confirmed; implementation remains pending.

Implementation gate:

- Perform a final user review of this migration ledger.
- Only then update the two camera configuration JSON files and produce the step-by-step programming checklist.
