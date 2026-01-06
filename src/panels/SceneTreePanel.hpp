/**
 * @file SceneTreePanel.hpp
 * @brief Scene hierarchy tree view panel
 */

#pragma once

#include <QWidget>

QT_BEGIN_NAMESPACE
class QTreeWidget;
class QTreeWidgetItem;
QT_END_NAMESPACE

namespace quantiloom {
class Scene;
}

/**
 * @class SceneTreePanel
 * @brief Displays scene hierarchy (meshes, nodes, materials)
 */
class SceneTreePanel : public QWidget {
    Q_OBJECT

public:
    explicit SceneTreePanel(QWidget* parent = nullptr);

    void setScene(const quantiloom::Scene* scene);
    void refresh();

signals:
    void nodeSelected(int nodeIndex);
    void materialSelected(int materialIndex);

private slots:
    void onItemClicked(QTreeWidgetItem* item, int column);

private:
    void populateTree();

    QTreeWidget* m_tree = nullptr;
    const quantiloom::Scene* m_scene = nullptr;
};
