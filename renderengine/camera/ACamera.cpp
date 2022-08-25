#include "ACamera.h"

glm::vec3 ACamera::getPosition() {
    return m_position;
}

void ACamera::setPosition(float x, float y, float z) {
    m_position.x = x;
    m_position.y = y;
    m_position.z = z;
}

void ACamera::setPosition(glm::vec3 position) {
    m_position = position;
}

glm::vec3 ACamera::getRotation() {
    return m_rotation;
}

void ACamera::setRotation(float x, float y, float z) {
    m_rotation.x = x;
    m_rotation.y = y;
    m_rotation.z = z;
}

void ACamera::setRotation(glm::vec3 rotation) {
    m_rotation = rotation;
}
