#ifndef VOXELVOLUMERENDERER_DDFE_H
#define VOXELVOLUMERENDERER_DDFE_H

#include <GL/glew.h>

#include "../../../renderengine/shader/ShaderProgram.h"
#include "../../../renderengine/shader/ComputeShaderProgram.h"
#include "../../../renderengine/camera/ACamera.h"
#include "../../../renderengine/textures/Texture.h"

/**
 * Directional Distance Field Emission.
 *
 * @author Mirco Werner
 */
class DDFE {
public:
    void init();

    void execute(GLuint emissionBufferTextureId, GLuint positionBufferTextureId,
                 GLuint normalBufferTextureId, GLuint rgbdfTextureId, GLuint cellPropertiesBufferId,
                 int width, int height);

    void cleanUp();

    void reload();

private:
    ComputeShaderProgram m_computeShaderProgram;
};

#endif