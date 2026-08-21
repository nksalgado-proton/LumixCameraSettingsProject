# Lumix G9 Macro + G9II Wildlife Configuration Project

## Status

Initial planning document. This is a new project track that uses the previous unified G9/G9II configuration as a reference, but does not replace it.

Reference project:
- `guides/PROJECT-NOTES.md` — previous unified travel/Costa Rica configuration
- `data/camera-config-g9mki.json` — previous G9 MkI settings data
- `data/camera-config-g9mkii.json` — previous G9 MkII settings data
- `data/field-cards.json` — previous printable field-card source

## New Goal

Design a two-camera configuration system where each body has a clear specialty:

- **G9 MkI:** primary macro photography camera
- **G9 MkII:** primary birds/wildlife photography camera
- **Both cameras:** still ready for occasional portraits, landscapes, travel, family, and general photos

This is not a full reset. The new project should preserve the best parts of the previous system:

1. Shared operating logic where it matters.
2. Stable Q.Menu positions.
3. Stable My Menu toolbox.
4. Back-button focus as the default focusing philosophy.
5. Simple mode names based on real shooting scenarios.
6. Field cards that are useful in the moment, not just reference documentation.

The big change is that the specialized modes no longer need to be symmetrical. The cameras should feel familiar, but they do not need to do the same job.

## Core Philosophy

The new system is:

**Two specialized camera systems with a shared operating language.**

Shared modes should make it easy to pick up either camera for normal photography. Specialized modes should make each camera excellent at its main job.

Mode names should answer the field question:

> What am I trying to photograph right now?

They should not primarily be named after technical features unless that feature is the real shooting scenario.

Examples:
- Better: "Birds in Flight"
- Weaker: "AFC Tracking Set 3"
- Better: "Tripod Macro Stack"
- Weaker: "Focus Bracketing"

## Camera Mode Naming

Working language:

- `M1` = shared custom mode 1
- `M2` = shared custom mode 2
- `M3+` = specialized modes

Panasonic camera language:

- G9 MkI has `C1`, `C2`, `C3-1`, `C3-2`, `C3-3`
- G9 MkII has `C1`, `C2`, `C3-1` through `C3-10`

For now, this project can discuss modes as `M1`, `M2`, `M3`, etc. Later, we will map them precisely to `C1`, `C2`, and `C3-x` for each body.

## Confirmed Direction

### Shared Modes on Both Cameras

The first two modes should be the same on both cameras:

| Mode | Purpose | Notes |
|------|---------|-------|
| M1 | General / Walkaround | Everyday photos, travel, family, quick documentation, "camera is just ready." |
| M2 | Portrait / People | People, groups, candid portraits, pleasant rendering, face/eye behavior where available. |

### Specialized Modes

After M1 and M2, the cameras diverge:

| Camera | Specialty | Specialized Mode Direction |
|--------|-----------|----------------------------|
| G9 MkI | Macro | Handheld macro, flash macro, available-light macro, tripod stacking, high magnification, moving insects, close-up details. |
| G9 MkII | Birds / Wildlife | General wildlife, perched birds, birds in flight, foliage/background problems, pre-burst, crop zoom, low light wildlife, wildlife video. |

## Important Constraint: G9 MkI Has Fewer Slots

The G9 MkI only has five custom mode slots total:

| Slot | Likely Role |
|------|-------------|
| C1 / M1 | Shared General |
| C2 / M2 | Shared Portrait |
| C3-1 / M3 | Macro specialty |
| C3-2 / M4 | Macro specialty |
| C3-3 / M5 | Macro specialty or backup general/wildlife |

This means the G9 MkI mode design must be very selective. We cannot create a separate mode for every macro technique.

The design question is:

> Which three macro scenarios deserve permanent mode slots on the G9?

## Initial Mode Skeleton

This is only a starting point for discussion, not a confirmed map.

| Mode | G9 MkI Candidate | G9 MkII Candidate |
|------|------------------|-------------------|
| M1 | General / Walkaround | General / Walkaround |
| M2 | Portrait / People | Portrait / People |
| M3 | Macro Flash Handheld | Wildlife General |
| M4 | Macro Available Light | Birds Perched / Static |
| M5 | Tripod Macro Stack | Birds in Flight |
| M6 | N/A | Birds in Foliage / Near Subject Priority |
| M7 | N/A | Pre-Burst / Action Moment |
| M8 | N/A | Crop Zoom / Extra Reach |
| M9 | N/A | Low Light Wildlife |
| M10 | N/A | Wildlife Video |

Possible G9 MkI alternatives:

| Alternative | Why It Might Matter |
|-------------|---------------------|
| Moving Insects / Fast Macro | If the camera is often used for active insects where flash and AF behavior differ from static macro. |
| High Magnification Macro | If Raynox, extension, or high-magnification setups need meaningfully different defaults. |
| Flowers / Close-Up Detail | If non-insect macro and detail shots are common enough to justify a softer, simpler mode. |
| Backup Wildlife / Action | If the G9 sometimes needs to cover action when the G9II is unavailable or has a different lens mounted. |
| Landscape / Tripod | If the G9 often has the wider lens while the G9II stays on the wildlife lens. |

## Hardware Decisions To Revisit

This section is intentionally open. We should update it before designing final modes.

### G9 MkI Macro System

Current preferred field macro gear:

- Panasonic Lumix G9 MkI body
- Olympus M.Zuiko 60mm f/2.8 Macro lens
- Pair of Godox MF12 macro flashes mounted at the front of the lens
- Godox X3 wireless trigger

Second macro hardware configuration to design around:

- Panasonic Lumix G9 MkI body
- OM System / Olympus M.Zuiko Digital ED 90mm f/3.5 Macro IS PRO
- Optional OM MC-20 2x teleconverter
- Same Godox X3 / flash ecosystem where practical

Initial understanding of the 90mm configuration:

- Produces excellent results when focus is nailed.
- Harder to nail focus than the 60mm setup.
- More working distance than the 60mm, useful for skittish subjects and better subject access.
- Higher native magnification ceiling than the 60mm.
- With MC-20, becomes an extreme magnification configuration and should be treated as a separate difficulty class.
- Weather sealing is better on the 90mm lens itself than on many non-PRO lenses, but system weather resistance is limited by the body, teleconverter, flash trigger, flash units, and any open ports/accessories.
- Field use is occasional, not the primary G9 macro workflow.
- Home/lab-style controlled macro is a major strength. Best waterdrop reflection photos so far were made with the 90mm setup.

Preferred field focusing workflow:

- Set magnification manually on the lens.
- Focus by rocking the whole camera/body forward and backward until the desired plane is sharp.
- The primary macro mode should therefore be **MF-first**, optimized for focus confirmation and flash exposure consistency rather than autofocus tracking.
- Ideal M3 behavior: once the subject is in tentative focus, fire a short burst to increase the chance of catching the eye or another useful focus plane. Single shots work, but they leave too much to timing luck.

Key pain point:

- Flash interaction is the bottleneck. TTL gives good single-shot results, but is too slow and too variable for burst shooting.
- M3 needs a tested manual-flash burst recipe: flash power + shutter mode + burst speed + aperture + ISO.

Initial technical constraints from manuals/specs:

- G9 MkI flash requires a flash-compatible shutter mode. Electronic shutter disables flash, and the super-high burst modes (`SH1`, `SH2`, and pre-burst variants) are electronic-shutter modes.
- Normal G9 flash sync is up to 1/250s according to the camera manual. Previous project cards used 1/200s as a conservative working sync speed.
- Normal G9 burst rates with mechanical shutter / electronic front curtain are approximately `H = 12 fps` in AFS/MF, `M = 7 fps`, and `L = 2 fps`.
- Godox MF12 supports TTL, Auto, and Manual flash. Manual output ranges from 1/128 to 1/1.
- Godox MF12 published recycle range is approximately 0.01s to 1.7s, depending on power.
- Godox MF12 published flash duration range is approximately 1/1200s to 1/34000s, depending on power.

Working inference:

- The best burst-macro configuration will almost certainly use Manual flash, not TTL.
- The practical burst ceiling is not the G9's maximum mechanical burst speed. It is the MF12 recycle time at the chosen manual power.
- `L` burst around 2 fps is likely the safe baseline.
- `M` burst around 7 fps may work only at very low power, probably around 1/128 or 1/64, and only if exposure is still sufficient.
- `H` burst around 12 fps is probably too optimistic for reliable evenly lit flash bursts unless flash power is extremely low and the subject is very close.

Tests needed:

1. MF12 manual 1/128, G9 mechanical or EFC, burst `L`, f/8, ISO 400.
2. MF12 manual 1/64, G9 mechanical or EFC, burst `L`, f/8, ISO 400.
3. MF12 manual 1/128, burst `M`, f/8, ISO 400.
4. Repeat promising settings at f/11 and ISO 400/800.
5. Check whether EFC behaves reliably with the X3/MF12 on the G9 MkI; if not, use mechanical.
6. Check whether the X3's TTL-to-manual conversion can be useful: take a good TTL single shot, convert/replicate its output in manual, then burst at the nearest sustainable lower power.

### Candidate M4: Natural Light Macro / No Flash

Purpose:

- Macro and close-up work when the subject is in usable natural light and flash is unnecessary, undesirable, or physically awkward.
- This mode removes flash recycle as a bottleneck, allowing faster bursts and more fluid shooting.
- It is not a replacement for M3. It is a different look and a different set of constraints.

Best scenarios:

- Subject is exposed to direct sun, bright open shade, or strong reflected light.
- Larger macro subjects where extreme depth of field is less critical.
- Flowers, leaves, textures, mushrooms, lizards, frogs, butterflies, dragonflies, and environmental close-ups.
- Subjects where flash would create harsh reflections, unnatural specular highlights, or a "night macro" look.
- Situations where the lens-front MF12 rig is cumbersome or too close to the subject.

Main advantages:

- No flash recycle delay.
- Burst can use normal camera burst speeds.
- No dependency on X3/MF12 batteries, connection, or flash mode.
- More natural rendering of ambient color and background light.
- Easier to include environment/background in the image.
- Electronic shutter and silent operation become available if rolling shutter risk is acceptable.

Main costs:

- Motion is frozen by shutter speed, not flash duration.
- Wind and subject movement become much harder.
- ISO may rise quickly on Micro Four Thirds.
- Depth of field competes directly with shutter speed and ISO.
- At high magnification, available light may simply not be enough for sharp handheld results.

Working design direction:

- M4 should be ambient-first and burst-friendly.
- It may use Aperture Priority with Auto ISO for ease, or Manual with Auto ISO for a stable shutter-speed floor.
- It should probably start wider than flash macro, around f/4 to f/5.6 for larger subjects or f/8 when light is strong.
- Minimum shutter speed should probably be much faster than normal close-up photography because rocking/body movement and subject movement are amplified at macro distances.
- Candidate shutter-speed targets: 1/500s baseline, 1/1000s for active insects or windy plants, slower only for static subjects.
- Burst `M` or `H` becomes realistic because there is no flash recycle bottleneck.

Open design questions:

1. Should M4 be Aperture Priority + Auto ISO, or Manual shutter/aperture + Auto ISO?
2. Should M4 default to mechanical/EFC for image quality, or electronic shutter for silent high burst?
3. What ISO ceiling is acceptable for this mode: 1600, 3200, or 6400?
4. Is M4 mostly for "pretty natural macro" or for "bursting active subjects without flash"?
5. Should focus remain MF-rocking, or should M4 allow AF/AF+MF for larger subjects?

### Magnification as the Mode Selector

Magnification is a primary decision axis, not a detail.

The same subject can belong in different modes depending on magnification. At low magnification, natural light and high burst are realistic. At high magnification, depth of field collapses, effective aperture increases, camera movement is amplified, and flash becomes much more important.

Working magnification bands for the G9 + Olympus 60mm:

| Magnification | Likely Mode | Notes |
|---------------|-------------|-------|
| 0.1x to 0.3x | M4 Natural Light Macro / Close-Up | Best range for no-flash work. Flowers, textures, larger insects, environmental close-ups. |
| 0.3x to 0.5x | M4 or M3 | Natural light works in strong light; flash becomes safer in shade, wind, or moving subjects. |
| 0.5x to 1.0x | M3 Flash Macro preferred | Natural light becomes demanding. Flash gives consistency and freezes motion. |
| >1.0x | M3 Flash Macro or controlled/tripod mode | Natural light is usually impractical handheld except in very strong sun with high ISO and compromises. |

Olympus 60mm lens scale markings:

| Lens Marking | Approx. Magnification | Focus Distance |
|--------------|-----------------------|----------------|
| 1:1 | 1.0x | 0.19 m |
| 1:1.3 | 0.77x | 0.20 m |
| 1:2 | 0.5x | 0.23 m |
| 1:4 | 0.25x | 0.34 m |
| Infinity | near 0x | infinity |

Field translation:

| Lens Marking | Likely Mode | Notes |
|--------------|-------------|-------|
| Infinity to 1:4 | M4 Natural Light / Close-Up | Best no-flash zone. Good for flowers, larger insects, textures, environmental detail. |
| 1:4 to 1:2 | M4 or M3 | Transition zone. Natural light can work in strong sun; flash safer for shade, wind, and small moving subjects. |
| 1:2 to 1:1.3 | M3 Flash Macro preferred | High enough magnification that flash becomes the dependable choice. |
| 1:1.3 to 1:1 | M3 Flash Macro | Serious macro. Natural light handheld is usually a special-case workflow. |

Raynox DCR-250:

- The project macro calculator models the Raynox DCR-250 as an 8-diopter accessory.
- On a 60mm lens, this adds approximately `+0.48x` effective magnification.
- Practical examples:
  - Lens at 1:4 / 0.25x -> effective about 0.73x
  - Lens at 1:2 / 0.5x -> effective about 0.98x
  - Lens at 1:1 / 1.0x -> effective about 1.48x

Working inference:

- The Raynox should be treated as a high-magnification regime, not merely an accessory.
- Raynox use belongs mostly in M3 Flash Macro or a controlled/tripod macro mode.
- Raynox + natural light is possible only in special conditions: very strong light, static subject, tolerable high ISO, and enough working room to avoid self-shadowing.
- Raynox makes the flash-burst problem more delicate: the subject is closer and may need less flash power, but depth of field is thinner and composition/focus are harder.

Design implication:

- M4 should probably be limited mentally to bare 60mm, mostly 0.1x to 0.5x, with occasional 0.5x to 1.0x only in very good light.
- M3 should cover bare 60mm from 0.5x to 1.0x and Raynox-assisted high magnification.
- A field card should show a "magnification gate" so the decision is fast:
  - Low magnification + good light -> M4
  - High magnification or Raynox -> M3
  - Need maximum depth/control -> tripod/stack mode

### 60mm vs 90mm Macro System

This discussion is frozen at the concept level until field testing, but the hardware split is important.

The 60mm and 90mm should not be treated as interchangeable macro lenses. They create different field behaviors.

| Configuration | Best Use | Main Strength | Main Cost |
|---------------|----------|---------------|-----------|
| 60mm + MF12 pair | Fast field flash macro | Compact, familiar, easier to rock-focus, integrated twin-flash lighting | Shorter working distance, easier to crowd/skittish subjects |
| 60mm + Raynox | High magnification field macro | More magnification while keeping the compact rig | Very thin DoF, special-case natural light, harder composition |
| 90mm bare | Higher-quality long macro | More working distance, better for timid subjects, native 2x S-Macro capability | Harder to hold steady and nail focus, narrower field of view |
| 90mm + MC-20 | Extreme macro | Very high magnification potential | Very hard focus, very thin DoF, likely controlled/tripod/stacking territory |

Initial decision logic:

- Use the 60mm + MF12 pair when the goal is speed, field reliability, and repeated attempts on active subjects.
- Use the 90mm bare when the subject is timid, dangerous, physically hard to approach, or benefits from more working distance.
- Use the 90mm bare when image quality and subject isolation matter more than speed.
- Use the 90mm bare for controlled/home/lab macro when the subject can be staged, stabilized, and refined.
- Use the 90mm + MC-20 when the subject is extremely small and conditions allow a slower, more deliberate workflow.
- Avoid treating 90mm + MC-20 as a casual handheld field mode until proven otherwise.

Weather sealing notes:

- The OM System 90mm f/3.5 Macro IS PRO is officially IP53 weathersealed when used with a compatible OM/Olympus splashproof body.
- The Panasonic G9 MkI is described by Panasonic as splash proof, but Panasonic cautions that this means protection against a minimal amount of moisture/water/dust, not waterproofing.
- The G9 splash-proof behavior depends on all doors/covers being closed and on using a lens designed to support splash-proof operation.
- A mixed Panasonic body + OM lens setup should be treated as weather-resistant, not weatherproof.
- The flash trigger, MF12 units, teleconverter, lens mount interfaces, and any open ports become part of the real system risk.

Practical implication:

- The 90mm is likely the better rainy/misty forest lens than the 60mm if the lens itself is the weak point, but the whole rig is only as resistant as the least-protected component.
- Do not assume the G9 + 90mm + X3 + MF12 system can be used freely in rain.
- For wet environments, prefer conservative handling: keep ports closed, avoid lens changes, protect the trigger/flashes, wipe down gear, and treat water droplets as a warning rather than permission.

Previously discussed or possible secondary hardware:

- Raynox DCR-250
- Godox AD100Pro
- Godox ML-CD15 diffuser
- Tripod

Questions:

1. Is the G9 MkI now permanently paired with the 60mm macro, or should it support multiple macro lenses?
2. Is flash macro the default, or only one of several equal macro workflows?
3. Is handheld macro more important than tripod stacking?
4. Is focus bracketing a field workflow or mostly a controlled/home workflow?
5. Is the Raynox a frequent setup or an occasional high-magnification setup?
6. Do we need separate modes for flash/no-flash, or can one mode handle both through Q.Menu/My Menu?

### G9 MkII Wildlife System

Known or previously discussed hardware:

- Leica DG Vario-Elmar 100-400mm f/4-6.3 II (incoming travel lens)
- Monopod
- Atomos Shinobi II
- GoPro HERO12 Black for video support
- Zeiss Victory SF 8x32 binocular for observation

Questions:

1. Is the 100-400mm the default wildlife lens?
2. Will the G9II usually stay on the long lens while the G9 handles close/general work?
3. How important is video on the G9II versus the GoPro?
4. Should Crop Zoom be a dedicated mode or just a button/Q.Menu behavior?
5. Should pre-burst have a dedicated mode?
6. Do perched birds and general wildlife need separate modes, or can one mode cover both?
7. Should "birds in foliage" be a mode, a button strategy, or an AF-area strategy?

### Shared General Photography

Questions:

1. Which body is more likely to carry the 12-35mm?
2. Which body is more likely to carry the 35-100mm?
3. Should M1 assume Aperture Priority, Program, or Manual with Auto ISO?
4. Should M2 prioritize simplicity, image quality, or face/eye reliability?
5. Should landscapes live inside M1, or does one camera need a dedicated landscape mode?

## What Should Stay Unified

Likely shared across both cameras:

- Back-button focus concept
- Q.Menu philosophy
- My Menu philosophy
- Display behavior
- Silent/no-beep behavior
- Focus peaking and zebra availability
- General button logic where hardware allows
- M1 and M2 purpose

Potentially not shared:

- Specialist modes after M2
- Burst rates
- Subject detection behavior
- Focus bracketing defaults
- Flash defaults
- Stabilizer mode
- Shutter type
- Video defaults

## Design Work Still Needed

1. Update the hardware inventory and remove assumptions from the previous trip project.
2. Decide how specialized the G9 MkI should be, given only three slots after M1/M2.
3. Decide whether G9II mode slots should be broad scenarios or very specific wildlife techniques.
4. Decide whether video deserves one or two G9II custom modes.
5. Decide if landscape deserves a dedicated mode on either camera.
6. Draft final mode maps for both cameras.
7. Only after mode maps are stable, design detailed settings for each mode.
8. Then update data files and regenerate field cards.

## Current Next Conversation

Before designing settings, discuss:

1. New hardware acquired since the previous project.
2. Hardware that is no longer relevant.
3. Which lenses will normally live on which body.
4. Which macro scenarios happen most often.
5. Which wildlife scenarios happen most often.
6. Whether the G9 MkI should remain friendly for general photography or become aggressively macro-specialized.
