/**
 * @file DebugVisualizationPanel.cpp
 * @brief Debug visualization mode selection panel implementation
 */

#include "DebugVisualizationPanel.hpp"

#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QGroupBox>
#include <QLabel>
#include <QComboBox>
#include <QFormLayout>

DebugVisualizationPanel::DebugVisualizationPanel(QWidget* parent)
    : QWidget(parent)
{
    setupUi();
}

void DebugVisualizationPanel::setupUi() {
    auto* mainLayout = new QVBoxLayout(this);
    mainLayout->setContentsMargins(4, 4, 4, 4);
    mainLayout->setSpacing(8);

    // Mode selection group
    auto* modeGroup = new QGroupBox(tr("Debug Mode"));
    auto* modeLayout = new QVBoxLayout(modeGroup);

    m_modeCombo = new QComboBox();

    // Normal rendering
    m_modeCombo->addItem(tr("None (Normal Rendering)"),
                         static_cast<int>(quantiloom::DebugVisualizationMode::None));

    // Separator and Geometry group
    m_modeCombo->insertSeparator(m_modeCombo->count());
    m_modeCombo->addItem(tr("-- Geometry --"), -1);
    m_modeCombo->addItem(tr("World Position"),
                         static_cast<int>(quantiloom::DebugVisualizationMode::WorldPosition));
    m_modeCombo->addItem(tr("Geometric Normal"),
                         static_cast<int>(quantiloom::DebugVisualizationMode::GeometricNormal));
    m_modeCombo->addItem(tr("Shaded Normal"),
                         static_cast<int>(quantiloom::DebugVisualizationMode::ShadedNormal));
    m_modeCombo->addItem(tr("Tangent"),
                         static_cast<int>(quantiloom::DebugVisualizationMode::Tangent));
    m_modeCombo->addItem(tr("UV Coordinates"),
                         static_cast<int>(quantiloom::DebugVisualizationMode::UV));
    m_modeCombo->addItem(tr("Material ID"),
                         static_cast<int>(quantiloom::DebugVisualizationMode::MaterialID));
    m_modeCombo->addItem(tr("Triangle ID"),
                         static_cast<int>(quantiloom::DebugVisualizationMode::TriangleID));
    m_modeCombo->addItem(tr("Barycentric Coords"),
                         static_cast<int>(quantiloom::DebugVisualizationMode::Barycentric));

    // Separator and Material group
    m_modeCombo->insertSeparator(m_modeCombo->count());
    m_modeCombo->addItem(tr("-- Material --"), -1);
    m_modeCombo->addItem(tr("Base Color (Albedo)"),
                         static_cast<int>(quantiloom::DebugVisualizationMode::BaseColor));
    m_modeCombo->addItem(tr("Metallic"),
                         static_cast<int>(quantiloom::DebugVisualizationMode::Metallic));
    m_modeCombo->addItem(tr("Roughness"),
                         static_cast<int>(quantiloom::DebugVisualizationMode::Roughness));
    m_modeCombo->addItem(tr("Normal Map Delta"),
                         static_cast<int>(quantiloom::DebugVisualizationMode::NormalMapDelta));
    m_modeCombo->addItem(tr("Emissive"),
                         static_cast<int>(quantiloom::DebugVisualizationMode::Emissive));
    m_modeCombo->addItem(tr("Alpha"),
                         static_cast<int>(quantiloom::DebugVisualizationMode::Alpha));

    // Separator and Lighting group
    m_modeCombo->insertSeparator(m_modeCombo->count());
    m_modeCombo->addItem(tr("-- Lighting --"), -1);
    m_modeCombo->addItem(tr("N dot L"),
                         static_cast<int>(quantiloom::DebugVisualizationMode::NdotL));
    m_modeCombo->addItem(tr("N dot V"),
                         static_cast<int>(quantiloom::DebugVisualizationMode::NdotV));
    m_modeCombo->addItem(tr("Direct Sun"),
                         static_cast<int>(quantiloom::DebugVisualizationMode::DirectSun));
    m_modeCombo->addItem(tr("Diffuse"),
                         static_cast<int>(quantiloom::DebugVisualizationMode::Diffuse));
    m_modeCombo->addItem(tr("Atmospheric Transmittance"),
                         static_cast<int>(quantiloom::DebugVisualizationMode::AtmosphericTransmittance));

    // Separator and BRDF group
    m_modeCombo->insertSeparator(m_modeCombo->count());
    m_modeCombo->addItem(tr("-- BRDF --"), -1);
    m_modeCombo->addItem(tr("Fresnel F0"),
                         static_cast<int>(quantiloom::DebugVisualizationMode::FresnelF0));
    m_modeCombo->addItem(tr("Fresnel"),
                         static_cast<int>(quantiloom::DebugVisualizationMode::Fresnel));
    m_modeCombo->addItem(tr("Full BRDF"),
                         static_cast<int>(quantiloom::DebugVisualizationMode::BRDF_Full));
    m_modeCombo->addItem(tr("Specular D (GGX)"),
                         static_cast<int>(quantiloom::DebugVisualizationMode::SpecularD));
    m_modeCombo->addItem(tr("Specular G (Smith)"),
                         static_cast<int>(quantiloom::DebugVisualizationMode::SpecularG));

    // Separator and IBL group
    m_modeCombo->insertSeparator(m_modeCombo->count());
    m_modeCombo->addItem(tr("-- IBL --"), -1);
    m_modeCombo->addItem(tr("Reflection Direction"),
                         static_cast<int>(quantiloom::DebugVisualizationMode::ReflectionDir));
    m_modeCombo->addItem(tr("Prefiltered Environment"),
                         static_cast<int>(quantiloom::DebugVisualizationMode::PrefilteredEnv));
    m_modeCombo->addItem(tr("BRDF LUT"),
                         static_cast<int>(quantiloom::DebugVisualizationMode::BrdfLut));
    m_modeCombo->addItem(tr("IBL Specular"),
                         static_cast<int>(quantiloom::DebugVisualizationMode::IblSpecular));
    m_modeCombo->addItem(tr("Sky Ambient"),
                         static_cast<int>(quantiloom::DebugVisualizationMode::SkyAmbient));

    // Separator and Spectral group
    m_modeCombo->insertSeparator(m_modeCombo->count());
    m_modeCombo->addItem(tr("-- Spectral --"), -1);
    m_modeCombo->addItem(tr("XYZ Tristimulus"),
                         static_cast<int>(quantiloom::DebugVisualizationMode::XYZ_Tristimulus));
    m_modeCombo->addItem(tr("Before Chroma Correction"),
                         static_cast<int>(quantiloom::DebugVisualizationMode::BeforeChromaCorrection));
    m_modeCombo->addItem(tr("Spectral Reflectance @550nm"),
                         static_cast<int>(quantiloom::DebugVisualizationMode::SpectralReflectance550));

    // Separator and IR group
    m_modeCombo->insertSeparator(m_modeCombo->count());
    m_modeCombo->addItem(tr("-- Infrared --"), -1);
    m_modeCombo->addItem(tr("Temperature"),
                         static_cast<int>(quantiloom::DebugVisualizationMode::Temperature));
    m_modeCombo->addItem(tr("IR Emissivity"),
                         static_cast<int>(quantiloom::DebugVisualizationMode::IREmissivity));
    m_modeCombo->addItem(tr("IR Emission"),
                         static_cast<int>(quantiloom::DebugVisualizationMode::IREmission));
    m_modeCombo->addItem(tr("IR Reflection"),
                         static_cast<int>(quantiloom::DebugVisualizationMode::IRReflection));

    connect(m_modeCombo, QOverload<int>::of(&QComboBox::currentIndexChanged),
            this, &DebugVisualizationPanel::onModeChanged);
    modeLayout->addWidget(m_modeCombo);

    // Category label
    m_categoryLabel = new QLabel();
    m_categoryLabel->setStyleSheet(
        "QLabel { background-color: #3a5a7a; color: white; padding: 4px 8px; "
        "border-radius: 4px; font-weight: bold; }");
    modeLayout->addWidget(m_categoryLabel);

    // Description label
    m_description = new QLabel();
    m_description->setWordWrap(true);
    m_description->setStyleSheet("color: gray; font-size: 10pt;");
    modeLayout->addWidget(m_description);

    mainLayout->addWidget(modeGroup);

    // Info group
    auto* infoGroup = new QGroupBox(tr("Output Interpretation"));
    auto* infoLayout = new QVBoxLayout(infoGroup);

    auto* infoText = new QLabel(tr(
        "<b>Color Encoding:</b><br>"
        "- Vectors: (V+1)/2 maps [-1,1] to [0,1] RGB<br>"
        "- Scalars: Grayscale intensity<br>"
        "- IDs: Hash to distinct colors<br>"
        "- Temperature: Blue (cold) to Red (hot)<br>"
        "<br>"
        "<b>Tips:</b><br>"
        "- Use 'Shaded Normal' to check normal mapping<br>"
        "- Use 'Material ID' to verify material assignment<br>"
        "- Use 'XYZ Tristimulus' to debug spectral integration"
    ));
    infoText->setWordWrap(true);
    infoText->setStyleSheet("font-size: 9pt;");
    infoLayout->addWidget(infoText);

    mainLayout->addWidget(infoGroup);

    mainLayout->addStretch();

    // Initialize with default mode
    updateDescription(quantiloom::DebugVisualizationMode::None);
}

void DebugVisualizationPanel::setDebugMode(quantiloom::DebugVisualizationMode mode) {
    m_mode = mode;

    // Find and select matching combo item
    for (int i = 0; i < m_modeCombo->count(); ++i) {
        if (m_modeCombo->itemData(i).toInt() == static_cast<int>(mode)) {
            m_modeCombo->blockSignals(true);
            m_modeCombo->setCurrentIndex(i);
            m_modeCombo->blockSignals(false);
            break;
        }
    }

    updateDescription(mode);
}

void DebugVisualizationPanel::onModeChanged(int index) {
    int modeValue = m_modeCombo->itemData(index).toInt();

    // Skip category headers (data == -1)
    if (modeValue < 0) {
        // Move to next valid item
        if (index + 1 < m_modeCombo->count()) {
            m_modeCombo->setCurrentIndex(index + 1);
        }
        return;
    }

    m_mode = static_cast<quantiloom::DebugVisualizationMode>(modeValue);
    updateDescription(m_mode);
    emit debugModeChanged(m_mode);
}

void DebugVisualizationPanel::updateDescription(quantiloom::DebugVisualizationMode mode) {
    QString category;
    QString desc;

    switch (mode) {
        // None
        case quantiloom::DebugVisualizationMode::None:
            category = tr("Normal");
            desc = tr("Standard rendering output. No debug visualization.");
            break;

        // Geometry (1-9)
        case quantiloom::DebugVisualizationMode::WorldPosition:
            category = tr("Geometry");
            desc = tr("World-space hit position. RGB = fractional XYZ coordinates.");
            break;
        case quantiloom::DebugVisualizationMode::GeometricNormal:
            category = tr("Geometry");
            desc = tr("Raw geometric normal from triangle vertices (before normal mapping).");
            break;
        case quantiloom::DebugVisualizationMode::ShadedNormal:
            category = tr("Geometry");
            desc = tr("Final shading normal after interpolation and normal map application.");
            break;
        case quantiloom::DebugVisualizationMode::Tangent:
            category = tr("Geometry");
            desc = tr("Tangent vector for normal mapping. Used for TBN matrix construction.");
            break;
        case quantiloom::DebugVisualizationMode::UV:
            category = tr("Geometry");
            desc = tr("Texture coordinates. RG = fractional UV, useful for texture mapping debug.");
            break;
        case quantiloom::DebugVisualizationMode::MaterialID:
            category = tr("Geometry");
            desc = tr("Material index visualized as distinct colors. Each material gets unique color.");
            break;
        case quantiloom::DebugVisualizationMode::TriangleID:
            category = tr("Geometry");
            desc = tr("Primitive (triangle) index. Useful for mesh topology inspection.");
            break;
        case quantiloom::DebugVisualizationMode::Barycentric:
            category = tr("Geometry");
            desc = tr("Barycentric coordinates within triangle. RGB = weights at 3 vertices.");
            break;

        // Material (10-19)
        case quantiloom::DebugVisualizationMode::BaseColor:
            category = tr("Material");
            desc = tr("Albedo/base color from texture or material parameters.");
            break;
        case quantiloom::DebugVisualizationMode::Metallic:
            category = tr("Material");
            desc = tr("Metallic parameter. 0 = dielectric, 1 = metal.");
            break;
        case quantiloom::DebugVisualizationMode::Roughness:
            category = tr("Material");
            desc = tr("Roughness parameter. 0 = mirror smooth, 1 = fully rough.");
            break;
        case quantiloom::DebugVisualizationMode::NormalMapDelta:
            category = tr("Material");
            desc = tr("Normal map perturbation from surface normal.");
            break;
        case quantiloom::DebugVisualizationMode::Emissive:
            category = tr("Material");
            desc = tr("Emissive color/intensity. Self-illumination without external lighting.");
            break;
        case quantiloom::DebugVisualizationMode::Alpha:
            category = tr("Material");
            desc = tr("Alpha/opacity value. 1 = opaque, 0 = transparent.");
            break;

        // Lighting (20-29)
        case quantiloom::DebugVisualizationMode::NdotL:
            category = tr("Lighting");
            desc = tr("Dot product of normal and light direction. Basic diffuse term.");
            break;
        case quantiloom::DebugVisualizationMode::NdotV:
            category = tr("Lighting");
            desc = tr("Dot product of normal and view direction. Affects Fresnel and specular.");
            break;
        case quantiloom::DebugVisualizationMode::DirectSun:
            category = tr("Lighting");
            desc = tr("Direct sunlight contribution after shadowing and attenuation.");
            break;
        case quantiloom::DebugVisualizationMode::Diffuse:
            category = tr("Lighting");
            desc = tr("Diffuse lighting term: kD * albedo * NdotL.");
            break;
        case quantiloom::DebugVisualizationMode::AtmosphericTransmittance:
            category = tr("Lighting");
            desc = tr("Atmospheric transmittance factor from scattering/absorption LUT.");
            break;

        // BRDF (30-39)
        case quantiloom::DebugVisualizationMode::FresnelF0:
            category = tr("BRDF");
            desc = tr("Base reflectivity at normal incidence. Depends on metallic and IOR.");
            break;
        case quantiloom::DebugVisualizationMode::Fresnel:
            category = tr("BRDF");
            desc = tr("Fresnel reflectance at current viewing angle (Schlick approximation).");
            break;
        case quantiloom::DebugVisualizationMode::BRDF_Full:
            category = tr("BRDF");
            desc = tr("Complete Cook-Torrance BRDF evaluation: D * G * F / (4 * NdotL * NdotV).");
            break;
        case quantiloom::DebugVisualizationMode::SpecularD:
            category = tr("BRDF");
            desc = tr("GGX/Trowbridge-Reitz normal distribution function.");
            break;
        case quantiloom::DebugVisualizationMode::SpecularG:
            category = tr("BRDF");
            desc = tr("Smith geometry/masking-shadowing function.");
            break;

        // IBL (40-49)
        case quantiloom::DebugVisualizationMode::ReflectionDir:
            category = tr("IBL");
            desc = tr("Mirror reflection direction for environment map sampling.");
            break;
        case quantiloom::DebugVisualizationMode::PrefilteredEnv:
            category = tr("IBL");
            desc = tr("Pre-filtered environment map sample at current roughness level.");
            break;
        case quantiloom::DebugVisualizationMode::BrdfLut:
            category = tr("IBL");
            desc = tr("BRDF integration LUT sample. RG = scale and bias for split-sum.");
            break;
        case quantiloom::DebugVisualizationMode::IblSpecular:
            category = tr("IBL");
            desc = tr("Final IBL specular contribution: prefiltered * (F * scale + bias).");
            break;
        case quantiloom::DebugVisualizationMode::SkyAmbient:
            category = tr("IBL");
            desc = tr("Ambient sky lighting contribution (diffuse IBL).");
            break;

        // Spectral (50-59)
        case quantiloom::DebugVisualizationMode::XYZ_Tristimulus:
            category = tr("Spectral");
            desc = tr("CIE XYZ tristimulus values from spectral integration. Before RGB conversion.");
            break;
        case quantiloom::DebugVisualizationMode::BeforeChromaCorrection:
            category = tr("Spectral");
            desc = tr("Linear RGB before chromaticity correction. May show color shifts.");
            break;
        case quantiloom::DebugVisualizationMode::SpectralReflectance550:
            category = tr("Spectral");
            desc = tr("Material spectral reflectance sampled at 550nm (green reference).");
            break;

        // IR (60-69)
        case quantiloom::DebugVisualizationMode::Temperature:
            category = tr("Infrared");
            desc = tr("Surface temperature in Kelvin. Blue = cold, Red = hot (colormap).");
            break;
        case quantiloom::DebugVisualizationMode::IREmissivity:
            category = tr("Infrared");
            desc = tr("IR emissivity factor. 1 = perfect blackbody, 0 = perfect reflector.");
            break;
        case quantiloom::DebugVisualizationMode::IREmission:
            category = tr("Infrared");
            desc = tr("Thermal emission contribution: emissivity * Planck(T, lambda).");
            break;
        case quantiloom::DebugVisualizationMode::IRReflection:
            category = tr("Infrared");
            desc = tr("IR reflection of ambient thermal radiation.");
            break;

        default:
            category = tr("Unknown");
            desc = tr("Unknown debug mode.");
            break;
    }

    m_categoryLabel->setText(category);
    m_description->setText(desc);
}
