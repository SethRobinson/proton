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

Because the capture is scheduled on the app clock, tick-driven animations land
in the same pose every run: four of the six apps reproduce with **zero**
differing pixels. No desktop pixels are captured, and the app window does not
need to be in the foreground (`-autoscreenshot` implies run-in-background on
Windows; see the parm handling in shared/win/app/main.cpp).

The harness converts each BMP to a 24bpp PNG, paints the scenario's
`IgnoreRects` magenta (FPS counters and the like), and either records it as a
golden or compares against the golden. A compare fails when more than
`MaxDiffPct` percent of pixels differ by more than `ChannelTol` per channel;
failures write a `*_DIFF.png` (golden dimmed, differing pixels red) into
`output/`.

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
- The two apps with tolerance above ~zero (RTBareBones 3%, RTMindWall 4%) have
  continuously-animating content that can land one frame apart between runs;
  the tolerance absorbs the resulting edge drift while still catching any real
  rendering change (wrong colors, missing textures, broken batching all show
  up as double-digit percentages).
- Paths passed to `-autoscreenshot` must not contain spaces (the Windows parm
  tokenizer splits on them).
