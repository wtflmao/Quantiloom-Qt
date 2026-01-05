/**
 * @file QuantiloomVulkanRenderer.cpp
 * @brief QVulkanWindowRenderer adapter implementation
 *
 * @author wtflmao
 */

#include "QuantiloomVulkanRenderer.hpp"
#include "QuantiloomVulkanWindow.hpp"

#include <renderer/ExternalRenderContext.hpp>

#include <QVulkanFunctions>
#include <QFile>
#include <QObject>
#include <cmath>

#include <glm/gtc/matrix_transform.hpp>

QuantiloomVulkanRenderer::QuantiloomVulkanRenderer(QuantiloomVulkanWindow* window)
    : m_window(window)
    , m_lastFrameTime(std::chrono::high_resolution_clock::now())
{
}

QuantiloomVulkanRenderer::~QuantiloomVulkanRenderer() {
    // Release resources in proper order
    m_renderContext.reset();
}

void QuantiloomVulkanRenderer::initResources() {
    // Extract Qt-managed Vulkan handles
    QVulkanInstance* inst = m_window->vulkanInstance();
    VkInstance vkInstance = inst->vkInstance();
    VkDevice device = m_window->device();
    VkPhysicalDevice physDevice = m_window->physicalDevice();

    // Get graphics queue
    QVulkanFunctions* f = inst->functions();
    VkQueue graphicsQueue;
    f->vkGetDeviceQueue(device, m_window->graphicsQueueFamilyIndex(), 0, &graphicsQueue);

    // Initialize libQuantiloom with external handles
    quantiloom::ExternalRenderContext::InitParams params{};
    params.instance = vkInstance;
    params.physicalDevice = physDevice;
    params.device = device;
    params.graphicsQueue = graphicsQueue;
    params.graphicsQueueFamily = static_cast<quantiloom::u32>(m_window->graphicsQueueFamilyIndex());
    params.targetColorFormat = m_window->colorFormat();

    QSize swapSize = m_window->swapChainImageSize();
    params.width = static_cast<quantiloom::u32>(swapSize.width());
    params.height = static_cast<quantiloom::u32>(swapSize.height());

    auto result = quantiloom::ExternalRenderContext::Create(params);
    if (!result) {
        qWarning("Failed to create ExternalRenderContext: %s",
                 qPrintable(QString::fromStdString(result.error())));
        return;
    }

    m_renderContext = std::move(result.value());
    m_initialized = true;

    // Load pending scene if any
    if (!m_pendingScenePath.isEmpty()) {
        loadScene(m_pendingScenePath);
        m_pendingScenePath.clear();
    }

    // Set initial camera
    m_renderContext->SetCameraLookAt(m_cameraPosition, m_cameraTarget, m_cameraUp);
    m_renderContext->SetCameraFOV(m_cameraFovY);
}

void QuantiloomVulkanRenderer::initSwapChainResources() {
    if (!m_renderContext) return;

    // Update render context with new swapchain size
    QSize swapSize = m_window->swapChainImageSize();
    m_renderContext->Resize(
        static_cast<quantiloom::u32>(swapSize.width()),
        static_cast<quantiloom::u32>(swapSize.height())
    );

    resetAccumulation();
}

void QuantiloomVulkanRenderer::releaseSwapChainResources() {
    // Nothing to do - render context manages its own resources
}

void QuantiloomVulkanRenderer::releaseResources() {
    m_renderContext.reset();
    m_initialized = false;
}

void QuantiloomVulkanRenderer::startNextFrame() {
    auto now = std::chrono::high_resolution_clock::now();
    float deltaTime = std::chrono::duration<float>(now - m_lastFrameTime).count();
    m_lastFrameTime = now;

    // Update camera based on input
    updateCamera(deltaTime);

    if (!m_renderContext) {
        m_window->frameReady();
        m_window->requestUpdate();
        return;
    }

    // Get current command buffer and swapchain image
    VkCommandBuffer cmd = m_window->currentCommandBuffer();
    int swapChainIndex = m_window->currentSwapChainImageIndex();
    VkImageView targetView = m_window->swapChainImageView(swapChainIndex);
    QSize swapSize = m_window->swapChainImageSize();

    // Render frame using libQuantiloom
    // The library will use VK_KHR_dynamic_rendering internally
    m_renderContext->RenderFrame(
        cmd,
        targetView,
        static_cast<quantiloom::u32>(swapSize.width()),
        static_cast<quantiloom::u32>(swapSize.height())
    );

    // Update sample count
    m_sampleCount = m_renderContext->GetAccumulatedSamples();

    // Calculate frame time
    auto frameEnd = std::chrono::high_resolution_clock::now();
    m_lastFrameTimeMs = std::chrono::duration<float, std::milli>(frameEnd - now).count();

    // Emit frame rendered signal
    emit m_window->frameRendered(m_lastFrameTimeMs, m_sampleCount);

    // Signal frame ready and request next frame
    m_window->frameReady();
    m_window->requestUpdate();
}

void QuantiloomVulkanRenderer::loadScene(const QString& filePath) {
    if (!m_initialized) {
        m_pendingScenePath = filePath;
        return;
    }

    if (!m_renderContext) {
        emit m_window->sceneLoaded(false, QObject::tr("Render context not initialized"));
        return;
    }

    std::string path = filePath.toStdString();
    auto result = m_renderContext->LoadSceneFromGltf(path);

    if (result) {
        resetAccumulation();
        emit m_window->sceneLoaded(true, QObject::tr("Scene loaded successfully"));
    } else {
        emit m_window->sceneLoaded(false,
            QObject::tr("Failed to load scene: %1").arg(QString::fromStdString(result.error())));
    }
}

void QuantiloomVulkanRenderer::resetCamera() {
    m_cameraPosition = glm::vec3(0.0f, 1.0f, 5.0f);
    m_cameraTarget = glm::vec3(0.0f, 0.0f, 0.0f);
    m_cameraUp = glm::vec3(0.0f, 1.0f, 0.0f);
    m_orbitDistance = 5.0f;
    m_orbitYaw = 0.0f;
    m_orbitPitch = 0.0f;

    if (m_renderContext) {
        m_renderContext->SetCameraLookAt(m_cameraPosition, m_cameraTarget, m_cameraUp);
        resetAccumulation();
    }
}

void QuantiloomVulkanRenderer::setSPP(uint32_t spp) {
    m_targetSPP = spp;
    if (m_renderContext) {
        m_renderContext->SetSPP(spp);
    }
}

void QuantiloomVulkanRenderer::setWavelength(float wavelength_nm) {
    m_wavelength = wavelength_nm;
    if (m_renderContext) {
        m_renderContext->SetWavelength(wavelength_nm);
        resetAccumulation();
    }
}

void QuantiloomVulkanRenderer::updateCameraMovement(
    bool forward, bool backward, bool left, bool right,
    bool up, bool down, bool fast)
{
    m_moveForward = forward;
    m_moveBackward = backward;
    m_moveLeft = left;
    m_moveRight = right;
    m_moveUp = up;
    m_moveDown = down;
    m_moveFast = fast;
}

void QuantiloomVulkanRenderer::orbitCamera(float deltaX, float deltaY) {
    const float sensitivity = 0.005f;

    m_orbitYaw -= deltaX * sensitivity;
    m_orbitPitch -= deltaY * sensitivity;

    // Clamp pitch to avoid gimbal lock
    m_orbitPitch = glm::clamp(m_orbitPitch, -glm::half_pi<float>() + 0.1f,
                                             glm::half_pi<float>() - 0.1f);

    // Calculate new camera position
    float x = m_orbitDistance * std::cos(m_orbitPitch) * std::sin(m_orbitYaw);
    float y = m_orbitDistance * std::sin(m_orbitPitch);
    float z = m_orbitDistance * std::cos(m_orbitPitch) * std::cos(m_orbitYaw);

    m_cameraPosition = m_cameraTarget + glm::vec3(x, y, z);

    if (m_renderContext) {
        m_renderContext->SetCameraLookAt(m_cameraPosition, m_cameraTarget, m_cameraUp);
        resetAccumulation();
    }
}

void QuantiloomVulkanRenderer::panCamera(float deltaX, float deltaY) {
    const float sensitivity = 0.01f;

    glm::vec3 forward = glm::normalize(m_cameraTarget - m_cameraPosition);
    glm::vec3 right = glm::normalize(glm::cross(forward, m_cameraUp));
    glm::vec3 up = glm::cross(right, forward);

    glm::vec3 pan = -right * deltaX * sensitivity + up * deltaY * sensitivity;
    m_cameraPosition += pan;
    m_cameraTarget += pan;

    if (m_renderContext) {
        m_renderContext->SetCameraLookAt(m_cameraPosition, m_cameraTarget, m_cameraUp);
        resetAccumulation();
    }
}

void QuantiloomVulkanRenderer::zoomCamera(float delta) {
    const float zoomSpeed = 0.5f;

    m_orbitDistance *= (1.0f - delta * zoomSpeed * 0.1f);
    m_orbitDistance = glm::clamp(m_orbitDistance, 0.1f, 1000.0f);

    // Recalculate camera position
    float x = m_orbitDistance * std::cos(m_orbitPitch) * std::sin(m_orbitYaw);
    float y = m_orbitDistance * std::sin(m_orbitPitch);
    float z = m_orbitDistance * std::cos(m_orbitPitch) * std::cos(m_orbitYaw);

    m_cameraPosition = m_cameraTarget + glm::vec3(x, y, z);

    if (m_renderContext) {
        m_renderContext->SetCameraLookAt(m_cameraPosition, m_cameraTarget, m_cameraUp);
        resetAccumulation();
    }
}

void QuantiloomVulkanRenderer::updateCamera(float deltaTime) {
    if (!m_moveForward && !m_moveBackward && !m_moveLeft &&
        !m_moveRight && !m_moveUp && !m_moveDown) {
        return;
    }

    const float baseSpeed = 5.0f;
    float speed = m_moveFast ? baseSpeed * 3.0f : baseSpeed;

    glm::vec3 forward = glm::normalize(m_cameraTarget - m_cameraPosition);
    glm::vec3 right = glm::normalize(glm::cross(forward, m_cameraUp));

    glm::vec3 movement(0.0f);
    if (m_moveForward)  movement += forward;
    if (m_moveBackward) movement -= forward;
    if (m_moveRight)    movement += right;
    if (m_moveLeft)     movement -= right;
    if (m_moveUp)       movement += m_cameraUp;
    if (m_moveDown)     movement -= m_cameraUp;

    if (glm::length(movement) > 0.0f) {
        movement = glm::normalize(movement) * speed * deltaTime;
        m_cameraPosition += movement;
        m_cameraTarget += movement;

        if (m_renderContext) {
            m_renderContext->SetCameraLookAt(m_cameraPosition, m_cameraTarget, m_cameraUp);
            resetAccumulation();
        }
    }
}

void QuantiloomVulkanRenderer::resetAccumulation() {
    m_sampleCount = 0;
    if (m_renderContext) {
        m_renderContext->ResetAccumulation();
    }
}
