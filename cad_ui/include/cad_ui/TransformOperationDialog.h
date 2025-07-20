#pragma once

#include <QDialog>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QGridLayout>
#include <QTabWidget>
#include <QGroupBox>
#include <QLabel>
#include <QPushButton>
#include <QDoubleSpinBox>
#include <QSpinBox>
#include <QFrame>
#include <QListWidget>
#include <QRadioButton>
#include <QButtonGroup>
#include <QCheckBox>
#include <vector>
#include "cad_core/Shape.h"
#include "cad_core/TransformCommand.h"

namespace cad_ui {

/**
 * @class TransformOperationDialog
 * @brief 变换操作对话框，支持平移、旋转、缩放操作
 */
class TransformOperationDialog : public QDialog {
    Q_OBJECT

public:
    explicit TransformOperationDialog(QWidget* parent = nullptr);
    ~TransformOperationDialog() = default;

    // 获取选中的对象
    std::vector<cad_core::ShapePtr> getSelectedObjects() const { return m_selectedObjects; }
    
    // 获取当前变换命令（用于执行）
    std::shared_ptr<cad_core::TransformCommand> getCurrentTransformCommand() const;

public slots:
    void onObjectSelectionClicked();
    void onObjectSelected(const cad_core::ShapePtr& shape);
    void onSelectionFinished();
    void onPreviewClicked();
    void onResetClicked();
    void onTransformTypeChanged();
    
    // 参数变化时的槽函数
    void onTranslationParameterChanged();
    void onRotationParameterChanged();
    void onScaleParameterChanged();

signals:
    void selectionModeChanged(bool enabled, const QString& prompt);
    void transformRequested(std::shared_ptr<cad_core::TransformCommand> command);
    void previewRequested(std::shared_ptr<cad_core::TransformCommand> command);
    void resetRequested();

private slots:
    void accept() override;
    void reject() override;

private:
    void setupUI();
    void setupTranslationTab();
    void setupRotationTab();
    void setupScaleTab();
    void updatePreview();
    void updateSelectionDisplay();
    void resetAllParameters();
    
    // UI组件
    QVBoxLayout* m_mainLayout;
    
    // 对象选择区域
    QGroupBox* m_selectionGroup;
    QGridLayout* m_selectionLayout;
    QFrame* m_objectFrame;
    QHBoxLayout* m_objectFrameLayout;
    QLabel* m_objectCount;
    QPushButton* m_objectSelectButton;
    QListWidget* m_objectList;
    
    // 变换类型选择
    QTabWidget* m_transformTabs;
    
    // 平移参数
    QWidget* m_translateTab;
    QGroupBox* m_translationGroup;
    QGridLayout* m_translationLayout;
    QDoubleSpinBox* m_translateX;
    QDoubleSpinBox* m_translateY;
    QDoubleSpinBox* m_translateZ;
    
    // 旋转参数
    QWidget* m_rotateTab;
    QGroupBox* m_rotationGroup;
    QGridLayout* m_rotationLayout;
    
    // 旋转轴点
    QGroupBox* m_axisPointGroup;
    QDoubleSpinBox* m_axisPointX;
    QDoubleSpinBox* m_axisPointY;
    QDoubleSpinBox* m_axisPointZ;
    
    // 旋转轴方向
    QGroupBox* m_axisDirectionGroup;
    QDoubleSpinBox* m_axisDirectionX;
    QDoubleSpinBox* m_axisDirectionY;
    QDoubleSpinBox* m_axisDirectionZ;
    QButtonGroup* m_axisPresetGroup;
    QRadioButton* m_axisXButton;
    QRadioButton* m_axisYButton;
    QRadioButton* m_axisZButton;
    QRadioButton* m_axisCustomButton;
    
    // 旋转角度
    QGroupBox* m_angleGroup;
    QDoubleSpinBox* m_rotationAngle;
    QRadioButton* m_angleRadians;
    QRadioButton* m_angleDegrees;
    
    // 缩放参数
    QWidget* m_scaleTab;
    QGroupBox* m_scaleGroup;
    QGridLayout* m_scaleLayout;
    
    // 缩放中心
    QGroupBox* m_scaleCenterGroup;
    QDoubleSpinBox* m_scaleCenterX;
    QDoubleSpinBox* m_scaleCenterY;
    QDoubleSpinBox* m_scaleCenterZ;
    QPushButton* m_centerAtOriginButton;
    QPushButton* m_centerAtBoundingBoxButton;
    
    // 缩放因子
    QGroupBox* m_scaleFactorGroup;
    QCheckBox* m_uniformScaleCheckBox;
    QDoubleSpinBox* m_scaleFactorUniform;
    QDoubleSpinBox* m_scaleFactorX;
    QDoubleSpinBox* m_scaleFactorY;
    QDoubleSpinBox* m_scaleFactorZ;
    
    // 控制按钮
    QHBoxLayout* m_buttonLayout;
    QPushButton* m_previewButton;
    QPushButton* m_resetButton;
    QPushButton* m_okButton;
    QPushButton* m_cancelButton;
    
    // 数据
    std::vector<cad_core::ShapePtr> m_selectedObjects;
    bool m_selectingObjects;
    bool m_previewActive;
    
    // 预设的轴方向常量
    static constexpr double AXIS_X[] = {1.0, 0.0, 0.0};
    static constexpr double AXIS_Y[] = {0.0, 1.0, 0.0};
    static constexpr double AXIS_Z[] = {0.0, 0.0, 1.0};
};

} // namespace cad_ui