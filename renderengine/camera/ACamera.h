#ifndef RENDERENGINE_ACAMERA_H
#define RENDERENGINE_ACAMERA_H

#include <glm/glm.hpp>

/**
 * Camera which stores information about position and rotation.
 * Use implementation: ThirdPersonCamera
 *
 * @author Mirco Werner
 */
class ACamera {
public:
    glm::vec3 getPosition();
    void setPosition(float x, float y, float z);
    void setPosition(glm::vec3 position);

    glm::vec3 getRotation();
    void setRotation(float x, float y, float z);
    void setRotation(glm::vec3 rotation);

protected:
    glm::vec3 m_position = glm::vec3(0.f);
    glm::vec3 m_rotation = glm::vec3(0.f);
};

#endif