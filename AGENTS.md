# AGENTS.md

Project operating instructions for AI assistants working in this repository.

## Shared Project Memory

- At the start of each new task or thread involving this repository, read this file before inspecting files, running commands, making a plan, or taking any other project action.
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

## Mac build/test machine

- A Mac for building and testing is reachable at `ssh seth@studiomac.local` (key auth already set up from Seth's PC). Proton checkout lives at `~/projects/proton` there.
- Build OSX demo apps with e.g. `xcodebuild -project RTLooneyLadders/OSX/RTLooneyLadders.xcodeproj -target RTLooneyLadders -configuration Debug build` (run from the repo root). SDL2/SDL2_mixer frameworks are installed in `~/Library/Frameworks` on that machine.
- GUI apps launched over ssh do run (a console session is active), so smoke tests via running the built binary and reading stdout work.
- To test without touching the real checkout, tar the tracked files over and build in `~/proton_warncheck` instead.

## Linux build/test machine

- An Ubuntu box is reachable at `ssh glados@glados.local` (key auth set up; the `glados` host alias in ssh config also works). No proton checkout lives there; tar tracked files to `~/proton_warncheck` to test.
- No passwordless sudo, and libsdl2-dev/zlib1g-dev are NOT installed, so only console targets build there (RTConsole works, it uses Proton's internal zlib). For zlib-needing console builds (RTPack), the local WSL `Ubuntu-24.04` distro has zlib1g-dev and works: `cmake -S RTPack/linux -B ~/rtpack_build && make -C ~/rtpack_build`. Full GL/SDL app builds need libsdl2-dev installed somewhere first.
- RTPack's linux CMake only enables the Raspberry Pi GLES path when `/opt/vc/include/bcm_host.h` exists (fixed Aug 2026; it used to force it on and fail linking `-lbcm_host` on PCs).

## Compiler warnings policy (cleanup pass done Aug 2026)

- The tracked projects build warning-free on MSVC /W3 (VS18), Apple clang (Xcode 26), GCC 13 default flags, and Emscripten 6. Please keep new code warning-clean.
- Vendored libs (ClanLib math, jpeglib, minizip) are quieted via targeted pragmas in `shared/ClanLib-2.0/Sources/Core/precomp.h`, `jmemmgr.c`, `jdhuff.c`, `jdphuff.c`, and `shared/util/unzip/unzip.c` rather than code edits.
- Intentionally left alone: RTPack Win32 Debug's LNK4075 (EditAndContinue vs /SAFESEH project setting), and Xcode project-level warnings (CFBundleIdentifier vs PRODUCT_BUNDLE_IDENTIFIER mismatch, ONLY_ACTIVE_ARCH, duplicate -rpath) since fixing those means touching pbxproj build settings.
- The legacy `if (this == 0)` null guards in BaseApp.cpp/HTTPComponent.cpp are kept but wrapped in clang pragmas; they are technically UB and a modern optimizer may delete them.

## HTML5 / Emscripten gotchas

- Emscripten lives at `d:\pro\emsdk` (6.0.3), set as `EMSCRIPTEN_ROOT` by
  `base_setup.bat`. A good HTML5 smoke build is
  `RTSocketCity\html5\build_release.bat nopause`, which compiles `shared/html5/`
  plus most of `shared/`. Note it defines both `RT_HTML5_USE_CUSTOM_MAIN` and
  `RT_EMTERPRETER_ENABLED`, so it exercises the emterpreter `while(1)` path in
  `HTML5Main.cpp`, not the `emscripten_set_main_loop` path.
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

## Git

- `.gitignore` uses a whitelist: `/*` ignores everything at the repo root, and
  tracked projects/files are re-included with `!/Name/` lines at the top of the
  file. New project folders in the root are ignored by default; to start
  tracking one, add a `!/FolderName/` line (do not add per-file ignore lists).
- Never add OpenAI/Codex/Claude etc as a co-author on git commits.
- NEVER `git push` unless explicitly told to push. "Commit" means commit
  locally only; committing is not permission to push.

