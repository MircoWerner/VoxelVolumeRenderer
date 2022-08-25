#include "Texture.h"

Texture::Texture(int textureUnit) : Image(), m_textureUnit(textureUnit) {

}

void Texture::load(const std::string &path, bool mipmapping) {
    float gamma = 1.0f;
    GLint internalFormat = GL_SRGB_ALPHA;
//    if (gamma == 2.2f) {
//        internalFormat = GL_SRGB_ALPHA;
//        gamma = 1.0f;
//    }

    float *data = loadData(path, gamma);

    glGenTextures(1, &m_textureId);

    glBindTexture(GL_TEXTURE_2D, m_textureId);

    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, m_width, m_height, 0, GL_RGBA, GL_FLOAT, data);

    if (mipmapping) {
        glGenerateMipmap(GL_TEXTURE_2D);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
    } else {
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    }
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
//
//    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
//    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
//
//    glGenerateMipmap(GL_TEXTURE_2D);

    glBindTexture(GL_TEXTURE_2D, 0);

    freeData(data);
}

void Texture::bind() const {
    glActiveTexture(GL_TEXTURE0 + m_textureUnit);
    glBindTexture(GL_TEXTURE_2D, m_textureId);
}

void Texture::unbind() {
    glBindTexture(GL_TEXTURE_2D, 0);
}
