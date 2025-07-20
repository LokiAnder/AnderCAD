#pragma once

#include <QDialog>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QGridLayout>
#include <QLabel>
#include <QPushButton>
#include <QFrame>
#include <QGroupBox>
#include <QListWidget>
#include <QDoubleSpinBox>
#include <QCheckBox>
#include <vector>
#include "cad_core/Shape.h"

namespace cad_ui {

// Forward declaration
class QtOccView;

enum class FilletChamferType {
    Fillet,
    Chamfer
};

class FilletChamferDialog : public QDialog {
    Q_OBJECT

public:
    explicit FilletChamferDialog(FilletChamferType operationType, QtOccView* viewer, QWidget* parent = nullptr);
    ~FilletChamferDialog() = default;

    // Get operation parameters
    std::vector<cad_core::ShapePtr> getSelectedEdges() const { return m_selectedEdges; }
    double getRadius() const { return m_radiusSpinBox->value(); }
    double getChamferDistance() const { return m_chamferDistance1SpinBox->value(); }
    double getChamferDistance2() const { return m_chamferDistance2SpinBox->value(); }
    bool isSymmetricChamfer() const { return m_symmetricCheckBox ? m_symmetricCheckBox->isChecked() : true; }

public slots:
    void onEdgeSelectionClicked();
    void onEdgeSelected(const cad_core::ShapePtr& edge);
    void onSelectionFinished();
    void onSymmetricChanged(bool symmetric);
    void onParameterChanged();

signals:
    void selectionModeChanged(bool enabled, const QString& prompt);
    void operationRequested(FilletChamferType type, 
                          const std::vector<cad_core::ShapePtr>& edges,
                          double radius, double distance1, double distance2);

private:
    void setupUI();
    void updateSelectionDisplay();
    void updateParameterVisibility();
    QString getOperationTitle() const;
    void syncWithViewerEdgeSelection();

    FilletChamferType m_operationType;
    QtOccView* m_viewer;
    
    // UI elements
    QVBoxLayout* m_mainLayout;
    
    // Edge selection group
    QGroupBox* m_selectionGroup;
    QGridLayout* m_selectionLayout;
    QLabel* m_edgeLabel;
    QFrame* m_edgeFrame;
    QHBoxLayout* m_edgeFrameLayout;
    QLabel* m_edgeCount;
    QPushButton* m_edgeSelectButton;
    QListWidget* m_edgeList;
    
    // Parameters group
    QGroupBox* m_parametersGroup;
    QGridLayout* m_parametersLayout;
    
    // Fillet parameters
    QLabel* m_radiusLabel;
    QDoubleSpinBox* m_radiusSpinBox;
    
    // Chamfer parameters
    QLabel* m_chamferDistance1Label;
    QDoubleSpinBox* m_chamferDistance1SpinBox;
    QLabel* m_chamferDistance2Label;
    QDoubleSpinBox* m_chamferDistance2SpinBox;
    QCheckBox* m_symmetricCheckBox;
    
    // Control buttons
    QHBoxLayout* m_buttonLayout;
    QPushButton* m_okButton;
    QPushButton* m_cancelButton;
    QPushButton* m_previewButton;
    
    // Data
    std::vector<cad_core::ShapePtr> m_selectedEdges;
    bool m_selectingEdges;
};

} // namespace cad_ui