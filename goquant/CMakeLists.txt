cmake_minimum_required(VERSION 3.10)
project(goquant)

set(CMAKE_CXX_STANDARD 17)

set(CMAKE_PREFIX_PATH "/usr/local/lib/cmake/jsoncpp/")
find_package(JsonCpp REQUIRED)


find_package(CURL REQUIRED)
find_package(JsonCpp REQUIRED)

add_executable(DeribitSetup main.cpp)

target_link_libraries(DeribitSetup CURL::libcurl jsoncpp_lib)

