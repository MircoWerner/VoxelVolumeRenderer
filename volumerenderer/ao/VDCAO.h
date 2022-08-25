#ifndef VOXELVOLUMERENDERER_VDCAO_H
#define VOXELVOLUMERENDERER_VDCAO_H

#include <GL/glew.h>

#include "../../../renderengine/shader/ShaderProgram.h"
#include "../../../renderengine/shader/ComputeShaderProgram.h"
#include "../../../renderengine/camera/ACamera.h"
#include "../../../renderengine/textures/Texture.h"

/**
 * Voxel Distance Field Cone Traced Ambient Occlusion.
 *
 * @author Mirco Werner
 */
class VDCAO {
public:
    void init();

    void execute(GLuint aoBufferTextureId, GLuint positionBufferTextureId,
                 GLuint normalBufferTextureId, GLuint voxelBufferTextureId, GLuint volumeTextureId,
                 GLuint sdfTextureId, glm::ivec3 volumeDimension, int width, int height);

    void cleanUp();

    void reload();

    int m_coneTraceSteps = 12;
    float m_coneTraceStepSize = 0.5f;
    float m_coneApertureAngle = 60.f; // 60 degrees
    float m_attenuation = 96.f;
private:
    ComputeShaderProgram m_computeShaderProgram;
};

#endif