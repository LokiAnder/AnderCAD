#include "cad_ui/CreatePrimitiveDialog.h"
#include <QApplication>
#pragma execution_character_set("utf-8")

namespace cad_ui {

// CreateBoxDialog Implementation
CreateBoxDialog::CreateBoxDialog(QWidget* parent) : QDialog(parent) {
    setWindowTitle("Create Box");
    setModal(true);
    resize(300, 200);
    
    SetupUI();
}

void CreateBoxDialog::SetupUI() {
    QVBoxLayout* mainLayout = new QVBoxLayout(this);
    
    // Parameters group
    QGroupBox* paramGroup = new QGroupBox("Box Parameters");
    QFormLayout* formLayout = new QFormLayout(paramGroup);
    
    // Width
    m_widthSpinBox = new QDoubleSpinBox();
    m_widthSpinBox->setRange(0.1, 1000.0);
    m_widthSpinBox->setValue(10.0);
    m_widthSpinBox->setDecimals(3);
    m_widthSpinBox->setSuffix(" mm");
    formLayout->addRow("Width:", m_widthSpinBox);
    
    // Height
    m_heightSpinBox = new QDoubleSpinBox();
    m_heightSpinBox->setRange(0.1, 1000.0);
    m_heightSpinBox->setValue(10.0);
    m_heightSpinBox->setDecimals(3);
    m_heightSpinBox->setSuffix(" mm");
    formLayout->addRow("Height:", m_heightSpinBox);
    
    // Depth
    m_depthSpinBox = new QDoubleSpinBox();
    m_depthSpinBox->setRange(0.1, 1000.0);
    m_depthSpinBox->setValue(10.0);
    m_depthSpinBox->setDecimals(3);
    m_depthSpinBox->setSuffix(" mm");
    formLayout->addRow("Depth:", m_depthSpinBox);
    
    // Button box
    QDialogButtonBox* buttonBox = new QDialogButtonBox(QDialogButtonBox::Ok | QDialogButtonBox::Cancel);
    connect(buttonBox, &QDialogButtonBox::accepted, this, &QDialog::accept);
    connect(buttonBox, &QDialogButtonBox::rejected, this, &QDialog::reject);
    
    mainLayout->addWidget(paramGroup);
    mainLayout->addWidget(buttonBox);
    
    setLayout(mainLayout);
}

double CreateBoxDialog::GetWidth() const {
    return m_widthSpinBox->value();
}

double CreateBoxDialog::GetHeight() const {
    return m_heightSpinBox->value();
}

double CreateBoxDialog::GetDepth() const {
    return m_depthSpinBox->value();
}

// CreateCylinderDialog Implementation
CreateCylinderDialog::CreateCylinderDialog(QWidget* parent) : QDialog(parent) {
    setWindowTitle("Create Cylinder");
    setModal(true);
    resize(300, 150);
    
    SetupUI();
}

void CreateCylinderDialog::SetupUI() {
    QVBoxLayout* mainLayout = new QVBoxLayout(this);
    
    // Parameters group
    QGroupBox* paramGroup = new QGroupBox("Cylinder Parameters");
    QFormLayout* formLayout = new QFormLayout(paramGroup);
    
    // Radius
    m_radiusSpinBox = new QDoubleSpinBox();
    m_radiusSpinBox->setRange(0.1, 1000.0);
    m_radiusSpinBox->setValue(5.0);
    m_radiusSpinBox->setDecimals(3);
    m_radiusSpinBox->setSuffix(" mm");
    formLayout->addRow("Radius:", m_radiusSpinBox);
    
    // Height
    m_heightSpinBox = new QDoubleSpinBox();
    m_heightSpinBox->setRange(0.1, 1000.0);
    m_heightSpinBox->setValue(10.0);
    m_heightSpinBox->setDecimals(3);
    m_heightSpinBox->setSuffix(" mm");
    formLayout->addRow("Height:", m_heightSpinBox);
    
    // Button box
    QDialogButtonBox* buttonBox = new QDialogButtonBox(QDialogButtonBox::Ok | QDialogButtonBox::Cancel);
    connect(buttonBox, &QDialogButtonBox::accepted, this, &QDialog::accept);
    connect(buttonBox, &QDialogButtonBox::rejected, this, &QDialog::reject);
    
    mainLayout->addWidget(paramGroup);
    mainLayout->addWidget(buttonBox);
    
    setLayout(mainLayout);
}

double CreateCylinderDialog::GetRadius() const {
    return m_radiusSpinBox->value();
}

double CreateCylinderDialog::GetHeight() const {
    return m_heightSpinBox->value();
}

// CreateSphereDialog Implementation
CreateSphereDialog::CreateSphereDialog(QWidget* parent) : QDialog(parent) {
    setWindowTitle("Create Sphere");
    setModal(true);
    resize(300, 120);
    
    SetupUI();
}

void CreateSphereDialog::SetupUI() {
    QVBoxLayout* mainLayout = new QVBoxLayout(this);
    
    // Parameters group
    QGroupBox* paramGroup = new QGroupBox("Sphere Parameters");
    QFormLayout* formLayout = new QFormLayout(paramGroup);
    
    // Radius
    m_radiusSpinBox = new QDoubleSpinBox();
    m_radiusSpinBox->setRange(0.1, 1000.0);
    m_radiusSpinBox->setValue(5.0);
    m_radiusSpinBox->setDecimals(3);
    m_radiusSpinBox->setSuffix(" mm");
    formLayout->addRow("Radius:", m_radiusSpinBox);
    
    // Button box
    QDialogButtonBox* buttonBox = new QDialogButtonBox(QDialogButtonBox::Ok | QDialogButtonBox::Cancel);
    connect(buttonBox, &QDialogButtonBox::accepted, this, &QDialog::accept);
    connect(buttonBox, &QDialogButtonBox::rejected, this, &QDialog::reject);
    
    mainLayout->addWidget(paramGroup);
    mainLayout->addWidget(buttonBox);
    
    setLayout(mainLayout);
}

double CreateSphereDialog::GetRadius() const {
    return m_radiusSpinBox->value();
}

} // namespace cad_ui

#include "CreatePrimitiveDialog.moc"