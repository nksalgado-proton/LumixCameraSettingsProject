# Mode Identification from EXIF Parameters

**Question:** If I have a photo file with access to all camera parameters except the mode code (C1, C2, etc.) and the mode name, what is the smallest set of parameters that would allow me to determine with 100% confidence which mode the camera was in when the picture was taken?

This analysis answers that question for the Lumix G9 MkI (5 custom modes) and G9 MkII (12 custom modes) configurations used in this project.

---

## Custom Mode Reference

### G9 MkI (DC-G9) — 5 modes

| Code | Name |
|------|------|
| C1 | Street / General |
| C2 | Portrait / People |
| C3-1 | Landscape |
| C3-2 | Macro Handheld (Single Shot) |
| C3-3 | Wildlife / Action |

### G9 MkII (DC-G9M2) — 12 modes

| Code | Name |
|------|------|
| C1 | Street / General |
| C2 | Portrait / People |
| C3-1 | Landscape |
| C3-2 | Macro Handheld (Single Shot) |
| C3-3 | Wildlife / Action |
| C3-4 | Tripod Macro (Focus Bracketing) |
| C3-5 | Birds with Crop Zoom |
| C3-6 | Landscape Tripod (Lightning) — Live View Composite |
| C3-7 | Indoor / Low Light |
| C3-8 | Birds in Flight (BIF) |
| C3-9 | Video — Travel / Documentary |
| C3-10 | Video — Wildlife in Motion |

**Total: 17 distinct modes across both cameras.**

---

## The 10 Parameters Needed

1. **Camera Model** — DC-G9 vs DC-G9M2 (first split: MkI's 5 modes vs MkII's 12)
2. **Exposure Mode** — A / M / Creative Video (plus Video sub-mode: P vs M)
3. **Focus Mode** — AFS / AFC / MF
4. **AF Area Mode** — Full Area / Zone / Tracking / 1-area
5. **AF Subject Detection** — Human / Animal / None
6. **Flash Fired** — Yes / No
7. **Focus Bracketing active** — Yes / No *(uniquely identifies C3-4)*
8. **Live View Composite active** — Yes / No *(uniquely identifies C3-6)*
9. **Silent Mode** — On / Off *(uniquely identifies C3-7)*
10. **Crop Zoom** — Active / Not *(uniquely identifies C3-5)*

---

## Why Each Parameter Is Needed

### Four "unique flag" parameters (#7-10)

These identify 4 MkII modes on their own — they're like bits where if one is set, the mode is determined unambiguously:

| Flag | Mode it identifies |
|------|-------------------|
| Focus Bracket ON | C3-4 Tripod Macro Stack |
| Live View Composite ON | C3-6 Lightning |
| Silent Mode ON | C3-7 Indoor / Low Light |
| Crop Zoom active | C3-5 Birds with Crop Zoom |

### Exposure Mode + Video sub-mode (#2)

Catches 2 more modes:

| Exposure combination | Mode |
|---------------------|------|
| Creative Video + Program (P) | C3-9 Video Travel |
| Creative Video + Manual (M) | C3-10 Video Wildlife |

### Combinations of parameters #3-6

The remaining 6 modes (C1, C2, C3-1, C3-2, C3-3 on both cameras; plus C3-8 on MkII) are distinguished by combinations of Focus Mode, AF Area, Subject Detection, and Flash:

| Mode | Signature |
|------|-----------|
| **C3-2** Macro | `M + MF + Flash fired` |
| **C3-3** Wildlife | `M + AFC + Animal detect + AF Area: Zone` |
| **C3-8** BIF | `M + AFC + Animal detect + AF Area: Tracking` *(only difference from C3-3 is Tracking vs Zone)* |
| **C1** Street | `A + AFS + Full Area + no specific detect` |
| **C2** Portrait | `A + AFS + Human detect` |
| **C3-1** Landscape | `A + AFS + no detect` *(ambiguous with C1 — see caveat below)* |

---

## The C1 / C3-1 Ambiguity

**C1 (Street) and C3-1 (Landscape) are the only ambiguous pair.** They have identical focus, AF area, subject detection, and flash configurations:

- Both use Aperture Priority (A)
- Both use AFS (Single AF)
- Both use Full Area AF
- Both have subject detection OFF
- Both have flash OFF

To break the tie, you'd need an **11th parameter**, either:

- **Photo Style** — C3-1 uses "Scenery", C1 uses "Standard"
- **Aperture** — C3-1 defaults to f/8, C1 defaults to f/5.6

### The problem with this tiebreaker

Both Photo Style and Aperture are **overrideable by the user at shoot time**. The camera loads the mode's default when you switch to it, but nothing prevents you from changing aperture with the rear dial or switching Photo Style via the Q.Menu. Neither is a hardware-locked indicator of the mode.

**Therefore, if we require strict 100% confidence under any user behavior, C1 and C3-1 are fundamentally indistinguishable.** They are essentially the same "landscape/street with deep focus" mode with different default apertures and photo styles.

---

## Summary

| Scenario | Parameters needed | Coverage |
|----------|-------------------|----------|
| **Strict 100% (no user overrides)** | 10 parameters | 17/17 modes (if we count C1 and C3-1 as a single "pair" that's resolved by Photo Style/Aperture defaults) |
| **Strict 100% (any user behavior)** | 10 parameters | 16/17 modes — C1 and C3-1 remain a tied pair |
| **Realistic minimum (defaults respected)** | 10 parameters + Photo Style as tiebreaker | 17/17 modes |

---

## Practical Implication for the Burst Culler Tool

This analysis explains why the `nks_focus_culler` tool uses a **camera-agnostic, scenario-based classification** instead of trying to recover the original mode code:

- **Mode codes are camera-specific.** "C3-3" means nothing on a Sony or Canon.
- **The underlying photography scenarios (wildlife, macro, landscape, portrait, etc.) are universal.**
- **EXIF parameters reliably encode the scenario**, even when they can't encode the mode name.

The classifier in `tools/nks_focus_culler/classifier.py` uses most of the same 10 parameters identified here to map any photo (from any camera) into one of 7 universal scenarios:

1. Macro
2. Stacks (focus brackets)
3. Wildlife — Action
4. Wildlife — Static
5. People
6. Night / Long Exposure
7. General Scene

This is why the tool doesn't need to know your mode dial configuration to work — it reads the same EXIF signals you'd use to identify the mode and maps them directly to photographic intent.

---

*Analysis based on the current mode configurations documented in:*
- `data/camera-config-g9mki.json`
- `data/camera-config-g9mkii.json`

*Generated: 2026-04-07*
