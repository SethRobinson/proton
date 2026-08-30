# RTShader

The smallest possible demonstration of Proton's shader pipeline features:

1. **Render-to-texture**: an animated scene is drawn into an offscreen
   texture each frame (`Surface::InitRenderTarget` / `BeginRenderTarget` /
   `EndRenderTarget`). Everything works exactly like normal screen drawing.
2. **Custom GLSL shaders**: that texture is blitted to the screen through a
   tiny fragment shader (the `RTShader` class + `SetActiveShader`), giving a
   full-screen post-processing effect in a few lines of shader code.

Click, tap, or press space to cycle the effects: no shader, wavy, grayscale,
color cycle, and scanlines+vignette. The whole tour is in
[source/App.cpp](source/App.cpp), written to be read top to bottom; the
engine's shader contract (which attributes and uniforms are bound for you) is
documented in the comment block at the top and in
`shared/Renderer/ShaderPipeline.h`.

Because a shader stays active for *every* engine draw until you call
`SetActiveShader(NULL)`, the same technique works on individual sprites, not
just full screens.

Shaders are written in GLSL ES 1.00 style and run everywhere Proton does.
Platform projects included (all cloned from RTBareBones):

| Platform | Where | Notes |
|----------|-------|-------|
| Windows | `windows_vs/RTShader.sln` | shader pipeline is the default; `-fixedpipeline` shows the fallback message |
| Mac | `OSX/RTShader.xcodeproj` | desktop GL 2.1 |
| iOS | `RTShader.xcodeproj` | pure GLES2 build (`RT_SHADER_PIPELINE_ONLY`) |
| Android | `AndroidGradle/` | GLES2 context via `SharedActivity.useGLES2` |
| Web | `html5/build_release.bat` | WebGL1, no `LEGACY_GL_EMULATION` |
| Linux | `linux/CMakeLists.txt` | GL2 via SDL |

Launch parms: `-effect <0..4>` picks the starting effect;
`-autoscreenshot shot.bmp 4000 -autoquit` captures a deterministic frame
(used by the regression harness in `tests/`, which keeps this demo
pixel-identical across engine changes on Windows, WebGL, and iOS).
