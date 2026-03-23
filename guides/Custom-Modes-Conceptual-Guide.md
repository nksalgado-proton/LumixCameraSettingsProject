# Custom Camera Mode System — A Conceptual Guide

**Author:** Nelson Salgado
**Date:** March 2026
**Cameras:** Panasonic Lumix G9 Mark II (12 custom modes) and G9 Mark I (5 custom modes)
**Context:** Designed for a 2-week wildlife + travel trip to Costa Rica

---

## The Problem

Modern mirrorless cameras expose 150–200 configurable parameters. When you switch from photographing a bird in flight to a macro insect to a landscape, dozens of settings need to change: exposure mode, shutter speed, ISO behavior, autofocus mode, subject detection, shutter type, stabilizer mode, flash, drive mode, and more. Changing these one by one in the field is slow, error-prone, and guarantees missed shots.

The solution is a **custom mode system** — a set of pre-configured camera profiles that you recall with a dial turn. Each profile stores the *complete* camera state for a specific shooting scenario.

## Architecture: Baseline + Overrides

Rather than configuring each mode from scratch, the system uses a **layered architecture**:

### Layer 1: The Baseline (~80 "set-and-forget" settings)

These are settings that remain **identical across every mode**. They define how the camera *behaves* regardless of what you're shooting. Here are representative examples from each category:

**Focus behavior:**
- **Shutter AF: OFF** — The shutter button never focuses. Focus is controlled exclusively by a dedicated back button (back-button focus). This separates the "think" (focus) and "act" (shoot) decisions, allowing focus lock, recompose, and selective tracking without the camera fighting you.
- **Focus priority (AFS and AFC): FOCUS** — The camera refuses to fire until the subject is sharp. A blurry frame has zero value; better to miss a frame than capture a soft one.
- **AF+MF: ON** — After autofocus locks, you can immediately fine-tune by turning the focus ring without switching modes.
- **Quick AF / Eye Sensor AF: OFF** — The camera never focuses on its own. It focuses only when commanded via the back button.

**Display and feedback:**
- **Constant Preview: ON** — The viewfinder shows the actual exposure in real time (what you see is what you get). Without this, Manual mode is a guessing game.
- **Auto Review: OFF** — The camera does not display the image after capture. During burst shooting or wildlife, review blocks the live view. You review when you choose.
- **Focus Peaking: ON (Low sensitivity, Red)** — Colored highlights on in-focus edges. Primary visual confirmation for manual focus and macro work.
- **Zebra Pattern: ON (~100%)** — Diagonal stripes on overexposed areas. A real-time blown-highlight alarm for white feathers, bright skies, and flash hotspots.
- **Grid: 9-section (rule of thirds)** — Always visible, never distracting.
- **Level Gauge: ON** — Prevents crooked horizons.

**Exposure behavior:**
- **ISO increments: 1/3 EV** — Fine control. Full-stop jumps (100 → 200 → 400) are too coarse.
- **Extended ISO: OFF** — ISO 50 is a "pull" that sacrifices highlight headroom. Not worth it.
- **Face Priority in Multi Metering: ON** — Prevents backlit portrait silhouettes by biasing exposure toward detected faces.
- **Auto Exposure in P/A/S/M: ON** — If you accidentally hit the video record button during stills, the camera uses auto exposure for video instead of your manual stills settings. Safety net.

**Ergonomics:**
- **All beeps: OFF** — Complete silence. Essential for wildlife (don't scare the bird) and street (don't announce yourself).
- **Touch Pad AF: OFF** — Prevents nose and cheek touches from moving the AF point while using the viewfinder.
- **Power Save: Sleep 2 min, Monitor Off 1 min** — Battery life matters on a multi-week trip.

**The key insight:** these ~80 settings are configured once and saved into every custom mode as an identical foundation. They never vary between modes because they represent *how you want the camera to work*, not *what you're shooting*.

### Layer 2: Mode-Specific Overrides (5–15 settings per mode)

Each custom mode changes only the parameters that matter for its specific scenario. Everything else inherits from the baseline. This means:

- Switching modes changes only what *needs* to change
- You never wonder "did I forget to reset the stabilizer?" because the baseline guarantees it
- Adding a new mode requires defining only the differences

---

## The Modes

### C1: Street / General

**Scenario:** Walk-around photography. Markets, parks, towns, beaches, sightseeing. Unpredictable subjects, mixed lighting, no time to think about settings. This is the "pick up and shoot" mode — it should handle anything competently.

| Parameter | Value | Rationale |
|-----------|-------|-----------|
| Exposure mode | **Aperture Priority (A)** | You control depth of field; camera handles shutter speed. Fastest reaction to changing scenes. |
| Aperture | **f/5.6** | Sweet spot for sharpness across the frame. Wide enough for reasonable shutter speeds, narrow enough for adequate depth of field. Adjust freely with the dial. |
| ISO | **Auto (no upper limit)** | Let the camera choose. In bright Costa Rica daylight, it will stay at base ISO most of the time. |
| Min. Shutter Speed | **Auto** | Camera auto-selects a safe handheld speed based on focal length. With stabilization on, this works well. |
| AF mode | **Single-area** | Medium focus box. You point it at the subject. Predictable, no surprises. |
| Subject detection | **OFF** | Not needed for general scenes. Can be toggled on quickly if you spot a person or animal. |
| White balance | **Auto (AWB)** | Adapts to mixed lighting — sun, shade, indoor, artificial. |
| Photo style | **Standard** | Neutral, balanced look. Good starting point for any scene. |
| Stabilizer | **ON, Normal mode** | Standard stabilization for handheld shooting. |
| Shutter type | **Electronic Front Curtain (EFC)** | Eliminates first-curtain vibration (sharper images) while maintaining flash compatibility. No rolling shutter at typical street shutter speeds. |
| Drive | **Single shot** | One press, one photo. Deliberate shooting. |

**Why Aperture Priority?** In a walk-around context, shutter speed rarely matters — you're not freezing fast action or doing long exposures. Aperture controls your depth of field (how much is in focus), which *is* a creative decision. Let the camera handle the rest.

---

### C2: Portrait / People

**Scenario:** Group shots, posed or candid portraits, people at events. The subject is a person and the goal is a sharp subject with a soft, separated background.

| Parameter | Value | Rationale |
|-----------|-------|-----------|
| Exposure mode | **Aperture Priority (A)** | Control background blur via aperture. |
| Aperture | **f/2.8** | Wide open for maximum background separation. On Micro Four Thirds, f/2.8 gives roughly equivalent depth of field to f/5.6 on full frame — you want every bit of blur available. |
| ISO | **Auto (no upper limit)** | Camera handles sensitivity. |
| Min. Shutter Speed | **1/125s** | Safety floor. People move — even "still" subjects sway, blink, gesture. Below 1/125s you risk motion blur on the subject. Camera will raise ISO rather than drop below this speed. |
| AF mode | **Full Area with Face/Eye detection** | Camera finds faces automatically across the entire frame and locks onto the nearest eye. You compose freely without placing an AF point manually. |
| Subject detection | **Human** | Activates face and eye detection specifically for people. |
| White balance | **Auto (AWB)** | Adapts to skin tones under any lighting. |
| Photo style | **Portrait** | Slightly warmer tones, softer skin rendering. Subtle but flattering for JPEG output. |
| Stabilizer | **ON, Normal mode** | Handheld stability. |
| Shutter type | **EFC** | Sharp images, no vibration. |
| Drive | **Single shot** | Portraits are deliberate. |

**Why f/2.8 and not wider?** On Micro Four Thirds, f/2.8 is the widest available on these zoom lenses. On full frame (like the Sony A6700's APS-C sensor), f/2.8 gives more blur, so you might stop down slightly for group shots to keep everyone in focus.

---

### C3-1: Landscape

**Scenario:** Scenic vistas, travel landscapes, wide compositions. Tripod optional. The goal is maximum sharpness across the entire frame and rich, saturated colors.

| Parameter | Value | Rationale |
|-----------|-------|-----------|
| Exposure mode | **Aperture Priority (A)** | You choose depth of field; camera handles the rest. |
| Aperture | **f/8** | Optimal diffraction-limited sharpness on Micro Four Thirds. Going narrower (f/11, f/16) introduces diffraction softness. On APS-C, f/8–f/11 is the sweet spot. |
| ISO | **Auto (no upper limit)** | Usually base ISO in bright daylight. Camera raises only if needed. |
| AF mode | **Single-area, no detection** | You place the focus point deliberately on the landscape element you want sharpest (typically 1/3 into the scene). Face detection would be distracting. |
| Subject detection | **OFF** | No subjects to detect in a landscape. |
| White balance | **Auto (AWB)** | Handles mixed sky/shadow lighting. Adjust in post from RAW if needed. |
| Photo style | **Scenery/Landscape** | Boosted saturation and contrast for vivid skies and foliage. Makes JPEGs pop. RAW files are unaffected. |
| Stabilizer | **ON, Normal mode** | For handheld. Turn off via quick menu if on tripod. |
| Shutter type | **EFC** | Vibration-free first curtain. |
| Drive | **Single shot** | Landscapes are composed, not sprayed. |

**Why no detection?** Subject detection (face, eye, animal) is designed for discrete subjects within a frame. In landscape photography, the "subject" is the entire scene. Detection would hunt for nonexistent faces or fixate on a distant hiker.

---

### C3-2: Macro Handheld (Single Shot)

**Scenario:** Insects, spiders, small reptiles, flowers at close range. Handheld, using a dedicated macro lens and external flash. No focus stacking — quick single shots of subjects that may move at any moment.

This is the most technically demanding mode because macro photography inverts many normal assumptions: depth of field is measured in millimeters, camera-to-subject distance is centimeters, and ambient light is often insufficient at working apertures.

| Parameter | Value | Rationale |
|-----------|-------|-----------|
| Exposure mode | **Manual (M)** | Full control is essential. At macro distances, even small changes in camera-to-subject distance shift exposure. Aperture Priority would constantly adjust shutter speed as you rock forward and back, creating inconsistent flash exposures. Manual gives repeatable results. |
| Aperture | **f/11** | Balances depth of field against diffraction. At 1:1 magnification on MFT, effective aperture is ~f/22 (2 stops of light loss from extension). This gives roughly 2–3mm of depth of field — enough to get an insect's head in focus. Going wider loses critical depth; going narrower hits severe diffraction. |
| Shutter speed | **1/200s** | Flash sync speed ceiling. The flash freezes the subject (flash duration ~1/4000s at low power), but the shutter must be open long enough to capture the flash pulse. 1/200s is the maximum sync speed with mechanical shutter. Going faster would clip the flash. |
| ISO | **Fixed 200 (base)** | At base ISO, the sensor captures maximum dynamic range with minimal noise. The flash provides all necessary illumination, so high ISO sensitivity isn't needed. Fixed (not Auto) because flash exposure is controlled by flash power and aperture, not ISO. |
| Focus | **Manual (MF)** | Autofocus cannot reliably acquire macro subjects — the lens hunts endlessly at close range. Manual focus via body rocking (moving your body forward/backward to shift the focus plane) is the standard macro technique. Focus peaking (always on in baseline) highlights the in-focus zone. |
| Flash | **ON, TTL mode** | TTL (through-the-lens metering) measures flash exposure through the lens and adjusts power automatically. Essential for handheld macro where distance to subject varies constantly. The diffused flash provides the primary illumination — ambient light at f/11 and 1/200s is usually negligible. |
| Flash compensation | **±0** | Starting point. Increase (+0.3 to +1.0) for dark subjects that absorb light (black beetles) or when using teleconverters that lose light. Decrease (-0.3 to -0.7) for reflective subjects (metallic beetles, wet surfaces). |
| Shutter type | **Mechanical** | Flash requires mechanical shutter. Electronic shutter cannot sync with flash because there is no physical curtain to trigger the flash pulse — the sensor reads line by line, causing partial illumination. |
| Stabilizer | **ON, Mode 1** | Helps frame the subject while rocking into focus. Mode 1 stabilizes all axes (vs. Mode 2 which is for panning). |
| Subject detection | **OFF** | AF detection is irrelevant in MF mode. The camera can't autofocus anyway. |

**Field variants:** This mode has 8 documented variants depending on the lens/accessory combination — different lenses and diopter attachments require adjusted aperture, flash compensation, and ISO. For example, adding a 2x teleconverter loses 2 stops of light, requiring wider aperture, higher flash power (+0.7 to +1.3 compensation), and potentially higher ISO (200–400).

---

### C3-3: Wildlife / Action

**Scenario:** Birds perched or moving, animals, any fast or unpredictable subject at distance. Using a telephoto lens (100–400mm or 35–100mm). The goal is to freeze motion with sharp focus on the animal.

| Parameter | Value | Rationale |
|-----------|-------|-----------|
| Exposure mode | **Manual (M)** | Wildlife lighting is unpredictable — a bird against bright sky vs. dark foliage can swing exposure wildly. Manual with Auto ISO gives you fixed aperture (for depth of field) and fixed shutter speed (to freeze motion), while ISO floats to handle exposure changes. This is the "manual with one auto variable" technique. |
| Aperture | **f/6.3** | Maximum aperture of the 100–400mm at 400mm. You want every photon you can get. On faster lenses (f/2.8), open wide for the same reason. |
| Shutter speed | **1/2000s** | Freezes most bird and animal movement. Walking birds: 1/1000s sufficient. Flying birds: 1/2000s–1/4000s needed. 1/2000s is a practical compromise. Adjust up for BIF, down for static perched birds. |
| ISO | **Auto (no upper limit)** | ISO floats to achieve correct exposure at your fixed aperture and shutter speed. In bright sun, it stays low. In shade or overcast, it climbs. No upper limit because a noisy sharp photo is infinitely better than a clean blurry one. |
| AF mode | **Zone** | A cluster of AF points covering a region of the frame. Wider coverage than single-point (easier to keep a moving bird in the zone) but narrower than full-area (doesn't grab the background). The sweet spot for erratic wildlife movement. |
| Subject detection | **Animal** | Camera identifies and tracks animal bodies, faces, and eyes. When an animal is detected, AF locks onto it even if it moves within or across zones. Eye detection prioritizes the eye for tack-sharp portraits. |
| Drive | **Burst (high speed)** | Continuous shooting at maximum frame rate. Wildlife moments are fleeting — a 1-second burst at 14fps gives 14 chances to capture the perfect wing position, head angle, or expression. |
| Shutter type | **Electronic** | Enables maximum burst frame rate (electronic shutter is faster than mechanical). Also completely silent — no shutter sound to startle wildlife. Rolling shutter artifacts are negligible at 1/2000s+ shutter speeds. |
| Flash | **OFF** | Flash startles wildlife, has insufficient range for telephoto distances, and is incompatible with electronic shutter. |
| Stabilizer | **ON, Mode 1 (Normal)** | Stabilizes all axes for general handheld telephoto shooting. |

**Why Manual + Auto ISO instead of Aperture Priority?** In Aperture Priority, the camera chooses shutter speed — and may choose too slow a speed for a moving bird, producing motion blur. You could set a minimum shutter speed floor, but then you're essentially constraining A-mode into behaving like M-mode with extra steps. Manual + Auto ISO is more direct: you choose aperture (lens-limited anyway), you choose shutter speed (based on subject speed), and ISO is the only variable that floats.

---

### C3-4: Tripod Macro (Focus Bracketing) — *G9 MkII only*

**Scenario:** Tripod-mounted macro with focus bracketing (focus stacking). The camera automatically takes a sequence of images, each focused slightly deeper into the subject, which are later merged in software to create a single image with extreme depth of field — far more than any single exposure can achieve.

| Parameter | Value | Rationale |
|-----------|-------|-----------|
| Exposure mode | **Manual (M)** | Exposure must be identical across all frames in the stack. Any variation causes banding in the merged result. |
| Aperture | **f/8** | Wider than single-shot macro (f/11) because each frame only needs to be sharp at one focus plane. The software combines the sharp slices. Wider aperture means less diffraction, more light, and faster flash recycling. |
| Shutter speed | **1/200s** | Flash sync speed, same as handheld macro. |
| ISO | **Fixed 400** | Slightly higher than handheld macro to allow lower flash power per frame, which means faster flash recycling between bracket shots. Noise at ISO 400 is negligible on modern sensors. |
| Focus | **Manual (MF)** | Set initial focus point at the nearest part of the subject. The camera shifts focus deeper automatically for each subsequent frame. |
| Bracketing | **ON — Focus Bracketing** | Camera takes N frames, shifting focus by a configurable step between each. Step size and frame count depend on magnification and desired depth — configured per session. |
| Shutter delay | **2 seconds** | Delay before first frame to let tripod vibrations from pressing the button die down. |
| Stabilizer | **OFF** | On a tripod, stabilization can actually *cause* micro-vibrations as the IS mechanism hunts. Always off on tripod. |
| Flash | **ON, TTL** | Same flash-based illumination as handheld macro. TTL adjusts power per frame as focus distance shifts slightly. |
| Shutter type | **Mechanical** | Required for flash sync. |

**Why not just use f/22 instead of stacking?** At macro magnifications, diffraction at f/22 severely degrades sharpness — the image turns to mush. Focus stacking at f/8 produces dramatically sharper results because each individual frame is captured at a diffraction-friendly aperture. The depth of field comes from combining dozens of razor-thin focus planes, not from stopping down.

---

### C3-5: Birds with Crop Zoom — *G9 MkII only*

**Scenario:** Distant birds where you need more reach than your longest lens provides. Crop Zoom digitally crops into the center of the frame, effectively multiplying focal length up to 2x, at the cost of switching from RAW to JPEG (the crop is applied in-camera, not to the raw data).

| Parameter | Value | Rationale |
|-----------|-------|-----------|
| Exposure mode | **Manual (M)** | Same rationale as Wildlife/Action. |
| Aperture / Shutter / ISO | **Same as C3-3 (Wildlife)** | Identical exposure strategy — freeze motion, let ISO float. |
| AF / Detection / Drive | **Same as C3-3 (Wildlife)** | Zone AF, Animal detection, burst shooting. |
| Image quality | **JPEG Fine (not RAW)** | Crop Zoom requires JPEG because the crop is performed on the processed image, not the raw sensor data. This is the main trade-off of this mode. |
| Crop Zoom | **ON, minimum image size S (max 2.0x)** | With a 100–400mm lens at 400mm (800mm equivalent on MFT), 2x crop gives 1600mm equivalent field of view. Useful for distant shorebirds, raptors soaring, etc. |

**Trade-off:** You lose RAW flexibility (white balance, exposure recovery, noise reduction in post) in exchange for 2x more reach. Use this mode only when the subject is genuinely too far for the standard Wildlife mode. If the bird is close enough to fill the frame at 400mm, use C3-3 instead and keep RAW.

---

### C3-6: Landscape Tripod / Lightning — Live View Composite — *G9 MkII only*

**Scenario:** Lightning photography using the camera's Live View Composite feature, or any scenario requiring additive long exposure (light trails, star trails). LVC keeps the shutter open and continuously composites only brighter-than-previous pixels into the image — so lightning bolts accumulate while the sky doesn't overexpose.

| Parameter | Value | Rationale |
|-----------|-------|-----------|
| Exposure mode | **Manual (M)** | Exposure for the "base" frame (sky/landscape) is set manually. Lightning bolts are added on top by the composite algorithm. |
| Aperture | **f/8** | Sharp landscape aperture. |
| Shutter speed | **4 seconds** | Base exposure for each composite cycle. The camera takes repeated 4-second exposures, compositing results live. Adjust in field based on ambient brightness. |
| ISO | **Fixed 400** | Low for clean base exposure. Lightning is extremely bright and doesn't need high ISO. |
| White balance | **Daylight (fixed)** | AWB would shift between frames as lightning illuminates the scene differently. Fixed WB ensures consistent color across the composite. |
| Aspect ratio | **16:9** | Wide cinematic framing suits sweeping storm landscapes. Lightning compositions benefit from horizontal space. |
| Focus | **Manual (MF)** | Pre-focus on the horizon or distant landmark, then disable AF. Nothing should change focus during the composite sequence. |
| Stabilizer | **OFF** | Tripod-mounted. IS would introduce micro-vibrations. |
| Live View Composite | **ON** | The defining feature of this mode. Camera enters a special composite mode that only adds brighter pixels to the running exposure. |
| Shutter type | **Mechanical (forced)** | LVC requires mechanical shutter — it's a hardware constraint. |
| Long Exposure NR | **ON (forced)** | LVC forces this on. The camera takes a dark frame to subtract hot pixels. |
| Subject detection | **OFF** | No subjects to detect in a storm landscape. |

**Side effects of LVC:** Silent Mode is disabled (mechanical shutter), maximum ISO is capped at 1600, electronic shutter is unavailable, and bracketing is disabled. These constraints exist because LVC operates at a firmware level that bypasses normal processing paths.

**Alternative variant (MIOPS trigger):** For photographers with an external lightning trigger, the same mode can be reconfigured with LVC off, Long Exposure NR off, RAW quality, and the external trigger handling shutter timing.

---

### C3-7: Indoor / Low Light — *G9 MkII only*

**Scenario:** Dim interiors, restaurants, forest canopy, dusk. Any situation where light is scarce and you need to maximize what the camera can capture without flash. Events, museums, churches, canopy walks.

| Parameter | Value | Rationale |
|-----------|-------|-----------|
| Exposure mode | **Manual (M)** | Precise control of the exposure triangle in difficult light. |
| Aperture | **f/2.8** | Wide open for maximum light gathering. Every fraction of a stop matters in low light. |
| Shutter speed | **1/125s** | Minimum for sharp handheld shots of people. Stabilization helps the camera, but the *subject* can still blur below 1/125s. |
| ISO | **Auto (upper limit 12800)** | Camera pushes ISO high to maintain exposure. 12800 is the practical ceiling where noise remains acceptable for the intended use (social sharing, documentation). Beyond this, images degrade noticeably. |
| AF mode | **Full Area with Face/Eye detection** | Camera finds and tracks faces across the entire frame. In low light, AF is already struggling — giving it the best chance by using the full sensor area and face detection locks. |
| Subject detection | **Human** | Indoor/low light scenarios usually involve people. |
| Silent Mode | **ON** | Completely silent operation — no shutter sound. Essential for concerts, ceremonies, museums, sleeping wildlife. Forces electronic shutter automatically. |
| Shutter type | **Electronic (forced by Silent Mode)** | Silent Mode requires electronic shutter. Rolling shutter risk is low at 1/125s unless subject is moving extremely fast. |
| Photo style | **Natural** | Slightly less contrast than Standard. In harsh indoor lighting (mixed artificial sources), Natural avoids crushed shadows and blown highlights. |
| Flash | **OFF** | Flash would defeat the purpose. Also incompatible with electronic shutter. |
| Stabilizer | **ON, Mode 1** | Critical for handheld low-light shooting. |

**Why not just use ISO Auto with no limit?** You *could*, but at ISO 25600+ on Micro Four Thirds, noise dominates the image. Capping at 12800 forces you to address the fundamental problem (not enough light) rather than papering over it with noise. If 12800 isn't enough, either use flash (different mode), use a tripod (different mode), or accept that the shot isn't possible at this quality level.

---

### C3-8: Birds in Flight (BIF) — *G9 MkII only*

**Scenario:** Dedicated mode for birds actively flying — the most demanding autofocus challenge in wildlife photography. Birds in flight change distance, speed, and direction unpredictably, against complex backgrounds (sky, trees, water).

| Parameter | Value | Rationale |
|-----------|-------|-----------|
| Exposure mode | **Manual (M)** | Same Manual + Auto ISO strategy as general wildlife. |
| Aperture | **f/6.3** | Maximum aperture at 400mm. Every photon counts for fast shutter speeds. |
| Shutter speed | **1/4000s** | Freezes wing motion. At 1/2000s, fast-beating wings (hummingbirds, kingfishers) still show blur. 1/4000s freezes nearly everything except hummingbird wings. |
| ISO | **Auto (no upper limit)** | ISO must float freely to compensate for the extremely fast shutter speed. A grainy sharp BIF shot is usable; a clean blurry one is not. |
| AF mode | **Tracking** | The most aggressive AF mode. Camera locks onto the subject and follows it across the entire frame, continuously adjusting focus. Unlike Zone (which tracks within a region), Tracking follows the subject anywhere. |
| Subject detection | **Animal** | Camera identifies the bird shape and prioritizes it. Eye detection locks onto the bird's eye when visible. Essential — without detection, the camera would grab the sky or trees behind the bird. |
| Drive | **Burst (high speed)** | Maximum frame rate. BIF is a numbers game — 14 frames per second gives you the best chance of capturing peak action (wings fully spread, head turned, talons extended). |
| Shutter type | **Electronic** | Maximum frame rate and complete silence. At 1/4000s, rolling shutter artifacts are imperceptible. |
| Stabilizer | **ON, Mode 2 (Panning)** | Mode 2 stabilizes only the vertical axis, allowing smooth horizontal panning to follow a bird in flight. Mode 1 would fight your panning motion, causing jerky tracking. |
| Flash | **OFF** | Incompatible with electronic shutter and insufficient range for BIF distances. |

**What distinguishes BIF from general Wildlife?** Three things: faster shutter speed (1/4000s vs 1/2000s) to freeze wings, Tracking AF instead of Zone (more aggressive following), and IS Mode 2 for panning. These seem like small changes, but they're the difference between capturing and missing a raptor diving at 200 km/h.

---

### C3-9: Video — Travel / Documentary — *G9 MkII only*

**Scenario:** Simple travel video. Documenting scenes, places, people, food, activities. Point-and-shoot video that looks good with minimal effort. Not cinema — this is for social sharing and trip memories.

| Parameter | Value | Rationale |
|-----------|-------|-----------|
| Exposure mode | **Program (P)** | Camera handles both aperture and shutter speed. For casual video, you don't need creative control — consistent, well-exposed footage is the goal. |
| Resolution | **4K 30p** | High quality without excessive file sizes. 30fps is natural-looking for travel content. 60fps would double storage with minimal benefit for slow-paced scenes. |
| AF mode | **Full Area with Face detection** | Camera tracks faces automatically. Travel video is often people-centric (conversations, guides, markets). |
| Subject detection | **Human** | Primary subjects are people in travel scenarios. |
| Photo style | **Standard** | Balanced look, no heavy color grading needed. |
| Electronic stabilization | **Standard** | Additional software stabilization on top of optical IS. Smooths handheld walking footage. Slight crop (~10%) is worthwhile for stability. "High" crops more aggressively. |
| Shutter type | **Electronic** | Silent recording. |
| Audio | **Internal mic, default levels** | Capture ambient sound. Adjust levels in field if needed. |

---

### C3-10: Video — Wildlife in Motion — *G9 MkII only*

**Scenario:** Video of wildlife behavior — birds feeding, animals moving, action sequences at telephoto distances. Requires manual exposure for consistency and aggressive autofocus.

| Parameter | Value | Rationale |
|-----------|-------|-----------|
| Exposure mode | **Manual (M)** | Consistent exposure across clips. Wildlife moves between shade and sun — auto exposure would pump (brightness oscillating), which looks terrible in video. |
| Aperture | **f/6.3** | Maximum at 400mm, same as stills wildlife. |
| Shutter speed | **1/500s** | Slower than stills BIF because video has its own motion blur aesthetic — the "180-degree rule" suggests shutter speed = 2x frame rate (1/60s for 30p). But 1/60s is too slow for telephoto wildlife. 1/500s is a practical compromise: some motion blur for natural movement, but sharp enough to see detail in freeze-frames. |
| ISO | **Auto (no upper limit)** | Same rationale as stills wildlife — ISO floats. |
| Resolution | **4K 30p** | Matches travel video. Consistent format simplifies editing. |
| AF mode | **Tracking with Animal detection** | Aggressive tracking follows the animal across the frame. |
| Electronic stabilization | **High** | More aggressive stabilization for telephoto handheld video. The additional crop is acceptable — you're already at 800mm equivalent. |
| Photo style | **Standard** | Consistent with travel video. Easy to edit. |

---

## Feature Side-Effects and Incompatibilities

Several camera features have hidden dependencies that constrain mode design:

| Feature | Side Effect |
|---------|-------------|
| **Flash** | Requires mechanical shutter. Electronic and electronic front curtain shutter cannot sync with flash. |
| **Silent Mode** | Forces electronic shutter. You cannot have silence with a mechanical shutter. |
| **Electronic Front Curtain (EFC)** | Maximum shutter speed capped at 1/2000s. Fast action modes must use full electronic shutter. |
| **Live View Composite** | Forces mechanical shutter, forces Long Exposure NR ON, disables Silent Mode, caps ISO at 1600, disables bracketing. |
| **Crop Zoom** | Requires JPEG. Cannot operate on RAW data. |
| **Manual Focus mode** | Subject detection (face/eye/animal) is grayed out. AF detection requires an AF mode to be active. |
| **Manual exposure with fixed SS** | Minimum shutter speed setting is grayed out (it only applies when the camera is choosing shutter speed). |

These constraints mean you cannot, for example, use flash with electronic shutter (macro needs mechanical), use Silent Mode with flash (Silent forces electronic), or shoot RAW with Crop Zoom. The mode design accounts for all of these.

---

## Implementing This on a Sony A6700

The Sony A6700 supports a very similar custom mode architecture. It offers **three "Memory Recall" (MR) slots** on the mode dial (MR1, MR2, MR3) plus additional slots accessible via the menu — functionally equivalent to Panasonic's C1/C2/C3 system. Each MR slot stores the complete camera state including exposure mode, AF settings, drive mode, and all menu parameters.

The baseline + overrides approach translates directly: configure all your "set-and-forget" settings first (back-button AF via `AF w/ shutter: OFF`, focus peaking, zebras, display settings, etc.), save that state as your first MR slot, then modify only the mode-specific parameters for each subsequent slot and save. Sony's **"Recall Custom hold"** feature is particularly powerful — it lets you temporarily activate a custom mode by holding a function button, reverting to your current settings when released. This could be mapped to your BIF or macro profile for instant switching without touching the mode dial.

Key Sony equivalents: Subject detection → Sony's Real-time Recognition AF (animal, bird, insect, human, etc. — Sony's detection categories are more granular than Panasonic's). Zone AF → Sony's Zone or Tracking. Electronic shutter → Sony's Silent Shooting. EFC → Sony has it too. The A6700 also has an Auto ISO minimum shutter speed setting with per-focal-length control, which is more refined than Panasonic's simpler AUTO option. One advantage Sony has: the A6700 shoots lossless compressed RAW even with its Clear Image Zoom (Sony's equivalent of Crop Zoom), whereas Panasonic's Crop Zoom is limited to JPEG.

---

## Summary Table

| Mode | Scenario | Exposure | Key AF | Key Features |
|------|----------|----------|--------|--------------|
| C1 | Street / General | A, f/5.6 | Single-area, no detection | Simple, forgiving |
| C2 | Portrait / People | A, f/2.8 | Full + Human detection | Min SS 1/125s, Portrait style |
| C3-1 | Landscape | A, f/8 | Single-area, no detection | Scenery style |
| C3-2 | Macro Handheld | M, f/11, 1/200s, ISO 200 | Manual Focus | Flash ON/TTL, mechanical shutter |
| C3-3 | Wildlife / Action | M, f/6.3, 1/2000s, Auto ISO | Zone + Animal detection | Burst, electronic shutter |
| C3-4 | Tripod Macro (Stacking) | M, f/8, 1/200s, ISO 400 | Manual Focus | Focus bracketing, flash, 2s delay |
| C3-5 | Birds + Crop Zoom | M, f/6.3, 1/2000s, Auto ISO | Zone + Animal detection | JPEG only, 2x digital crop |
| C3-6 | Lightning (LVC) | M, f/8, 4s, ISO 400 | Manual Focus | Live View Composite, 16:9, Daylight WB |
| C3-7 | Indoor / Low Light | M, f/2.8, 1/125s, Auto ISO ≤12800 | Full + Human detection | Silent Mode, Natural style |
| C3-8 | Birds in Flight | M, f/6.3, 1/4000s, Auto ISO | Tracking + Animal detection | Burst, IS Mode 2 (panning) |
| C3-9 | Video — Travel | P (auto) | Full + Human detection | 4K 30p, e-stabilization |
| C3-10 | Video — Wildlife | M, f/6.3, 1/500s, Auto ISO | Tracking + Animal detection | 4K 30p, high e-stabilization |
