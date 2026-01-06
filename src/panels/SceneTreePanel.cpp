/**
 * @file SceneTreePanel.cpp
 * @brief Scene hierarchy tree view implementation
 */

#include "SceneTreePanel.hpp"

#include <QTreeWidget>
#include <QVBoxLayout>
#include <QHeaderView>

// SDK headers
#include <scene/Scene.hpp>
#include <scene/Material.hpp>

SceneTreePanel::SceneTreePanel(QWidget* parent)
    : QWidget(parent)
{
    auto* layout = new QVBoxLayout(this);
    layout->setContentsMargins(0, 0, 0, 0);

    m_tree = new QTreeWidget();
    m_tree->setHeaderLabels({tr("Name"), tr("Type")});
    m_tree->header()->setStretchLastSection(true);
    m_tree->setAlternatingRowColors(true);
    m_tree->setSelectionMode(QAbstractItemView::SingleSelection);

    layout->addWidget(m_tree);

    connect(m_tree, &QTreeWidget::itemClicked, this, &SceneTreePanel::onItemClicked);
}

void SceneTreePanel::setScene(const quantiloom::Scene* scene) {
    m_scene = scene;
    populateTree();
}

void SceneTreePanel::refresh() {
    populateTree();
}

void SceneTreePanel::populateTree() {
    m_tree->clear();

    if (!m_scene) {
        return;
    }

    // Root: Scene name
    auto* sceneRoot = new QTreeWidgetItem(m_tree);
    sceneRoot->setText(0, QString::fromStdString(m_scene->name));
    sceneRoot->setText(1, tr("Scene"));
    sceneRoot->setExpanded(true);

    // Nodes section
    auto* nodesRoot = new QTreeWidgetItem(sceneRoot);
    nodesRoot->setText(0, tr("Nodes (%1)").arg(m_scene->nodes.size()));
    nodesRoot->setText(1, tr("Group"));
    nodesRoot->setExpanded(true);

    for (size_t i = 0; i < m_scene->nodes.size(); ++i) {
        const auto& node = m_scene->nodes[i];
        auto* nodeItem = new QTreeWidgetItem(nodesRoot);

        QString nodeName = QString("Node %1").arg(i);
        if (node.meshIndex < m_scene->meshes.size()) {
            const auto& mesh = m_scene->meshes[node.meshIndex];
            if (!mesh.name.empty()) {
                nodeName = QString::fromStdString(mesh.name);
            }
        }

        nodeItem->setText(0, nodeName);
        nodeItem->setText(1, tr("Node"));
        nodeItem->setData(0, Qt::UserRole, static_cast<int>(i));
        nodeItem->setData(0, Qt::UserRole + 1, QString("node"));
    }

    // Materials section
    auto* materialsRoot = new QTreeWidgetItem(sceneRoot);
    materialsRoot->setText(0, tr("Materials (%1)").arg(m_scene->materials.size()));
    materialsRoot->setText(1, tr("Group"));
    materialsRoot->setExpanded(true);

    for (size_t i = 0; i < m_scene->materials.size(); ++i) {
        const auto& mat = m_scene->materials[i];
        auto* matItem = new QTreeWidgetItem(materialsRoot);

        QString matName = mat.name.empty()
            ? QString("Material %1").arg(i)
            : QString::fromStdString(mat.name);

        matItem->setText(0, matName);
        matItem->setText(1, tr("Material"));
        matItem->setData(0, Qt::UserRole, static_cast<int>(i));
        matItem->setData(0, Qt::UserRole + 1, QString("material"));
    }

    // Textures section
    auto* texturesRoot = new QTreeWidgetItem(sceneRoot);
    texturesRoot->setText(0, tr("Textures (%1)").arg(m_scene->textures.size()));
    texturesRoot->setText(1, tr("Group"));

    for (size_t i = 0; i < m_scene->textures.size(); ++i) {
        const auto& tex = m_scene->textures[i];
        auto* texItem = new QTreeWidgetItem(texturesRoot);

        QString texName = tex.name.empty()
            ? QString("Texture %1").arg(i)
            : QString::fromStdString(tex.name);

        texItem->setText(0, texName);
        texItem->setText(1, QString("%1x%2").arg(tex.width).arg(tex.height));
    }

    // Statistics
    auto* statsRoot = new QTreeWidgetItem(sceneRoot);
    statsRoot->setText(0, tr("Statistics"));
    statsRoot->setText(1, tr("Info"));

    auto addStat = [&](const QString& name, const QString& value) {
        auto* item = new QTreeWidgetItem(statsRoot);
        item->setText(0, name);
        item->setText(1, value);
    };

    addStat(tr("Meshes"), QString::number(m_scene->meshes.size()));
    addStat(tr("Triangles"), QString::number(m_scene->GetTotalTriangleCount()));
    addStat(tr("Vertices"), QString::number(m_scene->GetTotalVertexCount()));

    m_tree->resizeColumnToContents(0);
}

void SceneTreePanel::onItemClicked(QTreeWidgetItem* item, int /*column*/) {
    QString type = item->data(0, Qt::UserRole + 1).toString();
    int index = item->data(0, Qt::UserRole).toInt();

    if (type == "node") {
        emit nodeSelected(index);
    } else if (type == "material") {
        emit materialSelected(index);
    }
}
