#ifndef RENDERENGINE_TEXTURE_H
#define RENDERENGINE_TEXTURE_H

#include <GL/glew.h>

#include "Image.h"

/**
 * 2D texture.
 *
 * @author Mirco Werner
 */
class Texture : public Image {
public:
    explicit Texture(int textureUnit);

    void load(const std::string &path, bool mipmapping = true);

    void bind() const;
    static void unbind() ;
private:
    GLuint m_textureId = 0;
    int m_textureUnit;
};

#endif