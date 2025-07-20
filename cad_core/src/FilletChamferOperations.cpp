#include "cad_core/FilletChamferOperations.h"
#include <BRepFilletAPI_MakeFillet.hxx>
#include <BRepFilletAPI_MakeChamfer.hxx>
#include <BRepCheck_Analyzer.hxx>
#include <TopExp_Explorer.hxx>
#include <TopExp.hxx>
#include <TopoDS.hxx>
#include <BRep_Tool.hxx>
#include <GeomAPI_ProjectPointOnCurve.hxx>
#include <Geom_Curve.hxx>
#include <GProp_GProps.hxx>
#include <BRepGProp.hxx>
#include <TopTools_IndexedDataMapOfShapeListOfShape.hxx>
#include <Standard_Failure.hxx>

namespace cad_core {

ShapePtr FilletChamferOperations::CreateFillet(const ShapePtr& shape, const std::vector<TopoDS_Edge>& edges, double radius) {
    return PerformFillet(shape, edges, radius);
}

ShapePtr FilletChamferOperations::CreateFillet(const ShapePtr& shape, const TopoDS_Edge& edge, double radius) {
    std::vector<TopoDS_Edge> edges = {edge};
    return PerformFillet(shape, edges, radius);
}

ShapePtr FilletChamferOperations::CreateVariableFillet(const ShapePtr& shape, const TopoDS_Edge& edge, double radius1, double radius2) {
    if (!shape || shape->GetOCCTShape().IsNull() || edge.IsNull()) {
        return nullptr;
    }
    
    try {
        BRepFilletAPI_MakeFillet fillet(shape->GetOCCTShape());
        fillet.Add(radius1, radius2, edge);
        fillet.Build();
        
        if (fillet.IsDone()) {
            TopoDS_Shape result = fillet.Shape();
            return PostProcessResult(result);
        }
    } catch (const Standard_Failure& e) {
        // 圆角操作失败
    }
    
    return nullptr;
}

ShapePtr FilletChamferOperations::CreateChamfer(const ShapePtr& shape, const std::vector<TopoDS_Edge>& edges, double distance) {
    return PerformChamfer(shape, edges, distance);
}

ShapePtr FilletChamferOperations::CreateChamfer(const ShapePtr& shape, const TopoDS_Edge& edge, double distance) {
    std::vector<TopoDS_Edge> edges = {edge};
    return PerformChamfer(shape, edges, distance);
}

ShapePtr FilletChamferOperations::CreateAsymmetricChamfer(const ShapePtr& shape, const TopoDS_Edge& edge, double distance1, double distance2) {
    if (!shape || shape->GetOCCTShape().IsNull() || edge.IsNull()) {
        return nullptr;
    }
    
    try {
        BRepFilletAPI_MakeChamfer chamfer(shape->GetOCCTShape());
        
        // 获取相邻面
        std::vector<TopoDS_Face> adjacentFaces = GetAdjacentFaces(shape, edge);
        if (adjacentFaces.size() >= 2) {
            chamfer.Add(distance1, distance2, edge, adjacentFaces[0]);
            chamfer.Build();
            
            if (chamfer.IsDone()) {
                TopoDS_Shape result = chamfer.Shape();
                return PostProcessResult(result);
            }
        }
    } catch (const Standard_Failure& e) {
        // 倒角操作失败
    }
    
    return nullptr;
}

ShapePtr FilletChamferOperations::CreateChamferByAngle(const ShapePtr& shape, const TopoDS_Edge& edge, double distance, double angle) {
    if (!shape || shape->GetOCCTShape().IsNull() || edge.IsNull()) {
        return nullptr;
    }
    
    try {
        BRepFilletAPI_MakeChamfer chamfer(shape->GetOCCTShape());
        
        // 获取相邻面
        std::vector<TopoDS_Face> adjacentFaces = GetAdjacentFaces(shape, edge);
        if (adjacentFaces.size() >= 2) {
            chamfer.AddDA(distance, angle, edge, adjacentFaces[0]);
            chamfer.Build();
            
            if (chamfer.IsDone()) {
                TopoDS_Shape result = chamfer.Shape();
                return PostProcessResult(result);
            }
        }
    } catch (const Standard_Failure& e) {
        // 倒角操作失败
    }
    
    return nullptr;
}

ShapePtr FilletChamferOperations::CreateFaceFillet(const ShapePtr& shape, const std::vector<TopoDS_Face>& faces, double radius) {
    if (!shape || shape->GetOCCTShape().IsNull() || faces.empty()) {
        return nullptr;
    }
    
    try {
        BRepFilletAPI_MakeFillet fillet(shape->GetOCCTShape());
        
        // 面圆角需要使用不同的API
        // 暂时不实现面圆角，返回原形状
        return shape;
        
        fillet.Build();
        
        if (fillet.IsDone()) {
            TopoDS_Shape result = fillet.Shape();
            return PostProcessResult(result);
        }
    } catch (const Standard_Failure& e) {
        // 面圆角操作失败
    }
    
    return nullptr;
}

std::vector<TopoDS_Edge> FilletChamferOperations::GetEdges(const ShapePtr& shape) {
    std::vector<TopoDS_Edge> edges;
    
    if (!shape || shape->GetOCCTShape().IsNull()) {
        return edges;
    }
    
    for (TopExp_Explorer exp(shape->GetOCCTShape(), TopAbs_EDGE); exp.More(); exp.Next()) {
        TopoDS_Edge edge = TopoDS::Edge(exp.Current());
        edges.push_back(edge);
    }
    
    return edges;
}

std::vector<TopoDS_Face> FilletChamferOperations::GetFaces(const ShapePtr& shape) {
    std::vector<TopoDS_Face> faces;
    
    if (!shape || shape->GetOCCTShape().IsNull()) {
        return faces;
    }
    
    for (TopExp_Explorer exp(shape->GetOCCTShape(), TopAbs_FACE); exp.More(); exp.Next()) {
        TopoDS_Face face = TopoDS::Face(exp.Current());
        faces.push_back(face);
    }
    
    return faces;
}

bool FilletChamferOperations::IsValidEdgeForFillet(const ShapePtr& shape, const TopoDS_Edge& edge) {
    if (!shape || shape->GetOCCTShape().IsNull() || edge.IsNull()) {
        return false;
    }
    
    // 检查边是否属于形状
    for (TopExp_Explorer exp(shape->GetOCCTShape(), TopAbs_EDGE); exp.More(); exp.Next()) {
        if (exp.Current().IsSame(edge)) {
            // 检查边是否有相邻面
            std::vector<TopoDS_Face> faces = GetAdjacentFaces(shape, edge);
            return faces.size() >= 2;
        }
    }
    
    return false;
}

bool FilletChamferOperations::IsValidEdgeForChamfer(const ShapePtr& shape, const TopoDS_Edge& edge) {
    return IsValidEdgeForFillet(shape, edge); // 同样的验证逻辑
}

std::vector<TopoDS_Face> FilletChamferOperations::GetAdjacentFaces(const ShapePtr& shape, const TopoDS_Edge& edge) {
    std::vector<TopoDS_Face> faces;
    
    if (!shape || shape->GetOCCTShape().IsNull() || edge.IsNull()) {
        return faces;
    }
    
    TopTools_IndexedDataMapOfShapeListOfShape edgeFaceMap;
    TopExp::MapShapesAndAncestors(shape->GetOCCTShape(), TopAbs_EDGE, TopAbs_FACE, edgeFaceMap);
    
    if (edgeFaceMap.Contains(edge)) {
        const TopTools_ListOfShape& faceList = edgeFaceMap.FindFromKey(edge);
        for (TopTools_ListIteratorOfListOfShape it(faceList); it.More(); it.Next()) {
            TopoDS_Face face = TopoDS::Face(it.Value());
            faces.push_back(face);
        }
    }
    
    return faces;
}

double FilletChamferOperations::GetSuggestedFilletRadius(const ShapePtr& shape, const TopoDS_Edge& edge) {
    if (!shape || shape->GetOCCTShape().IsNull() || edge.IsNull()) {
        return 0.0;
    }
    
    // 计算边长度的1/10作为建议半径
    double edgeLength = GetEdgeLength(edge);
    double suggestedRadius = edgeLength * 0.1;
    
    // 确保不超过最小半径
    double minRadius = GetMinimumRadius(shape, edge);
    if (suggestedRadius > minRadius) {
        suggestedRadius = minRadius * 0.8;
    }
    
    return suggestedRadius;
}

double FilletChamferOperations::GetSuggestedChamferDistance(const ShapePtr& shape, const TopoDS_Edge& edge) {
    return GetSuggestedFilletRadius(shape, edge); // 使用相同的逻辑
}

ShapePtr FilletChamferOperations::PerformFillet(const ShapePtr& shape, const std::vector<TopoDS_Edge>& edges, double radius) {
    if (!shape || shape->GetOCCTShape().IsNull() || edges.empty() || radius <= 0.0) {
        return nullptr;
    }
    
    try {
        BRepFilletAPI_MakeFillet fillet(shape->GetOCCTShape());
        
        for (const auto& edge : edges) {
            if (!edge.IsNull() && IsValidEdgeForFillet(shape, edge)) {
                fillet.Add(radius, edge);
            }
        }
        
        fillet.Build();
        
        if (fillet.IsDone()) {
            TopoDS_Shape result = fillet.Shape();
            return PostProcessResult(result);
        }
    } catch (const Standard_Failure& e) {
        // 圆角操作失败
    }
    
    return nullptr;
}

ShapePtr FilletChamferOperations::PerformChamfer(const ShapePtr& shape, const std::vector<TopoDS_Edge>& edges, double distance) {
    if (!shape || shape->GetOCCTShape().IsNull() || edges.empty() || distance <= 0.0) {
        return nullptr;
    }
    
    try {
        BRepFilletAPI_MakeChamfer chamfer(shape->GetOCCTShape());
        
        for (const auto& edge : edges) {
            if (!edge.IsNull() && IsValidEdgeForChamfer(shape, edge)) {
                std::vector<TopoDS_Face> faces = GetAdjacentFaces(shape, edge);
                if (faces.size() >= 1) {
                    chamfer.Add(distance, edge);
                }
            }
        }
        
        chamfer.Build();
        
        if (chamfer.IsDone()) {
            TopoDS_Shape result = chamfer.Shape();
            return PostProcessResult(result);
        }
    } catch (const Standard_Failure& e) {
        // 倒角操作失败
    }
    
    return nullptr;
}

ShapePtr FilletChamferOperations::PostProcessResult(const TopoDS_Shape& result) {
    if (result.IsNull()) {
        return nullptr;
    }
    
    // 验证结果形状
    BRepCheck_Analyzer analyzer(result);
    if (!analyzer.IsValid()) {
        return nullptr;
    }
    
    return std::make_shared<Shape>(result);
}

bool FilletChamferOperations::AnalyzeEdge(const ShapePtr& shape, const TopoDS_Edge& edge, double& minRadius, double& maxRadius) {
    if (!shape || shape->GetOCCTShape().IsNull() || edge.IsNull()) {
        return false;
    }
    
    // 分析边的几何属性
    double edgeLength = GetEdgeLength(edge);
    minRadius = edgeLength * 0.01;  // 1% of edge length
    maxRadius = edgeLength * 0.4;   // 40% of edge length
    
    return true;
}

double FilletChamferOperations::GetEdgeLength(const TopoDS_Edge& edge) {
    if (edge.IsNull()) {
        return 0.0;
    }
    
    GProp_GProps props;
    BRepGProp::LinearProperties(edge, props);
    return props.Mass();
}

double FilletChamferOperations::GetMinimumRadius(const ShapePtr& shape, const TopoDS_Edge& edge) {
    if (!shape || shape->GetOCCTShape().IsNull() || edge.IsNull()) {
        return 0.0;
    }
    
    // 简化实现：返回边长度的5%
    double edgeLength = GetEdgeLength(edge);
    return edgeLength * 0.05;
}

} // namespace cad_core