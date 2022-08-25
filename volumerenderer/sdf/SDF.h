#ifndef VOXELVOLUMERENDERER_SDF_H
#define VOXELVOLUMERENDERER_SDF_H

#include <GL/glew.h>

#include "../../../renderengine/shader/ShaderProgram.h"
#include "../../../renderengine/shader/ComputeShaderProgram.h"
#include "../../../renderengine/camera/ACamera.h"
#include "../../../renderengine/textures/Texture.h"

/**
 * Creates the Signed Distance Field.
 *
 * @author Mirco Werner
 */
class SDF {
public:
    /**
     * Call init(), execute(...) and cleanUp() afterwards.
     *
     * @param readFromFile read from disk if existing
     * @param writeToFile write to disk (overwrites if existing)
     * @param cellsFrameXXX path/name for the file on the disk "../resources/volume/<cellsFrameXXX>-sdf-cityblock-approx.raw"
     */
    SDF(bool readFromFile, bool writeToFile, std::string cellsFrameXXX);

    void init();

    GLuint execute(GLuint volumeTextureId, glm::ivec3 volumeDimension);

    void cleanUp();

    void reload();

private:
    ComputeShaderProgram m_computeShaderProgram;

    bool m_readFromFile;
    bool m_writeToFile;
    std::string m_cellsFrameXXX;
};

#endif