#include "SettingsDialog.hpp"
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QFormLayout>
#include <QLineEdit>
#include <QPushButton>
#include <QFileDialog>
#include <QDialogButtonBox>
#include <QGroupBox>
#include <QLabel>
#include <QDir>

#ifdef Q_OS_WIN
#include <QStandardPaths>
#endif

SettingsDialog::SettingsDialog(QWidget* parent)
    : QDialog(parent)
{
    setWindowTitle(tr("Settings"));
    setMinimumWidth(500);
    setupUi();
}

void SettingsDialog::setupUi() {
    auto* mainLayout = new QVBoxLayout(this);

    // Screenshot settings group
    auto* screenshotGroup = new QGroupBox(tr("Screenshot Settings"));
    auto* formLayout = new QFormLayout(screenshotGroup);

    // Path edit with browse button
    auto* pathLayout = new QHBoxLayout();
    m_pathEdit = new QLineEdit();
    m_pathEdit->setPlaceholderText(getDefaultScreenshotPath());

    auto* browseButton = new QPushButton(tr("Browse..."));
    connect(browseButton, &QPushButton::clicked, this, &SettingsDialog::onBrowse);

    pathLayout->addWidget(m_pathEdit);
    pathLayout->addWidget(browseButton);

    formLayout->addRow(tr("Save Location:"), pathLayout);

    // Info label
    auto* infoLabel = new QLabel(tr("Screenshots are saved as:\n"
                                    "• EXR format (HDR, full precision)\n"
                                    "• PNG format (8-bit, sRGB preview)\n"
                                    "Filename: YYYY-MM-DD_HH-MM-SS-mmm"));
    infoLabel->setStyleSheet("QLabel { color: gray; font-size: 10pt; }");
    formLayout->addRow(infoLabel);

    mainLayout->addWidget(screenshotGroup);

    // Restore defaults button
    auto* restoreButton = new QPushButton(tr("Restore Defaults"));
    connect(restoreButton, &QPushButton::clicked, this, &SettingsDialog::onRestoreDefaults);
    mainLayout->addWidget(restoreButton);

    // Dialog buttons
    auto* buttonBox = new QDialogButtonBox(
        QDialogButtonBox::Ok | QDialogButtonBox::Cancel
    );
    connect(buttonBox, &QDialogButtonBox::accepted, this, &QDialog::accept);
    connect(buttonBox, &QDialogButtonBox::rejected, this, &QDialog::reject);
    mainLayout->addWidget(buttonBox);
}

QString SettingsDialog::getDefaultScreenshotPath() const {
#ifdef Q_OS_WIN
    // Windows: %TEMP%/Quantiloom/screenshots
    QString tempDir = QStandardPaths::writableLocation(QStandardPaths::TempLocation);
    return QDir(tempDir).filePath("Quantiloom/screenshots");
#else
    // Linux: /tmp/Quantiloom/screenshots
    return "/tmp/Quantiloom/screenshots";
#endif
}

QString SettingsDialog::getScreenshotPath() const {
    QString path = m_pathEdit->text().trimmed();
    return path.isEmpty() ? getDefaultScreenshotPath() : path;
}

void SettingsDialog::setScreenshotPath(const QString& path) {
    m_pathEdit->setText(path);
}

void SettingsDialog::onBrowse() {
    QString currentPath = getScreenshotPath();
    QString dir = QFileDialog::getExistingDirectory(
        this,
        tr("Select Screenshot Save Location"),
        currentPath,
        QFileDialog::ShowDirsOnly | QFileDialog::DontResolveSymlinks
    );

    if (!dir.isEmpty()) {
        m_pathEdit->setText(dir);
    }
}

void SettingsDialog::onRestoreDefaults() {
    m_pathEdit->clear();  // Empty = use default
}
