cmake_minimum_required(VERSION 3.20)

project(AudioSplitter
    VERSION 0.1.0
    LANGUAGES CXX
)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

if(WIN32 AND EXISTS "C:/msys64/ucrt64")
  list(APPEND CMAKE_PREFIX_PATH "C:/msys64/ucrt64")
  set(MSYS2_UCRT64_BIN "C:/msys64/ucrt64/bin")
endif()

set(BUILD_SHARED_LIBS OFF CACHE BOOL "" FORCE)
set(RMLUI_SHELL OFF CACHE BOOL "" FORCE)
set(RMLUI_BACKEND Win32_GL2 CACHE STRING "" FORCE)
set(RMLUI_SAMPLES OFF CACHE BOOL "" FORCE)
set(RMLUI_LUA_BINDINGS OFF CACHE BOOL "" FORCE)
set(RMLUI_PRECOMPILED_HEADERS OFF CACHE BOOL "" FORCE)

add_subdirectory(ThirdParty/RmlUi)

add_executable(AudioSplitter WIN32
    Source/Main.cpp
    Source/App/Application.cpp
    Source/Audio/AudioManager.cpp
    Source/Platform/Window.cpp
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

if(MSVC)
  target_compile_options(AudioSplitter PRIVATE /W4 /permissive-)
else()
  target_compile_options(AudioSplitter PRIVATE -Wall -Wextra -Wpedantic)
endif()

add_custom_target(CopyAssets ALL
    COMMAND ${CMAKE_COMMAND} -E copy_directory
            ${CMAKE_SOURCE_DIR}/Assets
            $<TARGET_FILE_DIR:AudioSplitter>/Assets
    COMMAND ${CMAKE_COMMAND} -E copy_if_different
            ${CMAKE_SOURCE_DIR}/ThirdParty/RmlUi/Samples/assets/LatoLatin-Regular.ttf
            $<TARGET_FILE_DIR:AudioSplitter>/Assets/Fonts/LatoLatin-Regular.ttf
    COMMENT "Copying assets to output directory"
)
add_dependencies(AudioSplitter CopyAssets)

if(WIN32 AND DEFINED MSYS2_UCRT64_BIN)
  set(AUDIOSPLITTER_RUNTIME_DLLS
        libbrotlicommon.dll
        libbrotlidec.dll
        libbz2-1.dll
        libfreetype-6.dll
        libgcc_s_seh-1.dll
        libglib-2.0-0.dll
        libgraphite2.dll
        libharfbuzz-0.dll
        libiconv-2.dll
        libintl-8.dll
        libpcre2-8-0.dll
        libpng16-16.dll
        libstdc++-6.dll
        libwinpthread-1.dll
        zlib1.dll
    )

  foreach(RuntimeDll IN LISTS AUDIOSPLITTER_RUNTIME_DLLS)
    if(EXISTS "${MSYS2_UCRT64_BIN}/${RuntimeDll}")
      add_custom_command(
                TARGET AudioSplitter
                POST_BUILD
                COMMAND ${CMAKE_COMMAND} -E copy_if_different
                        "${MSYS2_UCRT64_BIN}/${RuntimeDll}"
                        "$<TARGET_FILE_DIR:AudioSplitter>/${RuntimeDll}"
                COMMENT "Copying ${RuntimeDll}"
            )
    endif()
  endforeach()
endif()
