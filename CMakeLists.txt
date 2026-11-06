cmake_minimum_required(VERSION 3.21)
project(BlockBlast3D VERSION 1.0.0 LANGUAGES C CXX)

set(CMAKE_C_STANDARD 99)
set(CMAKE_C_STANDARD_REQUIRED ON)
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

option(BLOCKBLAST_HEADLESS_BUILD "Build without Linux desktop backends for automated checks" OFF)
set(BUILD_SHARED_LIBS OFF CACHE BOOL "" FORCE)
set(GLFW_BUILD_DOCS OFF CACHE BOOL "" FORCE)
set(GLFW_BUILD_TESTS OFF CACHE BOOL "" FORCE)
set(GLFW_BUILD_EXAMPLES OFF CACHE BOOL "" FORCE)
set(GLFW_INSTALL OFF CACHE BOOL "" FORCE)

if(BLOCKBLAST_HEADLESS_BUILD)
    if(NOT UNIX OR APPLE)
        message(FATAL_ERROR "BLOCKBLAST_HEADLESS_BUILD is supported only on Linux")
    endif()
    set(GLFW_BUILD_X11 OFF CACHE BOOL "" FORCE)
    set(GLFW_BUILD_WAYLAND OFF CACHE BOOL "" FORCE)
endif()

add_subdirectory(third_party/glfw EXCLUDE_FROM_ALL)

set(GLM_BUILD_LIBRARY OFF CACHE BOOL "" FORCE)
set(GLM_BUILD_TESTS OFF CACHE BOOL "" FORCE)
set(GLM_BUILD_INSTALL OFF CACHE BOOL "" FORCE)
add_subdirectory(third_party/glm EXCLUDE_FROM_ALL)

add_library(glad STATIC third_party/glad/src/gl.c)
target_include_directories(glad PUBLIC third_party/glad/include)
target_link_libraries(glad PRIVATE ${CMAKE_DL_LIBS})

add_executable(BlockBlast3D
    src/main.cpp
    src/App.cpp
    src/OrbitCamera.cpp
    src/Game.cpp
    src/Shader.cpp
    src/Renderer.cpp
    src/Texture.cpp
    src/Hud.cpp
)

target_include_directories(BlockBlast3D PRIVATE src)
target_compile_definitions(BlockBlast3D PRIVATE GLFW_INCLUDE_NONE)
target_link_libraries(BlockBlast3D PRIVATE glfw glad glm::glm)

if(MSVC)
    target_compile_options(BlockBlast3D PRIVATE /W4 /permissive- /utf-8)
else()
    target_compile_options(BlockBlast3D PRIVATE -Wall -Wextra -Wpedantic)
endif()

add_custom_command(TARGET BlockBlast3D POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_directory
        "${CMAKE_CURRENT_SOURCE_DIR}/assets"
        "$<TARGET_FILE_DIR:BlockBlast3D>/assets"
    VERBATIM
)

set_property(DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR} PROPERTY VS_STARTUP_PROJECT BlockBlast3D)
set_property(TARGET BlockBlast3D PROPERTY VS_DEBUGGER_WORKING_DIRECTORY "$<TARGET_FILE_DIR:BlockBlast3D>")

include(CTest)
if(BUILD_TESTING)
    add_executable(GameTests tests/GameTests.cpp src/Game.cpp)
    target_include_directories(GameTests PRIVATE src)
    if(MSVC)
        target_compile_options(GameTests PRIVATE /W4 /permissive- /utf-8)
    else()
        target_compile_options(GameTests PRIVATE -Wall -Wextra -Wpedantic)
    endif()
    add_test(NAME GameLogic COMMAND GameTests)
endif()

install(TARGETS BlockBlast3D RUNTIME DESTINATION .)
install(DIRECTORY assets DESTINATION .)
