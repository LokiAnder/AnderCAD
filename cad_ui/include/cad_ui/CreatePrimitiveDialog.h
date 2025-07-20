#pragma once

#include <QDialog>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QFormLayout>
#include <QDoubleSpinBox>
#include <QLabel>
#include <QPushButton>
#include <QGroupBox>
#include <QDialogButtonBox>

namespace cad_ui {

class CreateBoxDialog : public QDialog {
    Q_OBJECT

public:
    explicit CreateBoxDialog(QWidget* parent = nullptr);
    ~CreateBoxDialog() = default;

    double GetWidth() const;
    double GetHeight() const;
    double GetDepth() const;

private:
    QDoubleSpinBox* m_widthSpinBox;
    QDoubleSpinBox* m_heightSpinBox;
    QDoubleSpinBox* m_depthSpinBox;
    
    void SetupUI();
};

class CreateCylinderDialog : public QDialog {
    Q_OBJECT

public:
    explicit CreateCylinderDialog(QWidget* parent = nullptr);
    ~CreateCylinderDialog() = default;

    double GetRadius() const;
    double GetHeight() const;

private:
    QDoubleSpinBox* m_radiusSpinBox;
    QDoubleSpinBox* m_heightSpinBox;
    
    void SetupUI();
};

class CreateSphereDialog : public QDialog {
    Q_OBJECT

public:
    explicit CreateSphereDialog(QWidget* parent = nullptr);
    ~CreateSphereDialog() = default;

    double GetRadius() const;

private:
    QDoubleSpinBox* m_radiusSpinBox;
    
    void SetupUI();
};

} // namespace cad_ui