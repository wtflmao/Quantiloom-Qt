/**
 * @file QuantiloomVulkanRenderer.cpp
 * @brief QVulkanWindowRenderer adapter implementation
 *
 * @author wtflmao
 */

#include "QuantiloomVulkanRenderer.hpp"
#include "QuantiloomVulkanWindow.hpp"

#include <renderer/ExternalRenderContext.hpp>
#include <renderer/LightingParams.hpp>
#include <scene/Material.hpp>
#include <scene/Scene.hpp>

#include <QVulkanFunctions>
#include <QFile>
#include <QFileInfo>
#include <QDir>
#include <QObject>
#include <QDebug>
#include <QProgressDialog>
#include <QApplication>
#include <QTimer>
#include <QStandardPaths>
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
    qDebug() << "QuantiloomVulkanRenderer::initResources() - Vulkan device ready";
    // Note: Swapchain is not ready yet, full initialization happens in initSwapChainResources()
}

void QuantiloomVulkanRenderer::initSwapChainResources() {
    qDebug() << "QuantiloomVulkanRenderer::initSwapChainResources() - Starting...";

    QSize swapSize = m_window->swapChainImageSize();
    qDebug() << "  Swapchain size:" << swapSize;

    if (swapSize.width() <= 0 || swapSize.height() <= 0) {
        qWarning() << "Invalid swapchain size, skipping initialization";
        return;
    }

    // If already initialized, just resize
    if (m_renderContext) {
        qDebug() << "  Resizing existing context...";
        m_renderContext->Resize(
            static_cast<quantiloom::u32>(swapSize.width()),
            static_cast<quantiloom::u32>(swapSize.height())
        );
        resetAccumulation();
        return;
    }

    // First time initialization - extract Qt-managed Vulkan handles
    QVulkanInstance* inst = m_window->vulkanInstance();
    VkInstance vkInstance = inst->vkInstance();
    VkDevice device = m_window->device();
    VkPhysicalDevice physDevice = m_window->physicalDevice();

    qDebug() << "  VkInstance:" << vkInstance;
    qDebug() << "  VkDevice:" << device;
    qDebug() << "  VkPhysicalDevice:" << physDevice;

    if (!device) {
        qCritical() << "Device is NULL! Qt failed to create Vulkan device.";
        qCritical() << "This usually means required device extensions are not supported.";
        return;
    }

    // Get graphics queue using device functions
    QVulkanDeviceFunctions* df = inst->deviceFunctions(device);
    VkQueue graphicsQueue;
    df->vkGetDeviceQueue(device, m_window->graphicsQueueFamilyIndex(), 0, &graphicsQueue);

    qDebug() << "  VkQueue:" << graphicsQueue;
    qDebug() << "  Queue family:" << m_window->graphicsQueueFamilyIndex();
    qDebug() << "  Color format:" << m_window->colorFormat();

    // Initialize libQuantiloom with external handles
    quantiloom::ExternalRenderContext::InitParams params{};
    params.instance = vkInstance;
    params.physicalDevice = physDevice;
    params.device = device;
    params.graphicsQueue = graphicsQueue;
    params.graphicsQueueFamily = static_cast<quantiloom::u32>(m_window->graphicsQueueFamilyIndex());
    params.targetColorFormat = m_window->colorFormat();
    params.width = static_cast<quantiloom::u32>(swapSize.width());
    params.height = static_cast<quantiloom::u32>(swapSize.height());
    // Note: pipelineCacheDir uses platform-specific default in ExternalRenderContext:
    //   Windows: %LOCALAPPDATA%/Quantiloom/cache/
    //   Linux:   ~/.cache/Quantiloom/
    //   macOS:   ~/Library/Caches/Quantiloom/

    qDebug() << "Creating ExternalRenderContext...";

    auto result = quantiloom::ExternalRenderContext::Create(params);
    if (!result) {
        qCritical() << "Failed to create ExternalRenderContext:"
                    << QString::fromStdString(result.error());
        return;
    }

    qDebug() << "ExternalRenderContext created successfully!";

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

    if (!m_renderContext || !m_renderContext->HasScene()) {
        // No scene loaded yet, just present empty frame
        m_window->frameReady();
        m_window->requestUpdate();
        return;
    }

    // Get current command buffer and swapchain image
    VkCommandBuffer cmd = m_window->currentCommandBuffer();
    int swapChainIndex = m_window->currentSwapChainImageIndex();
    VkImage targetImage = m_window->swapChainImage(swapChainIndex);
    QSize swapSize = m_window->swapChainImageSize();

    // Render frame using libQuantiloom
    // ExternalRenderContext handles layout transitions and blit to swapchain
    m_renderContext->RenderFrame(
        cmd,
        targetImage,
        VK_IMAGE_LAYOUT_UNDEFINED,  // Qt doesn't guarantee initial layout
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
    qDebug() << "QuantiloomVulkanRenderer::loadScene() - Path:" << filePath;

    if (!m_initialized) {
        qDebug() << "  Not initialized, saving as pending...";
        m_pendingScenePath = filePath;
        return;
    }

    if (!m_renderContext) {
        qCritical() << "  Render context is null!";
        emit m_window->sceneLoaded(false, QObject::tr("Render context not initialized"));
        return;
    }

    // Check if this is the first run (no pipeline cache)
    QProgressDialog* progressDialog = nullptr;
    if (isFirstRun()) {
        qDebug() << "  First run detected - showing shader compilation dialog";

        // Create modal progress dialog
        // Note: Using nullptr as parent since QVulkanWindow is not a QWidget
        progressDialog = new QProgressDialog(
            QObject::tr("First run, compiling shaders...\nIt may take a few minutes."),
            QString(),  // No cancel button
            0, 0,       // Indeterminate progress
            nullptr
        );
        progressDialog->setWindowTitle(QObject::tr("Initializing"));
        progressDialog->setWindowModality(Qt::ApplicationModal);
        progressDialog->setMinimumDuration(0);  // Show immediately
        progressDialog->setCancelButton(nullptr);  // Remove cancel button
        progressDialog->setAutoClose(true);
        progressDialog->setAutoReset(true);
        progressDialog->setMinimumWidth(350);

        // Make dialog non-closable
        progressDialog->setWindowFlags(
            Qt::Dialog | Qt::CustomizeWindowHint | Qt::WindowTitleHint
        );

        // Show dialog and process events to ensure it's displayed
        progressDialog->show();
        progressDialog->raise();
        progressDialog->activateWindow();
        QApplication::processEvents();
    }

    // Determine file type and call appropriate loader
    std::string path = filePath.toStdString();
    quantiloom::Result<void, quantiloom::String> result;

    if (filePath.endsWith(".usd", Qt::CaseInsensitive) ||
        filePath.endsWith(".usda", Qt::CaseInsensitive) ||
        filePath.endsWith(".usdc", Qt::CaseInsensitive) ||
        filePath.endsWith(".usdz", Qt::CaseInsensitive)) {
        qDebug() << "  Calling LoadSceneFromUsd...";
        result = m_renderContext->LoadSceneFromUsd(path);
    } else {
        qDebug() << "  Calling LoadSceneFromGltf...";
        result = m_renderContext->LoadSceneFromGltf(path);
    }

    // Close progress dialog
    if (progressDialog) {
        progressDialog->close();
        progressDialog->deleteLater();
    }

    qDebug() << "  Scene load returned";

    if (result) {
        qDebug() << "  Scene loaded successfully!";
        resetAccumulation();
        emit m_window->sceneLoaded(true, QObject::tr("Scene loaded successfully"));
    } else {
        qCritical() << "  Failed to load scene:" << QString::fromStdString(result.error());
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

void QuantiloomVulkanRenderer::setCamera(const glm::vec3& position, const glm::vec3& lookAt,
                                          const glm::vec3& up, float fovY) {
    m_cameraPosition = position;
    m_cameraTarget = lookAt;
    m_cameraUp = up;
    m_cameraFovY = fovY;

    // Update orbit distance based on new position/target
    m_orbitDistance = glm::length(position - lookAt);

    // Calculate orbit angles from the direction vector
    glm::vec3 dir = glm::normalize(position - lookAt);
    m_orbitPitch = glm::degrees(std::asin(dir.y));
    m_orbitYaw = glm::degrees(std::atan2(dir.x, dir.z));

    if (m_renderContext) {
        m_renderContext->SetCameraLookAt(m_cameraPosition, m_cameraTarget, m_cameraUp);
        m_renderContext->SetCameraFOV(fovY);
        resetAccumulation();
    }

    qDebug() << "Camera set: pos=(" << position.x << "," << position.y << "," << position.z
             << ") lookAt=(" << lookAt.x << "," << lookAt.y << "," << lookAt.z
             << ") fov=" << fovY;
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

void QuantiloomVulkanRenderer::setSpectralMode(quantiloom::SpectralMode mode) {
    if (m_renderContext) {
        m_renderContext->SetSpectralMode(mode);
        resetAccumulation();
    }
}

void QuantiloomVulkanRenderer::setLightingParams(const quantiloom::LightingParams& params) {
    if (m_renderContext) {
        m_renderContext->SetLightingParams(params);
        resetAccumulation();
    }
}

void QuantiloomVulkanRenderer::updateMaterial(int index, const quantiloom::Material& material) {
    if (m_renderContext && index >= 0) {
        m_renderContext->UpdateMaterial(static_cast<quantiloom::u32>(index), material);
        resetAccumulation();
    }
}

const quantiloom::Scene* QuantiloomVulkanRenderer::getScene() const {
    return m_renderContext ? m_renderContext->GetScene() : nullptr;
}

void QuantiloomVulkanRenderer::getCameraInfo(glm::vec3& position, glm::vec3& forward,
                                              glm::vec3& right, glm::vec3& up) const {
    position = m_cameraPosition;
    forward = glm::normalize(m_cameraTarget - m_cameraPosition);
    right = glm::normalize(glm::cross(forward, m_cameraUp));
    up = glm::cross(right, forward);
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

bool QuantiloomVulkanRenderer::isFirstRun() const {
    // Check if pipeline cache file exists
    // If it doesn't exist, this is the first run and shader compilation will be slow
    //
    // Cache location follows platform conventions:
    //   Windows: %LOCALAPPDATA%/Quantiloom/cache/pipeline_cache.bin
    //   Linux:   ~/.cache/Quantiloom/pipeline_cache.bin
    //   macOS:   ~/Library/Caches/Quantiloom/pipeline_cache.bin
    QString cacheDir = QStandardPaths::writableLocation(QStandardPaths::CacheLocation);
#ifdef Q_OS_WIN
    // Windows: QStandardPaths returns %LOCALAPPDATA%, add /Quantiloom/cache
    QString cachePath = cacheDir + "/Quantiloom/cache/pipeline_cache.bin";
#else
    // Linux/macOS: QStandardPaths returns ~/.cache or ~/Library/Caches
    QString cachePath = cacheDir + "/Quantiloom/pipeline_cache.bin";
#endif
    return !QFileInfo::exists(cachePath);
}
