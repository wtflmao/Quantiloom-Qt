/**
 * @file ConfigManager.hpp
 * @brief TOML configuration import/export using libQuantiloom's Config class
 */

#pragma once

#include <QString>
#include <QObject>

#include <core/Config.hpp>
#include <core/Types.hpp>
#include <renderer/LightingParams.hpp>

/**
 * @struct SceneConfig
 * @brief Extracted configuration values for UI panels
 */
struct SceneConfig {
    // [renderer]
    uint32_t width = 1280;
    uint32_t height = 720;
    uint32_t spp = 4;
    QString outputPath;
    QString environmentMap;

    // [spectral]
    quantiloom::SpectralMode spectralMode = quantiloom::SpectralMode::RGB;
    float wavelength_nm = 550.0f;
    float lambda_min = 380.0f;
    float lambda_max = 760.0f;
    float delta_lambda = 5.0f;

    // [scene]
    QString gltfPath;
    QString usdPath;
    float worldUnitsToMeters = 1.0f;

    // [camera]
    float cameraPosition[3] = {0.0f, 0.0f, 5.0f};
    float cameraLookAt[3] = {0.0f, 0.0f, 0.0f};
    float cameraUp[3] = {0.0f, 1.0f, 0.0f};
    float cameraFovY = 45.0f;

    // [lighting]
    quantiloom::LightingParams lighting;

    // Config file base directory (for resolving relative paths)
    QString baseDir;
};

/**
 * @class ConfigManager
 * @brief Manages TOML configuration import/export
 *
 * Uses libQuantiloom's Config class for parsing, extracts values for Qt panels.
 */
class ConfigManager : public QObject {
    Q_OBJECT

public:
    explicit ConfigManager(QObject* parent = nullptr);

    /**
     * @brief Load and parse TOML config file
     * @param filePath Path to TOML file
     * @param outConfig Output configuration struct
     * @return true if parsing succeeded
     */
    bool loadConfig(const QString& filePath, SceneConfig& outConfig);

    /**
     * @brief Get the raw Config object (for passing to ExternalRenderContext)
     * @return Pointer to loaded Config, or nullptr if not loaded
     */
    const quantiloom::Config* getRawConfig() const;

    /**
     * @brief Export configuration to TOML file
     * @param filePath Output file path
     * @param config Configuration to export
     * @return true if export succeeded
     */
    bool exportConfig(const QString& filePath, const SceneConfig& config);

    /**
     * @brief Get last error message
     */
    QString lastError() const { return m_lastError; }

private:
    void extractSceneConfig(const quantiloom::Config& config, SceneConfig& out);
    quantiloom::SpectralMode parseSpectralMode(const std::string& modeStr);

    QString m_lastError;
    std::unique_ptr<quantiloom::Config> m_loadedConfig;
};
