cmake_minimum_required(VERSION 2.8.4)
project(ALSA_example)

set(EXEC_NAME ALSA_example)

find_package(ALSA)

include_directories(${ALSA_INCLUDE_DIRS})

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11")

set(SOURCE_FILES main.cpp wav_functions.cpp)
add_executable(${EXEC_NAME} ${SOURCE_FILES})

target_link_libraries(${EXEC_NAME} ${ALSA_LIBRARIES})