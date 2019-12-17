
enable_testing()
find_package(Check REQUIRED)
include_directories(${CHECK_INCLUDE_DIRS} ${CMAKE_SOURCE_DIR}/src)

function(add_test2 test)
	add_custom_command(OUTPUT ${test}.c
	                   DEPENDS ${test}.ts
	                   COMMAND checkmk ${CMAKE_CURRENT_SOURCE_DIR}/${test}.ts > ${CMAKE_CURRENT_BINARY_DIR}/${test}.c)
	add_executable(${test} ${test}.c)
	target_link_libraries(${test} PUBLIC textar ${CHECK_LIBRARIES})
	add_test(NAME ${test} COMMAND ${test})
endfunction(add_test2)

add_test2(entry)
add_test2(list)
add_test2(pathfn)
add_test(NAME archive COMMAND ${CMAKE_CURRENT_SOURCE_DIR}/archive.sh)
