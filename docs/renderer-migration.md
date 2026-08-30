# Renderer migration: fixed-function GL1/GLES1 to a shader pipeline

Status and design notes for modernizing Proton's renderer. Read this before
touching `shared/Renderer/`, `shared/util/GLESUtils.cpp`, or anything the
regression suite (tests/) guards.

## Goal

Move the engine from OpenGL 1.x / GLES 1.1 fixed-function to a GLES2-class
shader pipeline (custom shaders + render-to-texture for new apps), while
existing apps keep building and running unchanged. Target API: GLES 2.0
semantics with one GLSL ES 1.00 shader dialect everywhere: ES2 contexts on
Android/iOS, WebGL1 on HTML5 (dropping emscripten's LEGACY_GL_EMULATION),
and the existing compatibility/legacy contexts on Windows/Linux/macOS (GLSL
120 compiles the same shaders; core profile is deliberately avoided).

## Phase status

- **Phase 0 (done, Aug 2026)**: regression suite in tests/ (screenshot goldens
  on win/html5/ios targets, android wired), `-autoscreenshot` deterministic
  capture mode in the engine, GL compat macros consolidated into
  `Renderer/GLCompatDesktop.h` + `Renderer/GLCompatGLES.h`, engine versioning
  (v1.0.0 tag = fixed-function baseline).
- **Phase 1 (done, Aug 2026)**: every fixed-function GL call the engine makes
  now goes through `shared/Renderer/RenderPipeline.h` (see below). Verified
  bit-identical: 0.000% pixel diff on all six suite apps, all platforms build.
- **Phase 2 (in progress)**: `shared/Renderer/ShaderPipeline.cpp/.h`, the
  shader implementation of RenderPipeline's surface: CPU matrix stacks
  feeding uProj/uMV uniforms, an ubershader cache keyed on
  texture/vertex-color/clip bits, client arrays fed straight to
  glVertexAttribPointer (a streaming VBO comes with the WebGL flip), and a
  proper eye-space clip plane. Runtime-selectable: projects that compile the
  file and define RT_SHADER_PIPELINE_AVAILABLE (currently RTBareBones and
  RTSimpleApp Debug GL configs) accept a `-shaderpipeline` launch parm;
  without it the legacy path runs untouched. Milestone 1 (Aug 2026): both
  apps render pixel-identical (0.000%) to the fixed-function goldens on
  Windows via `harness.ps1 -Mode test -ShaderPipe`. Milestone 2 (Aug 2026):
  the app compatibility shim, `Renderer/GL1ShaderShim.h`, included from
  PlatformSetup.h in shader-enabled projects: remaps the fixed-function gl*
  names apps call onto the rt* dispatch layer, so app source runs unchanged
  on either pipeline. RTDink, BlipArcade, and RTLooneyLadders all render at
  0.000% vs their fixed-function goldens under -shaderpipeline. Gotchas
  baked into the code: ShaderPipeline.cpp must see real GL, and MSVC PCHs
  bake the shim macros in, so it includes GL1ShaderShimUndef.h right after
  the precomp (RT_RENDERER_INTERNAL alone is not enough under /Yu);
  RenderBatcher has a *method* named glDrawArrays, so its internal calls to
  the free function are ::-qualified; GL_LINE_SMOOTH is rasterizer state
  that still works with shaders on desktop, and DrawLine's look depends on
  it, so SP_Enable passes it through under C_GL_MODE. RTMindWall stays on
  legacy until the single-light ubershader variant lands (its glLightfv/
  glShadeModel are shimmed as legacy-passthrough/shader-ignored). Still to
  do: that lighting variant, ES2 context creation per platform, the WebGL
  flip (streaming VBO + dropping LEGACY_GL_EMULATION), FBO render targets,
  and the public shader API. Notable finding recorded in ShaderPipeline.cpp:
  GL_ALPHA_TEST was always a no-op in Proton (glAlphaFunc never called, GL
  default is GL_ALWAYS), so the shader path needs no alpha discard.
- **Phase 3**: flip defaults per platform (HTML5 first).
- **Phase 4**: public shader + render-to-texture APIs.
- **Phase 5**: delete the legacy path (tag first).

## RenderPipeline.h (the Phase 1 funnel)

`shared/Renderer/RenderPipeline.h` is the single seam between the engine and
fixed-function GL. Engine code calls `rt*` functions (rtPushMatrix,
rtColor4x, rtVertexPointer, rtDrawArrays, rtEnable...) instead of the raw GL
names. Today they are inline passthroughs (zero overhead, zero behavior
change); the Phase 2 backend reimplements exactly this surface.

Rules:

- New engine rendering code must use the rt* functions for anything
  fixed-function: matrix stack, current color, client arrays/draws, and the
  fixed-function-only enums (GL_TEXTURE_2D, GL_ALPHA_TEST, GL_LIGHTING,
  GL_LINE_SMOOTH, GL_CLIP_PLANE0, GL_COLOR_MATERIAL) via rtEnable/rtDisable.
- GL that survives in ES2 stays raw on purpose: texture objects, blend,
  depth, cull, scissor, viewport, clear, glReadPixels. Don't wrap those.
- App-level code (RTDink etc.) keeps calling raw GL; the Phase 2 shim remaps
  those names onto the same layer, so apps need no source changes.
- Not converted (deliberately): the vendored `shared/Irrlicht` tree (no app
  compiles it), `shared/Renderer/linearparticle`'s point-sprite path (GLES1
  builds only, handled in Phase 2), `IrrlichtManager.cpp` (only compiles
  under `_IRR_STATIC_LIB_`, which nothing defines).

Known wart preserved on purpose: `rtClipPlane` keeps the historical
float*-cast-to-GLdouble* call, which means user clip planes have likely only
ever worked correctly on real GLES1 devices; fix when the shader path
replaces it, not before (the suite guards zero behavior change until then).

## Verification

Any change here runs `tests/harness.ps1 -Mode test` (win) and ideally
`-Target html5` and `-Target ios`; see tests/README.md. The win/html5 targets
reproduce with zero differing pixels, so ANY pixel change is a finding.

## Key inventory facts (from the Aug 2026 research pass)

- Engine: ~386 raw GL callsites before Phase 1, 273 of them in four files
  (GLESUtils.cpp, RenderUtils.cpp, Surface.cpp, RenderBatcher.cpp). No
  immediate mode, no display lists, no glTexEnv (relies on default
  GL_MODULATE: replacement fragment shader is `texture * color`), no
  VBO/FBO/shaders anywhere in live engine code.
- Apps: 13 of 17 in-repo apps touch raw GL only for glClear. The
  compatibility surface apps actually need: matrix stack (+ glGetFloatv
  readback of GL_MODELVIEW_MATRIX: RTDink's input hit-testing depends on it),
  glColor4x, client arrays + glDrawArrays, scissor, one clip plane
  (BlipArcade), one light (RTMindWall), clear/viewport/ortho.
- Render-to-texture is a green field (Surface::IsRenderTarget() is hardwired
  false; transitions use glReadPixels via Surface::CopyFromScreen).
- Texture formats are ES2-clean (RGBA/RGB + 8888/4444/565, POT-padded).
- In-tree prior art: `shared/flash/app/cpp/GLFlashAdaptor.cpp` (a 1661-line
  GLES1-on-shader-only emulation, the API contract) and Irrlicht's
  `COGLES2FixedPipelineShader.cpp` (fixed-function-in-GLSL reference).
