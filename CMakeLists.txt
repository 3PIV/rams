cmake_minimum_required(VERSION 3.1)

project(rams VERSION 0.1
                DESCRIPTION "Resource Acquisition and Management System"
                LANGUAGES CXX)

add_executable(rams src/main.cpp include/rams.hpp)

set_target_properties(rams PROPERTIES CXX_STANDARD 98)

target_include_directories(rams PUBLIC include)