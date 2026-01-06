/**
 * @file ConfigManager.cpp
 * @brief TOML configuration import/export implementation
 */

#include "ConfigManager.hpp"

#include <QFile>
#include <QFileInfo>
#include <QTextStream>
#include <QDebug>

ConfigManager::ConfigManager(QObject* parent)
    : QObject(parent)
{
}

bool ConfigManager::loadConfig(const QString& filePath, SceneConfig& outConfig) {
    // Use libQuantiloom's Config::Load
    auto result = quantiloom::Config::Load(filePath.toStdString());

    if (!result.has_value()) {
        m_lastError = QString::fromStdString(result.error());
        return false;
    }

    // Store the loaded config for later use
    m_loadedConfig = std::make_unique<quantiloom::Config>(std::move(result.value()));

    // Extract values for UI panels
    extractSceneConfig(*m_loadedConfig, outConfig);

    // Store base directory for resolving relative paths
    QFileInfo fileInfo(filePath);
    outConfig.baseDir = fileInfo.absolutePath();

    return true;
}

const quantiloom::Config* ConfigManager::getRawConfig() const {
    return m_loadedConfig.get();
}

void ConfigManager::extractSceneConfig(const quantiloom::Config& config, SceneConfig& out) {
    // Initialize with defaults
    out.lighting = quantiloom::CreateDefaultLightingParams();

    // [renderer]
    auto resolution = config.GetArray<quantiloom::i32>("renderer.resolution");
    if (resolution.size() >= 2) {
        out.width = static_cast<uint32_t>(resolution[0]);
        out.height = static_cast<uint32_t>(resolution[1]);
    }
    out.spp = config.Get<quantiloom::u32>("renderer.spp", 4);
    out.outputPath = QString::fromStdString(config.GetString("renderer.output", "output.exr"));
    out.environmentMap = QString::fromStdString(config.GetString("renderer.environment_map", ""));

    // [spectral]
    std::string modeStr = config.GetString("spectral.mode", "rgb_fused");
    out.spectralMode = parseSpectralMode(modeStr);
    out.wavelength_nm = config.GetFloat("spectral.wavelength_nm", 550.0f);
    out.lambda_min = config.GetFloat("spectral.lambda_min", 380.0f);
    out.lambda_max = config.GetFloat("spectral.lambda_max", 760.0f);
    out.delta_lambda = config.GetFloat("spectral.delta_lambda", 5.0f);

    // [scene]
    out.gltfPath = QString::fromStdString(config.GetString("scene.gltf", ""));
    out.worldUnitsToMeters = config.GetFloat("scene.world_units_to_meters", 1.0f);

    // [camera]
    auto camPos = config.GetArray<quantiloom::f32>("camera.position");
    if (camPos.size() >= 3) {
        out.cameraPosition[0] = camPos[0];
        out.cameraPosition[1] = camPos[1];
        out.cameraPosition[2] = camPos[2];
    }

    auto camLookAt = config.GetArray<quantiloom::f32>("camera.look_at");
    if (camLookAt.size() >= 3) {
        out.cameraLookAt[0] = camLookAt[0];
        out.cameraLookAt[1] = camLookAt[1];
        out.cameraLookAt[2] = camLookAt[2];
    }

    auto camUp = config.GetArray<quantiloom::f32>("camera.up");
    if (camUp.size() >= 3) {
        out.cameraUp[0] = camUp[0];
        out.cameraUp[1] = camUp[1];
        out.cameraUp[2] = camUp[2];
    }

    out.cameraFovY = config.GetFloat("camera.fov_y", 45.0f);

    // [lighting]
    auto sunDir = config.GetArray<quantiloom::f32>("lighting.sun_direction");
    if (sunDir.size() >= 3) {
        out.lighting.sunDirection = glm::normalize(glm::vec3(sunDir[0], sunDir[1], sunDir[2]));
    }

    auto sunRad = config.GetArray<quantiloom::f32>("lighting.sun_radiance");
    if (sunRad.size() >= 3) {
        out.lighting.sunRadiance_rgb = glm::vec3(sunRad[0], sunRad[1], sunRad[2]);
        out.lighting.sunRadiance_spectral = (sunRad[0] + sunRad[1] + sunRad[2]) / 3.0f;
    }

    auto skyRad = config.GetArray<quantiloom::f32>("lighting.sky_radiance");
    if (skyRad.size() >= 3) {
        out.lighting.skyRadiance_rgb = glm::vec3(skyRad[0], skyRad[1], skyRad[2]);
        out.lighting.skyRadiance_spectral = (skyRad[0] + skyRad[1] + skyRad[2]) / 3.0f;
    }

    out.lighting.atmosphereTemperature_K = config.GetFloat("lighting.atmosphere_temperature_k", 260.0f);
    out.lighting.transmittance = config.GetFloat("lighting.transmittance", 0.9f);
    out.lighting.worldUnitsToMeters = out.worldUnitsToMeters;
}

quantiloom::SpectralMode ConfigManager::parseSpectralMode(const std::string& modeStr) {
    if (modeStr == "single") {
        return quantiloom::SpectralMode::Single;
    } else if (modeStr == "mwir_fused") {
        return quantiloom::SpectralMode::MWIR_Fused;
    } else if (modeStr == "lwir_fused") {
        return quantiloom::SpectralMode::LWIR_Fused;
    } else if (modeStr == "swir_fused") {
        return quantiloom::SpectralMode::SWIR_Fused;
    }
    // Default to RGB_Fused
    return quantiloom::SpectralMode::RGB_Fused;
}

bool ConfigManager::exportConfig(const QString& filePath, const SceneConfig& config) {
    QFile file(filePath);
    if (!file.open(QIODevice::WriteOnly | QIODevice::Text)) {
        m_lastError = tr("Cannot open file for writing: %1").arg(filePath);
        return false;
    }

    QTextStream out(&file);
    out.setEncoding(QStringConverter::Utf8);

    // Header
    out << "# ============================================================================\n";
    out << "# Quantiloom Scene Configuration\n";
    out << "# Exported from Quantiloom Qt GUI\n";
    out << "# ============================================================================\n\n";

    // [renderer]
    out << "[renderer]\n";
    out << "resolution = [" << config.width << ", " << config.height << "]\n";
    out << "spp = " << config.spp << "\n";
    if (!config.outputPath.isEmpty()) {
        out << "output = \"" << config.outputPath << "\"\n";
    }
    if (!config.environmentMap.isEmpty()) {
        out << "environment_map = \"" << config.environmentMap << "\"\n";
    }
    out << "\n";

    // [spectral]
    out << "[spectral]\n";
    QString modeStr;
    switch (config.spectralMode) {
        case quantiloom::SpectralMode::Single: modeStr = "single"; break;
        case quantiloom::SpectralMode::MWIR_Fused: modeStr = "mwir_fused"; break;
        case quantiloom::SpectralMode::LWIR_Fused: modeStr = "lwir_fused"; break;
        case quantiloom::SpectralMode::SWIR_Fused: modeStr = "swir_fused"; break;
        default: modeStr = "rgb_fused"; break;
    }
    out << "mode = \"" << modeStr << "\"\n";
    if (config.spectralMode == quantiloom::SpectralMode::Single) {
        out << "wavelength_nm = " << config.wavelength_nm << "\n";
    }
    out << "\n";

    // [scene]
    out << "[scene]\n";
    if (!config.gltfPath.isEmpty()) {
        out << "gltf = \"" << config.gltfPath << "\"\n";
    }
    out << "world_units_to_meters = " << config.worldUnitsToMeters << "\n";
    out << "\n";

    // [camera]
    out << "[camera]\n";
    out << "position = [" << config.cameraPosition[0] << ", "
                          << config.cameraPosition[1] << ", "
                          << config.cameraPosition[2] << "]\n";
    out << "look_at = [" << config.cameraLookAt[0] << ", "
                         << config.cameraLookAt[1] << ", "
                         << config.cameraLookAt[2] << "]\n";
    out << "up = [" << config.cameraUp[0] << ", "
                    << config.cameraUp[1] << ", "
                    << config.cameraUp[2] << "]\n";
    out << "fov_y = " << config.cameraFovY << "\n";
    out << "\n";

    // [lighting]
    out << "[lighting]\n";
    out << "sun_direction = [" << config.lighting.sunDirection.x << ", "
                               << config.lighting.sunDirection.y << ", "
                               << config.lighting.sunDirection.z << "]\n";
    out << "sun_radiance = [" << config.lighting.sunRadiance_rgb.r << ", "
                              << config.lighting.sunRadiance_rgb.g << ", "
                              << config.lighting.sunRadiance_rgb.b << "]\n";
    out << "sky_radiance = [" << config.lighting.skyRadiance_rgb.r << ", "
                              << config.lighting.skyRadiance_rgb.g << ", "
                              << config.lighting.skyRadiance_rgb.b << "]\n";
    out << "atmosphere_temperature_k = " << config.lighting.atmosphereTemperature_K << "\n";
    out << "\n";

    file.close();
    return true;
}
