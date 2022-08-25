#ifndef VOXELVOLUMERENDERER_VOXELIZATION_H
#define VOXELVOLUMERENDERER_VOXELIZATION_H

#include "GL/glew.h"

#include "../../renderengine/shader/ShaderProgram.h"
#include "../../renderengine/shader/ComputeShaderProgram.h"
#include "../../renderengine/camera/ACamera.h"
#include "../../renderengine/textures/Texture.h"

/**
 * "Voxelization".
 * Creates the textures with the emission sources information and local occlusion values used by VCTAO and VCTE.
 *
 * @author Mirco Werner
 */
class Voxelization {
public:
    /**
     * Execute the "voxelization", i.e. the compute shaders are executed that store the local occlusion values and emission values of the sources in the textures. The required mip maps are created too.
     *
     * @param volumeTextureId
     * @param volumeDimension
     * @param cellPropertiesBufferId
     */
    void voxelization(GLuint volumeTextureId, glm::ivec3 volumeDimension, GLuint cellPropertiesBufferId);

    void cleanUp();

    /**
     * Clean up and redo the "voxelization".
     *
     * @param volumeTextureId
     * @param volumeDimension
     * @param cellPropertiesBufferId
     */
    void recalculate(GLuint volumeTextureId, glm::ivec3 volumeDimension, GLuint cellPropertiesBufferId);

    GLuint m_alphaTextureId = 0;
    GLuint m_albedoTextureId = 0;
};

#endif