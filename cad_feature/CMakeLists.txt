set(TARGET_NAME cad_feature)

# 头文件
set(HEADERS
    include/cad_feature/Feature.h
    include/cad_feature/ExtrudeFeature.h
    include/cad_feature/RevolveFeature.h
    include/cad_feature/SweepFeature.h
    include/cad_feature/LoftFeature.h
    include/cad_feature/FeatureManager.h
    include/cad_feature/ParameterPanel.h
    include/cad_feature/LivePreview.h
)

# 源文件
set(SOURCES
    src/Feature.cpp
    src/ExtrudeFeature.cpp
    src/RevolveFeature.cpp
    src/SweepFeature.cpp
    src/LoftFeature.cpp
    src/FeatureManager.cpp
    src/ParameterPanel.cpp
    src/LivePreview.cpp
)

# 创建静态库
add_library(${TARGET_NAME} STATIC ${HEADERS} ${SOURCES})

# 包含目录
target_include_directories(${TARGET_NAME} PUBLIC 
    ${CMAKE_CURRENT_SOURCE_DIR}/include
    ${OpenCASCADE_INCLUDE_DIR}
)

# 链接库
target_link_libraries(${TARGET_NAME} 
    cad_core
    cad_sketch
    ${OpenCASCADE_LIBRARIES}
    Qt5::Core
    Qt5::Widgets
    Qt5::Gui
)