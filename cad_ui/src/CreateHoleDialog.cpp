#include "cad_ui/CreateHoleDialog.h"
#include "cad_ui/QtOccView.h"
#include "cad_core/ShapeFactory.h"
#include <BRep_Tool.hxx>
#include <Geom_Surface.hxx>
#include <Geom_Plane.hxx>
#include <gp_Dir.hxx>
#include <gp_Ax3.hxx>
#include <gp_Pnt.hxx>
#include <gp_Trsf.hxx>
#include <BRepBuilderAPI_Transform.hxx>
#include <QMessageBox>
#include <QFormLayout>
#pragma execution_character_set("utf-8")

namespace cad_ui {

CreateHoleDialog::CreateHoleDialog(QtOccView* viewer, QWidget* parent)
    : QDialog(parent), m_isSelectingFace(false), m_viewer(viewer), m_previewActive(false) {
    setupUI();
    setModal(false);
    setWindowFlags(Qt::Dialog | Qt::WindowStaysOnTopHint);
    resize(380, 500);
}

// 实现析构函数
CreateHoleDialog::~CreateHoleDialog() {
}

void CreateHoleDialog::setupUI() {
    setWindowTitle("挖孔操作");
    QVBoxLayout* mainLayout = new QVBoxLayout(this);

    // --- 选择组 ---
    m_selectionGroup = new QGroupBox("选择", this);
    auto* selectionLayout = new QVBoxLayout(m_selectionGroup);

    m_selectFaceButton = new QPushButton("选择要挖孔的面", this);
    m_selectionList = new QListWidget(this);
    m_selectionList->setMaximumHeight(60);
    m_selectionList->addItem("尚未选择面...");
    m_selectionList->setStyleSheet("QListWidget::item { color: #888; }");

    selectionLayout->addWidget(m_selectFaceButton);
    selectionLayout->addWidget(m_selectionList);

    // --- 参数组 ---
    m_parametersGroup = new QGroupBox("参数", this);
    QFormLayout* parametersLayout = new QFormLayout(m_parametersGroup);

    m_diameterSpinBox = new QDoubleSpinBox(this);
    m_diameterSpinBox->setRange(0.1, 1000.0);
    m_diameterSpinBox->setValue(5.0);
    m_diameterSpinBox->setSuffix(" mm");
    parametersLayout->addRow("直径:", m_diameterSpinBox);

    m_depthSpinBox = new QDoubleSpinBox(this);
    m_depthSpinBox->setRange(0.1, 1000.0);
    m_depthSpinBox->setValue(5.0);
    m_depthSpinBox->setSuffix(" mm");
    parametersLayout->addRow("深度:", m_depthSpinBox);

    parametersLayout->addRow(new QLabel("--- 孔中心坐标 ---"));
    m_xCoordSpinBox = new QDoubleSpinBox(this);
    m_xCoordSpinBox->setRange(-1000.0, 1000.0);
    m_xCoordSpinBox->setValue(0.0);
    parametersLayout->addRow("坐标 X:", m_xCoordSpinBox);

    m_yCoordSpinBox = new QDoubleSpinBox(this);
    m_yCoordSpinBox->setRange(-1000.0, 1000.0);
    m_yCoordSpinBox->setValue(0.0);
    parametersLayout->addRow("坐标 Y:", m_yCoordSpinBox);

    m_zCoordSpinBox = new QDoubleSpinBox(this);
    m_zCoordSpinBox->setRange(-1000.0, 1000.0);
    m_zCoordSpinBox->setValue(0.0);
    parametersLayout->addRow("坐标 Z:", m_zCoordSpinBox);

    // --- 按钮组 ---
    QHBoxLayout* buttonLayout = new QHBoxLayout();
    m_okButton = new QPushButton("确定", this);
    m_cancelButton = new QPushButton("取消", this);
    m_okButton->setEnabled(false); // 默认不可用

    buttonLayout->addStretch();
    buttonLayout->addWidget(m_cancelButton);
    buttonLayout->addWidget(m_okButton);

    mainLayout->addWidget(m_selectionGroup);
    mainLayout->addWidget(m_parametersGroup);
    mainLayout->addStretch();
    mainLayout->addLayout(buttonLayout);

    // --- 连接信号和槽 ---
    connect(m_selectFaceButton, &QPushButton::clicked, this, &CreateHoleDialog::onSelectFaceClicked);
    connect(m_okButton, &QPushButton::clicked, this, &CreateHoleDialog::onAccept);
    connect(m_cancelButton, &QPushButton::clicked, this, &QDialog::reject);

    // 当参数改变时，自动更新预览
    connect(m_diameterSpinBox, QOverload<double>::of(&QDoubleSpinBox::valueChanged), this, &CreateHoleDialog::onParametersChanged);
    connect(m_depthSpinBox, QOverload<double>::of(&QDoubleSpinBox::valueChanged), this, &CreateHoleDialog::onParametersChanged);
    connect(m_xCoordSpinBox, QOverload<double>::of(&QDoubleSpinBox::valueChanged), this, &CreateHoleDialog::onParametersChanged);
    connect(m_yCoordSpinBox, QOverload<double>::of(&QDoubleSpinBox::valueChanged), this, &CreateHoleDialog::onParametersChanged);
    connect(m_zCoordSpinBox, QOverload<double>::of(&QDoubleSpinBox::valueChanged), this, &CreateHoleDialog::onParametersChanged);
}

void CreateHoleDialog::onSelectFaceClicked() {
    m_isSelectingFace = true;
    m_selectFaceButton->setText("完成");

    emit selectionModeChanged(true, "请选择一个面用于挖孔");

    // 切换按钮功能
    disconnect(m_selectFaceButton, &QPushButton::clicked, this, &CreateHoleDialog::onSelectFaceClicked);
    connect(m_selectFaceButton, &QPushButton::clicked, this, &CreateHoleDialog::onSelectionFinished);
}

void CreateHoleDialog::cleanupAndRestoreView() {
    // 如果预览是激活的，在关闭对话框时请求重置
    if (m_previewActive) {
        emit resetPreviewRequested();
    }
    // 恢复被设为半透明的实体的外观
    if (m_viewer && m_transparentShape) {
        m_viewer->ResetShapeDisplay(m_transparentShape);
    }
    // 关键：清空指针，切断与旧实体的任何联系
    m_transparentShape = nullptr;
}

void CreateHoleDialog::onObjectSelected(const cad_core::ShapePtr& shape) {
    if (m_isSelectingFace) {
        m_targetShape = shape;
    }
}

void CreateHoleDialog::onFaceSelected(const TopoDS_Face& face) {
    if (m_isSelectingFace) {
        m_selectedFace = face;

        if (m_viewer && m_targetShape) {
            if (m_transparentShape && m_transparentShape != m_targetShape) {
                m_viewer->ResetShapeDisplay(m_transparentShape);
            }
            m_viewer->SetShapeTransparency(m_targetShape, 0.8);
            m_transparentShape = m_targetShape;
        }

        updateSelectionDisplay();
        checkCanAccept();

        // 当一个面被选中时，立即激活并显示预览
        onParametersChanged();
        m_previewActive = true;
    }
}

void CreateHoleDialog::onSelectionFinished() {
    m_isSelectingFace = false;
    m_selectFaceButton->setText("选择");

    emit selectionModeChanged(false, "");

    // 恢复按钮功能
    disconnect(m_selectFaceButton, &QPushButton::clicked, this, &CreateHoleDialog::onSelectionFinished);
    connect(m_selectFaceButton, &QPushButton::clicked, this, &CreateHoleDialog::onSelectFaceClicked);
}

void CreateHoleDialog::updateSelectionDisplay() {
    m_selectionList->clear();
    if (!m_selectedFace.IsNull()) {
        m_selectionList->addItem("已选择 1 个面");
        m_selectionList->setStyleSheet("QListWidget::item { color: green; }");
    }
    else {
        m_selectionList->addItem("尚未选择面...");
        m_selectionList->setStyleSheet("QListWidget::item { color: #888; }");
    }
}

void CreateHoleDialog::checkCanAccept() {
    bool canAccept = m_targetShape && !m_selectedFace.IsNull();
    m_okButton->setEnabled(canAccept);
}

void CreateHoleDialog::onAccept() {
    if (!m_targetShape || m_selectedFace.IsNull()) {
        QMessageBox::warning(this, "错误", "请先选择一个有效的面。");
        return;
    }
    // 如果预览是激活的，先重置它
    if (m_previewActive) {
        emit resetPreviewRequested();
    }

    double diameter = m_diameterSpinBox->value();
    double depth = m_depthSpinBox->value();
    double x = m_xCoordSpinBox->value();
    double y = m_yCoordSpinBox->value();
    double z = m_zCoordSpinBox->value();

    emit operationRequested(m_targetShape, m_selectedFace, diameter, depth, x, y, z);
    accept();
}

void CreateHoleDialog::onParametersChanged()
{
    // 只有在预览激活时才更新
    if (m_previewActive) {
        cad_core::ShapePtr previewShape = createHolePreviewShape();
        if (previewShape) {
            emit previewRequested(previewShape);
        }
    }
}

cad_core::ShapePtr CreateHoleDialog::createHolePreviewShape() const
{
    if (!m_targetShape || m_selectedFace.IsNull()) {
        return nullptr;
    }

    Handle(Geom_Surface) surface = BRep_Tool::Surface(m_selectedFace);
    Handle(Geom_Plane) plane = Handle(Geom_Plane)::DownCast(surface);
    if (plane.IsNull()) {
        return nullptr; // 预览只支持平面
    }

    gp_Dir holeDirection = plane->Axis().Direction();
    if (m_selectedFace.Orientation() == TopAbs_REVERSED) {
        holeDirection.Reverse();
    }

    // 创建圆柱体作为预览
    auto cylinder = cad_core::ShapeFactory::CreateCylinder(m_diameterSpinBox->value() / 2.0, m_depthSpinBox->value());
    if (!cylinder) {
        return nullptr;
    }

    gp_Trsf transformation;
    gp_Ax3 targetCoordinateSystem(gp_Pnt(m_xCoordSpinBox->value(), m_yCoordSpinBox->value(), m_zCoordSpinBox->value()), holeDirection.Reversed());
    transformation.SetTransformation(targetCoordinateSystem, gp::XOY());

    // 应用变换
    BRepBuilderAPI_Transform transformer(cylinder->GetOCCTShape(), transformation, Standard_True);
    if (transformer.IsDone()) {
        return std::make_shared<cad_core::Shape>(transformer.Shape());
    }

    return nullptr;
}

void CreateHoleDialog::updateCenterCoords(double x, double y, double z) {
    m_xCoordSpinBox->setValue(x);
    m_yCoordSpinBox->setValue(y);
    m_zCoordSpinBox->setValue(z);
}

} // namespace cad_ui