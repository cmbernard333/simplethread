cmake_minimum_required(VERSION 2.8)
project(test-simple_thread)

set(cunit_DIR CUnit-2.1-3)
set(cunit_INCLUDE_PATH ${CMAKE_CURRENT_SOURCE_DIR}/${cunit_DIR}/build-${cunit_DIR}/include)
set(cunit_LIB_PATH ${CMAKE_CURRENT_SOURCE_DIR}/${cunit_DIR}/build-${cunit_DIR}/lib)

include_directories("../..")
link_directories(${cunit_LIB_PATH})

set(TEST_SRC_FILES
	simple_thread_suite.c
        cutest.c
)

include_directories(${cunit_INCLUDE_PATH})
add_executable(cutest ${TEST_SRC_FILES})
target_link_libraries(cutest cunit simple_thread)

if(WIN32)
	target_link_libraries(cutest ws2_32)
else()
	target_link_libraries(cutest pthread)
endif()
