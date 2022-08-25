#ifndef VOXELVOLUMERENDERER_LVAO_H
#define VOXELVOLUMERENDERER_LVAO_H

#include <GL/glew.h>

#include "../../../renderengine/shader/ShaderProgram.h"
#include "../../../renderengine/shader/ComputeShaderProgram.h"
#include "../../../renderengine/camera/ACamera.h"
#include "../../../renderengine/textures/Texture.h"

/**
 * Local Voxel Ambient Occlusion.
 *
 * @author Mirco Werner
 */
class LVAO {
public:
    void init();

    void execute(GLuint aoBufferTextureId, GLuint positionBufferTextureId, GLuint normalBufferTextureId,
                 GLuint voxelBufferTextureId, GLuint volumeTextureId, int width, int height);

    void cleanUp();

    void reload();

private:
    ComputeShaderProgram m_computeShaderProgram;
};

#endif