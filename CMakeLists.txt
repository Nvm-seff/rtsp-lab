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
    src/probe_helpers.cpp
    src/ffmpeg_raii.cpp
)

# Link the FFmpeg libraries and pthread
target_link_libraries(rtsp-lab 
    ${AV_LIBRARIES} 
    pthread
)

include(FetchContent)
FetchContent_Declare(
  googletest
  URL https://github.com/google/googletest/archive/refs/heads/main.zip
)
set(gtest_force_shared_crt ON CACHE BOOL "" FORCE)
FetchContent_MakeAvailable(googletest)

# Define the test executable
enable_testing()
add_executable(unit_tests tests/unit_tests.cpp src/probe_helpers.cpp)
target_link_libraries(unit_tests GTest::gtest_main)
add_test(NAME AllUnitTests COMMAND unit_tests)
# Add the Integration Test script to CTest
add_test(NAME IntegrationTests 
         COMMAND bash ${CMAKE_SOURCE_DIR}/tests/integration_test.sh)