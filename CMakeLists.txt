cmake_minimum_required(VERSION 2.8)
project(simple_thread)

if(WIN32)
	set(SRC_FILES ${SRC_FILES} simple_thread_comm.c win32_simple_thread.c)
else()
	set(SRC_FILES ${SRC_FILES} simple_thread_comm.c posix_simple_thread.c)
endif()

add_library(simple_thread STATIC ${SRC_FILES})

if(WIN32)
	target_link_libraries(simple_thread ws2_32)
else()
	target_link_libraries(simple_thread pthread)
endif()


# TODO: add cunit and cunit tests
# add_subdirectory(test/unit)

# EXPORT_HEADERS(simple_thread simple_thread.h)
# EXPORT_AS_DEPENDENCY(simple_thread)
