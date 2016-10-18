cmake_minimum_required (VERSION 3.6)

project (PSVManager)

set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_SOURCE_DIR}/bin)

set(EXTERNAL ${CMAKE_SOURCE_DIR}/external)

set(IMGUI_DIR ${EXTERNAL}/imgui)

file(GLOB IMGUI_SRC ${IMGUI_DIR}/*.cpp ${IMGUI_DIR}/*.h)

add_library(imgui ${IMGUI_SRC})


add_subdirectory (src)

set_property(DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR} PROPERTY VS_STARTUP_PROJECT PSVManager)
