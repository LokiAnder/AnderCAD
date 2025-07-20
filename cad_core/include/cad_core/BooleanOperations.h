#pragma once

#include "cad_core/Shape.h"
#include <vector>

namespace cad_core {

class BooleanOperations {
public:
    // 布尔运算类型
    enum class BooleanType {
        Union,        // 并集
        Intersection, // 交集
        Difference    // 差集
    };
    
    // 布尔运算
    static ShapePtr Union(const ShapePtr& shape1, const ShapePtr& shape2);
    static ShapePtr Union(const std::vector<ShapePtr>& shapes);
    
    static ShapePtr Intersection(const ShapePtr& shape1, const ShapePtr& shape2);
    static ShapePtr Intersection(const std::vector<ShapePtr>& shapes);
    
    static ShapePtr Difference(const ShapePtr& shape1, const ShapePtr& shape2);
    
    // 通用布尔运算
    static ShapePtr BooleanOperation(const ShapePtr& shape1, const ShapePtr& shape2, BooleanType type);
    static ShapePtr BooleanOperation(const std::vector<ShapePtr>& shapes, BooleanType type);
    
    // 验证形状是否有效
    static bool IsValidShape(const ShapePtr& shape);
    
    // 修复形状
    static ShapePtr FixShape(const ShapePtr& shape);
    
    // 简化形状
    static ShapePtr SimplifyShape(const ShapePtr& shape);
    
private:
    // 私有辅助方法
    static ShapePtr PerformUnion(const ShapePtr& shape1, const ShapePtr& shape2);
    static ShapePtr PerformIntersection(const ShapePtr& shape1, const ShapePtr& shape2);
    static ShapePtr PerformDifference(const ShapePtr& shape1, const ShapePtr& shape2);
    
    // 形状验证和修复
    static bool ValidateInputs(const ShapePtr& shape1, const ShapePtr& shape2);
    static ShapePtr PostProcessResult(const TopoDS_Shape& result);
};

} // namespace cad_core