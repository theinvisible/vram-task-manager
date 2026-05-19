cmake_minimum_required(VERSION 3.21)
project(vram-task-manager VERSION 0.1.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)
set(CMAKE_AUTOUIC ON)

if(NOT WIN32)
    message(FATAL_ERROR "vram-task-manager uses Windows-only PDH counters and only builds on Windows.")
endif()

# Try the standard find_package first — covers CI (install-qt-action sets
# Qt6_DIR / CMAKE_PREFIX_PATH / QT_ROOT_DIR) and any explicit user-supplied prefix.
find_package(Qt6 QUIET COMPONENTS Widgets)

# Local-dev fallback: scan known Qt roots (Desktop: H:/Qt, Notebook: C:/Qt) for
# the newest 6.x MSVC/MinGW kit.
if(NOT Qt6_FOUND)
    set(_qt_search_roots "H:/Qt" "C:/Qt")
    set(_qt_found_dir "")
    foreach(_qt_root IN LISTS _qt_search_roots)
        file(GLOB _qt_versions LIST_DIRECTORIES true "${_qt_root}/6.*")
        list(SORT _qt_versions ORDER DESCENDING)
        foreach(_qt_ver IN LISTS _qt_versions)
            file(GLOB _qt_kits LIST_DIRECTORIES true "${_qt_ver}/msvc*_64" "${_qt_ver}/mingw*_64")
            foreach(_qt_kit IN LISTS _qt_kits)
                if(EXISTS "${_qt_kit}/lib/cmake/Qt6/Qt6Config.cmake")
                    set(_qt_found_dir "${_qt_kit}")
                    break()
                endif()
            endforeach()
            if(_qt_found_dir)
                break()
            endif()
        endforeach()
        if(_qt_found_dir)
            break()
        endif()
    endforeach()
    if(_qt_found_dir)
        set(Qt6_DIR "${_qt_found_dir}/lib/cmake/Qt6" CACHE PATH "Qt6 config dir" FORCE)
        list(PREPEND CMAKE_PREFIX_PATH "${_qt_found_dir}")
        set(CMAKE_PREFIX_PATH "${CMAKE_PREFIX_PATH}" CACHE STRING "" FORCE)
        message(STATUS "Auto-detected Qt6 at ${_qt_found_dir}")
        find_package(Qt6 REQUIRED COMPONENTS Widgets)
    else()
        message(FATAL_ERROR "Qt6 nicht gefunden — Qt6_DIR/CMAKE_PREFIX_PATH setzen oder Qt unter H:/Qt bzw. C:/Qt installieren.")
    endif()
endif()

add_executable(vram-task-manager WIN32
    src/main.cpp
    src/MainWindow.h
    src/MainWindow.cpp
    src/VramModel.h
    src/VramModel.cpp
    src/VramSampler.h
    src/VramSampler.cpp
    src/NvmlSampler.h
    src/NvmlSampler.cpp
    src/GpuInventory.h
    src/GpuInventory.cpp
    resources/app.rc
    resources/app.qrc
)

target_link_libraries(vram-task-manager PRIVATE
    Qt6::Widgets
    pdh
    psapi
    dxgi
)

target_compile_definitions(vram-task-manager PRIVATE
    UNICODE
    _UNICODE
    NOMINMAX
    WIN32_LEAN_AND_MEAN
)

# Copy required Qt DLLs (and platform plugin) next to the executable so it
# launches from inside CLion without needing Qt on PATH.
get_target_property(_qt_qmake_exe Qt6::qmake IMPORTED_LOCATION)
get_filename_component(_qt_bin_dir "${_qt_qmake_exe}" DIRECTORY)
find_program(WINDEPLOYQT_EXE windeployqt HINTS "${_qt_bin_dir}")

if(WINDEPLOYQT_EXE)
    add_custom_command(TARGET vram-task-manager POST_BUILD
        COMMAND "${WINDEPLOYQT_EXE}"
                --no-translations
                --no-system-d3d-compiler
                --no-opengl-sw
                --no-quick-import
                $<IF:$<CONFIG:Debug>,--debug,--release>
                "$<TARGET_FILE:vram-task-manager>"
        COMMENT "Running windeployqt"
        VERBATIM
    )
else()
    message(WARNING "windeployqt nicht gefunden — Qt-DLLs muessen manuell neben die EXE.")
endif()
