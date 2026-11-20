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
    src/Environment.cpp
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

add_custom_target(BlockBlastAssets ALL
    COMMAND ${CMAKE_COMMAND} -E copy_directory
        "${CMAKE_CURRENT_SOURCE_DIR}/assets"
        "$<TARGET_FILE_DIR:BlockBlast3D>/assets"
    VERBATIM
)

add_dependencies(BlockBlastAssets BlockBlast3D)

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

if(BUILD_TESTING)
    add_executable(EnvironmentTests tests/EnvironmentTests.cpp src/Environment.cpp)
    target_include_directories(EnvironmentTests PRIVATE src)
    target_link_libraries(EnvironmentTests PRIVATE glm::glm)
    add_test(NAME EnvironmentAndSun COMMAND EnvironmentTests)
endif()

option(BLOCKBLAST_EGL_TEST "Linux surfaceless rendering checks (not needed on Windows)" OFF)
if(BUILD_TESTING AND BLOCKBLAST_EGL_TEST)
    if(NOT UNIX OR APPLE)
        message(FATAL_ERROR "EGL checks require Linux; leave this option OFF on Windows")
    endif()
    find_library(BLOCKBLAST_EGL NAMES EGL libEGL.so.1 REQUIRED)
    get_target_property(BLOCKBLAST_SOURCES BlockBlast3D SOURCES)
    list(REMOVE_ITEM BLOCKBLAST_SOURCES src/main.cpp)
    add_executable(RenderTests tests/RenderTests.cpp ${BLOCKBLAST_SOURCES})
    target_include_directories(RenderTests PRIVATE src)
    target_link_libraries(RenderTests PRIVATE glad glm::glm ${BLOCKBLAST_EGL} ${CMAKE_DL_LIBS})
    add_dependencies(RenderTests BlockBlastAssets)
    add_test(NAME RealOpenGL COMMAND RenderTests "${CMAKE_BINARY_DIR}/assets/shaders")
    set_tests_properties(RealOpenGL PROPERTIES WORKING_DIRECTORY "${CMAKE_BINARY_DIR}")
endif()
