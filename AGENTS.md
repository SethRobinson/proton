# AGENTS.md

Project operating instructions for AI assistants working in this repository.

## Shared Project Memory

- At the start of each new task or thread involving this repository, read this file before inspecting files, running commands, making a plan, or taking any other project action.
- Also read `agents_local.md` if it exists. It is untracked and holds machine-specific environment notes (ssh build/test boxes, local SDK paths, WSL setup) for the computer you are running on. Anything tied to one person's machines goes there, never in this file, since this file ships in the public repo.
- Treat follow-up replies in the same continuous task as part of that task. Do not reread this file unless the repository or working directory changes, this file is modified, or its instructions are no longer available in context.
- Treat this file as the shared project memory for AI assistants.
- Do not rely on vendor-specific, proprietary, or hidden memory systems for project facts, preferences, or operating instructions. (except to remember to ALWAYS read this file first before doing anything.  Remember that.)
- Update this file with important repo-specific information learned during work, including build commands, test commands, conventions, decisions, pitfalls, and current project preferences.
- Keep this file accurate and current. Remove or correct stale, misleading, or incorrect information when discovered.
- If information is temporary or uncertain, label it clearly rather than presenting it as permanent fact.

Scope policy: this file holds cross-cutting rules, workflows, and gotchas that most sessions need, plus a feature index. Keep it around 30 KB. Feature deep-dives live in `docs/<topic>.md`: before working on a feature listed in the index, read its doc; when finishing feature work, update that doc and keep the index entry here to one or two lines (where it lives + the non-obvious constraint). Cross-cutting rules and new gotchas still land here directly. When a change makes anything stale, here or in a linked doc, update it in the same change.

## Testing

- When possible, design automated tests for new features and bug fixes.
- Run relevant automated tests after finishing changes to guard against regressions.
- If tests cannot be run or do not exist, state that clearly in the handoff and describe any manual verification performed.


## Security

- Never commit sensitive data, including credentials, tokens, passwords, private keys, cookies, customer data, personal data, or machine-specific authentication material.
- If an AI assistant needs authentication data or other secrets for local work, use `agents_secret.md` for those notes.
- `agents_secret.md` must stay ignored by git and must not be committed.
- Do not put secrets in commit messages, logs, issue text, pull request descriptions, generated docs, or other tracked files.
- Before committing, review staged changes for accidental secrets.

## Build/test machines

- Cross-platform testing (OSX via xcodebuild over ssh, Linux via a remote box or WSL) uses whatever machines are described in `agents_local.md` on the current computer. Building OSX demo apps looks like `xcodebuild -project RTLooneyLadders/OSX/RTLooneyLadders.xcodeproj -target RTLooneyLadders -configuration Debug build` from the repo root.
- To test on a remote machine without disturbing a real checkout there, tar the git-tracked files over and build in a scratch dir (e.g. `~/proton_warncheck`).

## Linux build notes

- RTPack's linux CMake only enables the Raspberry Pi GLES path when `/opt/vc/include/bcm_host.h` exists (fixed Aug 2026; it used to force it on and fail linking `-lbcm_host` on PCs).
- RTConsole's linux build needs no system dev packages (it uses Proton's internal zlib); RTPack needs zlib1g-dev; the GL/SDL app builds need libsdl2-dev and friends.

## Compiler warnings policy (cleanup pass done Aug 2026)

- The tracked projects build warning-free on MSVC /W3 (VS18), Apple clang (Xcode 26), GCC 13 default flags, and Emscripten 6. Please keep new code warning-clean.
- Vendored libs (ClanLib math, jpeglib, minizip) are quieted via targeted pragmas in `shared/ClanLib-2.0/Sources/Core/precomp.h`, `jmemmgr.c`, `jdhuff.c`, `jdphuff.c`, and `shared/util/unzip/unzip.c` rather than code edits.
- Intentionally left alone: RTPack Win32 Debug's LNK4075 (EditAndContinue vs /SAFESEH project setting), and Xcode project-level warnings (CFBundleIdentifier vs PRODUCT_BUNDLE_IDENTIFIER mismatch, ONLY_ACTIVE_ARCH, duplicate -rpath) since fixing those means touching pbxproj build settings.
- The legacy `if (this == 0)` null guards in BaseApp.cpp/HTTPComponent.cpp are kept but wrapped in clang pragmas; they are technically UB and a modern optimizer may delete them.

## HTML5 / Emscripten gotchas

- The html5 build scripts expect `EMSCRIPTEN_ROOT` to be set, normally by
  `base_setup.bat` (the local emsdk install path belongs in `agents_local.md`).
  A good HTML5 smoke build is `RTSocketCity\html5\build_release.bat nopause`,
  which compiles `shared/html5/` plus most of `shared/`. Note it defines both
  `RT_HTML5_USE_CUSTOM_MAIN` and `RT_EMTERPRETER_ENABLED`, so it exercises the
  emterpreter `while(1)` path in `HTML5Main.cpp`, not the
  `emscripten_set_main_loop` path.
- `RTSimpleApp\html5\build_release.bat nopause` and RTBareBones' equivalent also
  work now (Aug 2026: both needed `-sUSE_SDL=1` added for newer Emscriptens;
  without it the SDL includes in `HTML5Main.cpp` fail and the .bat still exits
  0, so check the output for errors). ArduboySim's html5 script likely needs the
  same flag treatment.
- If an AI assistant's shell has `NoDefaultCurrentDirectoryInExePath=1` (common in
  sandboxed tooling), `cmd` refuses to run batch files from the current directory and
  these build scripts fail with "'emsdk_env.bat' is not recognized". Clear it for the
  child process: `cmd /c "set NoDefaultCurrentDirectoryInExePath=&& <script>.bat"`.
  The scripts themselves are fine; this is purely a caller-environment issue.
- Never use `clock()` for timing in the HTML5/wasm build. `clock_t` is 32-bit there
  and `CLOCKS_PER_SEC` is 1,000,000, so it overflows after 2^31 microseconds
  (~35.8 minutes). Emscripten enables non-trapping float-to-int by default, so it
  saturates at `INT32_MAX` and never advances again rather than wrapping. That froze
  `GetSystemTimeTick()` permanently and hung the FPS limiter's wait loop (fixed in
  PR #50). Use `emscripten_get_now()` instead.
- When converting a large `double` to an integer on wasm, cast to `uint64` first if
  you want modulo wrap-around. A direct `double`->`unsigned int` cast saturates at
  `UINT32_MAX` (same trap as above) instead of wrapping.
- `GetSystemTimeTick()` has no common epoch across platforms (time since boot on
  Windows, Unix epoch ms on iOS/OSX, time since page load on HTML5), so never assume
  a starting value. It also rolls over every ~49 days on every platform. Engine-level
  timing goes through `GetSystemTimeAccurateRangeChecked()` in
  `shared/Manager/GameTimer.cpp`, which clamps each delta to 100 ms and uses wrap-safe
  unsigned subtraction. Prefer `GetTick()`/`GetDeltaTick()` over raw
  `GetSystemTimeTick()` for anything that must survive a roll-over or a backgrounded
  browser tab.

## Feature index

- Renderer migration (fixed-function -> shaders): `docs/renderer-migration.md`.
  Non-obvious constraint: engine rendering code must use the rt* functions from
  `shared/Renderer/RenderPipeline.h` for fixed-function GL, never the raw gl*
  names; ES2-portable GL (textures/blend/depth/scissor/etc) stays raw.
  The shader pipeline is the DEFAULT in any build that defines
  `RT_SHADER_PIPELINE_AVAILABLE` (all desktop GL configs of the suite apps,
  Mac RTBareBones, Linux via Proton.cmake); launch with `-fixedpipeline` to
  get the legacy fixed-function path for comparison.

## Renderer regression tests (tests/)

- Test-app policy (per Seth, Aug 2026): only use apps tracked in the proton
  repo (RTBareBones, RTSimpleApp, RTLooneyLadders, ArduboySim, RTConsole,
  RTPack) plus RTDink, RTDScroll, and RTMindWall for testing. Do not use the
  other app folders that may exist locally (RTGameShow, Amigo, RTWhisperApp,
  BlipArcade, RTSocketCity, RTLibretroTest...). BlipArcade was removed from
  the suite under this policy; it was the only app exercising app-level raw
  client-side vertex arrays and clip planes, so that coverage now rests on
  the engine paths only.

- `tests/harness.ps1` captures golden screenshots of the example apps and
  diffs them after renderer changes; read `tests/README.md` before touching
  renderer code. Run `.\harness.ps1 -Mode test` from `tests/` after any change
  that could affect rendering (that tests the shader pipeline, the default);
  add `-LegacyPipe` to regress the legacy fixed-function path against the
  same goldens.
- Besides the default Windows target, `-Target html5` runs the wasm build in
  headless Edge (pixel-exact, needs the app's html5 build_release.bat run
  first), `-Target ios` runs on the Mac's iOS simulator over ssh (use
  `-PrepareMac` after code changes; ~0.01% rasterization jitter), and
  `-Target android` runs on an adb device/emulator (wired but unverified,
  none was available). Parms reach the app via URL query (?parms=...) on
  html5, launch args on iOS, and the "parms" intent extra on Android.
- Android note: the gradle build's PrepareResources.bat overwrites the app's
  java copies from `shared/android/v3_src/`, so edit the v3_src templates,
  never the per-app copies. Gradle needs JAVA_HOME set to a JDK 17 (the local
  path is machine-specific, see agents_local.md).
- Any Proton app supports `-autoscreenshot <file.bmp> <delayMS>` (+`-autoquit`)
  to write its framebuffer to a BMP and exit; the code is
  `BaseApp::ProcessAutoScreenshot()`. The parm also enables deterministic mode
  (locked 16ms timestep via `GameTimer::SetLockedTimestepMS`, timeline zeroed,
  fixed rand seed), making captures pixel-exact across runs. On Windows the
  parm implies run-in-background so it works without focus. Paths must not
  contain spaces.
- Goldens are GPU/driver specific and NOT tracked in git (tests/goldens/ is
  ignored); record a fresh set with `-Mode golden` on the current renderer
  before starting renderer work on any machine.
- The suite also trips on big slowdowns: the engine writes a perf sidecar
  (wall fps + engine ms/frame) with each capture; the harness baselines it
  next to the goldens and fails at <50% fps or >2x engine-ms. See "Speed
  check" in tests/README.md. Delete goldens/*.perf.txt to re-baseline after
  intentional perf changes.

## Automated GUI testing on Windows

- The Win32 demo apps can be driven programmatically without touching the real
  mouse: the input path in `shared/win/app/main.cpp` reads click coordinates
  straight from lParam and only requires the window to have focus
  (`ShouldIgnoreMouseButtonMessage` checks `g_bHasFocus`, nothing else). So
  `SetForegroundWindow` + `PostMessage` of WM_LBUTTONDOWN/WM_LBUTTONUP (client
  coords packed in lParam), WM_MOUSEMOVE for drags, and WM_MOUSEWHEEL (screen
  coords) for ScrollComponent areas all work. Screenshot the client area with
  GetClientRect/ClientToScreen + Graphics.CopyFromScreen to verify visually.
  Run the exe with working dir = the project's `bin/` folder (media lives there).
  A crash shows up as the process going not-responding with a WER dialog; the
  Application event log (Id 1000) has the exception code.

## Versioning

- The engine version lives in `shared/ProtonVersion.h`: `PROTON_VERSION_MAJOR/MINOR/PATCH`,
  a numeric `PROTON_VERSION` for compile-time checks (v1.2.3 = 10203), and
  `PROTON_VERSION_STRING`. `BaseApp::Init()` logs it at startup.
- When cutting a notable engine milestone, bump the header and create a matching
  annotated git tag (e.g. `v1.0.0`) in the same commit/tag pair.
- `BaseApp::GetAppVersion()` is the *app's* version (filled in via OS messages on
  iOS/Android), unrelated to the engine version.

## Git

- `.gitignore` uses a whitelist: `/*` ignores everything at the repo root, and
  tracked projects/files are re-included with `!/Name/` lines at the top of the
  file. New project folders in the root are ignored by default; to start
  tracking one, add a `!/FolderName/` line (do not add per-file ignore lists).
- Never add OpenAI/Codex/Claude etc as a co-author on git commits.
- NEVER `git push` unless explicitly told to push. "Commit" means commit
  locally only; committing is not permission to push.

