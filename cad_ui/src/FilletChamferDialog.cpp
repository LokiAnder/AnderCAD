#include "cad_ui/FilletChamferDialog.h"
#include "cad_ui/QtOccView.h"
#include <QApplication>
#include <QMessageBox>
#pragma execution_character_set("utf-8")

namespace cad_ui {

FilletChamferDialog::FilletChamferDialog(FilletChamferType operationType, QtOccView* viewer, QWidget* parent)
    : QDialog(parent), m_operationType(operationType), m_viewer(viewer), m_selectingEdges(false) {
    setupUI();
    setModal(false); // Allow interaction with main window for selection
    setWindowFlags(Qt::Dialog | Qt::WindowStaysOnTopHint);
    resize(380, 450);
    
    // Initial sync with viewer edge selection
    syncWithViewerEdgeSelection();
    updateSelectionDisplay();
}

void FilletChamferDialog::setupUI() {
    setWindowTitle(getOperationTitle());
    
    m_mainLayout = new QVBoxLayout(this);
    m_mainLayout->setSpacing(8);
    m_mainLayout->setContentsMargins(12, 12, 12, 12);
    
    // Edge selection group
    m_selectionGroup = new QGroupBox("边选择", this);
    m_selectionLayout = new QGridLayout(m_selectionGroup);
    m_selectionLayout->setSpacing(8);
    
    m_edgeLabel = new QLabel("选择要处理的边:", this);
    m_edgeLabel->setStyleSheet("font-weight: bold; color: #333333;");
    m_selectionLayout->addWidget(m_edgeLabel, 0, 0, 1, 2);
    
    // Edge selection frame (UGNX style)
    m_edgeFrame = new QFrame(this);
    m_edgeFrame->setFrameStyle(QFrame::StyledPanel | QFrame::Sunken);
    m_edgeFrame->setMinimumHeight(40);
    m_edgeFrame->setStyleSheet(
        "QFrame { background-color: #F8F8F8; border: 2px solid #E0E0E0; border-radius: 4px; }"
        "QFrame:hover { border-color: #009999; }"
    );
    
    m_edgeFrameLayout = new QHBoxLayout(m_edgeFrame);
    m_edgeFrameLayout->setContentsMargins(8, 4, 8, 4);
    
    m_edgeCount = new QLabel("请选择边", this);
    m_edgeCount->setStyleSheet("color: #666666; font-style: italic;");
    m_edgeFrameLayout->addWidget(m_edgeCount);
    m_edgeFrameLayout->addStretch();
    
    m_edgeSelectButton = new QPushButton("选择", this);
    m_edgeSelectButton->setMinimumSize(60, 24);
    m_edgeSelectButton->setStyleSheet(
        "QPushButton { background-color: #009999; color: white; border: none; border-radius: 3px; font-weight: bold; }"
        "QPushButton:hover { background-color: #00CCCC; }"
        "QPushButton:pressed { background-color: #007777; }"
    );
    m_edgeFrameLayout->addWidget(m_edgeSelectButton);
    
    m_selectionLayout->addWidget(m_edgeFrame, 1, 0, 1, 2);
    
    // Edge list widget
    m_edgeList = new QListWidget(this);
    m_edgeList->setMaximumHeight(80);
    m_edgeList->setVisible(false);
    m_edgeList->setStyleSheet(
        "QListWidget { border: 1px solid #E0E0E0; border-radius: 3px; background-color: white; }"
        "QListWidget::item { padding: 4px; border-bottom: 1px solid #F0F0F0; }"
        "QListWidget::item:selected { background-color: #009999; color: white; }"
    );
    m_selectionLayout->addWidget(m_edgeList, 2, 0, 1, 2);
    
    m_mainLayout->addWidget(m_selectionGroup);
    
    // Parameters group
    m_parametersGroup = new QGroupBox("参数设置", this);
    m_parametersLayout = new QGridLayout(m_parametersGroup);
    m_parametersLayout->setSpacing(8);
    
    int row = 0;
    
    if (m_operationType == FilletChamferType::Fillet) {
        // Fillet radius
        m_radiusLabel = new QLabel("圆角半径:", this);
        m_radiusLabel->setStyleSheet("font-weight: bold; color: #333333;");
        m_parametersLayout->addWidget(m_radiusLabel, row, 0);
        
        m_radiusSpinBox = new QDoubleSpinBox(this);
        m_radiusSpinBox->setRange(0.1, 1000.0);
        m_radiusSpinBox->setValue(1.0);
        m_radiusSpinBox->setDecimals(2);
        m_radiusSpinBox->setSuffix(" mm");
        m_radiusSpinBox->setStyleSheet(
            "QDoubleSpinBox { border: 2px solid #E0E0E0; border-radius: 4px; padding: 4px; }"
            "QDoubleSpinBox:focus { border-color: #009999; }"
        );
        m_parametersLayout->addWidget(m_radiusSpinBox, row, 1);
        
        // Hide chamfer controls
        m_chamferDistance1Label = nullptr;
        m_chamferDistance1SpinBox = nullptr;
        m_chamferDistance2Label = nullptr;
        m_chamferDistance2SpinBox = nullptr;
        m_symmetricCheckBox = nullptr;
        
    } else { // Chamfer
        // Symmetric chamfer checkbox
        m_symmetricCheckBox = new QCheckBox("对称倒角", this);
        m_symmetricCheckBox->setChecked(true);
        m_symmetricCheckBox->setStyleSheet("font-weight: bold; color: #333333;");
        m_parametersLayout->addWidget(m_symmetricCheckBox, row++, 0, 1, 2);
        
        // Distance 1
        m_chamferDistance1Label = new QLabel("距离1:", this);
        m_chamferDistance1Label->setStyleSheet("font-weight: bold; color: #333333;");
        m_parametersLayout->addWidget(m_chamferDistance1Label, row, 0);
        
        m_chamferDistance1SpinBox = new QDoubleSpinBox(this);
        m_chamferDistance1SpinBox->setRange(0.1, 1000.0);
        m_chamferDistance1SpinBox->setValue(1.0);
        m_chamferDistance1SpinBox->setDecimals(2);
        m_chamferDistance1SpinBox->setSuffix(" mm");
        m_chamferDistance1SpinBox->setStyleSheet(
            "QDoubleSpinBox { border: 2px solid #E0E0E0; border-radius: 4px; padding: 4px; }"
            "QDoubleSpinBox:focus { border-color: #009999; }"
        );
        m_parametersLayout->addWidget(m_chamferDistance1SpinBox, row++, 1);
        
        // Distance 2 (only for asymmetric)
        m_chamferDistance2Label = new QLabel("距离2:", this);
        m_chamferDistance2Label->setStyleSheet("font-weight: bold; color: #333333;");
        m_parametersLayout->addWidget(m_chamferDistance2Label, row, 0);
        
        m_chamferDistance2SpinBox = new QDoubleSpinBox(this);
        m_chamferDistance2SpinBox->setRange(0.1, 1000.0);
        m_chamferDistance2SpinBox->setValue(1.0);
        m_chamferDistance2SpinBox->setDecimals(2);
        m_chamferDistance2SpinBox->setSuffix(" mm");
        m_chamferDistance2SpinBox->setStyleSheet(
            "QDoubleSpinBox { border: 2px solid #E0E0E0; border-radius: 4px; padding: 4px; }"
            "QDoubleSpinBox:focus { border-color: #009999; }"
        );
        m_parametersLayout->addWidget(m_chamferDistance2SpinBox, row, 1);
        
        // Hide radius control
        m_radiusLabel = nullptr;
        m_radiusSpinBox = nullptr;
        
        // Connect symmetric checkbox
        connect(m_symmetricCheckBox, &QCheckBox::toggled, this, &FilletChamferDialog::onSymmetricChanged);
        onSymmetricChanged(true); // Initialize visibility
    }
    
    m_mainLayout->addWidget(m_parametersGroup);
    m_mainLayout->addStretch();
    
    // Control buttons
    m_buttonLayout = new QHBoxLayout();
    m_buttonLayout->setSpacing(8);
    
    m_previewButton = new QPushButton("预览", this);
    m_previewButton->setMinimumSize(80, 32);
    m_previewButton->setEnabled(false);
    
    m_buttonLayout->addWidget(m_previewButton);
    m_buttonLayout->addStretch();
    
    m_cancelButton = new QPushButton("取消", this);
    m_cancelButton->setMinimumSize(80, 32);
    
    m_okButton = new QPushButton("确定", this);
    m_okButton->setMinimumSize(80, 32);
    m_okButton->setDefault(true);
    m_okButton->setEnabled(false);
    
    m_buttonLayout->addWidget(m_cancelButton);
    m_buttonLayout->addWidget(m_okButton);
    
    m_mainLayout->addLayout(m_buttonLayout);
    
    // Connect signals
    connect(m_edgeSelectButton, &QPushButton::clicked, this, &FilletChamferDialog::onEdgeSelectionClicked);
    
    connect(m_okButton, &QPushButton::clicked, [this]() {
        double radius = m_radiusSpinBox ? m_radiusSpinBox->value() : 0.0;
        double distance1 = m_chamferDistance1SpinBox ? m_chamferDistance1SpinBox->value() : 0.0;
        double distance2 = m_chamferDistance2SpinBox ? m_chamferDistance2SpinBox->value() : distance1;
        emit operationRequested(m_operationType, m_selectedEdges, radius, distance1, distance2);
        accept();
    });
    
    connect(m_cancelButton, &QPushButton::clicked, this, &QDialog::reject);
    
    // Connect parameter change signals
    if (m_radiusSpinBox) {
        connect(m_radiusSpinBox, QOverload<double>::of(&QDoubleSpinBox::valueChanged), 
                this, &FilletChamferDialog::onParameterChanged);
    }
    if (m_chamferDistance1SpinBox) {
        connect(m_chamferDistance1SpinBox, QOverload<double>::of(&QDoubleSpinBox::valueChanged), 
                this, &FilletChamferDialog::onParameterChanged);
    }
    if (m_chamferDistance2SpinBox) {
        connect(m_chamferDistance2SpinBox, QOverload<double>::of(&QDoubleSpinBox::valueChanged), 
                this, &FilletChamferDialog::onParameterChanged);
    }
    
    // Style the dialog
    setStyleSheet(
        "QDialog { background-color: #F5F5F5; }"
        "QGroupBox { font-weight: bold; color: #333333; border: 1px solid #E0E0E0; border-radius: 6px; margin-top: 6px; }"
        "QGroupBox::title { subcontrol-origin: margin; left: 8px; padding: 0 4px 0 4px; }"
        "QPushButton { background-color: #009999; color: white; border: none; border-radius: 4px; padding: 6px 12px; font-weight: bold; }"
        "QPushButton:hover { background-color: #00CCCC; }"
        "QPushButton:pressed { background-color: #007777; }"
        "QPushButton:disabled { background-color: #CCCCCC; color: #999999; }"
        "QCheckBox { font-weight: bold; color: #333333; }"
        "QCheckBox::indicator { width: 16px; height: 16px; border: 2px solid #E0E0E0; border-radius: 3px; }"
        "QCheckBox::indicator:checked { background-color: #009999; border-color: #007777; }"
    );
}

void FilletChamferDialog::onEdgeSelectionClicked() {
    m_selectingEdges = true;
    
    m_edgeSelectButton->setText("完成");
    m_edgeSelectButton->setStyleSheet(
        "QPushButton { background-color: #FF6B35; color: white; border: none; border-radius: 3px; font-weight: bold; }"
        "QPushButton:hover { background-color: #FF8C5A; }"
    );
    
    QString prompt = QString("请在3D视图中选择要进行%1的边，完成后点击'完成'按钮")
                    .arg(m_operationType == FilletChamferType::Fillet ? "圆角" : "倒角");
    
    emit selectionModeChanged(true, prompt);
    
    // Change button to finish selection
    disconnect(m_edgeSelectButton, &QPushButton::clicked, this, &FilletChamferDialog::onEdgeSelectionClicked);
    connect(m_edgeSelectButton, &QPushButton::clicked, this, &FilletChamferDialog::onSelectionFinished);
}

void FilletChamferDialog::onEdgeSelected(const cad_core::ShapePtr& edge) {
    if (m_selectingEdges) {
        m_selectedEdges.push_back(edge);
        updateSelectionDisplay();
    }
}

void FilletChamferDialog::onSelectionFinished() {
    m_selectingEdges = false;
    
    // Reset button
    m_edgeSelectButton->setText("选择");
    m_edgeSelectButton->setStyleSheet(
        "QPushButton { background-color: #009999; color: white; border: none; border-radius: 3px; font-weight: bold; }"
        "QPushButton:hover { background-color: #00CCCC; }"
        "QPushButton:pressed { background-color: #007777; }"
    );
    
    // Reconnect original handler
    disconnect(m_edgeSelectButton, &QPushButton::clicked, this, &FilletChamferDialog::onSelectionFinished);
    connect(m_edgeSelectButton, &QPushButton::clicked, this, &FilletChamferDialog::onEdgeSelectionClicked);
    
    emit selectionModeChanged(false, "");
    
    // Sync with viewer's edge selection
    syncWithViewerEdgeSelection();
    updateSelectionDisplay();
}

void FilletChamferDialog::onSymmetricChanged(bool symmetric) {
    if (m_chamferDistance2Label && m_chamferDistance2SpinBox) {
        m_chamferDistance2Label->setVisible(!symmetric);
        m_chamferDistance2SpinBox->setVisible(!symmetric);
        
        if (symmetric) {
            m_chamferDistance1Label->setText("倒角距离:");
        } else {
            m_chamferDistance1Label->setText("距离1:");
        }
    }
}

void FilletChamferDialog::onParameterChanged() {
    // Enable preview when we have edges and valid parameters
    bool hasEdges = !m_selectedEdges.empty();
    bool hasValidParams = true;
    
    if (m_radiusSpinBox) {
        hasValidParams = m_radiusSpinBox->value() > 0;
    } else if (m_chamferDistance1SpinBox) {
        hasValidParams = m_chamferDistance1SpinBox->value() > 0;
        if (!m_symmetricCheckBox->isChecked() && m_chamferDistance2SpinBox) {
            hasValidParams = hasValidParams && m_chamferDistance2SpinBox->value() > 0;
        }
    }
    
    m_previewButton->setEnabled(hasEdges && hasValidParams);
    m_okButton->setEnabled(hasEdges && hasValidParams);
}

void FilletChamferDialog::updateSelectionDisplay() {
    if (!m_selectedEdges.empty()) {
        m_edgeCount->setText(QString("已选择 %1 条边").arg(m_selectedEdges.size()));
        m_edgeCount->setStyleSheet("color: #009999; font-weight: bold;");
        m_edgeList->setVisible(true);
        
        m_edgeList->clear();
        for (size_t i = 0; i < m_selectedEdges.size(); ++i) {
            m_edgeList->addItem(QString("边 %1").arg(i + 1));
        }
    } else {
        m_edgeCount->setText("请选择边");
        m_edgeCount->setStyleSheet("color: #666666; font-style: italic;");
        m_edgeList->setVisible(false);
    }
    
    onParameterChanged(); // Update button states
}

QString FilletChamferDialog::getOperationTitle() const {
    return m_operationType == FilletChamferType::Fillet ? "圆角操作" : "倒角操作";
}

void FilletChamferDialog::syncWithViewerEdgeSelection() {
    if (!m_viewer) return;
    
    // Get selected edges from viewer
    auto topoEdges = m_viewer->GetSelectedTopoEdges();
    
    // Clear current selection in dialog
    m_selectedEdges.clear();
    
    // Convert TopoDS_Edge to cad_core::ShapePtr (simplified approach)
    // In a real implementation, you'd want to properly track edge-to-shape mapping
    for (size_t i = 0; i < topoEdges.size(); ++i) {
        // Create a placeholder ShapePtr for each edge
        // This is a simplified approach - in production code you'd want proper edge tracking
        auto edgeShape = std::make_shared<cad_core::Shape>();
        m_selectedEdges.push_back(edgeShape);
    }
}

} // namespace cad_ui