/**
 * @file MaterialEditorPanel.cpp
 * @brief PBR material property editor implementation
 */

#include "MaterialEditorPanel.hpp"

#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QGroupBox>
#include <QLabel>
#include <QSlider>
#include <QDoubleSpinBox>
#include <QPushButton>
#include <QColorDialog>
#include <QFormLayout>

#include <scene/Material.hpp>

MaterialEditorPanel::MaterialEditorPanel(QWidget* parent)
    : QWidget(parent)
{
    setupUi();
}

void MaterialEditorPanel::setupUi() {
    auto* mainLayout = new QVBoxLayout(this);
    mainLayout->setContentsMargins(4, 4, 4, 4);
    mainLayout->setSpacing(8);

    // Material name
    m_materialName = new QLabel(tr("No material selected"));
    m_materialName->setStyleSheet("font-weight: bold;");
    mainLayout->addWidget(m_materialName);

    // Base Color group
    auto* colorGroup = new QGroupBox(tr("Base Color"));
    auto* colorLayout = new QHBoxLayout(colorGroup);
    m_baseColorBtn = new QPushButton();
    m_baseColorBtn->setFixedSize(80, 30);
    m_baseColorBtn->setStyleSheet("background-color: white;");
    connect(m_baseColorBtn, &QPushButton::clicked, this, &MaterialEditorPanel::onBaseColorClicked);
    colorLayout->addWidget(m_baseColorBtn);
    colorLayout->addStretch();
    mainLayout->addWidget(colorGroup);

    // Metallic-Roughness group
    auto* pbrGroup = new QGroupBox(tr("PBR Properties"));
    auto* pbrLayout = new QFormLayout(pbrGroup);

    // Metallic
    auto* metallicRow = new QHBoxLayout();
    m_metallicSlider = new QSlider(Qt::Horizontal);
    m_metallicSlider->setRange(0, 100);
    m_metallicSlider->setValue(0);
    m_metallicLabel = new QLabel("0.00");
    m_metallicLabel->setFixedWidth(40);
    connect(m_metallicSlider, &QSlider::valueChanged, this, &MaterialEditorPanel::onMetallicChanged);
    metallicRow->addWidget(m_metallicSlider);
    metallicRow->addWidget(m_metallicLabel);
    pbrLayout->addRow(tr("Metallic:"), metallicRow);

    // Roughness
    auto* roughnessRow = new QHBoxLayout();
    m_roughnessSlider = new QSlider(Qt::Horizontal);
    m_roughnessSlider->setRange(0, 100);
    m_roughnessSlider->setValue(100);
    m_roughnessLabel = new QLabel("1.00");
    m_roughnessLabel->setFixedWidth(40);
    connect(m_roughnessSlider, &QSlider::valueChanged, this, &MaterialEditorPanel::onRoughnessChanged);
    roughnessRow->addWidget(m_roughnessSlider);
    roughnessRow->addWidget(m_roughnessLabel);
    pbrLayout->addRow(tr("Roughness:"), roughnessRow);

    mainLayout->addWidget(pbrGroup);

    // Emissive group
    auto* emissiveGroup = new QGroupBox(tr("Emissive"));
    auto* emissiveLayout = new QHBoxLayout(emissiveGroup);

    auto createSpinBox = [this]() {
        auto* spin = new QDoubleSpinBox();
        spin->setRange(0.0, 100.0);
        spin->setSingleStep(0.1);
        spin->setDecimals(2);
        connect(spin, QOverload<double>::of(&QDoubleSpinBox::valueChanged),
                this, &MaterialEditorPanel::onEmissiveChanged);
        return spin;
    };

    emissiveLayout->addWidget(new QLabel("R:"));
    m_emissiveR = createSpinBox();
    emissiveLayout->addWidget(m_emissiveR);

    emissiveLayout->addWidget(new QLabel("G:"));
    m_emissiveG = createSpinBox();
    emissiveLayout->addWidget(m_emissiveG);

    emissiveLayout->addWidget(new QLabel("B:"));
    m_emissiveB = createSpinBox();
    emissiveLayout->addWidget(m_emissiveB);

    mainLayout->addWidget(emissiveGroup);

    mainLayout->addStretch();

    // Disable by default
    setEnabled(false);
}

void MaterialEditorPanel::setMaterial(int index, const quantiloom::Material* material) {
    m_currentIndex = index;
    m_currentMaterial = material;

    if (!material) {
        clear();
        return;
    }

    setEnabled(true);

    // Material name
    QString name = material->name.empty()
        ? QString("Material %1").arg(index)
        : QString::fromStdString(material->name);
    m_materialName->setText(name);

    // Base color
    m_baseColor = material->baseColorFactor;
    updateColorButton(m_baseColorBtn, glm::vec3(m_baseColor));

    // Metallic
    m_metallic = material->metallicFactor;
    m_metallicSlider->blockSignals(true);
    m_metallicSlider->setValue(static_cast<int>(m_metallic * 100));
    m_metallicSlider->blockSignals(false);
    m_metallicLabel->setText(QString::number(m_metallic, 'f', 2));

    // Roughness
    m_roughness = material->roughnessFactor;
    m_roughnessSlider->blockSignals(true);
    m_roughnessSlider->setValue(static_cast<int>(m_roughness * 100));
    m_roughnessSlider->blockSignals(false);
    m_roughnessLabel->setText(QString::number(m_roughness, 'f', 2));

    // Emissive
    m_emissive = material->emissiveFactor;
    m_emissiveR->blockSignals(true);
    m_emissiveG->blockSignals(true);
    m_emissiveB->blockSignals(true);
    m_emissiveR->setValue(m_emissive.r);
    m_emissiveG->setValue(m_emissive.g);
    m_emissiveB->setValue(m_emissive.b);
    m_emissiveR->blockSignals(false);
    m_emissiveG->blockSignals(false);
    m_emissiveB->blockSignals(false);
}

void MaterialEditorPanel::clear() {
    m_currentIndex = -1;
    m_currentMaterial = nullptr;
    m_materialName->setText(tr("No material selected"));
    setEnabled(false);
}

void MaterialEditorPanel::updateColorButton(QPushButton* btn, const glm::vec3& color) {
    int r = static_cast<int>(color.r * 255);
    int g = static_cast<int>(color.g * 255);
    int b = static_cast<int>(color.b * 255);
    btn->setStyleSheet(QString("background-color: rgb(%1, %2, %3);").arg(r).arg(g).arg(b));
}

void MaterialEditorPanel::onBaseColorClicked() {
    QColor initial = QColor::fromRgbF(m_baseColor.r, m_baseColor.g, m_baseColor.b);
    QColor color = QColorDialog::getColor(initial, this, tr("Select Base Color"));

    if (color.isValid()) {
        m_baseColor = glm::vec4(color.redF(), color.greenF(), color.blueF(), m_baseColor.a);
        updateColorButton(m_baseColorBtn, glm::vec3(m_baseColor));
        applyChanges();
    }
}

void MaterialEditorPanel::onMetallicChanged(int value) {
    m_metallic = value / 100.0f;
    m_metallicLabel->setText(QString::number(m_metallic, 'f', 2));
    applyChanges();
}

void MaterialEditorPanel::onRoughnessChanged(int value) {
    m_roughness = value / 100.0f;
    m_roughnessLabel->setText(QString::number(m_roughness, 'f', 2));
    applyChanges();
}

void MaterialEditorPanel::onEmissiveChanged() {
    m_emissive = glm::vec3(
        static_cast<float>(m_emissiveR->value()),
        static_cast<float>(m_emissiveG->value()),
        static_cast<float>(m_emissiveB->value())
    );
    applyChanges();
}

void MaterialEditorPanel::applyChanges() {
    if (m_currentIndex < 0 || !m_currentMaterial) {
        return;
    }

    // Create modified material
    quantiloom::Material modified = *m_currentMaterial;
    modified.baseColorFactor = m_baseColor;
    modified.metallicFactor = m_metallic;
    modified.roughnessFactor = m_roughness;
    modified.emissiveFactor = m_emissive;

    emit materialChanged(m_currentIndex, modified);
}
