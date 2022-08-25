#ifndef VOXELVOLUMERENDERER_VCTE_H
#define VOXELVOLUMERENDERER_VCTE_H

#include <GL/glew.h>

#include "../../../renderengine/shader/ShaderProgram.h"
#include "../../../renderengine/shader/ComputeShaderProgram.h"
#include "../../../renderengine/camera/ACamera.h"
#include "../../../renderengine/textures/Texture.h"

/**
 * Voxel Cone Traced Emission.
 *
 * @author Mirco Werner
 */
class VCTE {
public:
    void init();

    void execute(GLuint emissionBufferTextureId, GLuint positionBufferTextureId, GLuint normalBufferTextureId,
                 GLuint voxelBufferTextureId, GLuint albedoTextureId,
                 GLuint alphaTextureId, GLuint volumeTextureId, glm::ivec3 volumeDimension, int width, int height,
                 glm::vec3 rayOrigin, GLuint cellPropertiesBufferId);

    void cleanUp();

    void reload();

    bool m_diffuse = true;
    int m_diffuseConeTraceSteps = 16;
    float m_diffuseConeTraceStepSize = 0.65f;
    float m_diffuseConeApertureAngle = 60.f; // 60 degrees

    bool m_specular = false;
    int m_specularConeTraceSteps = 64;
    float m_specularConeTraceStepSize = 0.5f;
    float m_specularConeApertureAngle = 5.f; // 5 degrees
private:
    ComputeShaderProgram m_computeShaderProgram;
};

#endif
