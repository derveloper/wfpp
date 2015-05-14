cmake_minimum_required(VERSION 3.2)
project(wfpp C CXX)

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall -Wextra -Wshadow -Wconversion -std=c++14")
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
set(SOURCE_FILES main.cpp application.h)

include(FindPkgConfig)
foreach (pc_file luajit lua lua lua51 lua5.1 lua-5.1)
	if (${pc_file} STREQUAL "luajit")
		pkg_check_modules(LUA luajit>=2.0 luajit<=2.0.9999)
	else ()
		pkg_check_modules(LUA ${pc_file}>=5.1.0 ${pc_file}<=5.1.999)
	endif ()
        if (LUA_FOUND)
		list(APPEND ADDITIONAL_LIBRARIES ${LUA_LIBRARIES})
		list(APPEND SOURCES lwan-lua.c)
		include_directories(${LUA_INCLUDE_DIRS})
		add_definitions(-DHAVE_LUA)
		break()
        endif()
endforeach ()
if (NOT LUA_FOUND)
	message(STATUS "Disabling Lua support")
else ()
	message(STATUS "Building with Lua support using ${LUA_LIBRARIES}")
endif ()

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

find_package(Boost 1.57.0 COMPONENTS filesystem system)

include_directories(${CMAKE_BINARY_DIR} ${Boost_INCLUDE_DIRS})

include_directories(/usr/include ${LWAN_INC_PATH} ${LWAN_PATH}/tools)
link_directories(${LWAN_BUILD_PATH}/common)

target_link_libraries(wfpp pthread dl ${Boost_LIBRARIES} ${LWAN_BUILD_PATH}/common/liblwan-common.a ${ZLIB_LIBRARIES} ${ADDITIONAL_LIBRARIES} ${LUA_LIBRARIES})

