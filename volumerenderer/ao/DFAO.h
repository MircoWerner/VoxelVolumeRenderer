#ifndef VOXELVOLUMERENDERER_DFAO_H
#define VOXELVOLUMERENDERER_DFAO_H

#include <GL/glew.h>

#include "../../../renderengine/shader/ShaderProgram.h"
#include "../../../renderengine/shader/ComputeShaderProgram.h"
#include "../../../renderengine/camera/ACamera.h"
#include "../../../renderengine/textures/Texture.h"

/**
 * Distance Field Ambient Occlusion.
 *
 * @author Mirco Werner
 */
class DFAO {
public:
    void init();

    void
    execute(GLuint aoBufferTextureId, GLuint positionBufferTextureId, GLuint normalBufferTextureId, GLuint sdfTextureId,
            glm::ivec3 volumeDimension, int width, int height);

    void cleanUp();

    void reload();

    int m_numberOfSteps = 5;
    float m_contrast = 0.125f;
private:
    ComputeShaderProgram m_computeShaderProgram;
};

#endif