function(test_func TEST_NAME)
	add_executable(${TEST_NAME} "${TEST_NAME}.cpp")
	target_include_directories(${TEST_NAME} PRIVATE "../src")
	target_link_libraries(${TEST_NAME} PRIVATE xs)
	set_property(TARGET ${TEST_NAME} PROPERTY CXX_STANDARD 20)
endfunction()

test_func(zip_test)
test_func(json_test)
test_func(xlib_test)
test_func(parse_test)

execute_process(COMMAND ${CMAKE_COMMAND} -E copy "${CMAKE_CURRENT_SOURCE_DIR}/test.xs"  "${CMAKE_BINARY_DIR}/tests/Debug/" )
#add_custom_command( TARGET parse_test POST_BUILD copy "${CMAKE_CURRENT_SOURCE_DIR}/test.xs"  "${CMAKE_BINARY_DIR}" )

#message(status "${CMAKE_BINARY_DIR}/tests/Debug" )

#add_custom_command(TARGET parse_test POST_BUILD COMMAND copy ${CMAKE_CURRENT_SOURCE_DIR}/test.xs  ${CMAKE_BINARY_DIR}/tests/Debug/ )