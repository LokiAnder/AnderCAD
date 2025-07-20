#pragma once

#include <QDialog>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QComboBox>
#include <QLineEdit>
#include <QPushButton>
#include <QLabel>
#include <QGroupBox>
#include <QCheckBox>

namespace cad_ui {

class ExportDialog : public QDialog {
    Q_OBJECT

public:
    explicit ExportDialog(QWidget* parent = nullptr);
    ~ExportDialog() = default;

    QString GetFileName() const;
    QString GetFormat() const;
    
private slots:
    void OnBrowse();
    void OnFormatChanged();
    void OnAccept();

private:
    QVBoxLayout* m_mainLayout;
    QLineEdit* m_fileNameEdit;
    QComboBox* m_formatCombo;
    QPushButton* m_browseButton;
    QPushButton* m_okButton;
    QPushButton* m_cancelButton;
    
    void SetupUI();
    void UpdateFormatOptions();
};

} // namespace cad_ui