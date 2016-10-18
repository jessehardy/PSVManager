file(GLOB SRC *.cpp *.h)


add_executable (PSVManager WIN32 ${SRC})

target_link_libraries(PSVManager imgui)

target_include_directories (PSVManager PUBLIC ${EXTERNAL}/7z/CPP)
target_include_directories (PSVManager PUBLIC ${IMGUI_DIR})


