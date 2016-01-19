cmake_minimum_required(VERSION 2.8)
project(test-simple_thread)

include_directories("../..")

set(TEST_SRC_FILES
	simple_thread_suite.c
)

include_directories(${cunit_INCLUDE_PATH})
add_executable(cutest ${TEST_SRC_FILES})
target_link_libraries(cutest simple_thread)

if(WIN32)
	target_link_libraries(cutest ws2_32)
else()
	target_link_libraries(cutest pthread)
endif()
