cmake_minimum_required(VERSION 3.10)
project(rtsp_lab)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Find FFmpeg using PkgConfig
find_package(PkgConfig REQUIRED)
pkg_check_modules(AV REQUIRED libavformat libavcodec libavutil)

# Tell CMake where your header files are
include_directories(include ${AV_INCLUDE_DIRS})

# Define the output executable name as "rtsp-lab"
add_executable(rtsp-lab 
    src/main.cpp 
    src/probe.cpp 
    src/ffmpeg_raii.cpp
)

# Link the FFmpeg libraries and pthread
target_link_libraries(rtsp-lab 
    ${AV_LIBRARIES} 
    pthread
)