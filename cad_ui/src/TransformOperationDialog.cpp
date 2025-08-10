#include "cad_ui/TransformOperationDialog.h"
#include "cad_core/Point.h"
#include <QApplication>
#include <QMessageBox>
#include <QSplitter>
#include <QSlider>
#include <QDebug>
#include <cmath>
#pragma execution_character_set("utf-8")

namespace cad_ui {

// 静态常量定义
constexpr double TransformOperationDialog::AXIS_X[];
constexpr double TransformOperationDialog::AXIS_Y[];
constexpr double TransformOperationDialog::AXIS_Z[];

TransformOperationDialog::TransformOperationDialog(QWidget* parent)
    : QDialog(parent), m_selectingObjects(false), m_previewActive(false) {
    setupUI();
    setModal(false);
    setWindowFlags(Qt::Dialog | Qt::WindowStaysOnTopHint);
    resize(450, 600);
    setWindowTitle("变换操作");
}

void TransformOperationDialog::setupUI() {
    m_mainLayout = new QVBoxLayout(this);
    m_mainLayout->setSpacing(8);
    m_mainLayout->setContentsMargins(12, 12, 12, 12);
    
    // 对象选择区域
    m_selectionGroup = new QGroupBox("选择对象", this);
    m_selectionLayout = new QGridLayout(m_selectionGroup);
    m_selectionLayout->setSpacing(8);
    
    // 对象选择框（仿UGNX风格）
    m_objectFrame = new QFrame(this);
    m_objectFrame->setFrameStyle(QFrame::StyledPanel | QFrame::Sunken);
    m_objectFrame->setMinimumHeight(40);
    m_objectFrame->setStyleSheet(
        "QFrame { background-color: #F8F8F8; border: 2px solid #E0E0E0; border-radius: 4px; }"
        "QFrame:hover { border-color: #009999; }"
    );
    
    m_objectFrameLayout = new QHBoxLayout(m_objectFrame);
    m_objectFrameLayout->setContentsMargins(8, 4, 8, 4);
    
    m_objectCount = new QLabel("请选择要变换的对象", this);
    m_objectCount->setStyleSheet("color: #666666; font-style: italic;");
    m_objectFrameLayout->addWidget(m_objectCount);
    m_objectFrameLayout->addStretch();
    
    m_objectSelectButton = new QPushButton("选择", this);
    m_objectSelectButton->setFixedSize(60, 30);
    m_objectSelectButton->setStyleSheet(
        "QPushButton { background-color: #009999; color: white; border: none; border-radius: 4px; font-weight: bold; }"
        "QPushButton:hover { background-color: #007777; }"
        "QPushButton:pressed { background-color: #005555; }"
    );
    m_objectFrameLayout->addWidget(m_objectSelectButton);
    
    m_selectionLayout->addWidget(m_objectFrame, 0, 0, 1, 2);
    
    // 对象列表
    m_objectList = new QListWidget(this);
    m_objectList->setMaximumHeight(100);
    m_objectList->setStyleSheet(
        "QListWidget { border: 1px solid #D0D0D0; background-color: #FAFAFA; }"
        "QListWidget::item { padding: 4px; border-bottom: 1px solid #E0E0E0; }"
        "QListWidget::item:selected { background-color: #009999; color: white; }"
    );
    m_selectionLayout->addWidget(m_objectList, 1, 0, 1, 2);
    
    m_mainLayout->addWidget(m_selectionGroup);
    
    // 变换类型选择
    m_transformTabs = new QTabWidget(this);
    m_transformTabs->setStyleSheet(
        "QTabWidget::pane { border: 1px solid #C0C0C0; background-color: white; }"
        "QTabBar::tab { background-color: #F0F0F0; padding: 8px 16px; margin-right: 2px; }"
        "QTabBar::tab:selected { background-color: #009999; color: white; }"
        "QTabBar::tab:hover:!selected { background-color: #E0E0E0; }"
    );
    
    setupTranslationTab();
    setupRotationTab();
    setupScaleTab();
    
    m_mainLayout->addWidget(m_transformTabs);
    
    // 控制按钮
    m_buttonLayout = new QHBoxLayout();
    m_buttonLayout->setSpacing(8);
    
    m_previewButton = new QPushButton("预览", this);
    m_previewButton->setFixedSize(80, 35);
    m_previewButton->setStyleSheet(
        "QPushButton { background-color: #4CAF50; color: white; border: none; border-radius: 4px; font-weight: bold; }"
        "QPushButton:hover { background-color: #45A049; }"
        "QPushButton:disabled { background-color: #CCCCCC; }"
    );
    
    m_resetButton = new QPushButton("重置", this);
    m_resetButton->setFixedSize(80, 35);
    m_resetButton->setStyleSheet(
        "QPushButton { background-color: #FF9800; color: white; border: none; border-radius: 4px; font-weight: bold; }"
        "QPushButton:hover { background-color: #E68900; }"
    );
    
    m_okButton = new QPushButton("确定", this);
    m_okButton->setFixedSize(80, 35);
    m_okButton->setStyleSheet(
        "QPushButton { background-color: #2196F3; color: white; border: none; border-radius: 4px; font-weight: bold; }"
        "QPushButton:hover { background-color: #1976D2; }"
        "QPushButton:disabled { background-color: #CCCCCC; }"
    );
    
    m_cancelButton = new QPushButton("取消", this);
    m_cancelButton->setFixedSize(80, 35);
    m_cancelButton->setStyleSheet(
        "QPushButton { background-color: #F44336; color: white; border: none; border-radius: 4px; font-weight: bold; }"
        "QPushButton:hover { background-color: #D32F2F; }"
    );
    
    m_buttonLayout->addWidget(m_previewButton);
    m_buttonLayout->addWidget(m_resetButton);
    m_buttonLayout->addStretch();
    m_buttonLayout->addWidget(m_okButton);
    m_buttonLayout->addWidget(m_cancelButton);
    
    m_mainLayout->addLayout(m_buttonLayout);
    
    // 连接信号槽
    connect(m_objectSelectButton, &QPushButton::clicked, this, &TransformOperationDialog::onObjectSelectionClicked);
    connect(m_previewButton, &QPushButton::clicked, this, &TransformOperationDialog::onPreviewClicked);
    connect(m_resetButton, &QPushButton::clicked, this, &TransformOperationDialog::onResetClicked);
    connect(m_okButton, &QPushButton::clicked, this, &TransformOperationDialog::accept);
    connect(m_cancelButton, &QPushButton::clicked, this, &TransformOperationDialog::reject);
    connect(m_transformTabs, &QTabWidget::currentChanged, this, &TransformOperationDialog::onTransformTypeChanged);
    
    // 初始状态
    m_previewButton->setEnabled(false);
    m_okButton->setEnabled(false);
}

void TransformOperationDialog::setupTranslationTab() {
    m_translateTab = new QWidget();
    m_translationGroup = new QGroupBox("平移参数", m_translateTab);
    m_translationLayout = new QGridLayout(m_translationGroup);
    m_translationLayout->setSpacing(8);
    
    // X方向平移
    m_translationLayout->addWidget(new QLabel("X方向:", m_translateTab), 0, 0);
    m_translateX = new QDoubleSpinBox(m_translateTab);
    m_translateX->setRange(-999999.0, 999999.0);
    m_translateX->setDecimals(3);
    m_translateX->setSuffix(" mm");
    m_translateX->setValue(10.0);
    m_translationLayout->addWidget(m_translateX, 0, 1);
    
    // Y方向平移
    m_translationLayout->addWidget(new QLabel("Y方向:", m_translateTab), 1, 0);
    m_translateY = new QDoubleSpinBox(m_translateTab);
    m_translateY->setRange(-999999.0, 999999.0);
    m_translateY->setDecimals(3);
    m_translateY->setSuffix(" mm");
    m_translateY->setValue(0.0);
    m_translationLayout->addWidget(m_translateY, 1, 1);
    
    // Z方向平移
    m_translationLayout->addWidget(new QLabel("Z方向:", m_translateTab), 2, 0);
    m_translateZ = new QDoubleSpinBox(m_translateTab);
    m_translateZ->setRange(-999999.0, 999999.0);
    m_translateZ->setDecimals(3);
    m_translateZ->setSuffix(" mm");
    m_translateZ->setValue(0.0);
    m_translationLayout->addWidget(m_translateZ, 2, 1);
    
    QVBoxLayout* translateTabLayout = new QVBoxLayout(m_translateTab);
    translateTabLayout->addWidget(m_translationGroup);
    translateTabLayout->addStretch();
    
    m_transformTabs->addTab(m_translateTab, "平移");
    
    // 连接信号槽
    connect(m_translateX, QOverload<double>::of(&QDoubleSpinBox::valueChanged),
            this, &TransformOperationDialog::onTranslationParameterChanged);
    connect(m_translateY, QOverload<double>::of(&QDoubleSpinBox::valueChanged),
            this, &TransformOperationDialog::onTranslationParameterChanged);
    connect(m_translateZ, QOverload<double>::of(&QDoubleSpinBox::valueChanged),
            this, &TransformOperationDialog::onTranslationParameterChanged);
}

void TransformOperationDialog::setupRotationTab() {
    m_rotateTab = new QWidget();
    QVBoxLayout* rotateTabLayout = new QVBoxLayout(m_rotateTab);
    rotateTabLayout->setSpacing(8);
    
    // 旋转轴点
    m_axisPointGroup = new QGroupBox("旋转轴点", m_rotateTab);
    QGridLayout* axisPointLayout = new QGridLayout(m_axisPointGroup);
    
    axisPointLayout->addWidget(new QLabel("X:", m_rotateTab), 0, 0);
    m_axisPointX = new QDoubleSpinBox(m_rotateTab);
    m_axisPointX->setRange(-999999.0, 999999.0);
    m_axisPointX->setDecimals(3);
    m_axisPointX->setValue(0.0);
    axisPointLayout->addWidget(m_axisPointX, 0, 1);
    
    axisPointLayout->addWidget(new QLabel("Y:", m_rotateTab), 1, 0);
    m_axisPointY = new QDoubleSpinBox(m_rotateTab);
    m_axisPointY->setRange(-999999.0, 999999.0);
    m_axisPointY->setDecimals(3);
    m_axisPointY->setValue(0.0);
    axisPointLayout->addWidget(m_axisPointY, 1, 1);
    
    axisPointLayout->addWidget(new QLabel("Z:", m_rotateTab), 2, 0);
    m_axisPointZ = new QDoubleSpinBox(m_rotateTab);
    m_axisPointZ->setRange(-999999.0, 999999.0);
    m_axisPointZ->setDecimals(3);
    m_axisPointZ->setValue(0.0);
    axisPointLayout->addWidget(m_axisPointZ, 2, 1);
    
    rotateTabLayout->addWidget(m_axisPointGroup);
    
    // 旋转轴方向
    m_axisDirectionGroup = new QGroupBox("旋转轴方向", m_rotateTab);
    QVBoxLayout* axisDirectionLayout = new QVBoxLayout(m_axisDirectionGroup);
    
    // 预设轴方向
    QHBoxLayout* axisPresetLayout = new QHBoxLayout();
    m_axisPresetGroup = new QButtonGroup(m_rotateTab);
    
    m_axisXButton = new QRadioButton("X轴", m_rotateTab);
    m_axisYButton = new QRadioButton("Y轴", m_rotateTab);
    m_axisZButton = new QRadioButton("Z轴", m_rotateTab);
    m_axisCustomButton = new QRadioButton("自定义", m_rotateTab);
    
    m_axisZButton->setChecked(true); // 默认Z轴
    
    m_axisPresetGroup->addButton(m_axisXButton, 0);
    m_axisPresetGroup->addButton(m_axisYButton, 1);
    m_axisPresetGroup->addButton(m_axisZButton, 2);
    m_axisPresetGroup->addButton(m_axisCustomButton, 3);
    
    axisPresetLayout->addWidget(m_axisXButton);
    axisPresetLayout->addWidget(m_axisYButton);
    axisPresetLayout->addWidget(m_axisZButton);
    axisPresetLayout->addWidget(m_axisCustomButton);
    axisPresetLayout->addStretch();
    
    axisDirectionLayout->addLayout(axisPresetLayout);
    
    // 自定义轴方向
    QGridLayout* customAxisLayout = new QGridLayout();
    customAxisLayout->addWidget(new QLabel("X:", m_rotateTab), 0, 0);
    m_axisDirectionX = new QDoubleSpinBox(m_rotateTab);
    m_axisDirectionX->setRange(-1.0, 1.0);
    m_axisDirectionX->setDecimals(3);
    m_axisDirectionX->setValue(0.0);
    m_axisDirectionX->setEnabled(false);
    customAxisLayout->addWidget(m_axisDirectionX, 0, 1);
    
    customAxisLayout->addWidget(new QLabel("Y:", m_rotateTab), 1, 0);
    m_axisDirectionY = new QDoubleSpinBox(m_rotateTab);
    m_axisDirectionY->setRange(-1.0, 1.0);
    m_axisDirectionY->setDecimals(3);
    m_axisDirectionY->setValue(0.0);
    m_axisDirectionY->setEnabled(false);
    customAxisLayout->addWidget(m_axisDirectionY, 1, 1);
    
    customAxisLayout->addWidget(new QLabel("Z:", m_rotateTab), 2, 0);
    m_axisDirectionZ = new QDoubleSpinBox(m_rotateTab);
    m_axisDirectionZ->setRange(-1.0, 1.0);
    m_axisDirectionZ->setDecimals(3);
    m_axisDirectionZ->setValue(1.0);
    m_axisDirectionZ->setEnabled(false);
    customAxisLayout->addWidget(m_axisDirectionZ, 2, 1);
    
    axisDirectionLayout->addLayout(customAxisLayout);
    rotateTabLayout->addWidget(m_axisDirectionGroup);
    
    // 旋转角度
    m_angleGroup = new QGroupBox("旋转角度", m_rotateTab);
    QVBoxLayout* angleLayout = new QVBoxLayout(m_angleGroup);
    
    QHBoxLayout* angleUnitLayout = new QHBoxLayout();
    m_angleDegrees = new QRadioButton("度", m_rotateTab);
    m_angleRadians = new QRadioButton("弧度", m_rotateTab);
    m_angleDegrees->setChecked(true); // 默认使用度
    
    angleUnitLayout->addWidget(m_angleDegrees);
    angleUnitLayout->addWidget(m_angleRadians);
    angleUnitLayout->addStretch();
    angleLayout->addLayout(angleUnitLayout);
    
    m_rotationAngle = new QDoubleSpinBox(m_rotateTab);
    m_rotationAngle->setRange(-360.0, 360.0);
    m_rotationAngle->setDecimals(2);
    m_rotationAngle->setSuffix("°");
    m_rotationAngle->setValue(90.0);
    angleLayout->addWidget(m_rotationAngle);
    
    rotateTabLayout->addWidget(m_angleGroup);
    rotateTabLayout->addStretch();
    
    m_transformTabs->addTab(m_rotateTab, "旋转");
    
    // 连接信号槽
    connect(m_axisPresetGroup, QOverload<int>::of(&QButtonGroup::buttonClicked),
            [this](int id) {
                bool isCustom = (id == 3);
                m_axisDirectionX->setEnabled(isCustom);
                m_axisDirectionY->setEnabled(isCustom);
                m_axisDirectionZ->setEnabled(isCustom);
                
                if (!isCustom) {
                    const double* axis = nullptr;
                    switch (id) {
                        case 0: axis = AXIS_X; break;
                        case 1: axis = AXIS_Y; break;
                        case 2: axis = AXIS_Z; break;
                    }
                    if (axis) {
                        m_axisDirectionX->setValue(axis[0]);
                        m_axisDirectionY->setValue(axis[1]);
                        m_axisDirectionZ->setValue(axis[2]);
                    }
                }
                onRotationParameterChanged();
            });
    
    connect(m_angleDegrees, &QRadioButton::toggled, [this](bool checked) {
        if (checked) {
            m_rotationAngle->setRange(-360.0, 360.0);
            m_rotationAngle->setSuffix("°");
            m_rotationAngle->setValue(m_rotationAngle->value() * 180.0 / M_PI);
        }
        onRotationParameterChanged();
    });
    
    connect(m_angleRadians, &QRadioButton::toggled, [this](bool checked) {
        if (checked) {
            m_rotationAngle->setRange(-2*M_PI, 2*M_PI);
            m_rotationAngle->setSuffix(" rad");
            m_rotationAngle->setValue(m_rotationAngle->value() * M_PI / 180.0);
        }
        onRotationParameterChanged();
    });
    
    // 连接所有旋转参数变化的信号
    connect(m_axisPointX, QOverload<double>::of(&QDoubleSpinBox::valueChanged),
            this, &TransformOperationDialog::onRotationParameterChanged);
    connect(m_axisPointY, QOverload<double>::of(&QDoubleSpinBox::valueChanged),
            this, &TransformOperationDialog::onRotationParameterChanged);
    connect(m_axisPointZ, QOverload<double>::of(&QDoubleSpinBox::valueChanged),
            this, &TransformOperationDialog::onRotationParameterChanged);
    connect(m_axisDirectionX, QOverload<double>::of(&QDoubleSpinBox::valueChanged),
            this, &TransformOperationDialog::onRotationParameterChanged);
    connect(m_axisDirectionY, QOverload<double>::of(&QDoubleSpinBox::valueChanged),
            this, &TransformOperationDialog::onRotationParameterChanged);
    connect(m_axisDirectionZ, QOverload<double>::of(&QDoubleSpinBox::valueChanged),
            this, &TransformOperationDialog::onRotationParameterChanged);
    connect(m_rotationAngle, QOverload<double>::of(&QDoubleSpinBox::valueChanged),
            this, &TransformOperationDialog::onRotationParameterChanged);
}

void TransformOperationDialog::setupScaleTab() {
    m_scaleTab = new QWidget();
    QVBoxLayout* scaleTabLayout = new QVBoxLayout(m_scaleTab);
    scaleTabLayout->setSpacing(8);
    
    // 缩放中心
    m_scaleCenterGroup = new QGroupBox("缩放中心", m_scaleTab);
    QVBoxLayout* scaleCenterLayout = new QVBoxLayout(m_scaleCenterGroup);
    
    // 快速设置按钮
    QHBoxLayout* centerButtonsLayout = new QHBoxLayout();
    m_centerAtOriginButton = new QPushButton("原点", m_scaleTab);
    m_centerAtBoundingBoxButton = new QPushButton("包围盒中心", m_scaleTab);
    centerButtonsLayout->addWidget(m_centerAtOriginButton);
    centerButtonsLayout->addWidget(m_centerAtBoundingBoxButton);
    centerButtonsLayout->addStretch();
    scaleCenterLayout->addLayout(centerButtonsLayout);
    
    // 自定义中心点
    QGridLayout* centerPointLayout = new QGridLayout();
    centerPointLayout->addWidget(new QLabel("X:", m_scaleTab), 0, 0);
    m_scaleCenterX = new QDoubleSpinBox(m_scaleTab);
    m_scaleCenterX->setRange(-999999.0, 999999.0);
    m_scaleCenterX->setDecimals(3);
    m_scaleCenterX->setValue(0.0);
    centerPointLayout->addWidget(m_scaleCenterX, 0, 1);
    
    centerPointLayout->addWidget(new QLabel("Y:", m_scaleTab), 1, 0);
    m_scaleCenterY = new QDoubleSpinBox(m_scaleTab);
    m_scaleCenterY->setRange(-999999.0, 999999.0);
    m_scaleCenterY->setDecimals(3);
    m_scaleCenterY->setValue(0.0);
    centerPointLayout->addWidget(m_scaleCenterY, 1, 1);
    
    centerPointLayout->addWidget(new QLabel("Z:", m_scaleTab), 2, 0);
    m_scaleCenterZ = new QDoubleSpinBox(m_scaleTab);
    m_scaleCenterZ->setRange(-999999.0, 999999.0);
    m_scaleCenterZ->setDecimals(3);
    m_scaleCenterZ->setValue(0.0);
    centerPointLayout->addWidget(m_scaleCenterZ, 2, 1);
    
    scaleCenterLayout->addLayout(centerPointLayout);
    scaleTabLayout->addWidget(m_scaleCenterGroup);
    
    // 缩放因子
    m_scaleFactorGroup = new QGroupBox("缩放因子", m_scaleTab);
    QVBoxLayout* scaleFactorLayout = new QVBoxLayout(m_scaleFactorGroup);
    
    // 均匀缩放选项
    m_uniformScaleCheckBox = new QCheckBox("均匀缩放", m_scaleTab);
    m_uniformScaleCheckBox->setChecked(true);
    scaleFactorLayout->addWidget(m_uniformScaleCheckBox);
    
    // 均匀缩放因子
    QHBoxLayout* uniformScaleLayout = new QHBoxLayout();
    uniformScaleLayout->addWidget(new QLabel("缩放因子:", m_scaleTab));
    m_scaleFactorUniform = new QDoubleSpinBox(m_scaleTab);
    m_scaleFactorUniform->setRange(0.001, 1000.0);
    m_scaleFactorUniform->setDecimals(3);
    m_scaleFactorUniform->setValue(2.0);
    uniformScaleLayout->addWidget(m_scaleFactorUniform);
    uniformScaleLayout->addStretch();
    scaleFactorLayout->addLayout(uniformScaleLayout);
    
    // 非均匀缩放因子
    QGridLayout* nonUniformScaleLayout = new QGridLayout();
    
    nonUniformScaleLayout->addWidget(new QLabel("X因子:", m_scaleTab), 0, 0);
    m_scaleFactorX = new QDoubleSpinBox(m_scaleTab);
    m_scaleFactorX->setRange(0.001, 1000.0);
    m_scaleFactorX->setDecimals(3);
    m_scaleFactorX->setValue(1.0);
    m_scaleFactorX->setEnabled(false);
    nonUniformScaleLayout->addWidget(m_scaleFactorX, 0, 1);
    
    nonUniformScaleLayout->addWidget(new QLabel("Y因子:", m_scaleTab), 1, 0);
    m_scaleFactorY = new QDoubleSpinBox(m_scaleTab);
    m_scaleFactorY->setRange(0.001, 1000.0);
    m_scaleFactorY->setDecimals(3);
    m_scaleFactorY->setValue(1.0);
    m_scaleFactorY->setEnabled(false);
    nonUniformScaleLayout->addWidget(m_scaleFactorY, 1, 1);
    
    nonUniformScaleLayout->addWidget(new QLabel("Z因子:", m_scaleTab), 2, 0);
    m_scaleFactorZ = new QDoubleSpinBox(m_scaleTab);
    m_scaleFactorZ->setRange(0.001, 1000.0);
    m_scaleFactorZ->setDecimals(3);
    m_scaleFactorZ->setValue(1.0);
    m_scaleFactorZ->setEnabled(false);
    nonUniformScaleLayout->addWidget(m_scaleFactorZ, 2, 1);
    
    scaleFactorLayout->addLayout(nonUniformScaleLayout);
    scaleTabLayout->addWidget(m_scaleFactorGroup);
    scaleTabLayout->addStretch();
    
    m_transformTabs->addTab(m_scaleTab, "缩放");
    
    // 连接信号槽
    connect(m_uniformScaleCheckBox, &QCheckBox::toggled, [this](bool checked) {
        m_scaleFactorX->setEnabled(!checked);
        m_scaleFactorY->setEnabled(!checked);
        m_scaleFactorZ->setEnabled(!checked);
        
        if (checked) {
            double factor = m_scaleFactorUniform->value();
            m_scaleFactorX->setValue(factor);
            m_scaleFactorY->setValue(factor);
            m_scaleFactorZ->setValue(factor);
        }
        onScaleParameterChanged();
    });
    
    connect(m_centerAtOriginButton, &QPushButton::clicked, [this]() {
        m_scaleCenterX->setValue(0.0);
        m_scaleCenterY->setValue(0.0);
        m_scaleCenterZ->setValue(0.0);
        onScaleParameterChanged();
    });
    
    connect(m_centerAtBoundingBoxButton, &QPushButton::clicked, [this]() {
        // TODO: 计算选中对象的包围盒中心
        QMessageBox::information(this, "提示", "包围盒中心计算功能待实现");
    });
    
    // 连接缩放参数变化信号
    connect(m_scaleCenterX, QOverload<double>::of(&QDoubleSpinBox::valueChanged),
            this, &TransformOperationDialog::onScaleParameterChanged);
    connect(m_scaleCenterY, QOverload<double>::of(&QDoubleSpinBox::valueChanged),
            this, &TransformOperationDialog::onScaleParameterChanged);
    connect(m_scaleCenterZ, QOverload<double>::of(&QDoubleSpinBox::valueChanged),
            this, &TransformOperationDialog::onScaleParameterChanged);
    connect(m_scaleFactorUniform, QOverload<double>::of(&QDoubleSpinBox::valueChanged),
            this, &TransformOperationDialog::onScaleParameterChanged);
    connect(m_scaleFactorX, QOverload<double>::of(&QDoubleSpinBox::valueChanged),
            this, &TransformOperationDialog::onScaleParameterChanged);
    connect(m_scaleFactorY, QOverload<double>::of(&QDoubleSpinBox::valueChanged),
            this, &TransformOperationDialog::onScaleParameterChanged);
    connect(m_scaleFactorZ, QOverload<double>::of(&QDoubleSpinBox::valueChanged),
            this, &TransformOperationDialog::onScaleParameterChanged);
}

void TransformOperationDialog::onObjectSelectionClicked() {
    if (m_selectingObjects) {
        // 如果正在选择中，这次点击表示完成选择
        onSelectionFinished();
    } else {
        // 开始选择模式
        m_selectingObjects = true;
        m_objectSelectButton->setText("完成");
        m_objectSelectButton->setStyleSheet(
            "QPushButton { background-color: #FF5722; color: white; border: none; border-radius: 4px; font-weight: bold; }"
            "QPushButton:hover { background-color: #E64A19; }"
        );
        
        emit selectionModeChanged(true, "请选择要变换的对象，然后点击'完成'");
    }
}

void TransformOperationDialog::onObjectSelected(const cad_core::ShapePtr& shape) {
    if (!m_selectingObjects || !shape) {
        return;
    }
    
    // 避免重复添加
    for (const auto& existing : m_selectedObjects) {
        if (existing == shape) {
            return;
        }
    }
    
    m_selectedObjects.push_back(shape);
    updateSelectionDisplay();
    
    // 调试信息
    qDebug() << "Transform Dialog: Object selected, total count:" << m_selectedObjects.size();
}

void TransformOperationDialog::onSelectionFinished() {
    if (!m_selectingObjects) {
        return;
    }
    
    m_selectingObjects = false;
    m_objectSelectButton->setText("选择");
    m_objectSelectButton->setStyleSheet(
        "QPushButton { background-color: #009999; color: white; border: none; border-radius: 4px; font-weight: bold; }"
        "QPushButton:hover { background-color: #007777; }"
    );
    
    emit selectionModeChanged(false, "");
    
    bool hasObjects = !m_selectedObjects.empty();
    m_previewButton->setEnabled(hasObjects);
    m_okButton->setEnabled(hasObjects);
    
    // 调试信息
    qDebug() << "Transform Dialog: Selection finished, objects count:" << m_selectedObjects.size() 
             << "Preview enabled:" << hasObjects << "OK enabled:" << hasObjects;
}

void TransformOperationDialog::onPreviewClicked() {
    if (m_previewActive) {
        // 取消预览
        emit resetRequested();
        m_previewButton->setText("预览");
        m_previewActive = false;
    } else {
        // 显示预览
        auto command = getCurrentTransformCommand();
        if (command) {
            emit previewRequested(command);
            m_previewButton->setText("取消预览");
            m_previewActive = true;
        }
    }
}

void TransformOperationDialog::onResetClicked() {
    if (m_previewActive) {
        emit resetRequested();
        m_previewButton->setText("预览");
        m_previewActive = false;
    }
    
    resetAllParameters();
}

void TransformOperationDialog::onTransformTypeChanged() {
    if (m_previewActive) {
        updatePreview();
    }
}

void TransformOperationDialog::onTranslationParameterChanged() {
    if (m_previewActive) {
        updatePreview();
    }
}

void TransformOperationDialog::onRotationParameterChanged() {
    if (m_previewActive) {
        updatePreview();
    }
}

void TransformOperationDialog::onScaleParameterChanged() {
    if (m_previewActive) {
        updatePreview();
    }
}

void TransformOperationDialog::updatePreview() {
    if (!m_previewActive) {
        return;
    }
    
    auto command = getCurrentTransformCommand();
    if (command) {
        emit previewRequested(command);
    }
}

void TransformOperationDialog::updateSelectionDisplay() {
    m_objectList->clear();
    
    if (m_selectedObjects.empty()) {
        m_objectCount->setText("请选择要变换的对象");
        m_objectCount->setStyleSheet("color: #666666; font-style: italic;");
    } else {
        m_objectCount->setText(QString("已选择 %1 个对象").arg(m_selectedObjects.size()));
        m_objectCount->setStyleSheet("color: #009999; font-weight: bold;");
        
        for (size_t i = 0; i < m_selectedObjects.size(); ++i) {
            m_objectList->addItem(QString("对象 %1").arg(i + 1));
        }
    }
}

void TransformOperationDialog::resetAllParameters() {
    // 重置平移参数
    m_translateX->setValue(0.0);
    m_translateY->setValue(0.0);
    m_translateZ->setValue(0.0);
    
    // 重置旋转参数
    m_axisPointX->setValue(0.0);
    m_axisPointY->setValue(0.0);
    m_axisPointZ->setValue(0.0);
    m_axisZButton->setChecked(true);
    m_axisDirectionX->setValue(0.0);
    m_axisDirectionY->setValue(0.0);
    m_axisDirectionZ->setValue(1.0);
    m_rotationAngle->setValue(0.0);
    m_angleDegrees->setChecked(true);
    
    // 重置缩放参数
    m_scaleCenterX->setValue(0.0);
    m_scaleCenterY->setValue(0.0);
    m_scaleCenterZ->setValue(0.0);
    m_uniformScaleCheckBox->setChecked(true);
    m_scaleFactorUniform->setValue(1.0);
    m_scaleFactorX->setValue(1.0);
    m_scaleFactorY->setValue(1.0);
    m_scaleFactorZ->setValue(1.0);
}

std::shared_ptr<cad_core::TransformCommand> TransformOperationDialog::getCurrentTransformCommand() const {
    if (m_selectedObjects.empty()) {
        return nullptr;
    }
    
    int currentTab = m_transformTabs->currentIndex();
    
    switch (currentTab) {
        case 0: { // 平移
            cad_core::Point translation(
                m_translateX->value(),
                m_translateY->value(),
                m_translateZ->value()
            );
            return std::make_shared<cad_core::TranslateCommand>(m_selectedObjects, translation);
        }
        
        case 1: { // 旋转
            cad_core::Point axisPoint(
                m_axisPointX->value(),
                m_axisPointY->value(),
                m_axisPointZ->value()
            );
            cad_core::Point axisDirection(
                m_axisDirectionX->value(),
                m_axisDirectionY->value(),
                m_axisDirectionZ->value()
            );
            
            double angle = m_rotationAngle->value();
            if (m_angleDegrees->isChecked()) {
                angle = angle * M_PI / 180.0; // 转换为弧度
            }
            
            return std::make_shared<cad_core::RotateCommand>(m_selectedObjects, axisPoint, axisDirection, angle);
        }
        
        case 2: { // 缩放
            cad_core::Point center(
                m_scaleCenterX->value(),
                m_scaleCenterY->value(),
                m_scaleCenterZ->value()
            );
            
            if (m_uniformScaleCheckBox->isChecked()) {
                return std::make_shared<cad_core::ScaleCommand>(m_selectedObjects, center, m_scaleFactorUniform->value());
            } else {
                return std::make_shared<cad_core::ScaleCommand>(m_selectedObjects, center, 
                    m_scaleFactorX->value(), m_scaleFactorY->value(), m_scaleFactorZ->value());
            }
        }
        
        default:
            return nullptr;
    }
}

void TransformOperationDialog::accept() {
    auto command = getCurrentTransformCommand();
    if (command) {
        emit transformRequested(command);
        QDialog::accept();
    } else {
        QMessageBox::warning(this, "警告", "请选择对象并设置有效的变换参数");
    }
}

void TransformOperationDialog::reject() {
    if (m_previewActive) {
        emit resetRequested();
    }
    QDialog::reject();
}

} // namespace cad_ui