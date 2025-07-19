cmake_minimum_required(VERSION 3.1)

project(library)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(LIBRARY_OUTPUT_PATH ${PROJECT_BINARY_DIR}/libs)

include_directories(${PROJECT_SOURCE_DIR}/include)
aux_source_directory(${PROJECT_SOURCE_DIR}/src SOURCE)

add_library(${PROJECT_NAME} STATIC ${SOURCE})

