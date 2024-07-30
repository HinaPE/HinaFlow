cmake_minimum_required(VERSION 3.26)
cmake_policy(SET CMP0091 NEW)
set(CMAKE_MSVC_RUNTIME_LIBRARY MultiThreadedDLL)
set(CMAKE_CXX_STANDARD 17)
project(HinaFlow)

# Find Houdini
include(./FindHoudini.cmake)

set(HDK_NODEs
        SolvePoisson
        SolveDiffusion
        SolveWave
)

set(SOURCES
        hdk/Entrance.cpp
        common.h
        src/poisson.cpp
        src/poisson.h
        src/diffusion.cpp
        src/diffusion.h
        src/wave.cpp
        src/wave.h
)
foreach (NODE ${HDK_NODEs})
    list(APPEND SOURCES hdk/GAS_${NODE}.cpp hdk/GAS_${NODE}.h)
    configure_file(./icon.png ${HOUDINI_INSTALL_DIR}/config/icons/DOP_${NODE}.png COPYONLY)
endforeach ()
add_library(SIM_HinaFlow SHARED ${SOURCES})
target_link_libraries(SIM_HinaFlow PUBLIC Houdini)
target_include_directories(SIM_HinaFlow PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})

houdini_configure_target(SIM_HinaFlow)
