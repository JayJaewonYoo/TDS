cmake_minimum_required(VERSION 2.8.9)
project(TDS)


include_directories(include)
INCLUDE_DIRECTORIES(/usr/local/include)

file(GLOB SOURCES "src/*.cpp")

add_executable(TDS ${SOURCES})
TARGET_LINK_LIBRARIES(TDS /usr/local/lib/libbdd.so.0.0.0)


