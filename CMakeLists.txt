cmake_minimum_required(VERSION 3.0.0 FATAL_ERROR)

project(cppio)

set(CMAKE_INCLUDE_CURRENT_DIR ON)

set(WORKSPACE_ROOT ${CMAKE_CURRENT_SOURCE_DIR}/..)

find_package(Threads)

SET(CMAKE_CXX_FLAGS  "-Wall -fno-strict-aliasing -fexceptions -g --std=gnu++11 -mtune=generic ${PLATFORM_CXX_FLAGS} -fno-omit-frame-pointer")

include_directories(${CMAKE_CURRENT_SOURCE_DIR}/include)
include_directories(${CMAKE_CURRENT_SOURCE_DIR}/src)
include_directories(${CMAKE_CURRENT_SOURCE_DIR}/3rdparty)
include_directories(${CMAKE_CURRENT_SOURCE_DIR}/3rdparty/catch)
include_directories(${CMAKE_CURRENT_SOURCE_DIR}/3rdparty/mingw-std-threads)

add_definitions(-DCPPIO_BLOCKING_INPROC=1)

set(cppio-sources
		src/iolinemanager.cpp
		src/message.cpp

		src/common/inproc.cpp
		src/common/select_poller.cpp
	)

if(WIN32)
	list(APPEND cppio-sources
		src/win32/win_socket.cpp
		src/win32/pipes.cpp
		src/win32/createlinemanager.cpp
		)
elseif(UNIX)
	list(APPEND cppio-sources
		src/posix/createlinemanager.cpp
		src/posix/io_socket.cpp)
endif(WIN32)

add_library(cppio SHARED ${cppio-sources})
target_link_libraries(cppio ${CMAKE_THREAD_LIBS_INIT})

if(WIN32)
target_link_libraries(cppio -lws2_32)
endif(WIN32)

include_directories(
	${CMAKE_CURRENT_SOURCE_DIR}/3rdparty
	${CMAKE_CURRENT_SOURCE_DIR}/3rdparty/catch
	${CMAKE_CURRENT_BINARY_DIR}
	)

set(test-sources
		tests/inproc_test.cpp
		tests/message_test.cpp
		tests/inproc_integration_test.cpp
		tests/messageprotocol_test.cpp
	)

if(UNIX)
	list(APPEND test-sources
		tests/unixsocket_test.cpp
		)
endif(UNIX)

if(WIN32)
	list(APPEND test-sources
		tests/winsocket_test.cpp
		tests/pipes_test.cpp
		)
endif(WIN32)

add_executable(libcppio-tests tests/tests.cpp
	${test-sources})

target_compile_definitions(libcppio-tests PRIVATE)

target_link_libraries(libcppio-tests cppio ${CMAKE_THREAD_LIBS_INIT})

set_target_properties(libcppio-tests PROPERTIES COMPILE_FLAGS "-O0 -g -fprofile-arcs -ftest-coverage")
set_target_properties(libcppio-tests PROPERTIES LINK_FLAGS "-fprofile-arcs -lgcov")

if(WIN32)
target_link_libraries(libcppio-tests -lws2_32)
endif(WIN32)

