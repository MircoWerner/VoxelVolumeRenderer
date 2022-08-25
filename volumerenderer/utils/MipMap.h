#ifndef VOXELVOLUMERENDERER_MIPMAP_H
#define VOXELVOLUMERENDERER_MIPMAP_H

#include <GL/glew.h>

#include "../../../renderengine/shader/ShaderProgram.h"
#include "../../../renderengine/shader/ComputeShaderProgram.h"
#include "../../../renderengine/camera/ACamera.h"
#include "../../../renderengine/textures/Texture.h"

/**
 * Hardware accelerated mip map creation.
 *
 * @author Mirco Werner
 */
class MipMap {
public:
    void init(GLenum glInternalFormat, const std::string &shader);

    void execute(GLuint textureId, glm::ivec3 inputDimension, int levels);

    void cleanUp();

private:
    ComputeShaderProgram m_computeShaderProgram;

    GLenum m_glInternalFormat{};
};

#endif