cmake_minimum_required(VERSION 3.28)
project(rtcdatachannel)

set(CMAKE_CXX_STANDARD 20)
include_directories(${CMAKE_SOURCE_DIR}/include)

find_package(LibDataChannel CONFIG REQUIRED)
find_package(nlohmann_json CONFIG REQUIRED)

add_library(common_library STATIC src/common.cpp)
target_include_directories(common_library PUBLIC ${PROJECT_SOURCE_DIR}/include)
target_link_libraries(common_library PRIVATE LibDataChannel::LibDataChannelStatic nlohmann_json::nlohmann_json)


add_executable(main main.cpp)
add_executable(offerer offerer.cpp)

target_include_directories(main PRIVATE ${PROJECT_SOURCE_DIR}/include)
target_include_directories(offerer PRIVATE ${PROJECT_SOURCE_DIR}/include)


target_link_libraries(main PRIVATE LibDataChannel::LibDataChannelStatic nlohmann_json::nlohmann_json common_library)
target_link_libraries(offerer PRIVATE LibDataChannel::LibDataChannelStatic nlohmann_json::nlohmann_json common_library)