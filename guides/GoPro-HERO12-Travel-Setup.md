# GoPro HERO12 Black — US Parks 2026

**Status:** configured on 2026-08-20  
**Firmware:** 2.40  
**Control:** GoPro Quik on the owner's phone; the passenger operates the phone while the car is moving  
**Output:** local 4K material for PTE slideshows and household/family TV viewing; cloud upload remains off

## Quik interface model

- Use the bottom-center `GoPro` tab to enter camera control.
- `Enable Preview` is optional and is not required to edit a mode.
- Above the central mode area, select the capture family: `Time Lapse`, `Video` or `Photo`.
- The central shortcuts belong only to the selected family. Use the pencil beside a mode to edit it.
- `TimeWarp` is one mode inside `Time Lapse`; the other time-lapse effects are not part of the trip workflow.
- `Video` has one configured mode. The three useful Photo modes are `Photo`, `Burst` and `Night Photo`.

## Daily startup

1. On the phone, keep automatic date, time and time zone enabled.
2. Turn on the HERO12 and connect it to Quik.
3. In Quik camera settings, tap `Set Date and Time` at the start of every shooting day and after every time-zone change.
4. Confirm the intended capture family and mode before mounting the camera.
5. Verify the lens cover is clean, the door is fully latched, a charged battery is installed and the 256 GB V30 card is present.

## Saved modes

### TimeWarp — road highlights

- 4K, 16:9, 2x.
- Linear + Horizon Leveling (`L+`).
- Record 20–30 seconds to produce approximately 10–15 seconds of 2x material.
- Use the premium suction mount on a clean, dry, firm part of the hood, with short arms and a slight upward aim that excludes the hood from the frame.
- The passenger starts and stops recording from the owner's phone. Never ask the driver to operate Quik.

### Video — real-time road and trail

- 4K, 16:9, 30 fps.
- Profile Standard.
- Linear + Horizon Lock (`L+`).
- HyperSmooth AutoBoost.
- Duration No Limit.
- On a trail, use the PGYTECH backpack-strap clip and articulating adapter; keep the camera upright, level and clear of straps or clothing.

### Photo — family

- Lens Linear.
- Output SuperPhoto.
- Timer Off.
- Prefer normal horizontal framing for PTE/TV; use the front screen only when a selfie composition requires it.

### Burst — action

- Lens Wide.
- Rate Auto.
- Output Standard.
- Timer Off.

### Night Photo — supported camera

- Lens Wide.
- Shutter Auto.
- Output Standard.
- Timer 3 seconds.
- Use only on a firm support or tripod.

## Snorkel temporary conversion

The current Quik design exposes one Video mode, so snorkel is a temporary conversion rather than a separately saved video preset.

Before entering the water:

1. Open `Video` with the pencil.
2. Keep 4K, 16:9, Profile Standard and HyperSmooth AutoBoost.
3. Change 30 fps to 60 fps.
4. Change `L+` to `Wide`.
5. Fit the floating grip, inspect the door seal and latch, and start/stop with the physical shutter.

After leaving salt water:

1. Rinse the closed camera and floating grip with fresh water, then dry them before opening the door.
2. Restore Video to 30 fps and `Linear + Horizon Lock (L+)`.

## Horizon correction in Quik after capture

Use this only when a clip still needs correction; the saved `L+` modes should normally deliver a level result.

1. Download the clip locally from the camera; cloud upload is not required.
2. Open the local clip in Quik and choose Edit.
3. Open the Lens/Horizon tool and apply horizon leveling or the rotation control until the horizon is correct.
4. Export a new corrected copy. Keep the original until the corrected file has been transferred to the computer and accepted in PTE.

## Recovery

HERO12/Quik does not provide a Panasonic-style `.DAT` export. The authoritative recovery record is `data/camera-config-gopro-hero12.json`, supplemented by this guide and the GoPro cards in `data/field-cards.json`.
