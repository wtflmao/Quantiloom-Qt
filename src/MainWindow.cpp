/**
 * @file MainWindow.cpp
 * @brief Main application window implementation
 *
 * @author wtflmao
 */

#include "MainWindow.hpp"
#include "vulkan/QuantiloomVulkanWindow.hpp"

#include <QApplication>
#include <QMenuBar>
#include <QMenu>
#include <QAction>
#include <QDockWidget>
#include <QTabWidget>
#include <QStatusBar>
#include <QProgressBar>
#include <QLabel>
#include <QFileDialog>
#include <QMessageBox>
#include <QCloseEvent>
#include <QVBoxLayout>

MainWindow::MainWindow(QVulkanInstance* vulkanInstance, QWidget* parent)
    : QMainWindow(parent)
    , m_vulkanInstance(vulkanInstance)
{
    setWindowTitle(tr("Quantiloom - Spectral Path Tracer"));
    setMinimumSize(1280, 720);
    resize(1600, 900);

    setupUi();
    setupMenus();
    setupDockWidgets();
    setupStatusBar();
    setupConnections();
}

MainWindow::~MainWindow() = default;

void MainWindow::setupUi() {
    // Create Vulkan window
    m_vulkanWindow = new QuantiloomVulkanWindow();
    m_vulkanWindow->setVulkanInstance(m_vulkanInstance);

    // Wrap in QWidget container for use as central widget
    m_vulkanContainer = QWidget::createWindowContainer(m_vulkanWindow);
    m_vulkanContainer->setMinimumSize(640, 480);
    m_vulkanContainer->setFocusPolicy(Qt::StrongFocus);

    setCentralWidget(m_vulkanContainer);
}

void MainWindow::setupMenus() {
    // File menu
    QMenu* fileMenu = menuBar()->addMenu(tr("&File"));

    QAction* newAction = fileMenu->addAction(tr("&New Scene"), this, &MainWindow::onNewScene);
    newAction->setShortcut(QKeySequence::New);

    QAction* openAction = fileMenu->addAction(tr("&Open Scene..."), this, &MainWindow::onOpenScene);
    openAction->setShortcut(QKeySequence::Open);

    QAction* saveAction = fileMenu->addAction(tr("&Save Scene"), this, &MainWindow::onSaveScene);
    saveAction->setShortcut(QKeySequence::Save);

    fileMenu->addSeparator();

    QAction* exportAction = fileMenu->addAction(tr("&Export Image..."), this, &MainWindow::onExportImage);
    exportAction->setShortcut(QKeySequence(Qt::CTRL | Qt::Key_E));

    fileMenu->addSeparator();

    QAction* exitAction = fileMenu->addAction(tr("E&xit"), this, &QWidget::close);
    exitAction->setShortcut(QKeySequence::Quit);

    // Edit menu
    QMenu* editMenu = menuBar()->addMenu(tr("&Edit"));
    editMenu->addAction(tr("&Undo"))->setShortcut(QKeySequence::Undo);
    editMenu->addAction(tr("&Redo"))->setShortcut(QKeySequence::Redo);
    editMenu->addSeparator();
    editMenu->addAction(tr("&Delete"))->setShortcut(QKeySequence::Delete);

    // View menu
    QMenu* viewMenu = menuBar()->addMenu(tr("&View"));
    viewMenu->addAction(tr("&Reset Camera"), this, &MainWindow::onResetCamera);
    viewMenu->addSeparator();
    viewMenu->addAction(tr("&Parameter Panel"))->setCheckable(true);

    // Render menu
    QMenu* renderMenu = menuBar()->addMenu(tr("&Render"));

    QAction* startRenderAction = renderMenu->addAction(tr("&Start Render"), this, &MainWindow::onStartRender);
    startRenderAction->setShortcut(QKeySequence(Qt::Key_F5));

    QAction* stopRenderAction = renderMenu->addAction(tr("S&top Render"), this, &MainWindow::onStopRender);
    stopRenderAction->setShortcut(QKeySequence(Qt::Key_Escape));

    // Help menu
    QMenu* helpMenu = menuBar()->addMenu(tr("&Help"));
    helpMenu->addAction(tr("&About"), this, &MainWindow::onAbout);
    helpMenu->addAction(tr("About &Qt"), qApp, &QApplication::aboutQt);
}

void MainWindow::setupDockWidgets() {
    // Create parameter dock widget
    m_parameterDock = new QDockWidget(tr("Parameters"), this);
    m_parameterDock->setAllowedAreas(Qt::LeftDockWidgetArea | Qt::RightDockWidgetArea);
    m_parameterDock->setMinimumWidth(300);

    // Create tabbed widget for different parameter categories
    m_parameterTabs = new QTabWidget();

    // Placeholder tabs (will be replaced with actual panels)
    QWidget* sceneTab = new QWidget();
    sceneTab->setLayout(new QVBoxLayout());
    m_parameterTabs->addTab(sceneTab, tr("Scene"));

    QWidget* materialTab = new QWidget();
    materialTab->setLayout(new QVBoxLayout());
    m_parameterTabs->addTab(materialTab, tr("Material"));

    QWidget* lightingTab = new QWidget();
    lightingTab->setLayout(new QVBoxLayout());
    m_parameterTabs->addTab(lightingTab, tr("Lighting"));

    QWidget* renderTab = new QWidget();
    renderTab->setLayout(new QVBoxLayout());
    m_parameterTabs->addTab(renderTab, tr("Render"));

    QWidget* spectralTab = new QWidget();
    spectralTab->setLayout(new QVBoxLayout());
    m_parameterTabs->addTab(spectralTab, tr("Spectral"));

    m_parameterDock->setWidget(m_parameterTabs);
    addDockWidget(Qt::LeftDockWidgetArea, m_parameterDock);
}

void MainWindow::setupStatusBar() {
    m_statusLabel = new QLabel(tr("Ready"));
    m_fpsLabel = new QLabel(tr("FPS: --"));
    m_sampleCountLabel = new QLabel(tr("Samples: 0"));
    m_renderProgress = new QProgressBar();
    m_renderProgress->setMaximumWidth(200);
    m_renderProgress->setVisible(false);

    statusBar()->addWidget(m_statusLabel, 1);
    statusBar()->addPermanentWidget(m_sampleCountLabel);
    statusBar()->addPermanentWidget(m_fpsLabel);
    statusBar()->addPermanentWidget(m_renderProgress);
}

void MainWindow::setupConnections() {
    // Connect Vulkan window signals
    connect(m_vulkanWindow, &QuantiloomVulkanWindow::frameRendered,
            this, &MainWindow::onFrameRendered);
}

void MainWindow::closeEvent(QCloseEvent* event) {
    if (m_sceneModified) {
        QMessageBox::StandardButton reply = QMessageBox::question(
            this,
            tr("Unsaved Changes"),
            tr("The scene has been modified. Do you want to save your changes?"),
            QMessageBox::Save | QMessageBox::Discard | QMessageBox::Cancel
        );

        if (reply == QMessageBox::Save) {
            onSaveScene();
        } else if (reply == QMessageBox::Cancel) {
            event->ignore();
            return;
        }
    }

    event->accept();
}

// ============================================================================
// Slots
// ============================================================================

void MainWindow::onNewScene() {
    // TODO: Create new empty scene
    m_statusLabel->setText(tr("New scene created"));
}

void MainWindow::onOpenScene() {
    QString fileName = QFileDialog::getOpenFileName(
        this,
        tr("Open Scene"),
        QString(),
        tr("glTF Files (*.gltf *.glb);;TOML Config (*.toml);;All Files (*)")
    );

    if (!fileName.isEmpty()) {
        m_currentSceneFile = fileName;
        m_vulkanWindow->loadScene(fileName);
        m_statusLabel->setText(tr("Loaded: %1").arg(fileName));
    }
}

void MainWindow::onSaveScene() {
    if (m_currentSceneFile.isEmpty()) {
        QString fileName = QFileDialog::getSaveFileName(
            this,
            tr("Save Scene"),
            QString(),
            tr("TOML Config (*.toml)")
        );
        if (fileName.isEmpty()) return;
        m_currentSceneFile = fileName;
    }

    // TODO: Save scene to file
    m_sceneModified = false;
    m_statusLabel->setText(tr("Saved: %1").arg(m_currentSceneFile));
}

void MainWindow::onExportImage() {
    QString fileName = QFileDialog::getSaveFileName(
        this,
        tr("Export Image"),
        QString(),
        tr("EXR Image (*.exr);;PNG Image (*.png);;All Files (*)")
    );

    if (!fileName.isEmpty()) {
        // TODO: Export rendered image
        m_statusLabel->setText(tr("Exported: %1").arg(fileName));
    }
}

void MainWindow::onStartRender() {
    m_renderProgress->setVisible(true);
    m_renderProgress->setValue(0);
    m_statusLabel->setText(tr("Rendering..."));
    // TODO: Start high-quality render
}

void MainWindow::onStopRender() {
    m_renderProgress->setVisible(false);
    m_statusLabel->setText(tr("Render stopped"));
    // TODO: Stop render
}

void MainWindow::onResetCamera() {
    m_vulkanWindow->resetCamera();
    m_statusLabel->setText(tr("Camera reset"));
}

void MainWindow::onAbout() {
    QMessageBox::about(
        this,
        tr("About Quantiloom"),
        tr("<h3>Quantiloom</h3>"
           "<p>Version 1.0.0</p>"
           "<p>A spectral path tracer with hardware ray tracing support.</p>"
           "<p>Features:</p>"
           "<ul>"
           "<li>RTX hardware ray tracing</li>"
           "<li>Spectral rendering (380-2500nm)</li>"
           "<li>PBR materials with spectral extensions</li>"
           "<li>Atmospheric scattering</li>"
           "</ul>"
           "<p>Copyright (c) 2024 wtflmao</p>")
    );
}

void MainWindow::onFrameRendered(float frameTimeMs, uint32_t sampleCount) {
    float fps = (frameTimeMs > 0.0f) ? (1000.0f / frameTimeMs) : 0.0f;
    m_fpsLabel->setText(tr("FPS: %1").arg(fps, 0, 'f', 1));
    m_sampleCountLabel->setText(tr("Samples: %1").arg(sampleCount));
}
