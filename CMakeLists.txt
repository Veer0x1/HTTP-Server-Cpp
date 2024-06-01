cmake_minimum_required(VERSION 3.13)
project(http-server-starter-cpp)
set(CMAKE_CXX_STANDARD 20) # Enable the C++20 standard

set(SOURCE_FILES src/server.cpp src/request_handler.cpp)

find_package(ZLIB REQUIRED)

add_executable(server ${SOURCE_FILES})

target_link_libraries(server pthread ${ZLIB_LIBRARIES})
