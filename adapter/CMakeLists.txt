cmake_minimum_required(VERSION 3.16)
project(lidar_adapter CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

find_package(vsomeip3 REQUIRED)
find_package(Threads  REQUIRED)

add_executable(lidar_adapter
    main.cpp
    UdpReceiver.cpp
    LidarParser.cpp
)

target_include_directories(lidar_adapter PRIVATE ${CMAKE_CURRENT_SOURCE_DIR})
target_link_libraries(lidar_adapter PRIVATE vsomeip3 Threads::Threads)

# Copy vsomeip config next to the binary for convenience
configure_file(
    ${CMAKE_SOURCE_DIR}/vsomeip_adapter.json
    ${CMAKE_BINARY_DIR}/vsomeip_adapter.json
    COPYONLY
)