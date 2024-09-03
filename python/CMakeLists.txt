cmake_minimum_required(VERSION 3.26)
set(CMAKE_CXX_STANDARD 17)
project(HinaFlowPython)

add_subdirectory(pybind11-2.13.5)
pybind11_add_module(HinaFlow main.cpp)
target_compile_definitions(HinaFlow PRIVATE VERSION_INFO=0.0.1)
