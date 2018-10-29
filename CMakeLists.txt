cmake_minimum_required(VERSION 2.8.9)
project(TDS)


include_directories(include)
link_directories("/usr/local/lib")

file(GLOB SOURCES "src/*.cpp")

add_executable(TDS ${SOURCES})

