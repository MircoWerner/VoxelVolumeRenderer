#include "DFAO.h"

void DFAO::init() {
    m_computeShaderProgram.init();
    m_computeShaderProgram.createComputeShader("../resources/shaders/volumerenderer/ao/dfao.glsl");
    m_computeShaderProgram.link();
    m_computeShaderProgram.createUniform("tex_positionBuffer");
    m_computeShaderProgram.createUniform("tex_normalBuffer");
    m_computeShaderProgram.createUniform("tex_sdf");
    m_computeShaderProgram.createUniform("volumeDimension");
    m_computeShaderProgram.createUniform("numberOfSteps");
    m_computeShaderProgram.createUniform("contrast");
    ShaderProgram::unbind();
}

void DFAO::execute(GLuint aoBufferTextureId, GLuint positionBufferTextureId, GLuint normalBufferTextureId,
                   GLuint sdfTextureId, glm::ivec3 volumeDimension, int width,
                   int height) {
    m_computeShaderProgram.bind();

    m_computeShaderProgram.setUniform("tex_positionBuffer", 0);
    m_computeShaderProgram.setUniform("tex_normalBuffer", 1);
    m_computeShaderProgram.setUniform("tex_sdf", 2);
    m_computeShaderProgram.setUniform("volumeDimension", volumeDimension);
    m_computeShaderProgram.setUniform("numberOfSteps", m_numberOfSteps);
    m_computeShaderProgram.setUniform("contrast", m_contrast);

    glBindImageTexture(0, aoBufferTextureId, 0, false, 0, GL_READ_WRITE, GL_R32F);

    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, positionBufferTextureId);
    glActiveTexture(GL_TEXTURE1);
    glBindTexture(GL_TEXTURE_2D, normalBufferTextureId);
    glActiveTexture(GL_TEXTURE2);
    glBindTexture(GL_TEXTURE_3D, sdfTextureId);

    m_computeShaderProgram.dispatch(width, height);
    glMemoryBarrier(GL_SHADER_IMAGE_ACCESS_BARRIER_BIT);

    glBindTexture(GL_TEXTURE_2D, 0);

    glBindImageTexture(0, 0, 0, false, 0, GL_READ_WRITE, GL_R32F);

    ComputeShaderProgram::unbind();
}

void DFAO::cleanUp() {
    m_computeShaderProgram.cleanUp();
}

void DFAO::reload() {
    cleanUp();
    init();
}
