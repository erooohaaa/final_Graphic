# Third-party components and credits

This package includes the following libraries. Preserve their notices when sharing the project or preparing the final submission.

| Component | Bundled version | Purpose | Upstream source | License notice |
|---|---|---|---|---|
| GLFW | 3.4 | Window, input, OpenGL context | [glfw/glfw 3.4](https://github.com/glfw/glfw/tree/3.4) | `third_party/glfw/LICENSE.md`; zlib/libpng license |
| GLM | 1.0.1 | Vectors and matrix operations | [g-truc/glm 1.0.1](https://github.com/g-truc/glm/tree/1.0.1) | `third_party/glm/copying.txt`; used under its MIT option |
| GLAD | Generated with 2.0.8; OpenGL 3.3 core, no extensions | OpenGL function loading | [Dav1dde/glad v2.0.8](https://github.com/Dav1dde/glad/tree/v2.0.8) | `third_party/glad/LICENSE` and generated-file notices |

GLAD's generator is MIT-licensed. Its generated files identify their license as `(WTFPL OR CC0-1.0) AND Apache-2.0`; this project uses the CC0 alternative together with the Khronos Apache-2.0 terms. `third_party/glad/include/KHR/khrplatform.h` carries its own Khronos notice. Keep these notices with the generated loader.

Bundled dependency metadata, upstream archive URLs and hashes are recorded in `third_party/DEPENDENCIES.json`. GLAD generation options are recorded in `third_party/glad/GENERATION.txt`. The copied GLFW/GLM runtime sources and generated loader are unchanged; unused upstream documentation, examples and tests were omitted from the supplied dependency snapshot.

## Project-created content

The game uses the general block-placement and line-clearing puzzle concept. It is an independent educational OpenGL implementation and is not an official product of Roblox or Block Blast. No code or graphics from those games are included.

## Tools and documentation

CMake, Ninja, Visual Studio and the operating system's graphics driver are build/runtime tools, not redistributed project source. The Windows launcher uses Visual Studio's existing `vswhere.exe`, developer environment and CMake tools.

- [GLFW 3.4 documentation](https://www.glfw.org/docs/3.4/)
- [GLM 1.0.1 repository](https://github.com/g-truc/glm/tree/1.0.1)
- [GLAD repository](https://github.com/Dav1dde/glad)
- [CMake Visual Studio 2026 generator](https://cmake.org/cmake/help/v4.2/generator/Visual%20Studio%2018%202026.html)
- [Microsoft: CMake projects in Visual Studio](https://learn.microsoft.com/en-us/cpp/build/cmake-projects-in-visual-studio)
