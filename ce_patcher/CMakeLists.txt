cmake_minimum_required(VERSION 3.1)

project(ce_patcher)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(EXECUTABLE_OUTPUT_PATH ${PROJECT_BINARY_DIR}/bin)

aux_source_directory(${PROJECT_SOURCE_DIR}/src SOURCE)

add_library(${PROJECT_NAME} SHARED main.cpp ${SOURCE}) 

target_link_libraries(${PROJECT_NAME} library imgui)