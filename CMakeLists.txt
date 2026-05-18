cmake_minimum_required(VERSION 3.21)
project(vram-task-manager LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)
set(CMAKE_AUTOUIC ON)

if(NOT WIN32)
    message(FATAL_ERROR "vram-task-manager uses Windows-only PDH counters and only builds on Windows.")
endif()

# Auto-discover Qt6 under H:/Qt unless an explicit Qt6_DIR is already set.
if(NOT Qt6_DIR OR NOT EXISTS "${Qt6_DIR}/Qt6Config.cmake")
    set(_qt_found_dir "")
    file(GLOB _qt_versions LIST_DIRECTORIES true "H:/Qt/6.*")
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
        set(Qt6_DIR "${_qt_found_dir}/lib/cmake/Qt6" CACHE PATH "Qt6 config dir" FORCE)
        list(PREPEND CMAKE_PREFIX_PATH "${_qt_found_dir}")
        set(CMAKE_PREFIX_PATH "${CMAKE_PREFIX_PATH}" CACHE STRING "" FORCE)
        message(STATUS "Auto-detected Qt6 at ${_qt_found_dir}")
    else()
        message(FATAL_ERROR "Qt6 nicht unter H:/Qt/6.*/msvc*_64 gefunden — bitte Qt6_DIR manuell setzen.")
    endif()
endif()

find_package(Qt6 REQUIRED COMPONENTS Widgets)

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
    resources/app.rc
    resources/app.qrc
)

target_link_libraries(vram-task-manager PRIVATE
    Qt6::Widgets
    pdh
    psapi
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
