#include "cad_ui/SketchMode.h"
#include "cad_ui/QtOccView.h"
#include "cad_sketch/SketchPoint.h"
#include <QMouseEvent>
#include <QKeyEvent>
#include <QDebug>
#include <BRep_Tool.hxx>
#include <GeomAPI_ProjectPointOnSurf.hxx>
#include <gp_Pln.hxx>
#include <gp_Pnt.hxx>
#include <gp_Dir.hxx>
#include <Geom_Plane.hxx>
#include <V3d_View.hxx>
#include <cmath>
#pragma execution_character_set("utf-8")

namespace cad_ui {

// =============================================================================
// SketchRectangleTool Implementation
// =============================================================================

SketchRectangleTool::SketchRectangleTool(QObject* parent)
    : QObject(parent), m_isDrawing(false) {
}

void SketchRectangleTool::StartDrawing(const QPoint& startPoint) {
    m_isDrawing = true;
    m_startPoint = startPoint;
    m_currentPoint = startPoint;
    m_currentLines.clear();
    
    qDebug() << "Rectangle tool: Started drawing at" << startPoint;
}

void SketchRectangleTool::UpdateDrawing(const QPoint& currentPoint) {
    if (!m_isDrawing) {
        return;
    }
    
    m_currentPoint = currentPoint;
    
    // 创建临时矩形线条用于预览
    gp_Pnt startPnt = ScreenToSketchPlane(m_startPoint);
    gp_Pnt currentPnt = ScreenToSketchPlane(m_currentPoint);
    
    m_currentLines = CreateRectangleLines(startPnt, currentPnt);
}

void SketchRectangleTool::FinishDrawing(const QPoint& endPoint) {
    if (!m_isDrawing) {
        return;
    }
    
    m_currentPoint = endPoint;
    
    // 创建最终的矩形
    gp_Pnt startPnt = ScreenToSketchPlane(m_startPoint);
    gp_Pnt endPnt = ScreenToSketchPlane(m_currentPoint);
    
    auto rectangleLines = CreateRectangleLines(startPnt, endPnt);
    
    m_isDrawing = false;
    m_currentLines.clear();
    
    emit rectangleCreated(rectangleLines);
    
    qDebug() << "Rectangle tool: Finished drawing rectangle with" << rectangleLines.size() << "lines";
}

void SketchRectangleTool::CancelDrawing() {
    if (!m_isDrawing) {
        return;
    }
    
    m_isDrawing = false;
    m_currentLines.clear();
    
    emit drawingCancelled();
    
    qDebug() << "Rectangle tool: Drawing cancelled";
}

void SketchRectangleTool::SetSketchPlane(const gp_Pln& plane) {
    m_sketchPlane = plane;
}

void SketchRectangleTool::SetView(Handle(V3d_View) view) {
    m_view = view;
}

std::vector<cad_sketch::SketchLinePtr> SketchRectangleTool::GetCurrentRectangle() const {
    return m_currentLines;
}

gp_Pnt SketchRectangleTool::ScreenToSketchPlane(const QPoint& screenPoint) {
    if (m_view.IsNull()) {
        return gp_Pnt(0, 0, 0);
    }
    
    // 简化处理：直接使用屏幕坐标作为2D草图坐标
    // 在真实实现中，这里需要复杂的投影计算
    double scale = 0.1;  // 缩放因子
    double x = (screenPoint.x() - 400) * scale;  // 中心化并缩放
    double y = -(screenPoint.y() - 300) * scale; // Y轴反向并中心化
    double z = 0.0; // 草图平面上Z=0
    
    return gp_Pnt(x, y, z);
}

std::vector<cad_sketch::SketchLinePtr> SketchRectangleTool::CreateRectangleLines(
    const gp_Pnt& point1, const gp_Pnt& point2) {
    
    std::vector<cad_sketch::SketchLinePtr> lines;
    
    // 创建矩形的四个角点
    auto bottomLeft = std::make_shared<cad_sketch::SketchPoint>(
        std::min(point1.X(), point2.X()), 
        std::min(point1.Y(), point2.Y()));
    auto bottomRight = std::make_shared<cad_sketch::SketchPoint>(
        std::max(point1.X(), point2.X()), 
        std::min(point1.Y(), point2.Y()));
    auto topRight = std::make_shared<cad_sketch::SketchPoint>(
        std::max(point1.X(), point2.X()), 
        std::max(point1.Y(), point2.Y()));
    auto topLeft = std::make_shared<cad_sketch::SketchPoint>(
        std::min(point1.X(), point2.X()), 
        std::max(point1.Y(), point2.Y()));
    
    // 创建四条边
    lines.push_back(std::make_shared<cad_sketch::SketchLine>(bottomLeft, bottomRight)); // 底边
    lines.push_back(std::make_shared<cad_sketch::SketchLine>(bottomRight, topRight));   // 右边
    lines.push_back(std::make_shared<cad_sketch::SketchLine>(topRight, topLeft));       // 顶边
    lines.push_back(std::make_shared<cad_sketch::SketchLine>(topLeft, bottomLeft));     // 左边
    
    return lines;
}

// =============================================================================
// SketchMode Implementation
// =============================================================================

SketchMode::SketchMode(QtOccView* viewer, QObject* parent)
    : QObject(parent), m_viewer(viewer), m_isActive(false) {
    
    // 创建绘制工具
    m_rectangleTool = std::make_unique<SketchRectangleTool>(this);
    
    // 连接信号槽
    connect(m_rectangleTool.get(), &SketchRectangleTool::rectangleCreated,
            this, &SketchMode::OnRectangleCreated);
    connect(m_rectangleTool.get(), &SketchRectangleTool::drawingCancelled,
            this, &SketchMode::OnDrawingCancelled);
}

bool SketchMode::EnterSketchMode(const TopoDS_Face& face) {
    if (m_isActive) {
        qDebug() << "Already in sketch mode, exiting first";
        ExitSketchMode();
    }
    
    // 检查参数有效性
    if (face.IsNull()) {
        qDebug() << "Error: Cannot enter sketch mode with null face";
        return false;
    }
    
    if (!m_viewer) {
        qDebug() << "Error: No viewer available";
        return false;
    }
    
    try {
        // 保存当前视图状态
        if (!m_viewer->GetView().IsNull()) {
            Handle(Graphic3d_Camera) camera = m_viewer->GetView()->Camera();
            if (!camera.IsNull()) {
                m_savedEye = camera->Eye();
                m_savedAt = camera->Center();
                m_savedUp = camera->Up();
                m_savedScale = camera->Scale();
            } else {
                qDebug() << "Warning: Camera is null, using default values";
                m_savedEye = gp_Pnt(0, 0, 100);
                m_savedAt = gp_Pnt(0, 0, 0);
                m_savedUp = gp_Dir(0, 1, 0);
                m_savedScale = 1.0;
            }
        } else {
            qDebug() << "Warning: View is null, using default values";
            m_savedEye = gp_Pnt(0, 0, 100);
            m_savedAt = gp_Pnt(0, 0, 0);
            m_savedUp = gp_Dir(0, 1, 0);
            m_savedScale = 1.0;
        }
        
        // 设置草图信息
        m_sketchFace = face;
        SetupSketchPlane(face);
        
        // 创建新的草图
        m_currentSketch = std::make_shared<cad_sketch::Sketch>("Sketch_001");
        
        // 设置草图视图
        SetupSketchView();
        
        // 设置绘制工具
        m_rectangleTool->SetSketchPlane(m_sketchPlane);
        m_rectangleTool->SetView(m_viewer->GetView());
        
        m_isActive = true;
        
        emit sketchModeEntered();
        emit statusMessageChanged("进入草图模式 - 点击\"矩形\"工具开始绘制");
        
        qDebug() << "Entered sketch mode successfully";
        return true;
    }
    catch (const std::exception& e) {
        qDebug() << "Failed to enter sketch mode:" << e.what();
        return false;
    }
}

void SketchMode::ExitSketchMode() {
    if (!m_isActive) {
        return;
    }
    
    // 停止当前工具
    StopCurrentTool();
    
    // 恢复视图
    RestoreView();
    
    // 清理草图数据
    m_currentSketch.reset();
    m_sketchFace = TopoDS_Face();
    
    m_isActive = false;
    
    emit sketchModeExited();
    emit statusMessageChanged("退出草图模式");
    
    qDebug() << "Exited sketch mode";
}

void SketchMode::StartRectangleTool() {
    if (!m_isActive) {
        return;
    }
    
    StopCurrentTool();
    emit statusMessageChanged("矩形工具 - 点击并拖拽创建矩形");
    
    qDebug() << "Started rectangle tool";
}

void SketchMode::StopCurrentTool() {
    if (m_rectangleTool && m_rectangleTool->IsDrawing()) {
        m_rectangleTool->CancelDrawing();
    }
}

void SketchMode::HandleMousePress(QMouseEvent* event) {
    if (!m_isActive) {
        return;
    }
    
    if (event->button() == Qt::LeftButton) {
        m_rectangleTool->StartDrawing(event->pos());
    }
}

void SketchMode::HandleMouseMove(QMouseEvent* event) {
    if (!m_isActive) {
        return;
    }
    
    if (m_rectangleTool->IsDrawing()) {
        m_rectangleTool->UpdateDrawing(event->pos());
        // 可以在这里触发预览更新
    }
}

void SketchMode::HandleMouseRelease(QMouseEvent* event) {
    if (!m_isActive) {
        return;
    }
    
    if (event->button() == Qt::LeftButton && m_rectangleTool->IsDrawing()) {
        m_rectangleTool->FinishDrawing(event->pos());
    }
}

void SketchMode::HandleKeyPress(QKeyEvent* event) {
    if (!m_isActive) {
        return;
    }
    
    if (event->key() == Qt::Key_Escape) {
        if (m_rectangleTool->IsDrawing()) {
            m_rectangleTool->CancelDrawing();
        } else {
            ExitSketchMode();
        }
    }
}

void SketchMode::OnRectangleCreated(const std::vector<cad_sketch::SketchLinePtr>& lines) {
    if (!m_currentSketch) {
        return;
    }
    
    // 将线条添加到草图中
    for (const auto& line : lines) {
        m_currentSketch->AddElement(line);
        emit sketchElementCreated(line);
    }
    
    emit statusMessageChanged(QString("创建了矩形，包含 %1 条线").arg(lines.size()));
    
    qDebug() << "Added rectangle with" << lines.size() << "lines to sketch";
}

void SketchMode::OnDrawingCancelled() {
    emit statusMessageChanged("绘制已取消");
}

void SketchMode::SetupSketchPlane(const TopoDS_Face& face) {
    m_sketchPlane = ExtractPlaneFromFace(face);
    CreateSketchCoordinateSystem();
}

void SketchMode::SetupSketchView() {
    if (!m_viewer || m_viewer->GetView().IsNull()) {
        qDebug() << "Warning: Cannot setup sketch view - viewer or view is null";
        return;
    }
    
    try {
        Handle(V3d_View) view = m_viewer->GetView();
        Handle(Graphic3d_Camera) camera = view->Camera();
        
        if (camera.IsNull()) {
            qDebug() << "Warning: Camera is null in SetupSketchView";
            return;
        }
        
        // 获取草图平面的法向量和位置
        gp_Pnt planeOrigin = m_sketchPlane.Location();
        gp_Dir planeNormal = m_sketchPlane.Axis().Direction();
        
        // 验证方向向量 (gp_Dir已经是标准化的单位向量)
        try {
            gp_Dir testNormal = planeNormal;
        } catch (...) {
            qDebug() << "Warning: Invalid plane normal, using default Z direction";
            planeNormal = gp_Dir(0, 0, 1);
        }
        
        // 设置视图方向（正对草图平面）
        gp_Pnt eyePosition = planeOrigin.XYZ() + planeNormal.XYZ() * 100.0;
        
        // 验证坐标系Y方向
        gp_Dir yDir;
        try {
            yDir = m_sketchCS.YDirection();
        } catch (...) {
            qDebug() << "Warning: Invalid Y direction, using default";
            yDir = gp_Dir(0, 1, 0);
        }
        
        camera->SetEye(eyePosition);
        camera->SetCenter(planeOrigin);
        camera->SetUp(yDir);
        
        // 设置正交投影（对草图更合适）
        camera->SetProjectionType(Graphic3d_Camera::Projection_Orthographic);
        
        // 调整视图大小以适应面
        view->FitAll();
        view->ZFitAll();
        
        qDebug() << "Setup sketch view - Eye:" << eyePosition.X() << eyePosition.Y() << eyePosition.Z();
    }
    catch (const std::exception& e) {
        qDebug() << "Error in SetupSketchView:" << e.what();
    }
}

void SketchMode::RestoreView() {
    if (m_viewer->GetView().IsNull()) {
        return;
    }
    
    Handle(V3d_View) view = m_viewer->GetView();
    
    // 恢复透视投影
    view->Camera()->SetProjectionType(Graphic3d_Camera::Projection_Perspective);
    
    // 恢复保存的视图状态
    view->Camera()->SetEye(m_savedEye);
    view->Camera()->SetCenter(m_savedAt);
    view->Camera()->SetUp(m_savedUp);
    view->Camera()->SetScale(m_savedScale);
    
    qDebug() << "Restored view";
}

void SketchMode::CreateSketchCoordinateSystem() {
    try {
        // 基于草图平面创建坐标系
        gp_Pnt origin = m_sketchPlane.Location();
        gp_Dir zAxis = m_sketchPlane.Axis().Direction();
        gp_Dir xAxis = m_sketchPlane.XAxis().Direction();
        
        // 验证方向向量有效性 (gp_Dir已经是单位向量，检查是否为有效方向)
        try {
            // gp_Dir构造时会自动标准化，这里简单验证即可
            gp_Dir testZ = zAxis;
            gp_Dir testX = xAxis;
        } catch (...) {
            qDebug() << "Warning: Invalid axis directions, using default coordinate system";
            m_sketchCS = gp_Ax3(gp_Pnt(0, 0, 0), gp_Dir(0, 0, 1), gp_Dir(1, 0, 0));
            return;
        }
        
        m_sketchCS = gp_Ax3(origin, zAxis, xAxis);
        qDebug() << "Sketch coordinate system created successfully";
    }
    catch (const std::exception& e) {
        qDebug() << "Error creating sketch coordinate system:" << e.what();
        // 使用默认坐标系
        m_sketchCS = gp_Ax3(gp_Pnt(0, 0, 0), gp_Dir(0, 0, 1), gp_Dir(1, 0, 0));
    }
}

gp_Pln SketchMode::ExtractPlaneFromFace(const TopoDS_Face& face) {
    try {
        if (face.IsNull()) {
            qDebug() << "Error: Face is null, using default XY plane";
            return gp_Pln(gp_Pnt(0, 0, 0), gp_Dir(0, 0, 1));
        }
        
        Handle(Geom_Surface) surface = BRep_Tool::Surface(face);
        if (surface.IsNull()) {
            qDebug() << "Error: Surface is null, using default XY plane";
            return gp_Pln(gp_Pnt(0, 0, 0), gp_Dir(0, 0, 1));
        }
        
        Handle(Geom_Plane) plane = Handle(Geom_Plane)::DownCast(surface);
        
        if (!plane.IsNull()) {
            gp_Pln result = plane->Pln();
            qDebug() << "Successfully extracted plane from face";
            return result;
        }
        
        // 如果不是平面，创建一个默认的XY平面
        qDebug() << "Warning: Selected face is not a plane, using XY plane";
        return gp_Pln(gp_Pnt(0, 0, 0), gp_Dir(0, 0, 1));
    }
    catch (const std::exception& e) {
        qDebug() << "Error extracting plane from face:" << e.what();
        return gp_Pln(gp_Pnt(0, 0, 0), gp_Dir(0, 0, 1));
    }
}

} // namespace cad_ui

#include "SketchMode.moc"