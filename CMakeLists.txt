project(How-to-mock-free-function)
cmake_minimum_required(VERSION 2.8)

#FIND_PACKAGE( Boost REQUIRED )
#INCLUDE_DIRECTORIES( ${Boost_INCLUDE_DIR} )

add_definitions(-std=c++11 -Wall -pedantic)

INCLUDE_DIRECTORIES( ${PROJECT_SOURCE_DIR}/3rd-party/gmock-1.7.0 )

aux_source_directory(. SRC_LIST)
add_executable(${PROJECT_NAME}
               ${SRC_LIST}
               ${PROJECT_SOURCE_DIR}/3rd-party/gmock-1.7.0/gmock_main.cc
               ${PROJECT_SOURCE_DIR}/3rd-party/gmock-1.7.0/gmock-gtest-all.cc
              )

