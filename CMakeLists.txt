# CMakeList.txt

cmake_minimum_required(VERSION 2.6)

project(chat-server)

SET(CMAKE_CXX_FLAGS_DEBUG "-g -ggdb -Wall -Wl,--hash-style=sysv")

include_directories("${PROJECT_SOURCE_DIR}/mysql++/include" "/usr/local/services/include") 
link_directories("${PROJECT_SOURCE_DIR}/mysql++/lib" "/usr/local/services/lib") 

add_subdirectory (common)
add_subdirectory (event)
add_subdirectory (net)
add_subdirectory (json)
add_subdirectory (logicserver)
add_subdirectory (dataserver)


add_executable (logicserverd logicserver/logic_server.cpp)
add_executable (dataserverd dataserver/data_server.cpp)

target_link_libraries (logicserverd logicserver common event net json mysqlpp log4cxx)
target_link_libraries (dataserverd dataserver net event json mysqlpp log4cxx
    common config++)
