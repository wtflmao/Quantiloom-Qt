/**
 * @file DebugVisualizationPanel.hpp
 * @brief Debug visualization mode selection panel for pipeline debugging
 *
 * Provides UI for selecting debug visualization modes to inspect
 * intermediate rendering data at each pipeline stage.
 */

#pragma once

#include <QWidget>
#include <core/Types.hpp>

QT_BEGIN_NAMESPACE
class QComboBox;
class QLabel;
class QGroupBox;
QT_END_NAMESPACE

/**
 * @class DebugVisualizationPanel
 * @brief Panel for selecting debug visualization modes
 *
 * Groups debug modes by pipeline stage:
 * - Geometry: normals, UVs, positions, material IDs
 * - Material: albedo, metallic, roughness, emissive
 * - Lighting: NdotL, NdotV, direct sun, diffuse
 * - BRDF: Fresnel F0, full BRDF
 * - IBL: prefiltered env, BRDF LUT, specular
 * - Spectral: XYZ tristimulus, pre-correction RGB
 * - IR: temperature, emissivity, emission/reflection
 */
class DebugVisualizationPanel : public QWidget {
    Q_OBJECT

public:
    explicit DebugVisualizationPanel(QWidget* parent = nullptr);

    void setDebugMode(quantiloom::DebugVisualizationMode mode);
    [[nodiscard]] quantiloom::DebugVisualizationMode debugMode() const { return m_mode; }

signals:
    void debugModeChanged(quantiloom::DebugVisualizationMode mode);

private slots:
    void onModeChanged(int index);

private:
    void setupUi();
    void updateDescription(quantiloom::DebugVisualizationMode mode);

    quantiloom::DebugVisualizationMode m_mode = quantiloom::DebugVisualizationMode::None;

    QComboBox* m_modeCombo = nullptr;
    QLabel* m_description = nullptr;
    QLabel* m_categoryLabel = nullptr;
};
