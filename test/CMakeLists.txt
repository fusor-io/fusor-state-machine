cmake_minimum_required(VERSION 2.6)

#Set Arduino library root directory
set(ARDUINO_LIB_ROOT "../..")
message("IMPORTANT NOTE: update your ARDUINO_LIB_ROOT in a CMakeLists.txt file to meet your environment")
message(STATUS "ARDUINO_LIB_ROOT=${ARDUINO_LIB_ROOT}")

message(STATUS "Compiler: ${CMAKE_CXX_COMPILER_ID} ${CMAKE_CXX_COMPILER_VERSION}")

find_package(GTest REQUIRED)
message(STATUS "GTest: ${GTEST_INCLUDE_DIRS} ${GTEST_LIBRARIES}")

include_directories(${GTEST_INCLUDE_DIRS})
include_directories(${ARDUINO_LIB_ROOT}/ArduinoJson/src)
include_directories(../src/keycompare)
include_directories(../src/timers)
include_directories(../src/store)
include_directories(../src/compute)
include_directories(../src/plugin)

#Link runTests with what we want to test and the GTest and pthread library
add_executable(executeTests 
    test.cpp
    ../src/keycompare/keycompare.cpp
    ../src/timers/timers.cpp
    ../src/store/store.cpp
    ../src/compute/compute.cpp
    ../src/plugin/plugin.cpp
    ../src/StateMachineDebug.cpp
)
target_link_libraries(executeTests ${GTEST_LIBRARIES} pthread)

