
cmake_minimum_required(VERSION 2.8)

project(CoreframeCache)

set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11")

include_directories(.)
include_directories(..)
include_directories($ENV{GTEST_INC_PATH})
include_directories($ENV{GMOCK_INC_PATH})
include_directories($ENV{BOOST_INC_PATH})

set(HEADER_FILES
        Cache.hpp
        CacheInternal.hpp
        )

set(SOURCE_FILES
        Cache.cpp
        )

set(TEST_SOURCE_FILES
        tests/CacheTest.cpp
        )

set(LIBRARIES
        CoreframeLog
        CoreframeSharedMemory
        CoreframeMonitor
        CoreframeConfigurator
        CoreframeCommunication
        boost_filesystem
        boost_serialization
        boost_system
        boost_thread
        boost_program_options
        gmock
        gtest
        pthread
        dl
        )

link_directories($ENV{GTEST_LIB_PATH})
link_directories($ENV{GMOCK_LIB_PATH})
link_directories($ENV{BOOST_LIB_PATH})

add_library(CoreframeCache STATIC ${SOURCE_FILES})
add_library(CoreframeCacheTest_objlib OBJECT ${TEST_SOURCE_FILES})

#[[ excutable binary for coreframe unit tests. ]]
add_executable(CoreframeCacheUnitTests $<TARGET_OBJECTS:CoreframeCacheTest_objlib>)
target_link_libraries(CoreframeCacheUnitTests CoreframeCache ${LIBRARIES} gtest_main)

#add_dependencies(CoreframeCacheUnitTests exampleChild)