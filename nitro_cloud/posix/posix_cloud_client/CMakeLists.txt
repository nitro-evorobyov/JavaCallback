cmake_minimum_required(VERSION 3.5)
project(posix_cloud_client)

set(CMAKE_RUNTIME_OUTPUT_DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}/../bin" )
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11 -pthread")

set(SOURCE_FILES main.cpp)
add_executable(posix_cloud_client ${SOURCE_FILES})

set(BOOST_ROOT "/mnt/hgfs/nitro_cloud/boost")

find_package(Boost COMPONENTS system REQUIRED)

if (Boost_FOUND)
    message(STATUS "Boost found ${Boost_INCLUDE_DIRS}, ${Boost_LIBRARY_DIRS}")
    target_include_directories(posix_cloud_client PRIVATE ${Boost_INCLUDE_DIRS})
    target_link_libraries(posix_cloud_client ${Boost_SYSTEM_LIBRARY})
endif ()

include_directories("../../include/")



