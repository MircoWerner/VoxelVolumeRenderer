/**
 * Voxel Volume Renderer.
 *
 * @author Mirco Werner
 */
#include "renderengine/engine/RenderEngine.h"
#include "renderengine/utils/IOUtils.h"

#include "volumerenderer/VolumeRenderLogic.h"

void generateTestScreenshots(GLFWwindow *window, IRenderLogic *renderLogic) {
    glfwSetWindowSize(window, 1920, 1080);
    glfwPollEvents();
    glfwSwapBuffers(window);

    glFinish();

    int number = 0;
    IRenderLogic::SceneScreenshotInfo info;
    while (renderLogic->generateSceneScreenshot(number, &info)) {
        glfwSetWindowSize(window, info.width, info.height);
        glfwPollEvents();

        glFinish();

        for (int i = 0; i < info.iterations; i++) {
            glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
            renderLogic->render();
            glfwSwapBuffers(window);
        }

        IOUtils::writeFramebufferToFile(info.fileName, info.width, info.height);

        number++;
    }
}

int main(int argc, char *argv[]) {
    GLFWwindow *window = RenderEngine::initGL("Voxel Volume Render", 1920, 1080);
    if (window == nullptr) {
        return -1;
    }
    RenderEngine::initImGui(window);

    IRenderLogic *renderLogic = new VolumeRenderLogic();

    // startup parameters
    bool generateImages = false;
    for (int i = 1; i < argc; i++) {
        renderLogic->m_startupParameters.emplace_back(argv[i]);
        if (std::string("--generate-images") == argv[i]) {
            generateImages = true;
        }
    }

    // startup for screenshot generation
    if (generateImages) {
        auto *renderEngine = new RenderEngine(window, renderLogic);
        renderEngine->init();
        generateTestScreenshots(window, renderLogic);
        renderEngine->cleanUp();
        delete renderEngine;
        return 0;
    }

    // normal startup
    auto *renderEngine = new RenderEngine(window, renderLogic);
    renderEngine->run();
    delete renderEngine;

    return 0;
}
