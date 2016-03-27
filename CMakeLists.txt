project(SDRPlayPorts)

link_directories(/usr/local/lib)

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11")

add_executable(play_tcp play_tcp.c)
add_executable(play_sdr play_sdr.c)

target_link_libraries (play_sdr pthread m mirsdrapi-rsp)
target_link_libraries (play_tcp pthread m mirsdrapi-rsp)


