/**
 * @file SensorPanel.cpp
 * @brief Panel for sensor simulation configuration - Implementation
 *
 * @author wtflmao
 */

#include "SensorPanel.hpp"

#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QFormLayout>
#include <QDoubleSpinBox>
#include <QSpinBox>
#include <QGroupBox>
#include <QCheckBox>
#include <QComboBox>
#include <QLabel>

SensorPanel::SensorPanel(QWidget* parent)
    : QWidget(parent)
{
    setupUi();
}

void SensorPanel::setupUi() {
    auto* mainLayout = new QVBoxLayout(this);

    // Enable checkbox
    m_enabledCheck = new QCheckBox(tr("Enable Sensor Simulation"));
    m_enabledCheck->setChecked(false);
    mainLayout->addWidget(m_enabledCheck);

    // ========================================================================
    // Optics Group
    // ========================================================================
    m_opticsGroup = new QGroupBox(tr("Optics"));
    auto* opticsLayout = new QFormLayout(m_opticsGroup);

    m_focalLength = new QDoubleSpinBox();
    m_focalLength->setRange(1.0, 10000.0);
    m_focalLength->setDecimals(1);
    m_focalLength->setSingleStep(1.0);
    m_focalLength->setSuffix(" mm");
    m_focalLength->setValue(50.0);
    opticsLayout->addRow(tr("Focal Length:"), m_focalLength);

    m_fNumber = new QDoubleSpinBox();
    m_fNumber->setRange(0.5, 64.0);
    m_fNumber->setDecimals(1);
    m_fNumber->setSingleStep(0.1);
    m_fNumber->setPrefix("f/");
    m_fNumber->setValue(2.8);
    opticsLayout->addRow(tr("Aperture:"), m_fNumber);

    mainLayout->addWidget(m_opticsGroup);

    // ========================================================================
    // Detector Group
    // ========================================================================
    m_detectorGroup = new QGroupBox(tr("Detector"));
    auto* detectorLayout = new QFormLayout(m_detectorGroup);

    m_pixelPitch = new QDoubleSpinBox();
    m_pixelPitch->setRange(0.1, 100.0);
    m_pixelPitch->setDecimals(2);
    m_pixelPitch->setSingleStep(0.1);
    m_pixelPitch->setSuffix(QString::fromUtf8(" \u03BCm"));  // μm
    m_pixelPitch->setValue(5.0);
    detectorLayout->addRow(tr("Pixel Pitch:"), m_pixelPitch);

    m_quantumEfficiency = new QDoubleSpinBox();
    m_quantumEfficiency->setRange(0.0, 1.0);
    m_quantumEfficiency->setDecimals(2);
    m_quantumEfficiency->setSingleStep(0.01);
    m_quantumEfficiency->setValue(0.8);
    detectorLayout->addRow(tr("Quantum Efficiency:"), m_quantumEfficiency);

    m_wellCapacity = new QDoubleSpinBox();
    m_wellCapacity->setRange(100, 1e9);
    m_wellCapacity->setDecimals(0);
    m_wellCapacity->setSingleStep(1000);
    m_wellCapacity->setSuffix(" e-");
    m_wellCapacity->setValue(50000);
    detectorLayout->addRow(tr("Well Capacity:"), m_wellCapacity);

    m_bitDepth = new QSpinBox();
    m_bitDepth->setRange(8, 32);
    m_bitDepth->setValue(14);
    m_bitDepth->setSuffix(" bit");
    detectorLayout->addRow(tr("Bit Depth:"), m_bitDepth);

    m_integrationTime = new QDoubleSpinBox();
    m_integrationTime->setRange(0.0001, 10.0);
    m_integrationTime->setDecimals(4);
    m_integrationTime->setSingleStep(0.001);
    m_integrationTime->setSuffix(" s");
    m_integrationTime->setValue(0.01);
    detectorLayout->addRow(tr("Integration Time:"), m_integrationTime);

    mainLayout->addWidget(m_detectorGroup);

    // ========================================================================
    // Noise Group
    // ========================================================================
    m_noiseGroup = new QGroupBox(tr("Noise Model"));
    auto* noiseLayout = new QFormLayout(m_noiseGroup);

    m_readNoise = new QDoubleSpinBox();
    m_readNoise->setRange(0.0, 1000.0);
    m_readNoise->setDecimals(1);
    m_readNoise->setSingleStep(0.1);
    m_readNoise->setSuffix(" e- RMS");
    m_readNoise->setValue(10.0);
    noiseLayout->addRow(tr("Read Noise:"), m_readNoise);

    m_darkCurrent = new QDoubleSpinBox();
    m_darkCurrent->setRange(0.0, 10000.0);
    m_darkCurrent->setDecimals(1);
    m_darkCurrent->setSingleStep(1.0);
    m_darkCurrent->setSuffix(" e-/s");
    m_darkCurrent->setValue(50.0);
    noiseLayout->addRow(tr("Dark Current:"), m_darkCurrent);

    m_poissonNoise = new QCheckBox(tr("Photon Shot Noise (Poisson)"));
    m_poissonNoise->setChecked(true);
    noiseLayout->addRow(m_poissonNoise);

    m_fpnNoise = new QCheckBox(tr("Fixed Pattern Noise (FPN)"));
    m_fpnNoise->setChecked(false);
    noiseLayout->addRow(m_fpnNoise);

    mainLayout->addWidget(m_noiseGroup);

    mainLayout->addStretch();

    // Connect signals
    connect(m_enabledCheck, &QCheckBox::toggled,
            this, &SensorPanel::onEnabledChanged);

    // Optics params
    connect(m_focalLength, QOverload<double>::of(&QDoubleSpinBox::valueChanged),
            this, &SensorPanel::onParamChanged);
    connect(m_fNumber, QOverload<double>::of(&QDoubleSpinBox::valueChanged),
            this, &SensorPanel::onParamChanged);

    // Detector params
    connect(m_pixelPitch, QOverload<double>::of(&QDoubleSpinBox::valueChanged),
            this, &SensorPanel::onParamChanged);
    connect(m_quantumEfficiency, QOverload<double>::of(&QDoubleSpinBox::valueChanged),
            this, &SensorPanel::onParamChanged);
    connect(m_wellCapacity, QOverload<double>::of(&QDoubleSpinBox::valueChanged),
            this, &SensorPanel::onParamChanged);
    connect(m_bitDepth, QOverload<int>::of(&QSpinBox::valueChanged),
            this, &SensorPanel::onParamChanged);
    connect(m_integrationTime, QOverload<double>::of(&QDoubleSpinBox::valueChanged),
            this, &SensorPanel::onParamChanged);

    // Noise params
    connect(m_readNoise, QOverload<double>::of(&QDoubleSpinBox::valueChanged),
            this, &SensorPanel::onParamChanged);
    connect(m_darkCurrent, QOverload<double>::of(&QDoubleSpinBox::valueChanged),
            this, &SensorPanel::onParamChanged);
    connect(m_poissonNoise, &QCheckBox::toggled,
            this, &SensorPanel::onParamChanged);
    connect(m_fpnNoise, &QCheckBox::toggled,
            this, &SensorPanel::onParamChanged);

    // Update enabled state of groups
    m_opticsGroup->setEnabled(false);
    m_detectorGroup->setEnabled(false);
    m_noiseGroup->setEnabled(false);
}

void SensorPanel::setEnabled(bool enabled) {
    m_updatingUi = true;
    m_enabledCheck->setChecked(enabled);
    m_opticsGroup->setEnabled(enabled);
    m_detectorGroup->setEnabled(enabled);
    m_noiseGroup->setEnabled(enabled);
    m_updatingUi = false;
}

bool SensorPanel::isEnabled() const {
    return m_enabledCheck->isChecked();
}

void SensorPanel::setSensorParams(const quantiloom::SensorParams& params) {
    m_params = params;
    updateUiFromParams(params);
}

quantiloom::SensorParams SensorPanel::getSensorParams() const {
    return m_params;
}

void SensorPanel::onEnabledChanged(bool enabled) {
    m_opticsGroup->setEnabled(enabled);
    m_detectorGroup->setEnabled(enabled);
    m_noiseGroup->setEnabled(enabled);

    if (!m_updatingUi) {
        emit enabledChanged(enabled);
    }
}

void SensorPanel::onParamChanged() {
    if (m_updatingUi) return;

    // Update params from UI (using correct SDK member names)
    m_params.focalLength_mm = static_cast<float>(m_focalLength->value());
    m_params.fNumber = static_cast<float>(m_fNumber->value());

    m_params.pixelPitch_um = static_cast<float>(m_pixelPitch->value());
    m_params.quantumEfficiency = static_cast<float>(m_quantumEfficiency->value());
    m_params.wellCapacity_e = static_cast<float>(m_wellCapacity->value());
    m_params.bitDepth = static_cast<quantiloom::u32>(m_bitDepth->value());
    m_params.integrationTime_s = static_cast<float>(m_integrationTime->value());

    m_params.readNoise_e_rms = static_cast<float>(m_readNoise->value());
    m_params.darkCurrent_e_s = static_cast<float>(m_darkCurrent->value());
    m_params.enablePoissonNoise = m_poissonNoise->isChecked();
    m_params.enableFPN = m_fpnNoise->isChecked();

    emit paramsChanged(m_params);
}

void SensorPanel::updateUiFromParams(const quantiloom::SensorParams& params) {
    m_updatingUi = true;
    blockSignalsForUpdate(true);

    // Use correct SDK member names
    m_focalLength->setValue(static_cast<double>(params.focalLength_mm));
    m_fNumber->setValue(static_cast<double>(params.fNumber));

    m_pixelPitch->setValue(static_cast<double>(params.pixelPitch_um));
    m_quantumEfficiency->setValue(static_cast<double>(params.quantumEfficiency));
    m_wellCapacity->setValue(static_cast<double>(params.wellCapacity_e));
    m_bitDepth->setValue(static_cast<int>(params.bitDepth));
    m_integrationTime->setValue(static_cast<double>(params.integrationTime_s));

    m_readNoise->setValue(static_cast<double>(params.readNoise_e_rms));
    m_darkCurrent->setValue(static_cast<double>(params.darkCurrent_e_s));
    m_poissonNoise->setChecked(params.enablePoissonNoise);
    m_fpnNoise->setChecked(params.enableFPN);

    blockSignalsForUpdate(false);
    m_updatingUi = false;
}

void SensorPanel::blockSignalsForUpdate(bool block) {
    m_focalLength->blockSignals(block);
    m_fNumber->blockSignals(block);
    m_pixelPitch->blockSignals(block);
    m_quantumEfficiency->blockSignals(block);
    m_wellCapacity->blockSignals(block);
    m_bitDepth->blockSignals(block);
    m_integrationTime->blockSignals(block);
    m_readNoise->blockSignals(block);
    m_darkCurrent->blockSignals(block);
    m_poissonNoise->blockSignals(block);
    m_fpnNoise->blockSignals(block);
}
