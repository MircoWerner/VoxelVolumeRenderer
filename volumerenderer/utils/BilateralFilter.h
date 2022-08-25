#ifndef VOXELVOLUMERENDERER_BILATERALFILTER_H
#define VOXELVOLUMERENDERER_BILATERALFILTER_H

#include "GL/glew.h"

#include "../../renderengine/shader/ShaderProgram.h"
#include "../../renderengine/shader/ComputeShaderProgram.h"
#include "../../renderengine/camera/ACamera.h"
#include "../../renderengine/textures/Texture.h"

/**
 * Adjusted bilateral filter.
 *
 * @author Mirco Werner
 */
class BilateralFilter {
public:
    void init();

    void execute(GLuint ambientOcclusionBufferTextureId, GLuint voxelBufferTextureId,
                 GLuint normalBufferTextureId, GLuint positionBufferTextureId, int width, int height,
                 GLuint m_ambientOcclusionBufferFilterTargetTextureId);

    void cleanUp();

    void reload();

    int m_kernelRadius = 4;
    float m_sigmaSpatial = 5.f;
private:
    ComputeShaderProgram m_computeShaderProgram;
};

#endif