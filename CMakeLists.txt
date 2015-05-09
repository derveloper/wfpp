cmake_minimum_required(VERSION 3.2)
project(wfpp C CXX)

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++14 -ldl")
set(CMAKE_C_FLAGS "-Wall -Wextra -Wshadow -Wconversion -std=gnu11")
set(CMAKE_C_COMPILER "/usr/bin/clang")
set(CMAKE_CXX_COMPILER "/usr/bin/clang++")

set(LWAN_PATH ${CMAKE_SOURCE_DIR}/ext/lwan)
set(LWAN_INC_PATH ${LWAN_PATH}/common)
set(LWAN_BUILD_PATH ${LWAN_PATH}/build)
set(LWAN_SOURCES
	${LWAN_INC_PATH}/base64.c
	${LWAN_INC_PATH}/hash.c
	${LWAN_INC_PATH}/int-to-str.c
	${LWAN_INC_PATH}/list.c
	${LWAN_INC_PATH}/lwan.c
	${LWAN_INC_PATH}/lwan-cache.c
	${LWAN_INC_PATH}/lwan-config.c
	${LWAN_INC_PATH}/lwan-coro.c
	${LWAN_INC_PATH}/lwan-http-authorize.c
	${LWAN_INC_PATH}/lwan-io-wrappers.c
	${LWAN_INC_PATH}/lwan-job.c
	${LWAN_INC_PATH}/lwan-redirect.c
	${LWAN_INC_PATH}/lwan-request.c
	${LWAN_INC_PATH}/lwan-response.c
	${LWAN_INC_PATH}/lwan-serve-files.c
	${LWAN_INC_PATH}/lwan-socket.c
	${LWAN_INC_PATH}/lwan-status.c
	${LWAN_INC_PATH}/lwan-tables.c
	${LWAN_INC_PATH}/lwan-template.c
	${LWAN_INC_PATH}/lwan-thread.c
	${LWAN_INC_PATH}/lwan-trie.c
	${LWAN_INC_PATH}/realpathat.c
	${LWAN_INC_PATH}/sd-daemon.c
	${LWAN_INC_PATH}/strbuf.c
	${LWAN_INC_PATH}/murmur3.c
)
set(SOURCE_FILES ${LWAN_SOURCES} main.cpp application.h)

include(FindPkgConfig)

include(CheckFunctionExists)
set(CMAKE_EXTRA_INCLUDE_FILES time.h)
check_function_exists(clock_gettime HAS_CLOCK_GETTIME)
if (HAS_CLOCK_GETTIME)
	message(STATUS "libc has clock_gettime(). Good.")
else ()
	list(APPEND ADDITIONAL_LIBRARIES rt)
	message(STATUS "No clock_gettime() in libc. Linking with -lrt.")
endif ()

set(C_FLAGS_REL "-mtune=native")

include(CheckCCompilerFlag)
if (${CMAKE_BUILD_TYPE} MATCHES "Rel")
	check_c_compiler_flag(-flto HAS_LTO)
	if (HAS_LTO)
		set(C_FLAGS_REL "${C_FLAGS_REL} -flto")

		check_c_compiler_flag(-ffat-lto-objects HAS_FAT_LTO_OBJECTS)
		if (HAS_FAT_LTO_OBJECTS)
			set(C_FLAGS_REL "${C_FLAGS_REL} -ffat-lto-objects")
		endif ()
	endif ()

	check_c_compiler_flag(-mcrc32 HAS_CRC32)
	if (HAS_CRC32)
		add_definitions("-DUSE_HARDWARE_CRC32=1")
		set(C_FLAGS_REL "${C_FLAGS_REL} -mcrc32")
	endif ()
else ()
	check_c_compiler_flag(-fsanitize=undefined HAS_UNDEFINED_SANITIZER)
	if (HAS_UNDEFINED_SANITIZER)
		set(CMAKE_C_FLAGS_DEBUG "${CMAKE_C_FLAGS_DEBUG} -fsanitize=undefined")
	endif ()
endif ()


find_path(VALGRIND_INCLUDE_DIR valgrind.h /usr/include /usr/include/valgrind /usr/local/include /usr/local/include/valgrind)
if (VALGRIND_INCLUDE_DIR)
	message(STATUS "Building with Valgrind support")
	add_definitions("-DUSE_VALGRIND=1")
else ()
	message(STATUS "Valgrind headers not found -- disabling valgrind support")
endif()

find_library(TCMALLOC_LIBRARY NAMES tcmalloc_minimal tcmalloc)
if (TCMALLOC_LIBRARY)
	message(STATUS "tcmalloc found: ${TCMALLOC_LIBRARY}")
	list(APPEND ADDITIONAL_LIBRARIES ${TCMALLOC_LIBRARY})
else ()
	find_library(JEMALLOC_LIBRARY NAMES jemalloc)
	if (JEMALLOC_LIBRARY)
		message(STATUS "jemalloc found: ${JEMALLOC_LIBRARY}")
		list(APPEND ADDITIONAL_LIBRARIES ${JEMALLOC_LIBRARY})
	else ()
		message(STATUS "jemalloc and tcmalloc were not found, using system malloc")
	endif()
endif()

set(CMAKE_C_FLAGS_RELEASE "${CMAKE_C_FLAGS_RELEASE} ${C_FLAGS_REL}")
set(CMAKE_C_FLAGS_RELWITHDEBINFO "${CMAKE_C_FLAGS_RELWITHDEBINFO} ${C_FLAGS_REL}")
set(CMAKE_C_FLAGS_MINSIZEREL "${CMAKE_C_FLAGS_MINSIZEREL} ${C_FLAGS_REL}")
add_definitions("-D_FILE_OFFSET_BITS=64")

find_package(ZLIB REQUIRED)

add_executable(wfpp ${SOURCE_FILES})

add_custom_command(
	OUTPUT ${CMAKE_BINARY_DIR}/mime-types.h
	COMMAND ${PYTHON_EXECUTABLE} ${LWAN_PATH}/tools/generate-mime-types-table.py ${LWAN_PATH}/tools/mime.types > ${CMAKE_BINARY_DIR}/mime-types.h
	DEPENDS ${LWAN_PATH}/tools/mime.types ${LWAN_PATH}/tools/generate-mime-types-table.py
)
add_custom_target(generate_mime_types_table DEPENDS ${CMAKE_BINARY_DIR}/mime-types.h)
add_dependencies(wfpp generate_mime_types_table)
include_directories(${CMAKE_BINARY_DIR})

include_directories(/usr/include ${LWAN_INC_PATH} ${LWAN_PATH}/tools)
link_directories(${LWAN_BUILD_PATH}/common)

target_link_libraries(wfpp pthread ${ZLIB_LIBRARIES} ${ADDITIONAL_LIBRARIES} ${LUA_LIBRARIES})

