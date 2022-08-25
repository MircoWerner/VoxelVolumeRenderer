#include "Transformation.h"

glm::mat4 Transformation::getModelMatrix(glm::vec3 position, glm::vec3 rotation, glm::vec3 scale) {
    glm::mat4 modelMatrix(1.f);
    modelMatrix = glm::translate(modelMatrix, position);
    modelMatrix = glm::rotate(modelMatrix, rotation.x, glm::vec3(1, 0, 0));
    modelMatrix = glm::rotate(modelMatrix, rotation.y, glm::vec3(0, 1, 0));
    modelMatrix = glm::rotate(modelMatrix, rotation.z, glm::vec3(0, 0, 1));
    modelMatrix = glm::scale(modelMatrix, scale);
    return modelMatrix;
}

glm::mat4 Transformation::getViewMatrix(ACamera *camera) {
    glm::mat4 viewMatrix(1.f);;
    viewMatrix = glm::rotate(viewMatrix, camera->getRotation().x, glm::vec3(1, 0, 0));
    viewMatrix = glm::rotate(viewMatrix, camera->getRotation().y, glm::vec3(0, 1, 0));
    viewMatrix = glm::rotate(viewMatrix, camera->getRotation().z, glm::vec3(0, 0, 1));
    viewMatrix = glm::translate(viewMatrix, -camera->getPosition());
    return viewMatrix;
}

glm::mat4 Transformation::getProjectionMatrix(float fov, float width, float height, float zNear, float zFar) {
    return glm::perspective(fov, width / height, zNear, zFar);
}
