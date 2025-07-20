#include "cad_feature/ParameterPanel.h"
#include "cad_feature/ExtrudeFeature.h"
#include "cad_feature/RevolveFeature.h"

namespace cad_feature {

ParameterPanel::ParameterPanel(QWidget* parent) : QWidget(parent) {
    m_mainLayout = new QVBoxLayout(this);
    
    m_scrollArea = new QScrollArea(this);
    m_scrollArea->setWidgetResizable(true);
    m_scrollArea->setHorizontalScrollBarPolicy(Qt::ScrollBarAsNeeded);
    m_scrollArea->setVerticalScrollBarPolicy(Qt::ScrollBarAsNeeded);
    
    m_contentWidget = new QWidget();
    m_contentLayout = new QVBoxLayout(m_contentWidget);
    
    m_scrollArea->setWidget(m_contentWidget);
    m_mainLayout->addWidget(m_scrollArea);
    
    setLayout(m_mainLayout);
}

void ParameterPanel::SetFeature(const FeaturePtr& feature) {
    m_feature = feature;
    UpdateParameters();
}

const FeaturePtr& ParameterPanel::GetFeature() const {
    return m_feature;
}

void ParameterPanel::UpdateParameters() {
    ClearParameters();
    
    if (!m_feature) {
        return;
    }
    
    CreateParameterWidgets();
}

void ParameterPanel::ClearParameters() {
    QLayoutItem* item;
    while ((item = m_contentLayout->takeAt(0)) != nullptr) {
        delete item->widget();
        delete item;
    }
}

void ParameterPanel::SetParameterChangedCallback(std::function<void(const std::string&, double)> callback) {
    m_parameterChangedCallback = callback;
}

void ParameterPanel::OnParameterChanged() {
    // 这个槽将连接到参数控件
    // 实现取决于特定的控件类型
}

void ParameterPanel::CreateParameterWidgets() {
    if (!m_feature) {
        return;
    }
    
    // 根据特征类型创建参数控件
    switch (m_feature->GetType()) {
        case FeatureType::Extrude: {
            CreateGroupBox("拉伸参数");
            CreateDoubleParameter("distance", m_feature->GetParameter("distance"), 0.1, 1000.0);
            CreateDoubleParameter("direction_x", m_feature->GetParameter("direction_x"), -1.0, 1.0);
            CreateDoubleParameter("direction_y", m_feature->GetParameter("direction_y"), -1.0, 1.0);
            CreateDoubleParameter("direction_z", m_feature->GetParameter("direction_z"), -1.0, 1.0);
            CreateDoubleParameter("taper_angle", m_feature->GetParameter("taper_angle"), -90.0, 90.0);
            CreateBoolParameter("midplane", m_feature->GetParameter("midplane") != 0.0);
            break;
        }
        case FeatureType::Revolve: {
            CreateGroupBox("旋转参数");
            CreateDoubleParameter("angle", m_feature->GetParameter("angle"), 0.1, 360.0);
            CreateDoubleParameter("axis_x", m_feature->GetParameter("axis_x"), -1.0, 1.0);
            CreateDoubleParameter("axis_y", m_feature->GetParameter("axis_y"), -1.0, 1.0);
            CreateDoubleParameter("axis_z", m_feature->GetParameter("axis_z"), -1.0, 1.0);
            CreateBoolParameter("midplane", m_feature->GetParameter("midplane") != 0.0);
            break;
        }
        case FeatureType::Sweep: {
            CreateGroupBox("扫描参数");
            CreateDoubleParameter("twist_angle", m_feature->GetParameter("twist_angle"), -360.0, 360.0);
            CreateDoubleParameter("scale_factor", m_feature->GetParameter("scale_factor"), 0.1, 10.0);
            CreateBoolParameter("keep_orientation", m_feature->GetParameter("keep_orientation") != 0.0);
            break;
        }
        case FeatureType::Loft: {
            CreateGroupBox("放样参数");
            CreateBoolParameter("solid", m_feature->GetParameter("solid") != 0.0);
            CreateBoolParameter("ruled", m_feature->GetParameter("ruled") != 0.0);
            CreateBoolParameter("closed", m_feature->GetParameter("closed") != 0.0);
            break;
        }
        default:
            break;
    }
    
    // 在末尾添加伸缩
    m_contentLayout->addStretch();
}

void ParameterPanel::CreateDoubleParameter(const std::string& name, double value, double min, double max) {
    QHBoxLayout* layout = new QHBoxLayout();
    
    QLabel* label = new QLabel(QString::fromStdString(name));
    label->setFixedWidth(100);
    
    QDoubleSpinBox* spinBox = new QDoubleSpinBox();
    spinBox->setRange(min, max);
    spinBox->setValue(value);
    spinBox->setDecimals(3);
    spinBox->setSingleStep(0.1);
    
    // 连接到参数变更回调
    connect(spinBox, QOverload<double>::of(&QDoubleSpinBox::valueChanged), 
            [this, name](double value) {
                if (m_feature) {
                    m_feature->SetParameter(name, value);
                    NotifyParameterChanged(name, value);
                }
            });
    
    layout->addWidget(label);
    layout->addWidget(spinBox);
    
    m_contentLayout->addLayout(layout);
}

void ParameterPanel::CreateIntParameter(const std::string& name, int value, int min, int max) {
    QHBoxLayout* layout = new QHBoxLayout();
    
    QLabel* label = new QLabel(QString::fromStdString(name));
    label->setFixedWidth(100);
    
    QSpinBox* spinBox = new QSpinBox();
    spinBox->setRange(min, max);
    spinBox->setValue(value);
    
    // 连接到参数变更回调
    connect(spinBox, QOverload<int>::of(&QSpinBox::valueChanged), 
            [this, name](int value) {
                if (m_feature) {
                    m_feature->SetParameter(name, static_cast<double>(value));
                    NotifyParameterChanged(name, static_cast<double>(value));
                }
            });
    
    layout->addWidget(label);
    layout->addWidget(spinBox);
    
    m_contentLayout->addLayout(layout);
}

void ParameterPanel::CreateBoolParameter(const std::string& name, bool value) {
    QCheckBox* checkBox = new QCheckBox(QString::fromStdString(name));
    checkBox->setChecked(value);
    
    // 连接到参数变更回调
    connect(checkBox, &QCheckBox::toggled, 
            [this, name](bool checked) {
                if (m_feature) {
                    m_feature->SetParameter(name, checked ? 1.0 : 0.0);
                    NotifyParameterChanged(name, checked ? 1.0 : 0.0);
                }
            });
    
    m_contentLayout->addWidget(checkBox);
}

void ParameterPanel::CreateGroupBox(const std::string& title) {
    QGroupBox* groupBox = new QGroupBox(QString::fromStdString(title));
    m_contentLayout->addWidget(groupBox);
}

void ParameterPanel::NotifyParameterChanged(const std::string& name, double value) {
    if (m_parameterChangedCallback) {
        m_parameterChangedCallback(name, value);
    }
}

} // namespace cad_feature

#include "ParameterPanel.moc"