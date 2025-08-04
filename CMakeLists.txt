cmake_minimum_required(VERSION 3.10)
project(search3)

set (SOURCE_FILES
	src/search.c
	error/error.c
	linked_list/output.c
)

if (CMAKE_C_COMPILER_ID MATCHES "GNU|Clang")
	add_compile_options(-Wall -Werror -pedantic)
endif()


add_executable(search3 ${SOURCE_FILES})
target_link_options(search3 PRIVATE "-municode")

add_library (getopt_dll SHARED getopt/getopt.c)
target_compile_definitions(getopt_dll PRIVATE EXPORTS_GETOPT)

target_link_libraries(search3 PRIVATE getopt_dll)
