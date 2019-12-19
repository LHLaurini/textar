set(PUBLIC_HEADERS "${CMAKE_CURRENT_SOURCE_DIR}/textar.h"
                   "${CMAKE_CURRENT_SOURCE_DIR}/textar-internals.h")

add_library(textar textar.c entry.c list.c path.c stdimp.c)
set_target_properties(textar PROPERTIES PUBLIC_HEADER "${PUBLIC_HEADERS}")

add_executable(textar-bin main.c)
target_link_libraries(textar-bin PUBLIC textar)
set_target_properties(textar-bin PROPERTIES OUTPUT_NAME textar)