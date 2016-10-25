cmake_minimum_required (VERSION 3.6)

project (PSVManager)

set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_SOURCE_DIR}/bin)

set(EXTERNAL ${CMAKE_SOURCE_DIR}/external)

set(IMGUI_DIR ${EXTERNAL}/imgui)
file(GLOB IMGUI_SRC ${IMGUI_DIR}/*.cpp ${IMGUI_DIR}/*.h ${IMGUI_DIR}/examples/directx9_example/imgui_impl_dx9.*)
add_library(imgui ${IMGUI_SRC})
target_include_directories (imgui PUBLIC ${IMGUI_DIR})

add_subdirectory (src)

set_property(DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR} PROPERTY VS_STARTUP_PROJECT PSVManager)
