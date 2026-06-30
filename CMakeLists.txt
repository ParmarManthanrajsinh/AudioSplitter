cmake_minimum_required(VERSION 3.20)
set(CMAKE_POLICY_VERSION_MINIMUM 3.5)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

project(AudioSplitter
    VERSION 0.1.0
    LANGUAGES C CXX
)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

if(WIN32 AND EXISTS "C:/msys64/ucrt64")
  list(APPEND CMAKE_PREFIX_PATH "C:/msys64/ucrt64")
  set(MSYS2_UCRT64_BIN "C:/msys64/ucrt64/bin")
endif()

# ── Freetype ───────────────────────────────────────────────────────
set(FT_DISABLE_ZLIB ON CACHE BOOL "Disable Zlib" FORCE)
set(FT_DISABLE_BZIP2 ON CACHE BOOL "Disable Bzip2" FORCE)
set(FT_DISABLE_PNG ON CACHE BOOL "Disable PNG" FORCE)
set(FT_DISABLE_HARFBUZZ ON CACHE BOOL "Disable HarfBuzz" FORCE)
set(FT_DISABLE_BROTLI ON CACHE BOOL "Disable Brotli" FORCE)

add_subdirectory(ThirdParty/Freetype)
add_library(Freetype::Freetype ALIAS freetype)

# RmlUi
set(BUILD_SHARED_LIBS OFF CACHE BOOL "" FORCE)
set(RMLUI_SHELL OFF CACHE BOOL "" FORCE)
set(RMLUI_SAMPLES OFF CACHE BOOL "" FORCE)
set(RMLUI_LUA_BINDINGS OFF CACHE BOOL "" FORCE)
set(RMLUI_PRECOMPILED_HEADERS OFF CACHE BOOL "" FORCE)

if(WIN32)
  set(RMLUI_BACKEND Win32_GL2 CACHE STRING "" FORCE)
else()
  find_package(SDL3 REQUIRED CONFIG)
  find_package(SDL3_image REQUIRED CONFIG)
  set(RMLUI_BACKEND SDL_GL3 CACHE STRING "" FORCE)
endif()

add_subdirectory(ThirdParty/RmlUi)
find_package(OpenGL REQUIRED)
set(AUDIO_SPLITTER_SOURCES
    Source/Main.cpp
    Source/App/Application.cpp
    Source/Audio/AudioManager.cpp
    Source/Platform/Window.cpp
    Source/UI/UIManager.cpp
    ThirdParty/RmlUi/Backends/RmlUi_Renderer_GL3.cpp
)

if(WIN32)
  list(APPEND AUDIO_SPLITTER_SOURCES
        Source/Platform/Resource.rc
        ThirdParty/RmlUi/Backends/RmlUi_Platform_Win32.cpp
        ThirdParty/RmlUi/Backends/RmlUi_Backend_Win32_GL3.cpp
    )
  add_executable(AudioSplitter WIN32
        ${AUDIO_SPLITTER_SOURCES}
    )
else()
  list(APPEND AUDIO_SPLITTER_SOURCES
        ThirdParty/RmlUi/Backends/RmlUi_Platform_SDL.cpp
        ThirdParty/RmlUi/Backends/RmlUi_Backend_SDL_GL3.cpp
    )

  add_executable(AudioSplitter
        ${AUDIO_SPLITTER_SOURCES}
    )
endif()

target_include_directories(AudioSplitter PRIVATE
    Source
    ThirdParty/RmlUi/Backends
)

if(UNIX AND NOT APPLE)
  target_compile_definitions(AudioSplitter PRIVATE
        RMLUI_SDL_VERSION_MAJOR=3
    )
endif()

target_link_libraries(AudioSplitter PRIVATE
    RmlUi::RmlUi
    OpenGL::GL
)

if(WIN32)
  target_link_libraries(AudioSplitter PRIVATE
        imm32
    )
else()
  target_link_libraries(AudioSplitter PRIVATE
        SDL3::SDL3
        SDL3_image::SDL3_image
    )
endif()

if(MSVC)
  target_compile_options(AudioSplitter PRIVATE
        /W4
        /permissive-
    )
else()
  target_compile_options(AudioSplitter PRIVATE
        -Wall
        -Wextra
        -Wpedantic
    )
  if(WIN32)
    target_link_options(AudioSplitter PRIVATE
            -static-libgcc
            -static-libstdc++
            -static
        )
  endif()

  if(CMAKE_BUILD_TYPE STREQUAL "Release"
       OR CMAKE_BUILD_TYPE STREQUAL "MinSizeRel")
    target_link_options(AudioSplitter PRIVATE
            -s
        )
  endif()
endif()

add_custom_target(CopyAssets ALL
    COMMAND ${CMAKE_COMMAND} -E copy_directory
            ${CMAKE_SOURCE_DIR}/Assets
            $<TARGET_FILE_DIR:AudioSplitter>/Assets
    COMMENT "Copying assets to output directory"
)

add_dependencies(AudioSplitter CopyAssets)
