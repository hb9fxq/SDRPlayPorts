cmake_minimum_required(VERSION 3.4)
project(SDRPlayPorts)

link_directories(/usr/local/lib)

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11")

#[[set(SOURCE_FILES
        play_tcp.c play_sdr.c)]]

add_executable(play_tcp play_tcp)
add_executable(play_sdr play_sdr.c)

target_link_libraries (play_sdr pthread m mirsdrapi-rsp)
target_link_libraries (play_tcp pthread m mirsdrapi-rsp)


