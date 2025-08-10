#include "cad_ui/AboutDialog.h"
#include <QPixmap>
#include <QApplication>
#pragma execution_character_set("utf-8")

namespace cad_ui {

AboutDialog::AboutDialog(QWidget* parent) : QDialog(parent) {
    setWindowTitle("About Ander CAD");
    setModal(true);
    setFixedSize(400, 300);
    
    SetupUI();
}

void AboutDialog::SetupUI() {
    QVBoxLayout* mainLayout = new QVBoxLayout(this);
    
    // Application name and version
    QLabel* nameLabel = new QLabel("Ander CAD");
    nameLabel->setAlignment(Qt::AlignCenter);
    QFont nameFont = nameLabel->font();
    nameFont.setPointSize(18);
    nameFont.setBold(true);
    nameLabel->setFont(nameFont);
    
    QLabel* versionLabel = new QLabel("Version 1.0.0");
    versionLabel->setAlignment(Qt::AlignCenter);
    
    // Description
    QLabel* descLabel = new QLabel(
        "A Fusion 360-inspired CAD application built with Qt5 and OpenCASCADE.\n\n"
        "Features:\n"
        "• 3D modeling with parametric features\n"
        "• 2D sketching with constraints\n"
        "• STEP/IGES/STL import/export\n"
        "• Interactive 3D viewer\n"
        "• Modern UI with dark/light themes"
    );
    descLabel->setWordWrap(true);
    descLabel->setAlignment(Qt::AlignLeft);
    
    // Technology stack
    QLabel* techLabel = new QLabel(
        "Built with:\n"
        "• Qt 5.9\n"
        "• OpenCASCADE 7.8\n"
        "• CMake\n"
        "• Visual Studio 2022"
    );
    techLabel->setAlignment(Qt::AlignLeft);
    
    // OK button
    QPushButton* okButton = new QPushButton("OK");
    okButton->setDefault(true);
    connect(okButton, &QPushButton::clicked, this, &QDialog::accept);
    
    QHBoxLayout* buttonLayout = new QHBoxLayout();
    buttonLayout->addStretch();
    buttonLayout->addWidget(okButton);
    
    // Add all widgets to main layout
    mainLayout->addWidget(nameLabel);
    mainLayout->addWidget(versionLabel);
    mainLayout->addSpacing(20);
    mainLayout->addWidget(descLabel);
    mainLayout->addSpacing(10);
    mainLayout->addWidget(techLabel);
    mainLayout->addStretch();
    mainLayout->addLayout(buttonLayout);
    
    setLayout(mainLayout);
}

} // namespace cad_ui

#include "AboutDialog.moc"