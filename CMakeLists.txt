cmake_minimum_required(VERSION 3.10)
project(rtsp_lab)

set(CMAKE_CXX_STANDARD 17)

find_package(PkgConfig REQUIRED)
pkg_check_modules(AV REQUIRED libavformat libavcodec libavutil)

include_directories(include ${AV_INCLUDE_DIRS})

add_executable(rtsp-lab 
    src/main.cpp 
    src/probe.cpp 
    src/ffmpeg_raii.cpp
)

target_link_libraries(rtsp-lab ${AV_LIBRARIES} pthread)