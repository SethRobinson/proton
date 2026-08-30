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

## Usage

```powershell
cd tests
.\harness.ps1 -Mode test                    # Windows apps against win goldens
.\harness.ps1 -Mode test -App RTDink        # one app
.\harness.ps1 -Mode golden                  # re-record win goldens
.\harness.ps1 -Mode test -LegacyPipe        # win: force the legacy fixed-function path
.\harness.ps1 -Mode test -Target html5      # wasm/WebGL in headless Edge
.\harness.ps1 -Mode test -Target ios        # iOS simulator on the Mac (ssh)
.\harness.ps1 -Mode test -Target android    # device/emulator via adb
```

The shader pipeline is the engine default wherever it's compiled in
(`RT_SHADER_PIPELINE_AVAILABLE`), so a plain run tests it. `-LegacyPipe`
launches the apps with `-fixedpipeline` to regress the legacy path, which
must stay pixel-identical to the same goldens until it's retired.
(`-ShaderPipe` is a deprecated no-op from when the shader path was opt-in.)

Exit code 0 = all pass. Both `output/` (scratch) and `goldens/` are
git-ignored: goldens are GPU/driver specific, so each machine records its own
per target with `-Mode golden` before starting renderer work. Golden files are
prefixed by target (`html5_`, `ios_`, `android_`; win has no prefix).

Per-target notes:

- **html5**: needs the app's `html5/build_release.bat` run first (scenario key
  `Html5Page` points at the built page). The harness serves the page dir over
  a local HTTP server, launches headless Edge with the parms in the URL
  (`?parms=...`, see `AddCommandLineParmsFromURL` in HTML5Main.cpp), and the
  app POSTs its BMP back (`autoscreenshot_upload`). `-ShowBrowser` runs headed
  for debugging.
- **ios**: runs over ssh on the Mac (`-MacHost`, default seth@studiomac.local)
  against the `~/proton_warncheck` tree. Pass `-PrepareMac` to re-sync the
  tracked tree and xcodebuild the simulator apps (scenario key `IosProject`).
  Launch args flow through main.mm into command line parms; the BMP lands in
  /tmp on the Mac and is scp'd back. The simulator model is pinned via
  `-IosSimDevice` (default iPad Pro 12.9) and is booted if needed; goldens
  are per-model and captured in the fresh-boot portrait orientation. Expect
  a few hundred pixels of rasterization jitter between runs (~0.01%),
  comfortably inside the 0.5% limit.
- **ios real device**: add `-IosDevice` (goldens prefixed `iosdev_`). Uses
  devicectl against the first paired device (`-IosDeviceId` to pick one);
  the app writes to its sandbox Documents under a unique name and devicectl
  copies it out. Requirements: the device must be PAIRED, ON USB or the same
  network, and UNLOCKED (set Auto-Lock to Never on a test device: a locked
  screen fails with kAMDMobileImageMounterDeviceLocked). Device builds need
  signing: run `-PrepareMac -IosDevice -MacKeychainPassword <pw>` (the
  password lives in agents_secret.md, never in tracked files).
- **android**: the app must be installed (debuggable) on the first adb
  device/emulator; scenario key `AndroidPackage`. Parms are passed as the
  "parms" intent extra (see SharedActivity.onCreate +
  nativeAddCommandLineParm), the BMP is written to the app's internal files
  dir and pulled with `adb exec-out run-as`. NOTE: wired but not yet verified
  end to end (no device/emulator was available when this was written).

## Things to know

- **Goldens are GPU/driver specific.** Only compare against goldens recorded
  on the same machine. After a driver update or on a new machine, re-record
  (`-Mode golden`) and eyeball the images before trusting them.
- Per Seth, only apps tracked in the proton repo plus RTDink, RTDScroll, and
  RTMindWall may be used for testing. RTDink and RTMindWall are separate
  repos/folders that happen to live inside the proton checkout; the harness
  skips apps whose exe is missing.
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

## Speed check

Because the locked timestep fixes the number of frames until capture, the
wall clock those frames take is a benchmark for free. The engine writes a
`<shot>.bmp.perf.txt` sidecar with three numbers: frames, wall fps, and
**engine ms/frame** (average Update+Draw cost, measured from Update start to
Draw end so the swap/vsync wait is excluded). The harness records a baseline
next to the goldens on first sighting (or in `-Mode golden`) and fails a
later run when:

- wall fps drops below `MinFpsRatio` (default 0.5) of the baseline, or
- engine ms/frame exceeds 2x baseline AND baseline+0.5ms AND 1.0ms absolute
  (the guards keep sub-millisecond cache noise from flaking).

Scenario keys beyond the basics: `ShaderReady` (app's own raw GL is shimmed,
so it works on the shader pipeline; apps without it are skipped except under
`-LegacyPipe`), `RequiresShaderPipe` (scenario has no fixed-function
equivalent and is skipped under `-LegacyPipe`, e.g. the RTBareBonesRTT
render-to-texture demo), and `ExtraParms` (extra command line parms, passed
via URL on html5).

This is a tripwire for "everything got horribly slower" bugs, not a profiler.
Notes: on Windows, `-autoscreenshot` disables vsync and bypasses the app's
SetFPSLimit so throughput is real, but DWM still caps windowed wall fps near
the monitor refresh (hence engine-ms as the vsync-immune signal). On
html5/ios the browser/display cap (~60-120) applies to fps. Baselines are
per-machine like the goldens; delete the `.perf.txt` files in goldens/ to
re-baseline after intentional perf changes.
