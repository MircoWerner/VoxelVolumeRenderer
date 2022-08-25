#ifndef VOXELVOLUMERENDERER_AMBIENTOCCLUSION_H
#define VOXELVOLUMERENDERER_AMBIENTOCCLUSION_H

#include "RTAO.h"
#include "DFAO.h"
#include "LVAO.h"
#include "VDCAO.h"
#include "HBAO.h"
#include "VCTAO.h"

/**
 * Controls the execution of the ambient occlusion shaders.
 *
 * @author Mirco Werner
 */
class AmbientOcclusion {
public:
    void init();

    void execute(GLuint ambientOcclusionBufferTextureId, GLuint positionBufferTextureId,
                 GLuint normalBufferTextureId, GLuint voxelBufferTextureId, GLuint depthBufferTextureId,
                 GLuint volumeTextureId, GLuint sdfTextureId, GLuint alphaTextureId,
                 glm::ivec3 VOLUME_DIMENSION, int windowWidth, int windowHeight, glm::mat4 viewMatrix,
                 glm::mat4 inverseProjection);

    void executeAccumulationCall(GLuint ambientOcclusionBufferTextureId, GLuint positionBufferTextureId,
                                 GLuint normalBufferTextureId, GLuint volumeTextureId,
                                 GLuint sdfTextureId, glm::ivec3 VOLUME_DIMENSION, int windowWidth,
                                 int windowHeight);

    void cleanUp();

    void reload();

    enum AO {
        AO_DISABLED = 0,
        AO_RTAO = 1,
        AO_DFAO = 2,
        AO_LVAO = 3,
        AO_VDCAO = 4,
        AO_HBAO = 5,
        AO_VCTAO = 6,
    };

    int m_method = AO_DISABLED;

    RTAO m_rtao;
    DFAO m_dfao;
    LVAO m_voxelao;
    VDCAO m_vdcao;
    HBAO m_hbao;
    VCTAO m_vctao;
};


#endif
