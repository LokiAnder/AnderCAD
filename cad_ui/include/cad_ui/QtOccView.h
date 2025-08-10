#pragma once

#include <QWidget>
#include <QMouseEvent>
#include <QWheelEvent>
#include <QKeyEvent>
#include <QResizeEvent>
#include <QTimer>
#include <map>
#include <memory>

#include <V3d_View.hxx>
#include <V3d_Viewer.hxx>
#include <AIS_InteractiveContext.hxx>
#include <AIS_Shape.hxx>
#include <AIS_ViewController.hxx>
#include <Graphic3d_GraphicDriver.hxx>

#include "cad_core/Shape.h"
#include "cad_core/SelectionManager.h"

namespace cad_ui {

class QtOccView : public QWidget,protected AIS_ViewController {
    Q_OBJECT

public:
    explicit QtOccView(QWidget* parent = nullptr);
    ~QtOccView() = default;

    // 视图器初始化
    bool InitViewer();
    
    // 视图操作
    void FitAll();
    void ZoomIn();
    void ZoomOut();
    void Pan(int dx, int dy);
    void Rotate(int dx, int dy);
    
    // 视图模式
    void SetViewMode(const QString& mode);
    void SetProjectionMode(bool orthographic);
    
    // 形状显示
    void DisplayShape(const cad_core::ShapePtr& shape);
    void RemoveShape(const cad_core::ShapePtr& shape);
    void ClearShapes();

	// 预览形状显示
    void DisplayPreviewShape(const cad_core::ShapePtr& shape);
    void ClearPreviewShapes();
    void EnablePreviewDragging(const gp_Pln& plane);// 预览拖拽
    void DisablePreviewDragging();

    void RedrawAll();
    virtual QPaintEngine* paintEngine() const;
    
    // 背景和外观
    void SetBackgroundColor(const QColor& color);
    void SetBackgroundGradient(const QColor& color1, const QColor& color2);
    
    // 选择
    void SetSelectionMode(int mode);
    void SetSelectionMode(cad_core::SelectionMode mode);
    void ClearSelection();
    void SelectShape(const cad_core::ShapePtr& shape);
    
    // 用于操作的边和面选择
    void ClearEdgeSelection();
    std::vector<TopoDS_Edge> GetSelectedTopoEdges() const { return m_selectedEdges; }
    std::map<cad_core::ShapePtr, std::vector<TopoDS_Edge>> GetSelectedEdgesByShape() const;
    void HighlightEdge(const TopoDS_Edge& edge);
    void HighlightVertex(const TopoDS_Vertex& vertex);
    void HighlightFace(const TopoDS_Face& face);
    void SetShapeTransparency(const cad_core::ShapePtr& shape, double transparency);
    void ResetShapeDisplay(const cad_core::ShapePtr& shape);
    void UnhighlightAllEdges();
    void UnhighlightAllVertices();
    void UnhighlightAllFaces();
	
    
    // 高级选择方法
    std::vector<cad_core::SelectionInfo> GetSelectedShapes() const;
    std::vector<cad_core::SelectionInfo> GetSelectedFaces() const;
    std::vector<cad_core::SelectionInfo> GetSelectedEdges() const;
    std::vector<cad_core::SelectionInfo> GetSelectedVertices() const;
    
    // 选择管理器访问
    cad_core::SelectionManager* GetSelectionManager() { return m_selectionManager.get(); }
    
    // 视图访问
    Handle(V3d_View) GetView() const { return m_view; }
    Handle(AIS_InteractiveContext) GetContext() const { return m_context; }
    
    // 网格
    void ShowGrid(bool show);
    void SetGridSpacing(double spacing);
    
    // 坐标轴
    void ShowAxes(bool show);
    
    // 草图模式支持
    bool IsInSketchMode() const;
    void EnterSketchMode(const TopoDS_Face& face);
    void ExitSketchMode();
    void StartRectangleTool();


signals:
    void ShapeSelected(const cad_core::ShapePtr& shape);
    void FaceSelected(const TopoDS_Face& face, const cad_core::ShapePtr& parentShape);
    void ViewChanged();
    void SketchModeEntered();
    void SketchModeExited();
    void MousePositionChanged(int x, int y);
    void Mouse3DPositionChanged(double x, double y, double z);
	void previewObjectMoved(double x, double y, double z);// 拖拽时告知新作标

protected:
    void paintEvent(QPaintEvent* event) override;
    void resizeEvent(QResizeEvent* event) override;
    void mousePressEvent(QMouseEvent* event) override;
    void mouseMoveEvent(QMouseEvent* event) override;
    void mouseReleaseEvent(QMouseEvent* event) override;
    void wheelEvent(QWheelEvent* event) override;
    void keyPressEvent(QKeyEvent* event) override;
    void focusInEvent(QFocusEvent* event) override;
    void focusOutEvent(QFocusEvent* event) override;
    void enterEvent(QEvent* event) override;
    void leaveEvent(QEvent* event) override;
    void showEvent(QShowEvent* event) override;
    void changeEvent(QEvent* event) override;

private:
    Handle(V3d_Viewer) m_viewer;
    Handle(V3d_View) m_view;
    Handle(AIS_InteractiveContext) m_context;
    Handle(Graphic3d_GraphicDriver) m_driver;
    
    QPoint m_lastMousePos;
    Qt::MouseButton m_currentMouseButton;
    bool m_isInitialized;
    
    QTimer* m_redrawTimer;
    
    // 选择管理器
    std::unique_ptr<cad_core::SelectionManager> m_selectionManager;
    
    // 用于选择同步的形状映射
    std::map<cad_core::ShapePtr, Handle(AIS_Shape)> m_shapeToAIS;
    
    // 当前选择状态（单选模式）
    cad_core::ShapePtr m_currentSelectedShape;
    Handle(AIS_Shape) m_currentSelectedAIS;
    
    // 用于倒角/倒圆等操作的边选择状态
    std::vector<TopoDS_Edge> m_selectedEdges;
    std::vector<Handle(AIS_InteractiveObject)> m_highlightedEdges;
    std::vector<cad_core::ShapePtr> m_edgeParentShapes;  // 跟踪每个边的父形状（与m_selectedEdges索引相同）
    
    // 用于点选择的高亮状态
    std::vector<TopoDS_Vertex> m_selectedVertices;
    std::vector<Handle(AIS_InteractiveObject)> m_highlightedVertices;
    
    // 用于面选择的高亮状态
    std::vector<TopoDS_Face> m_selectedFaces;
    std::vector<Handle(AIS_InteractiveObject)> m_highlightedFaces;

	// 用于预览形状的显示
    std::vector<Handle(AIS_InteractiveObject)> m_previewAISShapes;

	// 拖拽预览相关（未实现）
    bool m_isDraggingPreview;
    Handle(AIS_InteractiveObject) m_draggedObject;
    gp_Pln m_draggingPlane;
    gp_Pnt m_dragStartPoint3D;
    
    // 草图模式
    std::unique_ptr<class SketchMode> m_sketchMode;
    
    // 当前选择模式
    int m_currentSelectionMode;
    
    void InitializeOCC();
    void RedrawView();
    void HandleSelection(const QPoint& point);
    
private slots:
    void OnRedrawTimer();
};

} // namespace cad_ui