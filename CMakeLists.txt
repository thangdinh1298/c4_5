cmake_minimum_required(VERSION 3.16)
project(C4_5)

set(CMAKE_CXX_STANDARD 14)

add_executable(C4_5 main.cpp include/rapid.h include/attribute.h include/helper.h include/tree_node.h include/stat.h test/helper_func_test.h)