#ifndef VOXELVOLUMERENDERER_RTAO_H
#define VOXELVOLUMERENDERER_RTAO_H

#include <GL/glew.h>

#include "../../../renderengine/shader/ShaderProgram.h"
#include "../../../renderengine/shader/ComputeShaderProgram.h"
#include "../../../renderengine/camera/ACamera.h"
#include "../../../renderengine/textures/Texture.h"

/**
 * Ray Traced Ambient Occlusion.
 *
 * @author Mirco Werner
 */
class RTAO {
public:
    void init();

    void execute(GLuint aoBufferTextureId, GLuint positionBufferTextureId, GLuint normalBufferTextureId,
                 GLuint volumeTextureId, GLuint sdfTextureId, glm::ivec3 volumeDimension, int width, int height,
                 bool accumulationCall = false);

    void cleanUp();

    void reload();

    int m_accumulatedSamples = 0;
    int m_samples = 4;
    int m_totalSamples = 256;
    float m_distanceToHalfOcclusion = 256.f;
private:
    ComputeShaderProgram m_computeShaderProgram;
};

#endif