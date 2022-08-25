#ifndef RENDERENGINE_RENDERENGINE_H
#define RENDERENGINE_RENDERENGINE_H

#include <GL/glew.h>
#include <GLFW/glfw3.h>
#include <string>

#include "IRenderLogic.h"

/**
 * Executes methods of the given IRenderLogic.
 *
 * @author Mirco Werner
 */
class RenderEngine {
public:
    static GLFWwindow *initGL(const std::string &title, int width, int height);

    static void initImGui(GLFWwindow *window);

    RenderEngine(GLFWwindow *window, IRenderLogic *renderLogic);

    ~RenderEngine();

    void run();

    void init();

    void cleanUp();

    constexpr static const float RED = 0.529f;
    constexpr static const float GREEN = 0.808f;
    constexpr static const float BLUE = 0.922f;
private:

    void renderLoop();

    void update();

    void render();

    void renderGui();

    void installCallbacks();

    GLFWwindow *m_window;
    IRenderLogic *m_renderLogic;
    KeyboardInput *m_keyboardInput;
    MouseInput *m_mouseInput;

    double m_lastTime = 0.0;
};

#endif