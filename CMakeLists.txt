cmake_minimum_required(VERSION 2.8.0 FATAL_ERROR)

project(cppio)

set(CMAKE_INCLUDE_CURRENT_DIR ON)

set(WORKSPACE_ROOT ${CMAKE_CURRENT_SOURCE_DIR}/..)

find_package(Threads)
if(WIN32)
else(WIN32)

set (Boost_USE_STATIC_LIBS OFF)
set (Boost_USE_MULTITHREADED ON)
set(Python_ADDITIONAL_VERSIONS 2.7)
FIND_PACKAGE(PythonInterp)
FIND_PACKAGE(PythonLibs)

find_package(Boost COMPONENTS system REQUIRED python)
include_directories(${Boost_INCLUDE_DIRS})
endif(WIN32)

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
	)

if(WIN32)
	list(APPEND cppio-sources
		src/win32/win_socket.cpp
		src/win32/pipes.cpp
		src/win32/createlinemanager.cpp
		)
elseif(UNIX)
	list(APPEND cppio-sources
		src/common/select_poller.cpp
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

if(Boost_PYTHON_FOUND)

include_directories(${PYTHON_INCLUDE_DIRS})

message("Python bindings enabled")
add_library(pycppio SHARED ${libcppio-sources} python/python_bindings.cpp)
target_link_libraries(pycppio ${Boost_LIBRARIES} ${PYTHON_LIBRARIES} ${CMAKE_THREAD_LIBS_INIT} cppio)
if(WIN32)
	target_link_libraries(pycppio -lwsock32 -lws2_32)
endif(WIN32)
set_target_properties(pycppio PROPERTIES PREFIX "")

else(Boost_PYTHON_FOUND)
message("Python bindings are NOT enabled")
endif(Boost_PYTHON_FOUND)
