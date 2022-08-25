#ifndef RENDERENGINE_IMAGE_H
#define RENDERENGINE_IMAGE_H

#include <string>
#include <vector>
#include <glm/glm.hpp>

/**
 * Methods to load images from disk and access single pixels in different modes.
 *
 * @author Mirco Werner
 */
class Image {
public:
    int getWidth() const;
    int getHeight() const;

    void loadImage(const std::string& path, float gamma);

    enum WrapMode { ZERO, CLAMP, REPEAT };
    glm::vec4 getPixel(int x, int y, WrapMode wrapMode);
    glm::vec4 getPixelST(float s, float t, WrapMode warpMode);
protected:
    int m_width;
    int m_height;

    std::vector<glm::vec4> m_pixels;

    float* loadData(const std::string& path, float gamma);
    static void freeData(float *data);
};

#endif