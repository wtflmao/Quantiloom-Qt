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

QuantiloomVulkanWindow::QuantiloomVulkanWindow(QWindow* parent)
    : QVulkanWindow(parent)
{
    // Request Vulkan 1.3 for ray tracing and dynamic rendering
    // Note: The actual API version is set on QVulkanInstance in main.cpp
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
