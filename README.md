# BlockBlast3D

BlockBlast3D is a block-placement puzzle built with C++17 and OpenGL 3.3 Core. Place pieces on an 8 × 8 grid, clear full rows and columns, and build a combo by clearing lines on consecutive moves. The board is rendered in 3D and can be played in three outdoor environments.

Developed for **Computer Graphics Fundamentals (OKG 2214)** at **Astana IT University** by **SE-2407**: Marat Yerkanat, Ayagan Bagdat, Yeleussinov Daniyal, and Dias Madyshev.

## Run on Windows

1. Download or clone the repository. If you downloaded a ZIP, extract it and open the folder containing `CMakeLists.txt`.
2. In Visual Studio Installer, install **Desktop development with C++** and **C++ CMake tools for Windows**. Use a graphics driver that supports OpenGL 3.3 Core.
3. Run [`BUILD_AND_RUN.bat`](BUILD_AND_RUN.bat). It configures an x64 Release build with Ninja, runs the configured tests, and launches the game.

After the first build, you can launch `build-ninja\BlockBlast3D.exe` directly. Keep the generated `assets` folder beside the executable. GLFW, GLAD, and GLM are bundled under `third_party/`.

### Build with Visual Studio CMake presets

Run these commands from the repository root in PowerShell. The Visual Studio 2026 preset requires CMake 4.2 or newer.

```powershell
cmake --preset windows-vs2026
cmake --build --preset windows-vs2026-release
ctest --preset windows-vs2026-release
.\build-vs2026\Release\BlockBlast3D.exe
```

For Visual Studio 2022, replace `vs2026` with `vs2022` in the presets and executable path.

## Controls

| Input | Action |
| --- | --- |
| Left click or drag | Select and place a piece |
| `1` / `2` / `3` | Select a piece from the tray |
| `Esc` | Cancel selection |
| Right mouse button + drag | Orbit the camera around the board |
| Mouse wheel | Zoom in or out |
| `R` | Reset the camera |
| `N` / `F2` | Start a new game / open the demo board |
| `T` | Cycle through the park, beach, and winter garden |
| `P` | Pause or resume animations and piece placement |
| `B` / `Q` | Toggle shadows / switch graphics quality |
| `←` / `→` | Adjust the sun's azimuth |
| `↑` / `↓` or `Page Up` / `Page Down` | Adjust the sun's elevation |
| `L` | Reset the sun direction |
| `F1` / `H` | Open the controls and team information |

## Gameplay and scoring

- Each piece must fit entirely within the board and occupy only empty cells. The placement preview is green for a valid move and red for an invalid one.
- The game deals three new pieces after all three pieces in the current tray are placed.
- A move clears all full rows and columns at once. Cells where cleared rows and columns intersect are removed once, while each completed line counts toward the score.
- A move earns `10 × placed cubes + 100 × cleared lines × combo`. The combo starts at 1 on the first clearing move, increases with consecutive clearing moves, and resets to 0 on a move that clears no lines.
- The game ends when none of the remaining pieces fits anywhere on the board.

Press **F2** to load a demo with two empty cells in the front row and a matching piece already selected. Fill the gap to demonstrate line clearing. Demo scores are excluded from the saved record. On Windows, the record is stored in `%LOCALAPPDATA%\AITUBlockBlast3D\best.txt`. On Linux, it is stored in `$XDG_DATA_HOME/AITUBlockBlast3D/best.txt`, with `~/.local/share/AITUBlockBlast3D/best.txt` as the fallback.

## Environments and rendering

- **Garden Park** features rounded trees, flower beds, stone paths, and wooden benches.
- **Sunlit Coast** features palms, beach umbrellas, lounge chairs, and animated water.
- **Winter Garden** features snowy trees, falling snow, and a decorated holiday tree.

Switching environments preserves the board and score. The sun uses a world-space direction shared by the lighting, shadows, and visible solar disc. Moving the camera leaves that direction fixed. The controls adjust azimuth and elevation, with elevation limited to 8–82°.

The renderer uses GLSL 330, procedural materials, GGX specular highlights, and percentage-closer filtering (PCF) for the shadow map. The sky shader draws clouds and the sun in the distance. **High** quality uses a 2048 × 2048 shadow map and more scenery detail. **Low** quality uses a 1024 × 1024 map and fewer decorative objects.

## Project layout

| Path | Responsibility |
| --- | --- |
| `src/Game.cpp`, `src/Game.hpp` | Board rules, piece generation, scoring, and game-over detection |
| `src/App.cpp`, `src/main.cpp`, `src/OrbitCamera.cpp` | Input, application state, and camera controls |
| `src/Environment.cpp`, `src/Renderer.cpp`, `src/Hud.cpp` | Procedural scenery, rendering passes, and interface |
| `assets/shaders/` | World, shadow depth, sky, and interface shaders |
| `tests/` | Gameplay, environment, and OpenGL integration tests |
| `third_party/` | GLFW 3.4, GLAD for OpenGL 3.3 Core, and GLM 1.0.1 |

The root `CMakeLists.txt` requires CMake 3.21 or newer and copies runtime assets to the build output.

## Validation

The integrated build passed `GameLogic`, `EnvironmentAndSun`, and `RealOpenGL` in a Linux Release build. The OpenGL test used EGL with Mesa software rendering to check scene rendering, shadow toggling, piece placement with the mouse, score retention during scene switches, window resizing, and stable sunlight direction. Windows launch and GPU performance can be verified on the target machine using the build steps above.
