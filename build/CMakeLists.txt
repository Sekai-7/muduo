cmake_minimum_required(VERSION 3.16.3)

project(muduo)

aux_source_directory(../src SRC)
include_directories(../include)
link_libraries(pthread)

add_executable(muduo ${SRC})