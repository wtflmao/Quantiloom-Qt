/**
 * @file MainWindow.cpp
 * @brief Main application window implementation
 *
 * @author wtflmao
 */

#include "MainWindow.hpp"
#include "vulkan/QuantiloomVulkanWindow.hpp"
#include "panels/SceneTreePanel.hpp"
#include "panels/MaterialEditorPanel.hpp"
#include "panels/LightingPanel.hpp"
#include "panels/RenderSettingsPanel.hpp"
#include "panels/SpectralConfigPanel.hpp"
#include "config/ConfigManager.hpp"

#include <QApplication>
#include <QMenuBar>
#include <QMenu>
#include <QAction>
#include <QActionGroup>
#include <QDockWidget>
#include <QTabWidget>
#include <QStatusBar>
#include <QProgressBar>
#include <QLabel>
#include <QFileDialog>
#include <QMessageBox>
#include <QCloseEvent>
#include <QVBoxLayout>
#include <QFileInfo>
#include <QSettings>

#include <core/Types.hpp>
#include <scene/Material.hpp>
#include <scene/Scene.hpp>
#include <renderer/LightingParams.hpp>
#include <glm/glm.hpp>

MainWindow::MainWindow(QVulkanInstance* vulkanInstance, QWidget* parent)
    : QMainWindow(parent)
    , m_vulkanInstance(vulkanInstance)
{
    setWindowTitle(tr("Quantiloom - Spectral Renderer"));
    setMinimumSize(1280, 720);
    resize(1600, 900);

    // Create configuration manager
    m_configManager = new ConfigManager(this);

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

    // Config import/export
    QAction* importConfigAction = fileMenu->addAction(tr("&Import Config..."), this, &MainWindow::onImportConfig);
    importConfigAction->setShortcut(QKeySequence(Qt::CTRL | Qt::Key_I));

    QAction* exportConfigAction = fileMenu->addAction(tr("E&xport Config..."), this, &MainWindow::onExportConfig);
    exportConfigAction->setShortcut(QKeySequence(Qt::CTRL | Qt::SHIFT | Qt::Key_E));

    fileMenu->addSeparator();

    QAction* exportAction = fileMenu->addAction(tr("Export &Image..."), this, &MainWindow::onExportImage);
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

    // Settings menu
    QMenu* settingsMenu = menuBar()->addMenu(tr("&Settings"));

    // Language submenu
    QMenu* languageMenu = settingsMenu->addMenu(tr("&Language"));
    QActionGroup* languageGroup = new QActionGroup(this);
    languageGroup->setExclusive(true);

    // Get current language setting
    QSettings settings;
    QString currentLocale = settings.value("language", "").toString();

    // English option
    QAction* englishAction = languageMenu->addAction("English");
    englishAction->setCheckable(true);
    englishAction->setData("en");
    languageGroup->addAction(englishAction);
    if (currentLocale.isEmpty() || currentLocale.startsWith("en")) {
        englishAction->setChecked(true);
    }

    // Chinese option
    QAction* chineseAction = languageMenu->addAction(QString::fromUtf8("中文"));
    chineseAction->setCheckable(true);
    chineseAction->setData("zh_CN");
    languageGroup->addAction(chineseAction);
    if (currentLocale.startsWith("zh")) {
        chineseAction->setChecked(true);
    }

    connect(languageGroup, &QActionGroup::triggered, this, [this](QAction* action) {
        onLanguageChanged(action->data().toString());
    });

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

    // Create panel instances
    m_sceneTreePanel = new SceneTreePanel();
    m_materialEditorPanel = new MaterialEditorPanel();
    m_lightingPanel = new LightingPanel();
    m_renderSettingsPanel = new RenderSettingsPanel();
    m_spectralConfigPanel = new SpectralConfigPanel();

    // Add panels to tabs
    m_parameterTabs->addTab(m_sceneTreePanel, tr("Scene"));
    m_parameterTabs->addTab(m_materialEditorPanel, tr("Material"));
    m_parameterTabs->addTab(m_lightingPanel, tr("Lighting"));
    m_parameterTabs->addTab(m_renderSettingsPanel, tr("Render"));
    m_parameterTabs->addTab(m_spectralConfigPanel, tr("Spectral"));

    m_parameterDock->setWidget(m_parameterTabs);
    addDockWidget(Qt::LeftDockWidgetArea, m_parameterDock);

    // Connect panel signals
    connect(m_sceneTreePanel, &SceneTreePanel::nodeSelected,
            this, &MainWindow::onNodeSelected);
    connect(m_sceneTreePanel, &SceneTreePanel::materialSelected,
            this, &MainWindow::onMaterialSelected);

    connect(m_materialEditorPanel, &MaterialEditorPanel::materialChanged,
            this, &MainWindow::onMaterialChanged);

    connect(m_lightingPanel, &LightingPanel::lightingChanged,
            this, &MainWindow::onLightingChanged);

    connect(m_renderSettingsPanel, &RenderSettingsPanel::sppChanged,
            this, &MainWindow::onSppChanged);
    connect(m_renderSettingsPanel, &RenderSettingsPanel::resetAccumulationRequested,
            this, &MainWindow::onResetAccumulation);

    connect(m_spectralConfigPanel, &SpectralConfigPanel::spectralModeChanged,
            this, &MainWindow::onSpectralModeChanged);
    connect(m_spectralConfigPanel, &SpectralConfigPanel::wavelengthChanged,
            this, &MainWindow::onWavelengthChanged);
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

    // Connect scene loaded signal to update panels
    connect(m_vulkanWindow, &QuantiloomVulkanWindow::sceneLoaded,
            this, [this](bool success, const QString& message) {
                if (success) {
                    updatePanelsFromScene();
                    m_statusLabel->setText(message);
                } else {
                    QMessageBox::warning(this, tr("Scene Load Failed"), message);
                    m_statusLabel->setText(tr("Failed to load scene"));
                }
            });
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
        // Check if it's a TOML config or a glTF file
        if (fileName.endsWith(".toml", Qt::CaseInsensitive)) {
            // Load TOML config (which may reference a glTF file)
            SceneConfig config;
            if (m_configManager->loadConfig(fileName, config)) {
                applyConfig(config);
                m_currentConfigFile = fileName;
                m_statusLabel->setText(tr("Config loaded: %1").arg(fileName));
            } else {
                QMessageBox::warning(this, tr("Load Failed"),
                    tr("Failed to load config: %1").arg(m_configManager->lastError()));
            }
        } else {
            // Direct glTF file loading
            m_currentSceneFile = fileName;
            m_vulkanWindow->loadScene(fileName);
            m_statusLabel->setText(tr("Loading: %1").arg(fileName));
        }
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
           "<p>Version 0.0.1</p>"
           "<p>A spectral renderer with hardware ray tracing support.</p>"
           "<p>Features:</p>"
           "<ul>"
           "<li>Hardware ray tracing</li>"
           "<li>Spectral rendering</li>"
           "<li>PBR materials with spectral extensions</li>"
           "<li>Atmospheric scattering</li>"
           "</ul>"
           "<p>Copyright (c) 2025-2026 wtflmao</p>")
    );
}

void MainWindow::onLanguageChanged(const QString& locale) {
    QSettings settings;
    QString currentLocale = settings.value("language", "").toString();

    // Only prompt if language actually changed
    if (currentLocale != locale) {
        settings.setValue("language", locale);

        QMessageBox::information(
            this,
            tr("Language Changed"),
            tr("The language setting has been changed.\n"
               "Please restart the application for the changes to take effect.")
        );
    }
}

void MainWindow::onFrameRendered(float frameTimeMs, uint32_t sampleCount) {
    float fps = (frameTimeMs > 0.0f) ? (1000.0f / frameTimeMs) : 0.0f;
    m_fpsLabel->setText(tr("FPS: %1").arg(fps, 0, 'f', 1));
    m_sampleCountLabel->setText(tr("Samples: %1").arg(sampleCount));

    // Update render settings panel
    m_renderSettingsPanel->setSampleCount(sampleCount);
}

// ============================================================================
// Panel Slots
// ============================================================================

void MainWindow::onNodeSelected(int nodeIndex) {
    Q_UNUSED(nodeIndex);
    // TODO: Highlight selected node in viewport
    m_statusLabel->setText(tr("Node %1 selected").arg(nodeIndex));
}

void MainWindow::onMaterialSelected(int materialIndex) {
    // Get scene from vulkan window
    const quantiloom::Scene* scene = m_vulkanWindow->getScene();
    if (scene && materialIndex >= 0 && static_cast<size_t>(materialIndex) < scene->materials.size()) {
        const auto& material = scene->materials[static_cast<size_t>(materialIndex)];
        m_materialEditorPanel->setMaterial(materialIndex, &material);
        m_parameterTabs->setCurrentWidget(m_materialEditorPanel);
        m_statusLabel->setText(tr("Material '%1' selected").arg(
            QString::fromStdString(material.name)));
    }
}

void MainWindow::onMaterialChanged(int index, const quantiloom::Material& material) {
    m_vulkanWindow->updateMaterial(index, material);
    m_sceneModified = true;
    m_statusLabel->setText(tr("Material modified"));
}

void MainWindow::onLightingChanged(const quantiloom::LightingParams& params) {
    m_vulkanWindow->setLightingParams(params);
    m_statusLabel->setText(tr("Lighting updated"));
}

void MainWindow::onSppChanged(uint32_t spp) {
    m_vulkanWindow->setSPP(spp);
    m_statusLabel->setText(tr("SPP set to %1").arg(spp));
}

void MainWindow::onSpectralModeChanged(quantiloom::SpectralMode mode) {
    m_vulkanWindow->setSpectralMode(mode);
    QString modeName;
    switch (mode) {
        case quantiloom::SpectralMode::RGB_Fused: modeName = "RGB"; break;
        case quantiloom::SpectralMode::Single: modeName = "Single"; break;
        case quantiloom::SpectralMode::MWIR_Fused: modeName = "MWIR"; break;
        case quantiloom::SpectralMode::LWIR_Fused: modeName = "LWIR"; break;
        default: modeName = "Unknown"; break;
    }
    m_statusLabel->setText(tr("Spectral mode: %1").arg(modeName));
}

void MainWindow::onWavelengthChanged(float wavelength_nm) {
    m_vulkanWindow->setWavelength(wavelength_nm);
    m_statusLabel->setText(tr("Wavelength: %1 nm").arg(wavelength_nm, 0, 'f', 0));
}

void MainWindow::onResetAccumulation() {
    m_vulkanWindow->resetAccumulation();
    m_statusLabel->setText(tr("Accumulation reset"));
}

void MainWindow::updatePanelsFromScene() {
    const quantiloom::Scene* scene = m_vulkanWindow->getScene();

    // Update scene tree
    m_sceneTreePanel->setScene(scene);

    // Clear material editor
    m_materialEditorPanel->clear();

    // Update lighting panel with current params
    if (scene) {
        m_lightingPanel->setLightingParams(quantiloom::CreateDefaultLightingParams());

        // Update spectral config
        m_spectralConfigPanel->setWavelengthRange(
            scene->lambda_min, scene->lambda_max, scene->delta_lambda);
    }
}

// ============================================================================
// Config Import/Export
// ============================================================================

void MainWindow::onImportConfig() {
    QString fileName = QFileDialog::getOpenFileName(
        this,
        tr("Import Configuration"),
        QString(),
        tr("TOML Config (*.toml);;All Files (*)")
    );

    if (!fileName.isEmpty()) {
        SceneConfig config;
        if (m_configManager->loadConfig(fileName, config)) {
            applyConfig(config);
            m_currentConfigFile = fileName;
            m_statusLabel->setText(tr("Config imported: %1").arg(fileName));
        } else {
            QMessageBox::warning(this, tr("Import Failed"),
                tr("Failed to import config: %1").arg(m_configManager->lastError()));
        }
    }
}

void MainWindow::onExportConfig() {
    QString fileName = QFileDialog::getSaveFileName(
        this,
        tr("Export Configuration"),
        m_currentConfigFile.isEmpty() ? "scene_config.toml" : m_currentConfigFile,
        tr("TOML Config (*.toml)")
    );

    if (!fileName.isEmpty()) {
        SceneConfig config;
        collectCurrentConfig(config);

        if (m_configManager->exportConfig(fileName, config)) {
            m_currentConfigFile = fileName;
            m_statusLabel->setText(tr("Config exported: %1").arg(fileName));
        } else {
            QMessageBox::warning(this, tr("Export Failed"),
                tr("Failed to export config: %1").arg(m_configManager->lastError()));
        }
    }
}

void MainWindow::applyConfig(const SceneConfig& config) {
    // Apply render settings
    m_renderSettingsPanel->setResolution(config.width, config.height);
    m_renderSettingsPanel->setTargetSPP(config.spp);
    m_vulkanWindow->setSPP(config.spp);

    // Apply spectral settings
    m_spectralConfigPanel->setSpectralMode(config.spectralMode);
    m_spectralConfigPanel->setWavelength(config.wavelength_nm);
    m_spectralConfigPanel->setWavelengthRange(config.lambda_min, config.lambda_max, config.delta_lambda);
    m_vulkanWindow->setSpectralMode(config.spectralMode);
    m_vulkanWindow->setWavelength(config.wavelength_nm);

    // Apply lighting settings
    m_lightingPanel->setLightingParams(config.lighting);
    m_vulkanWindow->setLightingParams(config.lighting);

    // Load the glTF file if specified
    if (!config.gltfPath.isEmpty()) {
        // Resolve relative path using config base directory
        QString gltfPath = config.gltfPath;
        if (!QFileInfo(gltfPath).isAbsolute() && !config.baseDir.isEmpty()) {
            gltfPath = config.baseDir + "/" + config.gltfPath;
        }
        m_currentSceneFile = gltfPath;
        m_vulkanWindow->loadScene(gltfPath);
    }

    // Apply camera settings (after scene load so renderer is ready)
    glm::vec3 camPos(config.cameraPosition[0], config.cameraPosition[1], config.cameraPosition[2]);
    glm::vec3 camLookAt(config.cameraLookAt[0], config.cameraLookAt[1], config.cameraLookAt[2]);
    glm::vec3 camUp(config.cameraUp[0], config.cameraUp[1], config.cameraUp[2]);
    m_vulkanWindow->setCamera(camPos, camLookAt, camUp, config.cameraFovY);
}

void MainWindow::collectCurrentConfig(SceneConfig& config) {
    // Collect render settings
    config.width = m_renderSettingsPanel->width();
    config.height = m_renderSettingsPanel->height();
    config.spp = m_renderSettingsPanel->spp();

    // Collect spectral settings from panel
    // These are tracked in the panel's internal state

    // Use current scene file as gltf path
    if (!m_currentSceneFile.isEmpty()) {
        config.gltfPath = m_currentSceneFile;
    }

    // Lighting is populated via the panel's emit signals
    // For now, use default or last known values
    config.lighting = quantiloom::CreateDefaultLightingParams();
}
