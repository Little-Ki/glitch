cmake_minimum_required(VERSION 3.1)

project(module)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(EXECUTABLE_OUTPUT_PATH ${PROJECT_BINARY_DIR}/bin)
set(MODULE_BACKEND "D3D11" CACHE STRING "Renderer backend")

set_property(CACHE MODULE_BACKEND PROPERTY STRINGS D3D12 D3D11 D3D10 D3D9)

include_directories(
    ${PROJECT_SOURCE_DIR}/include/sdk
    ${PROJECT_SOURCE_DIR}/../library/include
    ${PROJECT_SOURCE_DIR}/../third_party/imgui/source/
    ${PROJECT_SOURCE_DIR}/../third_party/imgui/source/backends
    ${PROJECT_SOURCE_DIR}/include
)

aux_source_directory(${PROJECT_SOURCE_DIR}/src SOURCE)
aux_source_directory(${PROJECT_SOURCE_DIR}/src/sdk SOURCE_SDK)

list(APPEND SOURCE_MENU ${PROJECT_SOURCE_DIR}/../third_party/imgui/source/backends/imgui_impl_win32.cpp)
list(APPEND SOURCE_MENU ${PROJECT_SOURCE_DIR}/src/ct_menu.cpp)

if(${MODULE_BACKEND} STREQUAL "D3D12")
    list(APPEND SOURCE_MENU ${PROJECT_SOURCE_DIR}/../third_party/imgui/source/backends/imgui_impl_dx12.cpp)
    list(APPEND SOURCE_MENU ${PROJECT_SOURCE_DIR}/src/menu_impl/ct_menu_dx12.cpp)
elseif(${MODULE_BACKEND} STREQUAL "D3D11")
    list(APPEND SOURCE_MENU ${PROJECT_SOURCE_DIR}/../third_party/imgui/source/backends/imgui_impl_dx11.cpp)
    list(APPEND SOURCE_MENU ${PROJECT_SOURCE_DIR}/src/menu_impl/ct_menu_dx11.cpp)
elseif(${MODULE_BACKEND} STREQUAL "D3D10")
    list(APPEND SOURCE_MENU ${PROJECT_SOURCE_DIR}/../third_party/imgui/source/backends/imgui_impl_dx10.cpp)
    list(APPEND SOURCE_MENU ${PROJECT_SOURCE_DIR}/src/menu_impl/ct_menu_dx10.cpp)
elseif(${MODULE_BACKEND} STREQUAL "D3D9")
    list(APPEND SOURCE_MENU ${PROJECT_SOURCE_DIR}/../third_party/imgui/source/backends/imgui_impl_dx9.cpp)
    list(APPEND SOURCE_MENU ${PROJECT_SOURCE_DIR}/src/menu_impl/ct_menu_dx9.cpp)
endif()

source_group("Source Files\\sdk" FILES ${SOURCE_SDK})
source_group("Source Files\\menu" FILES ${SOURCE_MENU})

add_library(${PROJECT_NAME} SHARED main.cpp ${SOURCE} ${SOURCE_MENU} ${SOURCE_SDK}) 

target_link_libraries(${PROJECT_NAME} library imgui)