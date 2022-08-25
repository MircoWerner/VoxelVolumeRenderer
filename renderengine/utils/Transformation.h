#ifndef RENDERENGINE_TRANSFORMATION_H
#define RENDERENGINE_TRANSFORMATION_H

#include <glm/ext.hpp>

#include "../camera/ACamera.h"

/**
 * Utility methods to create matrices from camera information.
 *
 * @author Mirco Werner
 */
class Transformation {
public:
    static glm::mat4 getModelMatrix(glm::vec3 position, glm::vec3 rotation, glm::vec3 scale);
    static glm::mat4 getViewMatrix(ACamera *camera);
    static glm::mat4 getProjectionMatrix(float fov, float width, float height, float zNear, float zFar);
};

#endif