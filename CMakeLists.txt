cmake_minimum_required(VERSION 3.26)
cmake_policy(SET CMP0091 NEW)
set(CMAKE_MSVC_RUNTIME_LIBRARY MultiThreadedDLL)
set(CMAKE_CXX_STANDARD 17)
project(HinaFlow)

# Find Houdini
include(./FindHoudini.cmake)

set(DOP_NODEs
        SolvePoisson
        SolveDiffusion
        SolveWave
)
set(SOP_NODEs
        Image2Volume
)

set(SOURCES
        hdk/Entrance.cpp
        common.h
        src/diffusion.cpp
        src/diffusion.h
        src/image.cpp
        src/image.h
        src/poisson.cpp
        src/poisson.h
        src/tomography.cpp
        src/tomography.h
        src/wave.cpp
        src/wave.h

        hdk/SOP_Image2Volume.cpp
        hdk/SOP_Image2Volume.h
)

foreach (NODE ${DOP_NODEs})
    list(APPEND SOURCES hdk/GAS_${NODE}.cpp hdk/GAS_${NODE}.h)
    configure_file(./icon.png ${HOUDINI_INSTALL_DIR}/config/icons/DOP_${NODE}.png COPYONLY)
endforeach ()

foreach (NODE ${SOP_NODEs})
    list(APPEND SOURCES hdk/SOP_${NODE}.cpp hdk/SOP_${NODE}.h)
    configure_file(./icon.png ${HOUDINI_INSTALL_DIR}/config/icons/SOP_${NODE}.png COPYONLY)
    houdini_generate_proto_headers(FILES hdk/SOP_${NODE}.cpp)
endforeach ()

add_library(SIM_HinaFlow SHARED ${SOURCES})
target_link_libraries(SIM_HinaFlow PUBLIC Houdini)
target_include_directories(SIM_HinaFlow PUBLIC ${CMAKE_CURRENT_SOURCE_DIR} ${CMAKE_CURRENT_BINARY_DIR})

houdini_configure_target(SIM_HinaFlow)
