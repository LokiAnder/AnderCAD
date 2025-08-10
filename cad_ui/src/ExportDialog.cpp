#include "cad_ui/ExportDialog.h"
#include <QFileDialog>
#include <QMessageBox>
#pragma execution_character_set("utf-8")

namespace cad_ui {

ExportDialog::ExportDialog(QWidget* parent) : QDialog(parent) {
    setWindowTitle("Export");
    setModal(true);
    resize(400, 200);
    
    SetupUI();
    
    connect(m_browseButton, &QPushButton::clicked, this, &ExportDialog::OnBrowse);
    connect(m_formatCombo, QOverload<int>::of(&QComboBox::currentIndexChanged), this, &ExportDialog::OnFormatChanged);
    connect(m_okButton, &QPushButton::clicked, this, &ExportDialog::OnAccept);
    connect(m_cancelButton, &QPushButton::clicked, this, &QDialog::reject);
}

void ExportDialog::SetupUI() {
    m_mainLayout = new QVBoxLayout(this);
    
    // File selection
    QGroupBox* fileGroup = new QGroupBox("File");
    QVBoxLayout* fileLayout = new QVBoxLayout(fileGroup);
    
    QHBoxLayout* filePathLayout = new QHBoxLayout();
    m_fileNameEdit = new QLineEdit();
    m_browseButton = new QPushButton("Browse...");
    filePathLayout->addWidget(m_fileNameEdit);
    filePathLayout->addWidget(m_browseButton);
    
    fileLayout->addLayout(filePathLayout);
    
    // Format selection
    QGroupBox* formatGroup = new QGroupBox("Format");
    QVBoxLayout* formatLayout = new QVBoxLayout(formatGroup);
    
    m_formatCombo = new QComboBox();
    m_formatCombo->addItem("STEP (*.step)", "step");
    m_formatCombo->addItem("IGES (*.iges)", "iges");
    m_formatCombo->addItem("STL (*.stl)", "stl");
    formatLayout->addWidget(m_formatCombo);
    
    // Buttons
    QHBoxLayout* buttonLayout = new QHBoxLayout();
    m_okButton = new QPushButton("Export");
    m_cancelButton = new QPushButton("Cancel");
    buttonLayout->addStretch();
    buttonLayout->addWidget(m_okButton);
    buttonLayout->addWidget(m_cancelButton);
    
    m_mainLayout->addWidget(fileGroup);
    m_mainLayout->addWidget(formatGroup);
    m_mainLayout->addLayout(buttonLayout);
    
    setLayout(m_mainLayout);
}

QString ExportDialog::GetFileName() const {
    return m_fileNameEdit->text();
}

QString ExportDialog::GetFormat() const {
    return m_formatCombo->currentData().toString();
}

void ExportDialog::OnBrowse() {
    QString format = GetFormat();
    QString filter;
    
    if (format == "step") {
        filter = "STEP Files (*.step *.stp)";
    } else if (format == "iges") {
        filter = "IGES Files (*.iges *.igs)";
    } else if (format == "stl") {
        filter = "STL Files (*.stl)";
    }
    
    QString fileName = QFileDialog::getSaveFileName(this, "Export File", "", filter);
    if (!fileName.isEmpty()) {
        m_fileNameEdit->setText(fileName);
    }
}

void ExportDialog::OnFormatChanged() {
    UpdateFormatOptions();
}

void ExportDialog::OnAccept() {
    if (m_fileNameEdit->text().isEmpty()) {
        QMessageBox::warning(this, "Warning", "Please specify a file name");
        return;
    }
    
    accept();
}

void ExportDialog::UpdateFormatOptions() {
    // Update format-specific options when format changes
    // This is a placeholder for format-specific settings
}

} // namespace cad_ui

#include "ExportDialog.moc"