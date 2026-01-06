/**
 * @file MaterialEditorPanel.hpp
 * @brief PBR material property editor panel
 */

#pragma once

#include <QWidget>
#include <glm/glm.hpp>

QT_BEGIN_NAMESPACE
class QDoubleSpinBox;
class QSlider;
class QLabel;
class QPushButton;
class QGroupBox;
QT_END_NAMESPACE

namespace quantiloom {
struct Material;
}

/**
 * @class MaterialEditorPanel
 * @brief Editor for PBR material properties
 */
class MaterialEditorPanel : public QWidget {
    Q_OBJECT

public:
    explicit MaterialEditorPanel(QWidget* parent = nullptr);

    void setMaterial(int index, const quantiloom::Material* material);
    void clear();

signals:
    void materialChanged(int index, const quantiloom::Material& material);

private slots:
    void onBaseColorClicked();
    void onMetallicChanged(int value);
    void onRoughnessChanged(int value);
    void onEmissiveChanged();
    void applyChanges();

private:
    void setupUi();
    void updateColorButton(QPushButton* btn, const glm::vec3& color);

    int m_currentIndex = -1;
    const quantiloom::Material* m_currentMaterial = nullptr;

    // UI elements
    QLabel* m_materialName = nullptr;
    QPushButton* m_baseColorBtn = nullptr;
    QSlider* m_metallicSlider = nullptr;
    QLabel* m_metallicLabel = nullptr;
    QSlider* m_roughnessSlider = nullptr;
    QLabel* m_roughnessLabel = nullptr;
    QDoubleSpinBox* m_emissiveR = nullptr;
    QDoubleSpinBox* m_emissiveG = nullptr;
    QDoubleSpinBox* m_emissiveB = nullptr;

    // Current values
    glm::vec4 m_baseColor{1.0f};
    float m_metallic = 0.0f;
    float m_roughness = 1.0f;
    glm::vec3 m_emissive{0.0f};
};
