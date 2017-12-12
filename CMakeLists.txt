cmake_minimum_required(VERSION 2.8) 
project(CAB_DAEMON)

set(CMAKE_CXX_COMPILER /usr/bin/g++)
set(CMAKE_CXX_FLAGS "-std=c++11 -g -Wall")

find_package(Boost 1.54.0 REQUIRED COMPONENTS system thread filesystem iostreams log log_setup gzip)
if(Boost_FOUND)
	message(STATUS "BOOST_LIBRARIES: " ${Boost_LIBRARIES})
endif()

add_definitions(-DBOOST_LOG_DYN_LINK)

set(CAB Address.hpp Rule.hpp RuleList.cpp Bucket.cpp BucketTree.cpp)
add_library(cab ${CAB})

add_executable(UnitTest UnitTest.cpp TraceGen.cpp parsePcap.cpp Separate.cpp OFswitch.cpp OFswitch_CB.cpp)

if(UNIX)
    target_link_libraries(
        UnitTest
        cab
        ${Boost_LIBRARIES}
        pthread
		z
		pcap
        )
elseif(WIN32)
elseif(APPLE)
endif(UNIX)

message(STATUS "c++ flag : " ${CMAKE_CXX_FLAGS}) 
message(STATUS "this is source dir " ${PROJECT_SOURCE_DIR})
message(STATUS "this is binary dir " ${PROJECT_BINARY_DIR})
