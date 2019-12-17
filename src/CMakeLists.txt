add_library(textar textar.c entry.c list.c path.c)

add_executable(textar-bin main.c)
target_link_libraries(textar-bin PUBLIC textar)
set_target_properties(textar-bin PROPERTIES OUTPUT_NAME textar)