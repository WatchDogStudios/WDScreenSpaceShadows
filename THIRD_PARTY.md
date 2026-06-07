# Third-party attributions — WDScreenSpaceShadows

## Bend Studio Screen Space Shadows

- **Component:** Bend Studio Screen Space Shadows (the wavefront screen-space contact-shadow technique used in *Days Gone* and other Sony titles).
- **Copyright:** Copyright 2023 Sony Interactive Entertainment (Bend Studio). Technique/implementation by Graham Aldridge.
- **License:** Apache License, Version 2.0. The full license text is in [`External/BendSSS/LICENSE`](External/BendSSS/LICENSE); the original per-file Apache-2.0 notices are retained verbatim at the top of each vendored header.
- **Upstream:** https://www.bendstudio.com/blog/inside-bend-screen-space-shadows/ — "EXAMPLE CODE" download (`code_final_candidate.zip`).
- **Retrieved:** 2026-06-02.

### Vendored files (verbatim, unmodified)
- `External/BendSSS/bend_sss_cpu.h` — CPU dispatch-list builder (`Bend::BuildDispatchList`).
- `External/BendSSS/bend_sss_gpu.h` — GPU/HLSL shader (`WriteScreenSpaceShadow`); kept as the reference, **not** compiled by Atom.

### Changes made (Apache-2.0 §4(b) "state changes")
- `bend_sss_cpu.h` is consumed **unmodified** by the feature processor through the thin wrapper
  `Code/Source/Render/WDSSSDispatch.{h,cpp}` (the only translation unit that includes it).
- `bend_sss_gpu.h`'s `WriteScreenSpaceShadow` is **ported** to AZSL in
  `Assets/Shaders/WDScreenSpaceShadows/WDSSSCompute.azsl`. The port:
  - moves the shader resources (`DepthTexture`, `OutputTexture`, `PointBorderSampler`) out of the
    `DispatchParameters` struct and into the gem's `ShaderResourceGroup` (AZSL requires resources in an SRG),
    rewriting the corresponding access sites;
  - populates the remaining (scalar) `DispatchParameters` fields from SRG constants;
  - adds an MSAA depth-fetch path (read sample 0) for multisampled pipelines such as the GMS project;
  - applies AZSL syntax rules (e.g. `select()` for vector-condition ternaries) as needed.
- No functional change to the algorithm itself.
