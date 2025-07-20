cmake_minimum_required(VERSION 3.16)
project(Ander_CAD VERSION 1.0.0)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# 查找 OpenCASCADE 7.8
find_package(OpenCASCADE REQUIRED
    PATHS "C:/Program Files/OCCT"
    NO_DEFAULT_PATH
)

# 查找 Qt5
find_package(Qt5 REQUIRED COMPONENTS Core Widgets Gui)

# 设置 Qt5 自动MOC
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)
set(CMAKE_AUTOUIC ON)

# 包含目录
include_directories(${OpenCASCADE_INCLUDE_DIR})

# 添加子目录
add_subdirectory(cad_core)
add_subdirectory(cad_sketch)
add_subdirectory(cad_feature)
add_subdirectory(cad_ui)
add_subdirectory(cad_app)

# 为 Visual Studio 设置启动项目
if(MSVC)
    set_property(DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR} PROPERTY VS_STARTUP_PROJECT cad_app)
endif()