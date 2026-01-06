/**
 * @file RenderSettingsPanel.cpp
 * @brief Render settings panel implementation
 */

#include "RenderSettingsPanel.hpp"

#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QGroupBox>
#include <QLabel>
#include <QSpinBox>
#include <QComboBox>
#include <QPushButton>
#include <QCheckBox>
#include <QFormLayout>
#include <QFileDialog>

RenderSettingsPanel::RenderSettingsPanel(QWidget* parent)
    : QWidget(parent)
{
    setupUi();
}

void RenderSettingsPanel::setupUi() {
    auto* mainLayout = new QVBoxLayout(this);
    mainLayout->setContentsMargins(4, 4, 4, 4);
    mainLayout->setSpacing(8);

    // Status group
    auto* statusGroup = new QGroupBox(tr("Status"));
    auto* statusLayout = new QFormLayout(statusGroup);

    m_sampleCountLabel = new QLabel("0");
    m_sampleCountLabel->setStyleSheet("font-weight: bold; font-size: 14pt;");
    statusLayout->addRow(tr("Accumulated Samples:"), m_sampleCountLabel);

    mainLayout->addWidget(statusGroup);

    // Quality group
    auto* qualityGroup = new QGroupBox(tr("Quality"));
    auto* qualityLayout = new QFormLayout(qualityGroup);

    // SPP preset
    m_sppPreset = new QComboBox();
    m_sppPreset->addItem(tr("Preview (1 SPP)"), 1);
    m_sppPreset->addItem(tr("Fast (4 SPP)"), 4);
    m_sppPreset->addItem(tr("Medium (16 SPP)"), 16);
    m_sppPreset->addItem(tr("High (64 SPP)"), 64);
    m_sppPreset->addItem(tr("Ultra (256 SPP)"), 256);
    m_sppPreset->addItem(tr("Production (1024 SPP)"), 1024);
    m_sppPreset->addItem(tr("Custom..."), -1);
    m_sppPreset->setCurrentIndex(1);  // Default: Fast
    connect(m_sppPreset, QOverload<int>::of(&QComboBox::currentIndexChanged),
            this, &RenderSettingsPanel::onSppPresetChanged);
    qualityLayout->addRow(tr("Target SPP:"), m_sppPreset);

    // Custom SPP
    m_customSpp = new QSpinBox();
    m_customSpp->setRange(1, 65536);
    m_customSpp->setValue(4);
    m_customSpp->setEnabled(false);
    connect(m_customSpp, QOverload<int>::of(&QSpinBox::valueChanged),
            this, &RenderSettingsPanel::onCustomSppChanged);
    qualityLayout->addRow(tr("Custom SPP:"), m_customSpp);

    // Progressive rendering
    m_progressiveCheck = new QCheckBox(tr("Progressive Rendering"));
    m_progressiveCheck->setChecked(true);
    m_progressiveCheck->setToolTip(tr("Accumulate samples over multiple frames"));
    qualityLayout->addRow(m_progressiveCheck);

    mainLayout->addWidget(qualityGroup);

    // Resolution group
    auto* resGroup = new QGroupBox(tr("Resolution"));
    auto* resLayout = new QFormLayout(resGroup);

    m_resolutionPreset = new QComboBox();
    m_resolutionPreset->addItem(tr("720p (1280x720)"), QSize(1280, 720));
    m_resolutionPreset->addItem(tr("1080p (1920x1080)"), QSize(1920, 1080));
    m_resolutionPreset->addItem(tr("1440p (2560x1440)"), QSize(2560, 1440));
    m_resolutionPreset->addItem(tr("4K (3840x2160)"), QSize(3840, 2160));
    m_resolutionPreset->addItem(tr("Window Size"), QSize(0, 0));
    m_resolutionPreset->setCurrentIndex(4);  // Default: Window Size
    connect(m_resolutionPreset, QOverload<int>::of(&QComboBox::currentIndexChanged),
            this, &RenderSettingsPanel::onResolutionPresetChanged);
    resLayout->addRow(tr("Preset:"), m_resolutionPreset);

    m_resolutionLabel = new QLabel("1280 x 720");
    resLayout->addRow(tr("Current:"), m_resolutionLabel);

    mainLayout->addWidget(resGroup);

    // Actions group
    auto* actionsGroup = new QGroupBox(tr("Actions"));
    auto* actionsLayout = new QVBoxLayout(actionsGroup);

    m_resetBtn = new QPushButton(tr("Reset Accumulation"));
    m_resetBtn->setToolTip(tr("Clear accumulated samples and restart rendering"));
    connect(m_resetBtn, &QPushButton::clicked, this, &RenderSettingsPanel::onResetClicked);
    actionsLayout->addWidget(m_resetBtn);

    m_exportBtn = new QPushButton(tr("Export Image..."));
    m_exportBtn->setToolTip(tr("Save current render to file"));
    connect(m_exportBtn, &QPushButton::clicked, this, &RenderSettingsPanel::onExportClicked);
    actionsLayout->addWidget(m_exportBtn);

    mainLayout->addWidget(actionsGroup);

    mainLayout->addStretch();
}

void RenderSettingsPanel::setSampleCount(uint32_t count) {
    m_sampleCountLabel->setText(QString::number(count));
}

void RenderSettingsPanel::setTargetSPP(uint32_t spp) {
    m_targetSPP = spp;

    // Find matching preset
    for (int i = 0; i < m_sppPreset->count() - 1; ++i) {
        if (m_sppPreset->itemData(i).toUInt() == spp) {
            m_sppPreset->blockSignals(true);
            m_sppPreset->setCurrentIndex(i);
            m_sppPreset->blockSignals(false);
            m_customSpp->setEnabled(false);
            return;
        }
    }

    // Custom value
    m_sppPreset->blockSignals(true);
    m_sppPreset->setCurrentIndex(m_sppPreset->count() - 1);
    m_sppPreset->blockSignals(false);
    m_customSpp->setEnabled(true);
    m_customSpp->blockSignals(true);
    m_customSpp->setValue(static_cast<int>(spp));
    m_customSpp->blockSignals(false);
}

void RenderSettingsPanel::setResolution(uint32_t width, uint32_t height) {
    m_width = width;
    m_height = height;
    m_resolutionLabel->setText(QString("%1 x %2").arg(width).arg(height));
}

void RenderSettingsPanel::onSppPresetChanged(int index) {
    int spp = m_sppPreset->itemData(index).toInt();

    if (spp < 0) {
        // Custom
        m_customSpp->setEnabled(true);
        spp = m_customSpp->value();
    } else {
        m_customSpp->setEnabled(false);
        m_customSpp->setValue(spp);
    }

    m_targetSPP = static_cast<uint32_t>(spp);
    emit sppChanged(m_targetSPP);
}

void RenderSettingsPanel::onCustomSppChanged(int value) {
    if (m_customSpp->isEnabled()) {
        m_targetSPP = static_cast<uint32_t>(value);
        emit sppChanged(m_targetSPP);
    }
}

void RenderSettingsPanel::onResolutionPresetChanged(int index) {
    QSize size = m_resolutionPreset->itemData(index).toSize();

    if (size.width() > 0 && size.height() > 0) {
        m_width = static_cast<uint32_t>(size.width());
        m_height = static_cast<uint32_t>(size.height());
        m_resolutionLabel->setText(QString("%1 x %2").arg(m_width).arg(m_height));
        emit resolutionChanged(m_width, m_height);
    }
    // Window Size option (0,0) - no emit, use actual window size
}

void RenderSettingsPanel::onExportClicked() {
    QString fileName = QFileDialog::getSaveFileName(
        this,
        tr("Export Image"),
        QString(),
        tr("EXR Image (*.exr);;PNG Image (*.png);;All Files (*)")
    );

    if (!fileName.isEmpty()) {
        QString format = "exr";
        if (fileName.endsWith(".png", Qt::CaseInsensitive)) {
            format = "png";
        }
        emit exportRequested(format);
    }
}

void RenderSettingsPanel::onResetClicked() {
    emit resetAccumulationRequested();
}
