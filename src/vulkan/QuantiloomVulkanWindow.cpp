/**
 * @file QuantiloomVulkanWindow.cpp
 * @brief QVulkanWindow subclass implementation
 *
 * @author wtflmao
 */

#include "QuantiloomVulkanWindow.hpp"
#include "QuantiloomVulkanRenderer.hpp"

#include <QKeyEvent>
#include <QMouseEvent>
#include <QWheelEvent>
#include <QDebug>

QuantiloomVulkanWindow::QuantiloomVulkanWindow(QWindow* parent)
    : QVulkanWindow(parent)
{
    // Request required device extensions for ray tracing
    setDeviceExtensions({
        // Ray tracing core extensions
        "VK_KHR_acceleration_structure",
        "VK_KHR_ray_tracing_pipeline",
        "VK_KHR_ray_query",  // Required if shaders use RayQuery capability
        "VK_KHR_deferred_host_operations",
        // Required by ray tracing
        "VK_KHR_buffer_device_address",
        "VK_KHR_spirv_1_4",
        "VK_KHR_shader_float_controls",
        // Dynamic rendering (Vulkan 1.3 core, but request as extension for compatibility)
        "VK_KHR_dynamic_rendering",
        // Synchronization2 (Vulkan 1.3 core)
        "VK_KHR_synchronization2",
        // Maintenance extensions often required
        "VK_KHR_maintenance3",
        "VK_KHR_maintenance4",
        // Descriptor indexing for bindless textures
        "VK_EXT_descriptor_indexing",
        // Scalar block layout
        "VK_EXT_scalar_block_layout"
    });

    // Enable required Vulkan features for ray tracing using Qt 6.7+ API
    // Reference: https://doc.qt.io/qt-6/qvulkanwindow.html#setEnabledFeaturesModifier
    setEnabledFeaturesModifier([this](VkPhysicalDeviceFeatures2& features) {
        qDebug() << "QuantiloomVulkanWindow: Enabling ray tracing device features...";

        // Initialize feature structures with sType
        m_bufferDeviceAddressFeatures.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_BUFFER_DEVICE_ADDRESS_FEATURES;
        m_bufferDeviceAddressFeatures.bufferDeviceAddress = VK_TRUE;

        m_accelerationStructureFeatures.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_ACCELERATION_STRUCTURE_FEATURES_KHR;
        m_accelerationStructureFeatures.accelerationStructure = VK_TRUE;

        m_rayTracingPipelineFeatures.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_RAY_TRACING_PIPELINE_FEATURES_KHR;
        m_rayTracingPipelineFeatures.rayTracingPipeline = VK_TRUE;

        m_rayQueryFeatures.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_RAY_QUERY_FEATURES_KHR;
        m_rayQueryFeatures.rayQuery = VK_TRUE;

        m_dynamicRenderingFeatures.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_DYNAMIC_RENDERING_FEATURES;
        m_dynamicRenderingFeatures.dynamicRendering = VK_TRUE;

        m_synchronization2Features.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_SYNCHRONIZATION_2_FEATURES;
        m_synchronization2Features.synchronization2 = VK_TRUE;

        m_descriptorIndexingFeatures.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_DESCRIPTOR_INDEXING_FEATURES;
        m_descriptorIndexingFeatures.shaderSampledImageArrayNonUniformIndexing = VK_TRUE;
        m_descriptorIndexingFeatures.runtimeDescriptorArray = VK_TRUE;
        m_descriptorIndexingFeatures.descriptorBindingVariableDescriptorCount = VK_TRUE;
        m_descriptorIndexingFeatures.descriptorBindingPartiallyBound = VK_TRUE;

        m_scalarBlockLayoutFeatures.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_SCALAR_BLOCK_LAYOUT_FEATURES;
        m_scalarBlockLayoutFeatures.scalarBlockLayout = VK_TRUE;

        // Build pNext chain (matches VulkanContext order):
        // features -> bufferDeviceAddress -> accelerationStructure -> rayTracingPipeline
        //          -> rayQuery -> dynamicRendering -> synchronization2
        //          -> descriptorIndexing -> scalarBlockLayout
        features.pNext = &m_bufferDeviceAddressFeatures;
        m_bufferDeviceAddressFeatures.pNext = &m_accelerationStructureFeatures;
        m_accelerationStructureFeatures.pNext = &m_rayTracingPipelineFeatures;
        m_rayTracingPipelineFeatures.pNext = &m_rayQueryFeatures;
        m_rayQueryFeatures.pNext = &m_dynamicRenderingFeatures;
        m_dynamicRenderingFeatures.pNext = &m_synchronization2Features;
        m_synchronization2Features.pNext = &m_descriptorIndexingFeatures;
        m_descriptorIndexingFeatures.pNext = &m_scalarBlockLayoutFeatures;
        m_scalarBlockLayoutFeatures.pNext = nullptr;

        // Enable required Vulkan 1.0 features
        features.features.shaderInt64 = VK_TRUE;
        features.features.samplerAnisotropy = VK_TRUE;

        qDebug() << "  Ray tracing features enabled via pNext chain (including rayQuery, synchronization2)";
    });

    qDebug() << "QuantiloomVulkanWindow: Requested ray tracing device extensions";
}

QuantiloomVulkanWindow::~QuantiloomVulkanWindow() = default;

QVulkanWindowRenderer* QuantiloomVulkanWindow::createRenderer() {
    m_renderer = new QuantiloomVulkanRenderer(this);

    // Load pending scene if set before renderer was created
    if (!m_pendingScenePath.isEmpty()) {
        m_renderer->loadScene(m_pendingScenePath);
        m_pendingScenePath.clear();
    }

    return m_renderer;
}

void QuantiloomVulkanWindow::loadScene(const QString& filePath) {
    if (m_renderer) {
        m_renderer->loadScene(filePath);
    } else {
        // Store for later loading when renderer is created
        m_pendingScenePath = filePath;
    }
}

void QuantiloomVulkanWindow::resetCamera() {
    if (m_renderer) {
        m_renderer->resetCamera();
    }
}

void QuantiloomVulkanWindow::setSPP(uint32_t spp) {
    if (m_renderer) {
        m_renderer->setSPP(spp);
    }
}

void QuantiloomVulkanWindow::setWavelength(float wavelength_nm) {
    if (m_renderer) {
        m_renderer->setWavelength(wavelength_nm);
    }
}

uint32_t QuantiloomVulkanWindow::currentSampleCount() const {
    return m_renderer ? m_renderer->currentSampleCount() : 0;
}

// ============================================================================
// Input Event Handlers
// ============================================================================

void QuantiloomVulkanWindow::keyPressEvent(QKeyEvent* event) {
    switch (event->key()) {
        case Qt::Key_W: m_keyW = true; break;
        case Qt::Key_A: m_keyA = true; break;
        case Qt::Key_S: m_keyS = true; break;
        case Qt::Key_D: m_keyD = true; break;
        case Qt::Key_Q: m_keyQ = true; break;
        case Qt::Key_E: m_keyE = true; break;
        case Qt::Key_Shift: m_shiftHeld = true; break;
        default:
            QVulkanWindow::keyPressEvent(event);
            return;
    }

    if (m_renderer) {
        m_renderer->updateCameraMovement(
            m_keyW, m_keyS, m_keyA, m_keyD, m_keyQ, m_keyE, m_shiftHeld);
    }
}

void QuantiloomVulkanWindow::keyReleaseEvent(QKeyEvent* event) {
    switch (event->key()) {
        case Qt::Key_W: m_keyW = false; break;
        case Qt::Key_A: m_keyA = false; break;
        case Qt::Key_S: m_keyS = false; break;
        case Qt::Key_D: m_keyD = false; break;
        case Qt::Key_Q: m_keyQ = false; break;
        case Qt::Key_E: m_keyE = false; break;
        case Qt::Key_Shift: m_shiftHeld = false; break;
        default:
            QVulkanWindow::keyReleaseEvent(event);
            return;
    }

    if (m_renderer) {
        m_renderer->updateCameraMovement(
            m_keyW, m_keyS, m_keyA, m_keyD, m_keyQ, m_keyE, m_shiftHeld);
    }
}

void QuantiloomVulkanWindow::mousePressEvent(QMouseEvent* event) {
    if (event->button() == Qt::RightButton || event->button() == Qt::MiddleButton) {
        m_mousePressed = true;
        m_lastMousePos = event->position();
        event->accept();
    } else {
        QVulkanWindow::mousePressEvent(event);
    }
}

void QuantiloomVulkanWindow::mouseReleaseEvent(QMouseEvent* event) {
    if (event->button() == Qt::RightButton || event->button() == Qt::MiddleButton) {
        m_mousePressed = false;
        event->accept();
    } else {
        QVulkanWindow::mouseReleaseEvent(event);
    }
}

void QuantiloomVulkanWindow::mouseMoveEvent(QMouseEvent* event) {
    if (m_mousePressed && m_renderer) {
        QPointF delta = event->position() - m_lastMousePos;
        m_lastMousePos = event->position();

        if (event->buttons() & Qt::RightButton) {
            // Right drag: orbit camera
            m_renderer->orbitCamera(
                static_cast<float>(delta.x()),
                static_cast<float>(delta.y())
            );
        } else if (event->buttons() & Qt::MiddleButton) {
            // Middle drag: pan camera
            m_renderer->panCamera(
                static_cast<float>(delta.x()),
                static_cast<float>(delta.y())
            );
        }

        event->accept();
    } else {
        QVulkanWindow::mouseMoveEvent(event);
    }
}

void QuantiloomVulkanWindow::wheelEvent(QWheelEvent* event) {
    if (m_renderer) {
        float delta = event->angleDelta().y() / 120.0f;
        m_renderer->zoomCamera(delta);
        event->accept();
    } else {
        QVulkanWindow::wheelEvent(event);
    }
}
