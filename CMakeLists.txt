cmake_minimum_required(VERSION 2.8)
project(textar C)

option(BUILD_SHARED_LIBS "" ON)

set(CMAKE_MODULE_PATH ${CMAKE_SOURCE_DIR}/cmake)

add_subdirectory(src)
add_subdirectory(tests)

configure_file(textar.pc.in textar.pc @ONLY)

install(TARGETS textar-bin RUNTIME)
install(TARGETS textar LIBRARY)
install(FILES textar.pc DESTINATION lib/pkgconfig)
