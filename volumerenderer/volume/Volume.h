#ifndef VOXELVOLUMERENDERER_VOLUME_H
#define VOXELVOLUMERENDERER_VOLUME_H

#include <GL/glew.h>
#include <glm/ext.hpp>

#include "properties/ACellProperties.h"
#include "Voxelization.h"

#include "converter/AVolumeConverter.h"

/**
 * Loads and handles the volume data and properties.
 *
 * @author Mirco Werner
 */
class Volume {

public:
    enum VOLUME_DATA_SET {
        INVALID = 0,
        V800 = 1,
        V400 = 2,
    };
    VOLUME_DATA_SET m_volumeDataSet = V800;

    static VOLUME_DATA_SET parseVolumeDataSet(std::string &str);

    /**
     * Loads the volume data (applies conversion) and cell properties.
     * Creates the SDF, directional RGB distance field and executes the "voxelization".
     */
    void load();

    void cleanUp();

    GLuint m_volumeTextureId = 0;
    GLuint m_sdfTextureId = 0;
    GLuint m_rgbdfTextureId = 0;

    glm::ivec3 m_volumeDimension;

    GLuint m_cellPropertiesBufferId = 0;

    ACellProperties *m_cellProperties;

    void reloadCellPropertiesBuffer(int id) const;

    void saveCellProperties() const;

    Voxelization m_voxelization;

    void recalculateEmissionInformation();

    int m_volumeMargin = 0;

private:
    void loadVolume(const std::string &pathVolume, const std::string &pathCSV, glm::ivec3 dimension, int margin,
                    AVolumeConverter<uint32_t> *volumeConverter);

    void createCellPropertiesBuffer();
};

#endif
