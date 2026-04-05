# Camera Configuration Project — Sony A6700

## For: Julia (or whoever is reading this)

This document describes a project to systematically configure your Sony A6700 camera so that you can walk into any shooting situation, turn a dial, and start shooting with confidence — without fumbling through menus.

This is the same process your uncle Nelson went through with his Lumix G9 cameras. The camera is different, the menus are different, but the photography principles are identical.

---

## What This Project Will Produce

By the end, you will have:

1. **A JSON configuration file** — the single source of truth for every setting on your camera. What's saved in each memory slot, what the global "set and forget" settings are, and why each choice was made.

2. **Printable field reference cards** — iPhone-sized laminated cards you carry in the field. The front tells you what to physically set up; the back explains why.

3. **A PWA (Progressive Web App)** — the same cards accessible on your iPhone, working offline. No app store needed.

4. **Deep understanding** — you won't just have settings. You'll understand WHY each parameter has that value, WHEN to change it in the field, and WHAT it affects in the photo.

---

## The Core Concept: Memory Recall Modes

Your Sony A6700 can memorize complete camera configurations and recall them instantly:

- **3 dial positions** (MR1, MR2, MR3) — turn the mode dial to recall. These are stored in the camera permanently.
- **4 card slots** (M1–M4) — recalled via menu. These are stored on the SD card (lost if you format the card).

**Total: 7 custom configurations.** You need to choose your 7 most important scenarios. The 3 most-used ones go on the dial for instant access.

### What Gets Saved in a Memory Slot

Everything: exposure mode (A/S/M), aperture, shutter speed, ISO settings, autofocus mode, autofocus area, subject detection settings, drive mode (single/burst), white balance, image stabilization mode, metering, photo style — the entire camera state.

### What Does NOT Get Saved (Physical Actions)

Some things require your hands:
- **Which lens** is mounted
- **The focus mode switch** on the lens (AF/MF)
- **The OSS switch** on the lens (stabilization ON/OFF)
- **Flash** mounted or not, on or off
- **Tripod** or handheld

The field cards will remind you of these physical steps.

---

## The Scenarios

Below are the shooting scenarios, described in photography terms (not camera-menu language). You and Claude will translate these into Sony A6700 menu settings together.

### Tier 1 — Mode Dial (MR1, MR2, MR3)

These are your most-used scenarios. One click on the dial and you're ready.

#### MR1: Street / General (Walk-Around)
- **What:** Markets, towns, beaches, travel, sightseeing. Unpredictable subjects.
- **Approach:** Aperture Priority. You control depth of field, camera handles the rest.
- **Starting aperture:** f/5.6 (sharp, versatile, good DoF)
- **ISO:** Auto (let the camera choose)
- **Autofocus:** Single-shot AF, single area (you point, it focuses, no surprises)
- **Drive:** Single shot
- **Stabilization:** ON
- **Why Aperture Priority?** In a walk-around context, shutter speed rarely matters — you're not freezing fast action. Aperture controls depth of field, which IS a creative decision. Let the camera handle the rest.

#### MR2: Wildlife / Action
- **What:** Birds, mammals, reptiles, anything that moves. Your primary interest.
- **Approach:** Manual exposure with Auto ISO. You lock aperture (for DoF control) and shutter speed (to freeze motion). ISO floats to compensate.
- **Starting aperture:** Wide open (f/4.5–6.3 depending on zoom — your 70-350mm varies). Open = more light + background blur.
- **Starting shutter speed:** 1/1000s. Enough for most perched or walking animals. In the field, dial UP to 1/2000–1/4000 for birds in flight.
- **ISO:** Auto, no upper limit. Modern sensors handle high ISO well. A noisy sharp photo beats a clean blurry one.
- **Autofocus:** Continuous AF (AF-C), Zone or Wide area, Animal/Bird detection ON.
- **Drive:** High-speed burst (you want many frames to catch the right moment)
- **Stabilization:** ON, Mode 2 if available (stabilizes vertically only — better for panning)
- **Why Manual + Auto ISO?** You need to guarantee BOTH enough DoF (aperture) AND frozen motion (shutter speed). Those are non-negotiable. ISO is the flexible variable — let the camera figure it out.

#### MR3: Landscape
- **What:** Vistas, mountains, waterfalls, forests, architecture. Static scenes where sharpness and depth of field are everything.
- **Approach:** Aperture Priority. You set a narrow aperture for deep DoF.
- **Starting aperture:** f/8–f/11 (sharpest range of most lenses, deep DoF)
- **ISO:** 100 (base ISO, cleanest image). Can be Auto if handheld.
- **Autofocus:** Single-shot AF, single area, focus on the scene's key element
- **Drive:** Single shot
- **Stabilization:** OFF if on tripod, ON if handheld
- **Why f/8–f/11?** This is the "sweet spot" for most lenses — maximum optical sharpness. Narrower (f/16+) causes diffraction blur. Wider loses depth of field.

### Tier 2 — Card Slots (M1–M4)

Recalled from the menu. Slightly slower to access, but still instant compared to setting up from scratch.

#### M1: Portrait / People
- **What:** People, posed or candid. Emphasis on subject separation from background.
- **Approach:** Aperture Priority. Wide aperture for background blur (bokeh).
- **Starting aperture:** Widest your lens allows. With the 70-350mm, shoot at 70mm f/4.5 for portraits.
- **ISO:** Auto
- **Autofocus:** Single-shot AF, Eye AF / Face detection ON
- **Drive:** Single shot
- **Note:** A dedicated portrait lens (e.g., Sony 50mm f/1.8 or Sigma 56mm f/1.4) would transform your portrait photography. The 70-350mm works but doesn't blur the background as much.

#### M2: Macro / Close-Up
- **What:** Insects, flowers, small details. Requires a macro lens (you don't have one yet — see lens recommendations below).
- **Approach:** Manual exposure. At macro distances, depth of field is razor-thin. You need full control.
- **Starting aperture:** f/11–f/20 (maximize DoF at close range)
- **Shutter speed:** 1/200s (if using flash) or 1/250s+ (if natural light, handheld)
- **ISO:** 200 fixed (if using flash) or Auto (natural light)
- **Autofocus:** Manual Focus. At macro magnification, AF hunts too much. Use the focus ring or rock your body forward/backward.
- **Flash:** External flash strongly recommended for macro. Provides all the light, lets you use low ISO and narrow aperture.
- **Note:** This slot is a placeholder until you get a macro lens. Skip it for now or use it for another scenario you prefer.

#### M3: Birds in Flight (BIF)
- **What:** Dedicated to the hardest wildlife challenge — birds flying across the frame.
- **Approach:** Manual + Auto ISO, but faster shutter than general wildlife.
- **Starting shutter speed:** 1/2000–1/4000s (freeze wing beats)
- **Aperture:** Wide open (maximum light)
- **ISO:** Auto, no limit
- **Autofocus:** Continuous AF (AF-C), Wide area or Tracking, Bird detection ON
- **Drive:** Maximum burst speed
- **Why a separate mode from MR2?** BIF needs significantly faster shutter speed and different AF area settings. Having a dedicated slot means you can switch instantly when a bird takes off.

#### M4: Night / Low Light
- **What:** Cityscapes at night, stars, low-light handheld.
- **Approach:** Depends on scenario:
  - Handheld night: Aperture Priority, wide open, Auto ISO (high), stabilization ON
  - Tripod night: Manual, f/8, low ISO, long exposure (2–30s)
  - Stars: Manual, widest aperture, highest usable ISO, 15–20s exposure, MF on infinity
- **Starting config (handheld night):** Wide open, Auto ISO, single-shot AF
- **Note:** You'll adjust this in the field depending on the specific situation.

---

## Global "Set and Forget" Settings

These are settings you configure ONCE and never touch again. They apply across all modes. Claude will help you find each one in the Sony menus.

### Image Quality
- **File format:** RAW + JPEG (RAW for editing flexibility, JPEG for quick sharing)
- **JPEG quality:** Fine or Extra Fine
- **Aspect ratio:** 3:2 (native sensor ratio — don't crop in-camera)

### Display
- **Grid lines:** Rule of thirds (helps composition)
- **Histogram:** ON (shows exposure distribution — learn to read it)
- **Level gauge:** ON (keeps horizons straight)

### Autofocus
- **Face/Eye priority:** ON (globally — individual modes can override)
- **AF illuminator:** ON (helps focus in dark environments)

### Playback
- **Auto review:** 2 seconds (see the shot, then back to shooting)

### Custom buttons
- Assign frequently-changed functions to physical buttons so you never need the menu in the field. Common assignments:
  - **C1 button:** ISO
  - **C2 button:** White Balance
  - **C3 button:** Focus Mode (AF-S/AF-C/MF)
  - **AEL button:** Eye AF (half-press to track eyes)

---

## Lens Recommendations

You currently have:

### Sony E 70-350mm f/4.5-6.3 G OSS
- **Great for:** Wildlife, birds, distant subjects
- **Limitations:** Cannot focus close (macro), not wide enough for landscapes/street, not fast enough for strong background blur in portraits

### Suggested additions (in priority order):

1. **Sony E 18-135mm f/3.5-5.6 OSS** (~$550) — The walk-around lens. Covers street, landscape, travel, casual portraits. With this + the 70-350mm you cover 18-350mm (27-525mm equivalent). This is the most impactful single addition.

2. **Sigma 56mm f/1.4 DC DN** (~$400) — Portrait lens. Beautiful background blur, great for people and pets in low light. Dramatic improvement over the zoom for portraits.

3. **Sony E 30mm f/3.5 Macro** (~$280) — Entry macro lens. True 1:1 magnification. Opens up the insect/flower world. Budget-friendly. (Or the Sigma 70mm f/2.8 Macro for better working distance with insects.)

4. **Sony E 11mm f/1.8** (~$300) — Ultra-wide. Dramatic landscapes, architecture, astrophotography, night sky.

**For starting out: Lens #1 (18-135mm) is the priority.** Two lenses (18-135 + 70-350) will cover 90% of situations.

---

## The Process — How to Work With Claude

### Step 1: Get the Camera Manual

1. Search Google for: **"Sony A6700 help guide PDF"** or go directly to Sony's support page
2. Download the **full PDF manual** for the ILCE-6700 (make sure it matches your current firmware version)
3. Check your firmware: **Menu → Setup → Version** on the camera
4. Save the PDF in your project folder under `reference/manuals/`
5. Ask Claude to read it — this gives Claude the exact menu structure, setting names, and available options for YOUR camera

### Step 2: Set Up the Project

1. Create a GitHub repository (Claude can help)
2. Create the folder structure:
   ```
   your-project/
   ├── data/                  # Single source of truth
   │   └── camera-config-a6700.json
   ├── docs/                  # PWA (GitHub Pages)
   ├── tools/                 # Card generators, utilities
   ├── guides/                # Written guides, walkthroughs
   ├── reference/             # Manuals (gitignored — too large)
   │   └── manuals/
   ├── output/                # Generated PDFs
   └── review/                # Sample photos for analysis (gitignored)
   ```

### Step 3: Global Settings ("Set and Forget")

Go through the camera menu systematically with Claude:
1. Claude reads the manual and identifies every menu item
2. For each setting, discuss: What does it do? Should we set it once and forget? Or does it change per scenario?
3. Configure the "set and forget" ones on the camera
4. Document everything in the JSON file

**This is the longest step.** It takes time but you only do it once. Every decision gets documented with its rationale.

### Step 4: Define Your Scenarios

1. Look at the scenarios listed above
2. Discuss with Claude: Do they match your shooting interests? Want to add, remove, or modify any?
3. For each scenario, Claude will recommend specific Sony A6700 settings based on the photography goals
4. YOU make the final decision on every value — Claude explains the tradeoffs, you choose

### Step 5: Program the Camera

1. For each Memory Recall slot (MR1-3, M1-4):
   - Set the camera to all the correct values manually
   - Save to the memory slot: **Menu → Shooting → Camera Set. Memory**
2. Test each slot by recalling it and verifying the settings loaded correctly

### Step 6: Create Field Reference Cards

1. Claude builds a `field-cards.json` with all the scenario data
2. A Python script generates printable PDF cards (iPhone-sized, front/back)
3. Print, laminate, carry in the field

### Step 7: Build the iPhone App (PWA)

1. Claude creates an `index.html` that reads from the JSON
2. Enable GitHub Pages on your repo
3. Open in Safari → Add to Home Screen → done

### Step 8: Field Test and Iterate

**This is the most important step.** Go shoot. Come back. Tell Claude:
- "The shutter speed for wildlife was too slow, birds were blurry"
- "I couldn't focus fast enough on the monkey"
- "The landscape photos were overexposed"

Claude updates the JSON, regenerates the cards, the app updates. Iterate until every mode feels right.

---

## Key Principles

1. **Understand before configuring.** Don't just copy settings. For every value, ask "why?" until you understand.

2. **Settings are a starting point, not a religion.** The saved mode gets you 80% there. The last 20% is adjusting in the field with the dials.

3. **Physical setup is as important as software.** A perfectly configured mode is useless if the lens stabilization switch is in the wrong position.

4. **Test in real conditions.** Indoor testing tells you almost nothing. Go outside, find subjects, shoot, review, adjust.

5. **Start simple, add complexity.** Begin with MR1 (Street) and MR2 (Wildlife). Get comfortable. Then add the others one at a time.

6. **The JSON is your single source of truth.** If you change a setting on the camera, update the JSON. If you forget what a mode is configured to, read the JSON. Everything else (cards, app) is generated FROM the JSON.

---

## What This Project Does NOT Cover

- **Post-processing** (Lightroom, Capture One, etc.) — that's a separate skill
- **Composition and artistic vision** — no settings can teach you where to point the camera
- **Advanced flash photography** — can be added later as a module
- **Video** — can be added as additional memory slots later
- **Lightning/storm alerts** — can be added later if interested

---

## Getting Started

Open Claude Code (or Claude in your IDE), paste this document, and say:

> "I want to start this project for my Sony A6700. Here's the project description my uncle wrote. Let's begin with Step 1 — I need to find and download the camera manual."

Claude will guide you from there. Have fun and good shooting!

---

*This project template was created by Nelson Salgado based on his experience configuring two Lumix G9 cameras. The methodology is camera-agnostic — the photography principles are universal.*
