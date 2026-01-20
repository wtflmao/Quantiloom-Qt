/**
 * @file AtmosphericPanel.hpp
 * @brief Panel for atmospheric scattering configuration
 *
 * @author wtflmao
 */

#pragma once

#include <QWidget>
#include <renderer/AtmosphericConfig.hpp>

QT_BEGIN_NAMESPACE
class QComboBox;
class QDoubleSpinBox;
class QGroupBox;
class QCheckBox;
class QLabel;
QT_END_NAMESPACE

/**
 * @class AtmosphericPanel
 * @brief UI panel for atmospheric scattering configuration
 *
 * Provides preset selection and advanced parameter tuning for
 * atmospheric effects like Rayleigh/Mie scattering.
 */
class AtmosphericPanel : public QWidget {
    Q_OBJECT

public:
    explicit AtmosphericPanel(QWidget* parent = nullptr);
    ~AtmosphericPanel() override = default;

    /**
     * @brief Set current preset from config file
     * @param preset Preset name string
     */
    void setPreset(const QString& preset);

    /**
     * @brief Get current preset name
     */
    QString preset() const;

    /**
     * @brief Set full atmospheric configuration
     * @param config Configuration to display
     */
    void setAtmosphericConfig(const quantiloom::AtmosphericConfig& config);

    /**
     * @brief Get current atmospheric configuration
     */
    quantiloom::AtmosphericConfig getAtmosphericConfig() const;

signals:
    /**
     * @brief Emitted when preset changes
     * @param preset New preset name
     */
    void presetChanged(const QString& preset);

    /**
     * @brief Emitted when any configuration value changes
     * @param config Updated configuration
     */
    void configChanged(const quantiloom::AtmosphericConfig& config);

private slots:
    void onPresetChanged(int index);
    void onAdvancedParamChanged();
    void onEnabledChanged(bool enabled);

private:
    void setupUi();
    void updateAdvancedParamsFromConfig(const quantiloom::AtmosphericConfig& config);
    void blockSignalsForUpdate(bool block);

    // Preset selector
    QComboBox* m_presetCombo = nullptr;
    QCheckBox* m_enabledCheck = nullptr;

    // Advanced parameters group
    QGroupBox* m_advancedGroup = nullptr;

    // Rayleigh scattering
    QDoubleSpinBox* m_rayleighBeta = nullptr;
    QDoubleSpinBox* m_rayleighScaleHeight = nullptr;

    // Mie scattering
    QDoubleSpinBox* m_mieBeta = nullptr;
    QDoubleSpinBox* m_mieScaleHeight = nullptr;
    QDoubleSpinBox* m_mieG = nullptr;
    QDoubleSpinBox* m_mieAlpha = nullptr;

    // Atmosphere parameters
    QDoubleSpinBox* m_planetRadius = nullptr;
    QDoubleSpinBox* m_atmosphereHeight = nullptr;

    // Current config
    quantiloom::AtmosphericConfig m_config;
    bool m_updatingUi = false;
};
