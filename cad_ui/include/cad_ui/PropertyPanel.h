#pragma once

#include <QWidget>
#include <QVBoxLayout>
#include <QFormLayout>
#include <QLabel>
#include <QLineEdit>
#include <QDoubleSpinBox>
#include <QGroupBox>
#include <QScrollArea>
#include "cad_core/Shape.h"
#include "cad_feature/Feature.h"

namespace cad_ui {

class PropertyPanel : public QWidget {
    Q_OBJECT

public:
    explicit PropertyPanel(QWidget* parent = nullptr);
    ~PropertyPanel() = default;

    void SetShape(const cad_core::ShapePtr& shape);
    void SetFeature(const cad_feature::FeaturePtr& feature);
    void Clear();

private:
    QVBoxLayout* m_mainLayout;
    QScrollArea* m_scrollArea;
    QWidget* m_contentWidget;
    QVBoxLayout* m_contentLayout;
    
    cad_core::ShapePtr m_currentShape;
    cad_feature::FeaturePtr m_currentFeature;
    
    void CreateShapeProperties();
    void CreateFeatureProperties();
    void ClearProperties();
    
    void AddProperty(const QString& name, const QString& value);
    void AddProperty(const QString& name, double value);
    void AddGroupBox(const QString& title);
};

} // namespace cad_ui