cmake_minimum_required(VERSION 3.1)

project(cheat-t)

add_subdirectory(module)
add_subdirectory(library)
add_subdirectory(${PROJECT_SOURCE_DIR}/third_party/imgui EXCLUDE_FROM_ALL)