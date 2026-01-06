/**
 * @file QuantiloomVulkanRenderer.hpp
 * @brief QVulkanWindowRenderer adapter for libQuantiloom integration
 *
 * @author wtflmao
 */

#pragma once

#include <QVulkanWindowRenderer>
#include <QString>
#include <QFuture>
#include <memory>
#include <chrono>

#include <glm/glm.hpp>
#include <core/Types.hpp>

class QuantiloomVulkanWindow;
class QProgressDialog;

namespace quantiloom {
class ExternalRenderContext;
class Scene;
struct LightingParams;
struct Material;
}

/**
 * @class QuantiloomVulkanRenderer
 * @brief Adapter layer connecting Qt's Vulkan infrastructure with libQuantiloom
 *
 * This class implements QVulkanWindowRenderer and uses libQuantiloom's
 * ExternalRenderContext to perform actual ray tracing rendering.
 * It bridges Qt-managed Vulkan handles with the library's rendering pipeline.
 */
class QuantiloomVulkanRenderer : public QVulkanWindowRenderer {
public:
    explicit QuantiloomVulkanRenderer(QuantiloomVulkanWindow* window);
    ~QuantiloomVulkanRenderer() override;

    // QVulkanWindowRenderer interface
    void initResources() override;
    void initSwapChainResources() override;
    void releaseSwapChainResources() override;
    void releaseResources() override;
    void startNextFrame() override;

    // Scene management
    void loadScene(const QString& filePath);
    void resetCamera();

    // Render settings
    void setSPP(uint32_t spp);
    void setWavelength(float wavelength_nm);
    void setSpectralMode(quantiloom::SpectralMode mode);
    void setLightingParams(const quantiloom::LightingParams& params);
    void updateMaterial(int index, const quantiloom::Material& material);
    void resetAccumulation();
    uint32_t currentSampleCount() const { return m_sampleCount; }

    // Camera setup from config
    void setCamera(const glm::vec3& position, const glm::vec3& lookAt,
                   const glm::vec3& up, float fovY);

    // Scene access
    const quantiloom::Scene* getScene() const;

    // Camera info for gizmo
    void getCameraInfo(glm::vec3& position, glm::vec3& forward,
                       glm::vec3& right, glm::vec3& up) const;

    // Render context access (for transform operations)
    quantiloom::ExternalRenderContext* getRenderContext() { return m_renderContext.get(); }

    // Camera control
    void updateCameraMovement(bool forward, bool backward, bool left, bool right,
                              bool up, bool down, bool fast);
    void orbitCamera(float deltaX, float deltaY);
    void panCamera(float deltaX, float deltaY);
    void zoomCamera(float delta);

private:
    void updateCamera(float deltaTime);

    // Check if this is the first run (no pipeline cache)
    bool isFirstRun() const;

    // Show shader compilation progress dialog
    void showShaderCompilationDialog();

    QuantiloomVulkanWindow* m_window;

    // libQuantiloom render context
    std::unique_ptr<quantiloom::ExternalRenderContext> m_renderContext;

    // Frame timing
    std::chrono::high_resolution_clock::time_point m_lastFrameTime;
    float m_lastFrameTimeMs = 0.0f;

    // Accumulation state
    uint32_t m_sampleCount = 0;
    uint32_t m_targetSPP = 4;  // Default SPP for preview

    // Camera state
    glm::vec3 m_cameraPosition{0.0f, 1.0f, 5.0f};
    glm::vec3 m_cameraTarget{0.0f, 0.0f, 0.0f};
    glm::vec3 m_cameraUp{0.0f, 1.0f, 0.0f};
    float m_cameraFovY = 45.0f;

    // Orbit camera state
    float m_orbitDistance = 5.0f;
    float m_orbitYaw = 0.0f;    // Horizontal angle
    float m_orbitPitch = 0.0f;  // Vertical angle

    // Movement state
    bool m_moveForward = false;
    bool m_moveBackward = false;
    bool m_moveLeft = false;
    bool m_moveRight = false;
    bool m_moveUp = false;
    bool m_moveDown = false;
    bool m_moveFast = false;

    // Spectral mode
    float m_wavelength = 550.0f;  // nm

    // Initialization state
    bool m_initialized = false;
    QString m_pendingScenePath;

    // First run shader compilation tracking
    bool m_isFirstRun = false;
    bool m_shaderCompilationChecked = false;
};
