#ifndef VOXELVOLUMERENDERER_VOLUMERENDERER_H
#define VOXELVOLUMERENDERER_VOLUMERENDERER_H

#include <GL/glew.h>

#include "../../../renderengine/shader/ShaderProgram.h"
#include "../../../renderengine/shader/ComputeShaderProgram.h"
#include "../../../renderengine/camera/ACamera.h"
#include "../../../renderengine/textures/Texture.h"

#include "volume/Voxelization.h"

#include "utils/BilateralFilter.h"

#include "ao/AmbientOcclusion.h"
#include "emission/Emission.h"

#include "volume/Volume.h"

/**
 * Renders the volume.
 *
 * @author Mirco Werner
 */
class VolumeRenderer {
public:
    /**
     * Initializes all shaders and load the volume.
     *
     * @param windowWidth
     * @param windowHeight
     */
    void init(int windowWidth, int windowHeight);

    /**
     * Renders the volume.
     *
     * @param camera
     * @param windowWidth
     * @param windowHeight
     * @param executeComputeShader true if the camera moved and the scene information has to be recomputed (execution of ray marching, ao and emission shaders), false otherwise (rendering from the buffers)
     */
    void render(ACamera *camera, int windowWidth, int windowHeight, bool executeComputeShader);

    void cleanUp();

    /**
     * Has to be called when the window is resized to resize the buffers.
     *
     * @param windowWidth new width
     * @param windowHeight new height
     */
    void resizeFramebuffers(int windowWidth, int windowHeight);

    void reloadVertexFragmentShader();

    void reloadVolumeComputeShader();

    /**
     * Filter the ambient occlusion values.
     *
     * @param width window/buffer width
     * @param height window/buffer height
     */
    void executeBilateralFilter(int width, int height);

    // AO
    AmbientOcclusion m_ambientOcclusion;

    // Emission
    Emission m_emission;
    float m_emissionFactor = 0.5; // strength of the gathered emission

    // ambient occlusion bilateral filter
    BilateralFilter m_bilateralFilter;

    // fragment
    enum FragmentDebugMode {
        DebugDisabled = 0,
        DebugNormal = 1,
        DebugDepth = 2,
        DebugAlbedo = 3,
        DebugAmbientOcclusion = 4,
        DebugEmission = 5
    };
    int m_fragmentDebugMode = FragmentDebugMode::DebugDisabled; // render buffer contents to the screen for debug purposes

    // debug sdf
    bool m_debugSdf = false;
    int m_debugSdfValue = 0;

    // volume
    Volume m_volume;

private:
    GLuint vaoId = 0;
    GLuint vertexVboId = 0;
    GLuint textureVboId = 0;
    GLuint indicesVboId = 0;

    GLsizei count = 0;

    ShaderProgram shaderProgram;
    ComputeShaderProgram computeShaderProgram;

    GLuint m_positionBufferTextureId = 0;
    GLuint m_colorBufferTextureId = 0;
    GLuint m_normalBufferTextureId = 0;
    GLuint m_ambientOcclusionBufferTextureId = 0;
    GLuint m_ambientOcclusionBufferFilterTargetTextureId = 0;
    GLuint m_depthBufferTextureId = 0;
    GLuint m_voxelBufferTextureId = 0;
    GLuint m_emissionBufferTextureId = 0;

    void cleanUpFramebuffers();

    static glm::vec3 ndcToWorldSpace(float ndcX, float ndcY, glm::mat4 inverseProjection, glm::mat4 inverseView);

    static GLuint createImageBuffer(int windowWidth, int windowHeight, int imageFormat);

    void swapAmbientOcclusionBuffers();
};

#endif
