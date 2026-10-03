cmake_minimum_required(VERSION 3.21)
project(EasyWorkspace VERSION 0.1.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)
set(CMAKE_AUTOMOC ON)

if(MSVC)
    # Добавлен обязательный для Qt6 флаг /Zc:__cplusplus
    add_compile_options(/utf-8 /W4 /Zc:__cplusplus)
else()
    add_compile_options(-Wall -Wextra)
endif()

find_package(Qt6 6.2 REQUIRED COMPONENTS Core Gui Widgets PrintSupport)

add_subdirectory(core)
add_subdirectory(word)
add_subdirectory(excel)
add_subdirectory(powerpoint)
