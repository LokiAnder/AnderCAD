set(TARGET_NAME cad_core)

# 头文件
set(HEADERS
    include/cad_core/Shape.h
    include/cad_core/Point.h
    include/cad_core/ShapeFactory.h
    include/cad_core/ICommand.h
    include/cad_core/CommandManager.h
    include/cad_core/CreateBoxCommand.h
    include/cad_core/CreateCylinderCommand.h
    include/cad_core/CreateSphereCommand.h
    include/cad_core/OCAFDocument.h
    include/cad_core/OCAFManager.h
    include/cad_core/SelectionManager.h
    include/cad_core/BooleanOperations.h
    include/cad_core/FilletChamferOperations.h
)

# 源文件
set(SOURCES
    src/Shape.cpp
    src/Point.cpp
    src/ShapeFactory.cpp
    src/CommandManager.cpp
    src/CreateBoxCommand.cpp
    src/CreateCylinderCommand.cpp
    src/CreateSphereCommand.cpp
    src/OCAFDocument.cpp
    src/OCAFManager.cpp
    src/SelectionManager.cpp
    src/BooleanOperations.cpp
    src/FilletChamferOperations.cpp
)

# 创建静态库
add_library(${TARGET_NAME} STATIC ${HEADERS} ${SOURCES})

# 包含目录
target_include_directories(${TARGET_NAME} PUBLIC 
    ${CMAKE_CURRENT_SOURCE_DIR}/include
    ${OpenCASCADE_INCLUDE_DIR}
)

# 链接 OpenCASCADE 库
target_link_libraries(${TARGET_NAME} 
    ${OpenCASCADE_LIBRARIES}
    # 额外的 OCAF 库
    TKCAF
    TKCDF
    TKDCAF
    TKLCAF
    TKStd
    TKStdL
    TKVCAF
    TKXCAF
    TKBinXCAF
    TKXmlXCAF
    TKBin
    TKXml
    TKBinL
    TKXmlL
)