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

#include <glm/glm.hpp>
#include <core/Types.hpp>

namespace quantiloom {
class Scene;
struct Material;
struct LightingParams;
}

class QuantiloomVulkanRenderer;
class SelectionManager;
class TransformGizmo;
class UndoStack;

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
     * @brief Set camera from config parameters
     */
    void setCamera(const glm::vec3& position, const glm::vec3& lookAt,
                   const glm::vec3& up, float fovY);

    /**
     * @brief Set render samples per pixel
     */
    void setSPP(uint32_t spp);

    /**
     * @brief Set spectral wavelength for mono-band mode
     */
    void setWavelength(float wavelength_nm);

    /**
     * @brief Set spectral rendering mode
     */
    void setSpectralMode(quantiloom::SpectralMode mode);

    /**
     * @brief Update lighting parameters
     */
    void setLightingParams(const quantiloom::LightingParams& params);

    /**
     * @brief Update material at specified index
     */
    void updateMaterial(int index, const quantiloom::Material& material);

    /**
     * @brief Reset render accumulation
     */
    void resetAccumulation();

    /**
     * @brief Get current sample count
     */
    uint32_t currentSampleCount() const;

    /**
     * @brief Get current scene (may be null)
     */
    const quantiloom::Scene* getScene() const;

    // ========================================================================
    // Scene Editing
    // ========================================================================

    /**
     * @brief Set editing components (owned by MainWindow)
     */
    void setEditingComponents(SelectionManager* selection,
                               TransformGizmo* gizmo,
                               UndoStack* undoStack);

    /**
     * @brief Set node transform
     */
    void setNodeTransform(int nodeIndex, const glm::mat4& transform);

    /**
     * @brief Get camera info for gizmo
     */
    void getCameraInfo(glm::vec3& position, glm::vec3& forward,
                       glm::vec3& right, glm::vec3& up) const;

    /**
     * @brief Check if in edit mode (vs camera mode)
     */
    [[nodiscard]] bool isEditMode() const { return m_editMode; }

    /**
     * @brief Set edit mode
     */
    void setEditMode(bool edit);

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

    /**
     * @brief Emitted when user clicks in viewport (for selection picking)
     * @param screenPos Screen position of click
     */
    void viewportClicked(const QPointF& screenPos);

    /**
     * @brief Emitted when edit mode changes
     */
    void editModeChanged(bool editMode);

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

    // Editing components (owned by MainWindow)
    SelectionManager* m_selection = nullptr;
    TransformGizmo* m_gizmo = nullptr;
    UndoStack* m_undoStack = nullptr;

    // Edit mode state
    bool m_editMode = true;  // Default to edit mode
    bool m_transformDragging = false;
    QPointF m_transformDragStart;
};
