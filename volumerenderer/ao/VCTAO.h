#ifndef VOXELVOLUMERENDERER_VCTAO_H
#define VOXELVOLUMERENDERER_VCTAO_H

#include <GL/glew.h>

#include "../../../renderengine/shader/ShaderProgram.h"
#include "../../../renderengine/shader/ComputeShaderProgram.h"
#include "../../../renderengine/camera/ACamera.h"
#include "../../../renderengine/textures/Texture.h"

/**
 * Voxel Cone Traced Ambient Occlusion.
 *
 * @author Mirco Werner
 */
class VCTAO {
public:
    void init();

    void execute(GLuint aoBufferTextureId, GLuint positionBufferTextureId, GLuint normalBufferTextureId,
                 GLuint alphaTextureId, glm::ivec3 volumeDimension, int width, int height);

    void cleanUp();

    void reload();

    int m_coneTraceSteps = 24;
    float m_coneTraceStepSize = 0.5f;
    float m_coneApertureAngle = 30.f; // 30 degrees
    float m_attenuation = 1024.f;
private:
    ComputeShaderProgram m_computeShaderProgram;
};

#endif