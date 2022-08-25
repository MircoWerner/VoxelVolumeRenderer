#include "MouseInput.h"

void MouseInput::update() {
    m_motion = glm::vec2(0.f);
    if (m_previousPosition.x > 0 && m_previousPosition.y > 0 && m_insideWindow) {
        m_motion.x = m_position.x - m_previousPosition.x;
        m_motion.y = m_position.y - m_previousPosition.y;
    }
    m_previousPosition = m_position;
}

glm::vec2 MouseInput::getPosition() {
    return m_position;
}

glm::vec2 MouseInput::getMotion() {
    return m_motion;
}

glm::vec2 MouseInput::getScroll(bool reset) {
    glm::vec2 scroll = m_scroll;
    if (reset) {
        m_scroll = glm::vec2(0.f);
    }
    return scroll;
}

bool MouseInput::isLeftButtonPressed() const {
    return m_leftButtonPressed;
}

bool MouseInput::isRightButtonPressed() const {
    return m_rightButtonPressed;
}

void MouseInput::onCursorPos(double x, double y) {
    m_position = glm::vec2(x, y);
}

void MouseInput::onCursorEnter(int entered) {
    m_insideWindow = entered;
}

void MouseInput::onMouseButton(int button, int action) {
    m_leftButtonPressed = button == GLFW_MOUSE_BUTTON_1 && action == GLFW_PRESS;
    m_rightButtonPressed = button == GLFW_MOUSE_BUTTON_2 && action == GLFW_PRESS;

    m_onLeftPressed = m_leftCurrentlyReleased && button == GLFW_MOUSE_BUTTON_1 && action == GLFW_PRESS;
    m_leftCurrentlyReleased = button == GLFW_MOUSE_BUTTON_1 && action == GLFW_RELEASE;

    m_onLeftReleased = m_leftCurrentlyPressed && button == GLFW_MOUSE_BUTTON_1 && action == GLFW_RELEASE;
    m_leftCurrentlyPressed = button == GLFW_MOUSE_BUTTON_1 && action == GLFW_PRESS;
}

void MouseInput::onScroll(double deltaX, double deltaY) {
    m_scroll = glm::vec2(deltaX, deltaY);
}
