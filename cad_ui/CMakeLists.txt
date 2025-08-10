set(TARGET_NAME cad_ui)

# 头文件
set(HEADERS
    include/cad_ui/MainWindow.h
    include/cad_ui/QtOccView.h
    include/cad_ui/DocumentTree.h
    include/cad_ui/PropertyPanel.h
    include/cad_ui/ToolBar.h
    include/cad_ui/StatusBar.h
    include/cad_ui/ThemeManager.h
    include/cad_ui/ExportDialog.h
    include/cad_ui/AboutDialog.h
    include/cad_ui/CreatePrimitiveDialog.h
    include/cad_ui/BooleanOperationDialog.h
    include/cad_ui/FilletChamferDialog.h
    include/cad_ui/TransformOperationDialog.h
    include/cad_ui/SketchMode.h
    include/cad_ui/FaceSelectionDialog.h
    include/cad_ui/CreateHoleDialog.h
    
)

# 源文件
set(SOURCES
    src/MainWindow.cpp
    src/QtOccView.cpp
    src/DocumentTree.cpp
    src/PropertyPanel.cpp
    src/ToolBar.cpp
    src/StatusBar.cpp
    src/ThemeManager.cpp
    src/ExportDialog.cpp
    src/AboutDialog.cpp
    src/CreatePrimitiveDialog.cpp
    src/BooleanOperationDialog.cpp
    src/FilletChamferDialog.cpp
    src/TransformOperationDialog.cpp
    src/SketchMode.cpp
    src/FaceSelectionDialog.cpp
    src/CreateHoleDialog.cpp
)

# 资源文件
set(RESOURCES
    resources/resources.qrc
)

# 创建静态库
add_library(${TARGET_NAME} STATIC ${HEADERS} ${SOURCES} ${RESOURCES} "include/cad_ui/CreateHoleDialog.h" "src/CreateHoleDialog.cpp")

# 包含目录
target_include_directories(${TARGET_NAME} PUBLIC 
    ${CMAKE_CURRENT_SOURCE_DIR}/include
    ${OpenCASCADE_INCLUDE_DIR}
)

# 链接库
target_link_libraries(${TARGET_NAME} 
    cad_core
    cad_sketch
    cad_feature
    ${OpenCASCADE_LIBRARIES}
    Qt5::Core
    Qt5::Widgets
    Qt5::Gui
)