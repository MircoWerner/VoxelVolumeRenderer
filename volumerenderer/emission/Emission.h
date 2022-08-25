#ifndef VOXELVOLUMERENDERER_EMISSION_H
#define VOXELVOLUMERENDERER_EMISSION_H

#include "VCTE.h"
#include "DDFE.h"

/**
 * Controls the execution of the emission shaders.
 *
 * @author Mirco Werner
 */
class Emission {
public:
    void init();

    void execute(GLuint emissionBufferTextureId, GLuint positionBufferTextureId,
                 GLuint normalBufferTextureId, GLuint voxelBufferTextureId, GLuint albedoTextureId,
                 GLuint alphaTextureId, GLuint rgbdfTextureId, GLuint volumeTextureId,
                 glm::ivec3 VOLUME_DIMENSION, int windowWidth, int windowHeight, glm::vec3 rayOrigin,
                 GLuint cellPropertiesBufferId);

    void cleanUp();

    void reload();

    enum EM {
        EM_DISABLED = 0,
        EM_VCTE = 1,
        EM_DDFE = 2,
    };

    int m_method = EM_DISABLED;

    VCTE m_vcte;
    DDFE m_ddfe;
};

#endif
