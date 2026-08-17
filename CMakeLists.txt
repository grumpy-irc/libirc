cmake_minimum_required(VERSION 3.0)
project(libirc)

option(QT5_BUILD "Build with Qt5" true)
option(QT6_BUILD "Build with Qt6" false)
option(LIBIRC_UNIT_TESTS "Build libirc unit tests" false)

if(QT5_BUILD AND QT6_BUILD)
    message(FATAL_ERROR "Select only one Qt version: QT5_BUILD or QT6_BUILD")
elseif(NOT QT5_BUILD AND NOT QT6_BUILD)
    message(FATAL_ERROR "Enable QT5_BUILD or QT6_BUILD")
endif()

if(QT6_BUILD)
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++17")
else()
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11")
endif()

add_subdirectory("libirc")
add_subdirectory("libircclient")

if (LIBIRC_UNIT_TESTS)
    enable_testing()
    add_subdirectory("tests")
endif()
