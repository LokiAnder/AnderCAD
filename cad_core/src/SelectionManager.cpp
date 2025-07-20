#include "cad_core/SelectionManager.h"
#include <AIS_Selection.hxx>
#include <SelectMgr_EntityOwner.hxx>
#include <StdSelect_BRepOwner.hxx>
#include <TopExp.hxx>
#include <TopTools_IndexedMapOfShape.hxx>
#include <Prs3d_Drawer.hxx>
#include <Quantity_Color.hxx>

namespace cad_core {

SelectionManager::SelectionManager() : m_currentMode(SelectionMode::Shape) {
}

SelectionManager::~SelectionManager() {
    if (!m_context.IsNull()) {
        ClearSelection();
    }
}

void SelectionManager::SetContext(Handle(AIS_InteractiveContext) context) {
    m_context = context;
    if (!m_context.IsNull()) {
        UpdateSelectionMode();
    }
}

void SelectionManager::SetView(Handle(V3d_View) view) {
    m_view = view;
}

void SelectionManager::SetSelectionMode(SelectionMode mode) {
    m_currentMode = mode;
    UpdateSelectionMode();
}

void SelectionManager::UpdateSelectionMode() {
    if (m_context.IsNull()) return;
    
    // 清除当前选择模式
    m_context->ClearCurrents(Standard_False);
    
    // 禁用所有选择模式
    m_context->Deactivate();
    
    // 激活对应的选择模式
    switch (m_currentMode) {
        case SelectionMode::Shape:
            m_context->Activate(0); // 形状模式
            break;
        case SelectionMode::Face:
            m_context->Activate(4); // 面模式
            break;
        case SelectionMode::Edge:
            m_context->Activate(2); // 边模式
            break;
        case SelectionMode::Vertex:
            m_context->Activate(1); // 顶点模式
            break;
    }
    
    // 设置高亮颜色
    Quantity_Color highlightColor;
    switch (m_currentMode) {
        case SelectionMode::Vertex:
            highlightColor = Quantity_NOC_RED;
            break;
        case SelectionMode::Edge:
            highlightColor = Quantity_NOC_YELLOW;
            break;
        case SelectionMode::Face:
            highlightColor = Quantity_NOC_CYAN1;
            break;
        default:
            highlightColor = Quantity_NOC_ORANGE;
            break;
    }
    
    // 应用高亮颜色设置
    Handle(Prs3d_Drawer) hilightDrawer = m_context->HighlightStyle();
    if (!hilightDrawer.IsNull()) {
        hilightDrawer->SetColor(highlightColor);
        hilightDrawer->SetDisplayMode(1); // 高亮显示的着色模式
    }
}

void SelectionManager::StartSelection(int x, int y) {
    if (m_context.IsNull() || m_view.IsNull()) return;
    
    m_context->MoveTo(x, y, m_view, Standard_True);
}

void SelectionManager::UpdateSelection(int x, int y) {
    if (m_context.IsNull() || m_view.IsNull()) return;
    
    m_context->MoveTo(x, y, m_view, Standard_True);
}

void SelectionManager::EndSelection(int x, int y) {
    if (m_context.IsNull() || m_view.IsNull()) return;
    
    m_context->MoveTo(x, y, m_view, Standard_True);
    m_context->Select(Standard_True);
    
    // 为选中的对象启用高亮显示
    m_context->HilightSelected(Standard_True);
    
    // 更新选择信息
    m_selectedItems.clear();
    
    for (m_context->InitSelected(); m_context->MoreSelected(); m_context->NextSelected()) {
        Handle(AIS_InteractiveObject) obj = m_context->SelectedInteractive();
        Handle(AIS_Shape) aisShape = Handle(AIS_Shape)::DownCast(obj);
        
        if (!aisShape.IsNull()) {
            SelectionInfo info = CreateSelectionInfo(aisShape);
            m_selectedItems.push_back(info);
        }
    }
}

void SelectionManager::StartMultiSelection(int x, int y) {
    if (m_context.IsNull() || m_view.IsNull()) return;
    
    m_context->MoveTo(x, y, m_view, Standard_True);
}

void SelectionManager::AddToSelection(int x, int y) {
    if (m_context.IsNull() || m_view.IsNull()) return;
    
    m_context->MoveTo(x, y, m_view, Standard_True);
    m_context->ShiftSelect(Standard_True);
    
    // 更新选择信息
    m_selectedItems.clear();
    
    for (m_context->InitSelected(); m_context->MoreSelected(); m_context->NextSelected()) {
        Handle(AIS_InteractiveObject) obj = m_context->SelectedInteractive();
        Handle(AIS_Shape) aisShape = Handle(AIS_Shape)::DownCast(obj);
        
        if (!aisShape.IsNull()) {
            SelectionInfo info = CreateSelectionInfo(aisShape);
            m_selectedItems.push_back(info);
        }
    }
}

void SelectionManager::RemoveFromSelection(int x, int y) {
    if (m_context.IsNull() || m_view.IsNull()) return;
    
    m_context->MoveTo(x, y, m_view, Standard_True);
    m_context->ShiftSelect(Standard_True);
    
    // 更新选择信息
    m_selectedItems.clear();
    
    for (m_context->InitSelected(); m_context->MoreSelected(); m_context->NextSelected()) {
        Handle(AIS_InteractiveObject) obj = m_context->SelectedInteractive();
        Handle(AIS_Shape) aisShape = Handle(AIS_Shape)::DownCast(obj);
        
        if (!aisShape.IsNull()) {
            SelectionInfo info = CreateSelectionInfo(aisShape);
            m_selectedItems.push_back(info);
        }
    }
}

std::vector<SelectionInfo> SelectionManager::GetSelectedShapes() const {
    std::vector<SelectionInfo> shapes;
    for (const auto& item : m_selectedItems) {
        if (item.shapeType == TopAbs_SOLID || item.shapeType == TopAbs_SHAPE) {
            shapes.push_back(item);
        }
    }
    return shapes;
}

std::vector<SelectionInfo> SelectionManager::GetSelectedFaces() const {
    std::vector<SelectionInfo> faces;
    for (const auto& item : m_selectedItems) {
        if (item.shapeType == TopAbs_FACE) {
            faces.push_back(item);
        }
    }
    return faces;
}

std::vector<SelectionInfo> SelectionManager::GetSelectedEdges() const {
    std::vector<SelectionInfo> edges;
    for (const auto& item : m_selectedItems) {
        if (item.shapeType == TopAbs_EDGE) {
            edges.push_back(item);
        }
    }
    return edges;
}

std::vector<SelectionInfo> SelectionManager::GetSelectedVertices() const {
    std::vector<SelectionInfo> vertices;
    for (const auto& item : m_selectedItems) {
        if (item.shapeType == TopAbs_VERTEX) {
            vertices.push_back(item);
        }
    }
    return vertices;
}

void SelectionManager::ClearSelection() {
    if (!m_context.IsNull()) {
        m_context->ClearCurrents(Standard_False);
    }
    m_selectedItems.clear();
}

bool SelectionManager::HasSelection() const {
    return !m_selectedItems.empty();
}

size_t SelectionManager::GetSelectionCount() const {
    return m_selectedItems.size();
}

void SelectionManager::HighlightShape(const Handle(AIS_Shape)& shape, bool highlight) {
    if (m_context.IsNull() || shape.IsNull()) return;
    
    if (highlight) {
        m_context->SetSelected(shape, Standard_False);
    } else {
        m_context->Unhilight(shape, Standard_False);
    }
}

void SelectionManager::HighlightAll(bool highlight) {
    if (m_context.IsNull()) return;
    
    if (highlight) {
        m_context->HilightCurrents(Standard_False);
    } else {
        m_context->UnhilightCurrents(Standard_False);
    }
}

void SelectionManager::EnableShapeSelection(bool enable) {
    if (m_context.IsNull()) return;
    
    m_context->SetSelectionModeActive(nullptr, 0, enable);
}

void SelectionManager::EnableFaceSelection(bool enable) {
    if (m_context.IsNull()) return;
    
    m_context->SetSelectionModeActive(nullptr, 4, enable);
}

void SelectionManager::EnableEdgeSelection(bool enable) {
    if (m_context.IsNull()) return;
    
    m_context->SetSelectionModeActive(nullptr, 2, enable);
}

void SelectionManager::EnableVertexSelection(bool enable) {
    if (m_context.IsNull()) return;
    
    m_context->SetSelectionModeActive(nullptr, 1, enable);
}

SelectionInfo SelectionManager::CreateSelectionInfo(const Handle(AIS_Shape)& aisShape, int subShapeIndex) {
    SelectionInfo info;
    
    if (aisShape.IsNull()) return info;
    
    TopoDS_Shape shape = aisShape->Shape();
    info.shape = std::make_shared<Shape>(shape);
    
    // 根据当前选择模式确定子形状
    switch (m_currentMode) {
        case SelectionMode::Shape:
            info.subShape = shape;
            info.shapeType = shape.ShapeType();
            info.index = 0;
            break;
            
        case SelectionMode::Face:
            if (subShapeIndex >= 0) {
                info.subShape = GetSubShape(shape, TopAbs_FACE, subShapeIndex);
                info.shapeType = TopAbs_FACE;
                info.index = subShapeIndex;
            }
            break;
            
        case SelectionMode::Edge:
            if (subShapeIndex >= 0) {
                info.subShape = GetSubShape(shape, TopAbs_EDGE, subShapeIndex);
                info.shapeType = TopAbs_EDGE;
                info.index = subShapeIndex;
            }
            break;
            
        case SelectionMode::Vertex:
            if (subShapeIndex >= 0) {
                info.subShape = GetSubShape(shape, TopAbs_VERTEX, subShapeIndex);
                info.shapeType = TopAbs_VERTEX;
                info.index = subShapeIndex;
            }
            break;
    }
    
    return info;
}

TopoDS_Shape SelectionManager::GetSubShape(const TopoDS_Shape& shape, TopAbs_ShapeEnum type, int index) {
    TopTools_IndexedMapOfShape subShapes;
    TopExp::MapShapes(shape, type, subShapes);
    
    if (index >= 0 && index < subShapes.Extent()) {
        return subShapes(index + 1); // TopTools使用基于1的索引
    }
    
    return TopoDS_Shape();
}

int SelectionManager::GetSubShapeIndex(const TopoDS_Shape& shape, const TopoDS_Shape& subShape, TopAbs_ShapeEnum type) {
    TopTools_IndexedMapOfShape subShapes;
    TopExp::MapShapes(shape, type, subShapes);
    
    for (int i = 1; i <= subShapes.Extent(); i++) {
        if (subShapes(i).IsSame(subShape)) {
            return i - 1; // 转换为基于0的索引
        }
    }
    
    return -1;
}

} // namespace cad_core