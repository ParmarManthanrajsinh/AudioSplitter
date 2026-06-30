cmake_minimum_required(VERSION 3.20)
set(CMAKE_POLICY_VERSION_MINIMUM 3.5)
project(AudioSplitter
    VERSION 0.1.0
    LANGUAGES C CXX
)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

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

# ── RmlUi options ──────────────────────────────────────────────────
set(BUILD_SHARED_LIBS OFF CACHE BOOL "" FORCE)
set(RMLUI_SHELL OFF CACHE BOOL "" FORCE)
set(RMLUI_BACKEND Win32_GL2 CACHE STRING "" FORCE)
set(RMLUI_SAMPLES OFF CACHE BOOL "" FORCE)
set(RMLUI_LUA_BINDINGS OFF CACHE BOOL "" FORCE)
set(RMLUI_PRECOMPILED_HEADERS OFF CACHE BOOL "" FORCE)

add_subdirectory(ThirdParty/RmlUi)

# ── Executable ─────────────────────────────────────────────────────
add_executable(AudioSplitter WIN32
    Source/Main.cpp
    Source/App/Application.cpp
    Source/Audio/AudioManager.cpp
    Source/Platform/Window.cpp
    Source/Platform/Resource.rc
    Source/UI/UIManager.cpp
    ThirdParty/RmlUi/Backends/RmlUi_Platform_Win32.cpp
    ThirdParty/RmlUi/Backends/RmlUi_Renderer_GL3.cpp
    ThirdParty/RmlUi/Backends/RmlUi_Backend_Win32_GL3.cpp
)

target_include_directories(AudioSplitter
    PRIVATE
        Source
        ThirdParty/RmlUi/Backends
)

target_link_libraries(AudioSplitter
    PRIVATE
        RmlUi::RmlUi
        opengl32
)

if(WIN32)
  target_link_libraries(AudioSplitter PRIVATE imm32)
endif()

# ── Compiler / linker flags ────────────────────────────────────────
if(MSVC)
  target_compile_options(AudioSplitter PRIVATE /W4 /permissive-)
else()
  target_compile_options(AudioSplitter PRIVATE -Wall -Wextra -Wpedantic)

  # Static-link the C++ runtime so we don't need to ship 15 DLLs.
  target_link_options(AudioSplitter PRIVATE -static-libgcc -static-libstdc++ -static)

  # Release: strip all symbols → exe drops from ~94 MB to ~3-5 MB.
  if(CMAKE_BUILD_TYPE STREQUAL "Release" OR CMAKE_BUILD_TYPE STREQUAL "MinSizeRel")
    target_link_options(AudioSplitter PRIVATE -s)
  endif()
endif()

# ── Copy assets ────────────────────────────────────────────────────
add_custom_target(CopyAssets ALL
    COMMAND ${CMAKE_COMMAND} -E copy_directory
            ${CMAKE_SOURCE_DIR}/Assets
            $<TARGET_FILE_DIR:AudioSplitter>/Assets
    COMMENT "Copying assets to output directory"
)
add_dependencies(AudioSplitter CopyAssets)

