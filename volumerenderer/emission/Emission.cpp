#include "Emission.h"

void Emission::init() {
    m_vcte.init();
    m_ddfe.init();
}

void Emission::execute(GLuint emissionBufferTextureId, GLuint positionBufferTextureId,
                       GLuint normalBufferTextureId, GLuint voxelBufferTextureId, GLuint albedoTextureId,
                       GLuint alphaTextureId, GLuint rgbdfTextureId, GLuint volumeTextureId,
                       glm::ivec3 VOLUME_DIMENSION, int windowWidth, int windowHeight, glm::vec3 rayOrigin,
                       GLuint cellPropertiesBufferId) {
    switch (m_method) {
        case 1:
            m_vcte.execute(emissionBufferTextureId, positionBufferTextureId, normalBufferTextureId,
                           voxelBufferTextureId, albedoTextureId, alphaTextureId,
                           volumeTextureId, VOLUME_DIMENSION, windowWidth, windowHeight, rayOrigin,
                           cellPropertiesBufferId);
            break;
        case 2:
            m_ddfe.execute(emissionBufferTextureId, positionBufferTextureId,
                           normalBufferTextureId, rgbdfTextureId,
                           cellPropertiesBufferId, windowWidth, windowHeight);
            break;
    }
}

void Emission::cleanUp() {
    m_vcte.cleanUp();
    m_ddfe.cleanUp();
}

void Emission::reload() {
    m_vcte.reload();
    m_ddfe.reload();
}
