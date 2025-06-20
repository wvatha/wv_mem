cmake_minimum_required(VERSION 3.16)

project(test_wv_mem)

file(GLOB_RECURSE TEST_SOURCES "*.c")

add_executable(test_wv_mem ${TEST_SOURCES})