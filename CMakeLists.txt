# CMakeList.txt

cmake_minimum_required(VERSION 2.6)

project(chat-server)

SET(CMAKE_CXX_FLAGS_DEBUG "-g -ggdb -Wall -Wl,--hash-style=sysv")

include_directories("${PROJECT_SOURCE_DIR}/mysql++/include")
link_directories("${PROJECT_SOURCE_DIR}/mysql++/lib")

add_subdirectory (common)
add_subdirectory (event)
add_subdirectory (net)
add_subdirectory (logicserver)


add_executable (logicserverd logicserver/logic_server.cpp)

target_link_libraries (logicserverd logicserver common event net mysqlpp)
