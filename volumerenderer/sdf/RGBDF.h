#ifndef VOXELVOLUMERENDERER_RGBDF_H
#define VOXELVOLUMERENDERER_RGBDF_H

#include <GL/glew.h>

#include "../../../renderengine/shader/ShaderProgram.h"
#include "../../../renderengine/shader/ComputeShaderProgram.h"
#include "../../../renderengine/camera/ACamera.h"
#include "../../../renderengine/textures/Texture.h"

/**
 * Creation of the RGB Distance Field.
 *
 * @author Mirco Werner
 */
class RGBDF {
public:
    /**
     * Call load(...) afterwards.
     *
     * @param readFromFile read from disk if existing
     * @param writeToFile write to disk (overwrites if existing)
     * @param cellsFrameXXX path/name for the file on the disk "../resources/volume/<cellsFrameXXX>-rgb_df.raw"
     */
    RGBDF(bool readFromFile, bool writeToFile, std::string cellsFrameXXX);

    GLuint load(GLuint volumeTextureId, glm::ivec3 volumeDimension, GLuint cellPropertiesBufferId);

private:
    void init();

    GLuint execute(GLuint volumeTextureId, glm::ivec3 volumeDimension, GLuint cellPropertiesBufferId);

    void cleanUp();

    ComputeShaderProgram m_computeShaderProgramInitialize;
    ComputeShaderProgram m_computeShaderProgramFilter;

    bool m_readFromFile;
    bool m_writeToFile;
    std::string m_cellsFrameXXX;
};

#endif