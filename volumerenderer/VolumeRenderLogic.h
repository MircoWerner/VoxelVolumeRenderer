#ifndef VOXELVOLUMERENDERER_VOLUMERENDERLOGIC_H
#define VOXELVOLUMERENDERER_VOLUMERENDERLOGIC_H

#include <GL/glew.h>

#include "../../renderengine/engine/IRenderLogic.h"
#include "../../renderengine/camera/ThirdPersonCamera.h"

#include "VolumeRenderer.h"
#include "utils/CoordinateSystemRenderer.h"

/**
 * Render logic of the program.
 * Handles inputs, gui and calls the volume renderer.
 *
 * @author Mirco Werner
 */
class VolumeRenderLogic : public IRenderLogic {
public:
    void init() override;

    void update(float time, KeyboardInput *keyboardInput, MouseInput *mouseInput) override;

    void render() override;

    void renderGui() override;

    void cleanUp() override;

    void onWindowResized(int width, int height) override;

    bool generateSceneScreenshot(int number, SceneScreenshotInfo *info) override;

private:
    ThirdPersonCamera m_camera;
    CoordinateSystemRenderer m_coordinateSystemRenderer;
    VolumeRenderer m_volumeRenderer;
    bool m_executeComputeShader = true; // only recalculate scene if the camera has moved

    glm::ivec2 windowSize = glm::ivec2(1920, 1080);

    bool wireframe = false;

    void teleportPos(int pos);

    enum CameraPosition {
        Position800Far = 0,
        Position800Near = 1,
        Position800Debug1 = 2,
        Position800Debug2 = 3,
        Position400Far = 4,
    };
};

#endif
