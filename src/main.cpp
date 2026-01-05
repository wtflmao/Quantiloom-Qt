/**
 * @file main.cpp
 * @brief Entry point for QuantiloomGUI application
 *
 * Initializes Qt6 application with Vulkan support and launches main window.
 *
 * @author wtflmao
 */

#include "MainWindow.hpp"

#include <QApplication>
#include <QVulkanInstance>
#include <QLoggingCategory>
#include <QTranslator>
#include <QLocale>

int main(int argc, char* argv[]) {
    // Enable high DPI scaling
    QApplication::setHighDpiScaleFactorRoundingPolicy(
        Qt::HighDpiScaleFactorRoundingPolicy::PassThrough);

    QApplication app(argc, argv);

    // Set application metadata
    app.setApplicationName("Quantiloom");
    app.setApplicationVersion("1.0.0");
    app.setOrganizationName("wtflmao");
    app.setOrganizationDomain("github.com/wtflmao");

    // Load translations based on system locale
    QTranslator translator;
    const QStringList uiLanguages = QLocale::system().uiLanguages();
    for (const QString& locale : uiLanguages) {
        const QString baseName = "quantiloom_" + QLocale(locale).name();
        if (translator.load(":/i18n/" + baseName)) {
            app.installTranslator(&translator);
            break;
        }
    }

    // Create Vulkan instance for Qt
    QVulkanInstance vulkanInstance;

    // Set Vulkan API version (1.3 required for ray tracing)
    vulkanInstance.setApiVersion(QVersionNumber(1, 3, 0));

    // Enable validation layers in debug builds
#ifdef QT_DEBUG
    vulkanInstance.setLayers({"VK_LAYER_KHRONOS_validation"});
    QLoggingCategory::setFilterRules("qt.vulkan=true");
#endif

    // Required extensions for ray tracing
    vulkanInstance.setExtensions({
        VK_KHR_GET_PHYSICAL_DEVICE_PROPERTIES_2_EXTENSION_NAME
    });

    if (!vulkanInstance.create()) {
        qFatal("Failed to create Vulkan instance: %d", vulkanInstance.errorCode());
        return 1;
    }

    // Create and show main window
    MainWindow mainWindow(&vulkanInstance);
    mainWindow.show();

    return app.exec();
}
