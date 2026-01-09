/**
 * @file SpectralConfigPanel.hpp
 * @brief Spectral rendering configuration panel
 */

#pragma once

#include <QWidget>
#include <core/Types.hpp>

QT_BEGIN_NAMESPACE
class QComboBox;
class QDoubleSpinBox;
class QSlider;
class QLabel;
class QGroupBox;
class QStackedWidget;
QT_END_NAMESPACE

/**
 * @class SpectralConfigPanel
 * @brief Editor for spectral rendering mode and wavelength settings
 */
class SpectralConfigPanel : public QWidget {
    Q_OBJECT

public:
    explicit SpectralConfigPanel(QWidget* parent = nullptr);

    void setSpectralMode(quantiloom::SpectralMode mode);
    void setWavelength(float wavelength_nm);
    void setWavelengthRange(float min_nm, float max_nm, float delta_nm);

signals:
    void spectralModeChanged(quantiloom::SpectralMode mode);
    void wavelengthChanged(float wavelength_nm);
    void wavelengthRangeChanged(float min_nm, float max_nm, float delta_nm);

private slots:
    void onModeChanged(int index);
    void onWavelengthSliderChanged(int value);
    void onWavelengthSpinChanged(double value);
    void onRangeChanged();

private:
    void setupUi();
    void updateModeDescription(quantiloom::SpectralMode mode);

    // Current settings
    quantiloom::SpectralMode m_mode = quantiloom::SpectralMode::RGB;
    float m_wavelength = 550.0f;
    float m_lambdaMin = 380.0f;
    float m_lambdaMax = 760.0f;
    float m_deltaLambda = 5.0f;

    // UI elements
    QComboBox* m_modeCombo = nullptr;
    QLabel* m_modeDescription = nullptr;
    QStackedWidget* m_settingsStack = nullptr;

    // Single wavelength controls
    QSlider* m_wavelengthSlider = nullptr;
    QDoubleSpinBox* m_wavelengthSpin = nullptr;
    QLabel* m_wavelengthColorPreview = nullptr;

    // Range controls (for hyperspectral)
    QDoubleSpinBox* m_lambdaMinSpin = nullptr;
    QDoubleSpinBox* m_lambdaMaxSpin = nullptr;
    QDoubleSpinBox* m_deltaSpin = nullptr;
    QLabel* m_bandCountLabel = nullptr;
};
