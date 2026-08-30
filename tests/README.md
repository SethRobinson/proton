# Renderer regression tests

Golden-screenshot tests guarding the renderer, created for the fixed-function
to shader-pipeline migration. The idea: capture reference screenshots of the
example apps on the current renderer, then re-run after every renderer change
and diff. A refactor that claims "no visual change" must produce (nearly)
identical pixels.

## How it works

`harness.ps1` launches each app in `scenarios.psd1` with Proton's
`-autoscreenshot <file.bmp> <delayMS>` command line parm (plus `-autoquit`).
The support lives in `BaseApp::ProcessAutoScreenshot()` (shared/BaseApp.cpp)
and is compiled into every Proton app, completely inert unless the parm is
passed: once the app's own timer passes `delayMS` it writes its framebuffer
(`glReadPixels` via `SoftSurface::BlitFromScreenFixed`) to the BMP and quits.

`-autoscreenshot` also switches the engine to a deterministic mode: the game
timer runs on a locked 16ms timestep with its timeline zeroed at Init (see
`GameTimer::SetLockedTimestepMS`), and the random seed is fixed. Time becomes
a pure function of the frame count, so animations, particles, and even the FPS
counter are identical every run: **all six apps reproduce with zero differing
pixels**, no masks needed. No desktop pixels are captured, and the app window
does not need to be in the foreground (`-autoscreenshot` implies
run-in-background on Windows; see the parm handling in
shared/win/app/main.cpp).

The harness converts each BMP to a 24bpp PNG, paints any `IgnoreRects` magenta
(none are currently needed; the mechanism remains for future
wall-clock-dependent content), and either records it as a golden or compares
against the golden. A compare fails when more than `MaxDiffPct` percent of
pixels (default 0.5) differ by more than `ChannelTol` per channel; failures
write a `*_DIFF.png` (golden dimmed, differing pixels red) into `output/`.

## Usage (Windows)

```powershell
cd tests
.\harness.ps1 -Mode test              # run the suite against the goldens
.\harness.ps1 -Mode test -App RTDink  # one app
.\harness.ps1 -Mode golden            # re-record all goldens
```

Exit code 0 = all pass. Both `output/` (scratch) and `goldens/` are
git-ignored: goldens are GPU/driver specific, so each machine records its own
with `-Mode golden` before starting renderer work.

## Things to know

- **Goldens are GPU/driver specific.** Only compare against goldens recorded
  on the same machine. After a driver update or on a new machine, re-record
  (`-Mode golden`) and eyeball the images before trusting them.
- RTDink, BlipArcade, and RTMindWall are separate repos/folders that happen to
  live inside the proton checkout; the harness skips apps whose exe is
  missing. Their goldens are still tracked here since the renderer under test
  is proton's.
- Build the `Debug GL` configs first (x64 for everything except RTBareBones;
  several Win32 configs are stale and fail with 0xc000007b loading the 64-bit
  DLLs sitting in `bin/`).
- RTDink's scenario expects a previous save to exist (it captures the
  "Continue your last session?" prompt).
- Paths passed to `-autoscreenshot` must not contain spaces (the Windows parm
  tokenizer splits on them).
- The deterministic mode belongs to the engine, not the harness: launching any
  app with `-autoscreenshot` gives locked-timestep fixed-seed behavior on
  every platform, so the same approach can drive Mac/Linux/HTML5 harnesses
  later.
