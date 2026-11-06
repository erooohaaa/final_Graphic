# Validation record

Date: 2026-10-18. These checks were performed during preparation of this source package. They do not establish that the team has tested Windows or measured its own computer.

## Compilation and rules

- Linux, GCC 13.3, CMake 4.4.3, Release, C++17.
- All production C++ files compiled and linked successfully.
- CTest `GameLogic` passed. Checks include bounds and overlap rejection, unchanged state after an invalid move, row/column intersection clearing, refill, reset, reachable game-over states, and the scripted demo.
- Game rule checks also passed AddressSanitizer and UndefinedBehaviorSanitizer during development; leak detection was disabled in that environment.
- Project sources use `-Wall -Wextra -Wpedantic` on GCC. CMake prints dependency deprecation warnings for GLM's older minimum CMake version; these are not compilation errors.

To repeat compilation and rule checks on Linux without desktop development headers:

```bash
cmake -S . -B build-check -DBLOCKBLAST_HEADLESS_BUILD=ON -DCMAKE_BUILD_TYPE=Release
cmake --build build-check --parallel
ctest --test-dir build-check --output-on-failure
```

The headless build option disables GLFW desktop backends for compilation checks. It is not a promise that the regular game executable opens a window on a headless machine. Use the normal Windows build for interactive play.

## Actual OpenGL integration

Production App, Renderer, Texture, Hud, OrbitCamera, Shader, and Game modules were exercised in a real EGL pbuffer context with Mesa llvmpipe, reporting OpenGL 4.5 core. The shaders use GLSL 330. This was software OpenGL, not a mocked renderer and not a native Windows GLFW window.

| Check | Result |
|---|---|
| Shader compile/link, board, pieces, HUD | Passed; no OpenGL errors reported |
| Invalid pointer placement | Board and score unchanged |
| Demo valid click | Front row cleared; score 120 |
| Orbit, zoom, then pointer placement | Correct grid cell |
| Resize to 960x600, then placement | Correct grid cell |
| Card-to-board drag/drop | One valid placement |
| Five-cell tall preview | Entire shape fits its card after adaptive projection fix |
| Natural Game Over | Reached after 11 pointer-driven moves in the tested seed; score 430 |
| Play Again | New empty board, score reset |
| Ordinary best-score file | Written and readable |
| Demo scoring | Did not overwrite stored ordinary-game record |

`preview.png` is a capture of the actual production rendering in this check. Its FPS readout uses measured elapsed time in the software-rendering test loop. It is not a benchmark of the Windows computer or a guaranteed frame rate.

## Windows verification still required

The Visual Studio 2026 presets and automatic BAT launcher were reviewed, but not executed on Windows in this environment. The team's Windows machine must verify the compiler, driver, real GLFW mouse/keyboard callbacks, VSync and application launch. See `DEMO_AND_NEXT_STEPS_RU.md` for the short acceptance checklist.

Optional Windows smoke run after a successful build:

```powershell
.\build-ninja\BlockBlast3D.exe --demo --smoke-test
.\build-ninja\BlockBlast3D.exe --demo --smoke-test --screenshot demo.ppm
```

The smoke run renders eight frames and exits. The second command writes a PPM capture; it is not a video. The `--smoke-test` flag disables persistent score-file access for that run.
