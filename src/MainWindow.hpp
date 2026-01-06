/**
 * @file MainWindow.hpp
 * @brief Main application window with Vulkan viewport and parameter panels
 *
 * @author wtflmao
 */

#pragma once

#include <QMainWindow>
#include <QVulkanInstance>
#include <memory>

#include <core/Types.hpp>

QT_BEGIN_NAMESPACE
class QDockWidget;
class QTabWidget;
class QStatusBar;
class QProgressBar;
class QLabel;
QT_END_NAMESPACE

namespace quantiloom {
class ExternalRenderContext;
struct LightingParams;
struct Material;
}

class QuantiloomVulkanWindow;
class SceneTreePanel;
class MaterialEditorPanel;
class LightingPanel;
class RenderSettingsPanel;
class SpectralConfigPanel;
class ConfigManager;
struct SceneConfig;

/**
 * @class MainWindow
 * @brief Main application window with 3D viewport and parameter panels
 *
 * Layout:
 * - Center: Vulkan 3D viewport (QuantiloomVulkanWindow)
 * - Left: Parameter panels in tabbed dock widget
 * - Bottom: Status bar with render info
 */
class MainWindow : public QMainWindow {
    Q_OBJECT

public:
    explicit MainWindow(QVulkanInstance* vulkanInstance, QWidget* parent = nullptr);
    ~MainWindow() override;

protected:
    void closeEvent(QCloseEvent* event) override;

private slots:
    // File menu actions
    void onNewScene();
    void onOpenScene();
    void onSaveScene();
    void onImportConfig();
    void onExportConfig();
    void onExportImage();

    // Render menu actions
    void onStartRender();
    void onStopRender();

    // View menu actions
    void onResetCamera();

    // Help menu actions
    void onAbout();

    // Status updates
    void onFrameRendered(float frameTimeMs, uint32_t sampleCount);

    // Panel signals
    void onNodeSelected(int nodeIndex);
    void onMaterialSelected(int materialIndex);
    void onMaterialChanged(int index, const quantiloom::Material& material);
    void onLightingChanged(const quantiloom::LightingParams& params);
    void onSppChanged(uint32_t spp);
    void onSpectralModeChanged(quantiloom::SpectralMode mode);
    void onWavelengthChanged(float wavelength_nm);
    void onResetAccumulation();

private:
    void setupUi();
    void setupMenus();
    void setupDockWidgets();
    void setupStatusBar();
    void setupConnections();
    void updatePanelsFromScene();

    // Vulkan instance (owned by main())
    QVulkanInstance* m_vulkanInstance = nullptr;

    // Vulkan viewport
    QuantiloomVulkanWindow* m_vulkanWindow = nullptr;
    QWidget* m_vulkanContainer = nullptr;

    // Parameter dock
    QDockWidget* m_parameterDock = nullptr;
    QTabWidget* m_parameterTabs = nullptr;

    // Parameter panels
    SceneTreePanel* m_sceneTreePanel = nullptr;
    MaterialEditorPanel* m_materialEditorPanel = nullptr;
    LightingPanel* m_lightingPanel = nullptr;
    RenderSettingsPanel* m_renderSettingsPanel = nullptr;
    SpectralConfigPanel* m_spectralConfigPanel = nullptr;

    // Status bar widgets
    QLabel* m_statusLabel = nullptr;
    QLabel* m_fpsLabel = nullptr;
    QLabel* m_sampleCountLabel = nullptr;
    QProgressBar* m_renderProgress = nullptr;

    // Configuration manager
    ConfigManager* m_configManager = nullptr;

    // Current scene file
    QString m_currentSceneFile;
    QString m_currentConfigFile;
    bool m_sceneModified = false;

    // Helper methods
    void applyConfig(const SceneConfig& config);
    void collectCurrentConfig(SceneConfig& config);
};
