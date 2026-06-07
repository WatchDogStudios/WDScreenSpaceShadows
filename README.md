# WDScreenSpaceShadows

Bend Studio's wavefront **screen-space contact shadows** for O3DE / Atom. A compute pass ray-marches the
scene depth buffer toward the sun to produce a screen-space shadow mask, which is optionally combined into
the directional-light visibility so the sun is darkened in contact areas that cascaded shadow maps miss —
with **no lighting-shader edits**.

Ports Bend Studio's Apache-2.0 reference implementation (Graham Aldridge / © 2023 Sony Interactive
Entertainment). See [`THIRD_PARTY.md`](THIRD_PARTY.md) and [`External/BendSSS/LICENSE`](External/BendSSS/LICENSE).


## How it works

The feature processor injects a parent pass **before `Forward`** with three children:

1. **WDSSSComputePass** — runs Bend's wavefront march (one compute dispatch per entry in the CPU dispatch
   list) over the hardware depth (`DepthMSAA`, read as `Texture2DMS` sample 0), writing an `R8_UNORM` mask
   published as the `WDSSSMask` pipeline-global.
2. **WDSSSDebugPass** — fullscreen grayscale visualization of the mask (only when the debug view is on).
3. **WDSSSApplyPass** — `min`s the mask into Atom's `FullscreenShadowBuffer` in place, so the Forward
   lighting (which already reads `m_fullscreenShadow`) darkens the sun. Only runs when *apply* is on.

## Controls

Console variables (all default off):

| CVar | Meaning |
|------|---------|
| `r.wdSSSEnabled 1` | Produce the mask (and enable the debug view / apply). |
| `r.wdSSSDebugView 1` | Show the mask as a grayscale overlay (1 = lit, 0 = shadowed). |
| `r.wdSSSApply 1` | Darken the sun with the mask (min into `FullscreenShadowBuffer`). |

ImGui panel: **"WD Screen Space Shadows"** → *SSS Panel* — toggles, the **sun direction**, and the Bend
tunables (surface thickness, bilinear threshold, shadow contrast, edge flags).

**Entity Inspector component:** add **WD Screen Space Shadows** (Add Component → Level/Game →
Graphics/Lighting) to a Level entity (or any entity) for per-level, authored + persisted settings — the
same enabled/apply toggles, sun direction, and tuning, shown as sliders/checkboxes with tooltips.

## Current limitations / follow-ups

- **Sun direction is config-driven.** `DirectionalLightFeatureProcessorInterface` exposes no CPU-side
  direction getter, and Bend's dispatch builder needs the light's screen position on the CPU, so set the
  sun direction (ImGui / component) to match the scene's sun. *Auto-follow* (reading the sun entity /
  light buffer) is a follow-up.
- **MSAA hardware depth.** WDSSS reads `DepthMSAA` as `Texture2DMS` (sample 0), matching Atom's
  FullscreenShadow and keeping Bend's non-linear depth space. A non-MSAA pipeline needs a `Texture2D`
  supervariant (the `o_msaa` path), not wired yet (TODO).
- **Apply mechanism.** The apply pass does an in-place R8 UAV `min` on `FullscreenShadowBuffer` (created as
  a render target by the shadow chain). If the frame graph rejects UAV usage on it, the fallbacks are:
  (A) re-publish `FullscreenShadowBuffer` from the apply output, or (B) a shader-option-guarded multiply in
  `DirectionalLightShadow.azsli`. This is the one item to watch on first run in the live Editor.
- **Shadow length** (`SAMPLE_COUNT`/`HARD_SHADOW_SAMPLES`/`FADE_OUT_SAMPLES`) is a compile-time shader
  `#define` (it sizes a `groupshared` array); runtime tuning would be a supervariant.
- **Early-out** (Bend's `UseEarlyOut`, a sky/occluded-pixel cull) is removed in WDSSS: it uses wave
  intrinsics that need a Vulkan 1.1 SPIR-V target env Atom doesn't set. Re-enabling it (a perf win)
  would require `-fspv-target-env=vulkan1.1` on the Vulkan build.

## License

WDScreenSpaceShadows is licensed under the MIT License. Feel free to fork or use privately if needed, just make sure to properly attribute.
