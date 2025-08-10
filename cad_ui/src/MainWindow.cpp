#include "cad_ui/MainWindow.h"
#include "cad_ui/ExportDialog.h"
#include "cad_ui/AboutDialog.h"
#include "cad_ui/CreatePrimitiveDialog.h"
#include "cad_core/CreateBoxCommand.h"
#include "cad_core/CreateCylinderCommand.h"
#include "cad_core/CreateSphereCommand.h"
#include "cad_core/OCAFManager.h"
#include "cad_core/ShapeFactory.h"
#include "cad_core/BooleanOperations.h"
#include "cad_core/FilletChamferOperations.h"
#include "cad_ui/CreateHoleDialog.h" 
#include "cad_core/SelectionManager.h"
#include <TopoDS.hxx>
#include <BRep_Tool.hxx>
#include <GProp_GProps.hxx>
#include <BRepGProp.hxx>
#include <gp_Pln.hxx>
#include <Geom_Surface.hxx>
#include <Geom_Plane.hxx>
#include <gp_Ax2.hxx>
#include <BRepBuilderAPI_Transform.hxx>

#include <iostream>
#include <QApplication>
#include <QFileDialog>
#include <QMessageBox>
#include <QInputDialog>
#include <QCloseEvent>
#include <QSplitter>
#include <QSettings>
#include <QTabWidget>
#include <QFile>
#include <QTextStream>
#include <QDebug>
#include <QToolButton>
#include <QHBoxLayout>
#include <QVBoxLayout>
#include <QFrame>
#include <QLabel>
#include <map>
#pragma execution_character_set("utf-8")

namespace cad_ui {

MainWindow::MainWindow(QWidget* parent) 
    : QMainWindow(parent), m_tabWidget(nullptr), m_documentModified(false), 
      m_isDragging(false), m_dragStartPosition(), m_titleBar(nullptr),
      m_titleLabel(nullptr), m_minimizeButton(nullptr), m_maximizeButton(nullptr),
      m_closeButton(nullptr), m_currentBooleanDialog(nullptr), m_currentFilletChamferDialog(nullptr),
      m_currentTransformDialog(nullptr), 
      m_waitingForFaceSelection(false) {
    
    // Load modern flat stylesheet
    QFile styleFile(":/resources/styles.qss");
    if (styleFile.open(QFile::ReadOnly | QFile::Text)) {
        QTextStream stream(&styleFile);
        QString style = stream.readAll();
        this->setStyleSheet(style);
        qDebug() << "Stylesheet loaded successfully from QRC, length:" << style.length();
    } else {
        qDebug() << "Failed to load stylesheet from QRC resources";
        // Fallback: try to load from file system for development
        QFile fallbackFile("C:\\Users\\Administrator\\source\\repos\\AnderCAD\\cad_ui\\resources\\styles.qss");
        if (fallbackFile.open(QFile::ReadOnly | QFile::Text)) {
            QTextStream fallbackStream(&fallbackFile);
            QString fallbackStyle = fallbackStream.readAll();
            this->setStyleSheet(fallbackStyle);
            qDebug() << "Fallback stylesheet loaded, length:" << fallbackStyle.length();
        }
    }
    
    // Initialize managers
    m_commandManager = std::make_unique<cad_core::CommandManager>();
    m_ocafManager = std::make_unique<cad_core::OCAFManager>();
    m_featureManager = std::make_unique<cad_feature::FeatureManager>();
    
    // Create UI components
    CreateActions();
    CreateMenus();
    
    // Create selection mode combo box before toolbars
    CreateSelectionModeCombo();
    
    CreateToolBars();
    CreateStatusBar();
    CreateDockWidgets();
    CreateTitleBar();
    CreateConsole();
    
    // Create multi-document tab interface
    m_tabWidget = new QTabWidget(this);
    m_tabWidget->setTabsClosable(true);
    m_tabWidget->setMovable(true);
    m_tabWidget->setObjectName("documentTabs");

    
    // Create first document tab
    m_viewer = new QtOccView(this);
    m_viewer->setObjectName("viewer3D");
    m_tabWidget->addTab(m_viewer, "Document 1");
    
    // Create main splitter with viewer and console
    m_mainSplitter = new QSplitter(Qt::Vertical, this);
    m_mainSplitter->addWidget(m_tabWidget);
    m_mainSplitter->addWidget(m_console);
    m_mainSplitter->setStretchFactor(0, 3); // Give viewer more space
    m_mainSplitter->setStretchFactor(1, 1); // Console gets less space
    
    setCentralWidget(m_mainSplitter);
    
    // Initialize theme manager
    m_themeManager = new ThemeManager(this);
    
    // Initialize dialog pointers to null
    m_currentBooleanDialog = nullptr;
    m_currentFilletChamferDialog = nullptr;
    m_currentHoleDialog = nullptr;
    
    // Connect signals
    ConnectSignals();
    
    // Connect tab widget signals
    connect(m_tabWidget, &QTabWidget::tabCloseRequested, this, &MainWindow::CloseDocumentTab);
    connect(m_tabWidget, &QTabWidget::currentChanged, this, &MainWindow::OnTabChanged);
    
    // Set window properties - frameless window
    setWindowTitle("Ander CAD");
    setWindowFlags(Qt::FramelessWindowHint | Qt::Window);
    setMinimumSize(800, 600);
    resize(1200, 800);
    
    // Enable mouse tracking for window moving
    setMouseTracking(true);
    
    // Update UI
    UpdateActions();
    UpdateWindowTitle();
}

bool MainWindow::Initialize() {
    
    // Initialize OCAF manager
    if (!m_ocafManager->Initialize()) {
        QMessageBox::critical(this, "Error", "Failed to initialize OCAF document manager");
        return false;
    }
    
    // Create initial document for undo/redo functionality
    if (!m_ocafManager->NewDocument()) {
        QMessageBox::critical(this, "Error", "Failed to create new OCAF document");
        return false;
    }
    
    // Set initial view and render
    m_viewer->FitAll();
    m_viewer->RedrawAll();  // 确保坐标轴立即显示
    
    return true;
}

void MainWindow::CreateActions() {
    // File actions
    m_newAction = new QAction("&New", this);
    m_newAction->setShortcut(QKeySequence::New);
    m_newAction->setStatusTip("Create a new document");
    
    m_openAction = new QAction("&Open...", this);
    m_openAction->setShortcut(QKeySequence::Open);
    m_openAction->setStatusTip("Open an existing document");
    
    m_saveAction = new QAction("&Save", this);
    m_saveAction->setShortcut(QKeySequence::Save);
    m_saveAction->setStatusTip("Save the document");
    
    m_saveAsAction = new QAction("Save &As...", this);
    m_saveAsAction->setShortcut(QKeySequence::SaveAs);
    m_saveAsAction->setStatusTip("Save the document with a new name");
    
    m_exitAction = new QAction("E&xit", this);
    m_exitAction->setShortcut(QKeySequence::Quit);
    m_exitAction->setStatusTip("Exit the application");
    
    // Edit actions
    m_undoAction = new QAction("&Undo", this);
    m_undoAction->setShortcut(QKeySequence::Undo);
    m_undoAction->setStatusTip("Undo the last operation");
    
    m_redoAction = new QAction("&Redo", this);
    m_redoAction->setShortcut(QKeySequence("Ctrl+Y"));
    m_redoAction->setStatusTip("Redo the last undone operation");
    
    // View actions
    m_fitAllAction = new QAction("Fit &All", this);
    m_fitAllAction->setShortcut(QKeySequence("F"));
    m_fitAllAction->setStatusTip("Fit all objects in view");
    
    m_zoomInAction = new QAction("Zoom &In", this);
    m_zoomInAction->setShortcut(QKeySequence::ZoomIn);
    m_zoomInAction->setStatusTip("Zoom in");
    
    m_zoomOutAction = new QAction("Zoom &Out", this);
    m_zoomOutAction->setShortcut(QKeySequence::ZoomOut);
    m_zoomOutAction->setStatusTip("Zoom out");
    
    // View mode actions
    m_viewWireframeAction = new QAction("&Wireframe", this);
    m_viewWireframeAction->setShortcut(QKeySequence("W"));
    m_viewWireframeAction->setCheckable(true);
    m_viewWireframeAction->setStatusTip("Show wireframe view");
    
    m_viewShadedAction = new QAction("&Shaded", this);
    m_viewShadedAction->setShortcut(QKeySequence("S"));
    m_viewShadedAction->setCheckable(true);
    m_viewShadedAction->setChecked(true);
    m_viewShadedAction->setStatusTip("Show shaded view");
    
    m_viewModeGroup = new QActionGroup(this);
    m_viewModeGroup->addAction(m_viewWireframeAction);
    m_viewModeGroup->addAction(m_viewShadedAction);
    
    // Projection mode actions
    m_viewOrthographicAction = new QAction("&Orthographic", this);
    m_viewOrthographicAction->setCheckable(true);
    m_viewOrthographicAction->setChecked(true);
    m_viewOrthographicAction->setStatusTip("Orthographic projection");
    
    m_viewPerspectiveAction = new QAction("&Perspective", this);
    m_viewPerspectiveAction->setCheckable(true);
    m_viewPerspectiveAction->setStatusTip("Perspective projection");
    
    m_projectionModeGroup = new QActionGroup(this);
    m_projectionModeGroup->addAction(m_viewOrthographicAction);
    m_projectionModeGroup->addAction(m_viewPerspectiveAction);
    
    // Create actions with 30x30 icons (icon-only display)
    m_createBoxAction = new QAction("", this);
    QIcon boxIcon(":/icons/icons/Prim-Box.svg");
    boxIcon.addPixmap(QPixmap(":/icons/icons/Prim-Box.svg").scaled(30, 30, Qt::KeepAspectRatio, Qt::SmoothTransformation));
    m_createBoxAction->setIcon(boxIcon);
    m_createBoxAction->setStatusTip("Create a box");
    
    m_createCylinderAction = new QAction("", this);
    QIcon cylinderIcon(":/icons/icons/Prim-Cylinder.svg");
    cylinderIcon.addPixmap(QPixmap(":/icons/icons/Prim-Cylinder.svg").scaled(30, 30, Qt::KeepAspectRatio, Qt::SmoothTransformation));
    m_createCylinderAction->setIcon(cylinderIcon);
    m_createCylinderAction->setStatusTip("Create a cylinder");
    
    m_createSphereAction = new QAction("", this);
    QIcon sphereIcon(":/icons/icons/Prim-Sphere.svg");
    sphereIcon.addPixmap(QPixmap(":/icons/icons/Prim-Sphere.svg").scaled(30, 30, Qt::KeepAspectRatio, Qt::SmoothTransformation));
    m_createSphereAction->setIcon(sphereIcon);
    m_createSphereAction->setStatusTip("Create a sphere");
    
    m_createExtrudeAction = new QAction("", this);
    QIcon extrudeIcon(":/icons/icons/Form-Extrude.svg");
    extrudeIcon.addPixmap(QPixmap(":/icons/icons/Form-Extrude.svg").scaled(30, 30, Qt::KeepAspectRatio, Qt::SmoothTransformation));
    m_createExtrudeAction->setIcon(extrudeIcon);
    m_createExtrudeAction->setStatusTip("Create an extrude feature");
    
    // Boolean operations with 30x30 icons (icon-only display)
    m_booleanUnionAction = new QAction("", this);
    QIcon fuseIcon(":/icons/icons/Boolean-Fuse.svg");
    fuseIcon.addPixmap(QPixmap(":/icons/icons/Boolean-Fuse.svg").scaled(30, 30, Qt::KeepAspectRatio, Qt::SmoothTransformation));
    m_booleanUnionAction->setIcon(fuseIcon);
    m_booleanUnionAction->setStatusTip("合并选中的形状");
    
    m_booleanIntersectionAction = new QAction("", this);
    QIcon commonIcon(":/icons/icons/Boolean-Common.svg");
    commonIcon.addPixmap(QPixmap(":/icons/icons/Boolean-Common.svg").scaled(30, 30, Qt::KeepAspectRatio, Qt::SmoothTransformation));
    m_booleanIntersectionAction->setIcon(commonIcon);
    m_booleanIntersectionAction->setStatusTip("获取选中形状的交集");
    
    m_booleanDifferenceAction = new QAction("", this);
    QIcon cutIcon(":/icons/icons/Boolean-Cut.svg");
    cutIcon.addPixmap(QPixmap(":/icons/icons/Boolean-Cut.svg").scaled(30, 30, Qt::KeepAspectRatio, Qt::SmoothTransformation));
    m_booleanDifferenceAction->setIcon(cutIcon);
    m_booleanDifferenceAction->setStatusTip("从一个形状中减去另一个形状");
    

    // Fillet and chamfer operations with 30x30 icons (icon-only display)
    m_filletAction = new QAction("", this);
    QIcon filletIcon(":/icons/icons/Mod-Fillet.svg");
    filletIcon.addPixmap(QPixmap(":/icons/icons/Mod-Fillet.svg").scaled(30, 30, Qt::KeepAspectRatio, Qt::SmoothTransformation));
    m_filletAction->setIcon(filletIcon);
    m_filletAction->setStatusTip("Add fillet to selected edges");
    
    m_chamferAction = new QAction("", this);
    QIcon chamferIcon(":/icons/icons/Mod-Chamfer.svg");
    chamferIcon.addPixmap(QPixmap(":/icons/icons/Mod-Chamfer.svg").scaled(30, 30, Qt::KeepAspectRatio, Qt::SmoothTransformation));
    m_chamferAction->setIcon(chamferIcon);
    m_chamferAction->setStatusTip("Add chamfer to selected edges");

	// create hole action with 30x30 icon (icon-only display)
    m_createHoleAction = new QAction("", this);
    //QIcon holeIcon(":/icons/icons/Boolean-Cut.svg");
    //holeIcon.addPixmap(QPixmap(":/icons/icons/Boolean-Cut.svg").scaled(30, 30, Qt::KeepAspectRatio, Qt::SmoothTransformation));
    //m_createHoleAction->setIcon(holeIcon);
    m_createHoleAction->setStatusTip("Create a hole on a face");

    
    // Transform actions
    m_transformAction = new QAction("&Transform...", this);
    m_transformAction->setShortcut(QKeySequence("Ctrl+T"));
    m_transformAction->setStatusTip("Transform objects (translate, rotate, scale)");
    
    // Sketch actions
    m_enterSketchAction = new QAction("Enter &Sketch", this);
    m_enterSketchAction->setShortcut(QKeySequence("Ctrl+Shift+S"));
    m_enterSketchAction->setStatusTip("Enter sketch mode");
    
    m_exitSketchAction = new QAction("E&xit Sketch", this);
    m_exitSketchAction->setShortcut(QKeySequence("Escape"));
    m_exitSketchAction->setStatusTip("Exit sketch mode");
    m_exitSketchAction->setEnabled(false);  // 初始禁用
    
    m_sketchRectangleAction = new QAction("&Rectangle", this);
    m_sketchRectangleAction->setShortcut(QKeySequence("R"));
    m_sketchRectangleAction->setStatusTip("Draw rectangle in sketch mode");
    m_sketchRectangleAction->setEnabled(false);  // 初始禁用
    
    // Selection mode now handled by combo box - old actions commented out for testing
    
    // Selection mode group now handled by combo box
    
    // Theme actions
    m_darkThemeAction = new QAction("&Dark Theme", this);
    m_darkThemeAction->setCheckable(true);
    m_darkThemeAction->setStatusTip("Use dark theme");
    
    m_lightThemeAction = new QAction("&Light Theme", this);
    m_lightThemeAction->setCheckable(true);
    m_lightThemeAction->setChecked(true);
    m_lightThemeAction->setStatusTip("Use light theme");
    
    m_themeGroup = new QActionGroup(this);
    m_themeGroup->addAction(m_darkThemeAction);
    m_themeGroup->addAction(m_lightThemeAction);
    
    // Help actions
    m_aboutAction = new QAction("&About", this);
    m_aboutAction->setStatusTip("Show the application's About box");
    
    m_aboutQtAction = new QAction("About &Qt", this);
    m_aboutQtAction->setStatusTip("Show the Qt library's About box");
}

void MainWindow::CreateMenus() {
    // File menu
    QMenu* fileMenu = menuBar()->addMenu("&File");
    fileMenu->addAction(m_newAction);
    fileMenu->addAction(m_openAction);
    fileMenu->addSeparator();
    fileMenu->addAction(m_saveAction);
    fileMenu->addAction(m_saveAsAction);
    fileMenu->addSeparator();
    fileMenu->addAction(m_exitAction);
    
    // Edit menu
    QMenu* editMenu = menuBar()->addMenu("&Edit");
    editMenu->addAction(m_undoAction);
    editMenu->addAction(m_redoAction);
    
    // View menu
    QMenu* viewMenu = menuBar()->addMenu("&View");
    viewMenu->addAction(m_fitAllAction);
    viewMenu->addAction(m_zoomInAction);
    viewMenu->addAction(m_zoomOutAction);
    viewMenu->addSeparator();
    viewMenu->addAction(m_viewWireframeAction);
    viewMenu->addAction(m_viewShadedAction);
    viewMenu->addSeparator();
    viewMenu->addAction(m_viewOrthographicAction);
    viewMenu->addAction(m_viewPerspectiveAction);
    
    // Create menu
    QMenu* createMenu = menuBar()->addMenu("&Create");
    createMenu->addAction(m_createBoxAction);
    createMenu->addAction(m_createCylinderAction);
    createMenu->addAction(m_createSphereAction);
    createMenu->addSeparator();
    createMenu->addAction(m_createExtrudeAction);
    
    // Boolean menu
    QMenu* booleanMenu = menuBar()->addMenu("&Boolean");
    booleanMenu->addAction(m_booleanUnionAction);
    booleanMenu->addAction(m_booleanIntersectionAction);
    booleanMenu->addAction(m_booleanDifferenceAction);
    
    // Modify menu
    QMenu* modifyMenu = menuBar()->addMenu("&Modify");
    modifyMenu->addAction(m_filletAction);
    modifyMenu->addAction(m_chamferAction);
    modifyMenu->addSeparator();
    modifyMenu->addAction(m_transformAction);
    
    // Sketch menu
    QMenu* sketchMenu = menuBar()->addMenu("&Sketch");
    sketchMenu->addAction(m_enterSketchAction);
    sketchMenu->addAction(m_exitSketchAction);
    sketchMenu->addSeparator();
    sketchMenu->addAction(m_sketchRectangleAction);
    
    // Selection menu - now handled by combo box in toolbar
    
    // Tools menu
    QMenu* toolsMenu = menuBar()->addMenu("&Tools");
    toolsMenu->addAction(m_darkThemeAction);
    toolsMenu->addAction(m_lightThemeAction);
    
    // Help menu
    QMenu* helpMenu = menuBar()->addMenu("&Help");
    helpMenu->addAction(m_aboutAction);
    helpMenu->addAction(m_aboutQtAction);
}

void MainWindow::CreateToolBars() {
    // Create main toolbar area widget with tabs
    QWidget* toolBarArea = new QWidget(this);
    toolBarArea->setObjectName("toolBarArea");
    toolBarArea->setMaximumHeight(180);
    toolBarArea->setMinimumHeight(180);
    
    // Create tab widget for organizing tools
    QTabWidget* toolTabWidget = new QTabWidget(toolBarArea);
    toolTabWidget->setObjectName("toolTabWidget");
    toolTabWidget->setTabPosition(QTabWidget::North);
    
    // File Tab - File operations and undo/redo
    QWidget* fileTab = new QWidget();
    QHBoxLayout* fileLayout = new QHBoxLayout(fileTab);
    fileLayout->setContentsMargins(5, 5, 5, 5);
    fileLayout->setSpacing(3);
    
    // File operations group
    QFrame* fileOpsFrame = new QFrame();
    fileOpsFrame->setFrameStyle(QFrame::StyledPanel);
    QVBoxLayout* fileOpsLayout = new QVBoxLayout(fileOpsFrame);
    fileOpsLayout->setContentsMargins(2, 1, 2, 2);
    fileOpsLayout->setSpacing(1);
    
    QLabel* fileLabel = new QLabel("文件");
    fileLabel->setAlignment(Qt::AlignCenter);
    fileOpsLayout->addWidget(fileLabel);
    QHBoxLayout* fileButtonsLayout = new QHBoxLayout();
    fileButtonsLayout->setSpacing(2);
    
    QToolButton* newBtn = new QToolButton();
    newBtn->setDefaultAction(m_newAction);
    newBtn->setToolButtonStyle(Qt::ToolButtonTextUnderIcon);
    newBtn->setMinimumSize(40, 40);
    newBtn->setMaximumSize(40, 40);
    fileButtonsLayout->addWidget(newBtn);
    
    QToolButton* openBtn = new QToolButton();
    openBtn->setDefaultAction(m_openAction);
    openBtn->setToolButtonStyle(Qt::ToolButtonTextUnderIcon);
    openBtn->setMinimumSize(90, 90);
    openBtn->setMaximumSize(90, 90);
    fileButtonsLayout->addWidget(openBtn);
    
    QToolButton* saveBtn = new QToolButton();
    saveBtn->setDefaultAction(m_saveAction);
    saveBtn->setToolButtonStyle(Qt::ToolButtonTextUnderIcon);
    saveBtn->setMinimumSize(90, 90);
    saveBtn->setMaximumSize(90, 90);
    fileButtonsLayout->addWidget(saveBtn);
    
    fileOpsLayout->addLayout(fileButtonsLayout);
    fileLayout->addWidget(fileOpsFrame);
    
    // History operations group
    QFrame* historyFrame = new QFrame();
    historyFrame->setFrameStyle(QFrame::StyledPanel);
    QVBoxLayout* historyLayout = new QVBoxLayout(historyFrame);
    historyLayout->setContentsMargins(2, 1, 2, 2);
    historyLayout->setSpacing(1);
    
    QLabel* historyLabel = new QLabel("历史");
    historyLabel->setAlignment(Qt::AlignCenter);
    historyLayout->addWidget(historyLabel);
    QHBoxLayout* historyButtonsLayout = new QHBoxLayout();
    historyButtonsLayout->setSpacing(2);
    
    QToolButton* undoBtn = new QToolButton();
    undoBtn->setDefaultAction(m_undoAction);
    undoBtn->setToolButtonStyle(Qt::ToolButtonTextUnderIcon);
    undoBtn->setMinimumSize(90, 90);
    undoBtn->setMaximumSize(90, 90);
    historyButtonsLayout->addWidget(undoBtn);
    
    QToolButton* redoBtn = new QToolButton();
    redoBtn->setDefaultAction(m_redoAction);
    redoBtn->setToolButtonStyle(Qt::ToolButtonTextUnderIcon);
    redoBtn->setMinimumSize(90, 90);
    redoBtn->setMaximumSize(90, 90);
    historyButtonsLayout->addWidget(redoBtn);
    
    historyLayout->addLayout(historyButtonsLayout);
    fileLayout->addWidget(historyFrame);
    
    fileLayout->addStretch();
    toolTabWidget->addTab(fileTab, "文件");
    
    // Design Tab - Primitive creation
    QWidget* designTab = new QWidget();
    QHBoxLayout* designLayout = new QHBoxLayout(designTab);
    designLayout->setContentsMargins(5, 5, 5, 5);
    designLayout->setSpacing(3);
    
    // Primitives group
    QFrame* primitivesFrame = new QFrame();
    primitivesFrame->setFrameStyle(QFrame::StyledPanel);
    QVBoxLayout* primitivesLayout = new QVBoxLayout(primitivesFrame);
    primitivesLayout->setContentsMargins(5, 3, 5, 8);
    primitivesLayout->setSpacing(1);
    
    QLabel* primitivesLabel = new QLabel("基本形状");
    primitivesLabel->setAlignment(Qt::AlignCenter);
    primitivesLayout->addWidget(primitivesLabel);
    QHBoxLayout* primitivesButtonsLayout = new QHBoxLayout();
    primitivesButtonsLayout->setSpacing(8);
    
    // Box button with label below
    QVBoxLayout* boxLayout = new QVBoxLayout();
    QToolButton* boxBtn = new QToolButton();
    boxBtn->setDefaultAction(m_createBoxAction);
    boxBtn->setToolButtonStyle(Qt::ToolButtonIconOnly);
    boxBtn->setIconSize(QSize(30, 30));
    boxBtn->setFixedSize(30, 30);
    boxBtn->setStyleSheet("QToolButton { border-radius: 8px; border: 1px solid #ccc; background-color: #f0f0f0; } QToolButton:hover { background-color: #e0e0e0; } QToolButton:pressed { background-color: #d0d0d0; }");
    QLabel* boxLabel = new QLabel("长方体");
    boxLabel->setAlignment(Qt::AlignCenter);
    boxLabel->setStyleSheet("font-size: 9px; color: #333; margin-top: 2px;");
    boxLayout->addWidget(boxBtn);
    boxLayout->addWidget(boxLabel);
    boxLayout->setSpacing(1);
    boxLayout->setContentsMargins(0, 0, 0, 0);
    primitivesButtonsLayout->addLayout(boxLayout);
    
    // Cylinder button with label below
    QVBoxLayout* cylinderLayout = new QVBoxLayout();
    QToolButton* cylinderBtn = new QToolButton();
    cylinderBtn->setDefaultAction(m_createCylinderAction);
    cylinderBtn->setToolButtonStyle(Qt::ToolButtonIconOnly);
    cylinderBtn->setIconSize(QSize(30, 30));
    cylinderBtn->setFixedSize(30, 30);
    cylinderBtn->setStyleSheet("QToolButton { border-radius: 8px; border: 1px solid #ccc; background-color: #f0f0f0; } QToolButton:hover { background-color: #e0e0e0; } QToolButton:pressed { background-color: #d0d0d0; }");
    QLabel* cylinderLabel = new QLabel("圆柱体");
    cylinderLabel->setAlignment(Qt::AlignCenter);
    cylinderLabel->setStyleSheet("font-size: 9px; color: #333; margin-top: 2px;");
    cylinderLayout->addWidget(cylinderBtn);
    cylinderLayout->addWidget(cylinderLabel);
    cylinderLayout->setSpacing(1);
    cylinderLayout->setContentsMargins(0, 0, 0, 0);
    primitivesButtonsLayout->addLayout(cylinderLayout);
    
    // Sphere button with label below
    QVBoxLayout* sphereLayout = new QVBoxLayout();
    QToolButton* sphereBtn = new QToolButton();
    sphereBtn->setDefaultAction(m_createSphereAction);
    sphereBtn->setToolButtonStyle(Qt::ToolButtonIconOnly);
    sphereBtn->setIconSize(QSize(30, 30));
    sphereBtn->setFixedSize(30, 30);
    sphereBtn->setStyleSheet("QToolButton { border-radius: 8px; border: 1px solid #ccc; background-color: #f0f0f0; } QToolButton:hover { background-color: #e0e0e0; } QToolButton:pressed { background-color: #d0d0d0; }");
    QLabel* sphereLabel = new QLabel("球体");
    sphereLabel->setAlignment(Qt::AlignCenter);
    sphereLabel->setStyleSheet("font-size: 9px; color: #333; margin-top: 2px;");
    sphereLayout->addWidget(sphereBtn);
    sphereLayout->addWidget(sphereLabel);
    sphereLayout->setSpacing(1);
    sphereLayout->setContentsMargins(0, 0, 0, 0);
    primitivesButtonsLayout->addLayout(sphereLayout);
    
    primitivesLayout->addLayout(primitivesButtonsLayout);
    designLayout->addWidget(primitivesFrame);
    
    // Features group
    QFrame* featuresFrame = new QFrame();
    featuresFrame->setFrameStyle(QFrame::StyledPanel);
    QVBoxLayout* featuresLayout = new QVBoxLayout(featuresFrame);
    featuresLayout->setContentsMargins(5, 3, 5, 8);
    featuresLayout->setSpacing(1);
    
    QLabel* featuresLabel = new QLabel("特征");
    featuresLabel->setAlignment(Qt::AlignCenter);
    featuresLayout->addWidget(featuresLabel);
    QHBoxLayout* featuresButtonsLayout = new QHBoxLayout();
    featuresButtonsLayout->setSpacing(8);
    
    // Extrude button with label below
    QVBoxLayout* extrudeLayout = new QVBoxLayout();
    QToolButton* extrudeBtn = new QToolButton();
    extrudeBtn->setDefaultAction(m_createExtrudeAction);
    extrudeBtn->setToolButtonStyle(Qt::ToolButtonIconOnly);
    extrudeBtn->setIconSize(QSize(30, 30));
    extrudeBtn->setFixedSize(30, 30);
    extrudeBtn->setStyleSheet("QToolButton { border-radius: 8px; border: 1px solid #ccc; background-color: #f0f0f0; } QToolButton:hover { background-color: #e0e0e0; } QToolButton:pressed { background-color: #d0d0d0; }");
    QLabel* extrudeLabel = new QLabel("拉伸");
    extrudeLabel->setAlignment(Qt::AlignCenter);
    extrudeLabel->setStyleSheet("font-size: 9px; color: #333; margin-top: 2px;");
    extrudeLayout->addWidget(extrudeBtn);
    extrudeLayout->addWidget(extrudeLabel);
    extrudeLayout->setSpacing(1);
    extrudeLayout->setContentsMargins(0, 0, 0, 0);
    featuresButtonsLayout->addLayout(extrudeLayout);
    
    featuresLayout->addLayout(featuresButtonsLayout);
    designLayout->addWidget(featuresFrame);
    
    designLayout->addStretch();
    toolTabWidget->addTab(designTab, "设计");
    
    // Modify Tab - Boolean operations and modifications
    QWidget* modifyTab = new QWidget();
    QHBoxLayout* modifyLayout = new QHBoxLayout(modifyTab);
    modifyLayout->setContentsMargins(5, 5, 5, 5);
    modifyLayout->setSpacing(3);
    
    // Boolean operations group
    QFrame* booleanFrame = new QFrame();
    booleanFrame->setFrameStyle(QFrame::StyledPanel);
    QVBoxLayout* booleanLayout = new QVBoxLayout(booleanFrame);
    booleanLayout->setContentsMargins(5, 3, 5, 8);
    booleanLayout->setSpacing(1);
    
    QLabel* booleanLabel = new QLabel("布尔运算");
    booleanLabel->setAlignment(Qt::AlignCenter);
    booleanLayout->addWidget(booleanLabel);
    QHBoxLayout* booleanButtonsLayout = new QHBoxLayout();
    booleanButtonsLayout->setSpacing(8);
    
    // Union button with label below
    QVBoxLayout* unionLayout = new QVBoxLayout();
    QToolButton* unionBtn = new QToolButton();
    unionBtn->setDefaultAction(m_booleanUnionAction);
    unionBtn->setToolButtonStyle(Qt::ToolButtonIconOnly);
    unionBtn->setIconSize(QSize(30, 30));
    unionBtn->setFixedSize(30, 30);
    unionBtn->setStyleSheet("QToolButton { border-radius: 8px; border: 1px solid #ccc; background-color: #f0f0f0; } QToolButton:hover { background-color: #e0e0e0; } QToolButton:pressed { background-color: #d0d0d0; }");
    QLabel* unionLabel = new QLabel("合并");
    unionLabel->setAlignment(Qt::AlignCenter);
    unionLabel->setStyleSheet("font-size: 9px; color: #333; margin-top: 2px;");
    unionLayout->addWidget(unionBtn);
    unionLayout->addWidget(unionLabel);
    unionLayout->setSpacing(1);
    unionLayout->setContentsMargins(0, 0, 0, 0);
    booleanButtonsLayout->addLayout(unionLayout);
    
    // Intersection button with label below
    QVBoxLayout* intersectionLayout = new QVBoxLayout();
    QToolButton* intersectionBtn = new QToolButton();
    intersectionBtn->setDefaultAction(m_booleanIntersectionAction);
    intersectionBtn->setToolButtonStyle(Qt::ToolButtonIconOnly);
    intersectionBtn->setIconSize(QSize(30, 30));
    intersectionBtn->setFixedSize(30, 30);
    intersectionBtn->setStyleSheet("QToolButton { border-radius: 8px; border: 1px solid #ccc; background-color: #f0f0f0; } QToolButton:hover { background-color: #e0e0e0; } QToolButton:pressed { background-color: #d0d0d0; }");
    QLabel* intersectionLabel = new QLabel("相交");
    intersectionLabel->setAlignment(Qt::AlignCenter);
    intersectionLabel->setStyleSheet("font-size: 9px; color: #333; margin-top: 2px;");
    intersectionLayout->addWidget(intersectionBtn);
    intersectionLayout->addWidget(intersectionLabel);
    intersectionLayout->setSpacing(1);
    intersectionLayout->setContentsMargins(0, 0, 0, 0);
    booleanButtonsLayout->addLayout(intersectionLayout);
    
    // Difference button with label below
    QVBoxLayout* differenceLayout = new QVBoxLayout();
    QToolButton* differenceBtn = new QToolButton();
    differenceBtn->setDefaultAction(m_booleanDifferenceAction);
    differenceBtn->setToolButtonStyle(Qt::ToolButtonIconOnly);
    differenceBtn->setIconSize(QSize(30, 30));
    differenceBtn->setFixedSize(30, 30);
    differenceBtn->setStyleSheet("QToolButton { border-radius: 8px; border: 1px solid #ccc; background-color: #f0f0f0; } QToolButton:hover { background-color: #e0e0e0; } QToolButton:pressed { background-color: #d0d0d0; }");
    QLabel* differenceLabel = new QLabel("切除");
    differenceLabel->setAlignment(Qt::AlignCenter);
    differenceLabel->setStyleSheet("font-size: 9px; color: #333; margin-top: 2px;");
    differenceLayout->addWidget(differenceBtn);
    differenceLayout->addWidget(differenceLabel);
    differenceLayout->setSpacing(1);
    differenceLayout->setContentsMargins(0, 0, 0, 0);
    booleanButtonsLayout->addLayout(differenceLayout);
    
    booleanLayout->addLayout(booleanButtonsLayout);
    modifyLayout->addWidget(booleanFrame);
    
    // Modifications group
    QFrame* modificationsFrame = new QFrame();
    modificationsFrame->setFrameStyle(QFrame::StyledPanel);
    QVBoxLayout* modificationsLayout = new QVBoxLayout(modificationsFrame);
    modificationsLayout->setContentsMargins(5, 3, 5, 8);
    modificationsLayout->setSpacing(1);
    
    QLabel* modificationsLabel = new QLabel("修改");
    modificationsLabel->setAlignment(Qt::AlignCenter);
    modificationsLayout->addWidget(modificationsLabel);
    QHBoxLayout* modificationsButtonsLayout = new QHBoxLayout();
    modificationsButtonsLayout->setSpacing(8);
    
    // Fillet button with label below
    QVBoxLayout* filletLayout = new QVBoxLayout();
    QToolButton* filletBtn = new QToolButton();
    filletBtn->setDefaultAction(m_filletAction);
    filletBtn->setToolButtonStyle(Qt::ToolButtonIconOnly);
    filletBtn->setIconSize(QSize(30, 30));
    filletBtn->setFixedSize(30, 30);
    filletBtn->setStyleSheet("QToolButton { border-radius: 8px; border: 1px solid #ccc; background-color: #f0f0f0; } QToolButton:hover { background-color: #e0e0e0; } QToolButton:pressed { background-color: #d0d0d0; }");
    QLabel* filletLabel = new QLabel("圆角");
    filletLabel->setAlignment(Qt::AlignCenter);
    filletLabel->setStyleSheet("font-size: 9px; color: #333; margin-top: 2px;");
    filletLayout->addWidget(filletBtn);
    filletLayout->addWidget(filletLabel);
    filletLayout->setSpacing(1);
    filletLayout->setContentsMargins(0, 0, 0, 0);
    modificationsButtonsLayout->addLayout(filletLayout);
    
    // Chamfer button with label below
    QVBoxLayout* chamferLayout = new QVBoxLayout();
    QToolButton* chamferBtn = new QToolButton();
    chamferBtn->setDefaultAction(m_chamferAction);
    chamferBtn->setToolButtonStyle(Qt::ToolButtonIconOnly);
    chamferBtn->setIconSize(QSize(30, 30));
    chamferBtn->setFixedSize(30, 30);
    chamferBtn->setStyleSheet("QToolButton { border-radius: 8px; border: 1px solid #ccc; background-color: #f0f0f0; } QToolButton:hover { background-color: #e0e0e0; } QToolButton:pressed { background-color: #d0d0d0; }");
    QLabel* chamferLabel = new QLabel("倒角");
    chamferLabel->setAlignment(Qt::AlignCenter);
    chamferLabel->setStyleSheet("font-size: 9px; color: #333; margin-top: 2px;");
    chamferLayout->addWidget(chamferBtn);
    chamferLayout->addWidget(chamferLabel);
    chamferLayout->setSpacing(1);
    chamferLayout->setContentsMargins(0, 0, 0, 0);
    modificationsButtonsLayout->addLayout(chamferLayout);

    // Hole button with label below
    QVBoxLayout* holeLayout = new QVBoxLayout();
    QToolButton* holeBtn = new QToolButton();
    holeBtn->setDefaultAction(m_createHoleAction);
    holeBtn->setToolButtonStyle(Qt::ToolButtonIconOnly);
    holeBtn->setIconSize(QSize(30, 30));
    holeBtn->setFixedSize(30, 30);
    holeBtn->setStyleSheet("QToolButton { border-radius: 8px; border: 1px solid #ccc; background-color: #f0f0f0; } QToolButton:hover { background-color: #e0e0e0; } QToolButton:pressed { background-color: #d0d0d0; }");
    QLabel* holeLabel = new QLabel("挖孔");
    holeLabel->setAlignment(Qt::AlignCenter);
    holeLabel->setStyleSheet("font-size: 9px; color: #333; margin-top: 2px;");
    holeLayout->addWidget(holeBtn);
    holeLayout->addWidget(holeLabel);
    holeLayout->setSpacing(1);
    holeLayout->setContentsMargins(0, 0, 0, 0);
    modificationsButtonsLayout->addLayout(holeLayout);
    
    // Transform button with label below
    QVBoxLayout* transformLayout = new QVBoxLayout();
    QToolButton* transformBtn = new QToolButton();
    transformBtn->setDefaultAction(m_transformAction);
    transformBtn->setToolButtonStyle(Qt::ToolButtonIconOnly);
    transformBtn->setFixedSize(30, 30);
    transformBtn->setStyleSheet("QToolButton { border-radius: 8px; border: 1px solid #ccc; background-color: #f0f0f0; } QToolButton:hover { background-color: #e0e0e0; } QToolButton:pressed { background-color: #d0d0d0; }");
    QLabel* transformLabel = new QLabel("变换");
    transformLabel->setAlignment(Qt::AlignCenter);
    transformLabel->setStyleSheet("font-size: 9px; color: #333; margin-top: 2px;");
    transformLayout->addWidget(transformBtn);
    transformLayout->addWidget(transformLabel);
    transformLayout->setSpacing(1);
    transformLayout->setContentsMargins(0, 0, 0, 0);
    modificationsButtonsLayout->addLayout(transformLayout);
    
    modificationsLayout->addLayout(modificationsButtonsLayout);
    modifyLayout->addWidget(modificationsFrame);
    
    modifyLayout->addStretch();
    toolTabWidget->addTab(modifyTab, "修改");
    
    // View Tab - View controls and selection
    QWidget* viewTab = new QWidget();
    QHBoxLayout* viewLayout = new QHBoxLayout(viewTab);
    viewLayout->setContentsMargins(5, 2, 5, 2);
    viewLayout->setSpacing(3);
    
    // Selection group
    QFrame* selectionFrame = new QFrame();
    selectionFrame->setFrameStyle(QFrame::StyledPanel);
    QVBoxLayout* selectionLayout = new QVBoxLayout(selectionFrame);
    selectionLayout->setContentsMargins(2, 1, 2, 2);
    selectionLayout->setSpacing(1);
    
    QLabel* selectionLabel = new QLabel("选择");
    selectionLabel->setAlignment(Qt::AlignCenter);
    selectionLayout->addWidget(selectionLabel);
    
    // Add selection mode combo box
    if (m_selectionModeCombo) {
        m_selectionModeCombo->setMinimumWidth(100);
        selectionLayout->addWidget(m_selectionModeCombo);
    }
    
    viewLayout->addWidget(selectionFrame);
    
    // View controls group
    QFrame* viewControlsFrame = new QFrame();
    viewControlsFrame->setFrameStyle(QFrame::StyledPanel);
    QVBoxLayout* viewControlsLayout = new QVBoxLayout(viewControlsFrame);
    viewControlsLayout->setContentsMargins(2, 1, 2, 2);
    viewControlsLayout->setSpacing(1);
    
    QLabel* viewControlsLabel = new QLabel("视图");
    viewControlsLabel->setAlignment(Qt::AlignCenter);
    viewControlsLayout->addWidget(viewControlsLabel);
    QHBoxLayout* viewControlsButtonsLayout = new QHBoxLayout();
    viewControlsButtonsLayout->setSpacing(2);
    
    QToolButton* fitAllBtn = new QToolButton();
    fitAllBtn->setDefaultAction(m_fitAllAction);
    fitAllBtn->setToolButtonStyle(Qt::ToolButtonTextUnderIcon);
    viewControlsButtonsLayout->addWidget(fitAllBtn);
    
    QToolButton* wireframeBtn = new QToolButton();
    wireframeBtn->setDefaultAction(m_viewWireframeAction);
    wireframeBtn->setToolButtonStyle(Qt::ToolButtonTextUnderIcon);
    viewControlsButtonsLayout->addWidget(wireframeBtn);
    
    QToolButton* shadedBtn = new QToolButton();
    shadedBtn->setDefaultAction(m_viewShadedAction);
    shadedBtn->setToolButtonStyle(Qt::ToolButtonTextUnderIcon);
    viewControlsButtonsLayout->addWidget(shadedBtn);
    
    viewControlsLayout->addLayout(viewControlsButtonsLayout);
    viewLayout->addWidget(viewControlsFrame);
    
    viewLayout->addStretch();
    toolTabWidget->addTab(viewTab, "视图");
    
    // Sketch Tab - Sketch mode controls
    QWidget* sketchTab = new QWidget();
    QHBoxLayout* sketchLayout = new QHBoxLayout(sketchTab);
    sketchLayout->setContentsMargins(5, 2, 5, 2);
    sketchLayout->setSpacing(3);
    
    // Sketch mode group
    QFrame* sketchModeFrame = new QFrame();
    sketchModeFrame->setFrameStyle(QFrame::StyledPanel);
    QVBoxLayout* sketchModeLayout = new QVBoxLayout(sketchModeFrame);
    sketchModeLayout->setContentsMargins(2, 1, 2, 2);
    sketchModeLayout->setSpacing(1);
    
    QLabel* sketchModeLabel = new QLabel("草图模式");
    sketchModeLabel->setAlignment(Qt::AlignCenter);
    sketchModeLayout->addWidget(sketchModeLabel);
    QHBoxLayout* sketchModeButtonsLayout = new QHBoxLayout();
    sketchModeButtonsLayout->setSpacing(2);
    
    QToolButton* enterSketchBtn = new QToolButton();
    enterSketchBtn->setDefaultAction(m_enterSketchAction);
    enterSketchBtn->setToolButtonStyle(Qt::ToolButtonTextUnderIcon);
    sketchModeButtonsLayout->addWidget(enterSketchBtn);
    
    QToolButton* exitSketchBtn = new QToolButton();
    exitSketchBtn->setDefaultAction(m_exitSketchAction);
    exitSketchBtn->setToolButtonStyle(Qt::ToolButtonTextUnderIcon);
    sketchModeButtonsLayout->addWidget(exitSketchBtn);
    
    sketchModeLayout->addLayout(sketchModeButtonsLayout);
    sketchLayout->addWidget(sketchModeFrame);
    
    // Sketch tools group
    QFrame* sketchToolsFrame = new QFrame();
    sketchToolsFrame->setFrameStyle(QFrame::StyledPanel);
    QVBoxLayout* sketchToolsLayout = new QVBoxLayout(sketchToolsFrame);
    sketchToolsLayout->setContentsMargins(2, 1, 2, 2);
    sketchToolsLayout->setSpacing(1);
    
    QLabel* sketchToolsLabel = new QLabel("绘制工具");
    sketchToolsLabel->setAlignment(Qt::AlignCenter);
    sketchToolsLayout->addWidget(sketchToolsLabel);
    QHBoxLayout* sketchToolsButtonsLayout = new QHBoxLayout();
    sketchToolsButtonsLayout->setSpacing(2);
    
    QToolButton* rectangleBtn = new QToolButton();
    rectangleBtn->setDefaultAction(m_sketchRectangleAction);
    rectangleBtn->setToolButtonStyle(Qt::ToolButtonTextUnderIcon);
    sketchToolsButtonsLayout->addWidget(rectangleBtn);
    
    sketchToolsLayout->addLayout(sketchToolsButtonsLayout);
    sketchLayout->addWidget(sketchToolsFrame);
    
    sketchLayout->addStretch();
    toolTabWidget->addTab(sketchTab, "草图");
    
    // Set layout for toolbar area
    QVBoxLayout* toolBarAreaLayout = new QVBoxLayout(toolBarArea);
    toolBarAreaLayout->setContentsMargins(0, 0, 0, 0);
    toolBarAreaLayout->addWidget(toolTabWidget);
    
    // Add toolbar area as a toolbar to maintain proper positioning
    QToolBar* containerToolBar = addToolBar("Container");
    containerToolBar->addWidget(toolBarArea);
    containerToolBar->setMovable(false);
    containerToolBar->setObjectName("containerToolBar");
    
    // Set selection mode buttons object names for styling
    // Old selection mode button styling removed - now using combo box
    
    // Set boolean operation buttons object names
    m_booleanUnionAction->setObjectName("booleanButton");
    m_booleanIntersectionAction->setObjectName("booleanButton");
    m_booleanDifferenceAction->setObjectName("booleanButton");
    
    // Set modify operation buttons object names
    m_filletAction->setObjectName("modifyButton");
    m_chamferAction->setObjectName("modifyButton");
}

void MainWindow::CreateStatusBar() {
    statusBar()->showMessage("Ready");
}

void MainWindow::CreateDockWidgets() {
    // Document tree dock
    m_documentDock = new QDockWidget("Document Tree", this);
    m_documentTree = new DocumentTree(this);
    m_documentDock->setWidget(m_documentTree);
    addDockWidget(Qt::LeftDockWidgetArea, m_documentDock);
    
    // Property panel dock
    m_propertyDock = new QDockWidget("Properties", this);
    m_propertyPanel = new PropertyPanel(this);
    m_propertyDock->setWidget(m_propertyPanel);
    addDockWidget(Qt::RightDockWidgetArea, m_propertyDock);
}

void MainWindow::ConnectSignals() {
    // File actions
    connect(m_newAction, &QAction::triggered, this, &MainWindow::OnNewDocument);
    connect(m_openAction, &QAction::triggered, this, &MainWindow::OnOpenDocument);
    connect(m_saveAction, &QAction::triggered, this, &MainWindow::OnSaveDocument);
    connect(m_saveAsAction, &QAction::triggered, this, &MainWindow::OnSaveDocumentAs);
    connect(m_exitAction, &QAction::triggered, this, &MainWindow::OnExit);
    
    // Edit actions
    connect(m_undoAction, &QAction::triggered, this, &MainWindow::OnUndo);
    connect(m_redoAction, &QAction::triggered, this, &MainWindow::OnRedo);
    
    // View actions
    connect(m_fitAllAction, &QAction::triggered, this, &MainWindow::OnFitAll);
    connect(m_zoomInAction, &QAction::triggered, this, &MainWindow::OnZoomIn);
    connect(m_zoomOutAction, &QAction::triggered, this, &MainWindow::OnZoomOut);
    connect(m_viewWireframeAction, &QAction::triggered, this, &MainWindow::OnViewWireframe);
    connect(m_viewShadedAction, &QAction::triggered, this, &MainWindow::OnViewShaded);
    connect(m_viewOrthographicAction, &QAction::triggered, this, &MainWindow::OnViewOrthographic);
    connect(m_viewPerspectiveAction, &QAction::triggered, this, &MainWindow::OnViewPerspective);
    
    // Create actions
    connect(m_createBoxAction, &QAction::triggered, this, &MainWindow::OnCreateBox);
    connect(m_createCylinderAction, &QAction::triggered, this, &MainWindow::OnCreateCylinder);
    connect(m_createSphereAction, &QAction::triggered, this, &MainWindow::OnCreateSphere);
    connect(m_createExtrudeAction, &QAction::triggered, this, &MainWindow::OnCreateExtrude);
    
    // Boolean actions
    connect(m_booleanUnionAction, &QAction::triggered, this, &MainWindow::OnBooleanUnion);
    connect(m_booleanIntersectionAction, &QAction::triggered, this, &MainWindow::OnBooleanIntersection);
    connect(m_booleanDifferenceAction, &QAction::triggered, this, &MainWindow::OnBooleanDifference);
    
    // Modify actions
    connect(m_filletAction, &QAction::triggered, this, &MainWindow::OnFillet);
    connect(m_chamferAction, &QAction::triggered, this, &MainWindow::OnChamfer);
	connect(m_createHoleAction, &QAction::triggered, this, &MainWindow::OnCreateHole);
    
    // Transform operations
    connect(m_transformAction, &QAction::triggered, this, &MainWindow::OnTransformObjects);
    
    // Sketch actions
    connect(m_enterSketchAction, &QAction::triggered, this, &MainWindow::OnEnterSketchMode);
    connect(m_exitSketchAction, &QAction::triggered, this, &MainWindow::OnExitSketchMode);
    connect(m_sketchRectangleAction, &QAction::triggered, this, &MainWindow::OnSketchRectangleTool);
    
    // Selection mode combo box connected in CreateSelectionModeCombo()
    
    // Theme actions
    connect(m_darkThemeAction, &QAction::triggered, this, &MainWindow::OnDarkTheme);
    connect(m_lightThemeAction, &QAction::triggered, this, &MainWindow::OnLightTheme);
    
    // Help actions
    connect(m_aboutAction, &QAction::triggered, this, &MainWindow::OnAbout);
    connect(m_aboutQtAction, &QAction::triggered, this, &MainWindow::OnAboutQt);
    
    // Viewer signals
    connect(m_viewer, &QtOccView::ShapeSelected, this, &MainWindow::OnShapeSelected);
    connect(m_viewer, &QtOccView::ViewChanged, this, &MainWindow::OnViewChanged);
    connect(m_viewer, &QtOccView::FaceSelected, this, &MainWindow::OnFaceSelected);
    connect(m_viewer, &QtOccView::SketchModeEntered, this, &MainWindow::OnSketchModeEntered);
    connect(m_viewer, &QtOccView::SketchModeExited, this, &MainWindow::OnSketchModeExited);
    
    // Mouse position signals
    //connect(m_viewer, &QtOccView::MousePositionChanged, m_statusBar, &StatusBar::updateMousePosition2D);
    //connect(m_viewer, &QtOccView::Mouse3DPositionChanged, m_statusBar, &StatusBar::updateMousePosition);
    
    // Document tree signals for selection synchronization
    connect(m_documentTree, &DocumentTree::ShapeSelected, this, &MainWindow::OnDocumentTreeShapeSelected);
    connect(m_documentTree, &DocumentTree::FeatureSelected, this, &MainWindow::OnDocumentTreeFeatureSelected);
}

void MainWindow::UpdateActions() {
    bool hasDocument = !m_currentFileName.isEmpty();
    bool canUndo = m_ocafManager->CanUndo();
    bool canRedo = m_ocafManager->CanRedo();
    
    m_saveAction->setEnabled(hasDocument && m_documentModified);
    m_saveAsAction->setEnabled(hasDocument);
    m_undoAction->setEnabled(canUndo);
    m_redoAction->setEnabled(canRedo);
    
    // Update action text based on availability
    m_undoAction->setText(canUndo ? "&Undo" : "&Undo");
    m_redoAction->setText(canRedo ? "&Redo" : "&Redo");
}

void MainWindow::RefreshUIFromOCAF() {
    if (!m_ocafManager) {
        return;
    }
    
    qDebug() << "Refreshing UI from OCAF document state";
    
    // Clear current UI state
    m_viewer->ClearShapes();
    m_documentTree->Clear();
    
    // Reload all shapes from OCAF document
    auto allShapes = m_ocafManager->GetAllShapes();
    qDebug() << "Found" << allShapes.size() << "shapes in OCAF document";
    
    for (const auto& shape : allShapes) {
        if (shape) {
            // Display in 3D viewer
            m_viewer->DisplayShape(shape);
            // Add to document tree
            m_documentTree->AddShape(shape);
        }
    }
    
    // Clear any selections
    m_viewer->ClearSelection();
    m_viewer->ClearEdgeSelection();
    
    // Force redraw
    m_viewer->RedrawAll();
    
    qDebug() << "UI refresh completed";
}

void MainWindow::UpdateWindowTitle() {
    QString title = "Ander CAD";
    if (!m_currentFileName.isEmpty()) {
        title += " - " + QFileInfo(m_currentFileName).baseName();
        if (m_documentModified) {
            title += " *";
        }
    }
    setWindowTitle(title);
}

void MainWindow::closeEvent(QCloseEvent* event) {
    if (SaveChanges()) {
        event->accept();
    } else {
        event->ignore();
    }
}

void MainWindow::resizeEvent(QResizeEvent* event) {
    QMainWindow::resizeEvent(event);
    
    // Adjust title bar size
    if (m_titleBar) {
        m_titleBar->resize(width(), 40);
    }
    
    // Adjust menu bar position to be below title bar
    if (menuBar()) {
        menuBar()->move(0, 40);
        menuBar()->resize(width(), menuBar()->height());
    }
}

bool MainWindow::SaveChanges() {
    if (m_documentModified) {
        QMessageBox::StandardButton result = QMessageBox::question(this, 
            "Save Changes", 
            "The document has been modified. Do you want to save your changes?",
            QMessageBox::Save | QMessageBox::Discard | QMessageBox::Cancel);
        
        if (result == QMessageBox::Save) {
            return OnSaveDocument();
        } else if (result == QMessageBox::Cancel) {
            return false;
        }
    }
    return true;
}

void MainWindow::SetDocumentModified(bool modified) {
    m_documentModified = modified;
    UpdateActions();
    UpdateWindowTitle();
}

// Slot implementations
void MainWindow::OnNewDocument() {
    NewDocumentTab();
}

void MainWindow::OnOpenDocument() {
    // Placeholder implementation
    QMessageBox::information(this, "Open Document", "Open document functionality not implemented yet");
}

bool MainWindow::OnSaveDocument() {
    if (m_currentFileName.isEmpty()) {
        return OnSaveDocumentAs();
    }
    // Placeholder implementation
    SetDocumentModified(false);
    return true;
}

bool MainWindow::OnSaveDocumentAs() {
    // Placeholder implementation
    QString fileName = QFileDialog::getSaveFileName(this, "Save Document", "", "CAD Files (*.cad)");
    if (!fileName.isEmpty()) {
        m_currentFileName = fileName;
        SetDocumentModified(false);
        return true;
    }
    return false;
}

void MainWindow::OnExit() {
    close();
}

void MainWindow::OnUndo() {
    qDebug() << "=== OnUndo TRIGGERED ===";
    qDebug() << "OnUndo called - checking undo availability:" << m_ocafManager->CanUndo();
    if (m_ocafManager->Undo()) {
        qDebug() << "Undo operation successful, refreshing UI";
        // Refresh UI from OCAF document state
        RefreshUIFromOCAF();
        SetDocumentModified(true);
        UpdateActions();
        statusBar()->showMessage("Undo completed", 2000);
    } else {
        qDebug() << "Undo operation failed - available undos:" << m_ocafManager->CanUndo();
        statusBar()->showMessage("Cannot undo", 2000);
    }
}

void MainWindow::OnRedo() {
    qDebug() << "=== OnRedo TRIGGERED ===";
    qDebug() << "OnRedo called - checking redo availability:" << m_ocafManager->CanRedo();
    if (m_ocafManager->Redo()) {
        qDebug() << "Redo operation successful, refreshing UI";
        // Refresh UI from OCAF document state
        RefreshUIFromOCAF();
        SetDocumentModified(true);
        UpdateActions();
        statusBar()->showMessage("Redo completed", 2000);
    } else {
        qDebug() << "Redo operation failed - available redos:" << m_ocafManager->CanRedo();
        statusBar()->showMessage("Cannot redo", 2000);
    }
}

void MainWindow::OnFitAll() {
    m_viewer->FitAll();
    m_viewer->RedrawAll();
}

void MainWindow::OnZoomIn() {
    m_viewer->ZoomIn();
}

void MainWindow::OnZoomOut() {
    m_viewer->ZoomOut();
}

void MainWindow::OnViewWireframe() {
    m_viewer->SetViewMode("wireframe");
}

void MainWindow::OnViewShaded() {
    m_viewer->SetViewMode("shaded");
}

void MainWindow::OnViewOrthographic() {
    m_viewer->SetProjectionMode(true);
}

void MainWindow::OnViewPerspective() {
    m_viewer->SetProjectionMode(false);
}

void MainWindow::OnCreateBox() {
    CreateBoxDialog dialog(this);
    if (dialog.exec() == QDialog::Accepted) {
        double width = dialog.GetWidth();
        double height = dialog.GetHeight();
        double depth = dialog.GetDepth();
        
        // Start OCAF transaction
        m_ocafManager->StartTransaction("Create Box");
        
        // Create the shape using ShapeFactory
        auto shape = cad_core::ShapeFactory::CreateBox(width, height, depth);
        if (shape) {
            // Add shape to OCAF document
            if (m_ocafManager->AddShape(shape, "Box")) {
                // Display the shape
                m_viewer->DisplayShape(shape);
                m_documentTree->AddShape(shape);
                
                // Commit the transaction
                m_ocafManager->CommitTransaction();
                SetDocumentModified(true);
                UpdateActions();
            } else {
                m_ocafManager->AbortTransaction();
                QMessageBox::warning(this, "Error", "Failed to add box to document.");
            }
        } else {
            m_ocafManager->AbortTransaction();
            QMessageBox::warning(this, "Error", "Failed to create box. Check parameters.");
        }
    }
}

void MainWindow::OnCreateCylinder() {
    CreateCylinderDialog dialog(this);
    if (dialog.exec() == QDialog::Accepted) {
        double radius = dialog.GetRadius();
        double height = dialog.GetHeight();
        
        // Start OCAF transaction
        m_ocafManager->StartTransaction("Create Cylinder");
        
        // Create the shape using ShapeFactory
        auto shape = cad_core::ShapeFactory::CreateCylinder(radius, height);
        if (shape) {
            // Add shape to OCAF document
            if (m_ocafManager->AddShape(shape, "Cylinder")) {
                // Display the shape
                m_viewer->DisplayShape(shape);
                m_documentTree->AddShape(shape);
                
                // Commit the transaction
                m_ocafManager->CommitTransaction();
                SetDocumentModified(true);
                UpdateActions();
            } else {
                m_ocafManager->AbortTransaction();
                QMessageBox::warning(this, "Error", "Failed to add cylinder to document.");
            }
        } else {
            m_ocafManager->AbortTransaction();
            QMessageBox::warning(this, "Error", "Failed to create cylinder. Check parameters.");
        }
    }
}

void MainWindow::OnCreateSphere() {
    CreateSphereDialog dialog(this);
    if (dialog.exec() == QDialog::Accepted) {
        double radius = dialog.GetRadius();
        
        // Start OCAF transaction
        m_ocafManager->StartTransaction("Create Sphere");
        
        // Create the shape using ShapeFactory
        auto shape = cad_core::ShapeFactory::CreateSphere(radius);
        if (shape) {
            // Add shape to OCAF document
            if (m_ocafManager->AddShape(shape, "Sphere")) {
                // Display the shape
                m_viewer->DisplayShape(shape);
                m_documentTree->AddShape(shape);
                
                // Commit the transaction
                m_ocafManager->CommitTransaction();
                SetDocumentModified(true);
                UpdateActions();
            } else {
                m_ocafManager->AbortTransaction();
                QMessageBox::warning(this, "Error", "Failed to add sphere to document.");
            }
        } else {
            m_ocafManager->AbortTransaction();
            QMessageBox::warning(this, "Error", "Failed to create sphere. Check parameters.");
        }
    }
}

void MainWindow::OnCreateExtrude() {
    QMessageBox::information(this, "Create Extrude", "Extrude feature creation not implemented yet");
}

void MainWindow::OnDarkTheme() {
    m_themeManager->SetTheme("dark");
}

void MainWindow::OnLightTheme() {
    m_themeManager->SetTheme("light");
}

void MainWindow::OnAbout() {
    AboutDialog dialog(this);
    dialog.exec();
}

void MainWindow::OnAboutQt() {
    QMessageBox::aboutQt(this);
}

void MainWindow::OnShapeSelected(const cad_core::ShapePtr& shape) {
    // Update property panel with selected shape
    m_propertyPanel->SetShape(shape);
    
    // Forward selection to active dialogs
    OnObjectSelected(shape);
}

void MainWindow::OnViewChanged() {
    // Handle view changes
}

// Document tree selection handlers
void MainWindow::OnDocumentTreeShapeSelected(const cad_core::ShapePtr& shape) {
    // When a shape is selected in the document tree, select it in the 3D viewer
    if (m_viewer && shape) {
        m_viewer->SelectShape(shape);
        m_propertyPanel->SetShape(shape);
    }
}

void MainWindow::OnDocumentTreeFeatureSelected(const cad_feature::FeaturePtr& feature) {
    // Handle feature selection from document tree
    if (feature) {
        // Update property panel to show feature properties
        // This would require extending PropertyPanel to handle features
        qDebug() << "Feature selected:" << QString::fromStdString(feature->GetName());
    }
}

// Missing slot implementations
void MainWindow::OnCut() {
    // Cut implementation placeholder
}

void MainWindow::OnCopy() {
    // Copy implementation placeholder
}

void MainWindow::OnPaste() {
    // Paste implementation placeholder
}

void MainWindow::OnDelete() {
    // Delete implementation placeholder
}

void MainWindow::OnSelectAll() {
    // Select all implementation placeholder
}

void MainWindow::OnCreateRevolve() {
    QMessageBox::information(this, "Create Revolve", "Revolve feature creation not implemented yet");
}

void MainWindow::OnCreateSweep() {
    QMessageBox::information(this, "Create Sweep", "Sweep feature creation not implemented yet");
}

void MainWindow::OnCreateLoft() {
    QMessageBox::information(this, "Create Loft", "Loft feature creation not implemented yet");
}

void MainWindow::OnImportSTEP() {
    QMessageBox::information(this, "Import STEP", "STEP import not implemented yet");
}

void MainWindow::OnImportIGES() {
    QMessageBox::information(this, "Import IGES", "IGES import not implemented yet");
}

void MainWindow::OnExportSTEP() {
    QMessageBox::information(this, "Export STEP", "STEP export not implemented yet");
}

void MainWindow::OnExportIGES() {
    QMessageBox::information(this, "Export IGES", "IGES export not implemented yet");
}

void MainWindow::OnExportSTL() {
    QMessageBox::information(this, "Export STL", "STL export not implemented yet");
}

void MainWindow::OnShowGrid() {
    // Toggle grid visibility
    static bool gridVisible = false;
    gridVisible = !gridVisible;
    m_viewer->ShowGrid(gridVisible);
}

void MainWindow::OnShowAxes() {
    // Toggle axes visibility
    static bool axesVisible = true;
    axesVisible = !axesVisible;
    m_viewer->ShowAxes(axesVisible);
}

void MainWindow::SetTheme(const QString& theme) {
    m_themeManager->SetTheme(theme);
}

// Boolean operations
void MainWindow::OnBooleanUnion() {
    // Clean up any existing dialog
    if (m_currentBooleanDialog) {
        m_currentBooleanDialog->deleteLater();
        m_currentBooleanDialog = nullptr;
    }
    
    // Create and show dialog
    m_currentBooleanDialog = new BooleanOperationDialog(BooleanOperationType::Union, this);
    
    // Connect dialog signals
    connect(m_currentBooleanDialog, &BooleanOperationDialog::selectionModeChanged,
            this, &MainWindow::OnSelectionModeChanged);
    connect(m_currentBooleanDialog, &BooleanOperationDialog::operationRequested,
            this, &MainWindow::OnBooleanOperationRequested);
    
    m_currentBooleanDialog->show();
    m_currentBooleanDialog->raise();
    m_currentBooleanDialog->activateWindow();
}

void MainWindow::OnBooleanIntersection() {
    // Clean up any existing dialog
    if (m_currentBooleanDialog) {
        m_currentBooleanDialog->deleteLater();
        m_currentBooleanDialog = nullptr;
    }
    
    // Create and show dialog
    m_currentBooleanDialog = new BooleanOperationDialog(BooleanOperationType::Intersection, this);
    
    // Connect dialog signals
    connect(m_currentBooleanDialog, &BooleanOperationDialog::selectionModeChanged,
            this, &MainWindow::OnSelectionModeChanged);
    connect(m_currentBooleanDialog, &BooleanOperationDialog::operationRequested,
            this, &MainWindow::OnBooleanOperationRequested);
    
    m_currentBooleanDialog->show();
    m_currentBooleanDialog->raise();
    m_currentBooleanDialog->activateWindow();
}

void MainWindow::OnBooleanDifference() {
    // Clean up any existing dialog
    if (m_currentBooleanDialog) {
        m_currentBooleanDialog->deleteLater();
        m_currentBooleanDialog = nullptr;
    }
    
    // Create and show dialog
    m_currentBooleanDialog = new BooleanOperationDialog(BooleanOperationType::Difference, this);
    
    // Connect dialog signals
    connect(m_currentBooleanDialog, &BooleanOperationDialog::selectionModeChanged,
            this, &MainWindow::OnSelectionModeChanged);
    connect(m_currentBooleanDialog, &BooleanOperationDialog::operationRequested,
            this, &MainWindow::OnBooleanOperationRequested);
    
    m_currentBooleanDialog->show();
    m_currentBooleanDialog->raise();
    m_currentBooleanDialog->activateWindow();
}

// Modify operations
void MainWindow::OnFillet() {
    // Clean up any existing dialog
    if (m_currentFilletChamferDialog) {
        m_currentFilletChamferDialog->deleteLater();
        m_currentFilletChamferDialog = nullptr;
    }
    
    // Create and show dialog
    m_currentFilletChamferDialog = new FilletChamferDialog(FilletChamferType::Fillet, m_viewer, this);
    
    // Connect dialog signals
    connect(m_currentFilletChamferDialog, &FilletChamferDialog::selectionModeChanged,
            this, &MainWindow::OnSelectionModeChanged);
    connect(m_currentFilletChamferDialog, &FilletChamferDialog::operationRequested,
            this, &MainWindow::OnFilletChamferOperationRequested);
    
    m_currentFilletChamferDialog->show();
    m_currentFilletChamferDialog->raise();
    m_currentFilletChamferDialog->activateWindow();
}

void MainWindow::OnChamfer() {
    // Clean up any existing dialog
    if (m_currentFilletChamferDialog) {
        m_currentFilletChamferDialog->deleteLater();
        m_currentFilletChamferDialog = nullptr;
    }
    
    // Create and show dialog
    m_currentFilletChamferDialog = new FilletChamferDialog(FilletChamferType::Chamfer, m_viewer, this);
    
    // Connect dialog signals
    connect(m_currentFilletChamferDialog, &FilletChamferDialog::selectionModeChanged,
            this, &MainWindow::OnSelectionModeChanged);
    connect(m_currentFilletChamferDialog, &FilletChamferDialog::operationRequested,
            this, &MainWindow::OnFilletChamferOperationRequested);
    
    m_currentFilletChamferDialog->show();
    m_currentFilletChamferDialog->raise();
    m_currentFilletChamferDialog->activateWindow();
}

void MainWindow::OnCreateHole() {
    if (m_currentHoleDialog) {
        m_currentHoleDialog->close();
        return;
    }

    // 创建对话框时传入 viewer 指针
    m_currentHoleDialog = new CreateHoleDialog(m_viewer, this);
    connect(m_viewer, &QtOccView::FaceSelected, m_currentHoleDialog, &CreateHoleDialog::onFaceSelected);
    connect(m_currentHoleDialog, &CreateHoleDialog::selectionModeChanged, this, &MainWindow::OnSelectionModeChanged);
    connect(m_currentHoleDialog, &CreateHoleDialog::operationRequested, this, &MainWindow::OnHoleOperationRequested);
    connect(m_currentHoleDialog, &CreateHoleDialog::previewRequested, this, &MainWindow::OnHolePreviewRequested);
    connect(m_currentHoleDialog, &CreateHoleDialog::resetPreviewRequested, this, &MainWindow::OnHoleResetPreviewRequested);
    connect(this, &MainWindow::faceSelectionInfo, m_currentHoleDialog, &CreateHoleDialog::updateCenterCoords);

    connect(m_currentHoleDialog, &QDialog::finished, this, [this](int result) {
        if (m_currentHoleDialog) {
            // 调用清理函数恢复视图
            m_currentHoleDialog->cleanupAndRestoreView();
            m_currentHoleDialog->deleteLater();
            m_currentHoleDialog = nullptr;
        }
        // 恢复主窗口的默认选择模式
        OnSelectionModeChanged(false, "");
        statusBar()->showMessage("Ready");
        });

    m_currentHoleDialog->show();
    m_currentHoleDialog->raise();
    m_currentHoleDialog->activateWindow();
}


// Selection mode combo box
void MainWindow::OnSelectionModeComboChanged(int index) {
    if (!m_selectionModeCombo) return;
    
    // Get the selection mode from combo box data
    cad_core::SelectionMode mode = static_cast<cad_core::SelectionMode>(
        m_selectionModeCombo->itemData(index).toInt());
    
    // Convert to OpenCASCADE selection mode integers
    int occSelectionMode = 0; // Default to shape
    switch (mode) {
        case cad_core::SelectionMode::Shape:
            occSelectionMode = 0;
            break;
        case cad_core::SelectionMode::Vertex:
            occSelectionMode = 1;
            break;
        case cad_core::SelectionMode::Edge:
            occSelectionMode = 2;
            break;
        case cad_core::SelectionMode::Face:
            occSelectionMode = 4;
            break;
    }
    
    // Set the selection mode in viewer
    m_viewer->SetSelectionMode(occSelectionMode);
    
    // Update status bar
    QString modeText = m_selectionModeCombo->itemText(index);
    statusBar()->showMessage("Selection Mode: " + modeText.replace("Select ", ""));
}

// Tab management
void MainWindow::CloseDocumentTab(int index) {
    if (m_tabWidget->count() <= 1) {
        return; // Keep at least one tab
    }
    
    QtOccView* viewer = qobject_cast<QtOccView*>(m_tabWidget->widget(index));
    if (viewer) {
        // Check for unsaved changes in this tab
        // For now, just close without checking
        m_tabWidget->removeTab(index);
        viewer->deleteLater();
    }
}

void MainWindow::OnTabChanged(int index) {
    if (index >= 0 && index < m_tabWidget->count()) {
        m_viewer = qobject_cast<QtOccView*>(m_tabWidget->widget(index));
        UpdateCurrentDocument();
    }
}

void MainWindow::NewDocumentTab() {
    QString tabName = QString("Document %1").arg(m_tabWidget->count() + 1);
    QtOccView* newViewer = new QtOccView(this);
    newViewer->setObjectName("viewer3D");
    newViewer->InitViewer();
    
    int tabIndex = m_tabWidget->addTab(newViewer, tabName);
    m_tabWidget->setCurrentIndex(tabIndex);
    
    // Connect viewer signals for new tab
    connect(newViewer, &QtOccView::ShapeSelected, this, &MainWindow::OnShapeSelected);
    connect(newViewer, &QtOccView::ViewChanged, this, &MainWindow::OnViewChanged);
}

QtOccView* MainWindow::GetCurrentViewer() const {
    if (m_tabWidget && m_tabWidget->currentIndex() >= 0) {
        return qobject_cast<QtOccView*>(m_tabWidget->currentWidget());
    }
    return nullptr;
}

void MainWindow::UpdateCurrentDocument() {
    m_viewer = GetCurrentViewer();
    UpdateActions();
    UpdateWindowTitle();
}

// Window dragging implementation for frameless window
void MainWindow::mousePressEvent(QMouseEvent* event) {
    if (event->button() == Qt::LeftButton) {
        // Check if click is in the title bar area
        QWidget* widget = childAt(event->pos());
        if (widget == m_titleBar || widget == m_titleLabel) {
            m_isDragging = true;
            m_dragStartPosition = event->globalPos() - frameGeometry().topLeft();
            event->accept();
            return;
        }
    }
    QMainWindow::mousePressEvent(event);
}

void MainWindow::mouseMoveEvent(QMouseEvent* event) {
    if (event->buttons() & Qt::LeftButton && m_isDragging) {
        move(event->globalPos() - m_dragStartPosition);
        event->accept();
        return;
    }
    QMainWindow::mouseMoveEvent(event);
}

void MainWindow::mouseReleaseEvent(QMouseEvent* event) {
    if (event->button() == Qt::LeftButton) {
        m_isDragging = false;
        event->accept();
        return;
    }
    QMainWindow::mouseReleaseEvent(event);
}

// Create custom title bar for frameless window
void MainWindow::CreateTitleBar() {
    m_titleBar = new QWidget(this);
    m_titleBar->setObjectName("titleBar");
    m_titleBar->setFixedHeight(40);
    
    // Create title label
    m_titleLabel = new QLabel("Ander CAD", m_titleBar);
    m_titleLabel->setObjectName("titleLabel");
    
    // Create window control buttons
	// 最小化按钮、最大化按钮、关闭按钮
    m_minimizeButton = new QPushButton("−", m_titleBar);
    m_minimizeButton->setObjectName("minimizeButton");
    m_minimizeButton->setFixedSize(40, 40);
    m_minimizeButton->setToolTip("Minimize");
    
    m_maximizeButton = new QPushButton("□", m_titleBar);
    m_maximizeButton->setObjectName("maximizeButton");
    m_maximizeButton->setFixedSize(40, 40);
    m_maximizeButton->setToolTip("Maximize");
    
    m_closeButton = new QPushButton("✕", m_titleBar);
    m_closeButton->setObjectName("closeButton");
    m_closeButton->setFixedSize(40, 40);
    m_closeButton->setToolTip("Close");
    
    
    // Layout for title bar
    QHBoxLayout* titleLayout = new QHBoxLayout(m_titleBar);
    titleLayout->setContentsMargins(8, 0, 0, 0);
    titleLayout->setSpacing(0);
    titleLayout->addWidget(m_titleLabel);
    titleLayout->addStretch();
    titleLayout->addWidget(m_minimizeButton);
    titleLayout->addWidget(m_maximizeButton);
    titleLayout->addWidget(m_closeButton);
    
    // Connect buttons to slots
    connect(m_minimizeButton, &QPushButton::clicked, this, &MainWindow::OnMinimizeWindow);
    connect(m_maximizeButton, &QPushButton::clicked, this, &MainWindow::OnMaximizeWindow);
    connect(m_closeButton, &QPushButton::clicked, this, &MainWindow::OnCloseWindow);
    
    // Position title bar at the top of the window
    m_titleBar->setParent(this);
    m_titleBar->move(0, 0);
    m_titleBar->resize(width(), 40);
}

// Window control slots
void MainWindow::OnMinimizeWindow() {
    showMinimized();
}

void MainWindow::OnMaximizeWindow() {
    if (isMaximized()) {
        showNormal();
        m_maximizeButton->setText("□");
        m_maximizeButton->setToolTip("Maximize");
    } else {
        showMaximized();
        m_maximizeButton->setText("◱");
        m_maximizeButton->setToolTip("Restore");
    }
}

void MainWindow::OnCloseWindow() {
    close();
}

void MainWindow::CreateSelectionModeCombo() {
    // Create the combo box
    m_selectionModeCombo = new QComboBox(this);
    m_selectionModeCombo->addItem("Select Shape", static_cast<int>(cad_core::SelectionMode::Shape));
    m_selectionModeCombo->addItem("Select Face", static_cast<int>(cad_core::SelectionMode::Face));
    m_selectionModeCombo->addItem("Select Edge", static_cast<int>(cad_core::SelectionMode::Edge));
    m_selectionModeCombo->addItem("Select Vertex", static_cast<int>(cad_core::SelectionMode::Vertex));
    
    // Set default selection
    m_selectionModeCombo->setCurrentIndex(0); // Shape mode by default
    
    // Connect signal
    connect(m_selectionModeCombo, QOverload<int>::of(&QComboBox::currentIndexChanged),
            this, &MainWindow::OnSelectionModeComboChanged);
}

void MainWindow::CreateConsole() {
    // Create console output text edit
    m_console = new QTextEdit(this);
    m_console->setObjectName("console");
    m_console->setMaximumHeight(200);
    m_console->setMinimumHeight(100);
    m_console->setReadOnly(true);
    m_console->setStyleSheet(
        "QTextEdit {"
        "   background-color: #1e1e1e;"
        "   color: #ffffff;"
        "   font-family: 'Consolas', 'Monaco', monospace;"
        "   font-size: 9pt;"
        "   border: 1px solid #3c3c3c;"
        "}"
    );
    
    // Install custom message handler to redirect qDebug to console
    qInstallMessageHandler([](QtMsgType type, const QMessageLogContext &context, const QString &msg) {
        // Get the main window instance to access console
        foreach (QWidget *widget, QApplication::topLevelWidgets()) {
            MainWindow *mainWindow = qobject_cast<MainWindow*>(widget);
            if (mainWindow && mainWindow->m_console) {
                QString formattedMsg;
                switch (type) {
                    case QtDebugMsg:
                        formattedMsg = QString("[DEBUG] %1").arg(msg);
                        break;
                    case QtWarningMsg:
                        formattedMsg = QString("[WARNING] %1").arg(msg);
                        break;
                    case QtCriticalMsg:
                        formattedMsg = QString("[CRITICAL] %1").arg(msg);
                        break;
                    case QtFatalMsg:
                        formattedMsg = QString("[FATAL] %1").arg(msg);
                        break;
                    case QtInfoMsg:
                        formattedMsg = QString("[INFO] %1").arg(msg);
                        break;
                }
                mainWindow->m_console->append(formattedMsg);
                break;
            }
        }
    });
    
    m_console->append("[SYSTEM] Console initialized");
}

// Dialog interaction slots
void MainWindow::OnSelectionModeChanged(bool enabled, const QString& prompt) {
    if (enabled) {
        // Enable 3D selection mode
        statusBar()->showMessage(prompt);
        
        // Determine selection mode based on active dialog
        if (m_currentFilletChamferDialog) {
            // For fillet/chamfer operations, switch to edge selection mode
            m_viewer->SetSelectionMode(2); // Edge mode (OpenCASCADE mode 2)
            m_viewer->ClearEdgeSelection(); // Clear previous edge selections
            
            // Update combo box selection
            if (m_selectionModeCombo) {
                m_selectionModeCombo->setCurrentIndex(2); // Edge mode
            }
        } else if (m_currentBooleanDialog) {
            // For boolean operations, use shape selection mode
            m_viewer->SetSelectionMode(cad_core::SelectionMode::Shape);
            
            // Update combo box selection
            if (m_selectionModeCombo) {
                m_selectionModeCombo->setCurrentIndex(0); // Shape mode
            }

        }else if (m_currentHoleDialog) { 
			// use face selection mode for hole creation
            m_viewer->SetSelectionMode(4); 

        }else {
            // Default to shape selection
            m_viewer->SetSelectionMode(cad_core::SelectionMode::Shape);
            
            // Update combo box selection
            if (m_selectionModeCombo) {
                m_selectionModeCombo->setCurrentIndex(0); // Shape mode
            }
        }
    } else {
        // Disable selection mode
        statusBar()->showMessage("Ready");
        // Return to default shape selection mode
        m_viewer->SetSelectionMode(cad_core::SelectionMode::Shape);
    }
}

void MainWindow::OnObjectSelected(const cad_core::ShapePtr& shape) {
    // Forward selection to the active dialog
    if (m_currentBooleanDialog) {
        m_currentBooleanDialog->onObjectSelected(shape);
    }
    if (m_currentFilletChamferDialog) {
        m_currentFilletChamferDialog->onEdgeSelected(shape);
    }
    if (m_currentTransformDialog) {
        m_currentTransformDialog->onObjectSelected(shape);
    }
}

void MainWindow::OnBooleanOperationRequested(BooleanOperationType type, 
                                           const std::vector<cad_core::ShapePtr>& targets,
                                           const std::vector<cad_core::ShapePtr>& tools) {
    // Validate selection based on operation type
    if (type == BooleanOperationType::Union) {
        if (targets.empty()) {
            QMessageBox::warning(this, "Boolean Union", "Please select multiple objects to merge.");
            return;
        }
        if (targets.size() < 2 && tools.empty()) {
            QMessageBox::warning(this, "Boolean Union", "Please select at least 2 objects to merge.");
            return;
        }
    } else {
        if (targets.empty() || tools.empty()) {
            QMessageBox::warning(this, "Boolean Operation", "Please select both target and tool objects.");
            return;
        }
    }
    
    // Start OCAF transaction
    QString operationName;
    switch (type) {
        case BooleanOperationType::Union:
            operationName = "Boolean Union";
            break;
        case BooleanOperationType::Intersection:
            operationName = "Boolean Intersection";
            break;
        case BooleanOperationType::Difference:
            operationName = "Boolean Difference";
            break;
    }
    
    m_ocafManager->StartTransaction(operationName.toStdString());
    
    cad_core::ShapePtr result;
    try {
        if (type == BooleanOperationType::Union) {
            // Combine all targets and tools for union
            std::vector<cad_core::ShapePtr> allShapes = targets;
            allShapes.insert(allShapes.end(), tools.begin(), tools.end());
            result = cad_core::BooleanOperations::Union(allShapes);
        } else if (type == BooleanOperationType::Intersection) {
            // Use first target as base, intersect with all others
            result = targets[0];
            for (size_t i = 1; i < targets.size(); ++i) {
                if (result) {
                    result = cad_core::BooleanOperations::Intersection({result, targets[i]});
                }
            }
            for (const auto& tool : tools) {
                if (result) {
                    result = cad_core::BooleanOperations::Intersection({result, tool});
                }
            }
        } else if (type == BooleanOperationType::Difference) {
            // Use first target as base, subtract all tools
            result = targets[0];
            for (const auto& tool : tools) {
                if (result) {
                    result = cad_core::BooleanOperations::Difference(result, tool);
                }
            }
        }
        
        if (result) {
            // Add result to document
            if (m_ocafManager->AddShape(result, (operationName + " Result").toStdString())) {
                // Display the new result shape
                m_viewer->DisplayShape(result);
                m_documentTree->AddShape(result);
                
                // For Union: Remove all original objects (targets and tools) from OCAF
                // For Intersection/Difference: Remove original and tool objects from OCAF, keep only result
                if (type == BooleanOperationType::Union) {
                    // Union: Remove all input objects (targets + tools) from OCAF document
                    for (const auto& shape : targets) {
                        m_ocafManager->RemoveShape(shape);  // Remove from OCAF
                        m_viewer->RemoveShape(shape);       // Remove from 3D view
                        m_documentTree->RemoveShape(shape); // Remove from document tree
                    }
                    for (const auto& shape : tools) {
                        m_ocafManager->RemoveShape(shape);  // Remove from OCAF
                        m_viewer->RemoveShape(shape);       // Remove from 3D view
                        m_documentTree->RemoveShape(shape); // Remove from document tree
                    }
                } else {
                    // Intersection/Difference: Remove original and tool objects from OCAF
                    for (const auto& shape : targets) {
                        m_ocafManager->RemoveShape(shape);  // Remove from OCAF
                        m_viewer->RemoveShape(shape);       // Remove from 3D view
                        m_documentTree->RemoveShape(shape); // Remove from document tree
                    }
                    for (const auto& shape : tools) {
                        m_ocafManager->RemoveShape(shape);  // Remove from OCAF
                        m_viewer->RemoveShape(shape);       // Remove from 3D view
                        m_documentTree->RemoveShape(shape); // Remove from document tree
                    }
                }
                
                m_ocafManager->CommitTransaction();
                SetDocumentModified(true);
                UpdateActions();
                statusBar()->showMessage(operationName + " completed successfully");
            } else {
                m_ocafManager->AbortTransaction();
                QMessageBox::warning(this, "Error", "Failed to add result to document.");
            }
        } else {
            m_ocafManager->AbortTransaction();
            QMessageBox::warning(this, "Error", operationName + " operation failed.");
        }
    } catch (const std::exception& e) {
        m_ocafManager->AbortTransaction();
        QMessageBox::warning(this, "Error", QString("Boolean operation failed: %1").arg(e.what()));
    }
    
    // Clean up dialog
    if (m_currentBooleanDialog) {
        m_currentBooleanDialog->deleteLater();
        m_currentBooleanDialog = nullptr;
    }
}

void MainWindow::OnFilletChamferOperationRequested(FilletChamferType type, 
                                                 const std::vector<cad_core::ShapePtr>& edges,
                                                 double radius, double distance1, double distance2) {
    if (edges.empty()) {
        QMessageBox::warning(this, "Fillet/Chamfer", "Please select edges for operation.");
        return;
    }
    
    // Get selected edges grouped by their parent shapes
    auto edgesByShape = m_viewer->GetSelectedEdgesByShape();
    if (edgesByShape.empty()) {
        QMessageBox::warning(this, "Fillet/Chamfer", "No edges selected in 3D view. Please select edges first.");
        return;
    }
    
    qDebug() << "Fillet/Chamfer operation requested with edges from" << edgesByShape.size() << "shape(s)";
    
    // Start OCAF transaction
    QString operationName = (type == FilletChamferType::Fillet) ? "Fillet" : "Chamfer";
    m_ocafManager->StartTransaction(operationName.toStdString());
    
    try {
        bool anySuccess = false;
        
        // Process each shape that has selected edges
        for (const auto& shapeEdgePair : edgesByShape) {
            cad_core::ShapePtr baseShape = shapeEdgePair.first;
            const std::vector<TopoDS_Edge>& edges = shapeEdgePair.second;
            
            if (!baseShape || edges.empty()) {
                continue;
            }
            
            qDebug() << "Processing" << edges.size() << "edges on shape";
            
            // Perform the operation on this shape with its edges
            cad_core::ShapePtr result;
            if (type == FilletChamferType::Fillet) {
                result = cad_core::FilletChamferOperations::CreateFillet(baseShape, edges, radius);
            } else {
                result = cad_core::FilletChamferOperations::CreateChamfer(baseShape, edges, distance1);
            }
            
            if (result) {
                QString shapeName = QString("%1 Result on Shape").arg(operationName);
                if (m_ocafManager->AddShape(result, shapeName.toStdString())) {
                    // Remove the original shape from OCAF, viewer, and document tree
                    qDebug() << "Removing original shape before displaying" << operationName << "result";
                    m_ocafManager->RemoveShape(baseShape);  // Remove from OCAF
                    m_viewer->RemoveShape(baseShape);       // Remove from 3D view
                    m_documentTree->RemoveShape(baseShape); // Remove from document tree
                    
                    // Display the new result
                    m_viewer->DisplayShape(result);
                    m_documentTree->AddShape(result);
                    anySuccess = true;
                    qDebug() << "Successfully created" << operationName << "with" << edges.size() << "edges";
                } else {
                    qDebug() << "Failed to add" << operationName << "result to OCAF";
                }
            } else {
                qDebug() << operationName << "operation failed for this shape";
            }
        }
        
        if (anySuccess) {
            m_ocafManager->CommitTransaction();
            SetDocumentModified(true);
            UpdateActions();
            statusBar()->showMessage(operationName + " completed successfully");
        } else {
            m_ocafManager->AbortTransaction();
            QMessageBox::warning(this, "Error", operationName + " operation failed.");
        }
    } catch (const std::exception& e) {
        m_ocafManager->AbortTransaction();
        QMessageBox::warning(this, "Error", QString("%1 operation failed: %2").arg(operationName).arg(e.what()));
    }
    
    // Clear edge selection after operation
    m_viewer->ClearEdgeSelection();
    
    // Clean up dialog
    if (m_currentFilletChamferDialog) {
        m_currentFilletChamferDialog->deleteLater();
        m_currentFilletChamferDialog = nullptr;
    }
}

// =============================================================================
// Transform Operations Implementation
// =============================================================================

void MainWindow::OnTransformObjects() {
    if (m_currentTransformDialog) {
        m_currentTransformDialog->raise();
        m_currentTransformDialog->activateWindow();
        return;
    }
    
    m_currentTransformDialog = new TransformOperationDialog(this);
    
    // Connect dialog signals
    connect(m_currentTransformDialog, &TransformOperationDialog::selectionModeChanged,
            this, &MainWindow::OnSelectionModeChanged);
    connect(m_currentTransformDialog, &TransformOperationDialog::transformRequested,
            this, &MainWindow::OnTransformOperationRequested);
    connect(m_currentTransformDialog, &TransformOperationDialog::previewRequested,
            this, &MainWindow::OnTransformPreviewRequested);
    connect(m_currentTransformDialog, &TransformOperationDialog::resetRequested,
            this, &MainWindow::OnTransformResetRequested);
    
    // Show dialog
    m_currentTransformDialog->show();
}

void MainWindow::OnTransformOperationRequested(std::shared_ptr<cad_core::TransformCommand> command) {
    if (!command) {
        return;
    }
    
    try {
        // Reset any preview first
        if (m_previewActive) {
            OnTransformResetRequested();
        }
        
        // Execute the transform command to get transformed shapes
        if (command->Execute()) {
            // Get original and transformed shapes
            auto originalShapes = m_currentTransformDialog->getSelectedObjects();
            auto transformedShapes = command->GetTransformedShapes();
            
            // Start OCAF transaction
            m_ocafManager->StartTransaction("Transform Objects");
            
            // Replace shapes in OCAF document
            for (size_t i = 0; i < originalShapes.size() && i < transformedShapes.size(); ++i) {
                if (m_ocafManager->ReplaceShape(originalShapes[i], transformedShapes[i])) {
                    // Update display
                    m_viewer->RemoveShape(originalShapes[i]);
                    m_viewer->DisplayShape(transformedShapes[i]);
                    
                    // Update document tree
                    m_documentTree->RemoveShape(originalShapes[i]);
                    m_documentTree->AddShape(transformedShapes[i]);
                } else {
                    m_ocafManager->AbortTransaction();
                    QMessageBox::warning(this, "错误", "无法更新形状");
                    return;
                }
            }
            
            // Commit transaction
            m_ocafManager->CommitTransaction();
            
            // Update display and mark as modified
            RefreshUIFromOCAF();
            SetDocumentModified(true);
            
            // Update status bar
            statusBar()->showMessage(QString("变换操作完成: %1").arg(command->GetName()), 0.5);
        } else {
            QMessageBox::warning(this, "错误", "变换操作执行失败");
        }
    } catch (const std::exception& e) {
        m_ocafManager->AbortTransaction();
        QMessageBox::warning(this, "错误", QString("变换操作失败: %1").arg(e.what()));
    }
    
    // Clean up dialog
    if (m_currentTransformDialog) {
        m_currentTransformDialog->deleteLater();
        m_currentTransformDialog = nullptr;
    }
}

void MainWindow::OnTransformPreviewRequested(std::shared_ptr<cad_core::TransformCommand> command) {
    if (!command) {
        return;
    }
    
    try {
        // Clear any existing preview
        if (m_previewActive) {
            OnTransformResetRequested();
        }
        
        // Get preview shapes from command
        auto previewShapes = command->GetTransformedShapes();
        
        if (!previewShapes.empty()) {
            // Store preview shapes
            m_previewShapes = previewShapes;
            m_previewActive = true;
            
            // Display preview shapes with a different color/style
            for (const auto& shape : m_previewShapes) {
                if (shape && shape->IsValid()) {
                    // TODO: Set preview material/color (semi-transparent or different color)
                    m_viewer->DisplayShape(shape);
                }
            }
            
            // Update display
            m_viewer->update();
        }
    } catch (const std::exception& e) {
        QMessageBox::warning(this, "错误", QString("预览生成失败: %1").arg(e.what()));
    }
}

void MainWindow::OnTransformResetRequested() {
    if (!m_previewActive) {
        return;
    }
    
    // Remove preview shapes from display
    for (const auto& shape : m_previewShapes) {
        if (shape) {
            m_viewer->RemoveShape(shape);
        }
    }
    
    // Clear preview data
    m_previewShapes.clear();
    m_previewActive = false;
    
    // Update display
    m_viewer->update();
}

// =============================================================================
// Sketch Mode Implementation
// =============================================================================

void MainWindow::OnEnterSketchMode() {
    if (!m_viewer) {
        qDebug() << "Error: No viewer available";
        return;
    }
    
    try {
        // 检查是否已经在草图模式中
        if (m_viewer->IsInSketchMode()) {
            qDebug() << "Already in sketch mode";
            return;
        }
        
        // 检查是否有可用的对象
        auto shapes = m_ocafManager->GetAllShapes();
        if (shapes.empty()) {
            if (m_statusBar) {
                statusBar()->showMessage("请先创建一个几何体（如盒子），然后选择一个面进入草图模式");
            }
            qDebug() << "No shapes available for face selection";
            return;
        }
        
        // 创建并显示面选择对话框
        FaceSelectionDialog* dialog = new FaceSelectionDialog(m_viewer, this);
        
        // 连接对话框信号
        connect(dialog, &FaceSelectionDialog::faceSelected, this, [this, dialog](const TopoDS_Face& face) {
            OnFaceSelectedForSketch(face);
            dialog->close();
            dialog->deleteLater();
        });
        
        connect(dialog, &FaceSelectionDialog::selectionCancelled, this, [this, dialog]() {
            if (m_statusBar) {
                statusBar()->showMessage("草图模式已取消");
            }
            dialog->close();
            dialog->deleteLater();
        });
        
        // 显示对话框 (非模态)
        dialog->show();
        
        qDebug() << "Face selection dialog shown";
    }
    catch (const std::exception& e) {
        qDebug() << "Error in OnEnterSketchMode:" << e.what();
        if (m_statusBar) {
            statusBar()->showMessage("进入草图模式失败");
        }
    }
}

void MainWindow::OnExitSketchMode() {
    if (!m_viewer || !m_viewer->IsInSketchMode()) {
        return;
    }
    
    m_viewer->ExitSketchMode();
}

void MainWindow::OnSketchRectangleTool() {
    if (!m_viewer || !m_viewer->IsInSketchMode()) {
        return;
    }
    
    m_viewer->StartRectangleTool();
    statusBar()->showMessage("矩形工具已激活 - 点击并拖拽创建矩形");
}

void MainWindow::OnFaceSelected(const TopoDS_Face& face, const cad_core::ShapePtr& parentShape) {
    if (m_currentHoleDialog && !face.IsNull()) {
        m_currentHoleDialog->onObjectSelected(parentShape);
        m_currentHoleDialog->onFaceSelected(face);


        GProp_GProps props;
        BRepGProp::SurfaceProperties(face, props);
        gp_Pnt faceCenter = props.CentreOfMass();
        emit faceSelectionInfo(faceCenter.X(), faceCenter.Y(), faceCenter.Z());
    }

    if (!m_waitingForFaceSelection) {
        return;
    }
    
    try {
        m_waitingForFaceSelection = false;
        m_selectedFace = face;
        
        // 检查面是否有效
        if (face.IsNull()) {
            qDebug() << "Error: Selected face is null";
            if (m_statusBar) {
                statusBar()->showMessage("选择的面无效");
            }
            return;
        }
        
        // Enter sketch mode with the selected face
        if (m_viewer) {
            m_viewer->EnterSketchMode(face);
        } else {
            qDebug() << "Error: No viewer available for sketch mode";
        }
        
        qDebug() << "Face selected, entering sketch mode";
    }
    catch (const std::exception& e) {
        qDebug() << "Error in OnFaceSelected:" << e.what();
        if (m_statusBar) {
            statusBar()->showMessage(QString("进入草图模式失败: %1").arg(e.what()));
        }
        m_waitingForFaceSelection = false;
    }
}

void MainWindow::OnFaceSelectedForSketch(const TopoDS_Face& face) {
    try {
        // 检查面是否有效
        if (face.IsNull()) {
            qDebug() << "Error: Selected face is null";
            if (m_statusBar) {
                statusBar()->showMessage("选择的面无效");
            }
            return;
        }
        
        // 直接进入草图模式
        if (m_viewer) {
            m_viewer->EnterSketchMode(face);
            if (m_statusBar) {
                statusBar()->showMessage("正在进入草图模式...");
            }
        } else {
            qDebug() << "Error: No viewer available for sketch mode";
            if (m_statusBar) {
                statusBar()->showMessage("视图不可用");
            }
        }
        
        qDebug() << "Face selected from dialog, entering sketch mode";
    }
    catch (const std::exception& e) {
        qDebug() << "Error in OnFaceSelectedForSketch:" << e.what();
        if (m_statusBar) {
            statusBar()->showMessage(QString("进入草图模式失败: %1").arg(e.what()));
        }
    }
}

void MainWindow::OnSketchModeEntered() {
    // Update UI state when sketch mode is entered
    m_enterSketchAction->setEnabled(false);
    m_exitSketchAction->setEnabled(true);
    m_sketchRectangleAction->setEnabled(true);
    
    // Reset selection mode
    m_viewer->SetSelectionMode(0);  // Shape selection mode
    
    statusBar()->showMessage(QString("已进入草图模式 - 选择绘制工具开始绘制"));
    
    qDebug() << "Sketch mode entered, UI updated";
}

void MainWindow::OnSketchModeExited() {
    // Update UI state when sketch mode is exited
    m_enterSketchAction->setEnabled(true);
    m_exitSketchAction->setEnabled(false);
    m_sketchRectangleAction->setEnabled(false);
    
    // Reset any waiting states
    m_waitingForFaceSelection = false;
    
    statusBar()->showMessage("已退出草图模式");
    
    qDebug() << "Sketch mode exited, UI updated";
}


void MainWindow::OnHoleOperationRequested(const cad_core::ShapePtr& targetShape, const TopoDS_Face& selectedFace,
    double diameter, double depth,
    double x, double y, double z) {

    // 获取孔的方向 
    Handle(Geom_Surface) surface = BRep_Tool::Surface(selectedFace);
    Handle(Geom_Plane) plane = Handle(Geom_Plane)::DownCast(surface);
    if (plane.IsNull()) {
        QMessageBox::warning(this, "挖孔失败", "挖孔操作目前只支持在平面上进行。");
        return;
    }
    gp_Dir holeDirection = plane->Axis().Direction();
    if (selectedFace.Orientation() == TopAbs_REVERSED) {
        holeDirection.Reverse();
    }

    // 在原点创建圆柱体，然后移动它
    // 在(0,0,0)创建一个朝向Z轴的圆柱体
    auto cylinderTool = cad_core::ShapeFactory::CreateCylinder(diameter / 2.0, depth);
    if (!cylinderTool) {
        QMessageBox::warning(this, "错误", "在原点创建圆柱工具失败。");
        return;
    }

    gp_Trsf transformation;
    gp_Ax3 targetCoordinateSystem(gp_Pnt(x, y, z), holeDirection.Reversed());
    // 移动到目标坐标系
    transformation.SetTransformation(targetCoordinateSystem, gp::XOY());

    // 应用变换
    BRepBuilderAPI_Transform transformer(cylinderTool->GetOCCTShape(), transformation, Standard_True);
    auto transformedCylinder = std::make_shared<cad_core::Shape>(transformer.Shape());

    // 执行布尔差集
    m_ocafManager->StartTransaction("Create Hole");
    auto resultShape = cad_core::BooleanOperations::Difference(targetShape, transformedCylinder);


    if (resultShape && resultShape->IsValid()) {
        // 调用清理函数
        if (m_currentHoleDialog) {
            m_currentHoleDialog->cleanupAndRestoreView();
        }

        if (m_ocafManager->ReplaceShape(targetShape, resultShape)) {
            // 更新模型
            m_viewer->RemoveShape(targetShape);
            m_documentTree->RemoveShape(targetShape);

            m_viewer->DisplayShape(resultShape);
            m_documentTree->AddShape(resultShape);

            SetDocumentModified(true);
            m_ocafManager->CommitTransaction();
            statusBar()->showMessage("挖孔成功！", 3000);
        }
        else {
            m_ocafManager->AbortTransaction();
            QMessageBox::warning(this, "挖孔失败", "无法在文档中替换实体。");
        }
    }
    else {
        m_ocafManager->AbortTransaction();
        QMessageBox::warning(this, "挖孔操作失败", "挖孔操作失败。请检查坐标是否在实体内部。");
    }
}

void MainWindow::OnHolePreviewRequested(const cad_core::ShapePtr & holePreviewShape)
{
    if (!m_viewer || !holePreviewShape) {
        return;
    }
    m_viewer->DisplayPreviewShape(holePreviewShape);
}

void MainWindow::OnHoleResetPreviewRequested()
{
    if (!m_viewer) {
        return;
    }

    m_viewer->ClearPreviewShapes();
}

} // namespace cad_ui

#include "MainWindow.moc"