#pragma once

#include <QDialog>

QT_BEGIN_NAMESPACE
class QLineEdit;
class QPushButton;
QT_END_NAMESPACE

class SettingsDialog : public QDialog {
    Q_OBJECT

public:
    explicit SettingsDialog(QWidget* parent = nullptr);

    QString getScreenshotPath() const;
    void setScreenshotPath(const QString& path);

private slots:
    void onBrowse();
    void onRestoreDefaults();

private:
    void setupUi();
    QString getDefaultScreenshotPath() const;

    QLineEdit* m_pathEdit = nullptr;
};
