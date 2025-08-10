#include "cad_ui/PropertyPanel.h"
#pragma execution_character_set("utf-8")

namespace cad_ui {

PropertyPanel::PropertyPanel(QWidget* parent) : QWidget(parent) {
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
    
    // Initialize with empty state
    Clear();
}

void PropertyPanel::SetShape(const cad_core::ShapePtr& shape) {
    m_currentShape = shape;
    m_currentFeature.reset();
    
    ClearProperties();
    CreateShapeProperties();
}

void PropertyPanel::SetFeature(const cad_feature::FeaturePtr& feature) {
    m_currentFeature = feature;
    m_currentShape.reset();
    
    ClearProperties();
    CreateFeatureProperties();
}

void PropertyPanel::Clear() {
    m_currentShape.reset();
    m_currentFeature.reset();
    ClearProperties();
    
    QLabel* label = new QLabel("No selection");
    label->setAlignment(Qt::AlignCenter);
    m_contentLayout->addWidget(label);
    m_contentLayout->addStretch();
}

void PropertyPanel::CreateShapeProperties() {
    if (!m_currentShape) {
        return;
    }
    
    AddGroupBox("Shape Properties");
    
    AddProperty("Type", "Shape");
    AddProperty("Valid", m_currentShape->IsValid() ? "Yes" : "No");
    
    if (m_currentShape->IsValid()) {
        AddProperty("Volume", m_currentShape->Volume());
        AddProperty("Area", m_currentShape->Area());
    }
    
    // Add stretch at the end
    m_contentLayout->addStretch();
}

void PropertyPanel::CreateFeatureProperties() {
    if (!m_currentFeature) {
        return;
    }
    
    AddGroupBox("Feature Properties");
    
    AddProperty("Name", QString::fromStdString(m_currentFeature->GetName()));
    AddProperty("ID", QString::number(m_currentFeature->GetId()));
    AddProperty("Active", m_currentFeature->IsActive() ? "Yes" : "No");
    
    QString stateText;
    switch (m_currentFeature->GetState()) {
        case cad_feature::FeatureState::Created:
            stateText = "Created";
            break;
        case cad_feature::FeatureState::Previewing:
            stateText = "Previewing";
            break;
        case cad_feature::FeatureState::Executed:
            stateText = "Executed";
            break;
        case cad_feature::FeatureState::Failed:
            stateText = "Failed";
            break;
    }
    AddProperty("State", stateText);
    
    QString typeText;
    switch (m_currentFeature->GetType()) {
        case cad_feature::FeatureType::Extrude:
            typeText = "Extrude";
            break;
        case cad_feature::FeatureType::Revolve:
            typeText = "Revolve";
            break;
        case cad_feature::FeatureType::Sweep:
            typeText = "Sweep";
            break;
        case cad_feature::FeatureType::Loft:
            typeText = "Loft";
            break;
        default:
            typeText = "Unknown";
            break;
    }
    AddProperty("Type", typeText);
    
    // Add stretch at the end
    m_contentLayout->addStretch();
}

void PropertyPanel::ClearProperties() {
    QLayoutItem* item;
    while ((item = m_contentLayout->takeAt(0)) != nullptr) {
        delete item->widget();
        delete item;
    }
}

void PropertyPanel::AddProperty(const QString& name, const QString& value) {
    QWidget* widget = new QWidget();
    QFormLayout* layout = new QFormLayout(widget);
    
    QLabel* valueLabel = new QLabel(value);
    valueLabel->setWordWrap(true);
    
    layout->addRow(name + ":", valueLabel);
    layout->setContentsMargins(0, 0, 0, 0);
    
    m_contentLayout->addWidget(widget);
}

void PropertyPanel::AddProperty(const QString& name, double value) {
    AddProperty(name, QString::number(value, 'f', 3));
}

void PropertyPanel::AddGroupBox(const QString& title) {
    QGroupBox* groupBox = new QGroupBox(title);
    m_contentLayout->addWidget(groupBox);
}

} // namespace cad_ui

#include "PropertyPanel.moc"