file(GLOB SRC *.cpp *.h)


add_executable (PSVManager WIN32 ${SRC})

target_link_libraries(PSVManager imgui d3d9 Shlwapi)

target_include_directories (PSVManager PUBLIC ${EXTERNAL}/7z/CPP)
target_include_directories (PSVManager PUBLIC ${IMGUI_DIR})
target_include_directories (PSVManager PUBLIC ${IMGUI_DIR}/examples/directx9_example)

target_include_directories (PSVManager PUBLIC ${EXTERNAL}/endian/src)
