#include "DDFE.h"

void DDFE::init() {
    m_computeShaderProgram.init();
    m_computeShaderProgram.createComputeShader("../resources/shaders/volumerenderer/emission/ddfe.glsl");
    m_computeShaderProgram.link();
    m_computeShaderProgram.createUniform("tex_positionBuffer");
    m_computeShaderProgram.createUniform("tex_normalBuffer");
    m_computeShaderProgram.createUniform("tex_rgbdf");
    ShaderProgram::unbind();
}

void
DDFE::execute(GLuint emissionBufferTextureId, GLuint positionBufferTextureId,
              GLuint normalBufferTextureId, GLuint rgbdfTextureId, GLuint cellPropertiesBufferId,
              int width, int height) {
    m_computeShaderProgram.bind();

    m_computeShaderProgram.setUniform("tex_positionBuffer", 0);
    m_computeShaderProgram.setUniform("tex_normalBuffer", 1);
    m_computeShaderProgram.setUniform("tex_rgbdf", 2);

    glBindImageTexture(0, emissionBufferTextureId, 0, false, 0, GL_READ_WRITE, GL_RGBA32F);
    glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 1, cellPropertiesBufferId);

    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, positionBufferTextureId);
    glActiveTexture(GL_TEXTURE1);
    glBindTexture(GL_TEXTURE_2D, normalBufferTextureId);
    glActiveTexture(GL_TEXTURE2);
    glBindTexture(GL_TEXTURE_3D, rgbdfTextureId);

    m_computeShaderProgram.dispatch(width, height);
    glMemoryBarrier(GL_SHADER_IMAGE_ACCESS_BARRIER_BIT);

    glBindTexture(GL_TEXTURE_2D, 0);

    glBindImageTexture(0, 0, 0, false, 0, GL_READ_WRITE, GL_RGBA32F);
    glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 1, 0);

    ComputeShaderProgram::unbind();
}

void DDFE::cleanUp() {
    m_computeShaderProgram.cleanUp();
}

void DDFE::reload() {
    cleanUp();
    init();
}
