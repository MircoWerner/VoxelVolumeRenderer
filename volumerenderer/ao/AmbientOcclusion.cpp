#include "AmbientOcclusion.h"

void AmbientOcclusion::init() {
    m_rtao.init();
    m_dfao.init();
    m_voxelao.init();
    m_vdcao.init();
    m_hbao.init();
    m_vctao.init();
}

void AmbientOcclusion::execute(GLuint ambientOcclusionBufferTextureId, GLuint positionBufferTextureId,
                               GLuint normalBufferTextureId, GLuint voxelBufferTextureId, GLuint depthBufferTextureId,
                               GLuint volumeTextureId, GLuint sdfTextureId, GLuint alphaTextureId,
                               glm::ivec3 VOLUME_DIMENSION, int windowWidth, int windowHeight, glm::mat4 viewMatrix,
                               glm::mat4 inverseProjection) {
    switch (m_method) {
        case AO_RTAO:
            m_rtao.execute(ambientOcclusionBufferTextureId, positionBufferTextureId, normalBufferTextureId,
                           volumeTextureId, sdfTextureId, VOLUME_DIMENSION, windowWidth, windowHeight);
            break;
        case AO_DFAO:
            m_dfao.execute(ambientOcclusionBufferTextureId, positionBufferTextureId, normalBufferTextureId,
                           sdfTextureId, VOLUME_DIMENSION, windowWidth, windowHeight);
            break;
        case AO_LVAO:
            m_voxelao.execute(ambientOcclusionBufferTextureId, positionBufferTextureId, normalBufferTextureId,
                              voxelBufferTextureId, volumeTextureId, windowWidth, windowHeight);
            break;
        case AO_VDCAO:
            m_vdcao.execute(ambientOcclusionBufferTextureId,
                            positionBufferTextureId, normalBufferTextureId,
                            voxelBufferTextureId, volumeTextureId, sdfTextureId,
                            VOLUME_DIMENSION, windowWidth, windowHeight);
            break;
        case AO_HBAO:
            m_hbao.execute(ambientOcclusionBufferTextureId, depthBufferTextureId, normalBufferTextureId,
                           positionBufferTextureId,
                           windowWidth, windowHeight, viewMatrix, inverseProjection);
            break;
        case AO_VCTAO:
            m_vctao.execute(ambientOcclusionBufferTextureId,
                            positionBufferTextureId,
                            normalBufferTextureId, alphaTextureId,
                            VOLUME_DIMENSION,
                            windowWidth,
                            windowHeight);
            break;
    }
}

void AmbientOcclusion::executeAccumulationCall(GLuint ambientOcclusionBufferTextureId, GLuint positionBufferTextureId,
                                               GLuint normalBufferTextureId, GLuint volumeTextureId,
                                               GLuint sdfTextureId, glm::ivec3 VOLUME_DIMENSION, int windowWidth,
                                               int windowHeight) {
    if (m_method == 1) {
        m_rtao.execute(ambientOcclusionBufferTextureId, positionBufferTextureId, normalBufferTextureId,
                       volumeTextureId, sdfTextureId, VOLUME_DIMENSION, windowWidth, windowHeight, true);
    }
}

void AmbientOcclusion::cleanUp() {
    m_rtao.cleanUp();
    m_dfao.cleanUp();
    m_voxelao.cleanUp();
    m_vdcao.cleanUp();
    m_hbao.cleanUp();
    m_vctao.cleanUp();
}

void AmbientOcclusion::reload() {
    m_rtao.reload();
    m_dfao.reload();
    m_voxelao.reload();
    m_vdcao.reload();
    m_hbao.reload();
    m_vctao.reload();
}
