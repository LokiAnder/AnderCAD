#pragma once

#include "Feature.h"
#include <QWidget>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QLabel>
#include <QDoubleSpinBox>
#include <QSpinBox>
#include <QCheckBox>
#include <QGroupBox>
#include <QScrollArea>
#include <functional>

namespace cad_feature {

class ParameterPanel : public QWidget {
    Q_OBJECT

public:
    explicit ParameterPanel(QWidget* parent = nullptr);
    ~ParameterPanel() = default;

    void SetFeature(const FeaturePtr& feature);
    const FeaturePtr& GetFeature() const;
    
    void UpdateParameters();
    void ClearParameters();
    
    // Callbacks
    void SetParameterChangedCallback(std::function<void(const std::string&, double)> callback);

private slots:
    void OnParameterChanged();

private:
    FeaturePtr m_feature;
    
    QVBoxLayout* m_mainLayout;
    QScrollArea* m_scrollArea;
    QWidget* m_contentWidget;
    QVBoxLayout* m_contentLayout;
    
    std::function<void(const std::string&, double)> m_parameterChangedCallback;
    
    void CreateParameterWidgets();
    void CreateDoubleParameter(const std::string& name, double value, double min = -1000.0, double max = 1000.0);
    void CreateIntParameter(const std::string& name, int value, int min = -1000, int max = 1000);
    void CreateBoolParameter(const std::string& name, bool value);
    void CreateGroupBox(const std::string& title);
    
    void NotifyParameterChanged(const std::string& name, double value);
};

} // namespace cad_feature