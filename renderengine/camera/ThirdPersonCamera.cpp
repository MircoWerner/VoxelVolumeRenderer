#include "ThirdPersonCamera.h"

void ThirdPersonCamera::moveCenter(float offsetX, float offsetY, float offsetZ) {
    if (offsetZ != 0) {
        m_center.x -= (float) glm::sin(m_rotation.y) * offsetZ;
        m_center.z += (float) glm::cos(m_rotation.y) * offsetZ;
    }
    if (offsetX != 0) {
        m_center.x -= (float) glm::sin(m_rotation.y - PI_HALF) * offsetX;
        m_center.z += (float) glm::cos(m_rotation.y - PI_HALF) * offsetX;
    }
    m_center.y += offsetY;
    updateXYZ();
}

void ThirdPersonCamera::move(float offsetR, float offsetPhi, float offsetTheta) {
    m_r = glm::max(0.f, m_r + offsetR);
    m_phi = glm::min(PI_HALF, glm::max(-PI_HALF, m_phi + offsetPhi));
    m_theta = glm::mod(m_theta + offsetTheta + TWO_PI, TWO_PI);
    updateXYZ();
}

void ThirdPersonCamera::setCenter(float x, float y, float z) {
    m_center.x = x;
    m_center.y = y;
    m_center.z = z;
    updateXYZ();
}

void ThirdPersonCamera::setCenter(glm::vec3 center) {
    m_center = center;
    updateXYZ();
}

void ThirdPersonCamera::setR(float r) {
    m_r = r;
    updateXYZ();
}

void ThirdPersonCamera::setPhi(float phi) {
    m_phi = phi;
    updateXYZ();
}

void ThirdPersonCamera::setTheta(float theta) {
    m_theta = theta;
    updateXYZ();
}

void ThirdPersonCamera::updateXYZ() {
    setPosition(m_r * glm::cos(m_phi) * glm::sin(m_theta) + m_center.x,
                m_r * glm::sin(m_phi) + m_center.y,
                m_r * glm::cos(m_phi) * glm::cos(m_theta) + m_center.z);
    setRotation(m_phi, -m_theta, 0);
}

float ThirdPersonCamera::getR() const {
    return m_r;
}

float ThirdPersonCamera::getPhi() const {
    return m_phi;
}

float ThirdPersonCamera::getTheta() const {
    return m_theta;
}
