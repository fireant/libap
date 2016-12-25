cmake_minimum_required(VERSION 3.5)
project(asiopiwik)

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11")

include_directories(../libap)

set(SOURCE_FILES
    ../libap/AsioPiwik.cpp
    main.cpp)

add_executable(asiopiwik ${SOURCE_FILES})

target_link_libraries(asiopiwik ssl crypto pthread)
