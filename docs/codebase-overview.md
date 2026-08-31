# Proton SDK codebase overview

A map of the engine for AI assistants and new contributors: what each part
does and which files to read. It is a map, not a tutorial. If a path looks
wrong, verify with a quick glob before trusting it, and when a change moves
or renames anything mentioned here, update this file in the same change.

Last updated: Aug 2026.

## The big picture

- Proton is a source-only C++ SDK: there is no engine .lib. Each app's
  project files compile the specific `shared/*.cpp` files it uses, so an
  app only pays for the features it pulls in.
- Anatomy of an app: `source/App.cpp` + `App.h` (the app class, derived
  from `BaseApp`), per-platform build folders (`windows_vs`, `linux`,
  `html5`, `OSX`, `<Name>.xcodeproj`, `AndroidGradle`), `bin/` (the
  runtime working dir, media lives under it), and `media/` (source assets
  that get packed into `bin/`).
- The frame loop: the platform entry point (see Platform backends below)
  pumps OS events and calls `BaseApp::Update()` then `BaseApp::Draw()`,
  which walk the root Entity tree calling each entity's components.
- How things communicate: entities hold named `Variant` properties;
  components bind to property changes and to named functions via
  boost::signals2; input and OS events arrive as `Message`s through
  `MessageManager`. GUI widgets are entities with components attached,
  built by helper functions in `EntityUtils`.

## Core architecture spine

- **`BaseApp`** (`shared/BaseApp.h/.cpp`): the central app object. Owns the
  root Entity, `ResourceManager`, `GameTimer`, `Console`, `RenderBatcher`,
  touch tracking (up to 12 touches), FPS/video mode, and the
  `-autoscreenshot` deterministic-capture hook. `BaseApp.h` is the umbrella
  header that includes most of the engine.
- **Entity/Component system** (`shared/Entity/Entity.h`, `Component.h`):
  an `Entity` is a node in a tree with a name, a `VariantDB` of properties,
  and a list of `EntityComponent`s that give it behavior (the class is
  `EntityComponent`, not `Component`, due to an old OSX name clash).
  `shared/Entity/EntityUtils.h/.cpp` is the big helper library everything
  uses: `CreateTextButtonEntity`, `CreateOverlayEntity`,
  `CreateTextLabelEntity`, fades, zooms, bobs, kill timers, and so on.
- **`Variant` / `VariantDB`** (`shared/util/Variant.h`,
  `shared/Manager/VariantDB.h`): the typed value used for all entity
  properties and function parameters (`VariantList`), plus `FunctionObject`
  signals; `GetVarWithDefault`, `GetFunction("OnSomething")->sig_function`
  is the standard wiring pattern.
- **`MessageManager`** (`shared/Manager/MessageManager.h/.cpp`): delivers
  timed/scheduled messages. `SendGUI`/`SendGame` for input and game events,
  `CallEntityFunction`/`CallComponentFunction`/`CallStaticFunction` for
  delayed calls. The `MESSAGE_TYPE_*` enum order is mirrored in Java in
  `shared/android/v3_src/SharedActivity.java`; keep them in sync.
- **`ResourceManager`** (`shared/Manager/ResourceManager.h/.cpp`): caches
  textures/fonts/surfaces; `GetResourceManager()->GetSurfaceAnim("x.rttex")`.
  `Surface` (`shared/Renderer/Surface.h`) is a GPU texture (also render
  targets via `InitRenderTarget`); `SoftSurface`
  (`shared/Renderer/SoftSurface.h`) is a CPU-side bitmap with format
  conversion and screen grabs.
- **`GameTimer`** (`shared/Manager/GameTimer.h/.cpp`): use
  `GetTick()`/`GetDeltaTick()`, never raw `GetSystemTimeTick()` (no common
  epoch across platforms and it rolls over ~49 days; see the timing gotchas
  in AGENTS.md). Also home of `SetLockedTimestepMS` (deterministic mode)
  and the game-vs-system timer split that lets game time pause.

## Subsystem map (`shared/`)

| Dir | What it is | Key files |
|---|---|---|
| `Entity/` | The entity/component system AND all GUI widgets/behaviors (~50 components: buttons, scrolling, text boxes, touch drag, HTTP, tweening) | `Entity.h`, `Component.h`, `EntityUtils.h`, `TemplateComponent.h` (copy-me starter) |
| `Manager/` | Global managers, accessed via `Get<Name>()` helpers | `MessageManager`, `ResourceManager`, `VariantDB`, `GameTimer`, `Console`; optional: `AdManager`, `IAPManager`, `HueManager`, `MySQLManager`, `OpenCVManager`, `libVLC_RTSP` |
| `Renderer/` | 2D/3D drawing, shader + fixed-function GL paths | `RenderPipeline.h` (the rt* dispatch layer, see AGENTS.md renderer rule), `ShaderPipeline.h/.cpp`, `GL1ShaderShim.h`, `Surface.h`, `SoftSurface.h`, `RenderBatcher.h`, `SurfaceAnim.h`, `SpriteAnimation.h` |
| `Audio/` | Abstract `AudioManager` base + one backend per file | `AudioManager.h`; backends: `AudioManagerSDL`, `AudioManagerOS.mm`, `AudioManagerAndroid`, `AudioManagerFMODStudio`, `AudioManagerAudiere` |
| `Network/` | HTTP and sockets | `NetHTTP.h/.cpp` (+ `NetHTTP_libCURL.cpp`, `html5/NetHTTP_HTML5.cpp`), `NetSocket.h`, `NetUtils.h`, `NetAdapter/`, vendored `enet/` |
| `AI/` | LLM chat-completion client for OpenAI-compatible servers, and a text-to-speech client for form-POST TTS servers (see `docs/ai-llm.md`) | `LLMClient.h/.cpp` (LLMConversation + LLMClient; needs `Network/` + `util/cJSON.c` compiled in), `TTSClient.h/.cpp` (text -> audio file, latest-wins queue; needs `Network/`) |
| `GUI/` | Fonts ONLY (the widget GUI is in `Entity/`) | `RTFont.h/.cpp` (.rtfont bitmap fonts, kerning, color codes), `FreeTypeManager.h` |
| `FileSystem/` | Virtual filesystem: mount disk paths or zips, streaming readers | `FileManager.h` (the router; `GetFileManager()`), `FileSystemZip.h`, `StreamingInstance*.h` |
| `Math/` | Only `rtRect`, `rtPlane`, `WeightRand`; vectors/matrices are ClanLib (`CL_Vec2f`, `CL_Mat4f`) from `ClanLib-2.0/` | `rtRect.h` |
| `Gamepad/` | Gamepad abstraction, provider/device pair per backend | `GamepadManager.h`; providers: XInput, DirectX, SDL2, iOS/GCController, Moga, iCade |
| `util/` | Grab bag: `Variant.h`, `MiscUtils.h`, `MathUtils.h`, `RenderUtils.h` (draw helpers), `GLESUtils.h`, `ResourceUtils.h`, `TextScanner.h`, `CRandom.h`, `Profile.h`, `RTFileFormat.h` (.rttex header) | plus vendored `boost/`, `cJSON`, `rapidxml/`, `zlib/`, `bzip2/`, `unzip/`, `archive/` (tar), `QR-Code-generator/` |
| `testfw/` | In-app test framework | `ProtonTester.h`, `ProtonTesterGUI.h` |
| `Ad/` | Ad provider abstraction | `AdProvider.h`, ChartBoost/Flurry impls |
| `Arduboy/` | Arduino/Arduboy API emulation layer used by ArduboySim | `Arduboy.h` |
| `addons/` | Optional heavyweight integrations | `whisper.cpp`, `WhisperModded/`, `TinyEXIF-master/`, `pplot/` |
| `ClanLib-2.0/` | Vendored math library (vectors, matrices, rects) | `Sources/Core/Math/` |
| `Bullet/` | Vendored Bullet physics (optional) | used with `Irrlicht/irrBullet` |
| `Irrlicht/` | Vendored Irrlicht 3D engine (optional) | `IrrlichtManager.h/.cpp` |
| `wiringPi/` | Raspberry Pi GPIO (optional) | used via `Manager/WiringPiManager` |

## Platform backends

`shared/PlatformSetup.h` dispatches on `WIN32` / `__APPLE__` /
`PLATFORM_ANDROID` / `PLATFORM_HTML5` / `RTLINUX` etc. to the right
`shared/<platform>/PlatformSetup*.h`. Each backend owns the entry point,
main loop, and input conversion into engine `Message`s:

| Platform | Entry / main loop | Notes |
|---|---|---|
| Windows | `shared/win/app/main.cpp` | Win32 message pump, `g_bHasFocus` gating, mouse capture, `-autoscreenshot` parm; vendored libs under `shared/win/` |
| HTML5/wasm | `shared/html5/HTML5Main.cpp` | Emscripten; two loop styles (emterpreter vs `emscripten_set_main_loop`); `templates/` has the shell html; many gotchas in AGENTS.md |
| Android | `shared/android/AndroidApp.cpp` | JNI bridge; ALWAYS edit the `v3_src/` java templates (`SharedActivity.java`), never per-app copies (gradle overwrites them) |
| iOS | `shared/iOS/app/` | `EAGLView.mm`, `MyAppDelegate.mm`, `MyViewController.mm` |
| OSX | `shared/OSX/app/` | `MainController.mm`, `MyOpenGLView.mm` |
| Linux | `shared/linux/LinuxMain.cpp` | `Proton.cmake` is the shared CMake include apps use |
| SDL2 | `shared/SDL/SDL2Main.cpp` | used by Linux builds and the Windows `Debug_GL_SDL` config |
| Legacy | `flash/`, `bbx/`, `WebOS/`, `psp2/` | mostly dormant; psp2 (Vita) has its own `Proton.cmake` |

## Sample apps (tracked in this repo)

| App | Demonstrates |
|---|---|
| `RTBareBones/` | Minimal starting template; just `source/App.cpp`. Also hosts the `-rttdemo` render-to-texture test scenario |
| `RTSimpleApp/` | The full "real app" template: menus, scroll bars, components under `source/Component/` and `source/GUI/` |
| `RTShader/` | Custom GLSL shaders + render-to-texture (see its README.md). Also the project-layout pattern to copy for new apps (`windows_vs` folder naming) |
| `RTLooneyLadders/` | A small game; unified touch/keyboard/gamepad input via `ArcadeInputComponent` + `GamepadManager` |
| `ArduboySim/` | Arduboy simulator built on `shared/Arduboy/` |
| `RTConsole/` | Text-only console app, no GL; Windows/Linux/HTML5 |
| `RTPack/` | Command-line asset tool: builds `.rttex` textures, `.rtfont` fonts, `.rtpack` compressed files (`source/TexturePacker.cpp`, `FontPacker.cpp`) |

Untracked app folders (RTDink, RTDScroll, etc.) may exist locally; see the
test-app policy in AGENTS.md for which ones may be used for testing.

## Where do I look to...

- **Make a button / label / image**: `shared/Entity/EntityUtils.h`
  (`CreateTextButtonEntity`, `CreateOverlayButtonEntity`,
  `CreateTextLabelEntity`, `CreateOverlayEntity`); button behavior is
  `Button2DComponent`. Menus in the samples: `RTSimpleApp/source/GUI/`.
- **Make a scrolling list**: `shared/Entity/ScrollComponent.h` +
  `ScrollBarRenderComponent.h` + `RenderScissorComponent.h`; example in
  `RTSimpleApp/source/GUI/AboutMenu.cpp`.
- **Render text**: `TextRenderComponent` (label) /
  `InputTextRenderComponent` (text entry) / `TextBoxRenderComponent`
  (wrapped box); font engine is `shared/GUI/RTFont.h`.
- **Play a sound**: `GetAudioManager()->Play("audio/foo.wav")`
  (`shared/Audio/AudioManager.h`).
- **Load / draw an image**: `GetResourceManager()->GetSurfaceAnim(...)` +
  `OverlayRenderComponent`, or `Surface::Blit` directly; low-level draw
  helpers in `shared/util/RenderUtils.h`.
- **Render to texture**: `Surface::InitRenderTarget` /
  `BeginRenderTarget`; examples: `RTShader/source/App.cpp` and
  RTBareBones' `-rttdemo`.
- **Animate / tween an entity**: `InterpolateComponent`
  (`shared/Entity/InterpolateComponent.h`) usually via `EntityUtils`
  helpers (`FadeOutEntity`, `ZoomToPositionEntity`, `BobEntity`).
- **Schedule a delayed call**: `GetMessageManager()->CallEntityFunction(
  pEnt, timeMS, "OnThing", &vlist)` or `CallStaticFunction`.
- **Do an HTTP request**: `HTTPComponent` (`shared/Entity/HTTPComponent.h`)
  or `NetHTTP` (`shared/Network/NetHTTP.h`) directly.
- **Use sockets**: `shared/Network/NetSocket.h`; game networking via
  vendored enet.
- **Read files / zips**: `GetFileManager()`
  (`shared/FileSystem/FileManager.h`); mount zips with `FileSystemZip`.
- **Parse JSON / XML**: `shared/util/cJSON.h` / `shared/util/rapidxml/`.
- **Handle touch/keyboard/gamepad input**: sign up via
  `TouchHandlerComponent` or entity input signals; unified game input is
  `ArcadeInputComponent` (`shared/Entity/ArcadeInputComponent.h`) fed by
  keys and `shared/Gamepad/GamepadManager.h`. Raw input arrives as
  `MESSAGE_TYPE_GUI_*` messages (see `shared/Manager/MessageManager.h`).
- **Particles**: `shared/Renderer/LinearParticle.h` +
  `shared/Renderer/linearparticle/`.
- **Physics / 3D**: `shared/Bullet/` and `shared/Irrlicht/`
  (`IrrlichtManager.h`); both optional, only compiled if the app includes
  them.
- **Write in-app tests**: `shared/testfw/ProtonTester.h`.
- **Pack assets** (.rttex/.rtfont): the `RTPack/` tool; formats in
  `shared/util/RTFileFormat.h` and `shared/GUI/RTFontFileFormat.h`.
- **Show a console/log overlay**: `shared/Manager/Console.h` +
  `shared/Entity/LogDisplayComponent.h`.

## Other docs

- `AGENTS.md`: operating rules, build/test commands, gotchas (read first).
- `docs/renderer-migration.md`: the shader pipeline design and the rt* GL
  call rule.
- `tests/README.md`: the golden-screenshot regression harness.
- `RTShader/README.md`: the shader sample.
- https://www.protonsdk.com : the original wiki/docs site.
