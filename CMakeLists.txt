cmake_minimum_required(VERSION 3.26)
cmake_policy(SET CMP0091 NEW)
set(CMAKE_MSVC_RUNTIME_LIBRARY MultiThreadedDLL)
set(CMAKE_CXX_STANDARD 17)
project(HinaFlow)

include(./FindHoudini.cmake)

set(SRC_FILES
        diffusion
        flip
        image
        pbf
        phiflow_smoke
        poisson
        tomography
        wave
)
set(DOP_NODEs
        AdaptiveDomain
        DebugSPHKernel
        PhiFlowFetchField
        PhiFlowFreeSolve
        PhiFlowSmoke2D
        PhiFlowSmoke3D
        PhiFlowVisualizeField
        ReadNPZFiles
        ReconstructDensity
        SolveDiffusion
        SolvePoisson
        SolverFLIP
        SolverPBF
        SolverPhiFlow
        SolveWave
        VolumeRender
)
set(SOP_NODEs
        Image2Volume
)

set(SOURCES
        hdk/Entrance.cpp
        common.h
)

foreach (NODE ${SRC_FILES})
    list(APPEND SOURCES src/${NODE}.cpp src/${NODE}.h)
endforeach ()

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
