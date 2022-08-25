#ifndef VOXELVOLUMERENDERER_COORDINATESYSTEMRENDERER_H
#define VOXELVOLUMERENDERER_COORDINATESYSTEMRENDERER_H

#include "GL/glew.h"

#include "../../renderengine/shader/ShaderProgram.h"
#include "../../renderengine/shader/ComputeShaderProgram.h"
#include "../../renderengine/camera/ACamera.h"
#include "../../renderengine/textures/Texture.h"

/**
 * Renders the coordinate axes on the top right corner of the screen.
 *
 * @author Mirco Werner
 */
class CoordinateSystemRenderer {
public:
    void init();

    void render(ACamera *camera, int windowWidth, int windowHeight);

    void cleanUp();

    bool m_render = false;
private:
    GLuint vaoId = 0;
    GLuint vertexVboId = 0;
    GLuint colorVboId = 0;
    GLuint indicesVboId = 0;

    GLsizei count = 0;

    ShaderProgram shaderProgram;
};

#endif