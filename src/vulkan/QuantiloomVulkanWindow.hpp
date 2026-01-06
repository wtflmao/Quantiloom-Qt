/**
 * @file QuantiloomVulkanWindow.hpp
 * @brief QVulkanWindow subclass for Quantiloom rendering
 *
 * @author wtflmao
 */

#pragma once

#include <QVulkanWindow>
#include <QString>
#include <memory>
#include <vulkan/vulkan.h>

class QuantiloomVulkanRenderer;

/**
 * @class QuantiloomVulkanWindow
 * @brief Custom Vulkan window that hosts Quantiloom rendering
 *
 * This class manages the Vulkan surface and coordinates with
 * QuantiloomVulkanRenderer for actual rendering operations.
 */
class QuantiloomVulkanWindow : public QVulkanWindow {
    Q_OBJECT

public:
    explicit QuantiloomVulkanWindow(QWindow* parent = nullptr);
    ~QuantiloomVulkanWindow() override;

    /**
     * @brief Create the Vulkan renderer
     * @return New QVulkanWindowRenderer instance
     */
    QVulkanWindowRenderer* createRenderer() override;

    /**
     * @brief Load a scene from file
     * @param filePath Path to glTF or TOML scene file
     */
    void loadScene(const QString& filePath);

    /**
     * @brief Reset camera to default position
     */
    void resetCamera();

    /**
     * @brief Set render samples per pixel
     */
    void setSPP(uint32_t spp);

    /**
     * @brief Set spectral wavelength for mono-band mode
     */
    void setWavelength(float wavelength_nm);

    /**
     * @brief Get current sample count
     */
    uint32_t currentSampleCount() const;

signals:
    /**
     * @brief Emitted after each frame is rendered
     * @param frameTimeMs Frame render time in milliseconds
     * @param sampleCount Current accumulated sample count
     */
    void frameRendered(float frameTimeMs, uint32_t sampleCount);

    /**
     * @brief Emitted when scene loading completes or fails
     * @param success True if loading succeeded
     * @param message Status message or error description
     */
    void sceneLoaded(bool success, const QString& message);

protected:
    void keyPressEvent(QKeyEvent* event) override;
    void keyReleaseEvent(QKeyEvent* event) override;
    void mousePressEvent(QMouseEvent* event) override;
    void mouseReleaseEvent(QMouseEvent* event) override;
    void mouseMoveEvent(QMouseEvent* event) override;
    void wheelEvent(QWheelEvent* event) override;

private:
    friend class QuantiloomVulkanRenderer;

    QuantiloomVulkanRenderer* m_renderer = nullptr;
    QString m_pendingScenePath;

    // Camera control state
    bool m_mousePressed = false;
    QPointF m_lastMousePos;
    bool m_keyW = false;
    bool m_keyA = false;
    bool m_keyS = false;
    bool m_keyD = false;
    bool m_keyQ = false;
    bool m_keyE = false;
    bool m_shiftHeld = false;

    // Vulkan feature structures (must persist during device creation)
    VkPhysicalDeviceBufferDeviceAddressFeatures m_bufferDeviceAddressFeatures{};
    VkPhysicalDeviceRayTracingPipelineFeaturesKHR m_rayTracingPipelineFeatures{};
    VkPhysicalDeviceAccelerationStructureFeaturesKHR m_accelerationStructureFeatures{};
    VkPhysicalDeviceDynamicRenderingFeatures m_dynamicRenderingFeatures{};
    VkPhysicalDeviceDescriptorIndexingFeatures m_descriptorIndexingFeatures{};
    VkPhysicalDeviceScalarBlockLayoutFeatures m_scalarBlockLayoutFeatures{};
    VkPhysicalDeviceRayQueryFeaturesKHR m_rayQueryFeatures{};
    VkPhysicalDeviceSynchronization2Features m_synchronization2Features{};
};
