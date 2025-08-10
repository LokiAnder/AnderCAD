#pragma once

#include <QMainWindow>
#include <QMenuBar>
#include <QStatusBar>
#include <QDockWidget>
#include <QSplitter>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QAction>
#include <QActionGroup>
#include <QLabel>
#include <QProgressBar>
#include <QTabWidget>
#include <QMouseEvent>
#include <QPushButton>
#include <QResizeEvent>
#include <QComboBox>
#include <QTextEdit>

#include "QtOccView.h"
#include "DocumentTree.h"
#include "PropertyPanel.h"
#include "ToolBar.h"
#include "StatusBar.h"
#include "ThemeManager.h"
#include "BooleanOperationDialog.h"
#include "FilletChamferDialog.h"
#include "cad_ui/CreateHoleDialog.h" 
#include "TransformOperationDialog.h"
#include "FaceSelectionDialog.h"
#include "cad_core/CommandManager.h"
#include "cad_core/OCAFManager.h"
#include "cad_core/TransformCommand.h"
#include "cad_feature/FeatureManager.h"

namespace cad_ui {

    class MainWindow : public QMainWindow {
        Q_OBJECT

    public:
        explicit MainWindow(QWidget* parent = nullptr);
        ~MainWindow() = default;

        // 初始化
        bool Initialize();

        // 文档管理
        void NewDocument();
        void OpenDocument();
        void SaveDocument();
        void SaveDocumentAs();
        void CloseDocument();

        // 视图操作
        void FitAll();
        void ZoomIn();
        void ZoomOut();
        void SetViewMode(const QString& mode);
        void SetProjectionMode(bool orthographic);

        // 特征操作
        void CreateExtrude();
        void CreateRevolve();
        void CreateSweep();
        void CreateLoft();

        // 导入/导出
        void ImportFile();
        void ExportFile();

        // 外观设置
        void SetTheme(const QString& theme);
        void ShowGrid(bool show);
        void ShowAxes(bool show);

    protected:
        void closeEvent(QCloseEvent* event) override;
        void resizeEvent(QResizeEvent* event) override;

    private slots:
        void OnNewDocument();
        void OnOpenDocument();
        bool OnSaveDocument();
        bool OnSaveDocumentAs();
        void OnExit();

        void OnUndo();
        void OnRedo();
        void OnCut();
        void OnCopy();
        void OnPaste();
        void OnDelete();
        void OnSelectAll();

        void OnFitAll();
        void OnZoomIn();
        void OnZoomOut();
        void OnViewWireframe();
        void OnViewShaded();
        void OnViewOrthographic();
        void OnViewPerspective();

        void OnCreateBox();
        void OnCreateCylinder();
        void OnCreateSphere();
        void OnCreateExtrude();
        void OnCreateRevolve();
        void OnCreateSweep();
        void OnCreateLoft();

        void OnImportSTEP();
        void OnImportIGES();
        void OnExportSTEP();
        void OnExportIGES();
        void OnExportSTL();

        void OnShowGrid();
        void OnShowAxes();
        void OnDarkTheme();
        void OnLightTheme();

        void OnAbout();
        void OnAboutQt();

        // 布尔运算
        void OnBooleanUnion();
        void OnBooleanIntersection();
        void OnBooleanDifference();

        // 修改操作
        void OnFillet();
        void OnChamfer();
        void OnCreateHole();

        // 变换操作
        void OnTransformObjects();

        // 草图操作
        void OnEnterSketchMode();
        void OnExitSketchMode();
        void OnSketchRectangleTool();

        // 对话框交互槽
        void OnSelectionModeChanged(bool enabled, const QString& prompt);
        void OnObjectSelected(const cad_core::ShapePtr& shape);
        void OnBooleanOperationRequested(BooleanOperationType type,
            const std::vector<cad_core::ShapePtr>& targets,
            const std::vector<cad_core::ShapePtr>& tools);
        void OnFilletChamferOperationRequested(FilletChamferType type,
            const std::vector<cad_core::ShapePtr>& edges,
            double radius, double distance1, double distance2);
        void OnHoleOperationRequested(const cad_core::ShapePtr& targetShape, 
            const TopoDS_Face& selectedFace, 
            double diameter, 
            double depth,
            double x, double y, double z);
        void OnTransformOperationRequested(std::shared_ptr<cad_core::TransformCommand> command);
        void OnTransformPreviewRequested(std::shared_ptr<cad_core::TransformCommand> command);
        void OnTransformResetRequested();

        void OnHolePreviewRequested(const cad_core::ShapePtr& holePreviewShape);
        void OnHoleResetPreviewRequested();

        // 移动预览圆柱体
        // void OnHolePreviewMoved(double x, double y, double z);

        // 选择模式组合框
        void OnSelectionModeComboChanged(int index);

        // 草图模式信号
        void OnSketchModeEntered();
        void OnSketchModeExited();
        void OnFaceSelectedForSketch(const TopoDS_Face& face);

		// 视图交互
        void OnFaceSelected(const TopoDS_Face& face, const cad_core::ShapePtr& parentShape);
        void OnShapeSelected(const cad_core::ShapePtr& shape);
        void OnViewChanged();

        // 文档树选择处理器
        void OnDocumentTreeShapeSelected(const cad_core::ShapePtr& shape);
        void OnDocumentTreeFeatureSelected(const cad_feature::FeaturePtr& feature);

        // 标签页管理
        void CloseDocumentTab(int index);
        void OnTabChanged(int index);
        void NewDocumentTab();

    private:
        // UI 组件
        QTabWidget* m_tabWidget;
        QtOccView* m_viewer;
        DocumentTree* m_documentTree;
        PropertyPanel* m_propertyPanel;
        ToolBar* m_toolBar;
        StatusBar* m_statusBar;
        ThemeManager* m_themeManager;
        QTextEdit* m_console;
        QSplitter* m_mainSplitter;

        // Dock widgets
        QDockWidget* m_documentDock;
        QDockWidget* m_propertyDock;

        // Managers
        std::unique_ptr<cad_core::CommandManager> m_commandManager;
        std::unique_ptr<cad_core::OCAFManager> m_ocafManager;
        std::unique_ptr<cad_feature::FeatureManager> m_featureManager;

        // Operation dialogs
        BooleanOperationDialog* m_currentBooleanDialog;
        FilletChamferDialog* m_currentFilletChamferDialog;
        TransformOperationDialog* m_currentTransformDialog;
        CreateHoleDialog* m_currentHoleDialog;

        // Current document info
        QString m_currentFileName;
        bool m_documentModified;

        // Transform preview support
        std::vector<cad_core::ShapePtr> m_previewShapes;
        bool m_previewActive;

        // Sketch mode support
        bool m_waitingForFaceSelection;
        TopoDS_Face m_selectedFace;

        void CreateMenus();
        void CreateToolBars();
        void CreateStatusBar();
        void CreateDockWidgets();
        void CreateActions();
        void ConnectSignals();
        void CreateSelectionModeCombo();
        void CreateConsole();

        void UpdateWindowTitle();
        void UpdateActions();
        void RefreshUIFromOCAF();  // Refresh UI from OCAF document state

        bool SaveChanges();
        void SetDocumentModified(bool modified);

        // Actions
        QAction* m_newAction;
        QAction* m_openAction;
        QAction* m_saveAction;
        QAction* m_saveAsAction;
        QAction* m_exitAction;

        QAction* m_undoAction;
        QAction* m_redoAction;
        QAction* m_cutAction;
        QAction* m_copyAction;
        QAction* m_pasteAction;
        QAction* m_deleteAction;
        QAction* m_selectAllAction;

        QAction* m_fitAllAction;
        QAction* m_zoomInAction;
        QAction* m_zoomOutAction;
        QAction* m_viewWireframeAction;
        QAction* m_viewShadedAction;
        QAction* m_viewOrthographicAction;
        QAction* m_viewPerspectiveAction;

        QAction* m_createBoxAction;
        QAction* m_createCylinderAction;
        QAction* m_createSphereAction;
        QAction* m_createExtrudeAction;
        QAction* m_createRevolveAction;
        QAction* m_createSweepAction;
        QAction* m_createLoftAction;

        // Boolean operations
        QAction* m_booleanUnionAction;
        QAction* m_booleanIntersectionAction;
        QAction* m_booleanDifferenceAction;

        // Fillet and chamfer
        QAction* m_filletAction;
        QAction* m_chamferAction;
        QAction* m_createHoleAction;

        // Transform operations
        QAction* m_transformAction;

        // Sketch mode actions
        QAction* m_enterSketchAction;
        QAction* m_exitSketchAction;
        QAction* m_sketchRectangleAction;

        // Selection mode combo box
        QComboBox* m_selectionModeCombo;

        QAction* m_importSTEPAction;
        QAction* m_importIGESAction;
        QAction* m_exportSTEPAction;
        QAction* m_exportIGESAction;
        QAction* m_exportSTLAction;

        QAction* m_showGridAction;
        QAction* m_showAxesAction;
        QAction* m_darkThemeAction;
        QAction* m_lightThemeAction;

        QAction* m_aboutAction;
        QAction* m_aboutQtAction;

        QActionGroup* m_viewModeGroup;
        QActionGroup* m_projectionModeGroup;
        QActionGroup* m_themeGroup;
        QActionGroup* m_selectionModeGroup;

        // Document tab management
        QtOccView* GetCurrentViewer() const;
        void UpdateCurrentDocument();

    protected:
        // Window management for frameless window
        void mousePressEvent(QMouseEvent* event) override;
        void mouseMoveEvent(QMouseEvent* event) override;
        void mouseReleaseEvent(QMouseEvent* event) override;

    private:
        // Window dragging
        bool m_isDragging;
        QPoint m_dragStartPosition;

        // Custom title bar
        QWidget* m_titleBar;
        QLabel* m_titleLabel;
        QPushButton* m_minimizeButton;
        QPushButton* m_maximizeButton;
        QPushButton* m_closeButton;

        void CreateTitleBar();

    private slots:
        void OnMinimizeWindow();
        void OnMaximizeWindow();
        void OnCloseWindow();



    signals: 
        void faceSelectionInfo(double x, double y, double z);
    };

} // namespace cad_ui