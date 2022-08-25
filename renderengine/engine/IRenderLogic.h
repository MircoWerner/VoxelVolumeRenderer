#ifndef RENDERENGINE_IRENDERLOGIC_H
#define RENDERENGINE_IRENDERLOGIC_H

#include "MouseInput.h"
#include "KeyboardInput.h"

#include <string>
#include <vector>

/**
 * Implements the logic of the program. Will be called by RenderEngine.
 *
 * @author Mirco Werner
 */
class IRenderLogic {
public:
    virtual void init() = 0;
    virtual void update(float time, KeyboardInput *keyboardInput, MouseInput *mouseInput) = 0;
    virtual void render() = 0;
    virtual void renderGui() = 0;
    virtual void cleanUp() = 0;
    virtual void onWindowResized(int width, int height) = 0;

    struct SceneScreenshotInfo {
        int width = 1920;
        int height = 1080;
        std::string fileName{};
        int iterations = 1;
    };
    virtual bool generateSceneScreenshot(int number, SceneScreenshotInfo *info) = 0;

    std::vector<std::string> m_startupParameters;
};

#endif