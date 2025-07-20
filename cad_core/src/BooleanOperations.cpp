#include "cad_core/BooleanOperations.h"
#include <BRepAlgoAPI_Fuse.hxx>
#include <BRepAlgoAPI_Common.hxx>
#include <BRepAlgoAPI_Cut.hxx>
#include <BRepCheck_Analyzer.hxx>
#include <ShapeFix_Shape.hxx>
#include <BRepBuilderAPI_MakeShape.hxx>
#include <TopExp_Explorer.hxx>
#include <TopoDS.hxx>
#include <Standard_Failure.hxx>

namespace cad_core {

ShapePtr BooleanOperations::Union(const ShapePtr& shape1, const ShapePtr& shape2) {
    return PerformUnion(shape1, shape2);
}

ShapePtr BooleanOperations::Union(const std::vector<ShapePtr>& shapes) {
    if (shapes.empty()) return nullptr;
    if (shapes.size() == 1) return shapes[0];
    
    ShapePtr result = shapes[0];
    for (size_t i = 1; i < shapes.size(); i++) {
        result = Union(result, shapes[i]);
        if (!result) return nullptr;
    }
    
    return result;
}

ShapePtr BooleanOperations::Intersection(const ShapePtr& shape1, const ShapePtr& shape2) {
    return PerformIntersection(shape1, shape2);
}

ShapePtr BooleanOperations::Intersection(const std::vector<ShapePtr>& shapes) {
    if (shapes.empty()) return nullptr;
    if (shapes.size() == 1) return shapes[0];
    
    ShapePtr result = shapes[0];
    for (size_t i = 1; i < shapes.size(); i++) {
        result = Intersection(result, shapes[i]);
        if (!result) return nullptr;
    }
    
    return result;
}

ShapePtr BooleanOperations::Difference(const ShapePtr& shape1, const ShapePtr& shape2) {
    return PerformDifference(shape1, shape2);
}

ShapePtr BooleanOperations::BooleanOperation(const ShapePtr& shape1, const ShapePtr& shape2, BooleanType type) {
    switch (type) {
        case BooleanType::Union:
            return Union(shape1, shape2);
        case BooleanType::Intersection:
            return Intersection(shape1, shape2);
        case BooleanType::Difference:
            return Difference(shape1, shape2);
        default:
            return nullptr;
    }
}

ShapePtr BooleanOperations::BooleanOperation(const std::vector<ShapePtr>& shapes, BooleanType type) {
    switch (type) {
        case BooleanType::Union:
            return Union(shapes);
        case BooleanType::Intersection:
            return Intersection(shapes);
        case BooleanType::Difference:
            // 对于差集操作，我们只能处理两个形状
            if (shapes.size() == 2) {
                return Difference(shapes[0], shapes[1]);
            }
            return nullptr;
        default:
            return nullptr;
    }
}

bool BooleanOperations::IsValidShape(const ShapePtr& shape) {
    if (!shape || shape->GetOCCTShape().IsNull()) {
        return false;
    }
    
    BRepCheck_Analyzer analyzer(shape->GetOCCTShape());
    return analyzer.IsValid();
}

ShapePtr BooleanOperations::FixShape(const ShapePtr& shape) {
    if (!shape || shape->GetOCCTShape().IsNull()) {
        return nullptr;
    }
    
    try {
        Handle(ShapeFix_Shape) fixer = new ShapeFix_Shape();
        fixer->Init(shape->GetOCCTShape());
        fixer->Perform();
        
        TopoDS_Shape fixedShape = fixer->Shape();
        if (!fixedShape.IsNull()) {
            return std::make_shared<Shape>(fixedShape);
        }
    } catch (const Standard_Failure& e) {
        // 修复失败，返回原形状
    }
    
    return shape;
}

ShapePtr BooleanOperations::SimplifyShape(const ShapePtr& shape) {
    if (!shape || shape->GetOCCTShape().IsNull()) {
        return nullptr;
    }
    
    // 简化形状的实现可以在这里添加
    // 目前返回原形状
    return shape;
}

ShapePtr BooleanOperations::PerformUnion(const ShapePtr& shape1, const ShapePtr& shape2) {
    if (!ValidateInputs(shape1, shape2)) {
        return nullptr;
    }
    
    try {
        BRepAlgoAPI_Fuse fuseOp(shape1->GetOCCTShape(), shape2->GetOCCTShape());
        fuseOp.Build();
        
        if (fuseOp.IsDone()) {
            TopoDS_Shape result = fuseOp.Shape();
            return PostProcessResult(result);
        }
    } catch (const Standard_Failure& e) {
        // 布尔运算失败
    }
    
    return nullptr;
}

ShapePtr BooleanOperations::PerformIntersection(const ShapePtr& shape1, const ShapePtr& shape2) {
    if (!ValidateInputs(shape1, shape2)) {
        return nullptr;
    }
    
    try {
        BRepAlgoAPI_Common commonOp(shape1->GetOCCTShape(), shape2->GetOCCTShape());
        commonOp.Build();
        
        if (commonOp.IsDone()) {
            TopoDS_Shape result = commonOp.Shape();
            return PostProcessResult(result);
        }
    } catch (const Standard_Failure& e) {
        // 布尔运算失败
    }
    
    return nullptr;
}

ShapePtr BooleanOperations::PerformDifference(const ShapePtr& shape1, const ShapePtr& shape2) {
    if (!ValidateInputs(shape1, shape2)) {
        return nullptr;
    }
    
    try {
        BRepAlgoAPI_Cut cutOp(shape1->GetOCCTShape(), shape2->GetOCCTShape());
        cutOp.Build();
        
        if (cutOp.IsDone()) {
            TopoDS_Shape result = cutOp.Shape();
            return PostProcessResult(result);
        }
    } catch (const Standard_Failure& e) {
        // 布尔运算失败
    }
    
    return nullptr;
}

bool BooleanOperations::ValidateInputs(const ShapePtr& shape1, const ShapePtr& shape2) {
    if (!shape1 || !shape2) {
        return false;
    }
    
    if (shape1->GetOCCTShape().IsNull() || shape2->GetOCCTShape().IsNull()) {
        return false;
    }
    
    return true;
}

ShapePtr BooleanOperations::PostProcessResult(const TopoDS_Shape& result) {
    if (result.IsNull()) {
        return nullptr;
    }
    
    // 创建结果形状
    ShapePtr resultShape = std::make_shared<Shape>(result);
    
    // 验证结果
    if (!IsValidShape(resultShape)) {
        // 尝试修复
        resultShape = FixShape(resultShape);
    }
    
    return resultShape;
}

} // namespace cad_core