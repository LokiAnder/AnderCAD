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
#include <vector>
#include "cad_core/Shape.h"

namespace cad_ui {

enum class BooleanOperationType {
    Union,
    Intersection,
    Difference
};

class BooleanOperationDialog : public QDialog {
    Q_OBJECT

public:
    explicit BooleanOperationDialog(BooleanOperationType operationType, QWidget* parent = nullptr);
    ~BooleanOperationDialog() = default;

    // Get selected objects
    std::vector<cad_core::ShapePtr> getTargetObjects() const { return m_targetObjects; }
    std::vector<cad_core::ShapePtr> getToolObjects() const { return m_toolObjects; }

public slots:
    void onTargetSelectionClicked();
    void onToolSelectionClicked();
    void onObjectSelected(const cad_core::ShapePtr& shape);
    void onSelectionFinished();

signals:
    void selectionModeChanged(bool enabled, const QString& prompt);
    void operationRequested(BooleanOperationType type, 
                          const std::vector<cad_core::ShapePtr>& targets,
                          const std::vector<cad_core::ShapePtr>& tools);

private:
    void setupUI();
    void updateSelectionDisplay();
    QString getOperationTitle() const;
    QString getTargetLabel() const;
    QString getToolLabel() const;

    BooleanOperationType m_operationType;
    
    // UI elements
    QVBoxLayout* m_mainLayout;
    QGroupBox* m_selectionGroup;
    QGridLayout* m_selectionLayout;
    
    // Target objects selection
    QLabel* m_targetLabel;
    QFrame* m_targetFrame;
    QHBoxLayout* m_targetFrameLayout;
    QLabel* m_targetCount;
    QPushButton* m_targetSelectButton;
    QListWidget* m_targetList;
    
    // Tool objects selection  
    QLabel* m_toolLabel;
    QFrame* m_toolFrame;
    QHBoxLayout* m_toolFrameLayout;
    QLabel* m_toolCount;
    QPushButton* m_toolSelectButton;
    QListWidget* m_toolList;
    
    // Control buttons
    QHBoxLayout* m_buttonLayout;
    QPushButton* m_okButton;
    QPushButton* m_cancelButton;
    QPushButton* m_previewButton;
    
    // Data
    std::vector<cad_core::ShapePtr> m_targetObjects;
    std::vector<cad_core::ShapePtr> m_toolObjects;
    bool m_selectingTargets;
    bool m_selectingTools;
};

} // namespace cad_ui