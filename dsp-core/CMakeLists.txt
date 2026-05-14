cmake_minimum_required(VERSION 3.16)
project(mygo_dsp_engine VERSION 1.0.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

# ---- DSP Engine Library ----
add_library(dsp_core
    src/WaveformGenerator.cpp
    src/FIRFilter.cpp
    src/IIRFilter.cpp
    src/Engine.cpp
    src/AutoFilterDesigner.cpp
)

target_include_directories(dsp_core PUBLIC
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
    $<INSTALL_INTERFACE:include>
)

target_compile_options(dsp_core PRIVATE
    $<$<CXX_COMPILER_ID:MSVC>:/W4 /utf-8>
    $<$<NOT:$<CXX_COMPILER_ID:MSVC>>:-Wall -Wextra -Wpedantic>
)

# ---- CLI executable (stdin/stdout IPC) ----
add_executable(mygo_dsp_engine
    src/main.cpp
)

target_link_libraries(mygo_dsp_engine PRIVATE dsp_core)

# ---- Static build for distribution ----
if(MINGW)
    set_target_properties(mygo_dsp_engine PROPERTIES
        LINK_FLAGS "-static -static-libgcc -static-libstdc++"
    )
endif()

# ---- Unit tests (optional, requires GoogleTest) ----
option(BUILD_TESTS "Build unit tests" OFF)
if(BUILD_TESTS)
    enable_testing()
    find_package(GTest REQUIRED)

    add_executable(dsp_tests
        tests/test_waveform.cpp
        tests/test_fir.cpp
        tests/test_iir.cpp
        tests/test_engine.cpp
        tests/test_auto_design.cpp
    )
    target_link_libraries(dsp_tests PRIVATE dsp_core GTest::GTest GTest::Main)
    target_include_directories(dsp_tests PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/include)

    add_test(NAME dsp_tests COMMAND dsp_tests)
endif()

# ---- Performance benchmarks ----
option(BUILD_BENCHMARKS "Build performance benchmarks" OFF)
if(BUILD_BENCHMARKS)
    add_executable(dsp_benchmarks
        tests/bench_engine.cpp
    )
    target_link_libraries(dsp_benchmarks PRIVATE dsp_core)
    target_include_directories(dsp_benchmarks PRIVATE
        ${CMAKE_CURRENT_SOURCE_DIR}/include
        ${CMAKE_CURRENT_SOURCE_DIR}/src
    )
    if(WIN32)
        target_link_libraries(dsp_benchmarks PRIVATE winmm)
    endif()
endif()

install(TARGETS dsp_core mygo_dsp_engine
    RUNTIME DESTINATION bin
    LIBRARY DESTINATION lib
    ARCHIVE DESTINATION lib
)
