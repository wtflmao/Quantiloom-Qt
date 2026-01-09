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
#include "editing/SelectionManager.hpp"
#include "editing/TransformGizmo.hpp"
#include "editing/UndoStack.hpp"
#include "editing/Commands.hpp"

#include <QApplication>
#include <QGuiApplication>
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
#include <QDebug>

#include <core/Types.hpp>
#include <scene/Material.hpp>
#include <scene/Scene.hpp>
#include <renderer/LightingParams.hpp>
#include <glm/glm.hpp>
#include <glm/gtc/quaternion.hpp>

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
    setupEditingSystem();
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
    m_undoAction = editMenu->addAction(tr("&Undo"));
    m_undoAction->setShortcut(QKeySequence::Undo);
    m_undoAction->setEnabled(false);

    m_redoAction = editMenu->addAction(tr("&Redo"));
    m_redoAction->setShortcut(QKeySequence::Redo);
    m_redoAction->setEnabled(false);

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
            this, [this](int nodeIndex) {
                // Sync with selection manager
                bool addToSelection = QGuiApplication::keyboardModifiers() & Qt::ControlModifier;
                m_selectionManager->select(nodeIndex, addToSelection);
            });
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
    m_editModeLabel = new QLabel(tr("[G] Translate"));
    m_editModeLabel->setStyleSheet("QLabel { background-color: #4a90d9; color: white; padding: 2px 8px; border-radius: 3px; font-weight: bold; }");
    m_renderProgress = new QProgressBar();
    m_renderProgress->setMaximumWidth(200);
    m_renderProgress->setVisible(false);

    statusBar()->addWidget(m_statusLabel, 1);
    statusBar()->addPermanentWidget(m_editModeLabel);
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

    // Connect viewport click for selection
    connect(m_vulkanWindow, &QuantiloomVulkanWindow::viewportClicked,
            this, &MainWindow::onViewportClicked);
}

void MainWindow::setupEditingSystem() {
    // Create editing components
    m_selectionManager = new SelectionManager(this);
    m_transformGizmo = new TransformGizmo(this);
    m_undoStack = new UndoStack(this);

    // Pass to Vulkan window
    m_vulkanWindow->setEditingComponents(m_selectionManager, m_transformGizmo, m_undoStack);

    // Connect undo/redo actions
    connect(m_undoAction, &QAction::triggered, m_undoStack, &UndoStack::undo);
    connect(m_redoAction, &QAction::triggered, m_undoStack, &UndoStack::redo);

    // Connect undo stack state changes
    connect(m_undoStack, &UndoStack::canUndoChanged, this, &MainWindow::onUndoRedoChanged);
    connect(m_undoStack, &UndoStack::canRedoChanged, this, &MainWindow::onUndoRedoChanged);

    // Connect selection changes
    connect(m_selectionManager, &SelectionManager::selectionChanged,
            this, &MainWindow::onSelectionChanged);

    // Connect gizmo transform changes
    connect(m_transformGizmo, &TransformGizmo::transformChanged,
            this, &MainWindow::onGizmoTransformChanged);
    connect(m_transformGizmo, &TransformGizmo::transformFinished,
            this, &MainWindow::onGizmoTransformFinished);

    // Update status bar when gizmo mode changes
    connect(m_transformGizmo, &TransformGizmo::modeChanged,
            this, [this](TransformGizmo::Mode mode) {
                QString modeText;
                switch (mode) {
                    case TransformGizmo::Mode::Translate:
                        modeText = tr("[G] Translate");
                        break;
                    case TransformGizmo::Mode::Rotate:
                        modeText = tr("[R] Rotate");
                        break;
                    case TransformGizmo::Mode::Scale:
                        modeText = tr("[T] Scale");
                        break;
                }
                m_editModeLabel->setText(modeText);
                m_statusLabel->setText(tr("Mode: %1").arg(modeText));
            });

    // Sync selection with scene tree panel (highlight selected items)
    connect(m_selectionManager, &SelectionManager::selectionChanged,
            m_sceneTreePanel, &SceneTreePanel::setSelectedNodes);

    connect(m_selectionManager, &SelectionManager::selectionCleared,
            m_sceneTreePanel, &SceneTreePanel::clearSelectionHighlight);
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
        tr("3D Scene Files (*.gltf *.glb *.usd *.usda *.usdc *.usdz);;glTF Files (*.gltf *.glb);;OpenUSD Files (*.usd *.usda *.usdc *.usdz);;TOML Config (*.toml);;All Files (*)")
    );

    if (!fileName.isEmpty()) {
        // Check if it's a TOML config or a scene file
        if (fileName.endsWith(".toml", Qt::CaseInsensitive)) {
            // Load TOML config (which may reference a glTF or USD file)
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
            // Direct scene file loading (glTF or USD)
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
        case quantiloom::SpectralMode::RGB: modeName = "RGB"; break;
        case quantiloom::SpectralMode::VIS_Fused: modeName = "VIS Fused"; break;
        case quantiloom::SpectralMode::Single: modeName = "Single"; break;
        case quantiloom::SpectralMode::MWIR_Fused: modeName = "MWIR"; break;
        case quantiloom::SpectralMode::LWIR_Fused: modeName = "LWIR"; break;
        case quantiloom::SpectralMode::NIR_Fused: modeName = "NIR"; break;
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

        // Show helpful hint in status bar
        m_statusLabel->setText(tr("Scene loaded - Click a node in Scene panel to select, use G/R/T keys to change transform mode"));
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

    // Load scene file (glTF or USD)
    QString scenePath;
    if (!config.usdPath.isEmpty()) {
        // USD file specified
        scenePath = config.usdPath;
        if (!QFileInfo(scenePath).isAbsolute() && !config.baseDir.isEmpty()) {
            scenePath = config.baseDir + "/" + config.usdPath;
        }
    } else if (!config.gltfPath.isEmpty()) {
        // glTF file specified
        scenePath = config.gltfPath;
        if (!QFileInfo(scenePath).isAbsolute() && !config.baseDir.isEmpty()) {
            scenePath = config.baseDir + "/" + config.gltfPath;
        }
    }

    if (!scenePath.isEmpty()) {
        m_currentSceneFile = scenePath;
        m_vulkanWindow->loadScene(scenePath);
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

// ============================================================================
// Editing Slots
// ============================================================================

void MainWindow::onViewportClicked(const QPointF& screenPos) {
    // Simple selection: for now, cycle through nodes based on click
    // A proper implementation would do ray casting
    // For MVP, we select from scene tree instead

    Q_UNUSED(screenPos);

    // Clear selection on empty click
    // Note: Real picking would cast a ray and find intersecting geometry
    // For now, users select via the scene tree panel
    m_statusLabel->setText(tr("Click in Scene panel to select objects"));
}

void MainWindow::onSelectionChanged(const QSet<int>& selectedNodes) {
    qDebug() << "Selection changed:" << selectedNodes.size() << "nodes";

    if (selectedNodes.isEmpty()) {
        m_statusLabel->setText(tr("Selection cleared - click a node in Scene panel to select"));
        m_transformStartStates.clear();
    } else if (selectedNodes.size() == 1) {
        int nodeIndex = *selectedNodes.constBegin();

        // Get node name for display
        QString nodeName = QString("Node %1").arg(nodeIndex);
        const auto* scene = m_vulkanWindow->getScene();
        if (scene && nodeIndex >= 0 && static_cast<size_t>(nodeIndex) < scene->nodes.size()) {
            const auto& node = scene->nodes[static_cast<size_t>(nodeIndex)];
            if (node.meshIndex < scene->meshes.size()) {
                const auto& mesh = scene->meshes[node.meshIndex];
                if (!mesh.name.empty()) {
                    nodeName = QString::fromStdString(mesh.name);
                }
            }
        }

        m_statusLabel->setText(tr("'%1' selected - Left-drag in viewport to transform").arg(nodeName));

        // Store original transform for undo
        if (scene && nodeIndex >= 0 && static_cast<size_t>(nodeIndex) < scene->nodes.size()) {
            m_transformStartStates.clear();
            m_transformStartStates.push_back({
                nodeIndex,
                scene->nodes[static_cast<size_t>(nodeIndex)].transform
            });
        }
    } else {
        m_statusLabel->setText(tr("%1 objects selected - Left-drag in viewport to transform").arg(selectedNodes.size()));

        // Store all original transforms
        const auto* scene = m_vulkanWindow->getScene();
        if (scene) {
            m_transformStartStates.clear();
            for (int nodeIndex : selectedNodes) {
                if (nodeIndex >= 0 && static_cast<size_t>(nodeIndex) < scene->nodes.size()) {
                    m_transformStartStates.push_back({
                        nodeIndex,
                        scene->nodes[static_cast<size_t>(nodeIndex)].transform
                    });
                }
            }
        }
    }
}

void MainWindow::onGizmoTransformChanged(const glm::vec3& translation,
                                          const glm::quat& rotation,
                                          const glm::vec3& scale) {
    Q_UNUSED(rotation);
    Q_UNUSED(scale);

    // Apply transform delta to all selected nodes
    const auto* scene = m_vulkanWindow->getScene();
    if (!scene || m_transformStartStates.empty()) {
        return;
    }

    qDebug() << "Transform delta:" << translation.x << translation.y << translation.z;

    for (const auto& state : m_transformStartStates) {
        glm::mat4 newTransform = m_transformGizmo->applyDelta(state.originalTransform);
        m_vulkanWindow->setNodeTransform(state.nodeIndex, newTransform);
        qDebug() << "  Applied transform to node" << state.nodeIndex;
    }

    m_sceneModified = true;
}

void MainWindow::onGizmoTransformFinished() {
    // Create undo command for the transform
    const auto* scene = m_vulkanWindow->getScene();
    if (!scene || m_transformStartStates.empty()) {
        return;
    }

    if (m_transformStartStates.size() == 1) {
        // Single node transform
        const auto& state = m_transformStartStates[0];
        if (state.nodeIndex >= 0 && static_cast<size_t>(state.nodeIndex) < scene->nodes.size()) {
            glm::mat4 newTransform = scene->nodes[static_cast<size_t>(state.nodeIndex)].transform;

            // Only push command if transform actually changed
            if (newTransform != state.originalTransform) {
                auto cmd = std::make_unique<TransformNodeCommand>(
                    m_vulkanWindow,
                    state.nodeIndex,
                    state.originalTransform,
                    newTransform
                );
                m_undoStack->push(std::move(cmd));
            }
        }
    } else {
        // Multi-node transform
        std::vector<MultiTransformCommand::NodeTransform> transforms;

        for (const auto& state : m_transformStartStates) {
            if (state.nodeIndex >= 0 && static_cast<size_t>(state.nodeIndex) < scene->nodes.size()) {
                glm::mat4 newTransform = scene->nodes[static_cast<size_t>(state.nodeIndex)].transform;
                if (newTransform != state.originalTransform) {
                    transforms.push_back({
                        state.nodeIndex,
                        state.originalTransform,
                        newTransform
                    });
                }
            }
        }

        if (!transforms.empty()) {
            auto cmd = std::make_unique<MultiTransformCommand>(m_vulkanWindow, transforms);
            m_undoStack->push(std::move(cmd));
        }
    }

    // Update start states for next transform
    onSelectionChanged(m_selectionManager->selectedNodes());
}

void MainWindow::onUndoRedoChanged() {
    m_undoAction->setEnabled(m_undoStack->canUndo());
    m_redoAction->setEnabled(m_undoStack->canRedo());
    m_undoAction->setText(m_undoStack->undoText());
    m_redoAction->setText(m_undoStack->redoText());
}
