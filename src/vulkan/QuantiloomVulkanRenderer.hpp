/**
 * @file QuantiloomVulkanRenderer.hpp
 * @brief QVulkanWindowRenderer adapter for libQuantiloom integration
 *
 * @author wtflmao
 */

#pragma once

#include <QVulkanWindowRenderer>
#include <QString>
#include <memory>
#include <chrono>

#include <glm/glm.hpp>

class QuantiloomVulkanWindow;

namespace quantiloom {
class ExternalRenderContext;
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
    uint32_t currentSampleCount() const { return m_sampleCount; }

    // Camera control
    void updateCameraMovement(bool forward, bool backward, bool left, bool right,
                              bool up, bool down, bool fast);
    void orbitCamera(float deltaX, float deltaY);
    void panCamera(float deltaX, float deltaY);
    void zoomCamera(float delta);

private:
    void updateCamera(float deltaTime);
    void resetAccumulation();

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
};
