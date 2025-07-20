#pragma once

#include "cad_core/Shape.h"
#include <TopoDS_Edge.hxx>
#include <TopoDS_Face.hxx>
#include <vector>

namespace cad_core {

class FilletChamferOperations {
public:
    // 圆角操作
    static ShapePtr CreateFillet(const ShapePtr& shape, const std::vector<TopoDS_Edge>& edges, double radius);
    static ShapePtr CreateFillet(const ShapePtr& shape, const TopoDS_Edge& edge, double radius);
    static ShapePtr CreateVariableFillet(const ShapePtr& shape, const TopoDS_Edge& edge, double radius1, double radius2);
    
    // 倒角操作
    static ShapePtr CreateChamfer(const ShapePtr& shape, const std::vector<TopoDS_Edge>& edges, double distance);
    static ShapePtr CreateChamfer(const ShapePtr& shape, const TopoDS_Edge& edge, double distance);
    static ShapePtr CreateAsymmetricChamfer(const ShapePtr& shape, const TopoDS_Edge& edge, double distance1, double distance2);
    static ShapePtr CreateChamferByAngle(const ShapePtr& shape, const TopoDS_Edge& edge, double distance, double angle);
    
    // 面圆角
    static ShapePtr CreateFaceFillet(const ShapePtr& shape, const std::vector<TopoDS_Face>& faces, double radius);
    
    // 获取形状的边
    static std::vector<TopoDS_Edge> GetEdges(const ShapePtr& shape);
    static std::vector<TopoDS_Face> GetFaces(const ShapePtr& shape);
    
    // 边验证
    static bool IsValidEdgeForFillet(const ShapePtr& shape, const TopoDS_Edge& edge);
    static bool IsValidEdgeForChamfer(const ShapePtr& shape, const TopoDS_Edge& edge);
    
    // 获取边的相邻面
    static std::vector<TopoDS_Face> GetAdjacentFaces(const ShapePtr& shape, const TopoDS_Edge& edge);
    
    // 计算建议的圆角/倒角尺寸
    static double GetSuggestedFilletRadius(const ShapePtr& shape, const TopoDS_Edge& edge);
    static double GetSuggestedChamferDistance(const ShapePtr& shape, const TopoDS_Edge& edge);
    
private:
    // 私有辅助方法
    static ShapePtr PerformFillet(const ShapePtr& shape, const std::vector<TopoDS_Edge>& edges, double radius);
    static ShapePtr PerformChamfer(const ShapePtr& shape, const std::vector<TopoDS_Edge>& edges, double distance);
    static ShapePtr PostProcessResult(const TopoDS_Shape& result);
    
    // 边分析
    static bool AnalyzeEdge(const ShapePtr& shape, const TopoDS_Edge& edge, double& minRadius, double& maxRadius);
    static double GetEdgeLength(const TopoDS_Edge& edge);
    static double GetMinimumRadius(const ShapePtr& shape, const TopoDS_Edge& edge);
};

} // namespace cad_core