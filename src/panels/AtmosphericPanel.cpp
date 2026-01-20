/**
 * @file AtmosphericPanel.cpp
 * @brief Panel for atmospheric scattering configuration - Implementation
 *
 * @author wtflmao
 */

#include "AtmosphericPanel.hpp"

#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QFormLayout>
#include <QComboBox>
#include <QDoubleSpinBox>
#include <QGroupBox>
#include <QCheckBox>
#include <QLabel>

AtmosphericPanel::AtmosphericPanel(QWidget* parent)
    : QWidget(parent)
{
    setupUi();
}

void AtmosphericPanel::setupUi() {
    auto* mainLayout = new QVBoxLayout(this);

    // Enable checkbox
    m_enabledCheck = new QCheckBox(tr("Enable Atmospheric Scattering"));
    m_enabledCheck->setChecked(false);
    mainLayout->addWidget(m_enabledCheck);

    // Preset selector
    auto* presetLayout = new QHBoxLayout();
    presetLayout->addWidget(new QLabel(tr("Preset:")));
    m_presetCombo = new QComboBox();
    m_presetCombo->addItem(tr("Disabled"), "disabled");
    m_presetCombo->addItem(tr("Clear Day"), "clear_day");
    m_presetCombo->addItem(tr("Hazy"), "hazy");
    m_presetCombo->addItem(tr("Polluted Urban"), "polluted_urban");
    m_presetCombo->addItem(tr("Mountain Top"), "mountain_top");
    m_presetCombo->addItem(tr("Mars"), "mars");
    presetLayout->addWidget(m_presetCombo, 1);
    mainLayout->addLayout(presetLayout);

    // Advanced parameters group (collapsible)
    m_advancedGroup = new QGroupBox(tr("Advanced Parameters"));
    m_advancedGroup->setCheckable(true);
    m_advancedGroup->setChecked(false);

    auto* advancedLayout = new QFormLayout(m_advancedGroup);

    // Rayleigh scattering section
    auto* rayleighLabel = new QLabel(tr("<b>Rayleigh Scattering</b>"));
    advancedLayout->addRow(rayleighLabel);

    m_rayleighBeta = new QDoubleSpinBox();
    m_rayleighBeta->setRange(0.0, 1e-4);
    m_rayleighBeta->setDecimals(8);
    m_rayleighBeta->setSingleStep(1e-7);
    m_rayleighBeta->setSuffix(" 1/m");
    advancedLayout->addRow(tr("Beta (550nm):"), m_rayleighBeta);

    m_rayleighScaleHeight = new QDoubleSpinBox();
    m_rayleighScaleHeight->setRange(100.0, 50000.0);
    m_rayleighScaleHeight->setSingleStep(100.0);
    m_rayleighScaleHeight->setSuffix(" m");
    advancedLayout->addRow(tr("Scale Height:"), m_rayleighScaleHeight);

    // Mie scattering section
    auto* mieLabel = new QLabel(tr("<b>Mie Scattering</b>"));
    advancedLayout->addRow(mieLabel);

    m_mieBeta = new QDoubleSpinBox();
    m_mieBeta->setRange(0.0, 1e-3);
    m_mieBeta->setDecimals(8);
    m_mieBeta->setSingleStep(1e-7);
    m_mieBeta->setSuffix(" 1/m");
    advancedLayout->addRow(tr("Beta (550nm):"), m_mieBeta);

    m_mieScaleHeight = new QDoubleSpinBox();
    m_mieScaleHeight->setRange(100.0, 10000.0);
    m_mieScaleHeight->setSingleStep(100.0);
    m_mieScaleHeight->setSuffix(" m");
    advancedLayout->addRow(tr("Scale Height:"), m_mieScaleHeight);

    m_mieG = new QDoubleSpinBox();
    m_mieG->setRange(-1.0, 1.0);
    m_mieG->setDecimals(3);
    m_mieG->setSingleStep(0.01);
    advancedLayout->addRow(tr("Asymmetry (g):"), m_mieG);

    m_mieAlpha = new QDoubleSpinBox();
    m_mieAlpha->setRange(0.0, 4.0);
    m_mieAlpha->setDecimals(3);
    m_mieAlpha->setSingleStep(0.01);
    advancedLayout->addRow(tr("Angstrom (alpha):"), m_mieAlpha);

    // Atmosphere parameters
    auto* atmosphereLabel = new QLabel(tr("<b>Atmosphere</b>"));
    advancedLayout->addRow(atmosphereLabel);

    m_planetRadius = new QDoubleSpinBox();
    m_planetRadius->setRange(1e5, 1e8);
    m_planetRadius->setDecimals(0);
    m_planetRadius->setSingleStep(1e5);
    m_planetRadius->setSuffix(" m");
    advancedLayout->addRow(tr("Planet Radius:"), m_planetRadius);

    m_atmosphereHeight = new QDoubleSpinBox();
    m_atmosphereHeight->setRange(1000.0, 200000.0);
    m_atmosphereHeight->setDecimals(0);
    m_atmosphereHeight->setSingleStep(1000.0);
    m_atmosphereHeight->setSuffix(" m");
    advancedLayout->addRow(tr("Atmosphere Height:"), m_atmosphereHeight);

    mainLayout->addWidget(m_advancedGroup);
    mainLayout->addStretch();

    // Connect signals
    connect(m_enabledCheck, &QCheckBox::toggled,
            this, &AtmosphericPanel::onEnabledChanged);
    connect(m_presetCombo, QOverload<int>::of(&QComboBox::currentIndexChanged),
            this, &AtmosphericPanel::onPresetChanged);

    // Connect advanced params
    connect(m_rayleighBeta, QOverload<double>::of(&QDoubleSpinBox::valueChanged),
            this, &AtmosphericPanel::onAdvancedParamChanged);
    connect(m_rayleighScaleHeight, QOverload<double>::of(&QDoubleSpinBox::valueChanged),
            this, &AtmosphericPanel::onAdvancedParamChanged);
    connect(m_mieBeta, QOverload<double>::of(&QDoubleSpinBox::valueChanged),
            this, &AtmosphericPanel::onAdvancedParamChanged);
    connect(m_mieScaleHeight, QOverload<double>::of(&QDoubleSpinBox::valueChanged),
            this, &AtmosphericPanel::onAdvancedParamChanged);
    connect(m_mieG, QOverload<double>::of(&QDoubleSpinBox::valueChanged),
            this, &AtmosphericPanel::onAdvancedParamChanged);
    connect(m_mieAlpha, QOverload<double>::of(&QDoubleSpinBox::valueChanged),
            this, &AtmosphericPanel::onAdvancedParamChanged);
    connect(m_planetRadius, QOverload<double>::of(&QDoubleSpinBox::valueChanged),
            this, &AtmosphericPanel::onAdvancedParamChanged);
    connect(m_atmosphereHeight, QOverload<double>::of(&QDoubleSpinBox::valueChanged),
            this, &AtmosphericPanel::onAdvancedParamChanged);

    // Set default values
    setAtmosphericConfig(quantiloom::AtmosphericConfig::Disabled());
}

void AtmosphericPanel::setPreset(const QString& preset) {
    QString lowerPreset = preset.toLower();

    for (int i = 0; i < m_presetCombo->count(); ++i) {
        if (m_presetCombo->itemData(i).toString() == lowerPreset) {
            m_updatingUi = true;
            m_presetCombo->setCurrentIndex(i);
            m_enabledCheck->setChecked(lowerPreset != "disabled");
            m_updatingUi = false;

            // Apply the preset config
            onPresetChanged(i);
            return;
        }
    }

    // Default to disabled
    m_presetCombo->setCurrentIndex(0);
}

QString AtmosphericPanel::preset() const {
    return m_presetCombo->currentData().toString();
}

void AtmosphericPanel::setAtmosphericConfig(const quantiloom::AtmosphericConfig& config) {
    m_config = config;
    m_updatingUi = true;

    m_enabledCheck->setChecked(config.IsEnabled());
    updateAdvancedParamsFromConfig(config);

    m_updatingUi = false;
}

quantiloom::AtmosphericConfig AtmosphericPanel::getAtmosphericConfig() const {
    return m_config;
}

void AtmosphericPanel::onPresetChanged(int index) {
    if (m_updatingUi) return;

    QString presetName = m_presetCombo->itemData(index).toString();

    // Create config from preset
    if (presetName == "clear_day") {
        m_config = quantiloom::AtmosphericConfig::ClearDay();
    } else if (presetName == "hazy") {
        m_config = quantiloom::AtmosphericConfig::Hazy();
    } else if (presetName == "polluted_urban") {
        m_config = quantiloom::AtmosphericConfig::PollutedUrban();
    } else if (presetName == "mountain_top") {
        m_config = quantiloom::AtmosphericConfig::MountainTop();
    } else if (presetName == "mars") {
        m_config = quantiloom::AtmosphericConfig::Mars();
    } else {
        m_config = quantiloom::AtmosphericConfig::Disabled();
    }

    // Update enabled checkbox
    m_updatingUi = true;
    m_enabledCheck->setChecked(m_config.IsEnabled());
    updateAdvancedParamsFromConfig(m_config);
    m_updatingUi = false;

    emit presetChanged(presetName);
    emit configChanged(m_config);
}

void AtmosphericPanel::onAdvancedParamChanged() {
    if (m_updatingUi) return;

    // Update config from UI values (using correct member names)
    m_config.rayleigh_beta_550nm = static_cast<float>(m_rayleighBeta->value());
    m_config.rayleigh_scale_height = static_cast<float>(m_rayleighScaleHeight->value());

    m_config.mie_beta_550nm = static_cast<float>(m_mieBeta->value());
    m_config.mie_scale_height = static_cast<float>(m_mieScaleHeight->value());
    m_config.mie_g = static_cast<float>(m_mieG->value());
    m_config.mie_alpha = static_cast<float>(m_mieAlpha->value());

    m_config.planet_radius = static_cast<float>(m_planetRadius->value());
    m_config.atmosphere_height = static_cast<float>(m_atmosphereHeight->value());

    emit configChanged(m_config);
}

void AtmosphericPanel::onEnabledChanged(bool enabled) {
    if (m_updatingUi) return;

    if (!enabled) {
        // Switch to disabled preset
        m_updatingUi = true;
        m_presetCombo->setCurrentIndex(0);  // Disabled
        m_config = quantiloom::AtmosphericConfig::Disabled();
        updateAdvancedParamsFromConfig(m_config);
        m_updatingUi = false;

        emit presetChanged("disabled");
        emit configChanged(m_config);
    } else {
        // Switch to Clear Day as default enabled preset
        m_updatingUi = true;
        m_presetCombo->setCurrentIndex(1);  // Clear Day
        m_config = quantiloom::AtmosphericConfig::ClearDay();
        updateAdvancedParamsFromConfig(m_config);
        m_updatingUi = false;

        emit presetChanged("clear_day");
        emit configChanged(m_config);
    }
}

void AtmosphericPanel::updateAdvancedParamsFromConfig(const quantiloom::AtmosphericConfig& config) {
    blockSignalsForUpdate(true);

    // Use correct member names from SDK
    m_rayleighBeta->setValue(static_cast<double>(config.rayleigh_beta_550nm));
    m_rayleighScaleHeight->setValue(static_cast<double>(config.rayleigh_scale_height));

    m_mieBeta->setValue(static_cast<double>(config.mie_beta_550nm));
    m_mieScaleHeight->setValue(static_cast<double>(config.mie_scale_height));
    m_mieG->setValue(static_cast<double>(config.mie_g));
    m_mieAlpha->setValue(static_cast<double>(config.mie_alpha));

    m_planetRadius->setValue(static_cast<double>(config.planet_radius));
    m_atmosphereHeight->setValue(static_cast<double>(config.atmosphere_height));

    blockSignalsForUpdate(false);
}

void AtmosphericPanel::blockSignalsForUpdate(bool block) {
    m_rayleighBeta->blockSignals(block);
    m_rayleighScaleHeight->blockSignals(block);
    m_mieBeta->blockSignals(block);
    m_mieScaleHeight->blockSignals(block);
    m_mieG->blockSignals(block);
    m_mieAlpha->blockSignals(block);
    m_planetRadius->blockSignals(block);
    m_atmosphereHeight->blockSignals(block);
}
