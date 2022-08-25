#include "Volume.h"

#include <algorithm>

#include "converter/VolumeConverter1000.h"
#include "converter/VolumeConverter400.h"
#include "properties/CellProperties1000.h"
#include "properties/CellProperties400.h"
#include "../sdf/SDF.h"
#include "../sdf/DRGBDF.h"
#include "../sdf/RGBDF.h"
#include "../utils/BinaryFileRW.h"

void Volume::createCellPropertiesBuffer() {
    static const int PROPERTIES = 6;
    auto *data = new float[PROPERTIES * m_cellProperties->m_properties.size()];
    for (int i = 0; i < m_cellProperties->m_properties.size(); i++) {
        data[i * PROPERTIES + 0] = static_cast<float>(m_cellProperties->m_properties[i].rgba[0]);
        data[i * PROPERTIES + 1] = static_cast<float>(m_cellProperties->m_properties[i].rgba[1]);
        data[i * PROPERTIES + 2] = static_cast<float>(m_cellProperties->m_properties[i].rgba[2]);
        data[i * PROPERTIES + 3] = static_cast<float>(m_cellProperties->m_properties[i].rgba[3]);
        data[i * PROPERTIES + 4] = static_cast<float>(m_cellProperties->m_properties[i].emittance);
        data[i * PROPERTIES + 5] = static_cast<float>(m_cellProperties->m_properties[i].roughness);
    }
    glGenBuffers(1, &m_cellPropertiesBufferId);
    glBindBuffer(GL_SHADER_STORAGE_BUFFER, m_cellPropertiesBufferId);
    glBufferData(GL_SHADER_STORAGE_BUFFER, sizeof(float) * PROPERTIES * m_cellProperties->m_properties.size(), data,
                 GL_STATIC_DRAW);
    glBindBuffer(GL_SHADER_STORAGE_BUFFER, 0);
    delete[] data;
}

void Volume::reloadCellPropertiesBuffer(int id) const {
    static const int PROPERTIES = 6;
    auto *data = new float[PROPERTIES];
    data[0] = static_cast<float>(m_cellProperties->m_properties[id].rgba[0]);
    data[1] = static_cast<float>(m_cellProperties->m_properties[id].rgba[1]);
    data[2] = static_cast<float>(m_cellProperties->m_properties[id].rgba[2]);
    data[3] = static_cast<float>(m_cellProperties->m_properties[id].rgba[3]);
    data[4] = static_cast<float>(m_cellProperties->m_properties[id].emittance);
    data[5] = static_cast<float>(m_cellProperties->m_properties[id].roughness);
    glBindBuffer(GL_SHADER_STORAGE_BUFFER, m_cellPropertiesBufferId);
    glBufferSubData(GL_SHADER_STORAGE_BUFFER, sizeof(float) * PROPERTIES * id, sizeof(float) * PROPERTIES, data);
    glBindBuffer(GL_SHADER_STORAGE_BUFFER, 0);
    delete[] data;
}

void Volume::saveCellProperties() const {
    m_cellProperties->writePropertiesCSV();
}

void Volume::recalculateEmissionInformation() {
    m_voxelization.recalculate(m_volumeTextureId, m_volumeDimension,
                               m_cellPropertiesBufferId);
}

void Volume::load() {
    std::string pathVolume;
    std::string pathCSV;
    glm::ivec3 dimension;
    int margin;
    AVolumeConverter<uint32_t> *volumeConverter;
    switch (m_volumeDataSet) {
        case V800:
            pathVolume = "1000/cells_frame055";
            pathCSV = "1000/output_cells-00055";
            dimension = glm::ivec3(1000);
            margin = 100;
            volumeConverter = new VolumeConverter1000();
            m_cellProperties = new CellProperties1000("../resources/volume/" + pathVolume + "-properties.csv");
            break;
        case V400:
            pathVolume = "400/cells_frame024";
            pathCSV = "400/output_cells-00249";
            dimension = glm::ivec3(400, 400, 402);
            margin = 0;
            volumeConverter = new VolumeConverter400();
            m_cellProperties = new CellProperties400("../resources/volume/" + pathVolume + "-properties.csv");
            break;
        default:
            throw std::runtime_error("Volume data set does not exist.");
    }

    std::chrono::steady_clock::time_point begin = std::chrono::steady_clock::now();
    loadVolume(pathVolume, pathCSV, dimension, margin, volumeConverter);
    delete volumeConverter;
    m_cellProperties->loadPropertiesCSV(true, true);
    createCellPropertiesBuffer();
    std::chrono::steady_clock::time_point end = std::chrono::steady_clock::now();
    std::cout << "[VOLUME_CONVERSION_Total] Time = "
              << std::chrono::duration_cast<std::chrono::microseconds>(end - begin).count()
              << "[µs]" << std::endl;

    {
        std::chrono::steady_clock::time_point begin = std::chrono::steady_clock::now();
//        RGBDF rgbdf(true, true, pathVolume);
        DRGBDF rgbdf(true, true, pathVolume);
        m_rgbdfTextureId = rgbdf.load(m_volumeTextureId, m_volumeDimension, m_cellPropertiesBufferId);
        std::chrono::steady_clock::time_point end = std::chrono::steady_clock::now();
        std::cout << "[DIRECTIONAL_RGB_DF_Total] Time = "
                  << std::chrono::duration_cast<std::chrono::microseconds>(end - begin).count()
                  << "[µs]" << std::endl;
    }

    {
        std::chrono::steady_clock::time_point begin = std::chrono::steady_clock::now();
        SDF sdf(true, true, pathVolume);
        sdf.init();
        m_sdfTextureId = sdf.execute(m_volumeTextureId, m_volumeDimension);
        sdf.cleanUp();
        std::chrono::steady_clock::time_point end = std::chrono::steady_clock::now();
        std::cout << "[SDF_Total] Time = "
                  << std::chrono::duration_cast<std::chrono::microseconds>(end - begin).count()
                  << "[µs]" << std::endl;
    }

    {
        std::chrono::steady_clock::time_point begin = std::chrono::steady_clock::now();
        m_voxelization.voxelization(m_volumeTextureId, m_volumeDimension, m_cellPropertiesBufferId);
        std::chrono::steady_clock::time_point end = std::chrono::steady_clock::now();
        std::cout << "[VOXELIZATION_Total] Time = "
                  << std::chrono::duration_cast<std::chrono::microseconds>(end - begin).count()
                  << "[µs]" << std::endl;
    }
}

void Volume::cleanUp() {
    glDeleteTextures(1, &m_volumeTextureId);
    glDeleteTextures(1, &m_sdfTextureId);
    glDeleteTextures(1, &m_rgbdfTextureId);
    glDeleteBuffers(1, &m_cellPropertiesBufferId);
    m_voxelization.cleanUp();
    delete m_cellProperties;
}

void Volume::loadVolume(const std::string &pathVolume, const std::string &pathCSV, glm::ivec3 dimension, int margin,
                        AVolumeConverter<uint32_t> *volumeConverter) {
    m_volumeMargin = margin;

    if (!BinaryFileRW::exists("../resources/volume/" + pathVolume + "-converted.raw")) {
        std::unordered_map<uint32_t, uint16_t> types;
        std::vector<uint8_t> volume;

        volumeConverter->loadVolume("../resources/volume/" + pathVolume + ".raw",
                                    "../resources/volume/" + pathCSV + ".csv", &volume, margin);

        BinaryFileRW::writeBytes("../resources/volume/" + pathVolume + "-converted.raw",
                                 volumeConverter->m_volumeDimensionX * volumeConverter->m_volumeDimensionY *
                                 volumeConverter->m_volumeDimensionZ,
                                 volume.data());
    }

    m_volumeDimension = dimension - 2 * m_volumeMargin;

    auto *volume = new uint8_t[m_volumeDimension.x * m_volumeDimension.y * m_volumeDimension.z];
    BinaryFileRW::readBytes("../resources/volume/" + pathVolume + "-converted.raw",
                            m_volumeDimension.x * m_volumeDimension.y * m_volumeDimension.z, volume);

    glEnable(GL_TEXTURE_3D);
    glGenTextures(1, &m_volumeTextureId);
    glBindTexture(GL_TEXTURE_3D, m_volumeTextureId);
    glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_WRAP_S, GL_CLAMP);
    glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_WRAP_T, GL_CLAMP);
    glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_WRAP_R, GL_CLAMP);
    glTexImage3D(GL_TEXTURE_3D, 0, GL_R8UI, m_volumeDimension.x, m_volumeDimension.y, m_volumeDimension.z, 0,
                 GL_RED_INTEGER,
                 GL_UNSIGNED_BYTE, volume);
    glBindTexture(GL_TEXTURE_3D, 0);

    delete[] volume;
}

Volume::VOLUME_DATA_SET Volume::parseVolumeDataSet(std::string &str) {
    std::string strLower;
    strLower.reserve(str.size());
    std::transform(str.begin(), str.end(), std::back_inserter(strLower), ::tolower);
    if (strLower == "--v800") {
        return VOLUME_DATA_SET::V800;
    } else if (strLower == "--v400") {
        return Volume::V400;
    } else {
        return Volume::INVALID;
    }
}

