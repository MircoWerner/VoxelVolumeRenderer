#ifndef RENDERENGINE_MOUSEINPUT_H
#define RENDERENGINE_MOUSEINPUT_H

#include <glm/ext.hpp>
#include <GLFW/glfw3.h>

/**
 * Methods to access the mouse.
 *
 * @author Mirco Werner
 */
class MouseInput {
public:
    void update();

    glm::vec2 getPosition();

    glm::vec2 getMotion();

    glm::vec2 getScroll(bool reset);

    bool isLeftButtonPressed() const;
    bool isRightButtonPressed() const;

    void onCursorPos(double x, double y);
    void onCursorEnter(int entered);
    void onMouseButton(int button, int action);
    void onScroll(double deltaX, double deltaY);

private:
    glm::vec2 m_position = glm::vec2(0.f);
    glm::vec2 m_previousPosition = glm::vec2(-1.f, -1.f);

    glm::vec2 m_motion = glm::vec2(0.f);
    glm::vec2 m_scroll = glm::vec2(0.f);

    bool m_insideWindow = false;

    bool m_leftButtonPressed = false;
    bool m_rightButtonPressed = false;
    bool m_leftCurrentlyReleased = true;
    bool m_leftCurrentlyPressed = false;
    bool m_onLeftReleased = false;
    bool m_onLeftPressed = false;
};

#endif