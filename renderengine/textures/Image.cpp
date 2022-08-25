#include "Image.h"

#define STB_IMAGE_IMPLEMENTATION

#include "../../lib/stb/stb_image.h"

int Image::getWidth() const {
    return m_width;
}

int Image::getHeight() const {
    return m_height;
}

float *Image::loadData(const std::string &path, float gamma) {
    int numComponents;
    stbi_ldr_to_hdr_gamma(gamma);
    stbi_set_flip_vertically_on_load(true);
    float *data = stbi_loadf(path.c_str(), &m_width, &m_height, &numComponents, 4);
    if (!data) {
        // TODO: error
    }
    return data;
}

void Image::freeData(float *data) {
    stbi_image_free(data);
}

void Image::loadImage(const std::string &path, float gamma) {
    float *data = loadData(path, gamma);
    m_pixels.resize(m_width * m_height);
    for (int y = 0; y < m_height; y++) {
        memcpy(&m_pixels[y * m_width],
               data + y * m_width * 4,
               4 * m_width * sizeof(float));
    }
    freeData(data);
}

glm::vec4 Image::getPixel(int x, int y, Image::WrapMode wrapMode) {
    switch (wrapMode) {
        case ZERO:
            if (x < 0 || x >= m_width || y < 0 || y >= m_height)
                return glm::vec4(0.f);
            break;
        case CLAMP:
            x = std::min(m_width - 1, std::max(0, x));
            y = std::min(m_height - 1, std::max(0, y));
            break;
        case REPEAT:
            while (x < 0)
                x += m_width;
            x = x % m_width;
            while (y < 0)
                y += m_height;
            y = y % m_height;
            break;
    }
    return m_pixels[x + y * m_width];
}

glm::vec4 Image::getPixelST(float s, float t, Image::WrapMode wrapMode) {
    return getPixel(static_cast<int>(s * static_cast<float>(m_width)), static_cast<int>(t * static_cast<float>(m_height)), wrapMode);
}
