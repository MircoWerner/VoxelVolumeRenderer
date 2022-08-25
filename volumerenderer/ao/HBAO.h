#ifndef VOXELVOLUMERENDERER_HBAO_H
#define VOXELVOLUMERENDERER_HBAO_H

#include <GL/glew.h>

#include "../../../renderengine/shader/ShaderProgram.h"
#include "../../../renderengine/shader/ComputeShaderProgram.h"
#include "../../../renderengine/camera/ACamera.h"
#include "../../../renderengine/textures/Texture.h"
#include "../utils/BilateralFilter.h"

/**
 * Horizon-Based Ambient Occlusion.
 *
 * @author Mirco Werner
 */
class HBAO {
public:
    void init();

    void execute(GLuint ambientOcclusionBufferTextureId, GLuint depthBufferTextureId, GLuint normalBufferTextureId,
                 GLuint positionBufferTextureId, int width, int height, glm::mat4 MV, glm::mat4 inverseProjection);

    void cleanUp();

    void reload();

    int m_Nd = 8;
    int m_Ns = 8;
    float m_stepSize = 1.f;
    float m_R = 64.f;
    float m_tangentBias = 0.0f;

    bool m_executeBilateralFilter = true;
private:
    ComputeShaderProgram m_computeShaderProgram;
};

#endif