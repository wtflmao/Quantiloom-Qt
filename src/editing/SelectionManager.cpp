/**
 * @file SelectionManager.cpp
 * @brief Selection state management implementation
 */

#include "SelectionManager.hpp"

#include <scene/Scene.hpp>
#include <scene/Mesh.hpp>

SelectionManager::SelectionManager(QObject* parent)
    : QObject(parent)
{
}

int SelectionManager::primarySelection() const {
    if (m_selectedNodes.isEmpty()) {
        return -1;
    }
    // Return first element (deterministic order via QSet iteration)
    return *m_selectedNodes.constBegin();
}

void SelectionManager::select(int nodeIndex, bool addToSelection) {
    if (nodeIndex < 0) {
        return;
    }

    if (!addToSelection) {
        // Single selection mode - clear others
        if (m_selectedNodes.size() == 1 && m_selectedNodes.contains(nodeIndex)) {
            // Already selected and only selection, no change
            return;
        }
        m_selectedNodes.clear();
    }

    if (!m_selectedNodes.contains(nodeIndex)) {
        m_selectedNodes.insert(nodeIndex);
        emit selectionChanged(m_selectedNodes);
        emit nodeSelected(nodeIndex);
    }
}

void SelectionManager::selectMultiple(const QSet<int>& nodeIndices) {
    if (nodeIndices == m_selectedNodes) {
        return;  // No change
    }

    m_selectedNodes = nodeIndices;
    emit selectionChanged(m_selectedNodes);
}

void SelectionManager::deselect(int nodeIndex) {
    if (m_selectedNodes.remove(nodeIndex)) {
        emit selectionChanged(m_selectedNodes);
        if (m_selectedNodes.isEmpty()) {
            emit selectionCleared();
        }
    }
}

void SelectionManager::clearSelection() {
    if (!m_selectedNodes.isEmpty()) {
        m_selectedNodes.clear();
        emit selectionChanged(m_selectedNodes);
        emit selectionCleared();
    }
}

void SelectionManager::toggleSelection(int nodeIndex) {
    if (nodeIndex < 0) {
        return;
    }

    if (m_selectedNodes.contains(nodeIndex)) {
        m_selectedNodes.remove(nodeIndex);
    } else {
        m_selectedNodes.insert(nodeIndex);
    }

    emit selectionChanged(m_selectedNodes);

    if (m_selectedNodes.isEmpty()) {
        emit selectionCleared();
    }
}

glm::vec3 SelectionManager::computeSelectionCenter(const quantiloom::Scene* scene) const {
    if (!scene || m_selectedNodes.isEmpty()) {
        return glm::vec3(0.0f);
    }

    glm::vec3 center(0.0f);
    int count = 0;

    for (int nodeIndex : m_selectedNodes) {
        if (nodeIndex >= 0 && static_cast<size_t>(nodeIndex) < scene->nodes.size()) {
            const auto& node = scene->nodes[static_cast<size_t>(nodeIndex)];
            // Extract translation from transform matrix
            glm::vec3 position(node.transform[3]);
            center += position;
            ++count;
        }
    }

    if (count > 0) {
        center /= static_cast<float>(count);
    }

    return center;
}

void SelectionManager::computeSelectionBounds(const quantiloom::Scene* scene,
                                               glm::vec3& outMin, glm::vec3& outMax) const {
    outMin = glm::vec3(std::numeric_limits<float>::max());
    outMax = glm::vec3(std::numeric_limits<float>::lowest());

    if (!scene || m_selectedNodes.isEmpty()) {
        outMin = outMax = glm::vec3(0.0f);
        return;
    }

    for (int nodeIndex : m_selectedNodes) {
        if (nodeIndex >= 0 && static_cast<size_t>(nodeIndex) < scene->nodes.size()) {
            const auto& node = scene->nodes[static_cast<size_t>(nodeIndex)];

            if (node.meshIndex < scene->meshes.size()) {
                const auto& mesh = scene->meshes[node.meshIndex];

                glm::vec3 meshMin, meshMax;
                mesh.ComputeBounds(meshMin, meshMax);

                // Transform bounds to world space (approximate with 8 corners)
                glm::vec4 corners[8] = {
                    {meshMin.x, meshMin.y, meshMin.z, 1.0f},
                    {meshMax.x, meshMin.y, meshMin.z, 1.0f},
                    {meshMin.x, meshMax.y, meshMin.z, 1.0f},
                    {meshMax.x, meshMax.y, meshMin.z, 1.0f},
                    {meshMin.x, meshMin.y, meshMax.z, 1.0f},
                    {meshMax.x, meshMin.y, meshMax.z, 1.0f},
                    {meshMin.x, meshMax.y, meshMax.z, 1.0f},
                    {meshMax.x, meshMax.y, meshMax.z, 1.0f}
                };

                for (const auto& corner : corners) {
                    glm::vec4 worldCorner = node.transform * corner;
                    glm::vec3 pos(worldCorner);
                    outMin = glm::min(outMin, pos);
                    outMax = glm::max(outMax, pos);
                }
            }
        }
    }

    // Fallback if nothing was computed
    if (outMin.x > outMax.x) {
        outMin = outMax = glm::vec3(0.0f);
    }
}
