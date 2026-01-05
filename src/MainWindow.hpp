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

QT_BEGIN_NAMESPACE
class QDockWidget;
class QTabWidget;
class QStatusBar;
class QProgressBar;
class QLabel;
QT_END_NAMESPACE

namespace quantiloom {
class ExternalRenderContext;
}

class QuantiloomVulkanWindow;

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

private:
    void setupUi();
    void setupMenus();
    void setupDockWidgets();
    void setupStatusBar();
    void setupConnections();

    // Vulkan instance (owned by main())
    QVulkanInstance* m_vulkanInstance = nullptr;

    // Vulkan viewport
    QuantiloomVulkanWindow* m_vulkanWindow = nullptr;
    QWidget* m_vulkanContainer = nullptr;

    // Parameter dock
    QDockWidget* m_parameterDock = nullptr;
    QTabWidget* m_parameterTabs = nullptr;

    // Status bar widgets
    QLabel* m_statusLabel = nullptr;
    QLabel* m_fpsLabel = nullptr;
    QLabel* m_sampleCountLabel = nullptr;
    QProgressBar* m_renderProgress = nullptr;

    // Current scene file
    QString m_currentSceneFile;
    bool m_sceneModified = false;
};
