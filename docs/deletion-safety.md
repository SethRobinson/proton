# Deletion safety audit (October 2026)

The audit covers git-tracked, Proton-maintained scripts and source, including
sample apps, legacy platform backends and the renderer test harness. It does
not rewrite vendored libraries' build systems (Irrlicht's JPEG/PNG builds,
ClanLib, bzip2, wiringPi, etc.) or all scripts in ignored local app repositories.
Those still contain cleanup commands needing separate review before use.
The follow-up consumer review includes RTDink, RTDScroll and RTMindWall;
RTDink's compatibility fixes live in its separate repository (see below).

## Script cleanup

- Windows batch cleanup uses `shared/win/utils/SafeRemove.ps1`: a script-derived
  absolute root plus a named relative child. The helper rejects blank/relative
  roots, drive roots, traversal, parent wildcards and junctions/reparse points.
  Wildcards are restricted to filenames with `-FilesOnly`; recursive wildcard
  cleanup preflights the tree. Every batch call checks `errorlevel`.
- Android assets, packed textures, packaging intermediates, PDBs, console build
  output and HTML5 output use named children. HTML5 output names and remote web
  project names are deliberately explicit in each app's script; update them
  when copying the app template. Backup environment variables are checked
  before use, and the cleanup helper also requires an absolute nonblank base.
- `shared/linux/safe_paths.sh` checks absolute physical roots and child paths
  before recursive removal or destructive rsync. It rejects linked parent
  paths and traversal. `clean_web_loader.sh` is sent over SSH and only accepts
  the four named web projects; it checks HOME before deleting a literal
  `www/web/<project>/WebLoaderData` child. No deployment is part of the tests.
- Android Gradle `clean` is confined to the literal project `build` child and
  refuses a redirected or overridden build directory.
- The harness's remote scratch cleanup checks HOME and its named scratch child;
  local screenshot deletion uses `-LiteralPath`.

The shared Linux helpers that formerly operated on the caller's working
directory now require an explicit **absolute** project-directory argument:

```sh
bash RTPack/linux/update_media.sh /absolute/MyApp/media
bash RTPack/linux/androidSyncAssets.sh /absolute/MyApp/android
bash RTPack/linux/prepareAndroid.sh /absolute/MyApp/android -i
bash RTPack/linux/buildAndRun.sh /absolute/MyApp/linux
```

The sample apps' `media/update_media.sh` wrappers supply this argument themselves.
The relevant shell scripts are kept as LF by `.gitattributes`, including the
remote script sent to `sh` from Windows.

## Source cleanup

`shared/util/SafeDelete.h` implements the common checked recursive operation.
All recursive platform implementations use it. The Vita implementation only
removes empty directories and does not traverse a tree.

`RemoveDirectoryRecursively` requires an absolute non-root path with no dot
components, wildcards or linked ancestors. It returns failure on invalid paths
or failed deletions. It does not infer an app directory from the current working
directory. Callers remain responsible for checking their base path before
appending a known child name. Existing relative-path callers must be updated;
there are no such app call sites in the tracked tree.

On POSIX, recursive deletion unlinks symlink entries rather than following them.
On Windows it refuses reparse points. The Windows implementation processes the
first directory entry and avoids the old fixed-size path buffers. On POSIX it
uses `lstat`, including on filesystems where `dirent.d_type` is unknown. Once
the supplied root is checked, enumerated child names are treated literally;
legal POSIX subdirectories such as `art: old`, `draft.` and `literal*question?`
must not fail halfway through cleanup because of Windows path restrictions.
Only Windows strips trailing backslashes; on POSIX a backslash is not a path
separator, and stripping it could silently select a different directory.

The legacy Windows `delete_wildcard` requires an absolute directory and a leaf
pattern; it no longer changes the process cwd. Apple's `RemoveFile` now uses
file-only `unlink`, consistent with the other platforms; recursive deletion must
go through the checked directory API.

These are protections against accidental path mistakes, not a sandbox against
concurrent filesystem mutation. Symbolic links in directory paths (including
OS-provided aliases such as macOS `/tmp`) are rejected; use a physical absolute
base. UNC/device paths are deliberately unsupported by the Windows checks.

## Regression tests

From a Visual Studio developer command prompt:

```bat
if not exist tests\output mkdir tests\output
cl /nologo /EHsc /W3 /WX tests\deletion_safety.cpp /Fe:tests\output\deletion_safety.exe /Fo:tests\output\deletion_safety.obj
python tests/deletion_safety.py --native tests/output/deletion_safety.exe
```

On Linux (also works in WSL):

```sh
mkdir -p tests/output
g++ -std=c++11 -Wall -Wextra -Werror tests/deletion_safety.cpp -o tests/output/deletion_safety
python3 tests/deletion_safety.py --native tests/output/deletion_safety
```

Fixtures remain under ignored `tests/output/deletion-safety/` for inspection.
The suite exercises blank/relative roots, traversal, wildcard scope, spaces,
links, outside sentinels, actual batch entry points from an unrelated cwd,
remote cleanup in a fake home, and recursive native deletion. It never runs a
production build/upload/cleanup script against its actual target directories.

The suite also exercises real Android `PrepareResources.bat` entry points,
media/Android rsync cleanup of stale nested directories, Windows read-only and
locked files, and legal POSIX child names. Packing is stubbed in the media sync
fixture; deletion and rsync are real. With a suitable `JAVA_HOME`, add
`--gradle-wrapper RTBareBones/AndroidGradle/gradlew.bat` to test all three actual
Gradle clean tasks against isolated build trees and redirected build directories.
The fixture loads only the clean task, not Android plugins or signing settings.

The native runner accepts repeated `--native` arguments. With the RTDink checkout
present, its production uninstall helper can be exercised too:

```bat
cl /nologo /EHsc /W3 /WX /I shared RTDink\tests\dmod_cleanup.cpp /Fe:tests\output\dmod_cleanup.exe /Fo:tests\output\dmod_cleanup.obj
python tests/deletion_safety.py --native tests/output/deletion_safety.exe --native tests/output/dmod_cleanup.exe
```

Use `g++ -std=c++11 -Wall -Wextra -Werror -I shared` for the Linux equivalent.
Both native fixtures can also run in Node's Emscripten filesystem:

```sh
em++ -std=c++11 -Wall -Wextra -Werror tests/deletion_safety.cpp --pre-js tests/deletion_safety_pre.js -sENVIRONMENT=node -o tests/output/deletion_safety_wasm.js
node tests/output/deletion_safety_wasm.js /deletion-safety/native
em++ -std=c++11 -Wall -Wextra -Werror -I shared RTDink/tests/dmod_cleanup.cpp --pre-js tests/deletion_safety_pre.js -sENVIRONMENT=node -o tests/output/dmod_cleanup_wasm.js
node tests/output/dmod_cleanup_wasm.js /deletion-safety/native
```

## Consumer review (October 7, 2026)

The initial engine-only audit missed RTDink's two recursive deletion callers.
Its Windows/Linux DMOD root is normally relative (`dmods/`); passing that to the
new engine API silently prevented uninstall and autotest cleanup. In RTDink's
own repo, `source/DMODCleanup.h` now anchors relative roots to the explicit app
base, validates that the requested target is one immediate DMOD child, and on
POSIX resolves the storage root so OS aliases such as iOS `/var` and Android
`/sdcard` work. The selected DMOD itself cannot be a link. The menu reports
failure, and autotest checks the deletion result rather than only the disappearance
of `dmod.diz` (which could hide a partially deleted tree). Startup cleanup of
`temp.dmod` also avoids prepending the save path to an already rooted cache path.

RTDink's standalone test covers full removal of nested assets, saves and empty
directories, relative and absolute roots, spaces, a different cwd, POSIX storage
aliases, linked-DMOD rejection, missing targets and outside sentinels. **Commit
the RTDink companion changes in that repo too; Proton's gitignore excludes it.**

Windows `-game name` intentionally returns an empty relative DMOD root. That
app-specific case is also tested: cleanup binds it to the checked absolute app
base and still requires one nonempty child. An empty app base never authorizes
relative deletion, and the engine API continues to reject empty/relative paths.

No recursive deletion or `delete_wildcard` callers were found in the other
reviewed sample apps, RTDScroll or RTMindWall. Their file deletion calls and
RTPack's `rt_temp_*.jpg` intermediates are file-only; so is UnpackArchiveComponent's
download cleanup. No caller in those apps relied on Apple's old recursive
`RemoveFile` behavior. Shared Linux script callers in these apps pass the new
explicit project argument.

Completed validation:

- Windows: RTDink, RTDScroll, RTMindWall, RTBareBones, RTShader,
  RTSimpleApp, RTLooneyLadders and RTConsole Release x64; RTPack Release Win32.
  The available RTPack x64 configs require old XP toolsets, so use its maintained
  Win32 configuration. ArduboySim's Windows project is still VS2005 format.
- Linux: RTDink, RTShader, RTConsole and RTPack, built from a scratch snapshot.
- HTML5: RTDink, RTDScroll, RTBareBones, RTShader, RTSimpleApp, RTConsole
  and ArduboySim. RTDink/RTDScroll used their existing compiler flags and source
  lists with outputs redirected to the review directory, leaving their old
  cleanup scripts and generated site pages untouched.
- Android: RTDink native Debug arm64-v8a library compiled and linked; no APK or
  device run. Its CMake source list was missing `AutoTester.cpp` and is fixed.
- macOS: RTDink, RTBareBones and RTShader Release builds, plus both native
  cleanup suites with Apple clang warnings treated as errors.
- iOS: RTBareBones Debug simulator build, including the changed iOSUtils.mm.
- Cleanup tests: Windows, Linux/WSL, macOS and WebAssembly native fixtures; PowerShell,
  shell, batch, media/Android sync, and all three Gradle clean tasks passed.

The Windows build sweep also exposed existing HostResolver link failures in
RTBareBones, RTShader and RTSimpleApp; their projects now compile NetSocket.cpp.
Full builds still emit some unrelated existing Emscripten/Android warnings and
Xcode project-setting warnings; standalone cleanup tests compile with warnings
treated as errors. Legacy platform SDKs were not tested. No live
upload, desktop control, gameplay smoke run, or deletion of real DMODs/saves ran.
Local build logs are under ignored `tests/output/compat-review/`.
