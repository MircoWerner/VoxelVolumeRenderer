#include "KeyboardInput.h"

KeyboardInput::KeyboardInput(GLFWwindow *window) : m_window(window) {

}

bool KeyboardInput::isKeyPressed(int keyCode) {
    return glfwGetKey(m_window, keyCode) == GLFW_PRESS;
}
